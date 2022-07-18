/*************************************************
 * File name : AsymmKey.h
 * Introduce : The header file for class AsymmKey, which implement
 *             RSA/ECC construction and distruction. 
 * 
 * Created by yyf on 21-08-04.
 * 
 *************************************************/

#ifndef _ASYMM_KEY_H_
#define _ASYMM_KEY_H_

#include <string>
#include <list>

// only support 256bits ecc
#define ECC_KEY_LEN    32

class AsymmKey
{
protected:
    /**
     * Don't call construction directly!
    */
    AsymmKey( );
public:
    virtual ~AsymmKey();

public:
    /**
     * Asymmetic key algorithms definition
    */
    typedef enum eASYMMKEY_ALG
    {
        RSA_KEY    = 1,
        ECC_KEY    = 2
    }ASYMMKEY_ALG;
    /**
     * Ecurve groups definition
    */
    typedef enum eECP_GROUP_ID
    {
        ECP_GROUPT_SECP256K1   = 1,
        ECP_GROUPT_SECP256R1   = 2,
        ECP_GROUPT_ED25519     = 32
    }ECP_GROUP_ID;
public:
    /**
     * Generate an RSA key object by random data, 
     * you need to call AsymmKey::key_free() for release.
     * 
    */
    static AsymmKey *gen_rsa_key( size_t key_bits = 4096 );
    /**
     * Generate an ECC key object by random data, 
     * you need to call AsymmKey::key_free() for release.
    */
    static AsymmKey *gen_ecp_key( eECP_GROUP_ID ecc_group_id = ECP_GROUPT_SECP256R1 );
    /**
     * construct a new ECC key from public key and private key bytes
     * group_id: see AsymmKey::eECP_GROUP_ID
     * pubkey_x: x coordinate bytes of public key, in big endian
     * pubkey_y: y coordinate bytes of public key, in big endian
     * privkey: bytes of private key, in big endian
    */
    static AsymmKey* gen_ecp_key( eECP_GROUP_ID group_id, const std::string & pubkey_x, const std::string & pubkey_y, const std::string & privkey );
    /**
     * construct a new ECC key from multi private key parts
    */
    static AsymmKey* merge_ecp_key( eECP_GROUP_ID group_id, std::list<std::string> & parts );
    /**
     * derive a new ECC key from a seed buff data
    */
    static AsymmKey* derive_ecp_key( eECP_GROUP_ID group_id, const std::string & seed );
    /**
     * import RSA/ECC key pair from a protected file, which is created by export_key_to_pem_file(), 
     * you need to call AsymmKey::key_free() for release.
    */
    static AsymmKey *import_key_from_pem_file( const std::string &path );
    /**
     * import RSA/ECC key pair from plain PEM srting, support PKCS#1 and PKCS#8, 
     * you need to call AsymmKey::key_free() for release.
    */
    static AsymmKey *import_key_pem( const std::string &pem_str );
    /**
     * import RSA/ECC public key from plain PEM srting, support PKCS#1 and PKCS#8, 
     * you need to call AsymmKey::key_free() for release.
    */
    static AsymmKey *import_pubkey_pem( const std::string &pem_str );
    /**
     * Release an asymmetic key object
    */
    static void key_free( AsymmKey* asymm_key );
    /**
     * Seal the keypair data, and then export to a protected file
    */
    virtual int export_key_to_pem_file( const std::string &path ) = 0;
    /**
     * export the plain keypair to a PEM string, in PKCS#1
    */
    virtual std::string export_key_pem() = 0;
    /**
     * export the plain public key to a PEM string, in PKCS#8
    */
    virtual std::string export_pubkey_pem() = 0;
    /**
     * export the private key to a PKCS#1 or SEC1 DER structure
    */
    virtual int export_key_der( std::string & der ) = 0;
    /**
     * export the public key to a SubjectPublicKeyInfo DER structure
    */
    virtual int export_pubkey_der( std::string &der ) = 0;
    /**
     * export the plain private key to a bytes buff, in big endian
     * for RSA, export d only
     * for ECC, export d
    */
    virtual int export_prikey_data( std::string & prikey ) = 0;
    /**
     * export the plain public key to a bytes buff, in big endian
     * for RSA, export N
     * for ECC, export Q
    */
    virtual int export_pubkey_data( std::string & pubkey ) = 0;
    /**
     * use the private key to encrypt data
    */
    virtual int encrypt( const std::string & input, std::string & ouput ) = 0;
    /**
     * use the public key to encrypt data
    */
    virtual int decrypt( const std::string & input, std::string & ouput ) = 0;
    /**
     * use the private key to sign data
    */
    virtual int sign( const std::string & input, const int hash_alg, std::string & sign ) = 0;
    /**
     * use the public key to verify data's signature
    */
    virtual int verify( const std::string & input, const int hash_alg, const std::string & sign ) = 0;
    /**
     * use this key to wrap input data as below:
     * 1. generate a AES key to encrypt input data in CBC mode
     * 2. use the public key to encrypt the AES key and iv data
     * 
     * input[in]: plain data
     * output[out]: wrapped data, it's a proto serialized string, defined as below: 
     *          message CypherData {
     *                  bytes key = 1;          // a random AES key cipher, encrypted by public key
     *                  bytes public_key = 2;   // ignore, will be not set
     *                  bytes data = 3;         // cipher of input, encrypted by AES key
     *          }
     * 
     * return: 0 if successuful, otherwise return an error code.
     * 
    */
    virtual int wrap_data( const std::string & input, std::string & output ) = 0;
    /**
     * use this key to unwrap input data as below:
     * 1. use the private key to decrypt AES key cipher
     * 2. use the AES key and iv data to decrypt data cipher
     * 
     * input[in]: cipher data, it's a serialized string of proto data WrappedData
     * output[out]: the decrypted data of WrappedData.data.
     * 
     * return: 0 if successuful, otherwise return an error code.
    */
    virtual int unwrap_data( const std::string & input, std::string & output) = 0;
    /**
     * use the public key in this object to wrap data and a public key (optional)
     * 
     * input[in]: plain data
     * public_key_pem[in]: optional, the public key of encrypter.
     * output[out]: wrapped data, it's a proto serialized string, defined as below: 
     *          message CypherData {
     *                  bytes key = 1;          // a random AES key cipher, encrypted by public key
     *                  bytes public_key = 2;   // cipher of public_key_pem, encrypted by AES key
     *                  bytes data = 3;         // cipher of input, encrypted by AES key
     *          }
     * 
     * return: 0 if successuful, otherwise return an error code.
    */   
    virtual int wrap_data( const std::string & input, const std::string & public_key_pem, std::string & output ) = 0;
    /**
     * use the private key in this object to unwrap a cipher data
     * 
     * input[in]: cipher data, it's a serialized string of proto data CypherData
     * output[out]: the decrypted data of CypherData.data.
     * public_key_pem[out]: the decrypted data of CypherData.public_key.
     * 
     * return: 0 if successuful, otherwise return an error code.
    */   
    virtual int unwrap_data( const std::string & input, std::string & output, std::string & public_key_pem ) = 0;
};

#endif //_ASYMM_KEY_H_
