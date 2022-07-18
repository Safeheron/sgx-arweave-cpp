//
// Created by EDY on 2022/6/22.
//

#include "app_util.h"
#include <string>
#include <safeheron/crypto-hash/sha256.h>
#include <stdio.h>
#include "../controller/data_verify.h"

using safeheron::bignum::BN;


bool get_sha256_hash(const std::string& input, std::string& out_hash_hex){
    safeheron::hash::CSHA256 pub_hash_sha256;
    uint8_t digest[safeheron::hash::CSHA256::OUTPUT_SIZE];

    /** Input the size and the message of the hash */
    pub_hash_sha256.Write((const uint8_t *)input.c_str(), input.size());

    /** Output the hash */
    pub_hash_sha256.Finalize(digest);

    /** Convert bytes to HEX */
    out_hash_hex = bytes2hex(digest, safeheron::hash::CSHA256::OUTPUT_SIZE);

    return true;
}

std::string bytes2hex(const uint8_t * input, size_t input_len){
    static const char *sha2_hex_digits = "0123456789abcdef";
    std::unique_ptr<char[]> output_hex(new char [input_len * 2 + 1]);
    const uint8_t *d = input;
    for (size_t i = 0; i < input_len; i++) {
        output_hex[i * 2] = sha2_hex_digits[(*d & 0xf0) >> 4];
        output_hex[i * 2 + 1] = sha2_hex_digits[*d & 0x0f];
        d++;
    }
    output_hex[input_len * 2] = (char)0;
    std::string ret;
    ret.assign(output_hex.get());
    return ret;
}
