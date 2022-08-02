/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CRYPTO_ZKP_DLN_PROOF_H
#define SAFEHERON_CRYPTO_ZKP_DLN_PROOF_H

#include <string>
#include "crypto-bn/bn.h"
#include "crypto-curve/curve.h"
#include "zkp.pb.h"

namespace safeheron{
namespace zkp {
namespace dln_proof{

class DLNProof {
public:
    std::vector<safeheron::bignum::BN> alpha_arr_;
    std::vector<safeheron::bignum::BN> t_arr_;

    DLNProof(){};

    void Prove(const safeheron::bignum::BN &N, const safeheron::bignum::BN &h1, const safeheron::bignum::BN &h2, const safeheron::bignum::BN &p, const safeheron::bignum::BN &q, const safeheron::bignum::BN &x);
    bool Verify(const safeheron::bignum::BN &N, const safeheron::bignum::BN &h1, const safeheron::bignum::BN &h2) const;

    bool ToProtoObject(safeheron::proto::DLNProof &dln_proof) const;
    bool FromProtoObject(const safeheron::proto::DLNProof &dln_proof);

    bool ToBase64(std::string& base64) const;
    bool FromBase64(const std::string& base64);

    bool ToJsonString(std::string &json_str) const;
    bool FromJsonString(const std::string &json_str);
};

}
}
}
#endif //SAFEHERON_CRYPTO_ZKP_DLN_PROOF_H
