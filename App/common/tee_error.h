#ifndef PROJECT_TEE_ERROR_H
#define PROJECT_TEE_ERROR_H

#include <sgx_error.h>

typedef struct _ra_errlist_t {
    int err;
    const char *msg;
    const char *sug; /* Suggestion */
} ra_errlist_t;

/* Error code returned by SGX api */
static ra_errlist_t sgx_errlist[] = {
        { SGX_SUCCESS, "Call the ecall function successfully.", nullptr },
        { SGX_ERROR_UNEXPECTED, "Unexpected error occurred.", nullptr },
        { SGX_ERROR_INVALID_PARAMETER, "The parameter is incorrect.", nullptr },
        { SGX_ERROR_OUT_OF_MEMORY, "Not enough memory is available to complete this operation.", nullptr },
        { SGX_ERROR_ENCLAVE_LOST, "Enclave lost after power transition or used in child process created by linux:fork().", nullptr },
        { SGX_ERROR_INVALID_STATE, "SGX API is invoked in incorrect order or state.", nullptr },
        { SGX_ERROR_FEATURE_NOT_SUPPORTED, "Feature is not supported on this platform.", nullptr },
        { SGX_PTHREAD_EXIT, "Enclave is exited with pthread_exit().", nullptr },

        { SGX_ERROR_INVALID_FUNCTION, "The ecall/ocall index is invalid.", nullptr },
        { SGX_ERROR_OUT_OF_TCS, "The enclave is out of TCS.", nullptr },
        { SGX_ERROR_ENCLAVE_CRASHED, "The enclave is crashed.", nullptr },
        { SGX_ERROR_ECALL_NOT_ALLOWED, "The ECALL is not allowed at this time, e.g. ecall is blocked by the dynamic entry table, or nested ecall is not allowed during initialization.", nullptr },
        { SGX_ERROR_OCALL_NOT_ALLOWED, "The OCALL is not allowed at this time, e.g. ocall is not allowed during exception handling.", nullptr },
        { SGX_ERROR_STACK_OVERRUN, "The enclave is running out of stack.", nullptr },

        { SGX_ERROR_UNDEFINED_SYMBOL, "The enclave image has undefined symbol.", nullptr },
        { SGX_ERROR_INVALID_ENCLAVE, "The enclave image is not correct.", nullptr },
        { SGX_ERROR_INVALID_ENCLAVE_ID, "The enclave id is invalid.", nullptr },
        { SGX_ERROR_INVALID_SIGNATURE, "Invalid enclave signature.", nullptr },
        { SGX_ERROR_NDEBUG_ENCLAVE, "The enclave is signed as product enclave, and can not be created as debuggable enclave.", nullptr },
        { SGX_ERROR_OUT_OF_EPC, "Not enough EPC is available to load the enclave.", nullptr },
        { SGX_ERROR_NO_DEVICE, "Invalid SGX device.", "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards." },
        { SGX_ERROR_MEMORY_MAP_CONFLICT, "Page mapping failed in driver.", nullptr },
        { SGX_ERROR_INVALID_METADATA, "Invalid enclave metadata.", nullptr },
        { SGX_ERROR_DEVICE_BUSY, "SGX device is busy, mostly EINIT failed.", nullptr },
        { SGX_ERROR_INVALID_VERSION, "Enclave version was invalid.", nullptr },
        { SGX_ERROR_MODE_INCOMPATIBLE, "The target enclave 32/64 bit mode or sim/hw mode is incompatible with the mode of current uRTS.", nullptr },
        { SGX_ERROR_ENCLAVE_FILE_ACCESS, "Can't open enclave file.", nullptr },
        { SGX_ERROR_INVALID_MISC, "The MiscSelct/MiscMask settings are not correct.", nullptr },
        { SGX_ERROR_INVALID_LAUNCH_TOKEN, "The launch token is not correct.", nullptr },

        { SGX_ERROR_MAC_MISMATCH, "Indicates verification error for reports, sealed datas, etc.", nullptr },
        { SGX_ERROR_INVALID_ATTRIBUTE, "The enclave is not authorized, e.g., requesting invalid attribute or launch key access on legacy SGX platform without FLC.", nullptr },
        { SGX_ERROR_INVALID_CPUSVN, "The cpu svn is beyond platform's cpu svn value.", nullptr },
        { SGX_ERROR_INVALID_ISVSVN, "The isv svn is greater than the enclave's isv svn.", nullptr },
        { SGX_ERROR_INVALID_KEYNAME, "The key name is an unsupported value.", nullptr },

        { SGX_ERROR_SERVICE_UNAVAILABLE, "Indicates aesm didn't respond or the requested service is not supported.", nullptr },
        { SGX_ERROR_SERVICE_TIMEOUT, "The request to aesm timed out.", nullptr },
        { SGX_ERROR_AE_INVALID_EPIDBLOB, "Indicates epid blob verification error.", nullptr },
        { SGX_ERROR_SERVICE_INVALID_PRIVILEGE, "Enclave not authorized to run, .e.g. provisioning enclave hosted in an app without access rights to /dev/sgx_provision.", nullptr },
        { SGX_ERROR_EPID_MEMBER_REVOKED, "The EPID group membership is revoked.", nullptr },
        { SGX_ERROR_UPDATE_NEEDED, "SGX needs to be updated.", nullptr },
        { SGX_ERROR_NETWORK_FAILURE, "Network connecting or proxy setting issue is encountered.", nullptr },
        { SGX_ERROR_AE_SESSION_INVALID, "Session is invalid or ended by server.", nullptr },
        { SGX_ERROR_BUSY, "The requested service is temporarily not available.", nullptr },
        { SGX_ERROR_MC_NOT_FOUND, "The Monotonic Counter doesn't exist or has been invalided.", nullptr },
        { SGX_ERROR_MC_NO_ACCESS_RIGHT, "Caller doesn't have the access right to specified VMC.", nullptr },
        { SGX_ERROR_MC_USED_UP, "Monotonic counters are used out.", nullptr },
        { SGX_ERROR_MC_OVER_QUOTA, "Monotonic counters exceeds quota limitation.", nullptr },
        { SGX_ERROR_KDF_MISMATCH, "Key derivation function doesn't match during key exchange.", nullptr },
        { SGX_ERROR_UNRECOGNIZED_PLATFORM, "EPID Provisioning failed due to platform not recognized by backend server.", nullptr },
        { SGX_ERROR_UNSUPPORTED_CONFIG, "The config for trigging EPID Provisiong or PSE Provisiong&LTP is invalid.", nullptr },

        { SGX_ERROR_NO_PRIVILEGE, "Not enough privilege to perform the operation.", nullptr },

        /* SGX Protected Code Loader Error codes*/
        { SGX_ERROR_PCL_ENCRYPTED, "trying to encrypt an already encrypted enclave.", nullptr },
        { SGX_ERROR_PCL_NOT_ENCRYPTED, "trying to load a plain enclave using sgx_create_encrypted_enclave.", nullptr },
        { SGX_ERROR_PCL_MAC_MISMATCH, "section mac result does not match build time mac.", nullptr },
        { SGX_ERROR_PCL_SHA_MISMATCH, "Unsealed key MAC does not match MAC of key hardcoded in enclave binary.", nullptr },
        { SGX_ERROR_PCL_GUID_MISMATCH, "GUID in sealed blob does not match GUID hardcoded in enclave binary.", nullptr },

        /* SGX errors are only used in the file API when there is no appropriate EXXX (EINVAL, EIO etc.) error code */
        { SGX_ERROR_FILE_BAD_STATUS, "The file is in bad status, run sgx_clearerr to try and fix it.", nullptr },
        { SGX_ERROR_FILE_NO_KEY_ID, "The Key ID field is all zeros, can't re-generate the encryption key.", nullptr },
        { SGX_ERROR_FILE_NAME_MISMATCH, "The current file name is different then the original file name (not allowed, substitution attack).", nullptr },
        { SGX_ERROR_FILE_NOT_SGX_FILE, "The file is not an SGX file.", nullptr },
        { SGX_ERROR_FILE_CANT_OPEN_RECOVERY_FILE, "A recovery file can't be opened, so flush operation can't continue (only used when no EXXX is returned).", nullptr },
        { SGX_ERROR_FILE_CANT_WRITE_RECOVERY_FILE, "A recovery file can't be written, so flush operation can't continue (only used when no EXXX is returned).", nullptr },
        { SGX_ERROR_FILE_RECOVERY_NEEDED, "When openeing the file, recovery is needed, but the recovery process failed.", nullptr },
        { SGX_ERROR_FILE_FLUSH_FAILED, "fflush operation (to disk) failed (only used when no EXXX is returned).", nullptr },
        { SGX_ERROR_FILE_CLOSE_FAILED, "fclose operation (to disk) failed (only used when no EXXX is returned).", nullptr },

        { SGX_ERROR_UNSUPPORTED_ATT_KEY_ID, "platform quoting infrastructure does not support the key.", nullptr },
        { SGX_ERROR_ATT_KEY_CERTIFICATION_FAILURE, "Failed to generate and certify the attestation key.", nullptr },
        { SGX_ERROR_ATT_KEY_UNINITIALIZED, "The platform quoting infrastructure does not have the attestation key available to generate quote.", nullptr },
        { SGX_ERROR_INVALID_ATT_KEY_CERT_DATA, "The data returned by the platform library's sgx_get_quote_config() is invalid.", nullptr },
        { SGX_ERROR_PLATFORM_CERT_UNAVAILABLE, "The PCK Cert for the platform is not available.", nullptr },

        { SGX_INTERNAL_ERROR_ENCLAVE_CREATE_INTERRUPTED, "The ioctl for enclave_create unexpectedly failed with EINTR.", nullptr }
};

const char * t_strerror( int error_code );

#endif //PROJECT_TEE_ERROR_H
