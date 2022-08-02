#ifndef TEE_ARWEAVE_SERVER_ENCLAVE_UTIL_H
#define TEE_ARWEAVE_SERVER_ENCLAVE_UTIL_H
#include "libcxx/memory"
#include <string>
#include <crypto-hash/sha256.h>
#include <stdio.h>
#include "Enclave_t.h"
#include <sgx_lfence.h>
#include <sgx_trts.h>

#define SAF_RELEASE(x) {if (x) {free(x); x=nullptr;} }

/**
 * output trace message in console
*/
#define TRACE_ENCLAVE(fmt, ...)    \
                                   \
    printf(                        \
        "%s ***%s(%d): " fmt "\n", \
        __FILE__,                  \
        __LINE__,                  \
        ##__VA_ARGS__)

/**
 * output message to console
*/
extern "C" {
int printf( const char* fmt, ... );
}


/**
 * output buffer in HEX string to console
*/
int printf_hex( uint8_t * buf, int len );

/**
 * output buffer to a HEX string
*/
const char* s_printf_hex( uint8_t * buf, int len );

/**
 * get current system time (in seconds) by OCALL
 * the return type is int64_t, it's same as time_t.
*/
int64_t get_system_time();



std::string bytes2hex(const uint8_t * input, size_t input_len);

/**
 * Hash the input.
 * @param [in]input the message needed to be hashed
 * @param [out]out_hash_hex hash output
 * @return true on success, false on failure
 */
bool get_sha256_hash(const std::string& input, std::string& out_hash_hex);

uint8_t* malloc_outside(size_t size);

#endif //TEE_ARWEAVE_SERVER_APP_UTIL_H
