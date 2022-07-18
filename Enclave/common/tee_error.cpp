#include "tee_error.h"

typedef struct _enc_errlist_t {
    int err;
    const char *msg;
    const char *sug; /* Suggestion */
} enc_errlist_t;

/* Error code returned by enclave api */
static enc_errlist_t enclave_errlist[] = {
    { TEE_OK, "no error", nullptr }, //!< No error
    { TEE_ERROR_GRPC_CONNECTION, "error occurs on grpc connection", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_CONFIGURE, "failed to load enclave configure file in enclave initialize", nullptr },
    { TEE_ERROR_FAILED_TO_DECRYPT_REQUEST, "failed to decrypt the request by tls private key", nullptr },
    { TEE_ERROR_FAILED_TO_DISPATCH_REQUEST, "failed to dispatch the request, maybe it has a wrong format", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_REMOTE_PUBKEY, "failed to load the remote public key in request", nullptr },
    { TEE_ERROR_FAILED_TO_ENCRYPT_RESULT, "failed to encrypt the result data by tls public key", nullptr },
    { TEE_ERROR_FAILED_TO_MALLOC, "failed to call malloc api", nullptr },
    { TEE_ERROR_TASK_INTERNAL_ERROR, "encounter an internal error", nullptr },
    { TEE_ERROR_INVALID_PARAM, "unction input parameters are invalid", nullptr },
    { TEE_ERROR_INVALID_CALL, "this call is invalid, maybe it's called in a wrong order", nullptr },
    { TEE_ERROR_BUFFER_TOO_SMALL, "the output buffer is too small", nullptr },
    { TEE_ERROR_ALG_NOTSUPPORT, "the specified algorith is not support", nullptr },
    { TEE_ERROR_FUNCTION_NOTSUPPORT, "this function is not supported yet", nullptr },
    { TEE_ERROR_FAILED_TO_VERIFY_REQUEST, "failed to verify the request's signature by remote public key", nullptr },
    { TEE_ERROR_FAILED_TO_SIGN_RESULT, "failed to sign result data by tls private key", nullptr },
    { TEE_ERROR_FAILED_TO_DECRYPT_ARWEAVE, "failed to decrypt ARWEAVE by tls private key", nullptr },
    { TEE_ERROR_FAILED_TO_WRAP_ARWEAVE, "failed to wrap ARWEAVE by work key", nullptr },
    { TEE_ERROR_FAILED_TO_UNWRAP_ARWEAVE, "failed to unwrap ARWEAVE by work key", nullptr },
    { TEE_ERROR_FAILED_TO_SIGN_ARWEAVE, "failed to sign ARWEAVE by work key", nullptr },
    { TEE_ERROR_FAILED_TO_VERIFY_ARWEAVE, "failed to verify ARWEAVE's signature by work key", nullptr },
    { TEE_ERROR_FAILED_TO_ENCODE_BASE64, "base64 encode failed", nullptr },
    { TEE_ERROR_FAILED_TO_DECODE_BASE64, "base64 decode failed", nullptr },
    { TEE_ERROR_FAILED_TO_CALC_HMAC, "failed to calculate the HMAC", nullptr },
    { TEE_ERROR_FAILED_TO_SEAL_MASTER_KEY, "failed to seal the master key during saving", nullptr },
    { TEE_ERROR_FAILED_TO_UNSEAL_MASTER_KEY, "failed to unseal the master key during loading", nullptr },
    { TEE_ERROR_FAILED_TO_SETUP_HTTPS, "failed to setup parameters for HTTPS", nullptr },
    { TEE_ERROR_FAILED_TO_POST_REQUEST, "failed to POST the request to HTTPS", nullptr },
    { TEE_ERROR_HTTP_RESPONSE_ERROR, "the HTTPS response data is wrong", nullptr },
    { TEE_ERROR_HTTP_RESPONSE_BODY_WRONG, "the HTTPS response body format is wrong", nullptr },
    { TEE_ERROR_DATABASE_OPT_FAILED, "the database opteration is failed", nullptr },
    { TEE_ERROR_NO_MASTER_KEY, "there is not a master key in configure and database", nullptr },
    { TEE_ERROR_NO_CALLER_PUBKEY, "there is not a caller public key in configure and database", nullptr },
    { TEE_ERROR_FAILED_TO_GEN_INIT_KEY, "failed to generate the initiliaze keypair for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_MASTER_KEY, "failed to load the master key for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_DERIVE_TLS_KEY, "failed to derive the tls key from master key for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_WORK_KEY, "failed to load the work key for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_CALLER_KEY, "failed to load the caller's public key for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_LOAD_PERVSTEP_KEY, "failed to load the pervious step (serialize service) public key for enclave", nullptr },
    { TEE_ERROR_FAILED_TO_SERIALIZE_DATA, "failed to serialize a data by protobuf", nullptr },
    { TEE_ERROR_FAILED_TO_UNSERIALIZE_DATA, "failed to unserialize a data by protobuf", nullptr },
    { TEE_ERROR_KMS_ENCRYPT_FAILED, "failed to encrypt a data by KMS", nullptr },
    { TEE_ERROR_KMS_DECRYPT_FAILED, "failed to decrypt a data by KMS", nullptr },
    { TEE_ERROR_WRAP_DATA_FAILED, "failed to wrap data with the public key", nullptr },
    { TEE_ERROR_UNWRAP_DATA_FAILED, "failed to wrap data with the private key", nullptr },
    { TEE_ERROR_VERIFY_SIGN_FAILED, "failed to verify signature with the public key", nullptr },
    { TEE_ERROR_SIGN_DATA_FAILED, "failed to sign data with the private key", nullptr },
    { TEE_ERROR_ARWEAVE_BELONG_WRONG, "the ARWEAVE belong information is wrong", nullptr },
    { TEE_ERROR_TEE_IS_BUSY, "TEE is busy! The grpc request numbers meet the max.", nullptr }
};

const char * t_strerror( int error_code )
{
    size_t idx = 0;
    size_t count = 0;
    
    count =sizeof( enclave_errlist )/sizeof( enclave_errlist[0] );
    for ( idx = 0; idx < count; idx++ ) {
        if ( error_code == enclave_errlist[idx].err ) {
            return enclave_errlist[idx].msg;
        }
    }
    
    return "Unknown error!";
}


