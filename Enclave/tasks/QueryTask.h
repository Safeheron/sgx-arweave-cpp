/**
 * @file QueryTask.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../shell/Dispatcher.h"

class QueryTask: public Task
{
public:
    virtual int execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg );
    virtual int get_task_type( );

private:
    std::string get_status_text( int status );
};