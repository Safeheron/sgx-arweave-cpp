/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#include "hmac.h"
#include <openssl/hmac.h>

namespace safeheron {
namespace ecies {

bool IHMAC::calcMAC(const unsigned char *key,
                    size_t key_size,
                    const unsigned char *input,
                    size_t in_size,
                    std::string &out) {
    bool ret = false;
    unsigned int mdlen = 0;
    unsigned char md[EVP_MAX_MD_SIZE] = {0};
    HMAC_CTX *ctx = nullptr;

    if (!key || key_size <= 0) {
        return false;
    }
    if (!input || in_size <= 0) {
        return false;
    }

    if (!(ctx = HMAC_CTX_new())) {
        return false;
    }

    if (!HMAC_Init_ex(ctx, key, key_size, md_, nullptr))
        goto err;
    if (!HMAC_Update(ctx, input, in_size))
        goto err;
    if (!HMAC_Final(ctx, md, &mdlen))
        goto err;

    out.assign((char *) md, mdlen);
    ret = true;
    err:
    if (ctx) {
        HMAC_CTX_free(ctx);
        ctx = nullptr;
    }
    return ret;
}

bool IHMAC::calcMAC(const std::string &key,
                    const std::string &input,
                    std::string &out) {
    return calcMAC((const unsigned char *) key.c_str(), key.length(),
                   (const unsigned char *) input.c_str(), input.length(), out);
}

}
}
