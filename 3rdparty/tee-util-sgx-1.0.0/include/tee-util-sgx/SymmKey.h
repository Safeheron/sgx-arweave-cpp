/*************************************************
 * File name : SymmKey.h
 * Introduce : The header file for class SymmKey, which implement
 *             symmetric crypto algorithms, such as AES. 
 *             You can use it like below:
 * 
 *             uint8_t iv[16] = {0};
 *             AESKey::gen_random_iv(iv);
 *             AESKey key = AESKey::gen_aes_key( 256 );
 *             ......
 *             key->encrypt_cbc(in, in_size, out, out_size, iv);
 *             ......
 *             delete key;
 *             key = nullptr;
 * 
 * Created by yyf on 2021-09-08.
 * 
 *************************************************/

#ifndef _SYMM_KEY_
#define _SYMM_KEY_

#include <string>

#define AES_KEY_IV_LEN          16              // the IV size of AES key, in bytes
#define MAX_INPUT_DATA_LEN      (10*1024*1024)  // the max data size for symmetic encrypt/decrypt or HASH

class SymmKey
{
protected:
    /**
     * Don't call construction directly!
    */
    SymmKey( );
public:
    virtual ~SymmKey();
public:
    /**
     * Symmetric crypto algorithms
    */
    typedef enum eSYMM_KEY_ALG
    {
        SYMM_KEY_AES       = 1  /* AES, support 128/192/256 bits */
    }SYMM_KEY_ALG;
public:
    /**
     * Generate an symmetric key object by random data
    */
    static SymmKey *gen_symm_key( eSYMM_KEY_ALG alg, size_t key_bits = 256 );
    /**
     * Generate an symmetric key by input data
    */
    static SymmKey *import_symm_key( eSYMM_KEY_ALG alg, const uint8_t *key_buf, size_t key_bits );
    /**
     * Release an symmetic key object
    */
    static void key_free( SymmKey* symm_key );
    /**
     * Get a random iv data
    */
    static int gen_random_iv( uint8_t *iv, size_t size );
    /**
     * Export symmetric key data and iv (for CBC) from key object
    */
    virtual int export_key( uint8_t *key_buf, size_t &key_bits ) = 0;
    /**
     * Encrypt data in cbc mode
    */
    virtual int encrypt_cbc( uint8_t *input, size_t ilen, uint8_t *output, size_t & olen, uint8_t *iv ) = 0;
    /**
     * Decrypt data in cbc mode
    */
    virtual int decrypt_cbc( uint8_t *input, size_t ilen, uint8_t *output, size_t & olen, uint8_t *iv ) = 0;
    /**
     * Encrypt data in ecb mode 
    */
    virtual int encrypt_ecb( uint8_t *input, size_t ilen, uint8_t *output, size_t & olen ) = 0;
    /**
     * Decrypt data in ecb mode
    */
    virtual int decrypt_ecb( uint8_t *input, size_t ilen, uint8_t *output, size_t & olen ) = 0;
};

#endif //_SYMM_KEY_
