/*************************************************
 * File name : HttpsClient.h
 * Introduce : Definition this library export functions and interface.
 *             This library implements HTTP/HTTPS request based on mbedtls library,
 *             and defines two export functions and an interface.
 * 
 *             If you POST/GET a HTTP/HTTPS request to host use this library, the calling
 *             code maybe like below.
 * 
 *              // create the request object
 *              IHttpsRequest* req = HttpsRequest_Create( );
 * 
 *              // do header configure
 *              HttpHeader header;
 *              cfg.is_https = false;
 *              cfg.verify_host = false; 
 *              int ret = req->https_setup( cfg );
 * 
 *              // do a GET request
 *              std::string resp;
 *              int ret = req->do_get_request( url, resp );
 *              // do something with resp
 * 
 *              // do an other POST request
 *              ret = req->do_post_request( url, body, resp );
 *              
 *              // do something with resp
 *              ......
 * 
 *              // close the connection
 *              ret = req->https_clear();
 * 
 *              // release object at last
 *              if ( req ) {
 *                  HttpsRequest_Destory( req );
 *                  req = nullptr;
 *              }
 *             
 * Create: 2021-6-10 by yyf
 * 
 *************************************************/

#ifndef _HTTPS_CLIENT_H_
#define _HTTPS_CLIENT_H_

#include <string>
#include <map>

/**
 * @brief error codes definition
 * 
 */
#define ERR_HTTPS_OK                            0   // OK
#define ERR_HTTPS_INVALIDPARAM                  1  // invalid parameters, maybe some parameters are nullptr
#define ERR_HTTPS_INVALIDCALL                   2  // invalid calling stack
#define ERR_HTTPS_URLSCHEME_WRONG               3  // scheme(https or http) in url is not same as setting.
#define ERR_HTTPS_MBEDTLS_SEED_FAILED           10 // set mbedtls drgb seed failed
#define ERR_HTTPS_MBEDTLS_CACERT_WRONG          11 // mbedtls library parse ca certitifcate chain string failed
#define ERR_HTTPS_MBEDTLS_CONFIG_FAILED         12 // mbedtls config failed
#define ERR_HTTPS_MBEDTLS_CONNECT_FAILED        13 // set mbedtls connect timeout failed
#define ERR_HTTPS_MBEDTLS_SSL_SETUP_FAILED      14 // ssl setup failed in mbedtls
#define ERR_HTTPS_MBEDTLS_SSL_HOST_FAILED       15 // set host name failed in mbedtls
#define ERR_HTTPS_MBEDTLS_HANDSHAKE_FAILED      16 // handshake failed in mbedtls
#define ERR_HTTPS_MBEDTLS_CERT_VERIFY_FAILED    17 // verify host ssl certificate failed in mbedtls
#define ERR_HTTPS_MBEDTLS_WRITE_FAILED          18 // mbedtls write data failed
#define ERR_HTTPS_MBEDTLS_READ_FAILED           19 // mbedtls read data failed
#define ERR_HTTPS_FAILED_TO_MALLOC              -20 // failed to malloc memory


/**
 * @brief HTTPS host configure
 * 
 */
typedef struct {
    bool is_https;
    bool verify_host;
    int time_out;
    std::string content_type;
    std::string connection;
    std::string ca_crt;
}HTTPS_SETTING;

/**
 * @brief HTTPS response data
 * 
 */
typedef struct {
    bool success;
    int status;
    std::string body;
}HTTPS_RESPONSE;

/**
 * @brief the interface for https request object
 * 
 */
class IHttpsRequest
{
public:
    IHttpsRequest(){};
    virtual ~IHttpsRequest(){};
    /******************************************
     * Name:https_setup
     * Introduce:Initialize a http context, this function should be called at first. 
     * Params:cfg:[IN]: HTTPS host connnection settings.
     * Return:return 0 if successful, otherwise return an error code.
     ******************************************/
    virtual int https_setup(HTTPS_SETTING & cfg) = 0;
    /******************************************
     * Name:do_get_request
     * Introduce:Send a GET request to url
     * Params:url:[IN]: full url for service address, like "https://www.baidu.com/login/#xxxxx?xxxxx"
     *        response:[OUT]: respose data for this requesting
     * Return:return 0 if successful, otherwise return an error code.
     ******************************************/
    virtual int do_get_request(std::string & url, HTTPS_RESPONSE & response) = 0;
    /******************************************
     * Name:do_post_request
     * Introduce:Send a POST request to url
     * Params:url:[IN]: full url for service address, like "https://www.baidu.com/login"
     *        body:[IN]: POST request body string
     *        response:[OUT]: respose data for this requesting
     * Return:return 0 if successful, otherwise return an error code.
     ******************************************/
    virtual int do_post_request(std::string & url, std::string & body, HTTPS_RESPONSE & response) = 0;
   /******************************************
     * Name:https_clear
     * Introduce:Release current http context object
     * Return:return 0 if successful, otherwise return an error code.
     ******************************************/ 
    virtual int https_clear() = 0;
};


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief To create a https request object
 * 
 * @return IHttpsRequest* 
 */
IHttpsRequest* HttpsRequest_Create( );
/**
 * @brief To release a https request object 
 *        which reated by HttpsRequest_Create()
 * 
 * @param req : the object created by HttpsRequest_Create( )
 */
void HttpsRequest_Destory( IHttpsRequest* req );

#ifdef __cplusplus
}
#endif

#endif  //_HTTPS_CLIENT_H_
