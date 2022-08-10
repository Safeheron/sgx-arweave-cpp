/**
 * @file msg_handler.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "msg_handler.h"
#include "listen_svr.h"
#include "Enclave_u.h"
#include "keyshare_param.h"
#include "../common/define.h"
#include "../common/log_u.h"
#include "../common/tee_error.h"
#include <cpprest/http_client.h>
#include <safeheron/crypto-bn/bn.h>
#include <safeheron/crypto-bn/rand.h>
#include <safeheron/crypto-encode/base64.h>
#include <sgx_urts.h>
#include <sgx_report.h>
#include <sgx_dcap_ql_wrapper.h>
#include <sgx_pce.h>
#include <sgx_error.h>
#include <sgx_quote_3.h>
#include <list>

using namespace web;
using namespace http;
using namespace utility;
using safeheron::bignum::BN;

#define SGX_AESM_ADDR "SGX_AESM_ADDR"
#if defined(_MSC_VER)
#define ENCLAVE_PATH _T("enclave.signed.dll")
#else
#define ENCLAVE_PATH "enclave.signed.so"
#endif

// defined in file App.cpp
extern sgx_enclave_id_t global_eid;

// thread pool and mutex object
std::list<ThreadTask*> msg_handler::s_thread_pool;
std::mutex msg_handler::s_thread_lock;

/**
 * @brief : To call ECALL generate a group key share,
 *          this function will be called in task thread function 
 *          because it will take a few seconds. 
 * 
 * @param keyshare_param 
 * @return int 
 */
static int GenerateKeyShare_Task( void* keyshare_param )
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
    KeyShareParam* param = (KeyShareParam*) keyshare_param;
    web::json::value result_json;

    if ( !param ) {
        ERROR( "keyshare_param is null in GenerateKeyShare()!" );
        return -1;
    }
    param_string = param->to_json_string();
    request_id = param->request_id_;

    // call ECALL to generate keys status in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Generate, request_id.c_str(), 
          param_string.c_str(), param_string.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() encounter an error! sgx_status: %d, error message: %s", 
            request_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        reply_body = msg_handler::GetMessageReply( request_id, false, "ECALL encounter an error!" );
        ret = -1;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Generate! ret: 0x%x, error message: %s", 
            request_id.c_str(), ret, result ? result : "" );
        ERROR( "Request ID: %s,  param_string: %s", request_id.c_str(), param_string.c_str() );
        reply_body = msg_handler::GetMessageReply( request_id, false, result );
        ret = -1;
        goto _exit;
    }
    INFO_OUTPUT_CONSOLE("Request ID: %s, generate key share successfully.", request_id.c_str());

    // get public key list hash in result
    result_json = json::value::parse( result );
    pubkey_list_hash = result_json.at( NODE_NAME_PUBKEY_LIST_HASH ).as_string();

    // create enclave quote
    if ( (ret = msg_handler::CreateEnclaveReport( request_id, pubkey_list_hash, enclave_report)) != 0 ) {
        ERROR( "Request ID: %s,  msg_handler::CreateEnclaveReport() failed! pubkey_list_hash: %s, ret: %d", 
            request_id.c_str(), pubkey_list_hash.c_str(), ret );
        ret = -1;
        goto _exit;
    }

    INFO_OUTPUT_CONSOLE("Request ID: %s, generate remote attestation report successfully.", request_id.c_str());

    // add remote attestation report to Json object
    result_json["tee_report"] = json::value( enclave_report );

    // serialize Json object to a String
    reply_body = result_json.serialize();

    INFO_OUTPUT_CONSOLE("Request ID: %s, second time packing data successfully.", request_id.c_str());

    // OK
    ret = 0;

_exit:
    try {
        listen_svr::PostRequest( param->request_id_, param->callback_, reply_body ).wait();
    } catch ( const std::exception &e ) {
        ERROR( "Request ID: %s Error exception: %s", param->request_id_.c_str(), e.what() );
    }
    if ( result ) {
        free( result );
        result = nullptr;
    }
    if ( param ) {
        delete param;
        param = nullptr;
    }

    return ret;
}

msg_handler::msg_handler()
{

}
msg_handler::~msg_handler()
{
    
}


/**
 * @brief : The HTTP message handle function
 * 
 * @param req_path : the request path name string
 * @param req_body : a JSON string for request body
 * @param resp_body : return the response body string, in JSON
 * @return int : return 0 if success, otherwise return an error code.
 */
int msg_handler::process( 
    const std::string & req_path, 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;

    if ( req_path == HTTP_REQ_GENERATE_KEYSHARE ) {
        ret = GenerateKeyShare( req_body, resp_body );
    }
    else if ( req_path == HTTP_REQ_QUERY_KEYSTATE ) {
        ret = QueryKeyShareState( req_body, resp_body );
    }
    else {
        ERROR( "Request path is unknown! req_path: %s", req_path.c_str() );
        resp_body = GetMessageReply( "", false, "Request path is unknown!" );
        ret = -1;
    }

    return ret;
}

// Generate an uniqne ID for every request process, and this ID
// will be output log file to identify logs for this request process.
std::string msg_handler::CreateRequestID( const std::string & prefix )
{
    std::string ret;

    BN rand_bn = safeheron::rand::RandomBNStrict( 16 );
    rand_bn.ToHexStr( ret );

    return prefix + ret;
}

// Construct a reply JSON string with nodes "success" and "message".
std::string msg_handler::GetMessageReply( 
    const std::string & request_id, 
    bool success, 
    const std::string & message )
{
    json::value root = json::value::object( true );
    root["request_id"] = json::value( request_id );
    root["success"] = json::value( success );
    root["message"] = json::value( message );
    return root.serialize();
}

// create the report for current enclave
int msg_handler::CreateEnclaveReport( 
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

_exit:
    if ( !p_quote_buffer ) {
        free( p_quote_buffer );
        p_quote_buffer = nullptr;
    }
    return ret;
}

// free all thread objects in s_thread_pool
void msg_handler::DestoryThreadPool()
{
    std::lock_guard<std::mutex> lock( s_thread_lock );
    for ( auto it = s_thread_pool.begin(); 
          it != s_thread_pool.end(); 
        ) {
        delete *it;
        it = s_thread_pool.erase( it );
    }
}

// To generate a key shards group in TEE
// This message will be handled in asynchronous, 
// we create a child thread and return immediately in this function.
// In the child thread function, we will call ECALL to generate a key shard group,
// and the result will be send to requester by callback function.
// 
int msg_handler::GenerateKeyShare( 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;
    std::string pubkey_list_hash;
    KeyShareParam* req_param = nullptr;
    
    std::lock_guard<std::mutex> lock( s_thread_lock );

    // free all stopped task threads in pool
    for ( auto it = s_thread_pool.begin(); it != s_thread_pool.end(); ) {
        if ( (*it)->is_stopped() ) {
            delete *it;
            s_thread_pool.erase( it );
        }
        else {
            it++;
        }
    }
    s_thread_lock.unlock();

    // all parameters must be OK!
    if ( !(req_param = new KeyShareParam( req_body )) ) {
        ERROR( "new KeyShareParam object failed!" );
        resp_body = GetMessageReply( "", false, "new KeyShareParam object failed!" );
        return -1;
    }
    if ( !req_param->pubkey_list_is_ok() ) {
        ERROR( "User pubkey list is wrong! size: %d", (int)req_param->pubkey_list_.size() );
        resp_body = GetMessageReply( "", false, "User pubkey list is wrong!" );
        return -1;
    }
    if ( !req_param->k_is_ok() ) {
        ERROR( "Parameter k is wrong! k: %d", req_param->k_ );
        resp_body = GetMessageReply( "", false, "Parameter k is wrong!" );
        return -1;
    }
    if ( !req_param->l_is_ok() ) {
        ERROR( "Parameter l is wrong! l: %d", req_param->l_ );
        resp_body = GetMessageReply( "", false, "Parameter l is wrong!" );
        return -1;
    }
    if ( !req_param->key_length_is_ok() ) {
        ERROR( "Parameter key length is wrong! key_legnth: %d", req_param->key_length_ );
        resp_body = GetMessageReply( "", false, "Parameter key length is wrong!" );
        return -1;
    }

    // calc pubkey list hash for request_id
    pubkey_list_hash = req_param->calc_pubkey_list_hash( );
    INFO( "User pubkey list hash: %s", pubkey_list_hash.c_str() );

    // generate the request_id for this requestion
    req_param->request_id_ = CreateRequestID( pubkey_list_hash );
    INFO( "Request ID: %s", req_param->request_id_.c_str() );

    // return is thread pool is full
    s_thread_lock.lock();
    if ( s_thread_pool.size() >= MAX_TASKTHREAD_COUNT ) {
        resp_body = GetMessageReply( req_param->request_id_, false, "TEE service is busy!" );
        return -1;
    }
    s_thread_lock.unlock();

    // create a thread for generation task
    ThreadTask* task = new ThreadTask( GenerateKeyShare_Task, req_param );
    if ( (ret = task->start()) != 0 ) {
        resp_body = GetMessageReply( req_param->request_id_, false, "Create task thread failed!" );
        return -1;
    }
    s_thread_lock.lock();
    s_thread_pool.push_back( task );
    s_thread_lock.unlock();

    // return OK
    resp_body = GetMessageReply( req_param->request_id_, true, "Request has been accepted." );

    return ret;
}

// To query key shard's status in TEE
// This message will be handled in synchronous
int msg_handler::QueryKeyShareState( 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;
    size_t result_len = 0;
    char* result = nullptr;
    sgx_status_t sgx_status;
    std::string pubkey_list_hash;
    std::string request_id;
    web::json::value req_json = json::value::parse(req_body);

    request_id = CreateRequestID( "" );

    // return error message if request body is invalid
    if ( !req_json.has_field( NODE_NAME_PUBKEY_LIST_HASH ) ||
         !req_json.at( NODE_NAME_PUBKEY_LIST_HASH ).is_string() ) {
        ERROR( "Request ID: %s, %s node is not in request body or has a wrong type!", 
            request_id.c_str(), NODE_NAME_PUBKEY_LIST_HASH );
        resp_body = GetMessageReply( request_id, false, "invalid input, please check your data.");
        ret = -1;
        goto _exit;
    }
    pubkey_list_hash = req_json.at( NODE_NAME_PUBKEY_LIST_HASH ).as_string();

    // call ECALL to query keys status in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Query, request_id.c_str(), 
          pubkey_list_hash.c_str(), pubkey_list_hash.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() encounter an error! sgx_status: %d, error message: %s", 
            request_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        resp_body = GetMessageReply( request_id, false, "ECALL encounter an error!");
        ret = -1;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Query! pubkey_list_hash: %s, ret: 0x%x, error message: %s", 
            request_id.c_str(), pubkey_list_hash.c_str(), ret, result ? result : "" );
        resp_body = GetMessageReply( request_id, false, result );
        ret = -1;
        goto _exit;
    }

    // OK
    resp_body = result;
    ret = 0;

_exit:
    if ( result ) {
        free( result );
        result = nullptr;
    }
    return ret;
}