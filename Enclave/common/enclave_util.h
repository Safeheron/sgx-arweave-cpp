//
// Created by edy on 2022/6/13.
//

#ifndef TEE_ARWEAVE_SERVER_ENCLAVE_UTIL_H
#define TEE_ARWEAVE_SERVER_ENCLAVE_UTIL_H
#include "libcxx/memory"
#include <string>
#include <crypto-hash/sha256.h>
#include <stdio.h>
#include "Enclave_t.h"
#include <sgx_lfence.h>
#include <sgx_trts.h>

std::string bytes2hex(const uint8_t * input, size_t input_len);

/**
 * Hash the input.
 * @param [in]input the message needed to be hashed
 * @param [out]out_hash_hex hash output
 * @return true on success, false on failure
 */
bool get_sha256_hash(const std::string& input, std::string& out_hash_hex);

int printf_in_trust(const char* fmt, ...);

uint8_t* malloc_outside(size_t size);

#endif //TEE_ARWEAVE_SERVER_APP_UTIL_H
