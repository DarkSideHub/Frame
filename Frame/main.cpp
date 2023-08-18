
#include "ExtensionSystem.h"
#include "Header.hpp"
#include "PlainTextHandler.h"

int main(int argc, char *argv[]) {
  QGuiApplication::setApplicationName("Text Editor");
  QGuiApplication::setOrganizationName("QtProject");
  QGuiApplication app(argc, argv);
  // 将C++中的Editor类注册为QML中的TextEditor类，命名空间为EditorComponents
  qmlRegisterType<EDITOR>("EditorComponents", 1, 0, "TextEditor");
  qmlRegisterType<LOGGER>("LoggerComponents", 1, 0, "Log");

  if (!std::filesystem::exists("logs")) {
    std::filesystem::create_directory("logs");
  }
  auto Filelogger = spdlog::daily_logger_st("Logger", "logs/daily.txt", 0, 00);
  QQmlApplicationEngine appEngine;
  appEngine.load(QUrl("/home/li/work/mhttp/Frame/resource/main.qml"));

  auto &mPluginManage =
      ExtensionSystem::PluginManager::PLUGIN_MANAGE::GetInstance();

  std::shared_ptr<LOGGER> LogObject;
  LogObject.reset(qobject_cast<LOGGER *>(
      appEngine.rootObjects().first()->findChild<QObject *>("LogObject")));
  if (LogObject) {
    QObject::connect(&mPluginManage,
                     &ExtensionSystem::PluginManager::PLUGIN_MANAGE::sendLog,
                     LogObject.get(), &LOGGER::ReceiveLog);
  } else {
    std::cout << "获取日志对象失败" << std::endl;
    return 0;
  }
  mPluginManage.Load();
  // 将插件的sendlog与日志对象的ReceiveLog绑定
  for (auto mPlugin : mPluginManage.GetAllObject()) {
    QObject::connect(
        &(*qobject_cast<ExtensionSystem::IPlugin *>(mPlugin.get())),
        &ExtensionSystem::IPlugin::sendLog, LogObject.get(),
        &LOGGER::ReceiveLog);
  }
  ExtensionSystem::MSG Msg;
  QJsonObject mjson;
  Msg = std::make_tuple("Window", "PluginB", mjson);
  mPluginManage.Receive(Msg);

  return app.exec();
}
