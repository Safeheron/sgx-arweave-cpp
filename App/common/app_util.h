#ifndef TEE_ARWEAVE_SERVER_APP_UTIL_H
#define TEE_ARWEAVE_SERVER_APP_UTIL_H

#include <iostream>
#include <safeheron/crypto-hash/sha256.h>
#include <safeheron/crypto-bn/bn.h>
#include <safeheron/crypto-bn/rand.h>



bool get_sha256_hash(const std::string& input, std::string& out_hash_hex);
std::string bytes2hex(const uint8_t * input, size_t input_len);



#endif //TEE_ARWEAVE_SERVER_APP_UTIL_H
