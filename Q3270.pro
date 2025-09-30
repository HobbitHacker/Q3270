QT       += core gui widgets network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

VERSION = 0.1.1

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           Q3270_VERSION=\"\\\"$${VERSION}\\\"\"

QMAKE_CXXFLAGS += -Wall -Wextra -Wunused-function \
                  -fdiagnostics-show-option -fno-diagnostics-color

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD

SOURCES += \
    ActiveSettings.cpp \
    Cell.cpp \
    CertificateDetails.cpp \
    CodePage.cpp \
    ColourTheme.cpp \
    DisplayScreen/ClickableSvgItem.cpp \
    DisplayScreen/DisplayScreen.cpp \
    DisplayScreen/DisplayScreen_Cursor.cpp \
    DisplayScreen/DisplayScreen_Status.cpp \
    DisplayScreen/DisplayScreen_Mouse.cpp \
    FunctionRegistry.cpp \
    Models/Colours.cpp \
    Models/KeyboardMap.cpp \
    Preferences/ColourSwatchWidget.cpp \
    Preferences/KeyboardMapWidget.cpp \
    Sessions/ManageAutoStartDialog.cpp \
    Sessions/ManageSessionsDialog.cpp \
    Sessions/OpenSessionDialog.cpp \
    HostAddressUtils.cpp \
    Keyboard.cpp \
    KeyboardThemeDialog.cpp \
    MainWindow.cpp \
    Preferences/PreferencesDialog.cpp \
    ProcessDataStream.cpp \
    Sessions/SaveSessionDialog.cpp \
    Models/Session.cpp \
    Sessions/SessionDialogBase.cpp \
    Sessions/SessionPreviewWidget.cpp \
    Stores/ColourStore.cpp \
    Stores/KeyboardStore.cpp \
    Stores/SessionStore.cpp \
    Terminal.cpp \
    main.cpp \
    SocketConnection.cpp

HEADERS += \
    ActiveSettings.h \
    Cell.h \
    CertificateDetails.h \
    CodePage.h \
    ColourTheme.h \
    DisplayScreen.h \
    DisplayScreen/ClickableSvgItem.h \
    FunctionRegistry.h \
    Models/Colours.h \
    Preferences/ColourSwatchWidget.h \
    Preferences/KeyboardMapWidget.h \
    Sessions/ManageAutoStartDialog.h \
    Sessions/ManageSessionsDialog.h \
    Sessions/OpenSessionDialog.h \
    HostAddressUtils.h \
    Keyboard.h \
    KeyboardThemeDialog.h \
    MainWindow.h \
    Preferences/PreferencesDialog.h \
    ProcessDataStream.h \
    Q3270.h \
    Sessions/SaveSessionDialog.h \
    Models/Session.h \
    Models/KeyboardMap.h \
    Sessions/SessionDialogBase.h \
    Sessions/SessionPreviewWidget.h \
    Stores/ColourStore.h \
    Stores/KeyboardStore.h \
    Stores/SessionStore.h \
    SocketConnection.h \
    Terminal.h

FORMS += \
    About.ui \
    AutoStart.ui \
    AutoStartAdd.ui \
    CertificateDetails.ui \
    ColourTheme.ui \
    KeyboardTheme.ui \
    MainWindowDialog.ui \
    Preferences/ColourSwatchWidget.ui \
    Preferences/KeyboardMapWidget.ui \
    Sessions/ManageAutoStartDialog.ui \
    Sessions/ManageSessions.ui \
    NewTheme.ui \
    Preferences/PreferencesDialog.ui \
    Sessions/SessionDialog.ui \
    Sessions/SessionPreview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Q3270.qrc

DISTFILES += \
    BACKLOG.md \
    Icons/clock.svg \
    Icons/connect.svg \
    Icons/database-gear.svg \
    Icons/database.svg \
    Icons/disconnect.svg \
    Icons/floppy.svg \
    Icons/keyboard.svg \
    Icons/lock-tick.svg \
    Icons/lock.svg \
    Icons/open.svg \
    Icons/palette.svg \
    Icons/saveas.svg \
    Icons/settings.svg \
    Icons/unlock.svg \
    README.md \
    UserGuide.md
