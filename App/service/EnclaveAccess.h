#ifndef TEE_ARWEAVE_SERVER_ENCLAVEACCESS_H
#define TEE_ARWEAVE_SERVER_ENCLAVEACCESS_H

#include "CJsonObject.h"
#include <iostream>
#include <map>
#include "quote_generation.h"
#include <log_u.h>
#include "tee_error.h"
using namespace std;


class EnclaveAccess {
public:
    /**
     * Enter the enclave to create key shards and output the result in Json format string.
     * @param [in]pubkey_list the string in Json format of public key list
     * @param [in]k the numerator of threshold
     * @param [in]l the denominator of threshold
     * @param [in]key_length key length
     * @param [out]final_result the string in Json format of the creation result
     * @return true on success, false on failure
     */
    static bool CreateKeyShard(const string& request_id, const string& pubkey_list, int k, int l, int key_length, string& final_result);

    /**
     * Enter enclave to query key shards status and output the reply in Json format string.
     * @param pubkey_list_hash the hash string of public key list
     * @param reply_message query result string in Json format
     * @return true on success, false on failure
     */
    static bool QueryKeyShareStatus(const string& pubkey_list_hash, string& reply_message);
};


#endif //TEE_ARWEAVE_SERVER_ENCLAVEACCESS_H
