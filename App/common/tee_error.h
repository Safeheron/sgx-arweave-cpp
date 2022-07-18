//
// Created by max on 19-12-26.
//

#ifndef PROJECT_TEE_ERROR_H
#define PROJECT_TEE_ERROR_H

/**
 * Put information message to both of log file and console
*/
#define INFO_OUTPUT_CONSOLE(format, args...)        \
    do {                                            \
      char info[MAX_LOG_LEN] = {0};                 \
      snprintf(info, MAX_LOG_LEN, format, ##args);  \
      fprintf( stdout, "INFO: %s\n", info );        \
      INFO( "%s", info );                           \
    }while(0);

// range: -1 ~ -1000
#define TEE_OK 0 //!< No error
#define TEE_ERROR_GRPC_CONNECTION               -1  /* a grpc error */
#define TEE_ERROR_FAILED_TO_LOAD_CONFIGURE      -2  /* failed to load enclave configure file in enclave initialize */
#define TEE_ERROR_FAILED_TO_DECRYPT_REQUEST     -3  /* failed to decrypt the request by tls private key */
#define TEE_ERROR_FAILED_TO_DISPATCH_REQUEST    -4  /* failed to dispatch the request, maybe it has a wrong format */
#define TEE_ERROR_FAILED_TO_LOAD_REMOTE_PUBKEY  -5  /* failed to load the remote public key in request */
#define TEE_ERROR_FAILED_TO_ENCRYPT_RESULT      -6  /* failed to encrypt the result data by tls public key */
#define TEE_ERROR_FAILED_TO_MALLOC              -7  /* failed to call malloc api */
#define TEE_ERROR_TASK_INTERNAL_ERROR           -8  /* encounter an internal error */
#define TEE_ERROR_INVALID_PARAM                 -9  /* function input parameters are invalid */
#define TEE_ERROR_INVALID_CALL                  -10 /* this call is invalid, maybe it's called in a wrong order */
#define TEE_ERROR_BUFFER_TOO_SMALL              -11 /* the output buffer is too small */
#define TEE_ERROR_ALG_NOTSUPPORT                -12 /* the specified algorith is not support */
#define TEE_ERROR_FUNCTION_NOTSUPPORT           -13 /* this function is not supported yet */
#define TEE_ERROR_FAILED_TO_VERIFY_REQUEST      -14 /* failed to verify the request's signature by remote public key */
#define TEE_ERROR_FAILED_TO_SIGN_RESULT         -15 /* failed to sign result data by tls private key */
#define TEE_ERROR_FAILED_TO_DECRYPT_ARWEAVE      -16 /* failed to decrypt ARWEAVE by tls private key */
#define TEE_ERROR_FAILED_TO_WRAP_ARWEAVE         -17 /* failed to wrap ARWEAVE by work key */
#define TEE_ERROR_FAILED_TO_UNWRAP_ARWEAVE       -18 /* failed to unwrap ARWEAVE by work key */
#define TEE_ERROR_FAILED_TO_SIGN_ARWEAVE         -19 /* failed to sign ARWEAVE by work key */
#define TEE_ERROR_FAILED_TO_VERIFY_ARWEAVE       -20 /* failed to verify ARWEAVE's signature by work key */
#define TEE_ERROR_FAILED_TO_ENCODE_BASE64       -21 /* base64 encode failed */
#define TEE_ERROR_FAILED_TO_DECODE_BASE64       -22 /* base64 decode failed */
#define TEE_ERROR_FAILED_TO_CALC_HMAC           -23 /* failed to calculate the HMAC */
#define TEE_ERROR_FAILED_TO_SEAL_MASTER_KEY     -24 /* failed to seal the master key during saving */
#define TEE_ERROR_FAILED_TO_UNSEAL_MASTER_KEY   -25 /* failed to unseal the master key during loading */
#define TEE_ERROR_FAILED_TO_SETUP_HTTPS         -26 /* failed to setup parameters for HTTPS */
#define TEE_ERROR_FAILED_TO_POST_REQUEST        -27 /* failed to POST the request to HTTPS */
#define TEE_ERROR_HTTP_RESPONSE_ERROR           -28 /* the HTTPS response data is wrong */
#define TEE_ERROR_HTTP_RESPONSE_BODY_WRONG      -29 /* the HTTPS response body format is wrong */
#define TEE_ERROR_DATABASE_OPT_FAILED           -30 /* the database opteration is failed */
#define TEE_ERROR_NO_MASTER_KEY                 -31 /* there is not a master key in configure and database */
#define TEE_ERROR_NO_CALLER_PUBKEY              -32 /* there is not a caller public key in configure and database */
#define TEE_ERROR_FAILED_TO_GEN_INIT_KEY        -33 /* failed to generate the initiliaze keypair for enclave */
#define TEE_ERROR_FAILED_TO_LOAD_MASTER_KEY     -34 /* failed to load the master key for enclave */
#define TEE_ERROR_FAILED_TO_DERIVE_TLS_KEY      -35 /* failed to derive the tls key from master key for enclave */
#define TEE_ERROR_FAILED_TO_LOAD_WORK_KEY       -36 /* failed to load the work key for enclave */
#define TEE_ERROR_FAILED_TO_LOAD_CALLER_KEY     -37 /* failed to load the caller's public key for enclave */
#define TEE_ERROR_FAILED_TO_LOAD_PERVSTEP_KEY   -38 /* failed to load the pervious step (serialize service) public key for enclave */
#define TEE_ERROR_FAILED_TO_SERIALIZE_DATA      -39 /* failed to serialize a data by protobuf */
#define TEE_ERROR_FAILED_TO_UNSERIALIZE_DATA    -40 /* failed to unserialize a data by protobuf */
#define TEE_ERROR_KMS_ENCRYPT_FAILED            -41 /* failed to encrypt a data by KMS */
#define TEE_ERROR_KMS_DECRYPT_FAILED            -42 /* failed to decrypt a data by KMS */
#define TEE_ERROR_WRAP_DATA_FAILED              -43 /* failed to wrap data with the public key */
#define TEE_ERROR_UNWRAP_DATA_FAILED            -44 /* failed to wrap data with the private key */
#define TEE_ERROR_VERIFY_SIGN_FAILED            -45 /* failed to verify signature with the public key */
#define TEE_ERROR_SIGN_DATA_FAILED              -46 /* failed to sign data with the private key */
#define TEE_ERROR_ARWEAVE_BELONG_WRONG           -47 /* the ARWEAVE belong information is wrong */
#define TEE_ERROR_DATABASE_SERVER_INVALID       -48 /* the database service is not available */
#define TEE_ERROR_DATABASE_SERVER_ERROR         -49 /* the database service response an error */
#define TEE_ERROR_DATABASE_INVALID              -50 /* the database is not available */
#define TEE_ERROR_DATABASE_ERROR                -51 /* the database status is wrong */
#define TEE_ERROR_TEE_IS_BUSY                   -1000 /* TEE is busy! The grpc request numbers meet the max */

const char * t_strerror( int error_code );
#endif //PROJECT_TEE_ERROR_H
