//
// Created by Sword03 on 2021/6/15.
//


#include "mta.h"
#include <google/protobuf/util/json_util.h>
#include "crypto-hash/sha256.h"
#include "crypto-bn/rand.h"
#include "crypto-encode/base64.h"
#include "exception/located_exception.h"

using std::string;
using std::vector;
using google::protobuf::util::Status;
using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonParseOptions;
using safeheron::bignum::BN;
using safeheron::curve::CurvePoint;
using safeheron::zkp::dlog::DLogProof;
using namespace safeheron::rand;
using namespace safeheron::encode;

namespace safeheron{
namespace mta{



void construct_message_a(BN &message_a, const pail::PailPubKey &pail_pub, const BN &input_a) {
    BN r_lt_pailN = RandomBNLtGcd(pail_pub.n());
    construct_message_a_with_R(message_a, pail_pub, input_a, r_lt_pailN);
}

void construct_message_a_with_R(BN &message_a, const pail::PailPubKey &pail_pub, const BN &input_a, const BN &r_lt_pailN) {
    message_a = pail_pub.EncryptWithR(input_a, r_lt_pailN);
}

void construct_message_b(MessageB &message_b, BN &beta, const pail::PailPubKey &pail_pub, const BN &input_b, const BN &message_a) {
    const curve::Curve * curv = curve::GetCurveParam(curve::CurveType::SECP256K1);
    BN r0_lt_pailN = RandomBNLt(pail_pub.n());
    BN r1_lt_curveN = RandomBNLt(curv->n);
    BN r2_lt_curveN = RandomBNLt(curv->n);
    construct_message_b_with_R(message_b, beta, pail_pub, input_b, message_a, r0_lt_pailN, r1_lt_curveN, r2_lt_curveN);
}

void construct_message_b_with_R(MessageB &message_b, BN &beta, const pail::PailPubKey &pail_pub, const BN &input_b, const BN &message_a, const BN &r0_lt_pailN,
                                const BN &r1_lt_curveN, const BN &r2_lt_curveN) {
    const curve::Curve * curv = curve::GetCurveParam(curve::CurveType::SECP256K1);
    const BN& c_a = message_a;
    const BN& beta_tag = r0_lt_pailN;
    string str;
    //beta_tag.ToHexStr(str);
    //std::cout << "beta_tag: " << str << std::endl;
    BN bma = pail_pub.Mul(c_a, input_b);
    BN c_b = pail_pub.AddPlain(bma, beta_tag);

    DLogProof dlog_proof(curve::CurveType::SECP256K1);
    dlog_proof.ProveWithR(input_b, r1_lt_curveN);
    message_b.dlog_proof_b_ = dlog_proof;
    dlog_proof.ProveWithR(beta_tag, r2_lt_curveN);
    message_b.dlog_proof_beta_tag_ = dlog_proof;
    message_b.c_b_ = c_b;
    //beta_tag.ToHexStr(str);
    //std::cout << "beta_tag: " << str << std::endl;
    //curv->n.ToHexStr(str);
    //std::cout << "curve.n: " << str << std::endl;
    beta = beta_tag.Neg() % curv->n;
}

bool get_alpha(BN &alpha, const MessageB &message_b, const BN &input_a, const pail::PailPrivKey &pail_priv) {
    if(!(message_b.dlog_proof_b_.Verify() && message_b.dlog_proof_beta_tag_.Verify())){
        return false;
    }
    const curve::Curve * curv = curve::GetCurveParam(curve::CurveType::SECP256K1);
    alpha = pail_priv.Decrypt(message_b.c_b_);
    alpha = alpha % curv->n;
    curve::CurvePoint left = curv->g * alpha;
    curve::CurvePoint right = message_b.dlog_proof_b_.pk_ * input_a + message_b.dlog_proof_beta_tag_.pk_;
    if( left != right ){
        return false;
    }
    return true;
}

bool MessageB::ToProtoObject(safeheron::proto::MtaMessageB &mtaMessageB) const {
    bool ok = true;
    safeheron::proto::DLogProof tmp;
    std::string str;

    // c_b_
    ok = (c_b_ != 0);
    if (!ok) return false;
    c_b_.ToHexStr(str);
    mtaMessageB.set_c_b(str);

    // dlog_proof_b_
    ok = dlog_proof_b_.ToProtoObject(tmp);
    if (!ok) return false;
    mtaMessageB.mutable_dlog_proof_b()->CopyFrom(tmp);

    // dlog_proof_beta_tag_
    ok = dlog_proof_beta_tag_.ToProtoObject(tmp);
    if (!ok) return false;
    mtaMessageB.mutable_dlog_proof_beta_tag()->CopyFrom(tmp);

    return true;
}

bool MessageB::FromProtoObject(const safeheron::proto::MtaMessageB &mtaMessageB) {
    bool ok = true;
    // c_b
    c_b_ = BN::FromHexStr(mtaMessageB.c_b());

    // dlog_proof_b
    ok = dlog_proof_b_.FromProtoObject(mtaMessageB.dlog_proof_b());
    if (!ok) return false;

    // dlog_proof_beta_tag
    ok = dlog_proof_beta_tag_.FromProtoObject(mtaMessageB.dlog_proof_beta_tag());
    if (!ok) return false;

    return true;
}

bool MessageB::ToBase64(string &b64) const {
    bool ok = true;
    b64.clear();
    safeheron::proto::MtaMessageB proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    string proto_bin = proto_object.SerializeAsString();
    b64 = base64::EncodeToBase64(proto_bin, true);
    return true;
}

bool MessageB::FromBase64(const string &b64) {
    bool ok = true;

    string data = base64::DecodeFromBase64(b64);

    safeheron::proto::MtaMessageB proto_object;
    ok = proto_object.ParseFromString(data);
    if (!ok) return false;

    return FromProtoObject(proto_object);
}

bool MessageB::ToJsonString(string &json_str) const {
    bool ok = true;
    json_str.clear();
    safeheron::proto::MtaMessageB proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    JsonPrintOptions jp_option;
    jp_option.add_whitespace = true;
    Status stat = MessageToJsonString(proto_object, &json_str, jp_option);
    if (!stat.ok()) return false;

    return true;
}


bool MessageB::FromJsonString(const string &json_str) {
    safeheron::proto::MtaMessageB proto_object;
    google::protobuf::util::JsonParseOptions jp_option;
    jp_option.ignore_unknown_fields = true;
    Status stat = JsonStringToMessage(json_str, &proto_object);
    if (!stat.ok()) return false;

    return FromProtoObject(proto_object);
}

}
}
