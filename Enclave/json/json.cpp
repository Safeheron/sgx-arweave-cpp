#include "json.h"
#include "cJSON.h"

namespace JSON
{
    Root::Root()
    : m_read_root( nullptr )
    , m_write_root( nullptr )
    {
    }
    Root::Root( const Root & other )
    : m_read_root( cJSON_Duplicate( (cJSON*)other.m_read_root, true) )
    , m_write_root( cJSON_Duplicate( (cJSON*)other.m_write_root, true) )
    {
    }
    Root::~Root()
    {
        clear();
    }
    Root Root::parse( const std::string & json )
    {
        Root root;

        if ( json.length() > 0 ) {
            root.m_read_root = cJSON_Parse( json.c_str() );
        }
        else {
            root.m_read_root = nullptr;
        }

        return root;    
    }
    std::string Root::write( const Root & root )
    {
        char* json = nullptr;
        std::string ret;
        cJSON* write_root = ( cJSON* )root.m_write_root;

        if ( write_root ) {
            json = cJSON_PrintUnformatted( write_root );
            if ( json ) {
                ret = json;
                cJSON_free( json );
            }
        }

        return ret;
    }
/*    Root Root::arrary( ) 
    {
        Root root;
        root.m_read_root = cJSON_CreateArray();
        root.m_write_root = cJSON_CreateArray();

        return root;    
    }
*/
    Root Root::operator=( const Root & t )
    {
        if ( this != &t ) {
            m_read_root = cJSON_Duplicate( (cJSON*)t.m_read_root, true );
            m_write_root = cJSON_Duplicate( (cJSON*)t.m_write_root, true );
        }
        return *this;
    }

    bool Root::is_valid()
    {
        return m_read_root ? true : false;
    }

    void Root::clear()
    {
        if ( m_read_root ) {
            cJSON_Delete( (cJSON*)m_read_root );
            m_read_root = nullptr;
        }
        if ( m_write_root ) {
            cJSON_Delete( (cJSON*)m_write_root );
            m_write_root = nullptr;
        }
    }

    Value Root::operator[]( const char* key )
    {
        Value json_val;
        cJSON* read_root = (cJSON*)m_read_root;
        cJSON* write_root = (cJSON*)m_write_root;

        // for read json
        if ( read_root ) {
            json_val.m_node = cJSON_GetObjectItem( read_root, key );
        }
        // for write json
        else {
            if ( !write_root ) {
                m_write_root = cJSON_CreateObject();
            }
            json_val.m_key = key;
            json_val.m_root = m_write_root;
        }

        return json_val;
    }
/*
    // add an arrary node in root
    Value Root::addArrary( const char* key )
    {
        Value json_val;
        cJSON* arrary_root = nullptr;
        cJSON* write_root = (cJSON*)m_write_root;

        if ( !write_root ) {
            m_write_root = cJSON_CreateObject();
        }
        arrary_root = cJSON_CreateArray();
        cJSON_AddItemToObject( write_root, key, cJSON_Duplicate(arrary_root, true) );

        json_val.m_key = key;
        json_val.m_root = arrary_root;

        return json_val;
    }
*/
    Value::Value()
    : m_root( nullptr )
    , m_node( nullptr )
    {
    }
    Value::~Value()
    {
        m_root = nullptr;
        m_node = nullptr;
    }

    Value::Value(const Value &other)
    : m_root( other.m_root )
    , m_node( other.m_node )
    , m_key( other.m_key )
    {
    }
    void Value::operator=(int32_t value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddNumberToObject( root, m_key.c_str(), value );
        }
    }
    void Value::operator=(uint32_t value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddNumberToObject( root, m_key.c_str(), value );
        }
    }
    void Value::operator=(const char* value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddStringToObject( root, m_key.c_str(), value );
        }
    }
    void Value::operator=(const std::string & value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddStringToObject( root, m_key.c_str(), value.c_str() );
        }
    }
    void Value::operator=(bool value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddBoolToObject( root, m_key.c_str(), value );
        }
    }
    void Value::operator=(const Root & value)
    {
        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddItemToObject( root, m_key.c_str(), cJSON_Duplicate((cJSON*)value.m_read_root, true) );
            cJSON_AddItemToObject( root, m_key.c_str(), cJSON_Duplicate((cJSON*)value.m_write_root, true) );
        }
    }
    void Value::operator=(const std::list<Root> & value)
    {
        cJSON* arrary = cJSON_CreateArray();
        for ( auto & it : value ) {
            cJSON_AddItemToArray( arrary, cJSON_Duplicate(((cJSON*)it.m_write_root), true) );
        }

        cJSON* root = ( cJSON* )m_root;
        if ( root ) {
            cJSON_AddItemToObject( root, m_key.c_str(), arrary );
        }
    }

    int32_t Value::asInt() const
    {
        int32_t ret = 0;
        cJSON* node = (cJSON*)m_node;

        if ( node && node->type == cJSON_Number ) {
            ret = node->valueint;
        }

        return ret;
    }   
    uint32_t Value::asUInt() const
    {
        int32_t ret = 0;
        cJSON* node = (cJSON*)m_node;

        if ( node && node->type == cJSON_Number ) {
            ret = node->valueint;
        }
        
        return ret;
    }
    std::string Value::asString() const
    {
        std::string ret;
        cJSON* node = (cJSON*)m_node;

        if ( node && node->type == cJSON_String ) {
            ret = std::string( node->valuestring );
        }
        
        return ret;
    }   
    bool Value::asBool() const
    {
        bool ret = false;
        cJSON* node = (cJSON*)m_node;

        if ( node ) {
            if ( node->type == cJSON_True ) {
                ret = true;
            }
            else if ( node->type == cJSON_False ) {
                ret = false;
            }
            else {
                ret = false;
            }
        }
        return ret;       
    }   
    
    Root Value::asJson() const
    {
        Root root;
        cJSON* node = (cJSON*)m_node;

        if ( node && node->type == cJSON_Object ) {
            root.m_read_root = cJSON_Duplicate( node, true );
        }
        return root;     
    }

    STR_ARRAY Value::asStringArrary() const
    {
        STR_ARRAY result;
        cJSON* node = (cJSON*)m_node;

        if ( node && node->type == cJSON_Array ) {
            int size = cJSON_GetArraySize( node );
            for ( int i = 0; i < size; ++i ) {
                result.push_back( cJSON_GetArrayItem( node, i )->valuestring );
            }
        }
        return result;
    }
};