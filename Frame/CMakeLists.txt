#cmake_minimum_required(VERSION 3.2)
project(Frame)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# 启用自动UI编译（AUTOUIC）
set(CMAKE_AUTOUIC ON)
# 自动元对象编译（AUTOMOC）
set(CMAKE_AUTOMOC ON)
# 自动资源编译（AUTORCC）
set(CMAKE_AUTORCC ON)

set(INSTALL_EXAMPLEDIR "minimal-qml")

set(CMAKE_PREFIX_PATH "/home/li/work/QT/6.5.1/gcc_64/")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 OPTIONAL_COMPONENTS Concurrent)


set(SOURCE_FILES "main.cpp" "PlainTextHandler.cpp" "ExtensionSystem.cpp" "TableModel.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)



target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::QuickControls2
    Qt::Concurrent
    HTTPLIB
)


install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
