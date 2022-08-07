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

/**
 * HTTP request paths defintion
 */
#define HTTP_REQ_GENERATE_KEYSHARE  "/arweave/create_key_share"
#define HTTP_REQ_QUERY_KEYSTATE     "/arweave/query_key_shard_state"

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
     * @param req_path : the request path name string
     * @param req_body : a JSON string for request body
     * @param resp_body : return the response body string, in JSON
     * @return int : return 0 if success, otherwise return an error code.
     */
    int process( const std::string & req_path, const std::string & req_body, std::string & resp_body );
    
public:
    static std::string generateRequestID( const std::string & prefix );
    static std::string getMessageReply( const std::string & request_id, bool success, const std::string & message );
    static void destoryThreadPool();

private:
    int generateKeyShare( const std::string & req_body, std::string & resp_body );
    int queryKeyShareState( const std::string & req_body, std::string & resp_body );
private:
    static std::list<ThreadTask*>  s_thread_pool;
};
#endif //_MSG_HANDLER_H_