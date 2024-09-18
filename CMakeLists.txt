cmake_minimum_required(VERSION 3.10)
project(forwardemulatorport)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system)

if (WIN32)
  add_compile_definitions(${PROJECT_NAME} "UNICODE")
endif()
