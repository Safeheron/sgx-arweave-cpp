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

int QueryTask::get_task_type( )
{
    return eTaskType_Query;
}

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

    // Check if request_id is null
    if (request.length() == 0) {
        error_msg = format_msg( "Request ID: %s, request is null!", request_id.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_INVALID_PARAMETER;
    }
    input_pubkey_hash = request;

    std::lock_guard<std::mutex> lock( g_list_mutex );

    // Check if there is an alive key context corresponding to pubkey list hash in g_keyContext_list.
    // Return success = false if there is not.
    if ( !g_keyContext_list.count( input_pubkey_hash ) ) {
        error_msg = format_msg( "Request ID: %s, input_pubkey_hash dose not exist! pubkey_list_hash: %s",
                                request_id.c_str(), input_pubkey_hash.c_str() );
        ERROR( "%s", error_msg.c_str() );
        ret = TEE_ERROR_PUBLIST_KEY_HASH;
        root["success"] = false;
        goto _exit;
    }
    if ( !(context = g_keyContext_list.at( input_pubkey_hash )) ) {
        error_msg = format_msg( "Request ID: %s, input_pubkey_hash exist, but the context is null! input_pubkey_hash: %s",
                                request_id.c_str(), input_pubkey_hash.c_str() );
        ERROR( "%s", error_msg.c_str() );
        ret = TEE_ERROR_INTERNAL_ERROR;
        root["success"] = false;
        goto _exit;
    }

    // Return success = true
    root["success"] = true;
    root["status_code"] = context->key_status;
    root["status_text"] = get_status_text( context->key_status );
    root["k"] = context->k;
    root["l"] = context->l;
    if ( eKeyStatus_Unknown == context->key_status || 
         eKeyStatus_Error == context->key_status ) {
        root["alive_time_seconds"] = 0;
    }
    else {
        root["alive_time_seconds"] = (context->finished_time == 0) ? 
                                    int(get_system_time() - context->start_time) :
                                    int(context->finished_time - context->start_time);
    }

    FUNC_END;

_exit:
    reply = JSON::Root::write( root );

    return ret;
}

std::string QueryTask::get_status_text( int status )
{
    switch ( status ) {
    case eKeyStatus_Generating:
        return "Generating";
    case eKeyStatus_Reporting:
        return "Reporting";
    case eKeyStatus_Callback:
        return "Callbacking";
    case eKeyStatus_Finished:
        return "Finished";
    case eKeyStatus_Error:
        return "Error";
    case eKeyStatus_Unknown:
    default:
        return "Unknown";
    } 
}