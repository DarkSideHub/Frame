// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "echoplugin.h"

#include "moc_echoplugin.cpp"

bool SinglePlugin::Initialize(std::vector<std::string> Args) noexcept {
  emit sendLog("SinglePlugin插件初始化完成", LOGLEVEL::Info);
  return true;
};

bool SinglePlugin::Run() noexcept { return true; }
void SinglePlugin::Receive(ExtensionSystem::MSG Msg) noexcept {
  emit sendLog("SinglePlugin插件接收到消息", LOGLEVEL::Info);
  DoWork("");
};
