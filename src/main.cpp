/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Icons/q3270.svg"));
    MainWindow w;
    w.show();
    return a.exec();
}
