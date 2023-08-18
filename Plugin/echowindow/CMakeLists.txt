# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(echopluginwindow
    echowindow.cpp echowindow.h
    main.cpp  ../../Frame/ExtensionSystem.h
    ../../Frame/ExtensionSystem.cpp
)

set_target_properties(echopluginwindow PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(echopluginwindow PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    Qt6::Widgets
)

target_include_directories(echopluginwindow PRIVATE
    ../../Frame
)

    # Build the shared plugin too when building this example target.
add_dependencies(echopluginwindow SinglePlugin PluginB)

install(TARGETS echopluginwindow
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
