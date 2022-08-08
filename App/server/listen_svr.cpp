/**
 * @file listen_svr.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "listen_svr.h"
#include "msg_handler.h"
#include "log_u.h"
#include <cpprest/http_client.h>

using namespace utility;
using namespace web::http;
using namespace web::http::client;

listen_svr::listen_svr( const std::string & url )
 : listener_( url )
{
    listener_.support(methods::POST, 
                      std::bind(&listen_svr::HandleMessage, 
                      this, 
                      std::placeholders::_1));
    listener_.support(methods::GET, 
                      std::bind(&listen_svr::HandleMessage, 
                      this, 
                      std::placeholders::_1));
}

listen_svr::~listen_svr()
{
}

// Start to listen
pplx::task<void> listen_svr::open() 
{ 
    return listener_.open(); 
}

// Stop to listen
pplx::task<void> listen_svr::close() 
{ 
    msg_handler::DestoryThreadPool();

    return listener_.close(); 
}

// A HTTP request message is received
void listen_svr::HandleMessage( const http_request & message )
{
    std::string path = message.request_uri().path();
    auto req_body = message.extract_json().get();

    if ( path.length() == 0 ) {
        message.reply( status_codes::BadRequest, "Unknow request path!" );
    }

    msg_handler handler;
    std::string resp_body;
    handler.process( path, req_body.serialize(), resp_body );
    message.reply( status_codes::OK, resp_body );
}

// Post a message to client
pplx::task<void> listen_svr::PostRequest( 
    const std::string & request_id, 
    const std::string & client_url, 
    const std::string & body )
{
    std::string path = "";
    http_client client{client_url};
    web::json::value resp_json = json::value::parse( body );

    auto request = client.request( methods::POST, path, resp_json )
            .then( [request_id](http_response resp ) {
                if ( resp.status_code() != status_codes::OK ) {
                    ERROR( "Request ID: %s, call back failed! Status code: %d", request_id.c_str(), resp.status_code() );
                }
                else{
                    INFO( "Request ID: %s, call back successfully.", request_id.c_str() );
                }
            });
    return request;
}