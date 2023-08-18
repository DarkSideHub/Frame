// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PLUGINB_H
#define PLUGINB_H

#include <QObject>
#include <QtPlugin>

#include "../../Frame/ExtensionSystem.h"

//! [0]
class PluginB : public ExtensionSystem::IPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.PluginB" FILE
                        "/home/li/work/mhttp/Plugin/pluginB/PluginB.json")
  Q_INTERFACES(ExtensionSystem::IPlugin)

 public:
  bool Initialize(
      std::vector<std::string> Args = std::vector<std::string>()) noexcept;
  bool Run() noexcept;
  ExtensionSystem::StopFlag Stop() noexcept {
    return ExtensionSystem::StopFlag::AsynchronousStop;
  };
  // 业务逻辑实现
  void DoWork(QString data) noexcept {
    emit sendLog("PluginB插件执行测试", LOGLEVEL::Info);
    QJsonObject mjson;
    mjson.insert("info", "PluginB插件测试结束");
    ExtensionSystem::MSG Msg = std::make_tuple("PluginB", "Single", mjson);
    emit sendSignal(Msg);
  };
 public slots:
  void Receive(ExtensionSystem::MSG Msg) noexcept;
};
//! [0]

#endif
