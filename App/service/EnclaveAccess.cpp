#include "EnclaveAccess.h"
#include "../common/CJsonObject.h"

extern sgx_enclave_id_t global_eid;

bool EnclaveAccess::CreateKeyShard(const string& request_id, const string& pubkey_list, int k, int l, int key_length, string& final_result) {
    bool ok;
    int ret = 0;
    string tee_report;
    string pubkey_list_hash;
    CJsonObject cjson_result;
    char* first_result = nullptr;
    sgx_status_t sgx_status;

    /** Enter enclave to create key shards */
    sgx_status = ecall_create_keyshard(global_eid, &ret, request_id.c_str(), pubkey_list.c_str(), k, l, key_length, &first_result);
    if (0 != ret || sgx_status != SGX_SUCCESS) {
        ERROR("Request ID: %s, ecall_create_keyshard failed, and the return value is %d.", request_id.c_str(), ret);
        return false;
    }

    /** Parse the string and construct a Json object */
    ok = cjson_result.Parse(first_result);
    if (!ok) goto exit;

    /** Get the hash of the public key list from the Json object */
    ok = cjson_result.GetValueString("pubkey_list_hash", pubkey_list_hash);
    if (!ok) goto exit;

    /** Generate the remote attestation report */
    ret = QuoteGeneration(pubkey_list_hash, tee_report);
    if (-1 == ret) {
        ERROR("Request ID: %s, generate remote attestation report failed.", request_id.c_str());
        return false;
    }

    INFO_OUTPUT_CONSOLE("Request ID: %s, generate remote attestation report successfully.", request_id.c_str());

    /** Add remote attestation report to Json object */
    ok = cjson_result.Add("tee_report", tee_report);
    if (!ok) goto exit;

    /** Serialize Json object to a String */
    ok = cjson_result.ToString(final_result);
    if (!ok) goto exit;

    INFO_OUTPUT_CONSOLE("Request ID: %s, second time packing data successfully.", request_id.c_str());

    if (first_result)
        free(first_result);
    return true;

exit:
    ERROR("Request ID: %s CJson operation is failed.", request_id.c_str());
    return false;
}


bool EnclaveAccess::QueryKeyShareStatus(const string& pubkey_list_hash, string& reply_message) {
    int ret = 0;
    sgx_status_t sgx_status;
    char* query_result = nullptr;

    /** Enter enclave to query the status of key shards generation */
    sgx_status = ecall_query_keyshard(global_eid, &ret, pubkey_list_hash.c_str(), &query_result);
    if (0 != ret || sgx_status != SGX_SUCCESS) {
        ERROR("pubkey list hash: %s, ecall_query_keyshard failed.", pubkey_list_hash.c_str());
        return false;
    }

    reply_message = query_result;

    if (query_result)
        free(query_result);

    return true;
}



