#include "thread_task.h"
#include "Enclave_u.h"
#include "CallBack.h"


extern sgx_enclave_id_t global_eid;
extern bool thread_clear_map_quit;

void RoutineThread(void* context)
{
    bool ok = true;
    string final_result;
    string pubkey_list_str;
    CJsonObject pubkey_array;
    CJsonObject cjson_public_list;
    taskpar* task_parameter = (struct taskpar*)context;

    /** Convert the public key to Json format from the public key list */
    ok = cjson_public_list.CreateObject();
    if (!ok) goto exit;

    ok = pubkey_array.CreateArray();
    if (!ok) goto exit;

    for (const auto& value : task_parameter->pubkey_list) {
        ok = pubkey_array.Add(value);
        if (!ok) goto exit;
    }

    ok = cjson_public_list.Add("pubkey_list", pubkey_array);
    if (!ok) goto exit;

    /** Serialize the Json object to a string */
    ok = cjson_public_list.ToString(pubkey_list_str);
    if (!ok) goto exit;

    INFO("Request ID: %s has been accepted to create key shards.\npubkey list string: %s\nk: %d, l: %d, key_length: %d", task_parameter->request_id.c_str(),
                         pubkey_list_str.c_str(), task_parameter->k, task_parameter->l, task_parameter->key_length);


    /** Enter the enclave to create key shards and output the result in Json format string */
    ok = EnclaveAccess::CreateKeyShard(task_parameter->request_id, pubkey_list_str, task_parameter->k,
                                       task_parameter->l, task_parameter->key_length, final_result);

    cout << "result:" << final_result << endl;
     /** Webhook service */
    if (!ok) {
        ERROR("Request ID: %s create key shard failed.", task_parameter->request_id.c_str());
        CallBackFunction call_back(task_parameter->request_id, false, "Your request failed, please try again.");

    } else {
        CallBackFunction call_back(task_parameter->request_id, true, final_result);
    }

    delete task_parameter;
    return ;
exit:
    ERROR("Request ID: %s CJson operation is failed.", task_parameter->request_id.c_str());
    delete task_parameter;
}

void ClearMapCache(void* context) {
    while(true) {
        if (thread_clear_map_quit) return;
        sleep(1);
        ecall_clear_map(global_eid);
    }
}

void PendingTask(const struct thrdpool_task *task) {

    taskpar* task_parameter = (taskpar*)task->context;
    CallBackFunction call_back(task_parameter->request_id, false, "Server is closed now, please try again later.");

    delete task_parameter;
}
