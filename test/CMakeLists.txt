cmake_minimum_required(VERSION 3.2)
project(project_tests)

set(CMAKE_AUTOMOC ON)

set(INSTALL_EXAMPLEDIR "minimal-qml")

set(CMAKE_PREFIX_PATH "/home/li/work/QT/6.5.1/gcc_64/")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)


include_directories(${PROJECT_HEADERS_DIR})
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SOURCE_FILES main.cpp src/httptest.cpp)


# set_target_properties(${PROJECT_NAME} PROPERTIES
#    WIN32_EXECUTABLE TRUE
#    MACOSX_BUNDLE TRUE
#)



add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} HTTPLIB  GTest::gtest_main  Qt::Core Qt::Gui Qt::Qml)
include(GoogleTest)
gtest_discover_tests(project_tests)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
