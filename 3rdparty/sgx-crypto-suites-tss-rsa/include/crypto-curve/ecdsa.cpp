/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "ecdsa.h"
#include "crypto-bn/rand.h"
#include "crypto-encode/base64.h"
#include "curve.h"
#include "openssl_curve_wrapper.h"
#include <openssl/ec.h>
#include "exception/safeheron_exceptions.h"

using std::string;
using safeheron::bignum::BN;
using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::exception::LocatedException;
using safeheron::exception::OpensslException;

namespace safeheron{
namespace curve {
namespace ecdsa {

bool RecoverPublicKey(safeheron::curve::CurvePoint &pub, safeheron::curve::CurveType c_type, const safeheron::bignum::BN &h, const safeheron::bignum::BN &r, const safeheron::bignum::BN &s, uint32_t j) {
    if(( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )");
    }

    if(j > 3) return false;
    const safeheron::curve::Curve *curv = safeheron::curve::GetCurveParam(c_type);
    // A set LSB signifies that the y-coordinate is odd
    bool is_y_odd = j & 1;
    bool is_second_key = j >> 1;
    if (is_second_key && (r >= (curv->p % curv->n))) return false;

    // x = r + j*n
    BN x;
    if (is_second_key){
        x = r + curv->n * j;
    } else{
        x = r;
    }
    CurvePoint R;
    bool ok = R.PointFromX(x, is_y_odd, c_type);
    if (!ok) return false;
    BN r_inv = r.InvM(curv->n);
    BN n_m = curv->n - h % curv->n;
    BN u1 = (n_m * r_inv) % curv->n;
    BN u2 = (s * r_inv) % curv->n;

    pub = curv->g * u1 + R * u2;
    return true;
}

bool RecoverPublicKey(safeheron::curve::CurvePoint &pub, const CurveType c_type, const uint8_t *sig64, uint32_t sig_len, const uint8_t *digest32, uint32_t digest32_len, uint32_t v){
    BN m = BN::FromBytesBE(digest32, digest32_len);
    BN r = BN::FromBytesBE(sig64, 32);
    BN s = BN::FromBytesBE(sig64 + 32, 32);
    return RecoverPublicKey(pub, c_type, m , r , s, v);
}

bool VerifyPublicKey(const safeheron::curve::CurvePoint &expected_pub, safeheron::curve::CurveType c_type, const safeheron::bignum::BN &h, const safeheron::bignum::BN &r, const safeheron::bignum::BN &s, uint32_t v){
    if(( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )");
    }
    safeheron::curve::CurvePoint pub;
    bool ok = RecoverPublicKey(pub, c_type, h, r, s, v);
    if (!ok) return false;
    return pub == expected_pub;
}

bool VerifyPublicKey(const CurvePoint &pub, const CurveType c_type,
                     const uint8_t *sig64, uint32_t sig_len,
                     const uint8_t *digest32, uint32_t digest32_len,
                     uint32_t v) {
    if(( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )");
    }
    BN m = BN::FromBytesBE(digest32, digest32_len);
    BN r = BN::FromBytesBE(sig64, 32);
    BN s = BN::FromBytesBE(sig64 + 32, 32);
    return VerifyPublicKey(pub, c_type, m, r, s, v);
}

void Sign(const CurveType c_type, const BN &priv, const uint8_t *digest32, uint8_t *sig64){
    if(( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )");
    }
    unsigned char sk[32];
    const curve::Curve *curv = curve::GetCurveParam(c_type);

    priv.ToBytes32BE((uint8_t *)sk);

    int ret = safeheron::_openssl_curve_wrapper::sign_digest(curv->grp, sk, digest32, sig64);
    if (0 != ret) {
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, ret, "(ret = safeheron::_openssl_curve_wrapper::sign_digest(curv->grp, sk, digest32, sig)) != 0");
    }
}

bool Verify(const CurveType c_type, const CurvePoint &pub,
            const uint8_t *digest32, const uint8_t *sig64)
{
    assert(sig && digest32);
    if(( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "( c_type != CurveType::SECP256K1 ) && (c_type != CurveType::P256 )");
    }

    if( c_type != pub.GetCurveType() ) return false;

    unsigned char pub_key[65];
    pub.EncodeFull(pub_key);

    return 0 == safeheron::_openssl_curve_wrapper::verify_digest(pub.GetEcdsaCurveGrp(), pub_key, digest32, sig64);
}

bool Sig64ToDer(const uint8_t *sig64, uint8_t *der)
{
    assert(sig && der);
    bool ret = 0;
    int der_len = 0;
    BIGNUM* bn_r = nullptr;
    BIGNUM* bn_s = nullptr;
    ECDSA_SIG* ecdsa_sig = nullptr;
    unsigned char* p = nullptr;

    if (!sig64 || !der) {
        return false;
    }

    if (!(bn_r = BN_new()) ||
        !(bn_s = BN_new())) {
        ret = false;
        goto err;
    }

    if (!BN_bin2bn(sig64, 32, bn_r) ||
        !BN_bin2bn(sig64 + 32, 32, bn_s)) {
        ret = false;
        goto err;
    }

    if (!(ecdsa_sig = ECDSA_SIG_new()) ||
        (ret = ECDSA_SIG_set0(ecdsa_sig, bn_r, bn_s)) != 1) {
        ret = false;
        goto err;
    }

    p = der;
    if ((ret = i2d_ECDSA_SIG(ecdsa_sig, &p)) <= 0) {
        ret = false;
        goto err;
    }

    ret = true;

    err:
    if (ecdsa_sig) {
        ECDSA_SIG_free(ecdsa_sig);
        ecdsa_sig = nullptr;
    }
    if (bn_s) {
        BN_free(bn_s);
        bn_s = nullptr;
    }
    if (bn_r) {
        BN_free(bn_r);
        bn_r = nullptr;
    }
    return ret;
}

bool DerToSig64(const uint8_t *der, size_t der_len, uint8_t sig64[64])
{
    bool ret = 0;
    int r_len = 0;
    int s_len = 0;
    uint8_t r[32] = {0};
    uint8_t s[32] = {0};
    ECDSA_SIG* ecdsa_sig = nullptr;
    const BIGNUM* bn_r = nullptr;
    const BIGNUM* bn_s = nullptr;
    const unsigned char* p = nullptr;

    if (!der || der_len <= 0) {
        return false;
    }

    p = der;
    if (!d2i_ECDSA_SIG(&ecdsa_sig, &p, der_len)) {
        return false;
    }

    if (!(bn_r = ECDSA_SIG_get0_r(ecdsa_sig)) ||
        !(bn_s = ECDSA_SIG_get0_s(ecdsa_sig))) {
        ret = false;
        goto err;
    }

    if ((r_len = BN_bn2bin(bn_r, r)) <= 0 ||
        r_len > 32) {
        ret = false;
        goto err;
    }
    if ((s_len = BN_bn2bin(bn_s, s)) <= 0 ||
        s_len > 32) {
        ret = false;
        goto err;
    }

    memcpy(sig64 + (32-r_len), r, r_len);
    memcpy(sig64 + 32 + (32-s_len), s, s_len);

    ret = true;

    err:
    if (ecdsa_sig) {
        ECDSA_SIG_free(ecdsa_sig);
        ecdsa_sig = nullptr;
    }
    return ret;
}

}
}
}
