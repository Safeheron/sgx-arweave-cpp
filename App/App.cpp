#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>     /* gethostname */
#include <netdb.h>      /* struct hostent */
#include <arpa/inet.h>  /* inet_ntop */
#include <pwd.h>


#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include "common/tee_error.h"
#include <common-util/HttpsServer.h>
#include <common-util/monitor.h>
#include <common-util/log.h>
#include <tee-util-sgx/log_u.h>
#include "Handler.h"
#include <pthread.h>


using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

#define PROJECT_NAME    "tee-arweave"


std::unique_ptr<handler> g_httpHandler;
bool thread_clear_map_quit = false;

std::string interface_create = "/arweave/create_key_shard";
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

/** Initialize enclave */
sgx_status_t initialize_enclave(const char* enclave_file)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /** Create enclave */
    ret = sgx_create_enclave(enclave_file, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        ERROR( "sgx_create_enclave() failed! errmsg: %s", t_strerror( (int)ret ) );
        return ret;
    }

    return ret;
}

int SGX_CDECL main(int argc, char *argv[])
{
    sgx_status_t result;
    string line;
    string log_dir = U("/var/log/glog-arweave");
    string port = U("40000");
    string address = U("http://0.0.0.0:");
    address.append(port);

    /** Initialize log */
    TeeLogHelper* glog_helper = nullptr;
    glog_helper = new TeeLogHelper( log_dir.c_str(), PROJECT_NAME );

    /** Initialize trusted execution environment */
    if ( ( result = initialize_enclave( (const char*)argv[1] ) ) != SGX_SUCCESS ) {
        ERROR( "initialize_enclave() failed! result: 0x%x, enclave file: %s", result, argv[1] );
        delete glog_helper;
        return -1;
    }

    ecall_print_enclave_id(global_eid);
    INFO_OUTPUT_CONSOLE( "Enclave is initialized!" );

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

    return 0;
}
