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
#define TEE_ERROR_INVALID_PARAMETER         0x00000001
#define TEE_ERROR_DISPATCH_REQUEST_FAILED   0x00000002
#define TEE_ERROR_PUBLIST_KEY_HASH          0x00000003
#define TEE_ERROR_CREATE_KEYCONTEXT         0x00000004
#define TEE_ERROR_CALC_HASH_FAILED          0x00000005
#define TEE_ERROR_GENERATE_KEYSHARDS        0x00000006
#define TEE_ERROR_KEYMETA_IS_WRONG          0x00000007
#define TEE_ERROR_PUBKEY_IS_WRONG           0x00000008
#define TEE_ERROR_ECIES_ENC_FAILED          0x00000009
#define TEE_ERROR_MALLOC_OUTSIDE            0x0000000A
#define TEE_ERROR_MALLOC_FAILED             0x0000000B
#define TEE_ERROR_REQUEST_IS_EXIST          0x0000000C
#define TEE_ERROR_KEYSTATUS_IS_BAD          0x0000000D