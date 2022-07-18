/*************************************************
 * File Name: hmac.h
 * Introduce: The header file for HMAC functions, which calculate HMAC
 *            value for inputing message and key.
 * Create: 2021-9-12 by yyf
 * 
 *************************************************/
#ifndef _HMAC_H_
#define _HMAC_H_

#include <string>

/*
 *  The public functions for calculating HMAC
 */
class HMAC
{
public:
    static int hmac_sha1( const std::string & key, const std::string & msg, std::string & hmac );
    static int hmac_sha224( const std::string & key, const std::string & msg, std::string & hmac );
    static int hmac_sha256( const std::string & key, const std::string & msg, std::string & hmac );
    static int hmac_sha384( const std::string & key, const std::string & msg, std::string & hmac );
    static int hmac_sha512( const std::string & key, const std::string & msg, std::string & hmac );
};

#endif //_HMAC_H_
