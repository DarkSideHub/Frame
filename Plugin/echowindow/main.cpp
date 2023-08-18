// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>

#include "echowindow.h"
// #include "echointerface.h"

//! [0]
int main(int argv, char *args[]) {
  QDir pluginsDir("/home/li/work/mhttp/Frame/plugins");
  ExtensionSystem::IPlugin *echoInterface;
  auto &minstance =
      ExtensionSystem::PluginManager::PLUGIN_MANAGE::GetInstance();
  const QStringList entries = pluginsDir.entryList(QDir::Files);
  for (const QString &fileName : entries) {
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();
    std::cout << pluginLoader.errorString().toStdString() << std::endl;
    if (plugin) {
      QJsonDocument jsDoc(pluginLoader.metaData());
      QFile file1("sa.json");
      file1.open(QIODevice::ReadWrite);
      file1.write(jsDoc.toJson());
      file1.close();
      echoInterface = qobject_cast<ExtensionSystem::IPlugin *>(plugin);

      // 插件sendSignal与PM的receive绑定
      QObject::connect(echoInterface, &ExtensionSystem::IPlugin::sendSignal,
                       &minstance,
                       &ExtensionSystem::PluginManager::PLUGIN_MANAGE::Receive);
      echoInterface->Initialize();
    }
    pluginLoader.unload();
  }

  return 0;
}
//! [0]
