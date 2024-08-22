cmake_minimum_required(VERSION 3.10)
project(Win32Window)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cpp)

add_compile_definitions(${PROJECT_NAME} "UNICODE")