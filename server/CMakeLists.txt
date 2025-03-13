cmake_minimum_required(VERSION 3.30)
project(server LANGUAGES CXX)

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(BOOST_ROOT "/opt/homebrew/opt/boost")
    set(Boost_INCLUDE_DIR "${BOOST_ROOT}/include")
    set(Boost_LIBRARY_DIR "${BOOST_ROOT}/lib")
    include_directories(${Boost_INCLUDE_DIR})
endif()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
)

add_definitions(-DBOOST_ASIO_HAS_CO_AWAIT)
set(Boost_USE_STATIC_LIBS ON)

include_directories("/opt/homebrew/opt/boost/include")
link_directories("/opt/homebrew/opt/boost/lib")
include_directories("/opt/homebrew/opt/nlohmann-json/include")
set(CMAKE_INCLUDE_PATH "/opt/homebrew/opt/boost/include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "/opt/homebrew/opt/boost/lib" ${CMAKE_LIBRARY_PATH})

find_package(Boost 1.87.0 REQUIRED COMPONENTS thread system chrono HINTS "/opt/homebrew/opt/boost")
find_package(nlohmann_json 3.11.3 REQUIRED)
find_package(nlohmann_json REQUIRED HINTS "/opt/homebrew/opt/nlohmann-json")

include_directories(
        ${Boost_INCLUDE_DIR}
        ${nlohmann_json_INCLUDE_DIRS}
)

set(SOURCES
        src/main.cpp
        src/connection_acceptor.cpp
        src/room.cpp
        src/rooms_singleton.cpp
        src/socket_connection.cpp
        src/server_controller.cpp
)

set(HEADERS
        include/headers.h
        include/connection_acceptor.h
        include/room.h
        include/rooms_singleton.h
        include/room_command.h
        include/room_message.h
        include/socket_connection.h
        include/server_controller.h
)

add_executable(server ${SOURCES} ${HEADERS})

target_link_libraries(server PRIVATE
        Boost::boost
        Boost::thread
        Boost::system
        Boost::chrono
        nlohmann_json::nlohmann_json
)

if(APPLE)
    find_library(SECURITY_LIB Security)
    find_library(SYSTEMCONFIGURATION_LIB SystemConfiguration)

    target_link_libraries(server PRIVATE
            ${SECURITY_LIB}
            ${SYSTEMCONFIGURATION_LIB}
            "-framework Foundation"
    )
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    add_compile_options(
            -O3
            -flto=full
            -mcpu=apple-m3
    )
    add_link_options(
            -lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib
            -mcpu=apple-m3
    )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)