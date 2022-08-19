/*************************************************
 * File name : iniconfig.cpp
 * Introduce : The implement file for class iniconfig, 
 *             which reads ini string and parses it.
 * 
 * Create: 2021-6-9 by yyf
 * 
 *************************************************/
#include "iniconfig.h"
#include "log_u.h"
#include <unistd.h>

/******************************************
 * Name:IniConfig
 * Introduce:construction function 
 * Params:N/A
 * Return:N/A
 ******************************************/
IniConfig::IniConfig()
{

}
/******************************************
 * Name:~IniConfig
 * Introduce:distruction function 
 * Params:N/A
 * Return:N/A
 ******************************************/
IniConfig::~IniConfig()
{

}
/**
 *  
*//******************************************
 * Name:load_file
 * Introduce: to load an INI file and parse it
 * Params:ini_file[IN]: an ini file file
 * Return:0 if successful, otherwise an error code
 ******************************************/
int IniConfig::load_file( const char* ini_file )
{
    int ret = 0;
    size_t f_size = 0;
    size_t r_size = 0;
    uint8_t* f_data = 0;
    FILE* fp = nullptr;

    if ( !ini_file || strlen(ini_file) <= 0 ) {
        ERROR( "ini_file is null!" );
        return ERR_INI_INVALID_PARAM;
    }

    // check if file exist
    if ( access( ini_file, F_OK ) == -1 ){
        ERROR( "file not exist - %s!", ini_file );
        return ERR_INI_FILE_NOT_EXIST;
    }

    // open file
    if ( (fp = fopen( ini_file, "rb" ) ) == nullptr ) {
        ERROR( "failed to load file: - %s!", ini_file );
        return ERR_INI_OPEN_FILE_FAILED;
    }

    // get file size
    fseek( fp, 0, SEEK_END );
    if ( ( f_size = ftell( fp ) ) <= 0 ) {
        ERROR( "failed to get file size: - %d!", (int)f_size );
        ret = ERR_INI_FILE_IS_EMPTY;
        goto exit;
    }
    fseek( fp, 0, SEEK_SET );

    // read file content
    f_data = (uint8_t*)malloc( f_size );
    memset( f_data, 0, f_size );
    r_size = fread( f_data, 1, f_size, fp );
    if ( r_size != f_size ) {
        ERROR( "failed to read file, read size: - %d!\n", (int)r_size );
        ret = ERR_INI_READ_FILE_FAILED;
        goto exit;
    }

    // parse file content
    if ( ( ret = parse_content( f_data, f_size ) ) != 0 ) {
        ERROR( "failed to read file, read size: - %d!\n", (int)r_size );
        ret = ERR_INI_PARSE_INI_FAILED;
        goto exit;
    }

    ret = ERR_INI_OK;

exit:
    if ( f_data) {
        free ( f_data );
        f_data = nullptr;
    }
    if ( fp ) {
        fclose ( fp );
        fp = nullptr;
    }

    return ret;
}
/******************************************
 * Name:parse_content
 * Introduce:to parse an ini file content 
 * Params:content[IN]: an ini file content
 *        size[IN]: content size, in bytes
 * Return:0 if successful, otherwise an error code
 ******************************************/
int IniConfig::parse_content( const uint8_t* content, size_t size )
{
    std::string ini_cfg;
    std::string str_line;
    std::string str_root;

    // checking
    if ( !content || size <= 0 ) {
        ERROR( "content is null!" );
        return ERR_INI_INVALID_PARAM;
    }

    ini_cfg.append( (char*)content, size );

    // get a line string
    while ( getline( ini_cfg, str_line ) ) {
        std::string str_key = "";
        std::string str_value = "";
        std::string::size_type left_pos = 0;
        std::string::size_type right_pos = 0;
        std::string::size_type equal_div_pos = 0;

        // a section
        if ( (str_line.npos != (left_pos = str_line.find("[") ) ) &&
             (str_line.npos != (right_pos = str_line.find("]") ) ) )
        {
            str_root = str_line.substr(left_pos+1, right_pos-1);
        }

        // key and value pair
        if ( str_line.npos != ( equal_div_pos = str_line.find("=") ) )
        {
           str_key = str_line.substr(0, equal_div_pos);
           str_value = str_line.substr(equal_div_pos+1, str_line.size()-1);
           str_key = trim_string(str_key);
           str_value = trim_string(str_value);
        }

        // get a valid config, save it
        if ( (!str_root.empty()) && (!str_key.empty()) && (!str_value.empty() ) )
        {
           ini_node node(str_key, str_value);
           map_ini_config::iterator it = m_config.find( str_root );

           // section name is exist, add the new node to it
           if ( it != m_config.end() && !it->second.empty() )
           {
               list_key_val* lst_key = &it->second;
               lst_key->push_back(node);
           }
           // otherwise create a new <key,value> list for the new node
           else
           {
               list_key_val lst_key;
               lst_key.push_back(node);
               m_config.insert(std::pair<std::string, list_key_val>(str_root, lst_key));
           }
        }
    };
    
    return ERR_INI_OK;
}
/******************************************
 * Name:get_int
 * Introduce:get a int value 
 * Params:section[IN]: the section name
 *        key[IN]: the key name
 * Return:the value if successful, otherwise return 0
 ******************************************/
int IniConfig::get_int( const char * section, const char * key )
{
    std::string val = get_string( section, key );
    if ( val.length() > 0)
    {
        return atoi( val.c_str() );
    }

    return 0;
}
/******************************************
 * Name:get_llong
 * Introduce:get a long long value 
 * Params:section[IN]: the section name
 *        key[IN]: the key name
 * Return:the value if successful, otherwise return 0
 ******************************************/
int64_t IniConfig::get_llong( const char * section, const char * key )
{
    std::string val = get_string( section, key );
    if ( val.length() > 0)
    {
        return atoll( val.c_str() );
    }

    return 0;
}
/******************************************
 * Name:get_string
 * Introduce:get a string value 
 * Params:section[IN]: the section name
 *        key[IN]: the key name
 * Return:the value if successful, otherwise return ""
 ******************************************/
std::string IniConfig::get_string( const char * section, const char * key )
{
    map_ini_config::iterator it;

    // checking
    if (!section || strlen(section) <= 0) {
        return "";
    }
    if (!key || strlen(key) <= 0) {
        return "";
    }
    if (m_config.size() <= 0) {
        return "";
    }

    // find
    it = m_config.find( section );
    if (it != m_config.end() && !it->second.empty()) {
        list_key_val lst_key = it->second;
        for (list_key_val::iterator it_key = lst_key.begin();
             it_key != lst_key.end();
             it_key++) {
            if (strcmp(key, it_key->m_key.c_str()) == 0) {
                return it_key->m_val;
            }
        }
    }

    return "";
}
/******************************************
 * Name:get_value_list
 * Introduce:get string(s) value in list
 * Params:section[IN]: the section name
 *        key[IN]: the key name
 *        val_list[OUT]: the result list
 * Return: N/A
 ******************************************/
void IniConfig::get_value_list( const char* section, 
                                const char * key, 
                                std::list<std::string> & val_list )
{
    map_ini_config::iterator it;

    val_list.clear();

    // checking
    if (!section || strlen(section) <= 0) {
        return;
    }
    if (!key || strlen(key) <= 0) {
        return;
    }
    if (m_config.size() <= 0) {
        return;
    }

    // find
    it = m_config.find( section );
    if (it != m_config.end() && !it->second.empty()) {
        list_key_val lst_key = it->second;
        for (list_key_val::iterator it_key = lst_key.begin();
             it_key != lst_key.end();
             it_key++) {
            if (strcmp(key, it_key->m_key.c_str()) == 0) {
                val_list.push_back( it_key->m_val );
            }
        }
    }    
}

// get a line for string
bool IniConfig::getline ( std::string & in, std::string & line, char delim )
{
    int ret = 0;
    int pos = 0;

    if ( in.length() <= 0 ) {
        return false;
    }

    pos = in.find( delim );
    if ( pos >= 0 ) {
        line = in.substr ( 0, pos );
        in = in.substr ( pos + 1, in.length() - (pos+1));
    } else {
        line = in;
        in = "";
    }

    return true;
}
// trim string with " " and "\r" chars
std::string & IniConfig::trim_string( std::string &str )
{
    std::string::size_type pos = 0;
    while (str.npos != (pos = str.find(" ")) ||
           str.npos != (pos = str.find("\r")) ||
           str.npos != (pos = str.find("\"")))
        str = str.replace(pos, pos+1, "");
    return str;
}