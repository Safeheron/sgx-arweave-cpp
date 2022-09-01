#include "thread_task.h"

// Thread function
void* task_thread_func( void *ptr )
{
    ThreadTask* task = (ThreadTask*)ptr;
    if ( task ) {
        task->thread_func();
    }
    return 0;
}

ThreadTask::ThreadTask()
 : stopped_( false )
 , task_func_( nullptr )
 , task_param_( nullptr )
{

}
ThreadTask::ThreadTask( task_handler task_func, void* task_param )
 : stopped_( false )
 , task_func_( task_func )
 , task_param_( task_param )
{

}
ThreadTask::~ThreadTask()
{
    stop();
}

int ThreadTask::start()
{
    stopped_ = false;
    pthread_cond_init( &thread_cond_, nullptr );
    pthread_mutex_init( &thread_mutex_, nullptr );
    return pthread_create( &thread_, nullptr, task_thread_func, this );
}

int ThreadTask::stop()
{
    pthread_join( thread_, nullptr );
    pthread_cond_destroy( &thread_cond_ );
    pthread_mutex_destroy( &thread_mutex_ );
    return 0;
}

bool ThreadTask::is_stopped()
{
    return stopped_;
}

// Call specified task process function by callback in the thread function
void ThreadTask::thread_func()
{
    int ret = 0;

    // call the specified task process function
    if ( task_func_ ) {
        ret = task_func_( task_param_ );
    }

    // signal waiting threads that this thread is about to terminate
    pthread_mutex_lock( &thread_mutex_ );
    pthread_cond_broadcast( &thread_cond_ );
    pthread_mutex_unlock( &thread_mutex_ );

    stopped_ = true;
}