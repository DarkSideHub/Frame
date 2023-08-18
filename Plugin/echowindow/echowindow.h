// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PLUGINUSING_H
#define PLUGINUSING_H

#include <QWidget>

#include "../../Frame/ExtensionSystem.h"
// #include "/home/li/work/mhttp/Frame/ExtensionSystem.h"

QT_BEGIN_NAMESPACE
class QString;
class QLineEdit;
class QLabel;
class QPushButton;
class QGridLayout;
QT_END_NAMESPACE

//! [0]
class EchoWindow : public QWidget {
  Q_OBJECT

 public:
  EchoWindow();

 private slots:
  void sendEcho();

 private:
  void createGUI();
  bool loadPlugin();

  // ExtensionSystem::PluginManager::IPlugin *echoInterface;
  QLineEdit *lineEdit;
  QLabel *label;
  QPushButton *button;
  QGridLayout *layout;
};
//! [0]

#endif
