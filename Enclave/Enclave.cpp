#include "Enclave.h"
#include "Enclave_t.h"
#include "common/tee_util.h"
#include "common/tee_error.h"
#include "shell/Dispatcher.h"
#include "tasks/TaskConstant.h"
#include "tasks/GenerateTask.h"
#include "tasks/QueryTask.h"

/**
 * The duration time (s) for a finished context
 */
#define KEY_CONTEXT_ALIVE_DURATION  10

Dispatcher g_dispatcher;
std::mutex g_list_mutex;
std::map<std::string, KeyShardContext*> g_keyContext_list;

// Traverse g_keyContext_list, and clear the element which status is an error or is expired
void clear_context(int duration );

// Initialize the enclave
int ecall_init()
{
    int ret = TEE_OK;

    FUNC_BEGIN;

    // Register TEE tasks
    g_dispatcher.register_task( new GenerateTask() );
    g_dispatcher.register_task( new QueryTask() );

    FUNC_END;

    return ret;
}

// Execute a TEE task in enclave
int ecall_run(
    uint32_t type, 
    const char* request_id,
    const char* input_data, 
    uint64_t data_len,
    char **output, 
    uint64_t *output_len)
{
    int ret = 0;
    int reply_len = 0;
    std::string plain_request;
    std::string plain_reply;
    std::string result_data;
    std::string error_msg;
    uint8_t* outside_buff = nullptr;

    FUNC_BEGIN;

    plain_request.assign( input_data, data_len );
    INFO("--->type: %d\n", type);
    INFO("--->request_id: %s\n", request_id);
    INFO("--->plain_request: %s\n", plain_request.c_str());

    // Clear context in list if its status is an error or is expired
    clear_context( KEY_CONTEXT_ALIVE_DURATION );

    // Dispatch requests
    if ( ( ret = g_dispatcher.dispatch(type, request_id, plain_request, result_data, error_msg) ) != 0 ) {
        ERROR( "Request ID: %s, Failed to dispatch! ret: 0x%x, error_msg: %s", request_id, ret, error_msg.c_str() );
        plain_reply = error_msg;    // Output error message for reply
    } 
    else {
        plain_reply = result_data;  // Output result data for reply
    }

    // Allocate a block of untrusted memory to pass the result from enclave to app
    reply_len = plain_reply.length();
    outside_buff = malloc_outside( reply_len );
    if ( !outside_buff ) {
        ERROR( "Request ID: %s, Failed to call malloc_outside()! size: %d", request_id, reply_len );
        ret = TEE_ERROR_MALLOC_OUTSIDE;
        goto _exit;
    }
    memcpy( outside_buff, plain_reply.c_str(), reply_len );
    *output = (char*)outside_buff;
    *output_len = reply_len;

    FUNC_END;

_exit:
    return ret;
}

// Free memory in the enclave
void ecall_free()
{
    FUNC_BEGIN;

    std::lock_guard<std::mutex> lock( g_list_mutex );

    for ( auto it = g_keyContext_list.begin(); 
          it != g_keyContext_list.end(); 
        ) {
        delete it->second;
        it = g_keyContext_list.erase( it );
    }

    FUNC_END;
}

// Return enclave id in hexadecimal
int ecall_get_enclave_id(
    char **output, 
    uint64_t *output_len)
{
    int id_len = 0;
    std::string mrenclave_id;
    sgx_target_info_t self_info;
    uint8_t* outside_buff = nullptr;

    FUNC_BEGIN;

    sgx_self_target(&self_info);
    mrenclave_id = safeheron::encode::hex::EncodeToHex( self_info.mr_enclave.m, SGX_HASH_SIZE );

    // Write memory of output reply
    id_len = mrenclave_id.length();
    if ( !(outside_buff = malloc_outside( id_len + 1 )) ) {
        return TEE_ERROR_MALLOC_OUTSIDE;
    }
    memcpy( outside_buff, mrenclave_id.c_str(), id_len );
    *output = (char*)outside_buff;
    *output_len = id_len;

    FUNC_END;

    return TEE_OK;
}

// Generate the enclave quote
int ecall_create_report(
    const char* request_id, 
    const char* pubkey_list_hash,
    const sgx_target_info_t* p_qe3_target, 
    sgx_report_t* p_report)
{
    int ret = 0;
    std::string key_meta_hash;
    std::string pubkey_and_meta;
    std::string total_hash_hex;
    std::string total_hash;
    sgx_status_t sgx_error;
    sgx_report_data_t report_data = { 0 };

    FUNC_BEGIN;

    // Get the key meta hash string from key context list
    std::lock_guard<std::mutex> lock( g_list_mutex );
    if ( !g_keyContext_list.count( pubkey_list_hash ) ) {
        ERROR( "Request ID: %s, Input pubkey list hash is not exist! pubkey_list_hash: %s", request_id, pubkey_list_hash );
        return TEE_ERROR_PUBLIST_KEY_HASH;
    }
    key_meta_hash = g_keyContext_list.at( pubkey_list_hash )->key_meta_hash;
    g_list_mutex.unlock();

    // Combine the hash of the public key list and the hash of the key meta
    pubkey_and_meta = safeheron::encode::hex::DecodeFromHex( pubkey_list_hash );
    pubkey_and_meta += safeheron::encode::hex::DecodeFromHex( key_meta_hash );

    // Hash the combined string
    if ( !sha256_hash( pubkey_and_meta, total_hash_hex ) ) {
        ERROR( "Request ID: %s, get_sha256_hash() failed with pubkey_and_meta!", request_id );
        ERROR( "Request ID: %s, pubkey_list_hash: %s", request_id, pubkey_list_hash );
        ERROR( "Request ID: %s, key_meta_hash: %s", request_id, key_meta_hash.c_str() );
        return TEE_ERROR_CALC_HASH_FAILED;
    }

    // Convert hex string to bytes, and put it into report's user data
    total_hash = safeheron::encode::hex::DecodeFromHex( total_hash_hex );
    memcpy( report_data.d, total_hash.c_str(), 32 );

    // Generate report
    if ( (ret = sgx_create_report( p_qe3_target, &report_data, p_report )) != SGX_SUCCESS ) {
        ERROR( "Request ID: %s, sgx_create_report() failed! ret: %d", key_meta_hash.c_str(), ret );
        return ret;
    }

    FUNC_END;

    return ret;
}

/**
 * Traverse g_keyContext_list and clear the item if:
 * 1. its status is an error
 * 2. its current_time - finished_time > duration
 */
void clear_context(int duration )
{
    long current_time = 0;

    FUNC_BEGIN;

    current_time = get_system_time();

    std::lock_guard<std::mutex> lock( g_list_mutex );

    // Traverse the list for checking status and duration
    for ( auto it = g_keyContext_list.begin();
          it != g_keyContext_list.end(); ) {
        // Free the item if its status is an error
        if ( it->second->key_status == eKeyStatus_Error ) {
            delete it->second;
            it = g_keyContext_list.erase( it );
        }
        // Free item if it is finished and the alive time is bigger than duration
        else if ( (it->second->key_status == eKeyStatus_Finished) &&
                  ((current_time - it->second->finished_time) > duration) ) {
                delete it->second;
                it = g_keyContext_list.erase( it );
        }
        else {
            it++;
        }
    }

    FUNC_END;
}