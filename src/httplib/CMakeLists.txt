cmake_minimum_required(VERSION 3.2)
project(HTTPLIB C CXX)

# set(SOURCE_FILES *.cpp)

file(GLOB_RECURSE  SOURCE_FILES *.cxx)

find_package(OpenSSL REQUIRED)
set(OpenSSL_LIBS
    OpenSSL::Crypto OpenSSL::SSL
)

link_libraries(${OpenSSL_LIBS})

add_library(${PROJECT_NAME} SHARED STATIC ${SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_INSTALL_LIB_DIR})
