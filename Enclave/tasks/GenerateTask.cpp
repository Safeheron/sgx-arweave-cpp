#include "GenerateTask.h"
#include "TaskConstant.h"
#include "common/tee_util.h"
#include "common/tee_error.h"
#include "common/log_t.h"
#include "json/json.h"
#include <crypto-curve/curve.h>
#include <crypto-ecies/ecies.h>
#include <crypto-encode/base64.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <map>


#include "Enclave_t.h"

using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::ecies::ECIES;

extern std::mutex g_list_mutex;
extern std::map<std::string, KeyShardContext*> g_keyContext_list;

int GenerateTask::get_task_type( )
{
    return eTaskType_Generate;
}

int GenerateTask::execute( 
    const std::string & request_id, 
    const std::string & request, 
    std::string & reply, 
    std::string & error_msg )
{
    int ret = 0;
    int k, l, key_bits;
    JSON::Root req_root;
    PUBKEY_LIST input_pubkey_list;
    KeyShardContext* context = nullptr;
    std::string pubkey_hash;
    std::string key_meta_hash;
    PRIVATE_KEYSHARD_LIST private_key_list;
    RSAPublicKey pubkey;
    RSAKeyMeta key_meta;

    FUNC_BEGIN;

    request_id_ = request_id;
    if (request.length() == 0) {
        error_msg = format_msg( "Request ID: %s, request is null!", request_id_.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_INVALID_PARAMETER;
    }

    std::lock_guard<std::mutex> lock( g_list_mutex );

    // Return busy if the size of key context list is bigger than 1000
    if ( g_keyContext_list.size() >= MAX_TASK_COUNT ) {
        error_msg = format_msg( "Request ID: %s, Key context list exceeds its maximum capacity! Current size is: %d",
            request_id_.c_str(), (int)g_keyContext_list.size() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_ENCLAVE_IS_BUSY;
    }

    g_list_mutex.unlock();

    // Parse request parameters from request body data
    req_root = JSON::Root::parse( request );
    if ( !req_root.is_valid() ) {
        error_msg = format_msg( "Request ID: %s, request body is not in JSON! request: %s",
            request_id_.c_str(), request.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_INVALID_PARAMETER;
    }
    k = req_root["k"].asInt();
    l = req_root["l"].asInt();
    key_bits = req_root["key_length"].asInt();
    input_pubkey_list = req_root["user_public_key_list"].asStringArrary();
    INFO("Request ID: %s, k: %d l: %d, keyLength: %d, pubkey count: %d", 
        request_id_.c_str(), k, l, key_bits, (int)input_pubkey_list.size());
    for (const auto& it : input_pubkey_list) 
        INFO("Request ID: %s, pubkey: %s", request_id_.c_str(), it.c_str());

    // Calculate users' public key hash (SHA256) after sorting "user_public_key_list"
    std::sort( input_pubkey_list.begin(), input_pubkey_list.end() );
    if ( (ret = get_pubkey_hash( input_pubkey_list, pubkey_hash )) != TEE_OK ) {
        error_msg = format_msg( "Request ID: %s, get_pubkey_hash() failed with input_pubkey_list! ret : 0x%x", 
            request_id_.c_str(), ret );
        ERROR( "%s", error_msg.c_str() );
        return ret;
    }

    // Check if there is a same public key list hash in context list
    // Return if there is, even if its status is finished!!!
    g_list_mutex.lock();
    if ( g_keyContext_list.count( pubkey_hash ) ) {
        error_msg = format_msg( "Request ID: %s, a same request is in queue!", request_id_.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_REQUEST_IS_EXIST;
    }
    g_list_mutex.unlock();

    // Construct a KeyShardContext object and add it into g_keyContext_list.
    if ( !(context = new KeyShardContext( k, l, key_bits )) ) {
        error_msg = format_msg( "Request ID: %s, new KeyShardContext failed!", request_id_.c_str() );
        ERROR( "%s", error_msg.c_str() );
        return TEE_ERROR_MALLOC_FAILED;
    }
    context->start_time = get_system_time();
    context->key_status = eKeyStatus_Generating;
    g_list_mutex.lock();
    g_keyContext_list.insert(std::pair<std::string, KeyShardContext*>(pubkey_hash, context));
    g_list_mutex.unlock();

    // Create key shards by calling Safeheron API
    if ( !(ret = safeheron::tss_rsa::GenerateKey(key_bits, l, k, private_key_list, pubkey, key_meta )) ) {
        error_msg = format_msg( "Request ID: %s, GenerateKey failed!", request_id_.c_str() );
        ERROR( "%s", error_msg.c_str() );
        context->key_status = eKeyStatus_Error;
        return false;
    }

    // Calculate the hash of key meta
    if ( (ret = get_keymeta_hash( key_meta, key_meta_hash )) != TEE_OK ) {
        error_msg = format_msg( "Request ID: %s, get_pubkey_hash() failed with key_mata! ret: 0x%x", 
            request_id_.c_str(), ret );
        ERROR( "%s", error_msg.c_str() );
        context->key_status = eKeyStatus_Error;
        return ret;
    }
    context->key_meta_hash = key_meta_hash;

    // Construct reply JSON string
    if ( (ret = get_reply_string(pubkey_hash, input_pubkey_list,
                                 pubkey, private_key_list, key_meta, reply )) != TEE_OK ) {
        error_msg = format_msg( "Request ID: %s, get_reply_string() failed! ret: 0x%x", 
            request_id_.c_str(), ret );
        ERROR( "%s", error_msg.c_str() );
        context->key_status = eKeyStatus_Error;
        return ret;
    }
    context->key_status = eKeyStatus_Finished;
    context->finished_time = get_system_time();

    FUNC_END;

    return ret;
}

// Calculate the hash of the public key list
int GenerateTask::get_pubkey_hash( 
    const PUBKEY_LIST & pubkey_list, 
    std::string & hash_hex )
{
    int ret = 0;
    std::string pubkey_string;

    FUNC_BEGIN;

    if ( pubkey_list.size() == 0 ) {
        ERROR( "Request ID: %s, pubkey_list is null!", request_id_.c_str() );
        return TEE_ERROR_INVALID_PARAMETER;
    }

    // Combine the public keys from the public key list into a string
    for ( int i = 0; i < pubkey_list.size(); ++i ) {
        pubkey_string += pubkey_list[i];
    }

    // Hash pubkey_string string
    if ( !sha256_hash(pubkey_string, hash_hex) ) {
        ERROR( "Request ID: %s, get_sha256_hash() failed with pubkey_list!", request_id_.c_str() );
        return TEE_ERROR_CALC_HASH_FAILED;
    }

    FUNC_END;
 
    return TEE_OK;
}

// Calc the hash of a RSAKeyMeta content
int GenerateTask::get_keymeta_hash( 
    const RSAKeyMeta & key_meta, 
    std::string & hash_hex )
{
    std::string vk_temp;
    std::string key_mata;

    key_mata += std::to_string(key_meta.k());
    key_mata += std::to_string(key_meta.l());

    key_meta.vkv().ToHexStr(vk_temp);
    key_mata += vk_temp;
    key_meta.vku().ToHexStr(vk_temp);
    key_mata += vk_temp;

    for (const auto& vki : key_meta.vki_arr()) {
        vki.ToHexStr(vk_temp);
        key_mata += vk_temp;
    }

    if ( !sha256_hash(key_mata, hash_hex) ) {
        ERROR( "Request ID: %s, get_sha256_hash() failed with key_meta!", request_id_.c_str() );
        return TEE_ERROR_CALC_HASH_FAILED;
    }

    return TEE_OK;
}

int GenerateTask::get_reply_string( 
    const std::string & input_pubkey_hash, 
    const PUBKEY_LIST & input_pubkey_list,
    const RSAPublicKey & pubkey,
    const PRIVATE_KEYSHARD_LIST & private_key_list,
    const RSAKeyMeta & key_meta,  
    std::string & out_str )
{
    int ret = 0;
    int index = 0;
    JSON::Root root;
    std::list<JSON::Root> pkg_array;

    FUNC_BEGIN;

    // Add string "pubkey_list_hash" to JSON object root
    root["pubkey_list_hash"] = input_pubkey_hash;

    // add string "key_shard_pubkey" to JSON object root
    std::string pubkey_json_str;
    pubkey.ToJsonString( pubkey_json_str );
    JSON::Root pubkey_root = JSON::Root::parse( pubkey_json_str );
    root["key_shard_pubkey"] = pubkey_root;

    // Add JSON list "key_shard_pkg" to JSON object root
    for ( const auto& prikey : private_key_list ) {
        JSON::Root arrary_node;
        std::string keyInfo_cipher;

        ++index;

        // Encrypt the private key shard and key meta
        if ( (ret = get_private_key_info_cipher(index, input_pubkey_list[index - 1],
                                                prikey, key_meta, keyInfo_cipher)) != TEE_OK ) {
            ERROR( "Request ID: %s, get_private_key_info_cipher() failed with index: %d!", request_id_.c_str(), index );
            return ret;
        }

        // Add the element to "key_shard_pkg"
        arrary_node["public_key"] = input_pubkey_list[index - 1];
        arrary_node["encrypt_key_info"] = keyInfo_cipher;
        pkg_array.push_back( arrary_node );
    }
    root["key_shard_pkg"] = pkg_array;

    // return JSON string
    out_str = JSON::Root::write( root );

    FUNC_END;

    return TEE_OK;
}

int GenerateTask::get_private_key_info_cipher(
    int index, 
    const std::string & input_pubkey,
    const RSAPrivateKeyShare & private_key,
    const RSAKeyMeta & key_meta,  
    std::string & out_str )
{
    int ret = 0;
    std::string key_meta_str;
    std::string key_shard_str;
    std::string plain_str;
    std::string pub_key_65;
    std::string ecies_cipher;
    JSON::Root root;
    JSON::Root key_meta_node;
    JSON::Root key_shard_node;
    ECIES ecies;
    CurvePoint ec_pubkey;

    FUNC_BEGIN;

    // Parse the key meta string into a JSON object key_meta_node
    // and add JSON object key_meta_node named "key_meta" to JSON object root
    key_meta.ToJsonString( key_meta_str );
    key_meta_node = JSON::Root::parse( key_meta_str );
    if ( !key_meta_node.is_valid() ) {
        ERROR( "Request ID: %s, JSON::Root::parse() failed with key_meta!", request_id_.c_str() );
        ERROR( "Request ID: %s, key_meta: %s", request_id_.c_str(), key_meta_str.c_str() );
        return TEE_ERROR_KEYMETA_IS_WRONG;
    }
    root["key_meta"] = key_meta_node;

    // Add "index" and "private_key_shard" into JSON object key_shard_node
    // Add JSON object key_shard_node named "key_shard" into JSON object root
    private_key.si().ToHexStr(key_shard_str);
    key_shard_node["index"] = index;
    key_shard_node["private_key_shard"] = key_shard_str;
    root["key_shard"] = key_shard_node;

    // Get JSON string
    plain_str = JSON::Root::write( root );

    // Construct a CurvePoint object from input public key hex string
    pub_key_65 = safeheron::encode::hex::DecodeFromHex( input_pubkey );
    if ( !ec_pubkey.DecodeFull( (uint8_t *)pub_key_65.c_str(), CurveType::P256 ) ) {
        ERROR( "Request ID: %s, CurvePoint::DecodeFull failed with index: %d!", request_id_.c_str(), index );
        ERROR( "Request ID: %s, input pubkey: %s", request_id_.c_str(), input_pubkey.c_str() );
        return TEE_ERROR_PUBKEY_IS_WRONG;
    }

    // Encrypt plain_str by input public key
    if ( !ecies.EncryptPack( ec_pubkey, plain_str, ecies_cipher ) ) {
        ERROR( "Request ID: %s, encrypt key_info message failed.", request_id_.c_str() );
        return TEE_ERROR_ECIES_ENC_FAILED;
    }

    // Convert cipher data to hex string
    out_str = safeheron::encode::hex::EncodeToHex( ecies_cipher );

    FUNC_END;

    return TEE_OK;
}