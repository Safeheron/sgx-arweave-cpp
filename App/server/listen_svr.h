/**
 * @file listen_svr.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _LISTEN_SVR_H_
#define _LISTEN_SVR_H_

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
    pplx::task<void>open() { return listener_.open(); }
    pplx::task<void>close() { return listener_.close(); }
public:
    static pplx::task<void> PostRequest( const std::string & request_id, const std::string & client_url, const std::string & body );

private:
    void HandlePost( const http_request & message );

private:
    http_listener   listener_;
};

#endif //_LISTEN_SVR_H_