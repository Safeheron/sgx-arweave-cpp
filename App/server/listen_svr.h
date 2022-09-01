/**
 * @file listen_svr.h
 * @brief : HTTP request listen server.
 *          A CPPREST http_listener is used in this class to listen an HTTP request, only POST request is supported.
 *          Callback function PostRequest() will called if a POST request is received!
 *
 * @copyright Copyright (c) 2022
 */
#ifndef _LISTEN_SVR_H_
#define _LISTEN_SVR_H_

#include "thread_task.h"
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <list>

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class listen_svr
{
public:
    listen_svr( const std::string & url );
    virtual ~listen_svr();
public:
    pplx::task<void>open();
    pplx::task<void>close();
public:
    static pplx::task<void> PostRequest( const std::string & request_id, const std::string & client_url, const std::string & body );

private:
    void HandleMessage( const http_request & message );
    std::string CreateRequestID( const std::string & prefix = "" );

public:
    bool            is_stopping_;
private:
    http_listener   listener_;      // CPPREST listener
    ThreadTask*     timer_thread_;  // a timer thread to release stopped tasks thread in pool.
};

#endif //_LISTEN_SVR_H_