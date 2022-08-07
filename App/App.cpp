#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include "tee_error.h"
#include <log_u.h>
#include "Handler.h"
#include "json/json.h"

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

    ////////////////////////Test/////////////////////////////
    char* reply = nullptr;
    size_t reply_len = 0;
    JSON::Root root = JSON::Root::parse( test_req );
    if ( !root.is_valid() ) {
        printf( "test_req is not a JSON!\n" );
        return -1;
    }
    sgx_status = ecall_init( global_eid, &ret );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf( "ecall_init() failed, error message: %s\n", t_strerror((int)sgx_status) );
        delete glog_helper;
        return -1;
    }
    sgx_status = ecall_run( global_eid, &ret, 101,  "1111", test_req.c_str(), test_req.length(), &reply, &reply_len );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf( "ecall_run() failed, ret: 0x%x, error message: %s\n", ret, t_strerror((int)sgx_status) );
        delete glog_helper;
        return -1;
    }
    printf( "--->reply lenght: %d\n", (int)reply_len );
    printf( "--->reply: %s\n", reply );
    free( reply );
    reply = nullptr;
    sgx_status = ecall_free( global_eid );
    if ( sgx_status != SGX_SUCCESS ) {
        printf( "ecall_free() failed, error message: %s\n", t_strerror((int)sgx_status) );
        delete glog_helper;
        return -1;
    }
    ////////////////////////Test/////////////////////////////

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
