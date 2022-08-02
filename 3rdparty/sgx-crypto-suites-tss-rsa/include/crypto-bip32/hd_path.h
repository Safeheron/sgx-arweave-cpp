/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

/**
 * To do:
 * - Check if input "keypath_str" in function ParseHDPath is valid.
 */

#ifndef SAFEHERON_CRYPTOBIP32_HDPATH_H
#define SAFEHERON_CRYPTOBIP32_HDPATH_H

#include <string>
#include <vector>

namespace safeheron {
namespace bip32 {

class HDPath {
public:
    /**
     * Parse an HD keypaths like "m/7/0'/2000".
     * @param[in] keypath_str like "m/7/0'/2000"
     * @param[out] keypath array of index
     * @return true on success, false on error.
     */
    static bool ParseHDPath(const std::string &keypath_str, std::vector<uint32_t> &keypath);

    /**
     * Write HD keypaths as strings
     * @param[in] keypath array of index
     * @return path like "/7/0'/2000"
     */
    static std::string WriteHDPath(const std::vector<uint32_t> &keypath);

    /**
     * Format a HD path
     * @param[in] path
     * @return path like "m/7/0'/2000"
     */
    static std::string FormatHDPath(const std::vector<uint32_t> &path);
};

}
}

#endif //SAFEHERON_CRYPTOBIP32_HDPATH_H
