/**
 * @file QueryTask.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "QueryTask.h"
#include "TaskConstant.h"
#include "common/tee_util.h"
#include "common/tee_error.h"
#include "common/log_t.h"
#include "json/json.h"
#include <mutex>
#include <map>

extern std::mutex g_list_mutex;
extern std::map<std::string, KeyShardContext*> g_keyContext_list;

/**
 *  Return the task's type
 */
int QueryTask::get_task_type( )
{
    return eTaskType_Query;
}
/**
 * Introduce: this task's process function 
 * Params:request_id[IN]:unqine ID for this requesting, it will be output to log for debug
 *        request[IN]:request data string, in plian
 *        reply[OUT]:reply data string
 *        error_msg[OUT]:return an error message string while failed
 * Return:TEE_OK if successful, otherwise return an error code
*/
int QueryTask::execute( 
    const std::string & request_id, 
    const std::string & request, 
    std::string & reply, 
    std::string & error_msg )
{
    int ret = 0;
    std::string input_pubkey_hash;
    KeyShardContext* context = nullptr;
    JSON::Root root;
    
    FUNC_BEGIN;

    // checking
    request_id_ = request_id;
    if (request.length() == 0) {
        error_msg = format_msg( "Request ID: %s, request is null!", request_id_.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_INVALID_PARAMETER;
    }
    input_pubkey_hash = request;

    std::lock_guard<std::mutex> lock( g_list_mutex );

    // find context by pubkey list hasd, and return success = false if it's not exist.
    if ( !g_keyContext_list.count( input_pubkey_hash ) ) {
        error_msg = format_msg( "Request ID: %s, Input pubkey list hash is not exist! pubkey_list_hash: %s", 
            request_id_.c_str(), input_pubkey_hash.c_str() );
        ERROR( "%s", error_msg.c_str() );
        ret = TEE_ERROR_PUBLIST_KEY_HASH;
        root["success"] = false;
        goto _exit;
    }
    if ( !(context = g_keyContext_list.at( input_pubkey_hash )) ) {
        error_msg = format_msg( "Request ID: %s, input_pubkey_hash is exist, but the context is null! input_pubkey_hash: %s", 
            request_id_.c_str(), input_pubkey_hash.c_str() );
        ERROR( "%s", error_msg.c_str() );
        ret = TEE_ERROR_INTERNAL_ERROR;
        root["success"] = false;
        goto _exit;
    }

    // return success = true
    root["success"] = true;
    root["status_code"] = context->key_status;
    root["status_text"] = get_status_text( context->key_status );
    root["k"] = context->k;
    root["l"] = context->l;
    root["alive_time_seconds"] = (context->finished_time == 0) ? 
                                 int(get_system_time() - context->start_time) :
                                 int(context->finished_time - context->start_time);

    FUNC_END;

_exit:
    reply = JSON::Root::write( root );

    return ret;
}

/**
 * @brief Return key share generation's status text
 * 
 * @param status : status code, one of eKeyStatus
 * @return std::string 
 */
std::string QueryTask::get_status_text( int status )
{
    switch ( status ) {
    case eKeyStatus_Generating:
        return "Generating";
    case eKeyStatus_Finished:
        return "Finished";
    case eKeyStatus_Error:
        return "Error";
    case eKeyStatus_Unknown:
    default:
        return "Unknown";
    } 
}