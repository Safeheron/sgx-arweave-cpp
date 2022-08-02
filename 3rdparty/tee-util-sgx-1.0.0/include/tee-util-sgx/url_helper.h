/*************************************************
 * File name : url_helper.h
 * Introduce : The header file for URL helpher functions. 
 * 
 * Create: 2021-08-20 by yyf
 * 
 *************************************************/

#ifndef _URL_HELPER_H_
#define _URL_HELPER_H_

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

std::string url_encode( const std::string & url );
std::string url_decode( const std::string & url );
std::string percent_encode( const std::string & url );

#ifdef __cplusplus
}
#endif

#endif //_URL_HELPER_H_
