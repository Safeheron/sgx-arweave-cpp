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

#set(CMAKE_PREFIX_PATH "/opt/protobuf/lib/pkgconfig")
#set(ENV{PKG_CONFIG_PATH}  "/opt/protobuf/lib/pkgconfig")
#set(PROTOC  "/opt/protobuf/bin/protoc")
#find_package(PkgConfig REQUIRED)

# sgx openssl
set(SGX_OPENSSL_ROOT /opt/intel/sgxssl)
set(SGX_OPENSSL_INCLUDE_DIR ${SGX_OPENSSL_ROOT}/include)
set(SGX_OPENSSL_LIBRARY_DIR ${SGX_OPENSSL_ROOT}/lib64)
set(SGX_OPENSSL_LIBRARY_FILE_TSSL ${SGX_OPENSSL_ROOT}/lib64/libsgx_tsgxssl.a)
set(SGX_OPENSSL_LIBRARY_FILE_USSL ${SGX_OPENSSL_ROOT}/lib64/libsgx_usgxssl.a)
set(SGX_OPENSSL_LIBRARY_FILE_TCRYPTO ${SGX_OPENSSL_ROOT}/lib64/libsgx_tsgxssl_crypto.a)

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
add_subdirectory(test)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "\n\n")