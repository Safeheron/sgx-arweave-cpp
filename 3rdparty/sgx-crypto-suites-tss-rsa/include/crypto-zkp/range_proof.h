/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CRYPTO_ZKP_RANGE_PROOF_H
#define SAFEHERON_CRYPTO_ZKP_RANGE_PROOF_H

#include <string>
#include "crypto-bn/bn.h"
#include "crypto-curve/curve.h"
#include "zkp.pb.h"

namespace safeheron{
namespace zkp {
namespace range_proof{

/**
 * @brief This protocol is based on the NIZK protocol in Paper "Fast multiparty threshold ECDSA with fast trustless setup".
 *
 * See "A.1 Range Proof" for full details.
 */

class AliceRangeProof {
public:
    safeheron::bignum::BN z_;
    safeheron::bignum::BN u_;
    safeheron::bignum::BN w_;
    safeheron::bignum::BN s_;
    safeheron::bignum::BN s1_;
    safeheron::bignum::BN s2_;

    AliceRangeProof(){};

    void Prove(const safeheron::bignum::BN &q, const safeheron::bignum::BN &N, const safeheron::bignum::BN &g, const safeheron::bignum::BN &N_tilde, const safeheron::bignum::BN &h1, const safeheron::bignum::BN &h2, const safeheron::bignum::BN &c, const safeheron::bignum::BN &m, const safeheron::bignum::BN &r);
    bool Verify(const safeheron::bignum::BN &q, const safeheron::bignum::BN &N, const safeheron::bignum::BN &g, const safeheron::bignum::BN &N_tilde, const safeheron::bignum::BN &h1, const safeheron::bignum::BN &h2, const safeheron::bignum::BN &c) const;

    bool ToProtoObject(safeheron::proto::AliceRangeProof &pail_proof) const;
    bool FromProtoObject(const safeheron::proto::AliceRangeProof &pail_proof);

    bool ToBase64(std::string& base64) const;
    bool FromBase64(const std::string& base64);

    bool ToJsonString(std::string &json_str) const;
    bool FromJsonString(const std::string &json_str);
};

}
}
}
#endif //SAFEHERON_CRYPTO_ZKP_RANGE_PROOF_H
