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
#include "../common/log_u.h"
#include <cpprest/http_client.h>
#include <safeheron/crypto-bn/bn.h>
#include <safeheron/crypto-bn/rand.h>

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using safeheron::bignum::BN;

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
    std::string request_id;
    std::string path = message.request_uri().path();
    auto req_body = message.extract_json().get();

    FUNC_BEGIN;

    // generate an unique ID for log lines
    request_id = CreateRequestID();
    INFO_OUTPUT_CONSOLE( "Request ID: %s is received!", request_id.c_str() );

    if ( path.length() == 0 ) {
        ERROR( "Request ID: %s, path is null!", request_id.c_str() );
        message.reply( status_codes::BadRequest, "Unknow request path!" );
    }

    // process request and reply its response to client
    msg_handler handler;
    std::string resp_body;
    handler.process( request_id, path, req_body.serialize(), resp_body );
    web::json::value resp_json = json::value::parse( resp_body );
    resp_json["request_id"] = json::value( request_id );
    message.reply( status_codes::OK, resp_json );

    INFO_OUTPUT_CONSOLE( "Request ID: %s is replied!", request_id.c_str() );

    FUNC_END;
}

// Generate an uniqne ID for every request process, and this ID
// will be output log file to identify logs for this request process.
std::string listen_svr::CreateRequestID( const std::string & prefix )
{
    std::string ret;

    // generate an 8 bytes rand number as the ID
    BN rand_bn = safeheron::rand::RandomBNStrict( 64 );
    rand_bn.ToHexStr( ret );

    return prefix + ret;
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
    resp_json["request_id"] = json::value( request_id );

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