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

/**
 *  Return the task's type
 */
int QueryTask::get_task_type( )
{
    return eTaskType_Query;
}
/**
 * Introduce:this task's process function 
 * Params:request[IN]:request data string, in plian
 *        reply[OUT]:reply data string
 *        error_msg[OUT]:return an error message string while failed
 * Return:TEE_OK if successful, otherwise return an error code
*/
int QueryTask::execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg )
{
    int ret = 0;

    return ret;
}