//
// Created by EDY on 2022/6/16.
//

#ifndef TEE_ARWEAVE_SERVER_CALLBACK_H
#define TEE_ARWEAVE_SERVER_CALLBACK_H

#include <iostream>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <cpprest/http_client.h>


using namespace utility;
using namespace web::http;
using namespace web::http::client;
using std::cerr;
using std::endl;

#ifdef _WIN32
#define tcout std::wcout
#else
#define tcout std::cout
#endif

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using std::cerr;
using std::endl;
using std::string;
using namespace web::json;


class CallBackFunction{
public:
    CallBackFunction(const string& request_id_in, bool success, const string& message);
    ~CallBackFunction();

    /**
     * A webhook for the successful thread tasks to send reply.
     * @return pplx::task<void>
     */
    pplx::task<void> PostRequest();

private:
    string request_id;
    web::json::value obj;
};


#endif //TEE_ARWEAVE_SERVER_CALLBACK_H