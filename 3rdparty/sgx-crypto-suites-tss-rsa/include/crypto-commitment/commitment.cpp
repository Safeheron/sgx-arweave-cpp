/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "commitment.h"
#include "crypto-hash/sha256.h"
#include "crypto-bn/rand.h"

using safeheron::bignum::BN;
using safeheron::hash::CSHA256;
using safeheron::curve::CurvePoint;

namespace safeheron{
namespace commitment {

BN CreateComWithBlind(BN &num, BN &blind_factor) {
    uint8_t digest[CSHA256::OUTPUT_SIZE];
    CSHA256 sha256;
    std::string buf;
    num.ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());
    blind_factor.ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());
    sha256.Finalize(digest);
    return BN::FromBytesBE(digest, CSHA256::OUTPUT_SIZE);
}

BN CreateComWithBlind(CurvePoint &point, BN &blind_factor) {
    uint8_t digest[CSHA256::OUTPUT_SIZE];
    CSHA256 sha256;
    std::string buf;
    point.x().ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());
    point.y().ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());

    blind_factor.ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());

    sha256.Finalize(digest);
    return BN::FromBytesBE(digest, CSHA256::OUTPUT_SIZE);
}

BN CreateComWithBlind(std::vector<CurvePoint> &points, BN &blind_factor) {
    uint8_t digest[CSHA256::OUTPUT_SIZE];
    CSHA256 sha256;
    std::string buf;
    for(size_t i = 0; i < points.size(); ++i) {
        points[i].x().ToBytesBE(buf);
        sha256.Write((const uint8_t *) buf.c_str(), buf.length());
        points[i].y().ToBytesBE(buf);
        sha256.Write((const uint8_t *) buf.c_str(), buf.length());
    }

    blind_factor.ToBytesBE(buf);
    sha256.Write((const uint8_t*)buf.c_str(), buf.length());

    sha256.Finalize(digest);
    return BN::FromBytesBE(digest, CSHA256::OUTPUT_SIZE);
}

BN CreateCom(BN &num) {
    size_t byte_len = num.BitLength() / 8;
    if(byte_len == 0) {
        byte_len = 1;
    }else if(byte_len * 8 < num.BitLength()){
        byte_len ++;
    }
    BN blind_factor = safeheron::rand::RandomBN(byte_len);
    return CreateComWithBlind(num, blind_factor);
}

BN CreateCom(CurvePoint &point) {
    BN blind_factor = safeheron::rand::RandomBN(32);
    return CreateComWithBlind(point, blind_factor);
}

BN CreateCom(std::vector<CurvePoint> &points) {
    BN blind_factor = safeheron::rand::RandomBN(32);
    return CreateComWithBlind(points, blind_factor);
}

}
}