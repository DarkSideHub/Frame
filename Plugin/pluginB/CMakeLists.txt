# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_plugin(PluginB
    CLASS_NAME PluginB
    PluginB.cpp PluginB.h
    ../../Frame/ExtensionSystem.cpp
)

set_target_properties(PluginB PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "/home/li/work/mhttp/Frame/plugins"
)

target_include_directories(PluginB PRIVATE
    ../../Frame
)

# target_include_directories(echoplugin PRIVATE
#     ../echowindow
# )

target_link_libraries(PluginB PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    Qt6::Widgets
)


