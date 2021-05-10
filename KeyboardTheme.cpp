#include "ui_KeyboardTheme.h"
#include "KeyboardTheme.h"

KeyboardTheme::KeyboardTheme() : QDialog(), ui(new Ui::KeyboardTheme)
{   
    ui->setupUi(this);

    // Set up factory map
    theme.clear();

    theme.insert("Enter",      { "Enter", "RCtrl" });
    theme.insert("Reset",      { "LCtrl" });
    theme.insert("Insert",     { "Insert" });
    theme.insert("Delete",     { "Delete" });
    theme.insert("Up",         { "Up" });
    theme.insert("Down",       { "Down" });
    theme.insert("Left",       { "Left" });
    theme.insert("Right",      { "Right" });

    theme.insert("Backspace",  { "Backspace" });

    theme.insert("Tab",        { "Tab" });
    theme.insert("Backtab",    { "Backtab", "Shift+Tab", "Shift+Backtab" });

    theme.insert("Home",       { "Home" });
    theme.insert("EraseEOF",   { "End" });
    theme.insert("NewLine",    { "Return" });
    theme.insert("EndLine",    { "Ctrl+End" });

    theme.insert("F1",         { "F1" });
    theme.insert("F2",         { "F2" });
    theme.insert("F3",         { "F3" });
    theme.insert("F4",         { "F4" });
    theme.insert("F5",         { "F5" });
    theme.insert("F6",         { "F6" });
    theme.insert("F7",         { "F7", "PgUp" });
    theme.insert("F8",         { "F8", "PgDown" });
    theme.insert("F9",         { "F9" });
    theme.insert("F10",        { "F10" });
    theme.insert("F11",        { "F11" });
    theme.insert("F12",        { "F12" });

    theme.insert("F13",        { "Shift+F1" });
    theme.insert("F14",        { "Shift+F2" });
    theme.insert("F15",        { "Shift+F3" });
    theme.insert("F16",        { "Shift+F4" });
    theme.insert("F17",        { "Shift+F5" });
    theme.insert("F18",        { "Shift+F6" });
    theme.insert("F19",        { "Shift+F7" } );
    theme.insert("F20",        { "Shift+F8" });
    theme.insert("F21",        { "Shift+F9" });
    theme.insert("F22",        { "Shift+F10" });
    theme.insert("F23",        { "Shift+F11" });
    theme.insert("F24",        { "Shift+F12" });

    theme.insert("PA1",        { "Alt+1" });
    theme.insert("PA2",        { "Alt+2" });
    theme.insert("PA3",        { "Alt+3" });

    theme.insert("Attn",       { "Escape" });

    theme.insert("ToggleRuler", { "Ctrl+Home" });

    theme.insert("Clear",       { "Pause" });

    theme.insert("Copy",        { "Ctrl+C" });
    theme.insert("Paste",       { "Ctrl+V" });

    // Add factory theme to list of themes
    themes.insert("Factory", theme);
}
