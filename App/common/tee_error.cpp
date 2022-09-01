#include "tee_error.h"
#include <stddef.h>

const char * t_strerror( int error_code )
{
    size_t idx = 0;
    size_t count = 0;

    count =sizeof( sgx_errlist )/sizeof( sgx_errlist[0] );
    for ( idx = 0; idx < count; idx++ ) {
        if ( error_code == sgx_errlist[idx].err ) {
            return sgx_errlist[idx].msg;
        }
    }
    
    return "Unknown error!";
}
