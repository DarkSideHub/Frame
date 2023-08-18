cmake_minimum_required(VERSION 3.2)

project(cmake-project-template)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O0")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(PROJECT_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(PROJECT_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(PROJECT_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/httplib)

include_directories(${PROJECT_INSTALL_INCLUDE_DIR})
include_directories(${PROJECT_HEADERS_DIR})

add_subdirectory(Plugin)
add_subdirectory(Frame)
add_subdirectory(src)
add_subdirectory(test)





