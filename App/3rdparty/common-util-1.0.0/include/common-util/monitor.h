/*************************************************
 * File Name: monitor.h
 * Introduce: The definition file for core error monitor class, 
 *            which uses libhttpsclient to POST error message to DingTalk robot.
 * Create: 2021-8-10 by yyf
 * 
 *************************************************/
#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <string>

/**
 * Error codes
*/
#define ERR_MONITOR_OK                  0
#define ERR_MONITOR_INVALID_PARAM       1
#define ERR_MONITOR_FAILED_TO_MALLOC    2
#define ERR_MONITOR_HTTP_RESPONSE_ERROR 3

/**
 * POST a message to the monitor, which is a HTTP server generally.
*/
class Monitor
{
public:
    static int setup( const std::string & url );
    static int post_info( const char* format, ... );
    static int post_error( const char* format, ... );
private:
    static int _post_monitor_msg( const std::string & prefix, const std::string & msg );
    static std::string & _get_post_body( const std::string & prefix, const std::string & msg );
};

#endif//_MONITOR_H_