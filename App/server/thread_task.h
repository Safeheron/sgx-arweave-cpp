/**
 * @file thread_task.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _THREAD_TASK_H_
#define _THREAD_TASK_H_

#include "keyshare_param.h"
#include <pthread.h>

// Define thread function pointer
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