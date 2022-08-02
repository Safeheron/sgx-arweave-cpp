#include "enclave_util.h"
#include "Enclave_t.h"
#include <sgx_utils.h>
#include "log_t.h"
#include <stdio.h>


/**
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
            va_end(ap);
    ocall_printf(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}
/**
 * output buffer in HEX string to console
*/
int printf_hex(uint8_t * buf, int len)
{
    printf( "%s\n", s_printf_hex( buf, len ) );

    return 0;
}
/**
 * output buffer to a HEX string
*/
const char* s_printf_hex(uint8_t * buf, int len)
{
    int i = 0;
    int pos = 0;
    const int MAX_LEN = 8*1024;
    int left_size = MAX_LEN;
    static char str[ MAX_LEN ] = { 0 };

    if ( !buf || len <= 0) {
        return "";
    }

    memset( str, 0, MAX_LEN );

    do {
        pos = 2*i;
        snprintf( str + pos, left_size, "%02X", buf[i++] );
        left_size = MAX_LEN - pos;
        if ( left_size <= 2 ) break;
    }while ( i < len );

    return str;
}
/**
 * get current system time (in seconds) by OCALL
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
        ERROR( "ocall_get_system_time() failed! ret = %d", ret );
        return 0;
    }
    t_now = *time_buff;
    ocall_free( (uint8_t*)time_buff );

    return t_now;
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

bool get_sha256_hash(const std::string& input, std::string& out_hash_hex){
    safeheron::hash::CSHA256 pub_hash_sha256;
    uint8_t digest[safeheron::hash::CSHA256::OUTPUT_SIZE];

    /** Input the size and the message of the hash string */
    pub_hash_sha256.Write((const uint8_t *)input.c_str(), input.size());

    /** Output hash */
    pub_hash_sha256.Finalize(digest);

    /** Bytes to Hex */
    out_hash_hex = bytes2hex(digest, safeheron::hash::CSHA256::OUTPUT_SIZE);
    if (out_hash_hex.empty()) {
        return false;
    }

    return true;
}

/** Apply for the memory outside the enclave */
uint8_t* malloc_outside(size_t size)
{
    uint8_t* outside_buf = nullptr;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;

    if ( size <= 0 ) {
        return nullptr;
    }

    // malloc outside buff
    status = ocall_malloc( size, &outside_buf );
    if ( SGX_SUCCESS != status ) {
        ERROR( "Failed to call ocall_malloc(), status: %d", status );
        return nullptr;
    }
    if ( !outside_buf ) {
        ERROR( "ocall_malloc() return null, out of memory!" );
        return nullptr;
    }
    //check buff is outside or not
    if ( sgx_is_outside_enclave( outside_buf, size ) != 1 ) {
        ERROR( "ocall_malloc() return buffer is not in outside!" );
        ocall_free( outside_buf );
        outside_buf = nullptr;
        return nullptr;
    }

    sgx_lfence();

    return outside_buf;
}
