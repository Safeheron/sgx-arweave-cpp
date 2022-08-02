#ifndef TEE_ARWEAVE_SERVER_DATA_VERIFY_H
#define TEE_ARWEAVE_SERVER_DATA_VERIFY_H

#include <string>
#include "thrdtask/thread_task.h"

#define USER_PUBLICKEY_LIST U("userPublicKeyList")
#define NUMERATOR_K U("k")
#define DENOMINATOR_L U("l")
#define KEY_LENGTH U("keyLength")
#define CALL_BACK_ADDRESS U("callBackAddress")

#define PUBKEY_LIST_HASH U("pubkey_list_hash")

/**
 * Verify inputs data, add public keys into a vector if the inputs are valid.
 * @param [in]message http request
 * @param [out]pubkey_list public key list
 * @param [out]k the numerator of threshold
 * @param [out]l the denominator of threshold
 * @param [out]key_length the key length
 * @return true on success, false on failure
 */
bool VerifyData(const http_request& message, vector<string>& pubkey_list, int& k, int& l, int& key_length);

/**
 * Check if the related key in the body.
 * @param [in]body_json the body value in Json format of the request
 * @return true on success, false on failure
 */
bool KeyExist(json::value& body_json);

/**
 * Check if the inputs is valid.
 * @param body_json the body value in Json format of the request
 * @return true on success, false on failure
 */
bool InputValid(json::value& body_json);

/**
 * Verify the public key.
 * @param public_key
 * @return true on success, false on failure
 */
bool VerifyPublicKey(const string& public_key);

#endif //TEE_ARWEAVE_SERVER_DATA_VERIFY_H
