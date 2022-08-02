/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "rand.h"
#include "../exception/safeheron_exceptions.h"
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <memory>

using safeheron::bignum::BN;
using safeheron::exception::LocatedException;
using safeheron::exception::OpensslException;
using safeheron::exception::BadAllocException;
using safeheron::exception::RandomSourceException;


namespace safeheron{
namespace rand{

void RandomBytes(unsigned char *buf, size_t size) {
    int ret = 0;
    if (!buf) {
        throw RandomSourceException(__FILE__, __LINE__, __FUNCTION__, -1, "!buf");
    }
    if ((ret = RAND_bytes(buf, size)) <= 0) {
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, ret, "(ret = RAND_bytes(buf, size)) <= 0");
    }
}

BN RandomBN(size_t byteSize) {
    BN n;
    std::unique_ptr<unsigned char[]> buf(new(std::nothrow) unsigned char[byteSize]);
    if (buf.get() == nullptr) throw BadAllocException(__FILE__, __LINE__, __FUNCTION__, byteSize, "buf.get() == nullptr");
    do{
        RandomBytes(buf.get(), byteSize);
        n = BN::FromBytesBE(buf.get(), byteSize);
    }while(n == 0);
    return n;
}

BN RandomBNStrict(size_t byteSize) {
    std::unique_ptr<unsigned char[]> buf(new(std::nothrow) unsigned char[byteSize]);
    if (buf == nullptr) throw BadAllocException(__FILE__, __LINE__, __FUNCTION__, byteSize, "buf == nullptr");
    do {
        RandomBytes(buf.get(), byteSize);
    }while((buf[0] & 0x80) == 0);
    BN n = BN::FromBytesBE(buf.get(), byteSize);
    return n;
}

BN RandomPrime(size_t byteSize) {
    BN n;
    BIGNUM* p = nullptr;
    int ret = 0;
    if (!(p = BN_new())) {
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, 0, "!(p = BN_new())");
    }
    if ((ret = BN_generate_prime_ex(p, byteSize * 8, 0, nullptr, nullptr, nullptr)) != 1) {
        BN_clear_free(p);
        p = nullptr;
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, ret, "(ret = BN_generate_prime_ex(p, byteSize * 8, 0, nullptr, nullptr, nullptr)) != 1");
    }
    n.Hold(p);
    return n;
}

BN RandomPrimeStrict(size_t byteSize) {
    BN n;
    do {
        n = RandomPrime(byteSize);
    }while (!n.IsBitSet(8*byteSize-1));
    return n;
}

BN RandomSafePrime(size_t byteSize) {
    BN n;
    BIGNUM* p = nullptr;
    int ret = 0;
    if (!(p = BN_new())) {
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, 0, "!(p = BN_new())");
    }
    if ((ret = BN_generate_prime_ex(p, byteSize * 8, 1, nullptr, nullptr, nullptr)) != 1) {
        BN_clear_free(p);
        p = nullptr;
        throw OpensslException(__FILE__, __LINE__, __FUNCTION__, ret, "(ret = BN_generate_prime_ex(p, byteSize * 8, 1, nullptr, nullptr, nullptr)) != 1");
    }
    n.Hold(p);
    return n;
}

BN RandomSafePrimeStrict(size_t byteSize) {
    BN n;
    do {
        n = RandomSafePrime(byteSize);
    }while (!n.IsBitSet(8*byteSize-1));
    return n;
}

BN RandomBNLt(const BN &max) {
    BN n;
    int byteLen = max.ByteLength();
    do{
        n = RandomBN(byteLen);
    }while (n >= max);
    return n;
}

BN RandomBNLtGcd(const BN &max) {
    BN n;
    do{
        n = RandomBNLt(max);
    }while (n.Gcd(max) != 1);
    return n;
}

BN RandomBNLtCoPrime(const BN &max) {
    BN n;
    do{
        n = RandomBNLt(max);
    }while (n.Gcd(max) != 1);
    return n;
}
}
}
