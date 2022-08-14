#include "App.h"
#include "Enclave_u.h"
#include "common/tee_error.h"
#include "common/log_u.h"
#include "server/listen_svr.h"
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sgx_urts.h>

#define PROJECT_NAME    "tee-arweave"

std::unique_ptr<listen_svr> g_httpServer = nullptr;
sgx_enclave_id_t global_eid = 0;

// Start the HTTP listen server, and receive
// request from clients
void HttpServer_Start( const std::string & address )
{
    uri_builder uri( address );

    auto addr = uri.to_uri().to_string();
    g_httpServer = std::unique_ptr<listen_svr>(new listen_svr( addr ));
    g_httpServer->open().wait();

    INFO_OUTPUT_CONSOLE("Listening for requests at: %s", address.c_str());
}

// Shutdown the HTTP listen server
void HttpServer_Shutdown()
{
    if ( g_httpServer ) {
        g_httpServer->close().wait();
    }
}

int SGX_CDECL main(int argc, char *argv[])
{
    int ret = 0;
    sgx_status_t sgx_status;
    char* enclave_id = nullptr;
    size_t enclave_id_len = 0;
    std::string listen_addr = "http://0.0.0.0";
    const std::string LIST_PORT = "40000";
    const std::string log_dir = "/root/glog-arweave";
    std::string input_line;

    /** Initialize log */
    TeeLogHelper* glog_helper = nullptr;
    glog_helper = new TeeLogHelper( log_dir.c_str(), PROJECT_NAME );

    /** Initialize trusted execution environment */
    sgx_status = sgx_create_enclave( (const char*)argv[1], 0, nullptr, nullptr, &global_eid, nullptr );
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

    sgx_status = ecall_init( global_eid, &ret );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf( "ecall_init() failed, error message: %s\n", t_strerror((int)sgx_status) );
        delete glog_helper;
        return -1;
    }
    
    /** Initializing the REST service listener */
    listen_addr.append(":");
    listen_addr.append(LIST_PORT);
    HttpServer_Start( listen_addr );

    /* End server until any key is pressed */
    INFO_OUTPUT_CONSOLE("Press ENTER to exit.");
    getline(std::cin, input_line);

    /** Server exit */
    INFO_OUTPUT_CONSOLE( "Server closed!");
    HttpServer_Shutdown();

    /** Free enclave resources */
    ecall_free( global_eid );

    /** Destroy enclave */
    sgx_destroy_enclave(global_eid);

    if ( glog_helper) {
        delete glog_helper;
        glog_helper = nullptr;
    }

    if ( enclave_id ) {
        free( enclave_id );
        enclave_id = nullptr;
    }

    return 0;
}
