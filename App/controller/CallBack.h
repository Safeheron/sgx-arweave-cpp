#ifndef TEE_ARWEAVE_SERVER_CALLBACK_H
#define TEE_ARWEAVE_SERVER_CALLBACK_H

#include <iostream>
#include <cpprest/http_client.h>


using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace web::json;
using std::cerr;
using std::endl;
using std::string;


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
