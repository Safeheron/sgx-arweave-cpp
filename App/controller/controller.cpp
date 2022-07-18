#include "controller.h"
#include <safeheron/crypto-hash/sha256.h>
#include <safeheron/crypto-bn/bn.h>
#include <safeheron/crypto-bn/rand.h>

using safeheron::bignum::BN;

extern sgx_enclave_id_t global_eid;
extern thrdpool_t* pool;
thread_local std::string request_id;

void HandleCreateKeyRequest(http_request& message) {
    int k = 0;
    int l = 0;
    int keylen = 0;
    int ret = 0;
    sgx_status_t sgx_status;
    vector<string> pubkey_list;
    string pubkey_list_hash;

    /** Verify inputs data, add public keys into a vector if the inputs are valid */
    bool ok = VerifyData(message, pubkey_list, k, l, keylen);
    if (!ok) {
        MessageReply(message, false, "invalid input, please check your data.");
        return ;
    }

    /** Create a unique ID for each request */
    GetUniqueID(pubkey_list, pubkey_list_hash);

    /** Enter enclave to check if the request is repeat */
    ecall_if_repeat(global_eid, &ret, pubkey_list_hash.c_str());
    if (ret) {
        MessageReply(message, false, "repeat input, please try again later.");
        return;
    }

    /** Enter enclave to check if the request is busy */
    ecall_is_server_available(global_eid, &ret);
    if (!ret) {
        MessageReply(message, false, "Server is busy now, please try again later.");
        return;
    }

    /** Add tasks to the thread pool */
    ok = AddTaskToPool(pubkey_list, k, l, keylen);
    if (!ok) {
        MessageReply(message, false, "The program is closing.");
        return;
    }

    /** The reply of the successful creation */
    MessageReply(message, true, "Request has been accepted.");
}


void HandleQueryKeyRequest(http_request& message) {
    bool ok = true;
    string reply_message;
    string pubkey_list_hash;

    auto body_json = message.extract_json().get();

    if (body_json.has_field(PUBKEY_LIST_HASH) && body_json.at(PUBKEY_LIST_HASH).is_string()) {
        pubkey_list_hash = body_json.at(PUBKEY_LIST_HASH).as_string();
    } else {
        MessageReply(message, false, "invalid input, please check your data.");
        return;
    }

    /** Call the EnclaveAccess interface to enter enclave */
    ok = EnclaveAccess::QueryKeyShareStatus(pubkey_list_hash, reply_message);
    if (!ok) return ;

    /** Parse a string and construct a JSON value to reply */
    web::json::value obj = json::value::parse(reply_message);
    message.reply(status_codes::OK, obj.serialize());

}

void MessageReply(http_request& message, bool success, const string& retrun_message) {
    json::value obj = json::value::object(true);
    obj["success"] = json::value(success);
    obj["message"] = json::value(retrun_message);
    if (success)
        message.reply(status_codes::OK, obj.serialize());
    else
        message.reply(status_codes::BadRequest, obj.serialize());
}


void GetUniqueID(vector<string>& pubkey_list, string& pubkey_list_hash) {
    string rand_number;
    string pubkey_list_str;

    /** Sort the public key list */
    std::sort(pubkey_list.begin(), pubkey_list.end());
    for (const auto &pubkey : pubkey_list) {
        pubkey_list_str += pubkey;
    }

    /** Get the hash of public key list */
    get_sha256_hash(pubkey_list_str, pubkey_list_hash);

    BN rand_bn = safeheron::rand::RandomBN(16);
    rand_bn.ToHexStr(rand_number);
    request_id = pubkey_list_hash + rand_number;
}

bool AddTaskToPool(vector<string>& pubkey_list, int k, int l, int key_length) {

    struct taskpar* thread_task_param = new taskpar;

    thread_task_param->pubkey_list = pubkey_list;
    thread_task_param->k = k;
    thread_task_param->l = l;
    thread_task_param->key_length = key_length;
    thread_task_param->request_id = request_id;

    /** Creat a thread task for creation request */
    struct thrdpool_task thread_task;
    thread_task.routine = &RoutineThread;
    thread_task.context = (void*)thread_task_param;

    /** Add the task to thread pool, return -1 on failure */
    int ret = thrdpool_schedule(&thread_task, pool);
    if (-1 == ret) {
        ERROR("Request ID: %s, add thread_task to pool failed.", request_id.c_str());
        delete thread_task_param;
        return false;
    }

    return true;
}
