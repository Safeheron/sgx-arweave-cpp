/**
 * @file keyshare_param.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _KEYSHARE_PARAM_H_
#define _KEYSHARE_PARAM_H_

#include <string>
#include <vector>

// JSON node name string in HTTP request body
#define NODE_NAME_USER_PUBLICKEY_LIST   "userPublicKeyList"
#define NODE_NAME_NUMERATOR_K           "k"
#define NODE_NAME_DENOMINATOR_L         "l"
#define NODE_NAME_KEY_LENGTH            "keyLength"
#define NODE_NAME_CALL_BACK_ADDRESS     "callBackAddress"
#define NODE_NAME_PUBKEY_LIST_HASH      "pubkey_list_hash"

typedef std::vector<std::string> PUBKEY_LIST;

class KeyShareParam
{
public:
    KeyShareParam();
    KeyShareParam( const std::string & json_str );
    KeyShareParam( PUBKEY_LIST& pubkey_list, int k, int l, int key_length, const std::string & callback );
    virtual ~KeyShareParam();
public:
    bool pubkey_list_is_ok();
    bool k_is_ok();
    bool l_is_ok();
    bool key_length_is_ok();
    bool callback_is_ok();
    std::string calc_pubkey_list_hash();
    std::string to_json_string();
public:
    PUBKEY_LIST pubkey_list_;
    int k_;
    int l_;
    int key_length_;
    std::string callback_;
    std::string request_id_;
};

#endif //_KEYSHARE_PARAM_H_