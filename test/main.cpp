/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

 #include <stdio.h>
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <list>

using namespace web;
using namespace http;
using namespace web::http;
using namespace web::http::client;
using namespace utility;
using namespace http::experimental::listener;

// A HTTP request message is received
void HandleMessage( const http_request & message )
{
    std::string path = message.request_uri().path();
    auto req_body = message.extract_json().get();
}

int main()
{
    std::string test_req = "{" \
    "\"userPublicKeyList\": ["  \
    "\"049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c\", "\
    "\"04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d\", "\
    "\"0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356\"  "\
    "], "\
    "\"k\": 2, "\
    "\"l\": 3, "\
    "\"keyLength\": 1024, "\
    "\"callBackAddress\": \"http://127.0.0.1:8008/sayHi\" "\
    "}";
    //
    http_listener   listener("http://127.0.0.1:8008");
    listener.support(methods::POST, std::bind(&HandleMessage, std::placeholders::_1));
    (pplx::task<void>(listener.open())).wait();
    //
    http_client client{"http://127.0.0.1:40000"};
    web::json::value resp_json = json::value::parse( test_req );

    auto request = client.request( methods::POST, "/arweave/create_key_share", resp_json )
            .then( [](http_response resp ) {
                if ( resp.status_code() != status_codes::OK ) {
                    printf( "Call back failed! Status code: %d\n", resp.status_code() );
                }
                else{
                    printf( "Call back successfully.\n" );
                }
            });
    request.wait();


    (pplx::task<void>(listener.close())).wait();

    return 0;
}