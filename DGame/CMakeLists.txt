cmake_minimum_required(VERSION 3.5)
project(DGame VERSION 1.0.0 LANGUAGES CXX)

add_library(DGame Main.cpp)

target_include_directories(DGame
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

#target_compile_options(DGame PRIVATE -std=2z)
target_compile_features(DGame PRIVATE cxx_std_17)