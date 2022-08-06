#ifndef TEE_ARWEAVE_SERVER_QUOTE_GENERATION_H
#define TEE_ARWEAVE_SERVER_QUOTE_GENERATION_H


#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER)
#include <Windows.h>
#include <tchar.h>
#endif

#include "sgx_urts.h"
#include "sgx_report.h"
#include "sgx_dcap_ql_wrapper.h"
#include "sgx_pce.h"
#include "sgx_error.h"
#include "sgx_quote_3.h"
#include <string>
#include "Enclave_u.h"
#include <iostream>

#define SGX_AESM_ADDR "SGX_AESM_ADDR"
#if defined(_MSC_VER)
#define ENCLAVE_PATH _T("enclave.signed.dll")
#else
#define ENCLAVE_PATH "enclave.signed.so"
#endif

/**
 * Generate the remote attestation report.
 * @param [in]pubkey_list_hash the hash of the public key list
 * @param [out]tee_report the remote attestation report in Base64 format
 * @return true on success, false on failure
 */
int QuoteGeneration(const std::string & request_id, const std::string& pubkey_list_hash, std::string& tee_report);



#endif //TEE_ARWEAVE_SERVER_QUOTE_GENERATION_H
