/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */


#include "pail_proof.h"
#include <google/protobuf/util/json_util.h>
#include "crypto-hash/sha256.h"
#include "crypto-bn/rand.h"
#include "crypto-paillier/pail.h"
#include "zkp.pb.h"
#include "crypto-encode/base64.h"
#include "exception/located_exception.h"

using std::string;
using std::vector;
using safeheron::bignum::BN;
using safeheron::curve::CurvePoint;
using safeheron::hash::CSHA256;
using safeheron::pail::PailPubKey;
using safeheron::pail::PailPrivKey;
using google::protobuf::util::Status;
using google::protobuf::util::MessageToJsonString;
using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::JsonParseOptions;
using namespace safeheron::encode;
using namespace safeheron::rand;

#define PRIME_UTIL 6370

namespace safeheron{
namespace zkp {
namespace pail {

static void prime_util(int n, std::vector<int> &prime_arr){
    assert(n > 0);
    prime_arr.clear();
    if(n < 2) return;
    for(int i = 3; i <= n; i++){
        for(int p: prime_arr){
            if(p * p >= i) {
                prime_arr.push_back(i);
                break;
            }
            if(i % p == 0) break;
        }
    }
}

static void uint_to_byte4(uint8_t buf[4], unsigned int ui){
    // Big endian
    buf[3] = ui & 0x000000ff;
    buf[2] = (ui & 0x0000ff00) >> 8;
    buf[1] = (ui & 0x00ff0000) >> 16;
    buf[0] = (ui & 0xff000000) >> 24;

}

void PailProof::GenerateXs(std::vector<BN> &x_arr, const BN &index, const BN &point_x, const BN &point_y, const BN &N, uint32_t proof_iters) const{
    x_arr.clear();
    uint32_t i = 0;
    int n = 0;
    int j = 0;
    // SHA256_DIGEST_LENGTH = 32
    int N_blocks = 1 + N.BitLength() / (CSHA256::OUTPUT_SIZE * 8);
    // uint8_t * blocks_buf = new uint8_t[N_blocks * SHA256_DIGEST_LENGTH];
    std::unique_ptr<uint8_t[]> blocks_buf(new uint8_t[N_blocks * CSHA256::OUTPUT_SIZE]);

    memset(blocks_buf.get(), 0, N_blocks * CSHA256::OUTPUT_SIZE);
    uint8_t byte4[4];
    string index_buf;
    string point_x_buf;
    string point_y_buf;
    string N_buf;
    // index
    index.ToBytesBE(index_buf);
    // point_x
    point_x.ToBytesBE(point_x_buf);
    // point_y
    point_y.ToBytesBE(point_y_buf);
    // N
    N.ToBytesBE(N_buf);

    while( i < proof_iters ){
        for( j = 0; j < N_blocks; ++j ){
            // digest = H(i || j || n || index || point_x || point_y || N)
            CSHA256 sha256;
            uint8_t sha256_digest[CSHA256::OUTPUT_SIZE];
            string str;
            // i
            uint_to_byte4(byte4, i);
            sha256.Write( byte4, 4);
            // j
            uint_to_byte4(byte4, j);
            sha256.Write( byte4, 4);
            // n
            uint_to_byte4(byte4, n);
            sha256.Write( byte4, 4);
            // index
            sha256.Write((const uint8_t *)(str.c_str()), str.length());
            // point_x
            sha256.Write((const uint8_t *)(str.c_str()), str.length());
            // point_y
            sha256.Write((const uint8_t *)(str.c_str()), str.length());
            // N
            sha256.Write((const uint8_t *)(str.c_str()), str.length());
            sha256.Finalize(sha256_digest);
            memcpy(blocks_buf.get() + CSHA256::OUTPUT_SIZE * j, sha256_digest, CSHA256::OUTPUT_SIZE);
        }

        BN x = BN::FromBytesBE(blocks_buf.get(), N_blocks * CSHA256::OUTPUT_SIZE);
        x = x % N;
        // x in Z_N*
        bool ok = (x > BN::ONE) && (x < N) && (x.Gcd(N) == BN::ONE);
        if (ok){
            i ++;
            x_arr.push_back(x);
        }else {
            n ++;
        }
    }

    //delete [] blocks_buf;
}

void PailProof::Prove(const PailPrivKey &pail_priv, const BN &index, const BN &point_x, const BN &point_y, uint32_t proof_iters) {
    vector<BN> x_arr;
    BN M = pail_priv.n().InvM(pail_priv.lambda());
    GenerateXs(x_arr, index, point_x, point_y, pail_priv.n(), proof_iters);
    for(uint32_t i = 0; i < proof_iters; ++i){
        BN y_N = x_arr[i].PowM(M, pail_priv.n());
        y_N_arr_.push_back(y_N);
    }
}

bool PailProof::Verify(const PailPubKey &pail_pub, const BN &index, const BN &point_x, const BN &point_y, uint32_t proof_iters) const {
    if(pail_pub.n().BitLength() < 2047)return false;

    // Check the pail N
    std::vector<int> prime_arr;
    prime_util(PRIME_UTIL, prime_arr);
    for(int p: prime_arr){
        if(pail_pub.n() % p == 0) return false;
    }

    vector<BN> x_arr;
    GenerateXs(x_arr, index, point_x, point_y, pail_pub.n(), proof_iters);
    if (x_arr.size() != proof_iters) return false;
    for (uint32_t i = 0; i < proof_iters; ++i) {
        BN x = y_N_arr_[i].PowM(pail_pub.n(), pail_pub.n());
        if (x != x_arr[i]) {
            return false;
        }
    }
    return true;
}

bool PailProof::ToProtoObject(safeheron::proto::PailProof &pail_proof) const {
    safeheron::proto::CurvePoint tmp;

    for(size_t i = 0; i < y_N_arr_.size(); ++i){
        string str;
        y_N_arr_[i].ToHexStr(str);
        pail_proof.add_y_n_arr(str);
    }

    return true;
}

bool PailProof::FromProtoObject(const safeheron::proto::PailProof &pail_proof) {
    safeheron::proto::CurvePoint point;

    for(int i = 0; i < pail_proof.y_n_arr_size(); ++i){
        BN y_N = BN::FromHexStr(pail_proof.y_n_arr(i));
        y_N_arr_.push_back(y_N);
    }

    return true;
}

bool PailProof::ToBase64(string &b64) const {
    bool ok = true;
    b64.clear();
    safeheron::proto::PailProof proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    string proto_bin = proto_object.SerializeAsString();
    b64 = base64::EncodeToBase64(proto_bin, true);
    return true;
}

bool PailProof::FromBase64(const string &b64) {
    bool ok = true;

    string data = base64::DecodeFromBase64(b64);

    safeheron::proto::PailProof proto_object;
    ok = proto_object.ParseFromString(data);
    if (!ok) return false;

    return FromProtoObject(proto_object);
}

bool PailProof::ToJsonString(string &json_str) const {
    bool ok = true;
    json_str.clear();
    safeheron::proto::PailProof proto_object;
    ok = ToProtoObject(proto_object);
    if (!ok) return false;

    JsonPrintOptions jp_option;
    jp_option.add_whitespace = true;
    Status stat = MessageToJsonString(proto_object, &json_str, jp_option);
    if (!stat.ok()) return false;

    return true;
}

bool PailProof::FromJsonString(const string &json_str) {
    safeheron::proto::PailProof proto_object;
    google::protobuf::util::JsonParseOptions jp_option;
    jp_option.ignore_unknown_fields = true;
    Status stat = JsonStringToMessage(json_str, &proto_object);
    if (!stat.ok()) return false;

    return FromProtoObject(proto_object);
}

}
}
}
