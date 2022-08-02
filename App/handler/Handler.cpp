#include <thread>
#include "Handler.h"
#include "Enclave_u.h"

extern std::string interface_create;
extern std::string interface_query;
extern thrdpool_t* pool;
extern sgx_enclave_id_t global_eid;


handler::handler(string url) : m_listener_(url) {

    /** Initialize the thread pool */
    pool = thrdpool_create(5, 1024);

    /** Add a timer task to the thread pool to clear the cache in the memory */
    thrdpool_task clear_map_task;
    clear_map_task.routine = &ClearMapCache;
    clear_map_task.context = NULL;

    int ret = thrdpool_schedule(&clear_map_task, pool);
    /** Failed to add task */
    if (ret == -1) {
        ERROR("start ClearMapCache() failed.");
        exit(-1);
    }

    m_listener_.support(methods::POST, std::bind(&handler::HandlePost, this, std::placeholders::_1));

}

handler::~handler() {

}


void handler::HandlePost(http_request message) {

    if (message.request_uri().path() == interface_create) {

        /** Handle the creation request */
        HandleCreateKeyRequest(message);

    } else if (message.request_uri().path() == interface_query) {

        /** Handle the query request */
        HandleQueryKeyRequest(message);

    }

};





