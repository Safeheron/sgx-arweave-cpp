/*************************************************
 * File Name: log_u.h
 * Introduce: untrust interface for glog
 * Create: 2021-8-25 by yyf
 * 
 *************************************************/

#ifndef _LOG_U_H_
#define _LOG_U_H_

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

#endif //_LOG_U_H_