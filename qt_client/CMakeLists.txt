cmake_minimum_required(VERSION 3.30)
project(QtClient)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Vulkan_FOUND FALSE)

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@6")

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

add_executable(QtClient
        src/main.cpp
        src/mainwindow.cpp
        src/networkclient.cpp
        src/mainwindow.ui
        include/mainwindow.h
        include/networkclient.h
)

target_include_directories(QtClient PRIVATE include)
target_link_libraries(QtClient PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
) 