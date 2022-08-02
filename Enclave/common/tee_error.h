#ifndef PROJECT_TEE_ERROR_H
#define PROJECT_TEE_ERROR_H

#include "Enclave_t.h"

#define MONITOR_MSG_PREFIX  "arweave可信服务"
#define MAX_ERROR_LEN       4096

#define TEE_MONITOR_INFO(format, args...)             \
    do{                                               \
      char info[MAX_ERROR_LEN] = {0};                 \
      snprintf(info, MAX_ERROR_LEN, format, ##args);  \
      INFO( "%s", error );                            \
    }while(0)

#define TEE_MONITOR_ERROR(format, args...)            \
    do{                                               \
      char error[MAX_ERROR_LEN] = {0};                \
      snprintf(error, MAX_ERROR_LEN, format, ##args); \
      ERROR( "%s", error );                           \
    }while(0)

#define TEE_EXCEPTION(a, format, args...)             \
    do{                                               \
      char error[MAX_ERROR_LEN] = {0};                \
      snprintf(error, MAX_ERROR_LEN, format, ##args); \
      ERROR( "%s", error );                           \
      throw TeeException(a, error);                   \
    }while(0)     

/**
 * Put information message to both of log file and console
*/
#define INFO_OUTPUT_CONSOLE(format, args...)        \
    do {                                            \
      char info[MAX_LOG_LEN] = {0};                 \
      snprintf(info, MAX_LOG_LEN, format, ##args);  \
      printf( "INFO: %s\n", info );                       \
      INFO( "%s", info );                           \
    }while(0);

// range: -1 ~ -1000
#define TEE_OK  0 //!< No error
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

/**
 * Monitor message formats
*/
#define MONITOR_MSG_LOAD_INIT_KEY_FAILED        "初始密钥文件存在，但是加载失败，请确认密钥文件是否正确！"
#define MONITOR_MSG_UNSEAL_MASTER_KEY_FAILED    "解密数据库中的主密钥失败，请确认数据是否正确！"
#define MONITOR_MSG_LOAD_MASTER_KEY_PEM_FAILED  "加载主密钥PEM失败，请确认数据是否正确！"
#define MONITOR_MSG_UPDATE_MASTER_KEY_FAILED    "更新主密钥失败，请确认数据是否正确！"
#define MONITOR_MSG_UNWRAP_CALLER_KEY_FAILED    "数据库中存在业务侧的公钥，但是解密或验证签名时失败，请确认数据是否正确！"
#define MONITOR_MSG_LOAD_CALLER_KEY_PEM_FAILED  "加载业务侧公钥PEM失败，请确认数据是否正确！"
#define MONITOR_MSG_UNWRAP_PERVSTEP_KEY_FAILED  "数据库中存在序列化服务公钥，但是解密或验证签名时失败，请确认数据是否正确！"
#define MONITOR_MSG_LOAD_PERVSTEP_KEY_PEM_FAILED "加载序列化服务公钥PEM失败，请确认数据是否正确！"
#define MONITOR_MSG_WORK_KEY_MISSED_IN_DATABASE "数据库中存在业务数据、但是工作密钥数据不存在，请核对数据库状态是否正确！"
#define MONITOR_MSG_KMS_DECRYPT_WORK_KEY_FAILED "KMS解密数据库中工作密钥失败，请确认数据是否正确！"
#define MONITOR_MSG_KMS_ENCRYPT_WORK_KEY_FAILED "KMS加密工作密钥失败，请确认KMS配置是否正确！"
#define MONITOR_MSG_UNWRAP_WORK_KEY_FAILED      "数据库中存在工作密钥，但是解密或验证签名时失败，请确认数据是否正确！"
#define MONITOR_MSG_LOAD_WORK_KEY_PEM_FAILED    "加载数据库中的工作密钥PEM时失败，请核对数据是否正确！"
#define MONITOR_MSG_LOAD_WORK_KEY_FAILED        "工作密钥文件存在，但是加载失败，请确认密文文件是否正确！"
#define MONITOR_MSG_DECRYPT_REQUEST_FAILED      "使用TEE的通信密钥解密请求失败，请确认加密请求时的公钥是否正确！"
#define MONITOR_MSG_VERIFY_REQUEST_FAILED       "使用业务侧的公钥验证请求的签名失败，请确认签名请求时的密钥是否正确！"
#define MONITOR_MSG_ENCRYPT_REPLY_FAILED        "使用业务侧的公钥加密响应数据失败，请检查TEE服务的运行环境是否正常！"
#define MONITOR_MSG_SIGN_REPLY_FAILED           "使用TEE的通信公钥签名响应数据失败，请检查TEE服务的运行环境是否正常！"
#define MONITOR_MSG_DECRYPT_EXT_ARWEAVE_FAILED   "解密外部的SECRET KEY失败，请核对传入的数据和格式是否正确！"
#define MONITOR_MSG_EXT_ARWEAVE_CALC_HMAC_FAILED "外部SECRETE KEY计算HMAC失败！"
#define MONITOR_MSG_SERIALIZEKEY_VERIFY_FAILED  "序列化公钥验证签名失败，请核对传入的数据和格式是否正确！"
#define MONITOR_MSG_DECRYPT_INPUT_ARWEAVE_FAILED "解密传入的SECRET KEY失败，请核对传入的数据和格式是否正确！"
#define MONITOR_MSG_ENCRYPT_ARWEAVE_FAILED       "保存SECRET KEY时，使用工作密钥加密SECRET KEY失败！"
#define MONITOR_MSG_SIGN_ARWEAVE_FAILED          "保存SECRET KEY时，使用工作密钥签名SECRET KEY失败！"
#define MONITOR_MSG_SAVE_ARWEAVE_FAILED          "保存SECRET KEY到数据库时失败！"
#define MONITOR_MSG_QUERY_ARWEAVE_FAILED         "从数据库查询SECRET KEY时失败！"
#define MONITOR_MSG_VERIFY_ARWEAVE_FAILED        "从数据库查询SECRET KEY成功，但是使用工作密钥验证其签名失败！"
#define MONITOR_MSG_DECRYPT_ARWEAVE_FAILED       "从数据库查询SECRET KEY成功，但是使用工作密钥解密时失败！"
#define MONITOR_MSG_VERIFY_ARWEAVE_INFO_FAILED   "从数据库查询SECRET KEY成功，但是其从属关系校验失败！"
#define MONITOR_MSG_ARWEAVE_CALC_HMAC_FAILED     "使用SECRET KEY计算HMAC时失败！"
#define MONITOR_MSG_MALLOC_FAILED               "[%s][%d]调用ocall_malloc()分配内存失败，请及时检查系统情况！"

const char * t_strerror( int error_code );
#endif //PROJECT_TEE_ERROR_H
