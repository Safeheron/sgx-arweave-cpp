/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */

#ifndef SAFEHERON_CRYPTO_ZKP_PAIL_PROOF_H
#define SAFEHERON_CRYPTO_ZKP_PAIL_PROOF_H

#include <string>
#include "crypto-bn/bn.h"
#include "crypto-curve/curve.h"
#include "zkp.pb.h"
#include "crypto-paillier/pail.h"

namespace safeheron{
namespace zkp {
namespace pail {

/**
 * @brief This protocol is based on the NIZK protocol in https://eprint.iacr.org/2018/057.pdf
 *
 * For parameters iteration = 11, alpha = 6370, see https://eprint.iacr.org/2018/987.pdf 6.2.3 for full details.
 */

class PailProof {
private:
    void GenerateXs(std::vector<safeheron::bignum::BN> &x_arr, const safeheron::bignum::BN &index, const safeheron::bignum::BN &point_x, const safeheron::bignum::BN &point_y, const safeheron::bignum::BN &N, uint32_t proof_iters = 11) const;
public:
    // List of y^N mod N
    std::vector<safeheron::bignum::BN> y_N_arr_;

    PailProof(){};

    void Prove(const safeheron::pail::PailPrivKey &pail_priv, const safeheron::bignum::BN &index, const safeheron::bignum::BN &point_x, const safeheron::bignum::BN &point_y, uint32_t proof_iters = 11);
    bool Verify(const safeheron::pail::PailPubKey &pail_pub, const safeheron::bignum::BN &index, const safeheron::bignum::BN &point_x, const safeheron::bignum::BN &point_y, uint32_t proof_iters = 11) const;

    bool ToProtoObject(safeheron::proto::PailProof &pail_proof) const;
    bool FromProtoObject(const safeheron::proto::PailProof &pail_proof);

    bool ToBase64(std::string& base64) const;
    bool FromBase64(const std::string& base64);

    bool ToJsonString(std::string &json_str) const;
    bool FromJsonString(const std::string &json_str);
};

}
}
}
#endif //SAFEHERON_CRYPTO_ZKP_PAIL_PROOF_H
