/**
 * @file msg_handler.h
 * @brief : msg_handler.h contains HTTP request handler which processes the requests from listen_svr.
 *          Each request will be passed into GenerateKeyShard() or QueryKeyShardState() by process().
 *          For generation requests, Each request will be served as a thread task to ecall into the enclave.
 *          For query requests, Each request will ecall into the enclave directly.
 *
 * @copyright Copyright (c) 2022
 */
#ifndef _MSG_HANDLER_H_
#define _MSG_HANDLER_H_

#include "thread_task.h"
#include <string>
#include <list>
#include <mutex>

/**
 * Max number of threads in s_thread_pool.
 * s_thread_pool is a thread pool to allocate thread resources.
 */
#define MAX_THREAD_TASK_COUNT       100

class msg_handler
{
public:
    msg_handler();
    virtual ~msg_handler();
public:
    /**
     * @brief : The HTTP request processing function.
     * 
     * @param[in] req_id : the request id corresponding to each HTTP request.
     * @param[in] req_path : the request path.
     * @param[in] req_body : the request body in JSON string format.
     * @param[out] resp_body : the response body in JSON string format.
     * @return int : return 0 if success, otherwise return an error code.
     */
    int process( const std::string & req_id, const std::string & req_path, const std::string & req_body, std::string & resp_body );
    
public:
    static std::string GetMessageReply( bool success, int code, const std::string & message );
    static int GenerateEnclaveReport(const std::string & request_id, const std::string& pubkey_list_hash, std::string & report );
    static void DestroyThreadPool();

private:
    /**
     * @brief : Generate key shards in TEE. The generation request is handled asynchronously.
     *          In this function, it creates a child thread to process the key shard generation and replies immediately.
     *          The kye shards generation result will be sent to webhook address once the request processing is finished.
     *
     * @param[in] req_id : the request id corresponding to each HTTP request.
     * @param[in] req_body : the request body in JSON string format.
     * @param[out] resp_body : the response body in JSON string format.
     * @return int : return 0 if success, otherwise return an error code.
     */
    int GenerateKeyShard(const std::string & req_id, const std::string & req_body, std::string & resp_body );

    /**
     * @brief : Query key shards' status in TEE. The query request is handled synchronously
     *
     * @param[in] req_id : the request id corresponding to each HTTP request.
     * @param[in] req_body : the request body in JSON string format.
     * @param[out] resp_body : the response body in JSON string format.
     * @return int : return 0 if success, otherwise return an error code.
     */
    int QueryKeyShardState(const std::string & req_id, const std::string & req_body, std::string & resp_body );
private:
    static std::mutex   s_thread_lock;
    static std::list<ThreadTask*>  s_thread_pool;
};
#endif //_MSG_HANDLER_H_