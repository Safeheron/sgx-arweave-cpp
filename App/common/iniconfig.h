/*************************************************
 * File name : iniconfig.h
 * Introduce : The header file for class iniconfig, which reads ini string
 *             and parses it.
 * 
 * Create: 2021-07-06 by yyf
 * 
 *************************************************/
#ifndef _INI_CONFIG_H_
#define _INI_CONFIG_H_

#include <string>
#include <list>
#include <map>

/**
 * Error codes
*/
#define ERR_INI_OK                  0
#define ERR_INI_INVALID_PARAM       1
#define ERR_INI_FILE_NOT_EXIST      2
#define ERR_INI_OPEN_FILE_FAILED    3
#define ERR_INI_FILE_IS_EMPTY       4
#define ERR_INI_READ_FILE_FAILED    5
#define ERR_INI_PARSE_INI_FAILED    6

class IniConfig
{
public:
    IniConfig();
    virtual ~IniConfig();
private:
    /**
     *  key-value pair for INI file config node
    */
    struct ini_node
    {
        ini_node( std::string key, std::string val ) {
            m_key = key;
            m_val = val;
        }
        std::string m_key;
        std::string m_val;
    };
    typedef std::list<ini_node> list_key_val;
    typedef std::map<std::string, list_key_val> map_ini_config;
    
public:
    /**
     * To load an INI file and parse it
    */
    int load_file( const char* ini_file );
    /**
     * To parse INI content string, content is INI file content data
    */
    int parse_content( const uint8_t* content, size_t size );
    /**
     * Return an int value based on section name and key name 
    */
    int get_int( const char * section, const char * key );
    /**
     * Return an longlong value based on section name and key name 
    */
    int64_t get_llong( const char * section, const char * key );
    /**
     * Return an string value based on section name and key name 
    */
    std::string get_string( const char * section, const char * key );
    /**
     * Return an string value list based on section name and key name 
    */
    void get_value_list( const char* section, const char * key, std::list<std::string> & val_list );
    
private:
    static bool getline ( std::string & in, std::string & line, char delim = '\n');
    static std::string & trim_string( std::string &str );

private:
    map_ini_config  m_config;
};

#endif//_INI_CONFIG_H_