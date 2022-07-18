//
// Created by max on 4/18/20.
//

#ifndef PROJECT_LOG_H
#define PROJECT_LOG_H

#include <string.h>

// The max log message length
#define MAX_LOG_LEN   4096

// Log Level
#define LL_INFO 0
#define LL_WARNING 1
#define LL_ERROR 2
#define LL_FATAL 3

#define FUNC_BEGIN INFO( "Begin!" )
#define FUNC_END INFO( "End!" )

// only output file name, but not full path in log
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

// Basic macro
#define LOG(level, format, args...)                                   \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ ); \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      tee_log(level, log);                                            \
    }while(0)

// Log on level 'INFO'
#define INFO(format, args...)                                         \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      tee_log(LL_INFO, log);                                          \
    }while(0)

// Log on level 'WARNING'
#define WARN(format, args...)                                         \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      tee_log(LL_WARNING, log);                                       \
    }while(0)

// Log on level 'ERROR'
#define ERROR(format, args...)                                        \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      tee_log(LL_ERROR, log);                                         \
    }while(0)

// The macro bellow is forbidden and "FATAL" message would be ignored finally
// Log on level 'FATAL'
#define FATAL(format, args...)                                        \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      tee_log(LL_FATAL, log);                                         \
    }while(0)

void tee_log(int level, char* log);
#endif //PROJECT_LOG_H
