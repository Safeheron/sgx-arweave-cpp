/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */


#include "dlog_proof.h"
#include <google/protobuf/util/json_util.h>
#include "crypto-hash/sha256.h"
#include "crypto-bn/rand.h"
#include "zkp.pb.h"
#include "crypto-encode/base64.h"
#include "exception/located_exception.h"

using std::string;
using std::vector;
using safeheron::bignum::BN;
using safeheron::curve::CurvePoint;
using safeheron::hash::CSHA256;
using google::protobuf::util::Status;
using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonParseOptions;
using namespace safeheron::encode;
using namespace safeheron::rand;

namespace safeheron{
namespace zkp {
namespace dlog {

DLogProof::DLogProof(curve::CurveType c_type) {
    curv_ = curve::GetCurveParam(c_type);
}

void DLogProof::InternalProve(const BN &sk, const CurvePoint &g, const BN &order) {
    BN r = RandomBNLt(order);
    InternalProveWithR(sk, g, order, r);
}

void DLogProof::InternalProveWithR(const BN &sk, const CurvePoint &g, const BN &order, const BN &r) {
    g_r_ = g * r;
    pk_ = g * sk;

    // c = H(G || g^r || g^sk || UserID || OtherInfo)
    CSHA256 sha256;
    uint8_t sha256_digest[CSHA256::OUTPUT_SIZE];
    string str;
    g_r_.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g_r_.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    pk_.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    pk_.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    sha256.Finalize(sha256_digest);
    BN c = BN::FromBytesBE(sha256_digest, 32);
    c = c % curv_->n;

    // res = r - sk * c mod n
    BN skc = (sk * c) % order;
    res_ = (r - skc) % order;
}

bool DLogProof::InternalVerify(const CurvePoint &g) const {
    assert(curv_->g == g);
    // c = H(G || g^r || g^sk || UserID || OtherInfo)
    CSHA256 sha256;
    uint8_t sha256_digest[CSHA256::OUTPUT_SIZE];
    string str;
    g_r_.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g_r_.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    g.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    pk_.x().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    pk_.y().ToBytes32BE(str);
    sha256.Write((const uint8_t *)(str.c_str()), str.length());
    sha256.Finalize(sha256_digest);
    BN c = BN::FromBytesBE(sha256_digest, 32);
    c = c % curv_->n;

    // Verify: g^r === g^[r - sk * c mod n] + pk * [c]
    CurvePoint expected_g_r = g * res_ + pk_ * c;
    return expected_g_r == g_r_;
}


void DLogProof::Prove(const BN &sk) {
    assert(curv_ != nullptr);
    BN r = RandomBNLt(curv_->n);
    ProveWithR(sk, r);
}

void DLogProof::ProveWithR(const BN &sk, const BN &r) {
    assert(curv_ != nullptr);
    DLogProof::InternalProveWithR(sk, curv_->g, curv_->n, r);
}

bool DLogProof::Verify() const {
    assert(curv_ != nullptr);
    return DLogProof::InternalVerify(curv_->g);
}

bool DLogProof::ToProtoObject(safeheron::proto::DLogProof &dlog_proof) const {
    bool ok = true;
    safeheron::proto::CurvePoint tmp;

    // g_r
    ok = g_r_.ToProtoObject(tmp);
    if (!ok) return false;
    dlog_proof.mutable_g_r()->CopyFrom(tmp);

    // pk
    ok = pk_.ToProtoObject(tmp);
    if (!ok) return false;
    dlog_proof.mutable_pk()->CopyFrom(tmp);

    // res
    std::string str;
    res_.ToHexStr(str);
    dlog_proof.set_res(str);

    return true;
}

bool DLogProof::FromProtoObject(const safeheron::proto::DLogProof &dlog_proof) {
    bool ok = true;
    safeheron::proto::CurvePoint point;
    // g^r
    point = dlog_proof.g_r();
    ok = g_r_.FromProtoObject(point);
    ok = ok && !g_r_.IsInfinity();
    if (!ok) return false;

    // public key
    point = dlog_proof.pk();
    ok = pk_.FromProtoObject(point);
    ok = ok && !pk_.IsInfinity();
    if (!ok) return false;

    // res
    res_ = BN::FromHexStr(dlog_proof.res());
    ok = (res_ != 0);
    if (!ok) return false;

    // Curve
    curv_ = curve::GetCurveParam(pk_.GetCurveType());
    ok = (curv_ != nullptr);
    if (!ok) return false;

    return true;
}

bool DLogProof::ToBase64(string &b64) const {
    bool ok = true;
    b64.clear();
    safeheron::proto::DLogProof proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    string proto_bin = proto_object.SerializeAsString();
    b64 = base64::EncodeToBase64(proto_bin, true);
    return true;
}

bool DLogProof::FromBase64(const string &b64) {
    bool ok = true;

    string data = base64::DecodeFromBase64(b64);

    safeheron::proto::DLogProof proto_object;
    ok = proto_object.ParseFromString(data);
    if (!ok) return false;

    return FromProtoObject(proto_object);
}

bool DLogProof::ToJsonString(string &json_str) const {
    bool ok = true;
    json_str.clear();
    safeheron::proto::DLogProof proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    JsonPrintOptions jp_option;
    jp_option.add_whitespace = true;
    Status stat = MessageToJsonString(proto_object, &json_str, jp_option);
    if (!stat.ok()) return false;

    return true;
}

bool DLogProof::FromJsonString(const string &json_str) {
    safeheron::proto::DLogProof proto_object;
    google::protobuf::util::JsonParseOptions jp_option;
    jp_option.ignore_unknown_fields = true;
    Status stat = JsonStringToMessage(json_str, &proto_object);
    if (!stat.ok()) return false;

    return FromProtoObject(proto_object);
}

}
}
}
