/**
 * @file QueryTask.h
 * @brief : QueryTask.h contains the execution of query tasks. The query request result is returned directly after
 *          checking if there is an alive key context corresponding to the public key list hash.
 *
 * @copyright Copyright (c) 2022
 * 
 */
#include "../shell/Dispatcher.h"

class QueryTask: public Task
{
public:
    /**
     * @brief : The execution of query tasks.
     * @param request_id[in] : The unique ID of each request.
     * @param request[in] : The request body, JSON string.
     * @param reply[out] : The key shards generation result, JSON string
     * @param error_msg[out] : An error message is returned when the execution is failed.
     * @return int : return 0 if success, otherwise return an error code.
     */
    virtual int execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg );

    /**
    * @brief : Get the current task type.
    * @return int : Return the task type number defined in TaskConstant.h
    */
    virtual int get_task_type( );

private:
   /**
    * @brief Get key shard generation status.
    * @param status : status code, defined in TaskConstant.h.
    * @return std::string .
    */
    std::string get_status_text( int status );
};