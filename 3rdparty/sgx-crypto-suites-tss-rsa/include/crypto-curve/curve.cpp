/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "curve.h"
#include <openssl/ec.h>
#include <openssl/obj_mac.h>

namespace safeheron{
namespace curve{

/**
 * Secp256k1
 *
 */
const static Curve Secp256k1(
        safeheron::bignum::BN("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F", 16),
        safeheron::bignum::BN("0", 16),
        safeheron::bignum::BN("7", 16),
        safeheron::bignum::BN("0", 16), // undefined 'c' for Secp256k1
        safeheron::bignum::BN("0", 16), // undefined 'd' for Secp256k1
        safeheron::bignum::BN("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16),
        CurvePoint(
                safeheron::bignum::BN("79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798", 16),
                safeheron::bignum::BN("483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8", 16),
                CurveType::SECP256K1));

/**
 * Ed25519
 *
 */
const static Curve Ed25519(
        safeheron::bignum::BN("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed", 16),
        safeheron::bignum::BN("-1", 10),
        safeheron::bignum::BN("0", 16), // undefined 'b' for Ed25519
        safeheron::bignum::BN("1", 10),
        safeheron::bignum::BN("52036cee2b6ffe738cc740797779e89800700a4d4141d8ab75eb4dca135978a3", 16),
        safeheron::bignum::BN("1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed", 16),
        CurvePoint(
                safeheron::bignum::BN("216936d3cd6e53fec0a4e231fdd6dc5c692cc7609525a7b2c9562d608f25d51a", 16),
                safeheron::bignum::BN("6666666666666666666666666666666666666666666666666666666666666658", 16),
                CurveType::ED25519));

/**
 * P256(Secp256r1)
 *grp
 */
const static Curve P256(
        safeheron::bignum::BN("ffffffff00000001000000000000000000000000ffffffffffffffffffffffff", 16),
        safeheron::bignum::BN("ffffffff00000001000000000000000000000000fffffffffffffffffffffffc", 16),
        safeheron::bignum::BN("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b", 16),
        safeheron::bignum::BN("0", 16), // undefined 'c' for Secp256r1(also named P256)
        safeheron::bignum::BN("0", 16), // undefined 'd' for Secp256r1(also named P256)
        safeheron::bignum::BN("ffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551", 16),
        CurvePoint(
                safeheron::bignum::BN("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296", 16),
                safeheron::bignum::BN("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5", 16),
                CurveType::P256));

/**
 * EC_GROUP for Secp256k1
 * 
 */
static ec_group_st* secp256k1_grp = nullptr;
/**
 * EC_GROUP for P256
 * 
 */
static ec_group_st* p256_grp = nullptr;


Curve::Curve(const safeheron::bignum::BN _p,
          const safeheron::bignum::BN _a,
          const safeheron::bignum::BN _b,
          const safeheron::bignum::BN _c,
          const safeheron::bignum::BN _d,
          const safeheron::bignum::BN _n,
          const CurvePoint _g) : p(_p), a(_a), b(_b), c(_c), d(_d), n(_n), g(_g), grp(nullptr) {
    grp = GetCurveGroup(g.GetCurveType());
}

Curve::~Curve() {
    if (grp) {
        EC_GROUP_free((EC_GROUP*)grp);
        grp = nullptr;
    }
}

const ec_group_st *GetCurveGroup(CurveType c_type) {
    switch (c_type) {
        case curve::CurveType::SECP256K1:
            if (!secp256k1_grp) secp256k1_grp = EC_GROUP_new_by_curve_name(NID_secp256k1);
            return secp256k1_grp;
        case curve::CurveType::P256:
            if (!p256_grp) p256_grp = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
            return p256_grp;
        default:
            return nullptr;
    }
}

const Curve *GetCurveParam(CurveType c_type) {
    switch (c_type) {
        case CurveType::SECP256K1:
            return &Secp256k1;
        case CurveType::P256:
            return &P256;
        case CurveType::ED25519:
            return &Ed25519;
        default:
            return nullptr;
    }
}

}
}
