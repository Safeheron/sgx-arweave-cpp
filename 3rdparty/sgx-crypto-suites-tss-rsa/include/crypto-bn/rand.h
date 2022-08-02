/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_RANDOM_H
#define SAFEHERON_RANDOM_H

#include "bn.h"

namespace safeheron {
namespace rand {

/**
 * Sample random bytes.
 * @param buf
 * @param size
 */
void RandomBytes(unsigned char * buf, size_t size);

/**
 * Sample random BN.
 * @param byteSize length by byte.
 * @return a random BN.
 */
safeheron::bignum::BN RandomBN(size_t byteSize);

/**
 * Sample random BN whose highest bit is 1.
 * @param byteSize length by byte.
 * @return a random BN.
 */
safeheron::bignum::BN RandomBNStrict(size_t byteSize);

/**
 * Sample random prime.
 * @param byteSize length by byte.
 * @return a random BN.
 */
safeheron::bignum::BN RandomPrime(size_t byteSize);

/**
 * Sample random prime whose highest bit is 1.
 * @param byteSize length by byte.
 * @return a random prime.
 */
safeheron::bignum::BN RandomPrimeStrict(size_t byteSize);

/**
 * Sample random safe prime.
 * @param byteSize length by byte.
 * @return a random safe prime.
 */
safeheron::bignum::BN RandomSafePrime(size_t byteSize);

/**
 * Sample random safe prime whose highest bit is 1.
 * @param byteSize length by byte.
 * @return a random safe prime.
 */
safeheron::bignum::BN RandomSafePrimeStrict(size_t byteSize);

/**
 * Sample random BN which is less than "max".
 * @param max
 * @return a random BN.
 */
safeheron::bignum::BN RandomBNLt(const safeheron::bignum::BN &max);

/**
 * Sample random BN which is less than and co-prime to "max"
 * @deprecated Use "RandomBNLtCoPrime" instead.
 * @param max
 * @return a random BN
 */
safeheron::bignum::BN RandomBNLtGcd(const safeheron::bignum::BN &max);

/**
 * Sample random BN which is less than and co-prime to "max"
 * @param max
 * @return a random BN
 */
safeheron::bignum::BN RandomBNLtCoPrime(const safeheron::bignum::BN &max);

};
};


#endif //SAFEHERON_RANDOM_H