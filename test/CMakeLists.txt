find_package(SafeheronCryptoSuites REQUIRED)

add_executable(test main.cpp)

target_compile_features(test PRIVATE cxx_std_11)

target_include_directories(test PRIVATE
        ${SafeheronCryptoSuites_INCLUDE_DIR}
        ${SafeheronCryptoSuites_INCLUDE_DIR}/crypto-suites
)

target_link_libraries(test PRIVATE
        ${SafeheronCryptoSuites_LIBRARY}
        -lboost_system
        -lcrypto -lssl
        -lcpprest
        -lpthread
)
