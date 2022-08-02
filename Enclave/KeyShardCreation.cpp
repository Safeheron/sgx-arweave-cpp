#include "KeyShardCreation.h"
#include <common/log_t.h>
#include "Enclave_t.h"
#include <string>
#include "sgx_report.h"
#include <pthread.h>

extern std::mutex enclave_mutex;
extern thread_local char request_id[100];
extern std::map<std::string, KeyContext*> current_alive_key_context;

EnclaveCreateKeyShard::EnclaveCreateKeyShard(int k_in, int l_in, int key_length_in) : k_(k_in), l_(l_in), key_length_(key_length_in){

}

EnclaveCreateKeyShard::~EnclaveCreateKeyShard(){

}

bool EnclaveCreateKeyShard::GetPubkeyHash(const char* pubkey_list){
    std::string all_pubkey;
    CJsonObject cjson_pubkey_list;

    /** Parse the string and construct a Json object */
    ok_ = cjson_pubkey_list.Parse(pubkey_list);
    if (!ok_) return false;

    ok_ = cjson_pubkey_list.GetArray("pubkey_list", pubkey_list_);
    if (!ok_) return false;

    std::sort(this->pubkey_list_.begin(), this->pubkey_list_.end());

    /** Combine the public keys from the public key list into a string */
    for (int i = 0; i < this->pubkey_list_.size(); ++i)
        all_pubkey += this->pubkey_list_[i];

    /** Hash all_pubkey string */
    ok_ = get_sha256_hash(all_pubkey, this->pubkey_list_hash_);
    if (!ok_) {
        ERROR("Request ID: %s, get_sha256_hash failed", request_id);
        return false;
    }
    return true;
}

bool EnclaveCreateKeyShard::CreateKeyContext() {

    std::lock_guard<std::mutex> lock(enclave_mutex);

    if (current_alive_key_context.count(pubkey_list_hash_)) {
        return false;
    }

    key_context_ = new KeyContext;

    /** Get current system time */
    long int *time_now = nullptr;
    ocall_get_system_time(&time_now);

    key_context_->key_length = key_length_;
    key_context_->k = k_;
    key_context_->l = l_;
    key_context_->key_status = STATUS_RUNNING;
    key_context_->start_time = *time_now;

    current_alive_key_context.insert(std::pair<std::string, KeyContext*>(pubkey_list_hash_, key_context_));
    ocall_free_long(time_now);
    return true;
}

bool EnclaveCreateKeyShard::GenerateRSAKey() {
    RSAPublicKey pub;
    RSAKeyMeta key_meta;
    std::string key_meta_hash;

    ok_ = safeheron::tss_rsa::GenerateKey(key_length_, l_, k_, priv_arr_, pub, key_meta);
    if (!ok_) {
        ERROR("Request ID: %s, GenerateKey failed: %d, %d, %d", request_id, key_length_, k_, l_);
        return false;
    }

    ok_ = GetKeyMetaHash(key_meta, key_meta_hash);
    if (!ok_) {
        ERROR("Request ID: %s, GenerateKey failed: %d, %d, %d", request_id, key_length_, k_, l_);
        return false;
    }

    std::lock_guard<std::mutex> lock(enclave_mutex);
    if (current_alive_key_context.count(pubkey_list_hash_))
        current_alive_key_context.at(pubkey_list_hash_)->key_meta_hash = key_meta_hash;

    ok_ = key_meta.ToJsonString(key_meta_str_);
    if (!ok_) {
        ERROR("Request ID: %s, GenerateKey failed: %d, %d, %d", request_id, key_length_, k_, l_);
        return false;
    }

    return true;
}

bool EnclaveCreateKeyShard::GetKeyMetaHash(const RSAKeyMeta& key_meta, std::string& key_meta_hash) {
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

    /** Hash key mata string */
    ok_ = get_sha256_hash(key_mata, key_meta_hash);
    if (!ok_) return false;

    return true;
}

bool EnclaveCreateKeyShard::FinalDataReturn(std::string& first_result) {
    int i = 0;
    std::string encrypt_key_info;
    CJsonObject cjson_key_shard_pkg;

    ok_ = cjson_creation_result_.CreateObject();
    if (!ok_) return false;

    ok_ = cjson_creation_result_.Add("pubkey_list_hash", pubkey_list_hash_);
    if (!ok_) return false;

    ok_ = cjson_key_shard_pkg.CreateArray();
    if (!ok_) return false;

    for (const auto& pri : priv_arr_){
        ++i;
        CJsonObject cjson_array_element;

        ok_ = cjson_array_element.CreateObject();
        if (!ok_) return false;

        ok_ = cjson_array_element.Add("public_key", pubkey_list_[i - 1]);
        if (!ok_) return false;

        ok_ = GetEncryptKeyInfo(i, pri, encrypt_key_info);
        if (!ok_) return false;

        ok_ = cjson_array_element.Add("encrypt_key_info", encrypt_key_info);
        if (!ok_) return false;

        ok_ = cjson_key_shard_pkg.Add(cjson_array_element);
        if (!ok_) return false;
    }

    ok_ = cjson_creation_result_.Add("key_shard_pkg", cjson_key_shard_pkg);
    if (!ok_) return false;

    ok_ = cjson_creation_result_.ToString(first_result);
    if (!ok_) return false;

    return true;
}

bool EnclaveCreateKeyShard::GetEncryptKeyInfo(int i, const RSAPrivateKeyShare& pri, std::string& encrypt_key_info) {
    CJsonObject cjson_key_info;
    CJsonObject cjson_key_shard;
    CJsonObject cjson_key_meta;
    std::string key_info;
    std::string key_shard_string;

    ok_ = cjson_key_info.CreateObject();
    if (!ok_) return false;

    ok_ = cjson_key_meta.Parse(key_meta_str_);
    if (!ok_) return false;

    ok_ = cjson_key_info.Add("key_meta", cjson_key_meta);
    if (!ok_) return false;

    ok_ = cjson_key_shard.CreateObject();
    if (!ok_) return false;

    ok_ = cjson_key_shard.Add("index", i);
    if (!ok_) return false;

    pri.si().ToHexStr(key_shard_string);

    ok_ = cjson_key_shard.Add("private_key_shard", key_shard_string.c_str());
    if (!ok_) return false;

    ok_ = cjson_key_info.Add("key_shard", cjson_key_shard);
    if (!ok_) return false;

    ok_ = cjson_key_info.ToString(key_info);
    if (!ok_) return false;

    ok_ = EncryptKeyShard(key_info, i, encrypt_key_info);
    if (!ok_) return false;

    return true;
}

bool EnclaveCreateKeyShard::EncryptKeyShard(const std::string& key_info, int i, std::string& encrypt_key_info) {

    ECIES enc;
    CurvePoint pub;
    std::string cypher;
    std::string data;

    enc.set_curve_type(CurveType::P256);

    /** Convert public keys format */
    std::string pub_key_65 = safeheron::encode::hex::DecodeFromHex(pubkey_list_[i - 1]);
    ok_ = pub.DecodeFull((uint8_t *)pub_key_65.c_str(), CurveType::P256);
    if (!ok_) {
        ERROR("Request ID: %s, pub DecodeFull failed.", request_id);
        return false;
    }

    /** Encrypt key_info by using public key */
    ok_ = enc.EncryptPack(pub, key_info, cypher);
    if (!ok_) {
        ERROR("Request ID: %s, encrypt key_info message failed.", request_id);
        return false;
    }

    encrypt_key_info = safeheron::encode::hex::EncodeToHex(cypher);
    return true;
}


void EnclaveCreateKeyShard::ChangeKeyStatus(int key_status) {
    std::lock_guard<std::mutex> lock(enclave_mutex);
    key_context_->key_status = key_status;
}