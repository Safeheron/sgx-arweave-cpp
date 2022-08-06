/**
 * @file TaskConstant.h
 * @author your name (you@domain.com)
 * @brief All constant definitions for TEE tasks
 *
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
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
 *  Key shards status definition
*/
enum eKeyStatus {
    eKeyStatus_Unknown      = 0,
    eKeyStatus_Generating   = 1,
    eKeyStatus_Finished     = 2,
    eKeyStatus_Destory      = 3,
    eKeyStatus_Error        = 4
};

/**
 * Key shards generation context data definition
 */
struct KeyShardContext {
    KeyShardContext() {
        k = 0;
        l = 0;
        key_bits = 0;
        start_time = 0;
        key_status = eKeyStatus_Unknown;
    }
    KeyShardContext( int k_, int l_, int key_bits_ ) {
        k = k_;
        l = l_;
        key_bits = key_bits_;
        start_time = 0;
        key_status = eKeyStatus_Unknown;
    }
    int k;
    int l;
    int key_bits;
    long start_time;
    eKeyStatus key_status;
    std::string key_meta_hash;
};
#endif //_TASK_CONSTANT_H_