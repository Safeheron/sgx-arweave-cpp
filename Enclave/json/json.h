/*************************************************
 * File name : json.h
 * Introduce : The header file for json, which defines
 *             read/write functions for json string. You
 *             can use this library like:
 * 
 *             #include "json.h"
 * 
 *             // Parse a JSON string
 *             const char* json = "{\"string\":\"This is a string\",\"int\":100,\"bool\":true}";
 *             JSON::Root root = JSON::Root::parse( json.c_str() );
 *             printf( "Json String: %s \n", root["string"].asString() );
 *             printf( "Json Int: %d \n", root["int"].asInt() );
 *             printf( "Json Bool: %d \n", root["bool"].asBool() );
 * 
 *             // Create a JSON string like:
 *             JSON::Root wroot;
 *             wroot["STRING"] = "This is an other string";
 *             wroot["BOOL"] = false;
 *             wroot["INT"] = 200;
 *             std::string new_json = JSON::Root::write( wroot );
 *             printf( "New Json: %s\n", new_json.c_str() );
 * 
 *************************************************/
#ifndef _JSON_H_
#define _JSON_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <list>

typedef std::vector<std::string> STR_ARRARY;

namespace JSON
{
    class Root;
    
    class Value
    {
    public:
        Value();
        Value(const Value &other);
    public:
        virtual ~Value();
    public:
        virtual void operator=(int32_t value);
        virtual void operator=(uint32_t value);
        virtual void operator=(const char* value);
        virtual void operator=(const std::string & value);
        virtual void operator=(bool value);
        virtual void operator=(const Root & value);
        virtual void operator=(const std::list<Root> & value); //Add to a JSON object arrary node
        virtual int32_t asInt() const;    
        virtual uint32_t asUInt() const;
        virtual std::string asString() const;
        virtual bool asBool() const;
        virtual Root asJson() const;
        virtual STR_ARRARY asStringArrary() const;
    public:/* internal values, don't use them directly! */
        void*   m_root;
        void*   m_node;
        std::string m_key;
    };

    class Root
    {
    public:
        Root();
        Root( const Root & other );
        virtual ~Root();
        //
        static Root parse( const std::string & json );
        static std::string write( const Root & root );
        virtual Root operator=( const Root & t );
        virtual bool is_valid();
        virtual void clear();
        virtual Value operator[]( const char* key );
        //
    public: /* internal values, don't use them directly! */
        void*   m_read_root;
        void*   m_write_root;
    };
};

#endif //_JSON_H_