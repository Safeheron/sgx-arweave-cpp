/**
 * @file tee_util.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _TEE_UTIL_H_
#define _TEE_UTIL_H_

#include <stdio.h>
#include <string>

#define MAX_BUF_LEN  4*1024

/**
 * Output message to console by calling ocall_printf()
 * The function MUST be implemented for protobuf.a
*/
#ifdef __cplusplus
extern "C" {
#endif
    int printf( const char* fmt, ... );
#ifdef __cplusplus
}
#endif

/**
 * Format a string
 */
std::string format_msg( const char* fmt, ... );

/**
 * Get current system time (in seconds) by OCALL
 * the return type is int64_t, it's same as time_t.
*/
int64_t get_system_time();

/**
 * Call OCALL to malloc a buffer with size outside of TEE
 * The malloced buffer will be set 0, and MUST be freed by code
 * outside of TEE at last.
 */
uint8_t* malloc_outside( size_t size );

/**
 * Do SHA256 for input, and output the result in hex string
 */
bool sha256_hash( const std::string& input, std::string& out_hex );


#endif //_TEE_UTIL_H_
