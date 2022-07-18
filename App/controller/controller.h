//
// Created by edy on 6/11/22.
//

#ifndef TEE_ARWEAVE_SERVER_CONTROLLER_H
#define TEE_ARWEAVE_SERVER_CONTROLLER_H

#include <mutex>
#include <iostream>
#include "Enclave_u.h"
#include <map>
#include "data_verify.h"
#include "../common/app_util.h"



/**
 * Add tasks to the thread pool.
 * @param [in]pubkey_list the public key list
 * @param [in]k the numerator of threshold
 * @param [in]l the denominator of threshold
 * @param [in]key_length the key length
 * @return true on success, false on failure
 */
bool AddTaskToPool(vector<string> &pubkey_list, int k, int l, int key_length);


/**
 * Handle the creation request.
 * @param [in]message http request
 */
void HandleCreateKeyRequest(http_request &message);

/**
 * Handle the query request.
 * @param [in]message http request
 */
void HandleQueryKeyRequest(http_request &message);

/**
 * the reply of any situation.
 * @param [in]message http request
 * @param [in]success the request is successful or not
 * @param [in]retrun_message message of the reply
 */
void MessageReply(http_request& message, bool success, const string& retrun_message);

/**
 * Get the unique ID of each creation request.
 * @param [in]pubkey_list the public key list
 * @param [out]pubkey_list_hash the hash of public key list
 */
void GetUniqueID(vector<string>& pubkey_list, string& pubkey_list_hash);


#endif //TEE_ARWEAVE_SERVER_CONTROLLER_H
