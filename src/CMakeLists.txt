cmake_minimum_required(VERSION 3.2)
project(SRC)



add_subdirectory(httplib)
set(SOURCE_FILES main.cpp)


add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} HTTPLIB)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_INSTALL_BIN_DIR})
