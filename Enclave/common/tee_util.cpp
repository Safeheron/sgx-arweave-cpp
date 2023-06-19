/**
 * @file tee_util.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "tee_util.h"
#include "Enclave_t.h"
#include <crypto-hash/sha256.h>
#include <crypto-encode/hex.h>
#include <sgx_utils.h>
#include <sgx_lfence.h>
#include <sgx_trts.h>


/**
 * Output message to console by calling ocall_printf()
*/
int printf( const char* fmt, ... )
{
    char buf[MAX_BUF_LEN] = { '\0' };
    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, MAX_BUF_LEN, fmt, ap );
    va_end( ap );
    ocall_printf( buf );
    return (int)strnlen( buf, MAX_BUF_LEN - 1 ) + 1;
}

/**
 * Format a string
 */
std::string format_msg( const char* fmt, ... )
{
    char buf[MAX_BUF_LEN] = { 0 };
    std::string ret;

    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, MAX_BUF_LEN - 1, fmt, ap );
    va_end(ap);
    ret = buf;

    return ret;
}

/**
 * Get current system time (in seconds) by OCALL
 * the return type is int64_t, it's same as time_t.
*/
int64_t get_system_time()
{
    int ret = 0;
    int64_t* time_buff = nullptr;
    int64_t t_now = 0;

    // get current time
    ret = ocall_get_system_time( &time_buff );
    if ( SGX_SUCCESS != ret ) {
        ocall_free( (uint8_t*)time_buff );
        return 0;
    }
    t_now = *time_buff;
    ocall_free( (uint8_t*)time_buff );

    return t_now;
}

/**
 * Call OCAL to malloc a buffer with size outside of TEE
 * The malloced buffer will be set 0, and MUST be freed by code
 * outside of TEE at last.
 */
uint8_t* malloc_outside( size_t size )
{
    uint8_t* outside_buf = nullptr;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;

    if ( size <= 0 ) {
        return nullptr;
    }

    // malloc outside buff
    status = ocall_malloc(size, &outside_buf);
    if (SGX_SUCCESS != status) {
        return nullptr;
    }
    if (!outside_buf) {
        return nullptr;
    }

    //check buff is outside or not
    if ( sgx_is_outside_enclave(outside_buf, size) != 1 ) {
        ocall_free(outside_buf);
        outside_buf = nullptr;
        return nullptr;
    }
    memset(outside_buf, 0, size);

    sgx_lfence();

    return outside_buf;
}

/**
 * Do SHA256 for input, and output the result in hex string
 */
bool sha256_hash( const std::string& input, std::string& out_hex )
{
    safeheron::hash::CSHA256 sha256;
    uint8_t digest[safeheron::hash::CSHA256::OUTPUT_SIZE] = { 0 };

    sha256.Write( (const uint8_t *)input.c_str(), input.size() );
    sha256.Finalize( digest );

    out_hex = safeheron::encode::hex::EncodeToHex( digest, safeheron::hash::CSHA256::OUTPUT_SIZE );
    return out_hex.length() == 0 ? false : true;
}