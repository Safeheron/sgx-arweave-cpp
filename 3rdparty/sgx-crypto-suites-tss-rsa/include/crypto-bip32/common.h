/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CRYPTOBIP32_COMMON_H
#define SAFEHERON_CRYPTOBIP32_COMMON_H

namespace safeheron {
namespace bip32 {

/**
 * Bip32 Version
 */
enum class Bip32Version : uint32_t {
    INVALID_VERSION = 0,
    BITCOIN_VERSION_PRIVATE = 0x0488ADE4,
    BITCOIN_VERSION_PUBLIC = 0x0488B21E,
    EDDSA_VERSIONS_PRIVATE = 0x03126f7c,
    EDDSA_VERSIONS_PUBLIC = 0x031273b7,
};

/**
 * HDNode Struct
 */
typedef struct {
    uint32_t curve_type_;    /**< type of elliptic curve */
    uint32_t depth_;    /**< depth of path */
    uint32_t child_num_;    /**< child index */
    uint8_t chain_code_[32];    /**< chain code */

    uint8_t private_key_[32];    /**< private key */
    uint8_t private_key_extension_[32];    /**< private key extension */

    uint8_t public_key_[33];    /**< public key */
} HDNode;

}
}

#endif //SAFEHERON_CRYPTOBIP32_COMMON_H
