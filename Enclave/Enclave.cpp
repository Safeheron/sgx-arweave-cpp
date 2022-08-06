#include "Enclave.h"
#include "Enclave_t.h"
//#include "KeyShardCreation.h"
#include "common/tee_util.h"
#include "common/tee_error.h"
#include "shell/Dispatcher.h"
#include "tasks/TaskConstant.h"
#include "tasks/GenerateTask.h"
#include "tasks/QueryTask.h"

//std::mutex enclave_mutex;
//thread_local char request_id[100];
//std::map<std::string, KeyContext*> current_alive_key_context;
//
Dispatcher g_dispatcher;
std::mutex g_list_mutex;
std::map<std::string, KeyShardContext*> g_keyContext_list;


/**
 *  To initliaze the enclave
 */
int ecall_init()
{
    int ret = TEE_OK;

    // Register TEE tasks
    g_dispatcher.register_task( new GenerateTask() );
    g_dispatcher.register_task( new QueryTask() );

    return ret;
}

/**
 *  To run a TEE task in enclave
 */
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
    std::string s_error_msg;
    uint8_t* outside_buff = nullptr;

    plain_request.assign( input_data, data_len );

    // Dispatch request
    if ( ( ret = g_dispatcher.dispatch( type, request_id, plain_request, plain_reply, s_error_msg ) ) != 0 ){
        ERROR( "Request ID: %s, Failed to dispatch request: %s", request_id, s_error_msg.c_str() );
        goto _exit;
    }

    // Write memory of output reply
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

_exit:

    return ret;
}

/**
 *  To deallocate memory in the enclave
 */
void ecall_free()
{
    std::lock_guard<std::mutex> lock( g_list_mutex );

    for ( auto it = g_keyContext_list.begin(); 
          it != g_keyContext_list.end(); 
        ) {
        delete it->second;
        it = g_keyContext_list.erase( it );
    }
}

/**
 *  To return this enclave id in hex string
 */
int ecall_get_enclave_id(
    char **output, 
    uint64_t *output_len)
{
    int id_len = 0;
    std::string mrenclave_id;
    sgx_target_info_t self_info;
    uint8_t* outside_buff = nullptr;

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

    return TEE_OK;
}

/**
 *  To generate the enclave quote
 */
int ecall_create_report(
    const char* request_id, 
    const char* pubkey_list_hash,
    const sgx_target_info_t* p_qe3_target, 
    sgx_report_t* p_report)
{
    std::string key_meta_hash;
    std::string pubkey_and_meta;
    std::string total_hash_hex;
    std::string total_hash;
    sgx_status_t sgx_error;
    sgx_report_data_t report_data = { 0 };

    // get the key meta hash string from context list
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

    // convert hex string to bytes, and put it into report data
    total_hash = safeheron::encode::hex::DecodeFromHex( total_hash_hex );
    memcpy( report_data.d, total_hash.c_str(), 32 );

    // Generate the report of this enclave
    return sgx_create_report( p_qe3_target, &report_data, p_report );
}
#if 0
void ecall_print_enclave_id() 
{
    std::string mrenclave_id;
    sgx_target_info_t self_info;

    sgx_self_target(&self_info);
    mrenclave_id = safeheron::encode::hex::EncodeToHex( self_info.mr_enclave.m, SGX_HASH_SIZE );
    printf("MRENCLAVE: %s\n", mrenclave_id.c_str());
}


/**
 * @brief 
 * 
 * @param request_id 
 * @param pubkey_list 
 * @param k 
 * @param l 
 * @param key_bits 
 * @param output 
 * @return int 
 */
int ecall_create_keyshard(
    const char* in_request_id, 
    const char* pubkey_list, 
    int k, int l, int key_length, 
    char** output) 
{
    bool ok = true;
    std::string first_result;

    EnclaveCreateKeyShard create_key_shard(k, l, key_length);

    memcpy(request_id, in_request_id, strlen(in_request_id));

    ok = create_key_shard.GetPubkeyHash(pubkey_list);
    if (!ok) return ERROR_PUBLIST_KEY_HASH;

    ok = create_key_shard.CreateKeyContext();
    if (!ok) return ERROR_CREATE_KEYCONTEXT;

    ok = create_key_shard.GenerateRSAKey();
    if (!ok) {
        create_key_shard.ChangeKeyStatus(STATUS_ERROR);
        return ERROR_GENERATE_KEYSHARDS;
    }

    ok = create_key_shard.FinalDataReturn(first_result);
    if (!ok) {
        create_key_shard.ChangeKeyStatus(STATUS_ERROR);
        return ERROR_FINAL_RETURN;
    }

    *output = (char*)malloc_outside(first_result.size() + 1);
    if (!*output) {
        ERROR("Request ID: %s, malloc_outside failed", request_id);
        create_key_shard.ChangeKeyStatus(STATUS_ERROR);
        return ERROR_MALLOC_OUTSIDE;
    }
    memcpy(*output, first_result.c_str(), first_result.size());

    create_key_shard.ChangeKeyStatus(STATUS_FINISHED);

    return 0;
}

int ecall_query_keyshard(const char* pubkey_list_hash, char** output) {
    bool ok;
    long *time_now = nullptr;
    std::string query_result;
    CJsonObject cjson_query;

    ok = cjson_query.CreateObject();
    if (!ok) return -1;

    std::lock_guard<std::mutex> lock(enclave_mutex);
    /** Query request failed */
    if (!current_alive_key_context.count(pubkey_list_hash) || current_alive_key_context.at(pubkey_list_hash)->key_status != STATUS_RUNNING) {

        ok = cjson_query.Add("success", false);
        if (!ok) return -1;

    } else {
        /** Get the current system time */
        ocall_get_system_time(&time_now);
        long runtime = *time_now - current_alive_key_context.at(pubkey_list_hash)->start_time;
        ocall_free_long(time_now);
        ok = cjson_query.Add("success", true);
        if (!ok) return -1;

        ok = cjson_query.Add("alive_time_seconds", (int)runtime);
        if (!ok) return -1;

        ok = cjson_query.Add("k", current_alive_key_context.at(pubkey_list_hash)->k);
        if (!ok) return -1;

        ok = cjson_query.Add("l", current_alive_key_context.at(pubkey_list_hash)->l);
        if (!ok) return -1;
    }

    /** Serialize the Json object to a string */
    ok = cjson_query.ToString(query_result);
    if (!ok) return -1;

    /** Apply for the memory outside the enclave */
    *output = (char*)malloc_outside(query_result.size() + 1);
    if (!*output) {
        ERROR("Request ID: %s, malloc_outside failed", request_id);
        return false;
    }
    memcpy(*output, query_result.c_str(), query_result.size());


    return 0;
}

int ecall_is_server_available() {

    /** The sever is available if the map size is smaller than 1000 */
    if (current_alive_key_context.size() < 1000) {
        return 1;
    }

    return 0;
}

void ecall_clear_map() {

    if (current_alive_key_context.empty()) return;
    
    /** Iterate each elements in the map,
     * If the status of the elements is STATUS_FINISHED, Convert is to STATUS_DESTROY
     * If the status of the elements is STATUS_DESTROY, Erase it from the map
     * If the status of the element is STATUS_ERROR, Erase it from the map
     */
    std::lock_guard<std::mutex> lock(enclave_mutex);
    for (auto it = current_alive_key_context.begin(); it != current_alive_key_context.end();) {

        if (it->second->key_status == STATUS_FINISHED) {

            it->second->key_status = STATUS_DESTROY;
            ++it;

        } else if(it->second->key_status == STATUS_DESTROY) {

            delete it->second;
            it = current_alive_key_context.erase(it);

        } else if(it->second->key_status == STATUS_ERROR) {

            delete it->second;
            it = current_alive_key_context.erase(it);

        } else {
            ++it;
        }
    }
    INFO_OUTPUT_CONSOLE("The number of members in map is %lu", current_alive_key_context.size());
}

int ecall_if_repeat(const char* pubkey_list) {

    if (current_alive_key_context.count(pubkey_list)) {
        return 1;
    }
    return 0;
}
#endif //0