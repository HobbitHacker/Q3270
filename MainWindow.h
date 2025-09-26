/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

class MainWindow : public QMainWindow
{

  Q_OBJECT
  
  public:

      struct LaunchParms {
          MainWindow *mw;
          QString session;
      };

      MainWindow(MainWindow::LaunchParms = { nullptr, ""} );
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

      // Triggered by Terminal on connection state, and Settings if Hostname is empty
      void enableConnectMenu(bool state);

      // Triggered by Terminal on connection/disconnection
      void enableDisconnectMenu();
      void disableDisconnectMenu();

      void keyboardChanged(const QString &name);
      void coloursChanged(const QString &name);
      void checkColourThemeChanged(const QString &name);

  private:

      void populateMRU();
      void updateMRUList();

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
