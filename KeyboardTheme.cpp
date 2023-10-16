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

#include "ui_NewTheme.h"
#include "KeyboardTheme.h"
#include "ui_KeyboardTheme.h"

KeyboardTheme::KeyboardTheme(QWidget *parent) : QDialog(parent), ui(new Ui::KeyboardTheme)
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

    // Store all functions: this is the definitive list of Q3270 functions
    functionList = theme.keys();

    // Ensure that the "Unassigned" function is available to map keys
    ui->KeyboardFunctionList->addItem("Unassigned");

    // Populate function list dropdown with available Q3270 functions
    ui->KeyboardFunctionList->addItems(functionList);

    // Populate theme drop-down list with Factory keyboard layout
    ui->keyboardThemes->addItem("Factory");

    // Now add those from the config file
    QSettings s;

    // Keyboard themes are all stored under the KeyboardThemes group
    s.beginGroup("KeyboardThemes");

    // Get a list of sub-groups
    QStringList themeList = s.childGroups();

    themeList.sort(Qt::CaseSensitive);

    // Populate themes list and combo boxes from config file
    for (int sc = 0; sc < themeList.count(); sc++)
    {
        qDebug() << themeList.at(sc);

        // Ignore Factory theme (shouldn't be present, but in case of accidents, or user fudging)
        if (themeList.at(sc).compare("Factory"))
        {
            qDebug() << "Storing " << themeList.at(sc);
            // Begin theme specific group
            s.beginGroup(themeList.at(sc));

            // All keyboard mappings for this theme
            QStringList keys = s.childKeys();

            // Clear existing mappings from temporary map
            theme.clear();

            for (int kb = 0; kb < keys.count(); kb++)
            {
                // Keyboard maps stored as Shift+F1=F1; extract Q3270 function into thisKey
                QString thisKey = s.value(keys.at(kb)).toString();

                // Ensure it's a known function before we store it
                if (functionList.contains(thisKey))
                {

                    // Append keyboard mapping to Q3270 function; if it doesn't exist, QMap creates one first
                    theme[thisKey].append(keys.at(kb));
                }
            }

            // Save theme, assuming it had valid mappings
            if (theme.count() > 0)
            {
                themes.insert(themeList.at(sc), theme);
                ui->keyboardThemes->addItem(themeList.at(sc));
            }

            // End theme specific group
            s.endGroup();
        }

    }

    // End themes main group
    s.endGroup();

    // Set default theme
    setTheme("Factory");

    // Initially sort by Q3270 function
    ui->KeyboardMap->sortByColumn(0, Qt::AscendingOrder);

    // Popup dialog for new themes
    newTheme = new Ui::NewTheme();

    newTheme->setupUi(&newThemePopUp);

    // Map the controls we're interested in
    connect(newTheme->newName, &QLineEdit::textChanged, this, &KeyboardTheme::checkDuplicate);
    connect(newTheme->buttonBox, &QDialogButtonBox::accepted, &newThemePopUp, &QDialog::accept);
    connect(newTheme->buttonBox, &QDialogButtonBox::rejected, &newThemePopUp, &QDialog::reject);

    // No "last chosen" keyboard row or key sequence; used to determine whether the next sequence
    // is shown for multiply-mapped functions
    lastRow = -1;
    lastSeq = -1;
}

QStringList KeyboardTheme::getThemes()
{
    // Return a list of themes
    return themes.keys();
}

KeyboardTheme::KeyboardMap KeyboardTheme::getTheme(QString keyboardThemeName)
{
    // Return theme, if it exists in the list, else return the Factory theme
    if (themes.contains(keyboardThemeName))
    {
        return themes[keyboardThemeName];
    }
    else
    {
        return themes["Factory"];
    }
}

void KeyboardTheme::setTheme(QString newTheme)
{
    // If we don't know the name of the theme, fall back to Factory. This allows users to delete themes, but
    // still leave them referenced in session configurations.
    if (!themes.contains(newTheme))
    {
        currentTheme = "Factory";
    }
    else
    {
        currentTheme = newTheme;
    }

    // Select it from the theme list
    ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(currentTheme));

    // Populate dialog table
    populateTable(ui->KeyboardMap, currentTheme);

    // Clear last row displayed
    lastRow = -1;
    lastSeq = -1;

    // Clear key sequence and message
    ui->keySequenceEdit->clear();
    ui->message->clear();
}

void KeyboardTheme::populateTable(QTableWidget *table, QString mapName)
{
    // Clear keyboard map table in dialog
    table->setRowCount(0);

    int row = 0;

    KeyboardTheme::KeyboardMap map = getTheme(mapName);
    KeyboardTheme::KeyboardMap::ConstIterator i = map.constBegin();

    // Iterate over keyboard map and insert into table
    while(i != map.constEnd())
    {
        // Insert new row into table widget, and add details
        table->insertRow(row);
        qDebug() << i.value();
        table->setItem(row, 0, new QTableWidgetItem(i.key()));
        table->setItem(row, 1, new QTableWidgetItem(i.value().join(", ")));
        i++;
        row++;
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void KeyboardTheme::themeChanged(int index)
{

    // Save new index
    currentThemeIndex = index;

    // Save the new name
    currentTheme = ui->keyboardThemes->itemText(index);

    // Update the keyboard map when the combobox changes
    setTheme(currentTheme);

    // Disable delete, set, keysequence and function dropdown for Factory theme
    if (currentThemeIndex == 0)
    {
        ui->deleteThemeButton->setDisabled(true);
        ui->KeyboardFunctionList->setDisabled(true);
        ui->keySequenceEdit->setDisabled(true);
        ui->setKeyboardMap->setDisabled(true);
    }
    else
    {
        ui->deleteThemeButton->setEnabled(true);
        ui->KeyboardFunctionList->setEnabled(true);
        ui->keySequenceEdit->setEnabled(true);
        ui->setKeyboardMap->setEnabled(true);
    }
}

void KeyboardTheme::addTheme()
{

    QString newName = "New Theme";

    // Create unique name for new theme
    if (themes.find(newName) != themes.end())
    {
        int i = 1;
        while(themes.find(newName + " " + QString::number(i)) != themes.end())
        {
            i++;
        }
        newName = "New Theme " + QString::number(i);
    }

    // Populate dialog box name
    newTheme->newName->setText(newName);

    // Allow user to modify name, disallowing existing names
    if(newThemePopUp.exec() == QDialog::Accepted)
    {
        // Save theme name
        newName = newTheme->newName->text();
        theme = themes[currentTheme];
        themes.insert(newName, theme);
        setTheme(newName);

        qDebug() << "Added " << newName << " Total items: " << themes.count();

        ui->keyboardThemes->addItem(newName);
        ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->count() - 1);
    }
}


void KeyboardTheme::checkDuplicate()
{
    // Check if new theme name being entered is a unique value
    if (themes.find(newTheme->newName->text()) == themes.end())
    {
        newTheme->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        newTheme->message->setText("");
    }
    else
    {
        newTheme->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        newTheme->message->setText("Duplicate theme name");
    }

}

void KeyboardTheme::deleteTheme()
{
    // Remove theme from lists
    themes.remove(ui->keyboardThemes->currentText());
    ui->keyboardThemes->removeItem(currentThemeIndex);
}

int KeyboardTheme::exec()
{
    // Save the initial state, to be restored should the user press cancel
    restoreThemeIndex = currentThemeIndex;
    restoreTheme = currentTheme;
    restoreThemes = themes;

    return QDialog::exec();
}

void KeyboardTheme::accept()
{
    // Save settings
    QSettings settings;

    // Group for Colours
    settings.beginGroup("KeyboardThemes");

    // Clear any existing settings
    settings.remove("");

    QMap<QString, KeyboardMap>::const_iterator i = themes.constBegin();

    while(i != themes.constEnd())
    {
        // Skip Factory theme
        if (i.key().compare("Factory"))
        {
            // Start new group for each theme
            settings.beginGroup(i.key());

            // Convenience variable
            theme = i.value();

            // Pull out each key mapping
            KeyboardMap::const_iterator j = theme.constBegin();

            while(j != theme.constEnd())
            {
                // Pull out each key sequence
                for(int k = 0; k < j.value().size(); k++)
                {
                    settings.setValue(j.value().at(k), j.key());
                }

                j++;
            }

            // End Theme group
            settings.endGroup();
        }

        // Next theme
        i++;

    }

    // Finish ColourThemes group
    settings.endGroup();

    QDialog::accept();

}

void KeyboardTheme::reject()
{
    // Restore initial state
    themes = restoreThemes;

    ui->keyboardThemes->clear();
    ui->keyboardThemes->addItems(themes.keys());

    setTheme(restoreTheme);

    QDialog::reject();
}

void KeyboardTheme::populateKeySequence(QTableWidgetItem *item)
{
    //NOTE: This doesn't handle the custom left-ctrl/right-ctrl stuff

    // Get current row number
    int thisRow = item->row();

    // Split the sequence by comma, to be able to display a rotating list of mappings
    QStringList keyList = ui->KeyboardMap->item(thisRow, 1)->text().split(", ");

    // If the row clicked is a different row to last time, set the sequence to the first mapping
    if (thisRow != lastRow)
    {
        lastSeq = 0;
    }
    else
    {
        // Otherwise increase sequence to the next mapping, limited to the size of the list
        if (++lastSeq >= keyList.size())
            lastSeq = 0;
    }

    // Set the function list field on the form to show the function defined by the row
    ui->KeyboardFunctionList->setCurrentIndex(ui->KeyboardFunctionList->findText(ui->KeyboardMap->item(thisRow, 0)->text()));

    // If the key is mapped, display the key sequence, otherwise, blank it out
    if (keyList.size() != 0)
    {
        ui->keySequenceEdit->setKeySequence(QKeySequence(keyList[lastSeq]));
    }
    else
    {
        ui->keySequenceEdit->clear();
    }

    // If there is only one mapping for this function, clear the message, otheriwse tell the user
    // there's more
    if (keyList.size() == 1)
    {
        ui->message->clear();
    }
    else
    {
        ui->message->setText("Click again for next key mapping");
    }

    // Set the last selected row to this one
    lastRow = thisRow;

}

void KeyboardTheme::setKey()
{
    // Search through the theme to find out if the key is already mapped
    QMap<QString, QStringList>::iterator i = theme.begin();

    while(i != theme.end())
    {
        // Loop through the mapped keys
        for (int s = 0; s < i.value().size(); s++)
        {
            // If the mapping is found, remove it because we can only have one key sequence mapped to a function
            if (QKeySequence(i.value()[s]) == ui->keySequenceEdit->keySequence())
            {
                printf("Found %s as %s - removing\n", i.value()[s].toLatin1().data(), i.key().toLatin1().data());
                fflush(stdout);
                // Remove existing entry
                i.value().removeAt(s);
            }
        }

        // If the current key is the matching function, process it
        if (!i.key().compare(ui->KeyboardFunctionList->currentText()))
        {
             // If the selected function was not 'Unassigned' - index 0 - then store the mapping
             if (ui->KeyboardFunctionList->currentIndex() != 0)
             {
                 printf("Adding to %s\n", i.key().toLatin1().data());
                 fflush(stdout);
                 theme[i.key()].append(ui->keySequenceEdit->keySequence().toString());
                 qDebug() << i.key() << i.value();
             }
        }
        i++;
    }

    // Update themes
    themes[currentTheme] = theme;

    // Rebuild the table display (probably inefficient)
    setTheme(currentTheme);
}

void KeyboardTheme::truncateShortcut()
{
    // Use only the first key sequence the user pressed
    int value = ui->keySequenceEdit->keySequence()[0];
    QKeySequence shortcut(value);
    ui->keySequenceEdit->setKeySequence(shortcut);
}

