/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */


#include "pail_pubkey.h"
#include <google/protobuf/util/json_util.h>
#include "crypto-bn/rand.h"
#include "crypto-encode/base64.h"

using std::string;
using safeheron::bignum::BN;
using google::protobuf::util::Status;
using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonParseOptions;

namespace safeheron{
namespace pail {
/**
 * Constructor of PailPubKey
 * @param n = pq
 * @param g = n + 1
 * @constructor
 */
PailPubKey::PailPubKey(const BN &n, const BN &g) {
    n_ = n;
    g_ = g;
    n_sqr_ = n * n;
}

PailPubKey::PailPubKey() {
    n_ = BN();
    g_ = BN();
    n_sqr_ = BN();
}

std::string PailPubKey::Inspect() const {
    std::string str;
    str.append("Paillier's public key: ");
    str.append("\n  -  n: ");
    str.append(n_.Inspect());
    str.append("\n  -  g: ");
    str.append(g_.Inspect());
    str.append("\n");
    return str;
}

/**
 * Encrypt:
 *     c = g^m * r^n mod n^2
 *
 * @param {BN} m: number to be encrypted
 * @param {BN} r : random number
 */
BN PailPubKey::EncryptWithR(const BN &m, const BN &r) const {
    BN gm = (m * n_ + 1) % n_sqr_;
    BN rn = r.PowM(n_, n_sqr_);
    return (gm * rn) % n_sqr_;
}

BN PailPubKey::EncryptWithR_v0(const BN &m, const BN &r) const {
    BN gm = g_.PowM(m, n_sqr_);
    BN rn = r.PowM(n_, n_sqr_);
    return (gm * rn) % n_sqr_;
}

/**
 * Encrypt:
 *     c = g^m * r^n mod n^2
 *       = (m * n + 1) mod n^2 * r^n mod n^2
 *
 * @param {BN} m: number to be encrypted
 */
BN PailPubKey::Encrypt(const BN &m) const {
    BN r = safeheron::rand::RandomBNLtGcd(n_);
    BN gm = (m * n_ + 1) % n_sqr_;
    BN rn = r.PowM(n_, n_sqr_);
    return (gm * rn) % n_sqr_;
}

BN PailPubKey::Encrypt_v0(const BN &m) const {
    BN r = safeheron::rand::RandomBNLtGcd(n_);
    BN gm = g_.PowM(m, n_sqr_);
    BN rn = r.PowM(n_, n_sqr_);
    return (gm * rn) % n_sqr_;
}


/**
 * Homomorphic add:
 *     E(a+b) = E(a) * E(b) mod n^2
 * @param {BN} e_a: encrypted num a
 * @param {BN} e_b: encrypted num b
 */
BN PailPubKey::Add(const BN &e_a, const BN &e_b) const {
    return (e_a * e_b) % n_sqr_;
}

/**
 * Homomorphic add:
 *     E(a+b) = E(a) * g^b mod n^2
 * @param {BN} e_a: encrypted num a
 * @param {BN} b: plain num b
 */
BN PailPubKey::AddPlain(const BN &e_a, const BN &b) const {
    BN gb = (b * n_ + 1) % n_sqr_;
    return (e_a * gb) % n_sqr_;
}

BN PailPubKey::AddPlain_v0(const BN &e_a, const BN &b) const {
    BN gb = g_.PowM(b, n_sqr_);
    return (e_a * gb) % n_sqr_;
}

/**
 * Homomorphic multiple:
 *     E(ka) = E(a) ^ k mod n^2
 * @param {BN} e_a: encrypted num a
 * @param {BN} k: plain num to multiple
 */
BN PailPubKey::Mul(const BN &e_a, const BN &k) const {
    return e_a.PowM(k, n_sqr_);
}

bool PailPubKey::ToProtoObject(safeheron::proto::PailPub &pail_pub) const {
    string str;
    n_.ToHexStr(str);
    pail_pub.set_n(str);
    g_.ToHexStr(str);
    pail_pub.set_g(str);
    return true;
}

bool PailPubKey::FromProtoObject(const safeheron::proto::PailPub &pail_pub) {
    bool ok = true;
    n_ = BN::FromHexStr(pail_pub.n());
    ok = (n_ != 0);
    if (!ok) return false;

    g_ = BN::FromHexStr(pail_pub.g());
    ok = (g_ != 0);
    if (!ok) return false;

    n_sqr_ = n_ * n_;
    return true;
}

bool PailPubKey::ToBase64(string &base64) const {
    base64.clear();
    safeheron::proto::PailPub proto_object;
    ToProtoObject(proto_object);
    string proto_bin = proto_object.SerializeAsString();
    base64 = safeheron::encode::base64::EncodeToBase64(proto_bin, true);
    return true;
}

bool PailPubKey::FromBase64(const string &base64) {
    bool ok = true;

    string data = safeheron::encode::base64::DecodeFromBase64(base64);

    safeheron::proto::PailPub proto_object;
    ok = proto_object.ParseFromString(data);
    if (!ok) return false;

    return FromProtoObject(proto_object);
}

bool PailPubKey::ToJsonString(string &json_str) const {
    bool ok = true;
    json_str.clear();
    safeheron::proto::PailPub proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    JsonPrintOptions jp_option;
    jp_option.add_whitespace = true;
    Status stat = MessageToJsonString(proto_object, &json_str, jp_option);
    if (!stat.ok()) return false;

    return true;
}

bool PailPubKey::FromJsonString(const string &json_str) {
    safeheron::proto::PailPub proto_object;
    JsonParseOptions jp_option;
    jp_option.ignore_unknown_fields = true;
    Status stat = JsonStringToMessage(json_str, &proto_object);
    if (!stat.ok()) return false;

    return FromProtoObject(proto_object);
}

};
};
