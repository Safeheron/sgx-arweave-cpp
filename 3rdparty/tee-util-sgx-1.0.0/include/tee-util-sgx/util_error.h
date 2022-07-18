/*************************************************
 * File Name: util_error.h
 * Introduce: Enum and constant things define file
 * Create: 2021-6-2 by yyf
 * 
 *************************************************/

#ifndef _UTIL_ERROR_H_
#define _UTIL_ERROR_H_

#define UTIL_LIB_OK                             0       /* OK */
#define UTIL_LIB_ERROR_INVALID_PARAM            -101    /* input parameter is invalid */
#define UTIL_LIB_ERROR_INVALID_CALL             -102    /* this call is invalid, or in the wrong step */
#define UTIL_LIB_ERROR_OPEN_FILE_FAILED         -103    /* call sgx_fopen_auto_key() failed */
#define UTIL_LIB_ERROR_GET_FILESIZE_FAILED      -104    /* call sgx_ftell() failed */
#define UTIL_LIB_ERROR_FILE_IS_EMPTY            -105    /* file size is 0 */
#define UTIL_LIB_ERROR_FAILED_TO_MALLOC         -106    /* call malloc() failed */
#define UTIL_LIB_ERROR_READ_FILE_FAILED         -107    /* call sgx_fread() failed */
#define UTIL_LIB_ERROR_WRITE_FILE_FAILED        -108    /* call sgx_fwrite() failed */
#define UTIL_LIB_ERROR_UNSEAL_INVALID_SIGNATURE -109    /* verify signature failed when unseal cipher data */
#define UTIL_LIB_ERROR_OUT_OF_MEMORY            -110    /* call malloc() failed, returns null */
#define UTIL_LIB_ERROR_GET_SEALKEY_FAILED       -111    /* call sgx_get_key() failed */
#define UTIL_LIB_ERROR_SIGN_SEALED_DATA_FAILED  -112    /* signing sealed data failed */
#define UTIL_LIB_ERROR_CIPHER_ERROR             -113    /* seal data failed */
#define UTIL_LIB_ERROR_UNSEALED_DATA_FAILED     -114    /* unseal data failed */
#define UTIL_LIB_ERROR_CREATE_SEFREPORT_FAILED  -115    /* call sgx_self_report() failed */
#define UTIL_LIB_ERROR_BUFFER_TOO_SMALL         -116    /* the output buffer is too small */
#define UTIL_LIB_ERROR_SET_AESKEY_FAILED        -117    /* failed to create an AES key */
#define UTIL_LIB_ERROR_AES_ENCRYPT_FAILED       -118    /* failed to encrypt data with an AES key */
#define UTIL_LIB_ERROR_AES_DECRYPT_FAILED       -119    /* failed to decrypt cipher with an AES key */
#define UTIL_LIB_ERROR_UNPAD_FAILED             -120    /* PKCS#5 unpad failed */
#define UTIL_LIB_ERROR_RANDOM_SEED_FAILED       -121    /* failed to set random seed for mbedtls */
#define UTIL_LIB_ERROR_RANDOM_GEN_FAILED        -122    /* failed to generate a random with mbedtls */
#define UTIL_LIB_ERROR_PROTO_SERIALIZE_FAILED   -123    /* failed to serialize data to string with protobuf */
#define UTIL_LIB_ERROR_PROTO_UNSERIALIZE_FAILED -124    /* failed to unserialize string to data with protobuf */
#define UTIL_LIB_ERROR_HTTP_RESPONSE_ERROR      -125    /* HTTP response an error */
#define UTIL_LIB_ERROR_ECC_ENCRYPT_FAILED       -126    /* failed to encrypt data by ecc public key */
#define UTIL_LIB_ERROR_ECC_DECRYPT_FAILED       -127    /* failed to decrypt data by ecc private key */
#define UTIL_LIB_ERROR_FAILED_TO_CALC_HMAC      -128    /* failed to calculate HMAC */


#endif //_UTIL_ERROR_H_

