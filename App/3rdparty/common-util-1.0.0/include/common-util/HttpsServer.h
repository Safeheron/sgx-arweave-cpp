/*************************************************
 * File Name: HttpsServer.h
 * Introduce: The definition file for HttpsServer class, 
 *            which implements an interface to create the 
 *            https server instance.
 * 
 * Create: 2021-8-18 by yyf
 * 
 *************************************************/
#ifndef _HTTPS_SERVER_H_
#define _HTTPS_SERVER_H_

#include <string>

/**
 * Error codes
*/
#define ERR_SERVER_OK                           0   // OK
#define ERR_SERVER_INVALIDPARAM                 1   // invalid parameters, maybe some parameters are nullptr
#define ERR_SERVER_INVALIDCALL                  2   // invalid calling stack
#define ERR_SERVER_URLSCHEME_WRONG              3   // scheme(https or http) in url is not same as setting.
#define ERR_SERVER_FAILED_TO_CREATE_THREAD      4   // pthread_create() failed

/**
 * http requst callback function
 * Caller need to implement this function, deal with the requestion and return response content
*/
typedef std::string & (*http_handler)( const std::string & method, const std::string & req_param );

/**
 * Name: HttpServer
 * Introduce: The http server interface, to create/free the http server instance.
*/
class HttpsServer
{
public:
    static HttpsServer* init_srv( const char* http_port, http_handler handler );
    static void free_srv( HttpsServer* srv );
};

#endif //_HTTPS_SERVER_H_
