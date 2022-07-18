//
// Created by YYF on 7-12-21.
//

#ifndef _TOOLS_H_
#define _TOOLS_H_
#include <string>

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

#endif //_TOOLS_H_
