cmake_minimum_required(VERSION 3.30)
project(server_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-c++98-compat
)

set(BOOST_ROOT "/opt/homebrew/opt/boost")
include_directories("/opt/homebrew/opt/boost/include")
link_directories("/opt/homebrew/opt/boost/lib")
include_directories("/opt/homebrew/opt/nlohmann-json/include")
set(CMAKE_INCLUDE_PATH "/opt/homebrew/opt/boost/include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "/opt/homebrew/opt/boost/lib" ${CMAKE_LIBRARY_PATH})

add_definitions(-DBOOST_ASIO_HAS_CO_AWAIT)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.87.0 REQUIRED COMPONENTS thread system chrono HINTS "/opt/homebrew/opt/boost")
find_package(nlohmann_json 3.11.3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
        src/main.cpp
        src/room_test.cpp
        src/rooms_singleton_test.cpp
        src/mock_socket_connection.cpp
        src/socket_connection_test.cpp
        ../server/src/server_controller.cpp
        ../server/src/rooms_singleton.cpp
        ../server/src/room.cpp
        ../server/src/socket_connection.cpp
        ../server/src/connection_acceptor.cpp
)

add_executable(server_tests ${TEST_SOURCES})

find_library(SECURITY_LIB Security)
find_library(SYSTEMCONFIGURATION_LIB SystemConfiguration)

target_link_libraries(server_tests
        PRIVATE
        ${SECURITY_LIB}
        ${SYSTEMCONFIGURATION_LIB}
        "-framework Foundation"
        gtest_main
        gmock_main
        Boost::boost
        Boost::thread
        Boost::system
        Boost::chrono
        nlohmann_json::nlohmann_json
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    add_compile_options(
            -O3
            -flto=full
            -mtune=apple-m3
    )
    add_link_options(
            -lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib
            -mtune=apple-m3
    )
endif()

add_test(
        NAME RoomsSingletonTest
        COMMAND server_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)