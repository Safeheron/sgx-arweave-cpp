/**
 * @file tee_error.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#define TEE_OK                              0
#define TEE_ERROR_BASE                      0x10000000
#define TEE_ERROR_INVALID_PARAMETER         TEE_ERROR_BASE + 0x1
#define TEE_ERROR_DISPATCH_REQUEST_FAILED   TEE_ERROR_BASE + 0x2
#define TEE_ERROR_PUBLIST_KEY_HASH          TEE_ERROR_BASE + 0x3
#define TEE_ERROR_CREATE_KEYCONTEXT         TEE_ERROR_BASE + 0x4
#define TEE_ERROR_CALC_HASH_FAILED          TEE_ERROR_BASE + 0x5
#define TEE_ERROR_GENERATE_KEYSHARDS        TEE_ERROR_BASE + 0x6
#define TEE_ERROR_KEYMETA_IS_WRONG          TEE_ERROR_BASE + 0x7
#define TEE_ERROR_PUBKEY_IS_WRONG           TEE_ERROR_BASE + 0x8
#define TEE_ERROR_ECIES_ENC_FAILED          TEE_ERROR_BASE + 0x9
#define TEE_ERROR_MALLOC_OUTSIDE            TEE_ERROR_BASE + 0xA
#define TEE_ERROR_MALLOC_FAILED             TEE_ERROR_BASE + 0xB
#define TEE_ERROR_REQUEST_IS_EXIST          TEE_ERROR_BASE + 0xC
#define TEE_ERROR_KEYSTATUS_IS_BAD          TEE_ERROR_BASE + 0xD
#define TEE_ERROR_ENCLAVE_IS_BUSY           TEE_ERROR_BASE + 0xE
#define TEE_ERROR_INTERNAL_ERROR            TEE_ERROR_BASE + 0xF