#include "quote_generation.h"
#include <safeheron/crypto-encode/base64.h>

extern sgx_enclave_id_t global_eid;

int QuoteGeneration(const std::string & request_id, 
                    const std::string& pubkey_list_hash, 
                    std::string& tee_report)
{
    std::string temp;
    std::string aaa;
    int ret = 0;
    uint32_t retval = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    quote3_error_t qe3_ret = SGX_QL_SUCCESS;
    uint32_t quote_size = 0;
    uint8_t* p_quote_buffer = nullptr;
    sgx_target_info_t qe_target_info;
    sgx_report_t app_report;
    sgx_quote3_t *p_quote;
    sgx_ql_auth_data_t *p_auth_data;
    sgx_ql_ecdsa_sig_data_t *p_sig_data;
    sgx_ql_certification_data_t *p_cert_data;
    FILE *fptr = nullptr;
    bool is_out_of_proc = false;
    char *out_of_proc = getenv(SGX_AESM_ADDR);
    if(out_of_proc)
        is_out_of_proc = true;


#if !defined(_MSC_VER)
    // There 2 modes on Linux: one is in-proc mode, the QE3 and PCE are loaded within the user's process.
    // the other is out-of-proc mode, the QE3 and PCE are managed by a daemon. If you want to use in-proc
    // mode which is the default mode, you only need to install libsgx-dcap-ql. If you want to use the
    // out-of-proc mode, you need to install libsgx-quote-ex as well. This sample is built to demo both 2
    // modes, so you need to install libsgx-quote-ex to enable the out-of-proc mode.
    if(!is_out_of_proc)
    {
        // Following functions are valid in Linux in-proc mode only.
        qe3_ret = sgx_qe_set_enclave_load_policy(SGX_QL_PERSISTENT);
        if(SGX_QL_SUCCESS != qe3_ret) {
            printf("Error in set enclave load policy: 0x%04x\n", qe3_ret);
            ret = -1;
            goto CLEANUP;
        }

        // Try to load PCE and QE3 from Ubuntu-like OS system path
        if (SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_PCE_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_pce.signed.so.1") ||
            SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_QE3_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_qe3.signed.so.1") ||
            SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_IDE_PATH, "/usr/lib/x86_64-linux-gnu/libsgx_id_enclave.signed.so.1")) {

            // Try to load PCE and QE3 from RHEL-like OS system path
            if (SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_PCE_PATH, "/usr/lib64/libsgx_pce.signed.so.1") ||
                SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_QE3_PATH, "/usr/lib64/libsgx_qe3.signed.so.1") ||
                SGX_QL_SUCCESS != sgx_ql_set_path(SGX_QL_IDE_PATH, "/usr/lib64/libsgx_id_enclave.signed.so.1")) {
                printf("Error in set PCE/QE3/IDE directory.\n");
                ret = -1;
                goto CLEANUP;
            }
        }

        qe3_ret = sgx_ql_set_path(SGX_QL_QPL_PATH, "/usr/lib/x86_64-linux-gnu/libdcap_quoteprov.so.1");
        if (SGX_QL_SUCCESS != qe3_ret) {
            qe3_ret = sgx_ql_set_path(SGX_QL_QPL_PATH, "/usr/lib64/libdcap_quoteprov.so.1");
            if(SGX_QL_SUCCESS != qe3_ret) {
                // Ignore the error, because user may want to get cert type=3 quote
                printf("Warning: Cannot set QPL directory, you may get ECDSA quote with `Encrypted PPID` cert type.\n");
            }
        }
    }
#endif

    qe3_ret = sgx_qe_get_target_info(&qe_target_info);
    if (qe3_ret == SGX_QL_NETWORK_ERROR)
        printf("test\n");
    if (SGX_QL_SUCCESS != qe3_ret) {
        printf("Error in sgx_qe_get_target_info. 0x%04x\n", qe3_ret);
        ret = -1;
        goto CLEANUP;
    }

    sgx_status = ecall_create_report(global_eid, &retval, (char*)request_id.c_str(), 
         (char*)pubkey_list_hash.c_str(), &qe_target_info, &app_report);
    if ((SGX_SUCCESS != sgx_status) || (0 != retval)) {
        printf("\nCall to get_app_enclave_report() failed\n");
        ret = -1;
        goto CLEANUP;
    }

    qe3_ret = sgx_qe_get_quote_size(&quote_size);
    if (SGX_QL_SUCCESS != qe3_ret) {
        printf("Error in sgx_qe_get_quote_size. 0x%04x\n", qe3_ret);
        ret = -1;
        goto CLEANUP;
    }

    p_quote_buffer = (uint8_t*)malloc(quote_size);
    if (nullptr == p_quote_buffer) {
        printf("Couldn't allocate quote_buffer\n");
        ret = -1;
        goto CLEANUP;
    }
    memset(p_quote_buffer, 0, quote_size);

    // Get the Quote
    qe3_ret = sgx_qe_get_quote(&app_report,
                               quote_size,
                               p_quote_buffer);
    if (SGX_QL_SUCCESS != qe3_ret) {
        printf( "Error in sgx_qe_get_quote. 0x%04x\n", qe3_ret);
        ret = -1;
        goto CLEANUP;
    }

    p_quote = (sgx_quote3_t*)p_quote_buffer;
    p_sig_data = (sgx_ql_ecdsa_sig_data_t *)p_quote->signature_data;
    p_auth_data = (sgx_ql_auth_data_t*)p_sig_data->auth_certification_data;
    p_cert_data = (sgx_ql_certification_data_t *)((uint8_t *)p_auth_data + sizeof(*p_auth_data) + p_auth_data->size);

#if defined(DEBUG_ARWEAVE)
#if _WIN32
    fopen_s(&fptr, "quote.dat", "wb");
#else
    fptr = fopen("quote.dat","wb");
#endif
    if( fptr )
    {
        fwrite(p_quote, quote_size, 1, fptr);
        fclose(fptr);
    }
#endif

    tee_report = safeheron::encode::base64::EncodeToBase64(p_quote_buffer, quote_size);

    if( !is_out_of_proc )
    {
        qe3_ret = sgx_qe_cleanup_by_policy();
        if(SGX_QL_SUCCESS != qe3_ret) {
            printf("Error in cleanup enclave load policy: 0x%04x\n", qe3_ret);
            ret = -1;
            goto CLEANUP;
        }
    }

    CLEANUP:

    if (nullptr != p_quote_buffer) {
        free(p_quote_buffer);
    }
    return ret;
}