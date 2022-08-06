#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include "tee_error.h"
#include <log_u.h>
#include "Handler.h"

#define PROJECT_NAME    "tee-arweave"

std::unique_ptr<handler> g_httpHandler;
bool thread_clear_map_quit = false;

std::string interface_create = "/arweave/create_key_share";
std::string interface_query  = "/arweave/query_key_shard_state";
std::string call_back_address = "http://127.0.0.1:8008/sayHi";

sgx_enclave_id_t global_eid = 0;
thrdpool_t* pool = nullptr;

void on_initialize(const string& address)
{
    uri_builder uri(address);

    auto addr = uri.to_uri().to_string();
    g_httpHandler = std::unique_ptr<handler>(new handler(addr));
    g_httpHandler->open().wait();

    INFO_OUTPUT_CONSOLE("Listening for requests at: %s", address.c_str());
}

void on_shutdown()
{
    /** Destroy the thread pool */
    thrdpool_destroy(&PendingTask, pool);

    g_httpHandler->close().wait();
}

int SGX_CDECL main(int argc, char *argv[])
{
    int ret = 0;
    sgx_status_t sgx_status;
    char* enclave_id = nullptr;
    size_t enclave_id_len = 0;
    //
    string line;
    string log_dir = U("/root/glog-arweave");
    string port = U("40000");
    string address = U("http://0.0.0.0:");
    address.append(port);

    /** Initialize log */
    TeeLogHelper* glog_helper = nullptr;
    glog_helper = new TeeLogHelper( log_dir.c_str(), PROJECT_NAME );

    /** Initialize trusted execution environment */
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, NULL, NULL, &global_eid, NULL);
    if (sgx_status != SGX_SUCCESS ) {
        ERROR("initialize enclave failed! enclave file: %s, error message: %s", argv[1], t_strerror((int)sgx_status));
        delete glog_helper;
        return -1;
    }

    sgx_status = ecall_get_enclave_id( global_eid, &ret, &enclave_id, &enclave_id_len );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        ERROR("ecall_get_enclave_id() failed, error message: %s", t_strerror((int)sgx_status));
        delete glog_helper;
        return -1;
    }
    INFO_OUTPUT_CONSOLE( "Enclave is initialized! Enclave ID: %s", enclave_id );

    /** Initializing the REST service listener */
    on_initialize(address);

    INFO_OUTPUT_CONSOLE("Press ENTER to exit.");
    getline(std::cin, line);

    /** Server exit */
    INFO_OUTPUT_CONSOLE( "Server closed!");
    thread_clear_map_quit = true;
    on_shutdown();

    /** Free enclave resources */
    ecall_free( global_eid);

    /** Destroy enclave */
    sgx_destroy_enclave(global_eid);
    delete glog_helper;

    if ( enclave_id ) {
        free( enclave_id );
        enclave_id = nullptr;
    }

    return 0;
}
