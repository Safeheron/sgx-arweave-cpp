/*************************************************
 * File name : json.h
 * Introduce : The header file for json, which defines
 *             read/write funtions for json string. You
 *             can use this library like:
 * 
 *             #include "json.h"
 * 
 *             // Parse a JSON string
 *             const char* json = "{\"string\":\"This is a string\",\"int\":100,\"bool\":true}";
 *             JSON::JsonRoot root = JSON::JsonRoot::parse( json.c_str() );
 *             printf( "Json String: %s \n", root["string"].asString() );
 *             printf( "Json Int: %d \n", root["int"].asInt() );
 *             printf( "Json Bool: %d \n", root["bool"].asBool() );
 * 
 *             // Create a JSON string like:
 *             JSON::JsonRoot wroot;
 *             wroot["STRING"] = "This is an other string";
 *             wroot["BOOL"] = false;
 *             wroot["INT"] = 200;
 *             std::string new_json = JSON::JsonRoot::write( wroot );
 *             printf( "New Json: %s\n", new_json.c_str() );
 * 
 * Create: 2021-07-13 by yyf
 * 
 *************************************************/
#ifndef _JSON_H_
#define _JSON_H_

#include <stdio.h>
#include <string>
#include <list>

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
        virtual void operator=(Root & valut);
        virtual int32_t asInt() const;    
        virtual uint32_t asUInt() const;
        virtual std::string asString() const;
        virtual bool asBool() const;
        virtual Root asJson() const;
    public:/* internal values, don't use them directly! */
        void*   m_root;
        void*   m_node;
        std::string m_key;
    };

    class Root
    {
    public:
        Root();
        Root( Root & other );
        virtual ~Root();
        //
        static Root parse( std::string & json );
        static std::string write( Root & root );
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