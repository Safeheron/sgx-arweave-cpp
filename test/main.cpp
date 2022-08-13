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
#include <crypto-hash/sha256.h>
#include <crypto-encode/hex.h>
#include <list>

using namespace web;
using namespace http;
using namespace web::http;
using namespace web::http::client;
using namespace http::experimental::listener;

bool g_result_received = false;
std::string g_genkey_result;
//
std::string g_arweave_server = "http://127.0.0.1:40000";
std::string g_genkey_path = "/arweave/create_key_share";
std::string g_query_path = "/arweave/query_key_shard_state";
//
int g_k = 2;
int g_l = 3;
int g_key_bits = 2048;
std::string g_callback_addr = "http://127.0.0.1:8008";
std::string g_callback_path = "/keyShareCallback";
std::string g_user_pubkey[] = {"049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c",
                               "04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d",
                               "0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356"};

int doGenerateKeyShareRequest();
int doQueryStatusRequest();

// handler for generation request result callback
void HandleMessage( const http_request & message )
{
    std::string path = message.request_uri().path();
    auto req_body = message.extract_json().get();
    if ( path == g_callback_path ) {
        g_result_received = true;
        g_genkey_result = req_body.serialize();
    }
    else {
        printf("--->Unknow path: %s\n", path.c_str());
    }
    message.reply( status_codes::OK );
}

int main()
{  
    int ret = 0;

    // start a listener for result callback
    http_listener callback_listener("http://127.0.0.1:8008");
    callback_listener.support(methods::POST, std::bind(&HandleMessage, std::placeholders::_1));
    (pplx::task<void>(callback_listener.open())).wait();

    // Post a key share generation request
    printf( "Try to post key share generation request...\n" );
    if ( (ret = doGenerateKeyShareRequest()) != 0 ) {
        goto _exit;
    }
    printf( "Key share generation request is post, waiting for result...\n\n" );

    // waiting for generating result, and query it's status per 2s.
    for (;;) {
        if ( g_result_received) break;

        // Post a status query request to check generating request is handling or not
        printf( "Try to post status query request...\n" );
        if ( (ret = doQueryStatusRequest()) != 0 ) {
            goto _exit;
        }
        printf( "Status is returned!\n\n" );
        sleep( 2 );
    }

    printf( "Result is received! result:\n%s\n", g_genkey_result.c_str() );

_exit:
    (pplx::task<void>(callback_listener.close())).wait();

    printf( "\nEnd! Press and key to exit!\n" );
    getchar();

    return 0;
}

// POST a key share generation request
int doGenerateKeyShareRequest()
{
    // request parameters
    std::string callback_url = g_callback_addr;
    callback_url.append( g_callback_path );
    json::value req_obj = json::value::object( true );
    req_obj["k"] = json::value( g_k );
    req_obj["l"] = json::value( g_l );
    req_obj["keyLength"] = json::value( g_key_bits );
    req_obj["callBackAddress"] = json::value( callback_url );
    std::vector<json::value> pubkey_array;
    for ( int i = 0; i < sizeof(g_user_pubkey) / sizeof(g_user_pubkey[0]); ++i ) {
        pubkey_array.push_back( json::value(g_user_pubkey[i]) );
    }
    req_obj["userPublicKeyList"] = json::value::array( pubkey_array );
    printf( "req_body: %s\n", req_obj.serialize().c_str() );

    // new a client to POST request to arweave server
    http_client client( g_arweave_server.c_str() );

    // post generate key share request to server, because server handles this request
    // in async model and returns immediately, so we can code in sysnc model here.
    http_response response = client.request( methods::POST, g_genkey_path.c_str(), req_obj.serialize().c_str(), "application/json" ).get();
    std::string resp_body = response.extract_json().get().serialize();
    printf( "resp_body: %s\n", resp_body.c_str() );
    if ( response.status_code() == status_codes::OK ) {
        printf( "Request post successfully!\n" );
    }
    else {
        printf( "Request post failed! code: %d\n", response.status_code() );
        return -1;
    }

    return 0;
}

// POST a key share generation's status query request
int doQueryStatusRequest()
{
    // Calc public key list hash (SHA256)
    std::vector<std::string> pubkey_list;
    for ( int i = 0; i < sizeof(g_user_pubkey) / sizeof(g_user_pubkey[0]); ++i ) {
        pubkey_list.push_back( g_user_pubkey[i] );
    }
    std::sort( pubkey_list.begin(), pubkey_list.end() );
    //
    std::string pubkey_list_str;
    for ( int i = 0; i < pubkey_list.size(); ++i ) {
        pubkey_list_str += pubkey_list[i];
    }
    //
    safeheron::hash::CSHA256 sha256;
    uint8_t digest[safeheron::hash::CSHA256::OUTPUT_SIZE] = { 0 };
    sha256.Write( (const uint8_t *)pubkey_list_str.c_str(), pubkey_list_str.size() );
    sha256.Finalize( digest );
    std::string pubkey_hex = safeheron::encode::hex::EncodeToHex( digest, safeheron::hash::CSHA256::OUTPUT_SIZE );

    // request parameters
    json::value req_obj = json::value::object( true );
    req_obj["pubkey_list_hash"] = json::value( pubkey_hex );
    printf( "req_body: %s\n", req_obj.serialize().c_str() );

    // new a client to POST request to arweave server
    http_client client( g_arweave_server.c_str() );

    // post key share generation's status request to server
    pplx::task<void> requestTask = client.request( methods::POST, g_query_path.c_str(), req_obj.serialize().c_str(), "application/json" )
        .then([]( http_response response ) {
            std::string resp_body = response.extract_json().get().serialize();
            printf( "resp_body: %s\n", resp_body.c_str() );
        });

    requestTask.wait();
    
    return 0;
}