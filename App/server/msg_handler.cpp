#include "msg_handler.h"
#include "listen_svr.h"
#include "Enclave_u.h"
#include "keyshard_param.h"
#include "../common/define.h"
#include "../common/log_u.h"
#include "../common/tee_error.h"
#include <cpprest/http_client.h>
#include <safeheron/crypto-encode/base64.h>
#include <sgx_urts.h>
#include <sgx_report.h>
#include <sgx_dcap_ql_wrapper.h>
#include <sgx_pce.h>
#include <sgx_error.h>
#include <sgx_quote_3.h>
#include <list>
#include "../App.h"

using namespace web;
using namespace http;
using namespace utility;

#define SGX_AESM_ADDR "SGX_AESM_ADDR"
#if defined(_MSC_VER)
#define ENCLAVE_PATH _T("enclave.signed.dll")
#else
#define ENCLAVE_PATH "enclave.signed.so"
#endif

// Defined in App.cpp
extern sgx_enclave_id_t global_eid;
extern std::string g_key_shard_generation_path;
extern std::string g_key_shard_query_path;
extern int g_max_thread_task_count;

// Thread pool and mutex
std::list<ThreadTask*> msg_handler::s_thread_pool;
std::mutex msg_handler::s_thread_lock;

/**
 * @brief : Call ECALL to enter the enclave. The key shards will be generated in TEE.
 *          This function is called by generation request task thread
 *          as it will take some time to calculate.
 *
 * @param[in] keyshard_param : The context of GenerateKeyShard_Task.
 * @return int: return 0 if success, otherwise return an error code.
 */
static int GenerateKeyShard_Task(void* keyshard_param )
{
    int ret;
    size_t result_len = 0;
    char* result = nullptr;
    sgx_status_t sgx_status;
    std::string request_id;
    std::string pubkey_list_hash;
    std::string param_string;
    std::string enclave_report;
    std::string reply_body;
    KeyShardParam* param = (KeyShardParam*) keyshard_param;
    web::json::value result_json;

    FUNC_BEGIN;

    if ( !param ) {
        ERROR( "keyshard_param is null in GenerateKeyShard()!" );
        reply_body = msg_handler::GetMessageReply( false, APP_ERROR_INVALID_PARAMETER, "keyshard_param is null in GenerateKeyShard()!" );
        ret = APP_ERROR_INVALID_PARAMETER;
        goto _exit;
    }
    param_string = param->to_json_string();
    request_id = param->request_id_;

    // Call ECALL to generate keys shards in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Generate, request_id.c_str(), 
          param_string.c_str(), param_string.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() raised an error! sgx_status: %d, error message: %s",
            request_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        reply_body = msg_handler::GetMessageReply( false, sgx_status, "ECALL raised an error!" );
        ret = sgx_status;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Generate! ret: 0x%x, error message: %s", 
            request_id.c_str(), ret, result ? result : "" );
        ERROR( "Request ID: %s,  param_string: %s", request_id.c_str(), param_string.c_str() );
        reply_body = msg_handler::GetMessageReply( false, ret, result ? result : "" );
        goto _exit;
    }
    INFO_OUTPUT_CONSOLE("Request ID: %s, generate key shards successfully.", request_id.c_str());

    // Get public key list hash in result
    result_json = json::value::parse( result );
    pubkey_list_hash = result_json.at(FIELD_NAME_PUBKEY_LIST_HASH ).as_string();

    // Generate enclave quote
    if ( (ret = msg_handler::GenerateEnclaveReport(request_id, pubkey_list_hash, enclave_report)) != 0 ) {
        ERROR( "Request ID: %s,  msg_handler::GenerateEnclaveReport() failed! pubkey_list_hash: %s, ret: %d",
            request_id.c_str(), pubkey_list_hash.c_str(), ret );
        reply_body = msg_handler::GetMessageReply( false, ret, "Failed to create enclave report!" );
        goto _exit;
    }

    INFO_OUTPUT_CONSOLE("Request ID: %s, generate remote attestation report successfully.", request_id.c_str());

    // Add remote attestation report to JSON object
    result_json["tee_report"] = json::value( enclave_report );

    // Serialize JSON object to a string
    reply_body = result_json.serialize();

    INFO_OUTPUT_CONSOLE("Request ID: %s, second time packing data successfully.", request_id.c_str());

    // OK
    ret = 0;

_exit:
    try {
        listen_svr::PostRequest( request_id, param->webhook_url_, reply_body ).wait();
        ecall_set_generation_status( global_eid, &ret, request_id.c_str(), pubkey_list_hash.c_str(), eKeyStatus_Finished );
        INFO_OUTPUT_CONSOLE("Request ID: %s, key shard generation result has post to callback address successfully.", request_id.c_str());
    } catch ( const std::exception &e ) {
        ecall_set_generation_status( global_eid, &ret, request_id.c_str(), pubkey_list_hash.c_str(), eKeyStatus_Error );
        ERROR( "Request ID: %s Error exception: %s", request_id.c_str(), e.what() );
    }
    if ( result ) {
        free( result );
        result = nullptr;
    }
    if ( param ) {
        delete param;
        param = nullptr;
    }

    FUNC_END;

    return ret;
}

msg_handler::msg_handler()
{

}
msg_handler::~msg_handler()
{
    
}

int msg_handler::process( 
    const std::string & req_id,
    const std::string & req_path, 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;

    FUNC_BEGIN;

    if ( req_path == g_key_shard_generation_path ) {
        ret = GenerateKeyShard(req_id, req_body, resp_body);
    }
    else if ( req_path == g_key_shard_query_path ) {
        ret = QueryKeyShardState(req_id, req_body, resp_body);
    }
    else {
        ERROR( "Request path is unknown! req_path: %s", req_path.c_str() );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_REQ_PATH, "Request path is unknown!" );
        ret = APP_ERROR_INVALID_REQ_PATH;
    }

    FUNC_END;

    return ret;
}

// Construct a reply JSON string with nodes "success" and "message".
std::string msg_handler::GetMessageReply( 
    bool success, 
    int code, 
    const char* format, ... )
{
    char message[ 4096 ] = { 0 };
    va_list args;
    va_start( args, format ); 
    vsnprintf( message, sizeof(message)-1, format, args );
    va_end( args );

    json::value root = json::value::object( true );
    root["success"] = json::value( success );
    root["code"] = json::value( code );
    root["message"] = json::value( message );
    return root.serialize();
}

// Generate the report for current enclave
int msg_handler::GenerateEnclaveReport(
    const std::string & request_id, 
    const std::string& pubkey_list_hash, 
    std::string & report )
{
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    quote3_error_t qe3_ret = SGX_QL_SUCCESS;
    uint32_t quote_size = 0;
    uint8_t* p_quote_buffer = nullptr;
    sgx_target_info_t qe_target_info;
    sgx_report_t app_report;
    sgx_quote3_t *p_quote = nullptr;
    sgx_ql_auth_data_t *p_auth_data = nullptr;
    sgx_ql_ecdsa_sig_data_t *p_sig_data = nullptr;
    sgx_ql_certification_data_t *p_cert_data = nullptr;
    FILE *fptr = nullptr;
    bool is_out_of_proc = false;
    char *out_of_proc = getenv(SGX_AESM_ADDR);

    FUNC_BEGIN;

    if ( request_id.length() == 0 ) {
        ERROR( "Request ID is null!" );
        return -1;
    }
    if ( pubkey_list_hash.length() == 0 ) {
        ERROR( "Request ID: %s, pubkey_list_hash is null!", request_id.c_str() );
        return -1;
    }
    INFO( "Request ID: %s, pubkey_list_hash: %s", request_id.c_str(), pubkey_list_hash.c_str() );

    if ( out_of_proc ) {
        is_out_of_proc = true;
    }

#if !defined(_MSC_VER)
    // There 2 modes on Linux: one is in-proc mode, the QE3 and PCE are loaded within the user's process.
    // the other is out-of-proc mode, the QE3 and PCE are managed by a daemon. If you want to use in-proc
    // mode which is the default mode, you only need to install libsgx-dcap-ql. If you want to use the
    // out-of-proc mode, you need to install libsgx-quote-ex as well. This sample is built to demo both 2
    // modes, so you need to install libsgx-quote-ex to enable the out-of-proc mode.
    if (!is_out_of_proc)
    {
        // Following functions are valid in Linux in-proc mode only.
        qe3_ret = sgx_qe_set_enclave_load_policy(SGX_QL_PERSISTENT);
        if(SGX_QL_SUCCESS != qe3_ret) {
            ERROR( "Request ID: %s, Error in set enclave load policy: 0x%04x", request_id.c_str(), qe3_ret );
            ret = -1;
            goto _exit;
        }

        // Try to load PCE and QE3 from Ubuntu-like OS system path
        if (SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_PCE_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_pce.signed.so.1") ||
            SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_QE3_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_qe3.signed.so.1") ||
            SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_IDE_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_id_enclave.signed.so.1")) {

            // Try to load PCE and QE3 from RHEL-like OS system path
            if (SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_PCE_PATH, "/usr/lib64/libsgx_pce.signed.so.1") ||
                SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_QE3_PATH, "/usr/lib64/libsgx_qe3.signed.so.1") ||
                SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_IDE_PATH, "/usr/lib64/libsgx_id_enclave.signed.so.1")) {
                ERROR( "Request ID: %s, Error in set PCE/QE3/IDE directory.", request_id.c_str() );
                ret = -1;
                goto _exit;
            }
        }

        qe3_ret = sgx_ql_set_path(SGX_QL_QPL_PATH, "/usr/lib/x86_64-linux-gnu/libdcap_quoteprov.so.1");
        if (SGX_QL_SUCCESS != qe3_ret) {
            qe3_ret = sgx_ql_set_path(SGX_QL_QPL_PATH, "/usr/lib64/libdcap_quoteprov.so.1");
            if (SGX_QL_SUCCESS != qe3_ret) {
                // Ignore the error, because user may want to get cert type=3 quote
                WARN( "Request ID: %s, Warning: Cannot set QPL directory, you may get ECDSA quote with `Encrypted PPID` cert type.", request_id.c_str() );
            }
        }
    }
#endif

    qe3_ret = sgx_qe_get_target_info(&qe_target_info);
    if (SGX_QL_SUCCESS != qe3_ret) {
        ERROR( "Request ID: %s, Error in sgx_qe_get_target_info. 0x%04x", request_id.c_str(), qe3_ret );
        ret = -1;
        goto _exit;
    }

    sgx_status = ecall_create_report(global_eid, &ret, (char*)request_id.c_str(), 
         (char*)pubkey_list_hash.c_str(), &qe_target_info, &app_report);
    if ( (SGX_SUCCESS != sgx_status) || (0 != ret) ) {
        ERROR( "Request ID: %s, Call to get_app_enclave_report() failed", request_id.c_str() );
        ret = -1;
        goto _exit;
    }

    qe3_ret = sgx_qe_get_quote_size(&quote_size);
    if (SGX_QL_SUCCESS != qe3_ret) {
        ERROR( "Request ID: %s, Error in sgx_qe_get_quote_size. 0x%04x", request_id.c_str(), qe3_ret );
        ret = -1;
        goto _exit;
    }

    p_quote_buffer = (uint8_t*)malloc(quote_size);
    if (nullptr == p_quote_buffer) {
        ERROR( "Request ID: %s, Couldn't allocate quote_buffer", request_id.c_str() );
        ret = -1;
        goto _exit;
    }
    memset(p_quote_buffer, 0, quote_size);

    // Get the Quote
    qe3_ret = sgx_qe_get_quote(&app_report,
                               quote_size,
                               p_quote_buffer);
    if (SGX_QL_SUCCESS != qe3_ret) {
        ERROR( "Request ID: %s, Error in sgx_qe_get_quote. 0x%04x", request_id.c_str(), qe3_ret );
        ret = -1;
        goto _exit;
    }

    p_quote = (sgx_quote3_t*)p_quote_buffer;
    p_sig_data = (sgx_ql_ecdsa_sig_data_t *)p_quote->signature_data;
    p_auth_data = (sgx_ql_auth_data_t*)p_sig_data->auth_certification_data;
    p_cert_data = (sgx_ql_certification_data_t *)((uint8_t *)p_auth_data + sizeof(*p_auth_data) + p_auth_data->size);

#if defined(DEBUG_ARWEAVE)
#if _WIN32
    fopen_s(&fptr, "quote.dat", "wb");
#else
    fptr = fopen("quote.dat","wb");
#endif
    if( fptr )
    {
        fwrite(p_quote, quote_size, 1, fptr);
        fclose(fptr);
    }
#endif

    report = safeheron::encode::base64::EncodeToBase64( p_quote_buffer, quote_size );

    if ( !is_out_of_proc )
    {
        qe3_ret = sgx_qe_cleanup_by_policy();
        if (SGX_QL_SUCCESS != qe3_ret ) {
            ERROR( "Request ID: %s, Error in cleanup enclave load policy: 0x%04x", request_id.c_str(), qe3_ret );
            ret = -1;
            goto _exit;
        }
    }

    // OK
    ret = 0;
    FUNC_END;

_exit:
    if ( !p_quote_buffer ) {
        free( p_quote_buffer );
        p_quote_buffer = nullptr;
    }
    return ret;
}

// Free all threads which are stopped in s_thread_pool
void msg_handler::ReleaseStoppedThreads()
{
    std::lock_guard<std::mutex> lock( s_thread_lock );

    // Free all stopped task threads in pool
    for ( auto it = s_thread_pool.begin(); it != s_thread_pool.end(); ) {
        if ( (*it)->is_stopped() ) {
            delete *it;
            it = s_thread_pool.erase( it );
        }
        else {
            it++;
        }
    }
}

// Free all thread objects in s_thread_pool
void msg_handler::DestroyThreadPool()
{
    std::lock_guard<std::mutex> lock( s_thread_lock );
    for ( auto it = s_thread_pool.begin(); 
          it != s_thread_pool.end(); 
        ) {
        delete *it;
        it = s_thread_pool.erase( it );
    }
}

// Generating key shard message handler
int msg_handler::GenerateKeyShard(
    const std::string & req_id, 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;
    KeyShardParam* req_param = nullptr;

    FUNC_BEGIN;
    
    // All parameters must be valid!
    if ( !(req_param = new KeyShardParam(req_body )) ) {
        ERROR( "Request ID: %s, new KeyShardParam object failed!", req_id.c_str() );
        resp_body = GetMessageReply( false, APP_ERROR_MALLOC_FAILED, "new KeyShardParam object failed!" );
        return APP_ERROR_MALLOC_FAILED;
    }
    if ( !req_param->check_pubkey_list() ) {
        ERROR( "Request ID: %s, User pubkey list is invalid! size: %d", req_id.c_str(), (int)req_param->pubkey_list_.size() );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_PUBLIC_KEY_LIST, "Field '%s' value is invalid!", FIELD_NAME_USER_PUBLICKEY_LIST );
        return APP_ERROR_INVALID_PUBLIC_KEY_LIST;
    }
    if ( !req_param->check_k() ) {
        ERROR( "Request ID: %s, Parameter k is invalid! k: %d", req_id.c_str(), req_param->k_ );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_K, "Field '%s' value is invalid!", FIELD_NAME_NUMERATOR_K );
        return APP_ERROR_INVALID_K;
    }
    if ( !req_param->check_l() ) {
        ERROR( "Request ID: %s, Parameter l is invalid! l: %d", req_id.c_str(), req_param->l_ );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_L, "Field '%s' value is invalid!", FIELD_NAME_DENOMINATOR_L );
        return APP_ERROR_INVALID_L;
    }
    if ( !req_param->check_key_length() ) {
        ERROR( "Request ID: %s, Parameter key length is invalid! key_length: %d", req_id.c_str(), req_param->key_length_ );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_KEYBITS, "Field '%s' value is invalid!", FIELD_NAME_KEY_LENGTH );
        return APP_ERROR_INVALID_KEYBITS;
    }
    if ( !req_param->check_webhook_url() ) {
        ERROR( "Request ID: %s, Parameter webhook url is invalid! webhook url: %s", req_id.c_str(), req_param->webhook_url_.c_str() );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_WEBHOOK_URL, "Field '%s' value is invalid!", FIELD_NAME_WEBHOOK_URL );
        return APP_ERROR_INVALID_WEBHOOK_URL;
    }
    req_param->request_id_ = req_id;

    // Return if thread pool has no thread resource
    std::lock_guard<std::mutex> lock( s_thread_lock );
    if ( s_thread_pool.size() >= g_max_thread_task_count ) {
        resp_body = GetMessageReply( false, APP_ERROR_SERVER_IS_BUSY, "TEE service is busy!" );
        return APP_ERROR_SERVER_IS_BUSY;
    }
    s_thread_lock.unlock();

    // Create a thread for generation task
    ThreadTask* task = new ThreadTask(GenerateKeyShard_Task, req_param);
    if ( (ret = task->start()) != 0 ) {
        resp_body = GetMessageReply( false, APP_ERROR_FAILED_TO_START_THREAD, "Create task thread failed!" );
        return APP_ERROR_FAILED_TO_START_THREAD;
    }
    s_thread_lock.lock();
    s_thread_pool.push_back( task );
    s_thread_lock.unlock();

    // return OK
    resp_body = GetMessageReply( true, 0, "Request has been accepted." );

    FUNC_END;

    return ret;
}

int msg_handler::QueryKeyShardState(
    const std::string & req_id, 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;
    size_t result_len = 0;
    char* result = nullptr;
    sgx_status_t sgx_status;
    std::string pubkey_list_hash;
    web::json::value req_json = json::value::parse(req_body);

    FUNC_BEGIN;

    // return error message if request body is invalid
    if ( !req_json.has_field(FIELD_NAME_PUBKEY_LIST_HASH ) ||
         !req_json.at(FIELD_NAME_PUBKEY_LIST_HASH ).is_string() ) {
        ERROR("Request ID: %s, %s node is not in request body or has a wrong type!",
              req_id.c_str(), FIELD_NAME_PUBKEY_LIST_HASH );
        resp_body = GetMessageReply( false, APP_ERROR_INVALID_PARAMETER,  "invalid input, please check your data.");
        ret = -1;
        goto _exit;
    }
    pubkey_list_hash = req_json.at(FIELD_NAME_PUBKEY_LIST_HASH ).as_string();

    // call ECALL to query keys status in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Query, req_id.c_str(), 
          pubkey_list_hash.c_str(), pubkey_list_hash.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() encounter an error! sgx_status: %d, error message: %s", 
            req_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        resp_body = GetMessageReply( false, sgx_status, "ECALL encounter an error!");
        ret = -1;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Query! pubkey_list_hash: %s, ret: 0x%x, error message: %s", 
            req_id.c_str(), pubkey_list_hash.c_str(), ret, result ? result : "" );
        resp_body = GetMessageReply( false, ret, result ? result : "" );
        ret = -1;
        goto _exit;
    }

    // OK
    resp_body = result;
    ret = 0;

    FUNC_END;

_exit:
    if ( result ) {
        free( result );
        result = nullptr;
    }
    return ret;
}