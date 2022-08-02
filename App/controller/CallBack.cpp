#include "CallBack.h"
#include "tee-util-sgx/log_u.h"
#include "common-util/log.h"

extern std::string call_back_address;

CallBackFunction::CallBackFunction(const string& request_id_in, bool success, const string& message){
    request_id = request_id_in;

    if (success) {
        obj = web::json::value::parse(message);

    } else {
        string reply_string = "Request id" + request_id + message;

        obj = web::json::value::object(true);
        obj["success"] = web::json::value(false);
        obj["message"] = web::json::value(reply_string);
    }

    try {
        auto request = PostRequest();
        request.wait();
    } catch (const std::exception &e) {
        ERROR("Request ID: %s Error exception: %s", request_id.c_str(), e.what());
    }

}

CallBackFunction::~CallBackFunction(){

}

pplx::task<void> CallBackFunction::PostRequest() {

    string path = "";
    http_client client{call_back_address};
    string request_ID = request_id;

    /** POST request */
    auto request = client.request(methods::POST,path,obj)
            .then([request_ID](http_response resp) {
                if (resp.status_code() != status_codes::OK) {
                    ERROR("Request ID: %s, call back failed.", request_ID.c_str());
                }
                else{
                    INFO("Request ID: %s, call back successfully.", request_ID.c_str());
                }
            });
    return request;
}
