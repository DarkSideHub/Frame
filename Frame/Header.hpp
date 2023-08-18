#ifndef HEADER_FILE
#define HEADER_FILE

#include <time.h>

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QFont>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMimeDatabase>
#include <QObject>
#include <QPluginLoader>
#include <QQmlContext>
#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickStyle>
#include <QQuickTextDocument>
#include <QStringDecoder>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QUrl>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#if defined(EXTENSIONSYSTEM_LIBRARY)
#define EXTENSIONSYSTEM_EXPORT Q_DECL_EXPORT
#else
#define EXTENSIONSYSTEM_EXPORT Q_DECL_IMPORT
#endif

#if defined(__linux) || defined(__linux__) || defined(linux)
#define PLATFORM "linux"
#elif defined(__CYGWEIN)
#define PLATFORM "cygwin"
#elif defined(__MINGW32)
#define PLATFORM "mingw"
#elif defined(____APPLE__)
#define PLATFORM "apple"
#elif defined(__WIN32) || defined(__WIN32__) || defined(WIN32)
#define PLATFORM "windows"
#elif defined(__FreeBSD)
#define PLATFORM "freebsd"
#endif

#endif
