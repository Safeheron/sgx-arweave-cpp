find_package(SafeheronCryptoSuitesSgx REQUIRED)
find_package(CryptoTSSRSASgx REQUIRED)

set(enclave enclave)

ssgx_add_enclave_library(${enclave}
        USE_SGXSSL ON
        SRCS
                Enclave.cpp
                common/log_t.cpp
                common/tee_util.cpp
                json/cJSON.c
                json/json.cpp
                shell/Dispatcher.cpp
                tasks/GenerateTask.cpp
                tasks/QueryTask.cpp
        TRUSTED_LIBS
                ${SafeheronCryptoSuitesSgx_LIBRARY}
                ${CryptoTSSRSASgx_LIBRARY}
        EDL Enclave.edl
        EDL_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${ssgx_EDL_DIRS}
)

target_compile_features(${enclave} PRIVATE cxx_std_14)

set_target_properties(${enclave} PROPERTIES PREFIX "")

target_include_directories(${enclave} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SafeheronCryptoSuitesSgx_INCLUDE_DIR}
        ${SafeheronCryptoSuitesSgx_INCLUDE_DIR}/crypto-suites
        ${CryptoTSSRSASgx_INCLUDE_DIR}
)

ssgx_sign_enclave(${enclave}
        KEY Enclave_private_test.pem
        CONFIG Enclave.config.xml
)

install(FILES $<TARGET_FILE_DIR:${enclave}>/${enclave}.signed.so
        DESTINATION ./
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
