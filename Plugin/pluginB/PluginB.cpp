// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "PluginB.h"

#include "moc_PluginB.cpp"

bool PluginB::Initialize(std::vector<std::string> Args) noexcept {
  emit sendLog("PluginB插件初始化完成", LOGLEVEL::Info);
  return true;
};

bool PluginB::Run() noexcept { return true; }
void PluginB::Receive(ExtensionSystem::MSG Msg) noexcept {
  emit sendLog("PluginB插件接收到消息", LOGLEVEL::Info);
  DoWork("");
};
