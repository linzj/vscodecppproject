cmake_minimum_required(VERSION 3.10)
project(Win32Window)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LibXml2 CONFIG REQUIRED)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE LibXml2::LibXml2)

if (WIN32)
  add_compile_definitions(${PROJECT_NAME} "UNICODE")
endif()
