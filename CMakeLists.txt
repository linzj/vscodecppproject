cmake_minimum_required(VERSION 3.10)
project(forwardemulatorport)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_IS_CLANG TRUE CACHE BOOL "Indicates whether the compiler is Clang or AppleClang")
else()
    set(CMAKE_IS_CLANG FALSE CACHE BOOL "Indicates whether the compiler is Clang or AppleClang")
endif()

add_executable(${PROJECT_NAME} src/main.cpp)

if (WIN32)
  add_compile_definitions(${PROJECT_NAME} "UNICODE")
endif()
