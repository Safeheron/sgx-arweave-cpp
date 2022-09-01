/**
 * @file keyshard_param.h
 * @brief : KeyShardParam contains the parameters for generating key shards and the validator function of each parameter.
 *          There are several rules for these parameters:
 *          1. The number of the elements in user_public_key_list must equal the value of l;
 *          2. The value of k must be less than or equal the value of l;
 *          3. The value of k must be greater than the half of value of l;
 *          4. The value of l must be greater than or equal 2, less than or equal 20;
 *          5. The key_length is only in 1024, 2048, 3072, 4096.
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef _KEYSHARE_PARAM_H_
#define _KEYSHARE_PARAM_H_

#include <string>
#include <vector>

// JSON fields in HTTP request body
#define FIELD_NAME_USER_PUBLICKEY_LIST   "user_public_key_list"
#define FIELD_NAME_NUMERATOR_K           "k"
#define FIELD_NAME_DENOMINATOR_L         "l"
#define FIELD_NAME_KEY_LENGTH            "key_length"
#define FIELD_NAME_WEBHOOK_URL           "webhook_url"
#define FIELD_NAME_PUBKEY_LIST_HASH      "pubkey_list_hash"

typedef std::vector<std::string> PUBKEY_LIST;

class KeyShardParam
{
public:
    KeyShardParam();
    KeyShardParam(const std::string & json_str );
    KeyShardParam(PUBKEY_LIST& pubkey_list, int k, int l, int key_length, const std::string & callback );
    virtual ~KeyShardParam();
public:
    bool check_pubkey_list();
    bool check_k();
    bool check_l();
    bool check_key_length();
    bool check_webhook_url();
    std::string calc_pubkey_list_hash();
    std::string to_json_string();
public:
    PUBKEY_LIST pubkey_list_;   // User public key list, which collects the ECDSA public key of each user
    int k_;   // Threshold numerator
    int l_;   // Threshold denominator
    int key_length_;   // The length of the private key to be generated. Only in 1024, 2048, 3072 and 4096
    std::string webhook_url_;   // Webhook address, the generated results will be called back to this address via a webhook service
    std::string request_id_;   // Each request has a unique ID
};

#endif //_KEYSHARE_PARAM_H_