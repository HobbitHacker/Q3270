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

#include "Q3270.h"
#include "ui_NewTheme.h"
#include "KeyboardThemeDialog.h"
#include "ui_KeyboardTheme.h"
#include "Keyboard.h"

/**
 * @brief   KeyboardTheme::KeyboardTheme - Keyboard Theme handling
 * @param   parent - parent widget
 *
 * @details KeyboardTheme handles the processing of Keyboard themes. The constructor
 *          creates the internal Factory keyboard map, along with any additional themes from the
 *          config file, and then populates the dialog box with the list of themes.
 */
KeyboardThemeDialog::KeyboardThemeDialog(KeyboardStore &storeRef, QWidget *parent)
    : QDialog(parent), store(storeRef), ui(new Ui::KeyboardTheme)
{
    ui->setupUi(this);

    // The shared store should already be loaded by the application startup.
    // Copy store themes into local in-memory themes so the dialog edits a working copy
    themes.clear();
    for (const QString &tname : store.themeNames()) {
        themes.insert(tname, store.theme(tname));
    }

    // Populate dropdowns
    ui->KeyboardFunctionList->addItem("Unassigned");
    ui->KeyboardFunctionList->addItems(Keyboard::allFunctionNames());
    ui->keyboardThemes->addItems(themes.keys());

    // Initialize selection to Factory (or first available) and populate the table
    if (ui->keyboardThemes->count() > 0) {
        const QString init = themes.contains("Factory") ? "Factory" : themes.cbegin().key();
        ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(init));
        setTheme(init);
    }

    // Wire up signals
    connect(ui->themeNameEdit, &QLineEdit::textChanged, this, &KeyboardThemeDialog::validateThemeName);
    connect(ui->keyboardThemes, &QComboBox::currentTextChanged, this, &KeyboardThemeDialog::loadTheme);
    connect(ui->newThemeButton, &QPushButton::clicked, this, &KeyboardThemeDialog::startNewTheme);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &KeyboardThemeDialog::saveTheme);

    // Explicitly wire Save/Apply/Close buttons
    QPushButton *saveBtn = ui->buttonBox->button(QDialogButtonBox::Save);
    if (saveBtn) connect(saveBtn, &QPushButton::clicked, this, &KeyboardThemeDialog::saveTheme);

    QPushButton *applyBtn = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyBtn) connect(applyBtn, &QPushButton::clicked, this, &KeyboardThemeDialog::apply);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(ui->deleteThemeButton, &QPushButton::clicked, this, &KeyboardThemeDialog::deleteTheme);

    // New Theme button
    connect(ui->newThemeButton, &QPushButton::clicked, this, &KeyboardThemeDialog::addTheme);

    // Key sequence edit finished editing
    connect(ui->keySequenceEdit, &QKeySequenceEdit::editingFinished, this, &KeyboardThemeDialog::truncateShortcut);

    // Set mapping button
    connect(ui->setKeyboardMap, &QPushButton::clicked, this, &KeyboardThemeDialog::setKey);

    // Hide Reset button
    ui->buttonBox->button(QDialogButtonBox::Reset)->hide();

    // Save and Apply buttons disabled until an edit is made
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);

    lastRow = -1;
    lastSeq = -1;
}
/**
 * @brief   KeyboardTheme::~KeyboardTheme - destructor
 * @return  Nothing
 *
 * @details Destructor to remove objects
 */
KeyboardThemeDialog::~KeyboardThemeDialog()
{
    delete ui;
}

/**
 * @brief   KeyboardTheme::setTheme - set the currently displayed theme
 * @param   newTheme - the theme name
 *
 * @details Set the theme displayed by the dialog. If the theme doesn't exist, show the Factory
 *          one instead.
 */
void KeyboardThemeDialog::setTheme(QString newTheme)
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
    ui->KeyboardMap->setTheme(themes[currentTheme]);

    // Populate theme name
    ui->themeNameEdit->setText(themes[currentTheme].name);

    // Clear last row displayed
    lastRow = -1;
    lastSeq = -1;

    // Clear key sequence and message
    ui->keySequenceEdit->clear();
    ui->message->clear();
}

/**
 * @brief   KeyboardTheme::themeChanged - the user selected a different theme
 * @param   index - the index of the theme selected
 *
 * @details This slot is signalled when the user selects a different theme in the drop-down list
 *          in the dialog.
 */
void KeyboardThemeDialog::themeChanged(int index)
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

/**
 * @brief   KeyboardTheme::addTheme - add a new theme
 *
 * @details Called when the user clicks the New Theme button. A dialog box is presented with a
 *          new theme name; there is a slot connected to this field to ensure it is unique and
 *          the OK button is only enabled when it is. The initially selected new theme name is
 *          "New Theme " concatenated to the first non-existent number (ie, "New Theme 1" or
 *          "New Theme 2" etc).
 *
 *          The new theme is copied from the currently displayed one.
 */
void KeyboardThemeDialog::addTheme()
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
}

/**
 * @brief   KeyboardTheme::exec - display the dialog
 * @return  The button pressed to exit the dialog
 *
 * @details Save the state of the themes and then display the dialog.
 */
int KeyboardThemeDialog::exec()
{
    // Save the initial state, to be restored should the user press cancel
    restoreThemeIndex = currentThemeIndex;
    restoreTheme = currentTheme;
    restoreThemes = themes;

    return QDialog::exec();
}

/**
 * @brief   KeyboardTheme::accept - OK button processing
 *
 * @details Called when the user presses the OK button to accept the changes they've made.
 *          The KeyboardThemes are all written to the config file.
 */
void KeyboardThemeDialog::accept()
{
    // Apply to shared store and persist
    store.setThemes(themes);
    store.saveAllThemes();

    emit themesApplied(currentTheme);

    QDialog::accept();
}

/**
 * @brief   KeyboardTheme::reject - Cancel button processing
 *
 * @details The user pressed the Cancel button. Restore the previously saved themes.
 */
void KeyboardThemeDialog::reject()
{
    // Restore initial state
    themes = restoreThemes;

    ui->keyboardThemes->clear();
    ui->keyboardThemes->addItems(themes.keys());

    setTheme(restoreTheme);

    QDialog::reject();
}

/**
 * @brief   KeyboardTheme::populateKeySequence - display the key sequence from the table
 * @param   item - the table
 *
 * @details When the keyboard mapping table is displayed, the user can click on a row
 *          to populate the mapping fields at the bottom of the dialog. If multiple mappings
 *          are in place (F8, PgDown both mapped to F8), this routine will cycle through them
 *          each time the row is clicked.
 */
void KeyboardThemeDialog::populateKeySequence(int row, const QString &functionName, const QStringList &keyList)
{
    //NOTE: This doesn't handle the custom left-ctrl/right-ctrl stuff

    // If the row clicked is a different row to last time, set the sequence to the first mapping
    if (row != lastRow)
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
    ui->KeyboardFunctionList->setCurrentIndex(ui->KeyboardFunctionList->findText(functionName));

    // If the key is mapped, display the key sequence, otherwise, blank it out
    if (!keyList.isEmpty())
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
    lastRow = row;
}

/**
 * @brief   KeyboardTheme::setKey - set the key mapping
 *
 * @details When the user clicks the 'Set Mapping' button, this routine populates the keyboard map
 *          with the desired settings. The map is searched for an existing mapping, and if present, it
 *          is removed (it's not possible to map the same key to two different functions). Finally, the
 *          key sequence is stored in the map, and the table is rebuilt.
 */
void KeyboardThemeDialog::setKey()
{
    theme.setKeyMapping(
        ui->KeyboardFunctionList->currentText(),
        ui->keySequenceEdit->keySequence()
        );

    themes[currentTheme] = theme; // or store.updateTheme(...)
    setTheme(currentTheme);

    ui->keyboardThemes->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Reset)->show();
}

/**
 * @brief   KeyboardTheme::truncateShortcut - remove any additional key sequences
 *
 * @details Qt allows multiple key sequences, but Q3270 is only interested in the first.
 *          Truncate the incoming sequence to just the first.
 */
void KeyboardThemeDialog::truncateShortcut()
{
    // Use only the first key sequence the user pressed
    int value = ui->keySequenceEdit->keySequence()[0];
    QKeySequence shortcut(value);
    ui->keySequenceEdit->setKeySequence(shortcut);
}


void KeyboardThemeDialog::validateThemeName(const QString &name)
{
    bool duplicate = store.themeNames().contains(name, Qt::CaseSensitive);
    bool empty = name.trimmed().isEmpty();

    // Example: disable save if invalid
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!empty && !duplicate);

    // Optional: visual feedback
    // TODO: Implement visual feedback across other dialogs for error fields

    /*
    QPalette pal = ui->themeNameEdit->palette();
    pal.setColor(QPalette::Base, (empty || duplicate) ? QColor(255, 230, 230) : Qt::white);
    ui->themeNameEdit->setPalette(pal);
    */
}

void KeyboardThemeDialog::loadTheme(const QString &name)
{
    if (!store.themeNames().contains(name))
        return;

    const KeyboardMap map = store.theme(name);

    ui->themeNameEdit->setText(name);
    ui->themeDescriptionEdit->clear(); // placeholder until descriptions are stored

    // Update combobox
    ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(name));

    // Build the keyboard map table
    ui->KeyboardMap->setTheme(map);

    // Reset selection state
    lastRow = -1;
    lastSeq = -1;
}

void KeyboardThemeDialog::startNewTheme()
{
    ui->themeNameEdit->clear();
    ui->themeDescriptionEdit->clear();

    ui->themeNameEdit->setFocus();
    validateThemeName(QString());
}

void KeyboardThemeDialog::saveTheme()
{
    const QString name = ui->themeNameEdit->text().trimmed();

    if (name.isEmpty())
        return; // Should be prevented by validation

    KeyboardMap map = ui->KeyboardMap->currentMappings();
    // Optionally set description in map if you add that field later

    // Update the shared store in-memory and persist the change
    store.setTheme(name, map);
    store.saveAllThemes();

    // Refresh dropdown if new theme
    if (!(ui->keyboardThemes->findText(name, Qt::MatchExactly) >= 0))
        ui->keyboardThemes->addItem(name);

    ui->keyboardThemes->setCurrentText(name);

    ui->keyboardThemes->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Reset)->hide();


}

void KeyboardThemeDialog::apply()
{
    // Update the shared store in-memory without persisting to disk
    store.setThemes(themes);
    emit themesApplied(currentTheme);
}

void KeyboardThemeDialog::deleteTheme()
{
    const QString name = ui->keyboardThemes->currentText();

    if (name == "Factory")
        return; // Don't delete factory theme

    store.removeTheme(name);

    int index = ui->keyboardThemes->findText(name);
    if (index >= 0)
        ui->keyboardThemes->removeItem(index);

    // Load factory theme after deletion
    ui->keyboardThemes->setCurrentText("Factory");
}
