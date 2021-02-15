QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QT_FLUSH_PAINT=1

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Buffer.cpp \
    DisplayScreen.cpp \
    FontSelection.cpp \
    Host.cpp \
    Keyboard.cpp \
    ProcessDataStream.cpp \
    Settings.cpp \
    TerminalTab.cpp \
    TerminalView.cpp \
    main.cpp \
    mainwindow.cpp \
    SocketConnection.cpp \
    text.cpp

HEADERS += \
    Buffer.h \
    DisplayScreen.h \
    3270.h \
    FontSelection.h \
    Host.h \
    Keyboard.h \
    ProcessDataStream.h \
    Settings.h \
    TerminalTab.h \
    TerminalView.h \
    mainwindow.h \
    SocketConnection.h \
    text.h

FORMS += \
    FontSelection.ui \
    Host.ui \
    Settings.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
