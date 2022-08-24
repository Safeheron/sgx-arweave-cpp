#include "App.h"
#include "Enclave_u.h"
#include "common/iniconfig.h"
#include "common/tee_error.h"
#include "common/log_u.h"
#include "server/listen_svr.h"
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sgx_urts.h>

#define PROJECT_NAME    "tee-arweave"

sgx_enclave_id_t global_eid = 0;

std::unique_ptr<listen_svr> g_httpServer = nullptr;
std::string g_key_shard_generation_path;
std::string g_key_shard_query_path;

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
    IniConfig cfg;
    std::string listen_addr;
    int log_enable = 0;
    std::string log_path;
    std::string input_line;
    TeeLogHelper* glog_helper = nullptr;

    // Read configuration file server.ini
    if ((ret = cfg.load_file("./server.ini")) != ERR_INI_OK) {
        INFO_OUTPUT_CONSOLE( "Failed to load configure file ./server.ini!" );
        return -1;
    }
    listen_addr = cfg.get_string( "server", "host_address");
    if ( listen_addr.length() == 0 ) {
        INFO_OUTPUT_CONSOLE( "Failed to read 'host_address' from configuration file ./server.ini!" );
        return -1;
    }
  g_key_shard_generation_path = cfg.get_string("server", "key_shard_generation_path");
    if ( listen_addr.length() == 0 ) {
        INFO_OUTPUT_CONSOLE( "Failed to read 'key_shard_generation_path' from configuration file ./server.ini!" );
        return -1;
    }
    g_key_shard_query_path = cfg.get_string( "server", "key_shard_query_path");
    if ( listen_addr.length() == 0 ) {
        INFO_OUTPUT_CONSOLE( "Failed to read 'key_shard_query_path' from configuration file ./server.ini!" );
        return -1;
    }
    log_enable = cfg.get_int( "log", "log_enable" );
    log_path = cfg.get_string( "log", "log_path" );
    INFO_OUTPUT_CONSOLE( "Configuration file ./server.ini is loaded!" );

    // Initialize log file
    if ( 1 == log_enable ) {
        if ( log_path.length() == 0 ) {
            INFO_OUTPUT_CONSOLE( "Log service is enabled, but failed to read 'log_path' from configuration file ./server.ini!" );
        }
        glog_helper = new TeeLogHelper( log_path.c_str(), PROJECT_NAME );
    }
    INFO_OUTPUT_CONSOLE( "Log service is enabled and log file will be written in %s!", log_path.c_str() );

    // Initialize enclave
    sgx_status = sgx_create_enclave( (const char*)argv[1], 0, nullptr, nullptr, &global_eid, nullptr );
    if (sgx_status != SGX_SUCCESS ) {
        ERROR("Initialize enclave failed! enclave file: %s, error message: %s", argv[1], t_strerror((int)sgx_status));
        ret = -1;
        goto _exit;
    }
    sgx_status = ecall_get_enclave_id( global_eid, &ret, &enclave_id, &enclave_id_len );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        ERROR("ecall_get_enclave_id() failed, error message: %s", t_strerror((int)sgx_status));
        ret = -1;
        goto _exit;
    }
    INFO_OUTPUT_CONSOLE( "Enclave is created! Enclave ID: %s", enclave_id );

    sgx_status = ecall_init( global_eid, &ret );
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        ERROR( "ecall_init() failed, error message: %s", t_strerror((int)sgx_status) );
        ret = -1;
        goto _exit;
    }
    INFO_OUTPUT_CONSOLE( "ecall_init() is called successfully!");
    
    // Initializing the REST service listener
    HttpServer_Start( listen_addr );

    // End server until any key is pressed
    INFO_OUTPUT_CONSOLE("Press ENTER to exit.");
    getline(std::cin, input_line);

_exit:
    // Server exit
    INFO_OUTPUT_CONSOLE( "Server closed!");
    HttpServer_Shutdown();

    // Free enclave
    ecall_free( global_eid );

    // Destroy enclave
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
