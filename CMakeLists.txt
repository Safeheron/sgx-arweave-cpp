cmake_minimum_required(VERSION 3.13)
project(tee-arweave-server "C" "CXX")

set(PROJECT_VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(SGX_SDK "/opt/intel/sgxsdk")
set(SGX_ARCH x64)
set(SGX_MODE HW CACHE STRING "SGX mode: SIM or HW")
set(SGX_BUILD RELEASE CACHE STRING "Build: DEBUG, PRERELEASE or RELEASE")

include (cmake/Init.cmake)
include (cmake/ConfigSGX.cmake)

set(CMAKE_PREFIX_PATH "/opt/protobuf/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH}  "/opt/protobuf/lib/pkgconfig")
set(PROTOC  "/opt/protobuf/bin/protoc")
find_package(PkgConfig REQUIRED)

set(TEE_UTIL_SGX_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/tee-util-sgx-1.0.0)
set(TEE_UTIL_SGX_INCLUDE_DIR ${TEE_UTIL_SGX_ROOT}/include)
set(TEE_UTIL_SGX_LIBRARY_DIR ${TEE_UTIL_SGX_ROOT}/lib)
set(TEE_UTIL_SGX_LIB_U ${TEE_UTIL_SGX_LIBRARY_DIR}/libtee_util_sgx_u.a)
set(TEE_UTIL_SGX_LIB_T ${TEE_UTIL_SGX_LIBRARY_DIR}/libtee_util_sgx_t.a)

set(MBEDTLS_SGX_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/mbedtls-sgx-2.6.0)
set(MBEDTLS_SGX_INCLUDE_DIR ${MBEDTLS_SGX_ROOT}/include)
set(MBEDTLS_SGX_LIBRARY_DIR ${MBEDTLS_SGX_ROOT}/lib)
set(MBEDTLS_SGX_LIB_U ${MBEDTLS_SGX_LIBRARY_DIR}/libmbedtls_SGX_u.a)
set(MBEDTLS_SGX_LIB_T ${MBEDTLS_SGX_LIBRARY_DIR}/libmbedtls_SGX_t.a)

set(SGX_SUITS_TSS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sgx-crypto-suites-tss-rsa)
set(SGX_SUITS_TSS_INCLUDE_DIR ${SGX_SUITS_TSS_ROOT}/include)
set(SGX_SUITS_TSS_LIBRARY_DIR ${SGX_SUITS_TSS_ROOT}/lib)

option(DEBUG "option for debug" OFF)
if (DEBUG)
        set(CMAKE_BUILD_TYPE "Debug")
        add_definitions(-DDEBUG -UNDEBUG -UEDEBUG)
else ()
        set(CMAKE_BUILD_TYPE "Release")
        add_definitions(-DNDEBUG -UEDEBUG -UDEBUG)
endif ()

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION})

add_subdirectory(App)
add_subdirectory(Enclave)


# package installed files to an tar.gz file.
add_custom_target(package
        COMMAND tar -zcvf ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz ${PROJECT_NAME}-${PROJECT_VERSION}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
message(STATUS "COMMAND: tar -zcvf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}")

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "\n\n")