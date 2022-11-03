/**
 * @file GenerateTask.h
 * @brief : GenerateTask.h contains the execution of generation requests. The process has 3 steps.
 *          Firstly, The public key list hash is calculated according to the "user_public_key_list"
 *          from request body and Safeheron's API is called to generate the private key shards.
 *          Then, the private key shards' information is encrypted using the public key from "user_public_key_list".
 *          Finally, all the results are packed into a JSON structure and returned.
 *
 * @copyright Copyright (c) 2022
 * 
 */

#include "../shell/Dispatcher.h"
#include <crypto-tss-rsa/tss_rsa.h>
#include <crypto-tss-rsa/RSAPrivateKeyShare.h>
#include <crypto-tss-rsa/RSAPublicKey.h>
#include <crypto-tss-rsa/RSASigShare.h>
#include <crypto-tss-rsa/RSAKeyMeta.h>
#include <crypto-tss-rsa/KeyGenParam.h>
#include <crypto-encode/hex.h>
 #include <string>
 #include <vector>

using safeheron::tss_rsa::RSAPrivateKeyShare;
using safeheron::tss_rsa::RSAPublicKey;
using safeheron::tss_rsa::RSAKeyMeta;
using safeheron::tss_rsa::RSASigShare;
using safeheron::tss_rsa::KeyGenParam;

 typedef std::vector<std::string> PUBKEY_LIST;
 typedef std::vector<RSAPrivateKeyShare> PRIVATE_KEYSHARD_LIST;

class GenerateTask: public Task
{
public:
    /**
     * @brief : The execution of generation tasks.
     * @param request_id[in] : The unique ID of each request.
     * @param request[in] : The request body, JSON string.
     * @param reply[out] : The key shard generation result, JSON string
     * @param error_msg[out] : An error message is returned when the execution is failed.
     * @return int : return 0 if success, otherwise return an error code.
     */
    virtual int execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg );

    /**
     * @brief : Get the current task type.
     * @return int : Return the task type number defined in TaskConstant.h
     */
    virtual int get_task_type( );

private:
    /**
     * @brief : Get the concatenation of public keys in "pubkey_list" using SHA256. The "pubkey_list" is sorted in advance.
     * @param request_id[in] : The unique ID of each request.
     * @param pubkey_list[in]
     * @param hash_hex[out]
     * @return int : Return TEE_OK if success, otherwise return an error code.
     */
    int get_pubkey_hash( const std::string & request_id, const PUBKEY_LIST & pubkey_list, std::string & hash_hex );

    /**
     * @brief : Get key meta hash.
     * @param request_id[in] : The unique ID of each request.
     * @param key_meta[in] : The collateral generated with the private key shards.
     * @param hash_hex[out]
     * @return int : Return TEE_OK if success, otherwise return an error code.
     */
    int get_keymeta_hash( const std::string & request_id, const RSAKeyMeta & key_meta, std::string & hash_hex );

    /**
     * @brief : Construct a JSON string after the key shard generation is done.
     *          The key shard information is encrypted and assigned to a JSON field named "encrypt_key_info".
     * @param request_id[in] : The unique ID of each request.
     * @param input_pubkey_hash[in] : The public key list hash.
     * @param input_pubkey_list [in] : The public key list.
     * @param pubkey [in] : The public key corresponding to the generated private key shards.
     * @param private_key_list[in] : The private key shard list. Each key shard will eb added into this list
     *                               after successful generation.
     * @param key_meta[in] : The collateral generated with the private key shards.
     * @param out_str[out] : A JSON string that represents the generation result.
     *                       The JSON structure is shown as below.
     *   {
     *     "pubkey_list_hash": "public key list hash",
     *     "key_shard_pubkey": "The public key corresponding to the generated private key shard",
     *     "key_shard_pkg": [
     *         {
     *             "public_key": "pubkey1 hex string",
     *             "encrypt_key_info": "cipher of private key shard 1 and key meta"
     *         },
     *         {
     *             "public_key": "pubkey2 hex string",
     *             "encrypt_key_info": "cipher of private key shard 2 and key meta"
     *         },
     *         {
     *             "public_key": "pubkey3 hex string",
     *             "encrypt_key_info": "cipher of private key shard 3 and key meta"
     *         }
     *      ]
     *   }
     *
     * @return int : Return TEE_OK if success, otherwise return an error code.
     */
    int get_reply_string( const std::string & request_id, const std::string & input_pubkey_hash, const PUBKEY_LIST & input_pubkey_list, const RSAPublicKey & pubkey, const PRIVATE_KEYSHARD_LIST & private_key_list, const RSAKeyMeta & key_meta, std::string & out_str );

    /**
     * @brief : Encrypt the key shards' information using the public key in public key list.
     *          The plain text is a JSON string and the JSON structure is shown as below:
     *   {
     *       "key_meta": {
     *                   "k": 2,
     *                   "l": 3,
     *                   "vkv": "lajksjkdja",
     *                   "vku": "iojkcjasjicj",
     *                   "vki_arr": [
     *                   "kasjhdkjashvalue",
     *                   "asjhdkjashdjavalue",
     *                   "looizjxcnaisvalue"
     *                   ]
     *               },
     *       "key_shard": {
     *                   "index": 1,
     *                   "private_key_shard": "kasdkajshdasd"
     *               }
     *   }
     * @param request_id[in] : The unique ID of each request.
     * @param index[in] : The index of the private key shard.
     * @param input_pubkey[in] : The public key to encrypt key shards' information.
     * @param private_key[in] : The private key shard.
     * @param key_meta[in] : The collateral generated with the private key shards.
     * @param out_str[out] : A string that represented the encrypted key shard information.
     * @return int : Return TEE_OK if success, otherwise return an error code.
     */
    int get_private_key_info_cipher( const std::string & request_id, int index, const std::string & input_pubkey, const RSAPrivateKeyShare & private_key, const RSAKeyMeta & key_meta, std::string & out_str );
};