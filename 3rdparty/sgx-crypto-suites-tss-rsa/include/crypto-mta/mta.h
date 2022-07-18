//
// Created by Sword03 on 2021/6/15.
//

#ifndef SAFEHERON_CRYPTO__MTA_H
#define SAFEHERON_CRYPTO__MTA_H

#include <string>
#include "crypto-bn/bn.h"
#include "crypto-curve/curve.h"
#include "crypto-paillier/pail.h"
#include "crypto-zkp/zkp.h"
#include "mta.pb.h"

namespace safeheron{
namespace mta {

struct MessageB {
    safeheron::bignum::BN c_b_;
    safeheron::zkp::dlog::DLogProof dlog_proof_b_;
    safeheron::zkp::dlog::DLogProof dlog_proof_beta_tag_;

    bool ToProtoObject(safeheron::proto::MtaMessageB &mtaMessageB) const;

    bool FromProtoObject(const safeheron::proto::MtaMessageB &mtaMessageB);

    bool ToBase64(std::string &b64) const;

    bool FromBase64(const std::string &b64);

    bool ToJsonString(std::string &json_str) const;

    bool FromJsonString(const std::string &json_str);
};

void construct_message_a(safeheron::bignum::BN &message_a, const pail::PailPubKey &pub, const safeheron::bignum::BN &input_a);

void construct_message_a_with_R(safeheron::bignum::BN &message_a, const pail::PailPubKey &pub, const safeheron::bignum::BN &input_a, const safeheron::bignum::BN &r_lt_pailN);

void construct_message_b(MessageB &message_b, safeheron::bignum::BN &beta, const pail::PailPubKey &pub, const safeheron::bignum::BN &input_b, const safeheron::bignum::BN &message_a);

void construct_message_b_with_R(MessageB &message_b, safeheron::bignum::BN &beta, const pail::PailPubKey &pub, const safeheron::bignum::BN &input_b,
                                const safeheron::bignum::BN &message_a, const safeheron::bignum::BN &r0_lt_pailN, const safeheron::bignum::BN &r1_lt_curveN,
                                const safeheron::bignum::BN &r2_lt_curveN);

bool get_alpha(safeheron::bignum::BN &alpha, const MessageB &message_b, const safeheron::bignum::BN &input_a, const pail::PailPrivKey &priv);

}
}

#endif //SAFEHERON_CRYPTO__MTA_H
