cmake_minimum_required(VERSION 3.16)
project(CustomFormParentDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(CustomFormParentDemo
    main.cpp
    mainwindow.h
    mainwindow.cpp
    customform.h
    customform.cpp
)

target_link_libraries(CustomFormParentDemo PRIVATE Qt6::Widgets)