#include "Enclave.h"
#include "Enclave_t.h"
#include "KeyShardCreation.h"

std::mutex enclave_mutex;
thread_local char request_id[100];
std::map<std::string, KeyContext*> current_alive_key_context;


int ecall_create_keyshard(const char* in_request_id, const char* pubkey_list, int k, int l, int key_length, char** output) {
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

uint32_t enclave_create_report(const sgx_target_info_t* p_qe3_target, sgx_report_t* p_report, const char* pubkey_list_hash)
{
    bool ok = true;
    sgx_report_data_t report_data = { 0 };
    std::string total_hash_hex;
    std::string total_hash;

    if (!current_alive_key_context.count(pubkey_list_hash)) return -1;

    /** Combine the hash of the public key list and the hash of the key meta */
    std::string pub_and_key_meta = safeheron::encode::hex::DecodeFromHex(pubkey_list_hash);
    pub_and_key_meta += safeheron::encode::hex::DecodeFromHex(current_alive_key_context.at(pubkey_list_hash)->key_meta_hash);

    /** Hash the combined string */
    ok = get_sha256_hash(pub_and_key_meta, total_hash_hex);
    if (!ok) return -1;

    /** Hex format to bytes */
    total_hash = safeheron::encode::hex::DecodeFromHex(total_hash_hex);

    /** Put the hash of the combined string into report data */
    memcpy(report_data.d, total_hash.c_str(), 32);

    /** Generate the report */
    sgx_status_t  sgx_error = sgx_create_report(p_qe3_target, &report_data, p_report);

    return sgx_error;
}

void ecall_print_enclave_id() {

    sgx_target_info_t self_info;
    sgx_self_target(&self_info);
    std::string g_mrenclave = s_printf_hex(self_info.mr_enclave.m, SGX_HASH_SIZE);
    printf("MRENCLAVE: %s\n", g_mrenclave.c_str());
}

void ecall_free() {

    for (auto it = current_alive_key_context.begin(); it != current_alive_key_context.end();) {
        std::lock_guard<std::mutex> lock(enclave_mutex);
        delete it->second;
        it = current_alive_key_context.erase(it);
    }
}

