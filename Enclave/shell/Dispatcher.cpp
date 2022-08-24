#include "Dispatcher.h"
#include "common/tee_error.h"

int Dispatcher::dispatch( uint32_t task_type, const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg )
{
    for( int i = 0; i < m_vTask.size(); i ++ ){
        if ( m_vTask[i]->get_task_type() == task_type ){
            return m_vTask[i]->execute( request_id, request, reply, error_msg );
        }
    }
    return TEE_ERROR_DISPATCH_REQUEST_FAILED;
}

void Dispatcher::register_task( Task * t )
{
    m_vTask.push_back( t );
}
