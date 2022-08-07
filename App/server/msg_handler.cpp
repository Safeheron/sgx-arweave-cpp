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
#include "keyshare_param.h"
#include "define.h"
#include "Enclave_u.h"
#include "log_u.h"
#include "tee_error.h"
#include <cpprest/http_client.h>
#include <safeheron/crypto-bn/bn.h>
#include <safeheron/crypto-bn/rand.h>
#include <sgx_urts.h>
#include <list>

using namespace web;
using namespace http;
using namespace utility;
using safeheron::bignum::BN;

// defined in file App.cpp
extern sgx_enclave_id_t global_eid;

// thread pool
std::list<ThreadTask*> msg_handler::s_thread_pool;

/**
 * @brief : To call ECALL generate a group key share,
 *          this function will be called in thread function 
 *          because it will take a few seconds. 
 * 
 * @param keyshare_param 
 * @return int 
 */
static int GenerateKeyShare( void* keyshare_param )
{
    int ret;
    size_t result_len = 0;
    char* result = nullptr;
    sgx_status_t sgx_status;
    std::string request_id;
    std::string param_json_str;
    std::string reply_body;
    KeyShareParam* param = (KeyShareParam*) keyshare_param;

    if ( !param ) {
        ERROR( "keyshare_param is null in GenerateKeyShare()!" );
        return -1;
    }
    param_json_str = param->to_json_string();
    request_id = param->request_id_;

    // call ECALL to generate keys status in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Generate, request_id.c_str(), 
          param_json_str.c_str(), param_json_str.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() encounter an error! sgx_status: %d, error message: %s", 
            request_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        reply_body = msg_handler::getMessageReply( request_id, false, "ECALL encounter an error!" );
        ret = -1;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Generate! ret: 0x%x, error message: %s", 
            request_id.c_str(), ret, result ? result : "" );
        ERROR( "Request ID: %s,  param_json_str: %s", request_id.c_str(), param_json_str.c_str() );
        reply_body = msg_handler::getMessageReply( request_id, false, result );
        ret = -1;
        goto _exit;
    }

    // OK
    reply_body = result;
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
        ret = generateKeyShare( req_body, resp_body );
    }
    else if ( req_path == HTTP_REQ_QUERY_KEYSTATE ) {
        ret = queryKeyShareState( req_body, resp_body );
    }
    else {
        ERROR( "Request path is unknown! req_path: %s", req_path.c_str() );
        resp_body = getMessageReply( "", false, "Request path is unknown!" );
        ret = -1;
    }

    return ret;
}

// Generate an uniqne ID for every request process, and this ID
// will be output log file to identify logs for this request process.
std::string msg_handler::generateRequestID( const std::string & prefix )
{
    std::string ret;

    BN rand_bn = safeheron::rand::RandomBNStrict( 16 );
    rand_bn.ToHexStr( ret );

    return prefix + ret;
}

// Construct a reply JSON string with nodes "success" and "message".
std::string msg_handler::getMessageReply( 
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

// free all thread objects in s_thread_pool
void msg_handler::destoryThreadPool()
{
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
int msg_handler::generateKeyShare( 
    const std::string & req_body, 
    std::string & resp_body )
{
    int ret = 0;
    std::string pubkey_list_hash;
    KeyShareParam* req_param = nullptr;
    
    if ( !(req_param = new KeyShareParam( req_body )) ) {
        ERROR( "new KeyShareParam object failed!" );
        resp_body = getMessageReply( "", false, "new KeyShareParam object failed!" );
        return -1;
    }
    if ( !req_param->pubkey_list_is_ok() ) {
        ERROR( "User pubkey list is wrong! size: %d", (int)req_param->pubkey_list_.size() );
        resp_body = getMessageReply( "", false, "User pubkey list is wrong!" );
        return -1;
    }
    if ( !req_param->k_is_ok() ) {
        ERROR( "Parameter k is wrong! k: %d", req_param->k_ );
        resp_body = getMessageReply( "", false, "Parameter k is wrong!" );
        return -1;
    }
    if ( !req_param->l_is_ok() ) {
        ERROR( "Parameter l is wrong! l: %d", req_param->l_ );
        resp_body = getMessageReply( "", false, "Parameter l is wrong!" );
        return -1;
    }
    if ( !req_param->key_length_is_ok() ) {
        ERROR( "Parameter key length is wrong! key_legnth: %d", req_param->key_length_ );
        resp_body = getMessageReply( "", false, "Parameter key length is wrong!" );
        return -1;
    }

    pubkey_list_hash = req_param->calc_pubkey_list_hash( );
    INFO( "User pubkey list hash: %s", pubkey_list_hash.c_str() );

    req_param->request_id_ = generateRequestID( pubkey_list_hash );
    INFO( "Request ID: %s", req_param->request_id_.c_str() );

    ThreadTask* task = new ThreadTask( GenerateKeyShare, req_param );
    s_thread_pool.push_back( task );

    return ret;
}

// To query key shard's status in TEE
// This message will be handled in synchronous
int msg_handler::queryKeyShareState( 
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

    request_id = generateRequestID( "" );

    // return error message if request body is invalid
    if ( !req_json.has_field( NODE_NAME_PUBKEY_LIST_HASH ) ||
         !req_json.at( NODE_NAME_PUBKEY_LIST_HASH ).is_string() ) {
        ERROR( "Request ID: %s, %s node is not in request body or has a wrong type!", 
            request_id.c_str(), NODE_NAME_PUBKEY_LIST_HASH );
        resp_body = getMessageReply( request_id, false, "invalid input, please check your data.");
        ret = -1;
        goto _exit;
    }
    pubkey_list_hash = req_json.at( NODE_NAME_PUBKEY_LIST_HASH ).as_string();

    // call ECALL to query keys status in TEE
    if ( (sgx_status = ecall_run( global_eid, &ret, eTaskType_Query, request_id.c_str(), 
          pubkey_list_hash.c_str(), pubkey_list_hash.length(), &result, &result_len )) != SGX_SUCCESS) {
        ERROR( "Request ID: %s,  ecall_run() encounter an error! sgx_status: %d, error message: %s", 
            request_id.c_str(), sgx_status, t_strerror( (int)sgx_status));
        resp_body = getMessageReply( request_id, false, "ECALL encounter an error!");
        ret = -1;
        goto _exit;
    }
    if ( 0 != ret ) {
        ERROR( "Request ID: %s,  ecall_run() failed with eTaskType_Query! pubkey_list_hash: %s, ret: 0x%x, error message: %s", 
            request_id.c_str(), pubkey_list_hash.c_str(), ret, result ? result : "" );
        resp_body = getMessageReply( request_id, false, result );
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

