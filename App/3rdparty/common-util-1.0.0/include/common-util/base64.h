/*************************************************
 * File name : base64.h
 * Introduce : The header file for base64 encode and decode functions.
 * 
 * Create: 2021-07-28 by yyf
 * 
 *************************************************/
#ifndef _BASE64_H_
#define _BASE64_H_

#include <string>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Encode bytes to base64 string
*/
void base64_encode( const uint8_t* input, size_t size, std::string & b64 );
/**
 * Decode base64 string to bytes, and return in output.
*/
void base64_decode( const std::string b64, std::string & output );

#ifdef __cplusplus
}
#endif

#endif//_BASE64_H_