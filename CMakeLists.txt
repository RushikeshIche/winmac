cmake_minimum_required(VERSION 3.20)
project(WinMac VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(winmac WIN32 src/main.cpp)

target_compile_definitions(winmac PRIVATE UNICODE _UNICODE)
target_include_directories(winmac PRIVATE include)
