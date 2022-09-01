/**
 * @file TaskConstant.h
 * @brief All constant definitions for TEE tasks
 *
 * @copyright Copyright (c) 2022
 */
#ifndef _TASK_CONSTANT_H_
#define _TASK_CONSTANT_H_

#include <string>

/**
 *  TEE tasks type definition
*/
enum eTaskType
{
    /* Add your task types below */
    eTaskType_Generate  = 101,
    eTaskType_Query     = 102
};

/**
 *  Key shard status definition
*/
enum eKeyStatus {
    eKeyStatus_Unknown      = 0,
    eKeyStatus_Generating   = 1,    // generating key shard
    eKeyStatus_Reporting    = 2,    // creating the report
    eKeyStatus_Callback     = 3,    // doing result callback
    eKeyStatus_Finished     = 4,    // the task is finished
    eKeyStatus_Error        = 0xFF
};

/**
 * Key shard generation context data definition
 */
struct KeyShardContext {
    KeyShardContext() {
        k = 0;
        l = 0;
        key_bits = 0;
        start_time = 0;
        generated_time = 0;
        finished_time = 0;
        key_status = eKeyStatus_Unknown;
    }
    KeyShardContext( int k_, int l_, int key_bits_ ) {
        k = k_;
        l = l_;
        key_bits = key_bits_;
        start_time = 0;
        generated_time = 0;
        finished_time = 0;
        key_status = eKeyStatus_Unknown;
    }
    int k;
    int l;
    int key_bits;
    long start_time;
    long generated_time;
    long finished_time;
    eKeyStatus key_status;
    std::string key_meta_hash;
};
#endif //_TASK_CONSTANT_H_
