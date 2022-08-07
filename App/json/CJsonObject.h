#ifndef CJSONOBJECT_HPP_
#define CJSONOBJECT_HPP_

#include "cJSON.h"
#include <string>
#include <vector>

class CJsonObject {
public:
    CJsonObject();
    virtual ~CJsonObject();

    /** add data to object */
    bool Add(const std::string& key_str, const std::string& data_str);
    bool Add(const std::string& key_str, int data_number);
    bool Add(const std::string& key_str, bool data_bool);
    bool Add(const std::string& key_str, CJsonObject& cjson_object);

    /** add data to array */
    bool Add(const std::string& data_str);
    bool Add(CJsonObject& cjson_object);

    /** parse cjson string */
    bool Parse(const std::string& data_str);

    /** change json to json string */
    bool ToString(std::string& cjson_to_str);

    bool CreateObject();
    bool CreateArray();

    /** get array to vector */
    bool GetArray(const std::string& key_str, std::vector<std::string>& pubkey_list);
    bool GetValueString(const std::string& key_str, std::string& value_str);

    /** point cjson_data to nullptr */
    void PointToNull();

    /** return the pointer of cjson_data */
    cJSON* GainCJsonData();

private:
    cJSON_bool cjson_ok_;
    cJSON* cjson_data_ = nullptr;
    cJSON* cjson_check_return_ = nullptr;
};


#endif