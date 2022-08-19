/**
 * @file msg_handler.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _MSG_HANDLER_H_
#define _MSG_HANDLER_H_

#include "thread_task.h"
#include <string>
#include <list>
#include <mutex>

/**
 * Max thread count in s_thread_pool
 * s_thread_pool is a thread pool to save
 * all asynchronous task threads
 */
#define MAX_TASKTHREAD_COUNT       100

/**
 * Class for HTTP request message handler
 * 
 */
class msg_handler
{
public:
    msg_handler();
    virtual ~msg_handler();
public:
    /**
     * @brief : The HTTP message handle function
     * 
     * @param req_id : the request id for log lines
     * @param req_path : the request path name string
     * @param req_body : a JSON string for request body
     * @param resp_body : return the response body string, in JSON
     * @return int : return 0 if success, otherwise return an error code.
     */
    int process( const std::string & req_id, const std::string & req_path, const std::string & req_body, std::string & resp_body );
    
public:
    static std::string GetMessageReply( bool success, int code, const std::string & message );
    static int CreateEnclaveReport( const std::string & request_id, const std::string& pubkey_list_hash, std::string & report );
    static void DestoryThreadPool();

private:
    int GenerateKeyShare( const std::string & req_id, const std::string & req_body, std::string & resp_body );
    int QueryKeyShareState( const std::string & req_id, const std::string & req_body, std::string & resp_body );
private:
    static std::mutex   s_thread_lock;
    static std::list<ThreadTask*>  s_thread_pool;
};
#endif //_MSG_HANDLER_H_