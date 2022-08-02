#ifndef TEE_ARWEAVE_SERVER_THREAD_TASK_H
#define TEE_ARWEAVE_SERVER_THREAD_TASK_H

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>


#include <sys/time.h>

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/json.h"
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"
#include "cpprest/producerconsumerstream.h"

#pragma warning ( push )
#pragma warning ( disable : 4457 )
#pragma warning ( pop )
#include <locale>
#include <ctime>

#include "thrdpool.h"
#include <iostream>
#include <stdio.h>
#include "list.h"
#include "EnclaveAccess.h"
#include "CallBack.h"
#include "CJsonObject.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

struct taskpar {
    vector<string> pubkey_list;
    int k;
    int l;
    int key_length;
    string request_id;
};


/**
 * The thread task for key shard creation.
 * @param [in]context the context of thread task
 */
void RoutineThread(void *context);

/**
 * Execute this function for tasks that are not yet executed in the thread pool at the end of the program.
 * @param [in]task the context of thread task
 */
void PendingTask(const struct thrdpool_task *task);

/**
 * Clear the cache in map.
 * @param [in]context NULL
 */
void ClearMapCache(void* context);
#endif //TEE_ARWEAVE_SERVER_THREAD_TASK_H
