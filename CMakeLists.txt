cmake_minimum_required(VERSION 3.20)
project(WinMac VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(winmac WIN32 src/main.cpp)
target_compile_definitions(winmac PRIVATE UNICODE _UNICODE)
target_include_directories(winmac PRIVATE include)

# DLL that contains the subclassing logic to be injected into target apps
add_library(winmac_hook SHARED src/WindowCustomizer.cpp)
target_compile_definitions(winmac_hook PRIVATE UNICODE _UNICODE)
target_include_directories(winmac_hook PRIVATE include)
target_link_libraries(winmac_hook comctl32)

# Target Application for testing
add_executable(test_app WIN32 src/test_app.cpp)
target_compile_definitions(test_app PRIVATE UNICODE _UNICODE)
