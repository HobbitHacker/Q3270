QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

VERSION = 0.1.1

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           Q3270_VERSION=\"\\\"$${VERSION}\\\"\"

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ColourTheme.cpp \
    DisplayScreen.cpp \
    Glyph.cpp \
    Host.cpp \
    Keyboard.cpp \
    KeyboardTheme.cpp \
    MainWindow.cpp \
    ProcessDataStream.cpp \
    SessionManagement.cpp \
    Settings.cpp \
    TerminalTab.cpp \
    TerminalView.cpp \
    main.cpp \
    SocketConnection.cpp

HEADERS += \
    ColourTheme.h \
    DisplayScreen.h \
    Glyph.h \
    Host.h \
    Keyboard.h \
    KeyboardTheme.h \
    MainWindow.h \
    ProcessDataStream.h \
    Q3270.h \
    SessionManagement.h \
    Settings.h \
    TerminalTab.h \
    TerminalView.h \
    SocketConnection.h

FORMS += \
    About.ui \
    ColourTheme.ui \
    Host.ui \
    KeyboardTheme.ui \
    MainWindow.ui \
    NewTheme.ui \
    OpenSession.ui \
    SaveSession.ui \
    Settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Q3270.qrc
