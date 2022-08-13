# CMake for application
#
# Set safeheron library paths
#set(SAFEHERON_INCLUDE_DIR /usr/local/include/safeheron)
#set(SAFEHERON_LIB /usr/local/lib/libCryptoSuites.so)


# Set all flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

# Set safeheron library paths
set(SAFEHERON_INCLUDE_DIR /usr/local/include/safeheron)
set(SAFEHERON_LIB /usr/local/lib/libCryptoSuites.so)

include_directories(
		${SAFEHERON_INCLUDE_DIR}
)

add_executable(test
        	main.cpp
			callback_listener.cpp
)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(test
			-lboost_system
			-lcrypto -lssl
			-lcpprest
			${SAFEHERON_LIB}
)

message(STATUS "\n\n")
