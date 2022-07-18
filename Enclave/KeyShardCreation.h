//
// Created by edy on 2022/7/1.
//

#ifndef TEE_ARWEAVE_SERVER_ECLAVECLASS_H
#define TEE_ARWEAVE_SERVER_ECLAVECLASS_H
#include "common/CJsonObject.h"
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <unistd.h>
#include <crypto-tss-rsa/tss_rsa.h>
#include <crypto-tss-rsa/RSAPrivateKeyShare.h>
#include <crypto-tss-rsa/RSAPublicKey.h>
#include <crypto-tss-rsa/RSASigShare.h>
#include <crypto-tss-rsa/RSAKeyMeta.h>
#include <crypto-tss-rsa/KeyGenParam.h>
#include <crypto-bn/bn.h>
#include <crypto-bn/rand.h>
#include <exception/safeheron_exceptions.h>
#include <crypto-tss-rsa/tss_rsa.h>
#include "../3rdparty/tee-util-sgx-1.0.0/include/tee-util-sgx/cJSON.h"
#include <crypto-hash/sha256.h>
#include <cstring>
#include "crypto-bn/rand.h"
#include "crypto-curve/curve.h"
#include "crypto-encode/hex.h"
#include "crypto-ecies/ecies.h"
#include "crypto-ecies/auth_enc.h"
#include "common/enclave_util.h"
#include "common/CJsonObject.h"


using safeheron::tss_rsa::RSAPrivateKeyShare;
using safeheron::tss_rsa::RSAPublicKey;
using safeheron::tss_rsa::RSAKeyMeta;
using safeheron::tss_rsa::RSASigShare;
using safeheron::tss_rsa::KeyGenParam;
using safeheron::exception::LocatedException;
using safeheron::exception::OpensslException;
using safeheron::exception::BadAllocException;
using safeheron::exception::RandomSourceException;
using safeheron::bignum::BN;
using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::ecies::ECIES;
using safeheron::ecies::AuthEnc;


#define STATUS_RUNNING       1
#define STATUS_FINISHED      2
#define STATUS_DESTROY       3
#define STATUS_ERROR         4

#define ERROR_PUBLIST_KEY_HASH      -1
#define ERROR_CREATE_KEYCONTEXT     -2
#define ERROR_GENERATE_KEYSHARDS    -3
#define ERROR_FINAL_RETURN          -4
#define ERROR_MALLOC_OUTSIDE        -5

struct KeyContext {
    int key_status = 0;
    long start_time = 0;
    int k = 0;
    int l = 0;
    int key_length = 0;
    std::string key_meta_hash;
};


class EnclaveCreateKeyShard {

public:
    /**
     * Constructor.
     * @param[in] k_in the numerator of threshold
     * @param[in] l_in the denominator of threshold
     * @param[in] key_length_in
     */
    EnclaveCreateKeyShard(int k_in, int l_in, int key_length_in);
    ~EnclaveCreateKeyShard();

    /**
     * Get the public key list hash.
     * @param pubkey_list
     * @return true on success, false on failure
     */
    bool GetPubkeyHash(const char* pubkey_list);

    /**
     * Create key context.
     * @return true on success, false on failure
     */
    bool CreateKeyContext();

    /**
     * Generate key shards and get the key meta hash.
     * @return true on success, false on failure
     */
    bool GenerateRSAKey();

    /**
     * Generate the creation result string in Json format which is without report.
     * @param[out] first_result
     * @return true on success, false on failure
     */
    bool FinalDataReturn(std::string& first_result);

    /**
     * Combine the creation results into a Json object and encrypt key shards information.
     * @param[in] i the index of the key shard
     * @param[in] pri the private key shard
     * @param[out] encrypt_key_info encrypted key shards information
     * @return true on success, false on failure
     */
    bool GetEncryptKeyInfo(int i, const RSAPrivateKeyShare& pri, std::string& encrypt_key_info);

    /**
     * Get key meta hash.
     * @param[in] key_meta
     * @param[out] key_meta_hash
     * @return true on success, false on failure
     */
    bool GetKeyMetaHash(const RSAKeyMeta& key_meta, std::string& key_meta_hash);

    /**
     * Encrypt key shards information.
     * @param[in] key_info
     * @param[in] i
     * @param[out] encrypt_key_info
     * @return true on success, false on failure
     */
    bool EncryptKeyShard(const std::string& key_info, int i, std::string& encrypt_key_info);

    /**
     * Change the key status.
     * @param[in] key_status
     */
    void ChangeKeyStatus(int key_status);

private:
    bool ok_;
    int k_;
    int l_;
    int key_length_;
    std::vector<RSAPrivateKeyShare> priv_arr_;
    std::vector<std::string> pubkey_list_;
    std::string pubkey_list_hash_;
    std::string key_meta_str_;
    KeyContext* key_context_ = nullptr;
    CJsonObject cjson_creation_result_;
};


#endif //TEE_ARWEAVE_SERVER_ECLAVECLASS_H
