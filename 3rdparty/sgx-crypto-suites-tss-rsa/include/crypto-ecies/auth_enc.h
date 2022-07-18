/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CRYPTO_AUTH_ENC_H
#define SAFEHERON_CRYPTO_AUTH_ENC_H

#include "crypto-bn/bn.h"
#include "crypto-curve/curve_point.h"

using namespace safeheron::bignum;
using namespace safeheron::curve;

namespace safeheron {
namespace ecies {

class AuthEnc{
private:
    CurveType curve_type_;
public:
    AuthEnc(){ curve_type_ = CurveType::P256; }
    void set_curve_type(CurveType curve_type);
    bool Encrypt(const BN &local_priv, const CurvePoint &remote_pub, const std::string &in_plain, std::string &out_cypher);
    bool Encrypt(const BN &local_priv, const CurvePoint &remote_pub, const unsigned char *in_plain, size_t in_plain_len, unsigned char **out_cypher, size_t *out_cypher_len);
    bool Decrypt(const BN &local_priv, const CurvePoint &remote_pub, const std::string &in_cypher, std::string &out_plain);
    bool Decrypt(const BN &local_priv, const CurvePoint &remote_pub, const unsigned char *in_cypher, size_t in_cypher_len, unsigned char **out_plain, size_t *out_plain_len);
};

}
}


#endif //SAFEHERON_CRYPTO_AUTH_ENC_H
