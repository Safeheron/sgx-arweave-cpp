# CMake for application
#
# Safeheron crypto suites library
find_package(SafeheronCryptoSuites REQUIRED)

# Set all flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

include_directories(
		${SafeheronCryptoSuites_INCLUDE_DIR}
)

add_executable(test
        	main.cpp
)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(test
			${SafeheronCryptoSuites_LIBRARY}
			-lboost_system
			-lcrypto -lssl
			-lcpprest
)

message(STATUS "\n\n")
