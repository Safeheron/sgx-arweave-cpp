/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CURVE_TYPE_H
#define SAFEHERON_CURVE_TYPE_H

#include <cstdlib>

namespace safeheron{
namespace curve{

/**
 * Curve type
 * - 0, invalid
 * - 1 ~ 2^5-1, short curve
 * - 2^5 ~ 2^6-1, edwards curve
 * - 2^6 ~ 2^6+2^5-1, montgomery curve
 */
enum class CurveType: uint32_t {
    INVALID_CURVE = 0xFFFFFFFF, /**< Invalid Curve. */
    SECP256K1 = 1, /**< Curve Secp256k1 */
    P256 = 2, /**< Curve Secp256r1 */
    ED25519 = 32, /**< Curve Ed25519 */
};

};
};
#endif //SAFEHERON_CURVE_TYPE_H
