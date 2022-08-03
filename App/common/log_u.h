#ifndef TEE_ARWEAVE_SERVER_LOG_U_H
#define TEE_ARWEAVE_SERVER_LOG_U_H

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <string.h>

// The max log message length
#define MAX_LOG_LEN   4096

// LOG LEVEL
#define LL_INFO 0
#define LL_WARNING 1
#define LL_ERROR 2
#define LL_FATAL 3

// only output file name, but not full path in log
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

//// Basic macro
//#define LOG(level, format, args...)                                   \
//    do{                                                               \
//      char log[MAX_LOG_LEN] = {0};                                    \
//      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ ); \
//      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
//      glog_log(level, log);                                            \
//    }while(0)

// Log on level 'INFO'
#define INFO(format, args...)                                         \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      glog_log(LL_INFO, log);                                          \
    }while(0)

// Log on level 'WARNING'
#define WARN(format, args...)                                         \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      glog_log(LL_WARNING, log);                                       \
    }while(0)

// Log on level 'ERROR'
#define ERROR(format, args...)                                        \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      glog_log(LL_ERROR, log);                                         \
    }while(0)

// The macro bellow is forbidden and "FATAL" message would be ignored finally
// Log on level 'FATAL'
#define FATAL(format, args...)                                        \
    do{                                                               \
      char log[MAX_LOG_LEN] = {0};                                    \
      snprintf(log, MAX_LOG_LEN, "[%s(%d)]%s:", __FILENAME__, __LINE__, __FUNCTION__ );     \
      snprintf(log+strlen(log), MAX_LOG_LEN-strlen(log), format, ##args); \
      glog_log(LL_FATAL, log);                                         \
    }while(0)

/**
 * Put information message to both of log file and console
*/
#define INFO_OUTPUT_CONSOLE(format, args...)        \
    do {                                            \
      char info[MAX_LOG_LEN] = {0};                 \
      snprintf(info, MAX_LOG_LEN, format, ##args);  \
      fprintf( stdout, "INFO: %s\n", info );        \
      INFO( "%s", info );                           \
    }while(0);



// log output function
void glog_log(int level, char* log);


class TeeLogHelper
{
public:
    /**
    * dir: log files output dir, use "/tmp" is it is null.
    * program: the program name string, as the log file prefix
    * log_level: message logged at level >= this level will be logged
    */
    TeeLogHelper(const char* dir, const char* program, const int log_level = 0);
    ~TeeLogHelper();
};



#endif //TEE_ARWEAVE_SERVER_LOG_U_H
