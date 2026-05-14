cmake_minimum_required(VERSION 3.24)
project(tee-arweave-server VERSION 0.9.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ssgx REQUIRED)
ssgx_set_build_mode(Release)
ssgx_set_hardware_mode(ON)

ssgx_ensure_rsa_key_exists(
        KEY_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Enclave/Enclave_private_test.pem"
        KEY_SIZE 3072
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION})

add_subdirectory(Enclave)
add_subdirectory(App)
add_subdirectory(test)
