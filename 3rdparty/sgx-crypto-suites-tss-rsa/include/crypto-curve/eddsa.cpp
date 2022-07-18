/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "eddsa.h"
#include "exception/safeheron_exceptions.h"
#include "ed25519_ex.h"

using safeheron::exception::LocatedException;

namespace safeheron{
namespace curve {
namespace eddsa {

std::string Sign(const CurveType c_type,
                 const safeheron::bignum::BN &priv,
                 const CurvePoint &pub,
                 const uint8_t *msg, size_t len){
    if( c_type != CurveType::ED25519){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, "c_type != CurveType::ED25519");
    }

    unsigned char sk[32];
    const curve::Curve *curv = curve::GetCurveParam(c_type);

    priv.ToBytes32LE((uint8_t *)sk);
    ed25519_public_key pub32;
    pub.EncodeEdwardsPoint(pub32) ;
    ed25519_signature RS;
    ed25519_sign(msg, len, sk, pub32, RS);

    std::string ret;
    ret.assign(reinterpret_cast<const char *>(RS), sizeof(ed25519_signature));
    memset(RS, 0, sizeof(ed25519_signature));
    return ret;
}

bool verify(const CurveType c_type, const CurvePoint &pub,
            const uint8_t *sig, const uint8_t *msg, size_t len){
    if( c_type != CurveType::ED25519 ){
        throw LocatedException(__FILE__, __LINE__, __FUNCTION__, -1, " c_type != CurveType::ED25519 ");
    }

    ed25519_public_key pub32;
    pub.EncodeEdwardsPoint(pub32) ;
    ed25519_signature RS;
    return 0 == ed25519_sign_open(msg, len, pub32, RS);
}

}
}
}
