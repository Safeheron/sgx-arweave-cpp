#ifndef _DEFINE_H_
#define _DEFINE_H_

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
 *  MUST be same as deinition in file ./Enclave/tasks/TaskConstant.h
*/
enum eKeyStatus {
    eKeyStatus_Unknown      = 0,
    eKeyStatus_Generating   = 1,    // generating key shard
    eKeyStatus_Reporting    = 2,    // creating the report
    eKeyStatus_Callback     = 3,    // doing result callback
    eKeyStatus_Finished     = 4,    // the task is finished
    eKeyStatus_Error        = 0xFF
};


#endif //_DEFINE_H_