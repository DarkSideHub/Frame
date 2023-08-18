// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PLUGINUSING_H
#define PLUGINUSING_H

#include <QObject>
#include <QtPlugin>

#include "../../Frame/ExtensionSystem.h"

//! [0]
class SinglePlugin : public ExtensionSystem::IPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Examples.SinglePlugin" FILE
                        "/home/li/work/mhttp/Plugin/plugin/SinglePlugin.json")
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
    emit sendLog("SinglePlugin插件执行测试", LOGLEVEL::Info);
    QJsonObject mjson;
    mjson.insert("info", "SinglePlugin插件测试结束");
    ExtensionSystem::MSG Msg = std::make_tuple("Single", "Window", mjson);
    emit sendSignal(Msg);
  };
 public slots:
  void Receive(ExtensionSystem::MSG Msg) noexcept;
};
//! [0]

#endif
