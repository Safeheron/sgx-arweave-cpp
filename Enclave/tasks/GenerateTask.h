/**
 * @file GenerateTask.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../shell/Dispatcher.h"
#include <crypto-tss-rsa/tss_rsa.h>
#include <crypto-tss-rsa/RSAPrivateKeyShare.h>
#include <crypto-tss-rsa/RSAPublicKey.h>
#include <crypto-tss-rsa/RSASigShare.h>
#include <crypto-tss-rsa/RSAKeyMeta.h>
#include <crypto-tss-rsa/KeyGenParam.h>
#include <crypto-encode/hex.h>
 #include <string>
 #include <vector>

using safeheron::tss_rsa::RSAPrivateKeyShare;
using safeheron::tss_rsa::RSAPublicKey;
using safeheron::tss_rsa::RSAKeyMeta;
using safeheron::tss_rsa::RSASigShare;
using safeheron::tss_rsa::KeyGenParam;

 typedef std::vector<std::string> PUBKEY_LIST;
 typedef std::vector<RSAPrivateKeyShare> PRIKEYSHARE_LIST;

class GenerateTask: public Task
{
public:
    virtual int execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg );
    virtual int get_task_type( );

private:
    int get_pubkey_hash( const PUBKEY_LIST & pubkey_list, std::string & hash_hex );
    int get_keymeta_hash( const RSAKeyMeta & key_meta, std::string & hash_hex );
    int get_reply_string( const std::string & input_pubkey_hash, const PUBKEY_LIST & input_pubkey_list, const RSAPublicKey & pubkey, const PRIKEYSHARE_LIST & prikey_list, const RSAKeyMeta & key_meta,  std::string & out_str );
    int get_privkey_info_cipher( int index, const std::string & input_pubkey, const RSAPrivateKeyShare & prikey, const RSAKeyMeta & key_meta,  std::string & out_str );
};