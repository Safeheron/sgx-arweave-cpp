#include "CJsonObject.h"
#include <log_u.h>

extern thread_local std::string request_id;

CJsonObject::CJsonObject() {

}

CJsonObject::~CJsonObject() {
    if (cjson_data_ != nullptr) {
        cJSON_Delete(cjson_data_);
    }
}

bool CJsonObject::Add(const std::string& key_str, const std::string& data_str) {
    cjson_check_return_ = cJSON_AddStringToObject(cjson_data_, key_str.c_str(), data_str.c_str());
    if (cjson_check_return_ == nullptr) {
        ERROR("Request ID: %s, Add %s failed.", request_id.c_str(), key_str.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::Add(const std::string& key_str, int data_number) {
    cjson_check_return_ = cJSON_AddNumberToObject(cjson_data_, key_str.c_str(), data_number);
    if (cjson_check_return_ == nullptr) {
        ERROR("Request ID: %s, Add %s failed.", request_id.c_str(), key_str.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::Add(const std::string& key_str, bool data_bool) {
    cjson_check_return_ = cJSON_AddBoolToObject(cjson_data_, key_str.c_str(), data_bool);
    if (cjson_check_return_ == nullptr) {
        ERROR("Request ID: %s, Add %s failed.", request_id.c_str(), key_str.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::Add(const std::string& key_str, CJsonObject& cjson_object) {
    if (cJSON_IsArray(cjson_object.GainCJsonData()) || cJSON_IsObject(cjson_object.GainCJsonData())) {
        cjson_ok_ = cJSON_AddItemToObject(this->cjson_data_, key_str.c_str(), cjson_object.GainCJsonData());
        if (!cjson_ok_) {
            ERROR("Request ID: %s, Add %s failed.", request_id.c_str(), key_str.c_str());
        }
    } else {
        ERROR("Request ID: %s, Key: %s invalid input.", request_id.c_str(), key_str.c_str());
        return false;
    }
    cjson_object.PointToNull();
    return true;
}

bool CJsonObject::Add(const std::string& data_str) {
    cjson_ok_ = cJSON_AddItemToArray(cjson_data_, cJSON_CreateString(data_str.c_str()));
    if (!cjson_ok_) {
        ERROR("Request ID: %s, Add string to array failed.", request_id.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::Add(CJsonObject& cjson_object) {
    cjson_ok_ = cJSON_AddItemToArray(cjson_data_, cjson_object.GainCJsonData());
    if (!cjson_ok_) {
        ERROR("Request ID: %s, Add object to array failed.", request_id.c_str());
        return false;
    }
    cjson_object.PointToNull();
    return true;
}

bool CJsonObject::Parse(const std::string& data_str) {
    cjson_data_ = cJSON_Parse(data_str.c_str());
    if (cjson_data_ == nullptr) {
        ERROR("Request ID: %s, Parse cjson data failed.", request_id.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::ToString(std::string& cjson_to_str) {
    char* temp = cJSON_Print(cjson_data_);
    cjson_to_str.assign(temp, strlen(temp));
    if (cjson_to_str.empty()) {
        ERROR("Request ID: %s, Change cjson to string failed.", request_id.c_str());
        return false;
    }
    free(temp);
    return true;
}

bool CJsonObject::CreateObject() {
    cjson_data_ = cJSON_CreateObject();
    if (cjson_data_ == nullptr) {
        ERROR("Request ID: %s, Create object failed.", request_id.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::CreateArray() {
    cjson_data_ = cJSON_CreateArray();
    if (cjson_data_ == nullptr) {
        ERROR("Request ID: %s, Create array failed.", request_id.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::GetValueString(const std::string& key_str, std::string& value_str) {
    value_str = cJSON_GetObjectItem(cjson_data_, key_str.c_str())->valuestring;
    if (value_str.empty()) {
        ERROR("Request ID: %s, Get %s failed.", request_id.c_str(), key_str.c_str());
        return false;
    }
    return true;
}

bool CJsonObject::GetArray(const std::string& key_str, std::vector<std::string>& pubkey_list) {
    cJSON* cjson_array = cJSON_GetObjectItem(cjson_data_, key_str.c_str());
    if (!cjson_array) {
        ERROR("Request ID: %s, Get %s failed", request_id.c_str(), key_str.c_str());
        return false;
    }
    int array_size = cJSON_GetArraySize(cjson_array);
    if (0 == array_size) {
        ERROR("Request ID: %s, pubkey_list_size error", request_id.c_str());
        return false;
    }

    for (int i = 0; i < array_size; ++i) {
        pubkey_list.emplace_back(cJSON_GetArrayItem(cjson_array, i)->valuestring);
    }

    return true;
}

void CJsonObject::PointToNull() {
    cjson_data_ = nullptr;
}

cJSON* CJsonObject::GainCJsonData() {
    return cjson_data_;
}