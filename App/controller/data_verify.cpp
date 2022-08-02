#include "data_verify.h"
#include <safeheron/crypto-encode/hex.h>
#include <safeheron/crypto-curve/curve.h>

using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;

bool VerifyData(const http_request& message, vector<string>& pubkey_list,
                int& k, int& l, int& key_length) {
    bool ok = true;
    auto body_json = message.extract_json().get();

    /** Check if the key exists */
    ok = KeyExist(body_json);
    if (!ok) return false;

    /** Check if the inputs are valid */
    ok = InputValid(body_json);
    if (!ok) return false;

    /** Assign value */
    k = body_json.at(NUMERATOR_K).as_integer();
    l = body_json.at(DENOMINATOR_L).as_integer();
    key_length = body_json.at(KEY_LENGTH).as_integer();

    auto pub_key_array = body_json.at(USER_PUBLICKEY_LIST).as_array();

    /** Put public keys into the vector */
    for (const auto& value : pub_key_array) {
        ok = VerifyPublicKey(value.as_string());
        if (!ok) return false;

        pubkey_list.emplace_back(value.as_string());
    }

    /**
     *  The threshold denominator must be greater than 1
     *  The threshold denominator must be smaller than 21
     *  The threshold numerator must be greater than 0
     *  The threshold numerator must be greater than half of threshold denominator
     *  The threshold numerator must be smaller than or equal to the threshold denominator
     */
    if (l <= 1 || k <= 0 || k < (l/2+1) || k > l || l >= 21) return false;

    /** The number of public keys should equal to the number of the threshold denominator */
    if (pubkey_list.size() != l) return false;

    /** The key length should be 1024, 2048, 3072 or 4096 */
    if (key_length != 1024 && key_length != 2048 && key_length != 3072 && key_length != 4096) return false;

    return true;
}

bool KeyExist(json::value& body_json) {
    bool ok = true;

    ok = body_json.has_field(USER_PUBLICKEY_LIST);
    if (!ok) return false;

    ok = body_json.has_field(NUMERATOR_K);
    if (!ok) return false;

    ok = body_json.has_field(DENOMINATOR_L);
    if (!ok) return false;

    ok = body_json.has_field(KEY_LENGTH);
    if (!ok) return false;

    ok = body_json.has_field(CALL_BACK_ADDRESS);
    if (!ok) return false;

    return true;
}

bool InputValid(json::value& body_json) {
    bool ok = true;

    ok = body_json.at(USER_PUBLICKEY_LIST).is_array();
    if (!ok) return false;

    ok = body_json.at(NUMERATOR_K).is_integer();
    if (!ok) return false;

    ok = body_json.at(DENOMINATOR_L).is_integer();
    if (!ok) return false;

    ok = body_json.at(KEY_LENGTH).is_integer();
    if (!ok) return false;

    ok = body_json.at(CALL_BACK_ADDRESS).is_string();
    if (!ok) return false;

    return true;
}

bool VerifyPublicKey(const string& public_key) {
    bool ok;
    CurvePoint pub;
    string pub_key_65;

    if(public_key.size() % 2 != 0) return false;

    pub_key_65 = safeheron::encode::hex::DecodeFromHex(public_key);
    if (pub_key_65.empty()) return false;

    ok = pub.DecodeFull((uint8_t *)pub_key_65.c_str(), CurveType::P256);
    if (!ok) return false;

    return true;
}