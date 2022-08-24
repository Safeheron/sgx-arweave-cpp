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
#include <crypto-curve/curve.h>
#include <crypto-ecies/ecies.h>
#include <crypto-hash/sha256.h>
#include <crypto-encode/hex.h>
#include <crypto-bn/bn.h>
#include <crypto-bn/rand.h>
#include <list>
#include <map>

using namespace web;
using namespace http;
using namespace web::http;
using namespace web::http::client;
using namespace http::experimental::listener;
using safeheron::bignum::BN;
using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::ecies::ECIES;

bool g_result_received = false;
std::string g_genkey_result;
//
std::string g_arweave_server = "http://127.0.0.1:40000";
std::string g_genkey_path = "/arweave/create_key_shard";
std::string g_query_path = "/arweave/query_key_shard";
//
int g_k = 2;
int g_l = 3;
int g_key_bits = 4096;
std::string g_callback_addr = "http://127.0.0.1:8008";
std::string g_callback_path = "/keyShareCallback";
std::map<std::string, std::string> g_users_ecc_key;
//
int InputUserECCKeyPairs();
int GenerateUserECCKeyPairs();
int PostGenerateKeyShareRequest();
int PostQueryStatusRequest();
int ParseKeyShareResult( const std::string & result );

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

    // Generate users ECC keypair for encrypting private key shard
    printf( "Try to generate users ECC keypair...\n" );
#if 0
    if ( (ret = InputUserECCKeyPairs()) != 0 ) {
        goto _exit;
    }
#else
    if ( (ret = GenerateUserECCKeyPairs()) != 0 ) {
        printf( "GenerateUserECCKeyPairs() failed! ret: %d\n", ret );
        goto _exit;
    }
#endif //
    printf( "Users ECC keypair are generated!\n\n" );

    // Post a key shard generation request
    printf( "Try to post key shard generation request...\n" );
    if ( (ret = PostGenerateKeyShareRequest()) != 0 ) {
        goto _exit;
    }
    printf( "Key shard generation request is post, waiting for result...\n\n" );

    // Waiting for generating result, and query it's status per 2s.
    // You can post a generation request agian if query request's response is failed 
    // or current status is unknown or error.
    for (;;) {
        if ( g_result_received) break;

        // Post a status query request to check generating request is handling or not
        printf( "Try to post status query request...\n" );
        if ( (ret = PostQueryStatusRequest()) != 0 ) {
            goto _exit;
        }
        printf( "Status is returned!\n\n" );
        sleep( 2 );
    }

    // key shards result is received, parse it!
    printf( "Key shards result: %s\n\n", g_genkey_result.c_str() );
    printf( "Result is received! Try to parse it...\n" );
    if ( (ret = ParseKeyShareResult( g_genkey_result ) ) != 0 ) {
        printf( "ParseKeyShareResult() failed! ret: %d\n", ret );
        printf( "result: %s\n", g_genkey_result.c_str() );
        goto _exit;
    }
    printf( "Result is parsed succussfully!\n\n" );

_exit:
    (pplx::task<void>(callback_listener.close())).wait();

    printf( "\nEnd! Press and key to exit!\n" );
    getchar();

    return 0;
}

/**
 * @brief Use a const group user ECC key pairs for test 
 * 
 * @return int 
 */
int InputUserECCKeyPairs()
{
    std::string priv[] = { "8bab3e786c5e1ffd30dc475f62f3a5cb1aa0c5efe8ba2019e528c77ac2ba99bc",
                           "a37359cf38aab6208599416a74e5fef293cbc3cb5e03a038e3ef37eb65ad1289",
                           "2207e9e61ac486f2c01cfd926fe3f24252b36a68d40ce6bfdf3c5f2e5b72b7e8"
    };
    std::string pubkey[] = { "049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c",
                             "04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d",
                             "0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356"
    };

    g_users_ecc_key.clear();
    for ( int i = 0; i < sizeof(priv)/sizeof(priv[0]); i++ ) {
        g_users_ecc_key.insert(std::pair<std::string, std::string>(pubkey[i], priv[i]));
        printf("--->%d: Private Key: %s\n", i+1, priv[i].c_str());
        printf("--->%d: Public Key: %s\n", i+1, pubkey[i].c_str());
    }

    return 0;
}

/**
 * @brief Generate an ECC keypair for every user, and the ECC public key will
 *        be added in request, TEE uses it to encrypt TSS-RSA private key shard.
 * 
 * @return int 
 */
int GenerateUserECCKeyPairs()
{
    int ret = 0;
    const Curve *curv = GetCurveParam( CurveType::P256 );

    // create ecc keypair for every user
    g_users_ecc_key.clear();
    for (int i = 0; i < g_l; i++ ) {
        std::string privkey_hex;
        std::string pubkey_hex;
        uint8_t pubkey_65[65] = { 0 };
        BN privkey = safeheron::rand::RandomBNLtGcd( curv->n );
        privkey.ToHexStr( privkey_hex );
        printf("--->%d: Private Key: %s\n", i+1, privkey_hex.c_str());

        CurvePoint pubkey = curv->g * privkey;
        pubkey.EncodeFull( pubkey_65 );
        pubkey_hex = safeheron::encode::hex::EncodeToHex( pubkey_65, sizeof(pubkey_65) );
        printf("--->%d: Public Key: %s\n", i+1, pubkey_hex.c_str());

        g_users_ecc_key.insert(std::pair<std::string, std::string>(pubkey_hex, privkey_hex));
    }

    return 0;
}

/**
 * @brief POST a key shard generation request to TEE server,
 *        the request body is a JSON as below:
 *  {
 *   "userPublicKeyList": [], //user ecc public key arrary, in hex string
 *   "k": 2, //the numerator of threshold, l/2 <= k <= l 
 *   "l": 3, //the denominator of threshold, 1 <= l <= 20
 *   "keyLength": 4096, // TSS-RSA key bits, support 1024,2048, 3072 and 4096
 *   "callBackAddress": "http://127.0.0.1:8008/keyShareCallback"  // listen server url for receiving generation result.
 *  }
 * 
 * The response body of this request is a JSON like below:
 * {
 *   "code":0,
 *   "message":"Request has been accepted.",
 *   "request_id":"B65F1E071A91F775",
 *   "success":true
 *   }
 *      
 * After key shards are generated in TEE server, the result will be send to listen address specied in "callBackAddress".
 *
 * @return int 
 */
int PostGenerateKeyShareRequest()
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
    for ( auto & keypair : g_users_ecc_key ) {
        pubkey_array.push_back( json::value(keypair.first) );
    }
    req_obj["userPublicKeyList"] = json::value::array( pubkey_array );
    printf( "req_body: %s\n", req_obj.serialize().c_str() );

    // new a client to POST request to arweave server
    http_client client( g_arweave_server.c_str() );

    // post generate key shard request to server, because server handles this request
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

/**
 * @brief POST a key shard generation's status query request to TEE server,
 *        the request body is a JSON as below:
 *  {
 *    "pubkey_list_hash": "" //SHA256 of users ecc public key list, use it as the unique ID to find task in TEE server
 *  }
 *
 * If the query ID (pubkey_list_hash) is exist, the response body is a JSON like below:
 * {
 *   "alive_time_seconds":0,  // this key shards generating duration, in seconds.
 *   "k":2, 
 *   "l":3,
 *   "request_id":"8A773D52A07F6695",
 *   "status_code":1,   // current task's status, 0-unknown;1-generating;2-finished;3-error
 *   "status_text":"Generating",
 *   "success":true
 *   }
 *
 * Othereise, the response body like below:
 * {
 *   "success":false
 * }
 * 
 * @return int 
 */
int PostQueryStatusRequest()
{
    // Calc public key list hash (SHA256)
    std::vector<std::string> pubkey_list;
    for ( auto & keypair : g_users_ecc_key ) {
        pubkey_list.push_back( keypair.first );
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

    // post key shard generation's status request to server
    pplx::task<void> requestTask = client.request( methods::POST, g_query_path.c_str(), req_obj.serialize().c_str(), "application/json" )
        .then([]( http_response response ) {
            std::string resp_body = response.extract_json().get().serialize();
            printf( "resp_body: %s\n", resp_body.c_str() );
        });

    requestTask.wait();
    
    return 0;
}

/**
 * @brief To parse key shard generation result, users private key shard
 *        is an ecies cipher in response JSON, it can be dencrypted by users
 *        ECC private key created in GenerateUserECCKeyPairs().
 * 
 * @param result The result data of key shard generation, is a JSON as below:
 *      
 *  {
 *    "request_id": "",  //an unique ID returned by server for debug    
 *    "pubkey_list_hash": "",   //users public key list hash (SHA256), in hex
 *    "key_shard_pubkey": {},   //generated RSA public key of private key shard, in JSON
 *    "key_shard_pkg": [        //RSA key shard array
 *        {
 *            "public_key": "", //ecc public key of user 1
 *            "encrypt_key_info": "" //cipher of private key shard 1 and meta
 *        },
 *        {
 *            "public_key": "", //ecc public key of user 2
 *            "encrypt_key_info": "" //cipher of private key shard 2 and meta
 *        },
 *        {
 *            "public_key": "", //ecc public key of user 3
 *            "encrypt_key_info": "" //cipher of private key shard 3 and meta
 *        }
 *     ],
 *    "tee_report": "" // Intel SGX enclave report of the TEE server, in base64.
 *  }
 *
 *  node "encrypt_key_info" is an ecies cipher of private key shard's data, TEE server use 
 *  the below algorithms for ecies:
 *      KDF: KDF2_18033 with SHA512
 *      Symmetic: AES256_CBC, key size: 256 bits
 *      MAC: HMAC_SHA512, key size: 1024 bits 
 *  And the ecies cipher format is: 0x04|x|y|c|h|iv, iv is 16 bytes length.
 *  We can use function safeheron::ecies::ECIES::DecryptPack() to dencrypt it directly.
 *  
 *  After decrypted, private key shard plain data is a JSON as below:
 *  {
 *      "key_meta": {
 *                  "k": 2,
 *                  "l": 3,
 *                  "vkv": "lajksjkdja",
 *                  "vku": "iojkcjasjicj",
 *                    "vki_arr": [
 *                    "kasjhdkjashvalue",
 *                    "asjhdkjashdjavalue",
 *                    "looizjxcnaisvalue"
 *                  ]
 *              },
 *      "key_shard": {
 *                  "index": 1,
 *                  "private_key_shard": "kasdkajshdasd"
 *              }
 *   }
 * @return int 
 */
int ParseKeyShareResult( const std::string & result )
{
    web::json::value ret_json;

    try {
        ret_json = json::value::parse( result );
    }
    catch( const std::exception &e ) {
        printf( "result string is wrong! Exception message: %s", e.what() );
        return -1;
    }

    // "request_id" node
    std::string request_id = ret_json.at( "request_id" ).as_string();
    printf("request_id: %s\n", request_id.c_str());

    // "pubkey_list_hash" node
    std::string pubkey_list_hash = ret_json.at( "pubkey_list_hash" ).as_string();
    printf("pubkey_list_hash: %s\n", pubkey_list_hash.c_str());
    
    // "tee_report" node
    std::string tee_report = ret_json.at( "tee_report" ).as_string();
    printf("tee_report: %s\n", tee_report.c_str());

    // "key_shard_pubkey" node, is a JSON object
    auto jsonObj = ret_json.as_object();
    std::string rsa_pubkey = jsonObj["key_shard_pubkey"].serialize();
    printf("key_shard_pubkey: %s\n", rsa_pubkey.c_str());

    // "key_shard_pkg"
    auto array = ret_json.at( "key_shard_pkg" ).as_array();
    for ( const auto & value : array ) {
        auto obj = value.as_object();
        std::string user_pubkey = obj["public_key"].as_string();
        std::string shard_cipher = obj[ "encrypt_key_info" ].as_string();
        std::string cipher_data = safeheron::encode::hex::DecodeFromHex( shard_cipher );
        printf( "Try to decrypt key shard cipher for public_key: %s!\n", user_pubkey.c_str() );

        // to decrypt shard_cipher by users ecc private key
        ECIES ecies;
        std::string shard_plain;
        std::string user_ecc_privkey = g_users_ecc_key[user_pubkey];
        BN prikey = BN::FromHexStr( user_ecc_privkey );
        if ( !ecies.DecryptPack( prikey, cipher_data, shard_plain ) ) {
            printf( "Decrypt key shard cipher failed!\n" );
            printf( "--->ecc private key: %s\n", user_ecc_privkey.c_str() );
            printf( "--->rsa key shard cipher: %s\n", shard_cipher.c_str() );
            continue;
        }
        printf( "Key shard cipher is decrypted!\n" );

        // key meta and key shard data
        web::json::value key_pkg_json = json::value::parse( shard_plain );
        std::string key_meta = key_pkg_json.as_object()["key_meta"].serialize();
        std::string key_shard = key_pkg_json.as_object()["key_shard"].serialize();
        printf( "--->Key meta: %s\n", key_meta.c_str() );
        printf( "--->Key shard: %s\n", key_shard.c_str() );
    }

    return 0;
}