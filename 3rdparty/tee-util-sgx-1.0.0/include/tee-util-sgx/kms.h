/*************************************************
 * File name : kms.h
 * Introduce : The header file for KMS/Key Vault interfaces. 
 *             They privode key vault and encryption key classes, you
 *             can use this library like:
 * 
 *             #include "kms.h"
 * 
 *             // Create a key vault
 *             KMS::KeyVault_Info kv_info = {....};
 *             KMS::KeyValut* kv = KMS::KeyVault_new( kv_info );
 * 
 *             // Add keys information to key vault object
 *             kv->add_key( name, version, alg );
 * 
 *             // Get a key by name when you need to use it to encrypt/decrypt secret data
 *             KMS::EncryptionKey* key = kv->get_key( name, version );
 *             if ( key ) {
 *                  ret = key->encrypt( input, outpunt );
 *             }
 * 
 *             ......
 * 
 *             // Free it at last
 *             KMS::KeyVault_free( kv );
 *             kv = nullptr;
 * 
 * Create: 2021-08-20 by yyf
 * 
 *************************************************/
#ifndef _KMS_H_
#define _KMS_H_

#include <string>

/**
 * Error codes
*/
#define ERR_KMS_OK                          0   /* OK */
#define ERR_KMS_INVALID_PARAM               1   /* input parameter(s) is null or invalid */
#define ERR_KMS_INVALID_CALL                2   /* this calling is invalid */
#define ERR_KMS_CREATE_HTTP_OBJ_FAILED      3   /* failed to create IHttpsRequest object */
#define ERR_KMS_HTTP_CONFIG_FAILED          4   /* do http configure failed */
#define ERR_KMS_HTTP_REQUEST_FAILED         5   /* failed to do a http request */
#define ERR_KMS_HTTP_RESPONSE_BODY_IS_WRONG 6   /* http response's body is not a valid JSON string */
#define ERR_KMS_GET_TIMESTAMP_FAILED        7   /* failed to get time stamp */
#define ERR_KMS_ENCODE_PARAMS_FAILED        8   /* failed to encode request parameters for signature */
#define ERR_KMS_SIGN_PARAMS_FAILED          9   /* failed to sign the request parameters */

/**
 * kms provider names
*/
#define KMS_PROVIDER_AZURE   "azure"
#define KMS_PROVIDER_ALIYUN  "aliyun"
#define KMS_PROVIDER_AWS     "aws"

/**
 * The max length of secret data, which will be encrypted/decrypted by key vault
*/
#define KMS_MAX_DATA_LEN        (8*1024)

namespace KMS
{   
    /**
     * A key vault information, used to create a KeyVault object
     * by calling KeyVault_new().
    */
    typedef struct _KeyVault_Info
    {
        int         version;        /* data configure version, such as 1,2,3…… */
        std::string provider;       /* key vault cloud service provider name, such as "azure","aws" */
        std::string api_version;    /* key vault cloud service client api version, such as "api-version=7.2" */
        int         token_expires;  /* login expires time, in s */
        std::string client_id;      /* the client id of key vault account ( access key id) */
        std::string secret;         /* the secret key of key vault account ( access secret key) */
        std::string tennat;         /* the tennat name of key vault account */
        std::string base_url;       /* key vault base url string */
        std::string key_alg;        /* key algorithm name, such as "RSA-OAEP-256" */
        std::string key_name;       /* the encrypt/decrypt key name in key vault */
        std::string key_version;    /* key version of the encrypt/decrypt key */
        int         key_type;       /* 0: aysmmetric key; 1: symmetric key*/
    }KeyVault_Info;

    /**
     * The interface of key valut, which has an encryption key
    */
    class IKeyVault
    {
    public:
        IKeyVault(){};
        virtual ~IKeyVault(){};
        virtual int get_version() = 0;
        virtual std::string get_provider() = 0;
        virtual std::string get_baseurl() = 0;
        virtual std::string get_keyname() = 0;
        virtual std::string get_keyversion() = 0;
        virtual std::string get_keyalg() = 0;
        virtual int encrypt( const std::string & input, std::string & output ) = 0;
        virtual int decrypt( const std::string & input, std::string & output ) = 0;
    };

    /**
     * Call this API to create a KeyVault object
    */
    IKeyVault* KeyVault_new( KeyVault_Info & info );
    /**
     * Call this API to free a KeyVault object
    */
    void KeyVault_free( IKeyVault * kv );
};

#endif //_KMS_H_