/**
 * @file thread_task.h
 *
 * @copyright Copyright (c) 2022
 */
#ifndef _THREAD_TASK_H_
#define _THREAD_TASK_H_

#include "keyshard_param.h"
#include <pthread.h>

// Define callback function task pointer
typedef int (*task_handler) ( void* );

class ThreadTask
{
public:
    ThreadTask();
    ThreadTask( task_handler task_func, void* task_param );
    virtual ~ThreadTask();
public:
    int start();
    int stop();
    bool is_stopped();
    void thread_func();
private:
    bool            stopped_;
    pthread_t       thread_;
    pthread_mutex_t thread_mutex_;
    pthread_cond_t  thread_cond_;
    task_handler    task_func_;
    void*           task_param_;
};

#endif //_THREAD_TASK_H_