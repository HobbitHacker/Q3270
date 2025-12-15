/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QAction>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QTextDocument>
#include <QHostAddress>
#include <QTextEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QFontDialog>
#include <QSettings>
#include <QWidgetAction>
#include <QLabel>
#include <QString>

#include "Preferences/PreferencesDialog.h"
#include "Terminal.h"
#include "ColourTheme.h"
#include "KeyboardThemeDialog.h"
#include "ActiveSettings.h"
#include "CertificateDetails.h"
#include "Stores/SessionStore.h"
#include "Stores/KeyboardStore.h"
#include "Stores/ColourStore.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindowDialog;
    class CertificateDetails;
}

QT_END_NAMESPACE

#define MRU_COUNT 10

struct LaunchParms
{
        QWidget *parent = nullptr;
        QString session = {} ;
};

class MainWindow : public QMainWindow
{

  Q_OBJECT
  
  public:

      explicit MainWindow(LaunchParms lp = {});
      ~MainWindow();

  public slots:

      void closeEvent(QCloseEvent *c);
      void showEvent(QShowEvent *s);
      void resizeEvent(QResizeEvent *s);

  private slots:

      //File menu
      void menuNew();
      void menuDuplicate();
      void mruConnect();
      void menuSaveSessionAs();
      void menuSaveSession();
      void menuOpenSession();
      void menuManageSessions();
      void menuManageAutostartSessions();
      void menuQuit();

      // Session menu
      void menuConnect();
      void menuDisconnect();
      void menuSessionPreferences();

      // Themes menu
      void menuKeyboardTheme();
      void menuColourTheme();

      // The Help menu
      void menuAbout();
      void menuAboutConnection();

      // Triggered by Terminal on connection state
      void enableConnectMenu(bool state);

      // Triggered by Terminal on connection/disconnection
      void enableDisconnectMenu();
      void disableDisconnectMenu();

      // Triggered when a new keyboard or colour theme is chosen
      void activeKeyboardNameChanged(const QString &name);
      void activeColoursNameChanged(const QString &name);

      // Triggered when the user modifies a keyboard or colour theme
      void checkKeyboardThemeModified(const QString &name);
      void checkColourThemeModified(const QString &name);

      void checkHostNameChange(const QString &hostName, const int port, const QString &hostLU);

  private:

      void populateMRU();
      void updateMRUList();
      void storeAppWideSettings();

      ActiveSettings activeSettings;

      // Persistence Stores
      SessionStore sessionStore;
      KeyboardStore keyboardStore;
      ColourStore colourStore;

      // 3270 'hardware' layers
      Keyboard keyboard;
      CodePage codePage;
      Terminal *terminal;

      // Dialogs
      KeyboardThemeDialog *keyboardTheme;
      PreferencesDialog *settings;
      ColourTheme *colourTheme;

      int maxMruCount;

      QStringList mruList;

      Ui::MainWindowDialog *ui;

};
 
#endif
