#include "keyshard_param.h"
#include <cpprest/http_client.h>
#include <safeheron/crypto-encode/hex.h>
#include <safeheron/crypto-curve/curve.h>
#include <crypto-hash/sha256.h>

using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using namespace web;    // for json

KeyShardParam::KeyShardParam()
{

}
KeyShardParam::KeyShardParam(
    const std::string & json_str )
{
    web::json::value root = json::value::parse( json_str );

    // node "user_public_key_list"
    auto array = root.at(FIELD_NAME_USER_PUBLICKEY_LIST ).as_array();
    for ( const auto & value : array ) {
        pubkey_list_.emplace_back( value.as_string() );
    }

    // node "k"
    k_ = root.at(FIELD_NAME_NUMERATOR_K ).as_integer();

    // node "l"
    l_ = root.at(FIELD_NAME_DENOMINATOR_L ).as_integer();

    // node "key_length"
    key_length_ = root.at(FIELD_NAME_KEY_LENGTH ).as_integer();

    // node "webhook_url"
    webhook_url_ = root.at(FIELD_NAME_WEBHOOK_URL ).as_string();
}
KeyShardParam::KeyShardParam(
    PUBKEY_LIST& pubkey_list, 
    int k, int l, int key_length, 
    const std::string & callback )
{
    pubkey_list_ = pubkey_list;
    k_ = k;
    l_ = l;
    key_length_ = key_length;
  webhook_url_ = callback;
}
KeyShardParam::~KeyShardParam()
{

}

// User public key must be a P256 curve point
bool KeyShardParam::pubkey_list_is_ok()
{
    if ( pubkey_list_.size() != l_) return false;

    for ( auto & pubkey : pubkey_list_ ) {
        CurvePoint pub;
        std::string pub_key_65;

        // In hexadecimal
        if ( pubkey.size() % 2 != 0 ) return false;

        // The length must be 65 bytes
        pub_key_65 = safeheron::encode::hex::DecodeFromHex( pubkey );
        if ( pub_key_65.empty() ) return false;

        // It must be a P256 curve point
        if ( !pub.DecodeFull((uint8_t *)pub_key_65.c_str(), CurveType::P256) ) return false;
    }
    return true;
}

// The threshold numerator must be greater than 0
// The threshold numerator must be greater than half of threshold denominator
// The threshold numerator must be smaller than or equal to the threshold denominator
bool KeyShardParam::k_is_ok()
{
    if ( k_ <= 0 || k_ < l_/2 + 1 || k_ > l_ )
        return false;
    return true;
}

// The threshold denominator must be greater than 1
// The threshold denominator must be smaller than 21
bool KeyShardParam::l_is_ok()
{
    if ( l_ <= 1 || l_ >= 21 )
        return false;
    return true;
}

// Supported RSA key length: 1024/2048/3072/4096 
bool KeyShardParam::key_length_is_ok()
{
    if ( key_length_ != 1024 && key_length_ != 2048 && 
         key_length_ != 3072 && key_length_ != 4096 ) 
         return false;
    return true;
}

// Callback address MUST not be null!
bool KeyShardParam::webhook_url_is_ok()
{
    return webhook_url_.length() == 0 ? false : true;
}

// Calculate the hash of all public keys in list
std::string KeyShardParam::calc_pubkey_list_hash()
{
    std::string pubkey_str;

    if ( pubkey_list_.size() == 0 ) {
        return "";
    }

    std::sort( pubkey_list_.begin(), pubkey_list_.end() );
    for ( int i = 0; i < pubkey_list_.size(); ++i ) {
        pubkey_str += pubkey_list_[i];
    }

    safeheron::hash::CSHA256 pub_hash_sha256;
    uint8_t digest[safeheron::hash::CSHA256::OUTPUT_SIZE];
    pub_hash_sha256.Write( (const uint8_t *)pubkey_str.c_str(), pubkey_str.size() );
    pub_hash_sha256.Finalize( digest );
    return safeheron::encode::hex::EncodeToHex( digest, safeheron::hash::CSHA256::OUTPUT_SIZE );
}

// Serialize parameters to a JSON string
std::string KeyShardParam::to_json_string( )
{
    json::value root = json::value::object( true );
    std::vector<json::value> pubkey_array;
    for ( int i = 0; i < pubkey_list_.size(); ++i ) {
        pubkey_array.push_back( json::value(pubkey_list_[i]) );
    }
    root[FIELD_NAME_USER_PUBLICKEY_LIST] = json::value::array(pubkey_array );
    root[FIELD_NAME_NUMERATOR_K] = json::value(k_ );
    root[FIELD_NAME_DENOMINATOR_L] = json::value(l_ );
    root[FIELD_NAME_KEY_LENGTH] = json::value(key_length_ );
    return root.serialize();
}
