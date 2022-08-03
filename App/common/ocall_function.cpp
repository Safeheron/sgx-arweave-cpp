#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  /* inet_ntop */
#include <time.h>

#ifdef __cplusplus
extern "C"{
#endif

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
*  Output message to console in enclave
*/
void ocall_printf( const char *str )
{
    fprintf( stdout, "%s", str );
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
/*
*  free a buffer in the untrusted memory,
*  which is malloc by calling ocall_get_system_time()
*/
void ocall_free_long( long* p_out_side ) {
    if ( p_out_side ) {
        free ( p_out_side );
        p_out_side = nullptr;
    }
}

#ifdef __cplusplus
}
#endif

