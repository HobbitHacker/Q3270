/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "FunctionRegistry.h"

static QList<FunctionInfo> build()
{
    return {
        { "Enter",        UsageContext::Keyboard, "Send the Enter key" },
        { "Reset",        UsageContext::Keyboard, "Reset the keyboard" },

        { "Up",           UsageContext::Keyboard, "Move cursor up" },
        { "Down",         UsageContext::Keyboard, "Move cursor down" },
        { "Left",         UsageContext::Keyboard, "Move cursor left" },
        { "Right",        UsageContext::Keyboard, "Move cursor right" },

        { "Backspace",    UsageContext::Keyboard, "Delete character to the left" },

        { "Tab",          UsageContext::Keyboard, "Move to next field" },
        { "Backtab",      UsageContext::Keyboard, "Move to previous field" },

        { "NewLine",      UsageContext::Keyboard, "Insert a new line" },
        { "Home",         UsageContext::Keyboard, "Move cursor to start of line" },
        { "EndLine",      UsageContext::Keyboard, "Move cursor to end of line" },

        { "EraseEOF",     UsageContext::Keyboard, "Erase to end of field" },

        { "Insert",       UsageContext::Keyboard, "Toggle insert mode" },
        { "Delete",       UsageContext::Keyboard, "Delete character at cursor" },

        { "F1",           UsageContext::Keyboard, "Function key 1" },
        { "F2",           UsageContext::Keyboard, "Function key 2" },
        { "F3",           UsageContext::Keyboard, "Function key 3" },
        { "F4",           UsageContext::Keyboard, "Function key 4" },
        { "F5",           UsageContext::Keyboard, "Function key 5" },
        { "F6",           UsageContext::Keyboard, "Function key 6" },
        { "F7",           UsageContext::Keyboard, "Function key 7" },
        { "F8",           UsageContext::Keyboard, "Function key 8" },
        { "F9",           UsageContext::Keyboard, "Function key 9" },
        { "F10",          UsageContext::Keyboard, "Function key 10" },
        { "F11",          UsageContext::Keyboard, "Function key 11" },
        { "F12",          UsageContext::Keyboard, "Function key 12" },

        { "F13",          UsageContext::Keyboard, "Function key 13" },
        { "F14",          UsageContext::Keyboard, "Function key 14" },
        { "F15",          UsageContext::Keyboard, "Function key 15" },
        { "F16",          UsageContext::Keyboard, "Function key 16" },
        { "F17",          UsageContext::Keyboard, "Function key 17" },
        { "F18",          UsageContext::Keyboard, "Function key 18" },
        { "F19",          UsageContext::Keyboard, "Function key 19" },
        { "F20",          UsageContext::Keyboard, "Function key 20" },
        { "F21",          UsageContext::Keyboard, "Function key 21" },
        { "F22",          UsageContext::Keyboard, "Function key 22" },
        { "F23",          UsageContext::Keyboard, "Function key 23" },
        { "F24",          UsageContext::Keyboard, "Function key 24" },

        { "Attn",         UsageContext::Keyboard, "Attention key" },

        { "PA1",          UsageContext::Keyboard, "Program Attention 1" },
        { "PA2",          UsageContext::Keyboard, "Program Attention 2" },
        { "PA3",          UsageContext::Keyboard, "Program Attention 3" },

        { "Clear",        UsageContext::Keyboard, "Clear the screen" },

        { "ToggleRuler",  UsageContext::Keyboard | UsageContext::Menu, "Toggle the ruler display" },

        { "Copy",         UsageContext::Keyboard | UsageContext::Menu | UsageContext::Toolbar, "Copy selection" },
        { "Paste",        UsageContext::Keyboard | UsageContext::Menu | UsageContext::Toolbar, "Paste from clipboard" },
        { "Info",         UsageContext::Keyboard | UsageContext::Menu, "Show information" },
        { "Fields",       UsageContext::Keyboard | UsageContext::Menu, "Show field list" },
        { "DumpScreen",   UsageContext::Keyboard | UsageContext::Menu, "Dump the screen"},

        { "Blah",         UsageContext::Keyboard, "Unlock the keyboard" }
    };
}

const QList<FunctionInfo> &FunctionRegistry::all()
{
    static const QList<FunctionInfo> funcs = build();
    return funcs;
}

QStringList FunctionRegistry::namesFor(UsageContext context)
{
    QStringList result;
    for (const FunctionInfo &f : all()) {
        if (f.contexts.testFlag(context))
            result.append(f.name);
    }
    return result;
}

QList<FunctionInfo> FunctionRegistry::functionsFor(UsageContext context)
{
    QList<FunctionInfo> result;
    for (const FunctionInfo &f : all()) {
        if (f.contexts.testFlag(context))
            result.append(f);
    }
    return result;
}
