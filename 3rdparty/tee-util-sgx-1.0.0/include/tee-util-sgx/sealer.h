/*************************************************
 * File name : sealer.h
 * Introduce : The header file for class Sealer, which seal/unseal
 *             data use enclave seal key.
 * 
 * Create: 2021-07-06 by yyf
 * 
 *************************************************/

#ifndef _SEALER_H_
#define _SEALER_H_

#include <mbedtls/aes.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <sgx_trts.h>
#include <sgx_tseal.h>
#include <sgx_key.h>
#include <sgx_utils.h>
#include <string>

using namespace std;

#define SEAL_KEY_SIZE 16
#define CIPHER_BLOCK_SIZE 16
#define ENCRYPT_OPERATION true
#define DECRYPT_OPERATION false
#define HASH_VALUE_SIZE_IN_BYTES 32

#define POLICY_UNIQUE 1
#define POLICY_PRODUCT 2

#define MAX_OPT_MESSAGE_LEN 128
#define IV_SIZE 16
#define SIGNATURE_LEN 32

// errors shared by host and enclaves
#define ERROR_SIGNATURE_VERIFY_FAIL 1
#define ERROR_OUT_OF_MEMORY 2
#define ERROR_GET_SEALKEY 3
#define ERROR_SIGN_SEALED_DATA_FAIL 4
#define ERROR_CIPHER_ERROR 5
#define ERROR_UNSEALED_DATA_FAIL 6

typedef struct _sealed_signed_data_t
{
    size_t total_size;
    unsigned char signature[SIGNATURE_LEN];
    unsigned char opt_msg[MAX_OPT_MESSAGE_LEN];
    unsigned char iv[IV_SIZE];
    uint16_t key_policy; 
    size_t original_data_size;
    size_t encrypted_data_len;
    unsigned char encrypted_data[];
} sealed_signed_data_t;


class Sealer
{
public:
    Sealer();
    virtual ~Sealer();

public:
    int seal_data(
        uint16_t seal_policy,
        unsigned char* opt_mgs,
        size_t opt_msg_len,
        unsigned char* data,
        size_t data_size,
        sealed_signed_data_t** sealed_data,
        size_t* sealed_data_size);

    int unseal_data(
        sealed_signed_data_t* sealed_data,
        size_t sealed_data_size,
        unsigned char** data,
        size_t* data_size);

  private:
    void init_mbedtls(void);
    void cleanup_mbedtls(void);
    int generate_iv(unsigned char* iv, unsigned int ivLen);
    int get_seal_key_by_policy(int policy, sgx_key_128bit_t * seal_key);
    int padding_data(const unsigned char* data, int data_len, unsigned char* padded_data, int* padded_len);
    int cipher_data(bool encrypt, const unsigned char* input_data, const unsigned int input_data_size, const unsigned char* key, const unsigned int key_size,  const unsigned char* iv,  unsigned char* output_data);
    int sign_sealed_data( sealed_signed_data_t* sealed_data, unsigned char* key, unsigned int key_size, uint8_t* signature );
    //
    void dump_data(const char* name, unsigned char* data, size_t data_size);

  private:
    mbedtls_ctr_drbg_context m_ctr_drbg_contex;
    mbedtls_entropy_context m_entropy_context;
};
#endif //_SEALER_H_
