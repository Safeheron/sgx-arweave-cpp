/**
 * @file ocall.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Enclave_u.h"
#include "../common/log_u.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// defined in log_u.cpp
extern const char* g_log_project_name;
extern __int64_t time_usec();

#ifdef __cplusplus
extern "C"{
#endif

/*
*  Output message to console in enclave
*/
void ocall_printf( const char *str )
{
    fprintf( stdout, "%s", str );
}

/**
 * get current system time (time_t) from application
*/
void ocall_get_system_time( int64_t** now )
{
    int64_t* t_now = nullptr;

    t_now = (int64_t*)malloc( sizeof( int64_t ) );
    if ( !t_now ) {
        fprintf( stderr, "[%s][%d]maolloc() failed with size:%d\n ", __FILE__, __LINE__, (int)sizeof( int64_t ) );
        return;
    }
    memset( t_now, 0, sizeof(int64_t) );
    //
    time_t t = time( nullptr );
    *t_now = (int64_t)t;
    //
    *now = t_now;
}

/*
*  malloc a buffer in the untrusted memory
*/
void ocall_malloc( size_t size, uint8_t** ret )
{
    if ( size > 0 ) {
        *ret = ( uint8_t* )malloc( size );
        if ( *ret ) {
            memset( *ret, 0, size );
        }
        else {
            fprintf( stderr, "[%s][%d]ocall_malloc() failed with size: %d!", __FILE__, __LINE__, (int)size );
        }
    }
}

/*
*  free a buffer in the untrusted memory,
*  which is malloc by calling ocall_malloc()
*/
void ocall_free( uint8_t* p_out_side )
{
    if ( p_out_side ) {
        free ( p_out_side );
        p_out_side = nullptr;
    }
}

/**
 * Output log messag with glog library.
 * add log prefix by hand, don't use the default prefix in glog (set FLAGS_log_prefix = false)
 * because we need to use a standard log format which is defined
 * by log monitor server.
 * the standard log format is below:
 * yyyy-MM-dd HH:mm:ss.SSS[${PROJECT_NAME}][LEVEL] - message
*/
void ocall_log(uint32_t level, char * message ){
    __int64_t t_usec;
    double wall_time;
    time_t t_now;
    struct tm tm_time;
    char log_time[256] = { 0 };
    char log_prefix[512] = { 0 };

    // get current time in microseconds
    t_usec = time_usec();

    // convert it to seconds
    wall_time = 0.000001 * t_usec;

    // convert it to time_t
    t_now = static_cast<time_t>(wall_time);

    // convert it with local
    localtime_r( &t_now, &tm_time );

    sprintf(log_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            1900+tm_time.tm_year, 1+tm_time.tm_mon, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            static_cast<int32_t>((wall_time - t_now) * 1000) );

    switch ( level ){
        case LL_INFO:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "INFO" );
            LOG(INFO) << log_prefix << message;
            break;
        case LL_WARNING:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "WARNING" );
            LOG(WARNING) << log_prefix << message;
            break;
        case LL_ERROR:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "ERROR" );
            LOG(ERROR) << log_prefix << message;
            break;
//    case LL_FATAL:
//      LOG(FATAL) << message;
//      break;
    }
}

#ifdef __cplusplus
}
#endif