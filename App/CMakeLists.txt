find_package(SafeheronCryptoSuites REQUIRED)
find_package(glog REQUIRED)

set(app ${PROJECT_NAME})

ssgx_add_untrusted_executable(${app}
        SRCS
                App.cpp
                common/iniconfig.cpp
                common/log_u.cpp
                common/tee_error.cpp
                ocall/ocall.cpp
                server/keyshard_param.cpp
                server/listen_svr.cpp
                server/msg_handler.cpp
                server/thread_task.cpp
        UNTRUSTED_LIBS
                ${SafeheronCryptoSuites_LIBRARY}
                glog::glog
                -lcrypto -lssl
                -lboost_system -lcpprest
                -lz
        EDL Enclave.edl
        EDL_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/Enclave ${ssgx_EDL_DIRS}
)

target_compile_features(${app} PRIVATE cxx_std_11)

target_include_directories(${app} PRIVATE
        ${SafeheronCryptoSuites_INCLUDE_DIR}
        ${SafeheronCryptoSuites_INCLUDE_DIR}/crypto-suites
)

install(TARGETS ${app}
        DESTINATION ./
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/server.ini
        DESTINATION ./
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
)
