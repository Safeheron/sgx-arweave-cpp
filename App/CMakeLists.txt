# CMake for application
#
set(CMAKE_PREFIX_PATH "/opt/protobuf/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH}  "/opt/protobuf/lib/pkgconfig")
set(PROTOC  "/opt/protobuf/bin/protoc")

# Set common-util library paths
set(COMMON_UTIL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/common-util-1.0.0)
set(COMMON_UTIL_INCLUDE_DIR ${COMMON_UTIL_ROOT}/include)
set(COMMON_UTIL_LIBRARY_DIR ${COMMON_UTIL_ROOT}/lib)
set(COMMON_UTIL_LIB ${COMMON_UTIL_LIBRARY_DIR}/libcommon_util.a)

# Set safeheron library paths
set(SAFEHERON_INCLUDE_DIR /usr/local/include/safeheron)
set(SAFEHERON_LIB /usr/local/lib/libCryptoSuites.so)

# Set glog library paths
set(GLOG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glog-0.4.0)
set(GLOG_INCLUDE_DIR ${GLOG_ROOT}/include)
set(GLOG_LIBRARY_DIR ${GLOG_ROOT}/lib)
set(GLOG_LIB_GLOG ${GLOG_LIBRARY_DIR}/libglog.a)
set(GLOG_LIB_GFLAGS ${GLOG_LIBRARY_DIR}/libgflags.a)

# Set all flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron ${CMAKE_C_FLAGS} -std=c++11")

# Source files
set(APP_SRC_FILES 
        App.cpp
		common/tee_error.cpp common/app_util.cpp common/CJsonObject.cpp
		handler/Handler.cpp
		controller/thrdtask/thrdpool.cpp controller/controller.cpp controller/thrdtask/thread_task.cpp controller/CallBack.cpp controller/data_verify.cpp
		service/EnclaveAccess.cpp service/quote_generation.cpp
		)

# Use the edger8r to generate C bindings from the EDL file.

add_custom_command(OUTPUT Enclave_u.h Enclave_u.c
        DEPENDS ${CMAKE_SOURCE_DIR}/Enclave/Enclave.edl
        COMMAND ${SGX_EDGER8R} --untrusted ${CMAKE_SOURCE_DIR}/Enclave/Enclave.edl
        --search-path ${SGX_SDK}/include
        --search-path ${TEE_UTIL_SGX_LIBRARY_DIR}
)
message(STATUS "COMMAND: ${SGX_EDGER8R} --untrusted ${CMAKE_SOURCE_DIR}/Enclave.edl")

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} 
        ${APP_SRC_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/Enclave_u.c
)

include_directories(
        ${COMMON_UTIL_INCLUDE_DIR}
        ${SGX_SDK}/include
		${SAFEHERON_INCLUDE_DIR}
        ${TEE_UTIL_SGX_INCLUDE_DIR}
		handler
		controller
		controller/thrdtask
		service
)
# Needed for the generated file Enclave_u.h
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(${PROJECT_NAME}
        ${COMMON_UTIL_LIB}
        ${GLOG_LIB_GLOG} ${GLOG_LIB_GFLAGS}
        ${TEE_UTIL_SGX_LIB_U}
        -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -l${SGX_USVC_LIB} -lsgx_uprotected_fs
        -lpthread
		Threads::Threads
		-lboost_system -lcrypto -lssl -lcpprest -lsgx_dcap_ql -lsgx_quote_ex -lsgx_urts ${SAFEHERON_LIB}
	-lz
)

# Install target file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
        DESTINATION ./
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
)

message(STATUS "\n\n")
