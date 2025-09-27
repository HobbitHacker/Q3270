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
        themes.insert(tname, store.getTheme(tname));
    }

    // Populate dropdowns
    ui->KeyboardFunctionList->addItem("Unassigned");
    ui->KeyboardFunctionList->addItems(Keyboard::allFunctionNames());
    ui->keyboardThemes->addItems(store.themeNames());

    // Initialize selection to Factory (or first available) and populate the table
    if (ui->keyboardThemes->count() > 0) {
        const QString init = themes.contains("Factory") ? "Factory" : themes.cbegin().key();
        ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(init));
        setTheme(init);
    }

    // Wire up signals
    connect(ui->themeNameEdit, &QLineEdit::textChanged, this, &KeyboardThemeDialog::validateThemeName);
    connect(ui->keyboardThemes, &QComboBox::currentTextChanged, this, &KeyboardThemeDialog::handleThemeChanged);
    connect(ui->newThemeButton, &QPushButton::clicked, this, &KeyboardThemeDialog::createNewTheme);

    connect(ui->KeyboardMap, &KeyboardMapWidget::mappingClicked, this, &KeyboardThemeDialog::populateKeySequence);

    connect(ui->buttonBox->button(QDialogButtonBox::Close),  &QPushButton::clicked, this, &QDialog::close);
    connect(ui->buttonBox->button(QDialogButtonBox::Save),  &QPushButton::clicked, this, &KeyboardThemeDialog::saveTheme);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KeyboardThemeDialog::applyTheme);
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &KeyboardThemeDialog::revertTheme);

    connect(ui->deleteThemeButton, &QPushButton::clicked, this, &KeyboardThemeDialog::removeTheme);

    // Key sequence edit finished editing
    connect(ui->keySequenceEdit, &QKeySequenceEdit::editingFinished, this, &KeyboardThemeDialog::truncateShortcut);

    // Set mapping button
    connect(ui->setKeyboardMap, &QPushButton::clicked, this, &KeyboardThemeDialog::setKey);

    // Save, Apply and Reset buttons disabled until an edit is made
    ui->buttonBox->button(QDialogButtonBox::Reset)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);

    dirty = false;
    unapplied = false;

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
 * @brief   KeyboardTheme::exec - display the dialog
 * @return  The button pressed to exit the dialog
 *
 * @details Save the state of the themes and then display the dialog.
 */
int KeyboardThemeDialog::exec()
{
    // Save the initial state, to be restored should the user press cancel
    restoreThemes = themes;

    updateUiState();

    return QDialog::exec();
}

/**
 * @brief   KeyboardTheme::setTheme - set the currently displayed theme
 * @param   newTheme - the theme name
 *
 * @details Set the theme displayed by the dialog. If the theme doesn't exist, show the Factory
 *          one instead.
 */
void KeyboardThemeDialog::setTheme(const QString &themeName)
{
    // If we don't know the name of the theme, fall back to Factory. This allows users to delete themes, but
    // still leave them referenced in session configurations.

    auto it = themes.find(themeName);

    if (it == themes.end())
        it = themes.find("Factory");

    currentTheme = &it.value();

    // Select it from the theme list
    ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(currentTheme->name));

    // Populate dialog table
    ui->KeyboardMap->setTheme(*currentTheme);

    // Populate theme name
    ui->themeNameEdit->setText(currentTheme->name);

    // Clear last row displayed
    lastRow = -1;
    lastSeq = -1;

    // Clear key sequence and message
    ui->keySequenceEdit->clear();
    ui->message->clear();
}

/**
 * @brief   KeyboardThemeDialog::handleThemeChanged - handle a change of Keyboard Theme from the combobox
 * @param   name - the selected theme name
 *
 * @details Signalled when the user selects a different theme from the combobox.
 */
void KeyboardThemeDialog::handleThemeChanged(const QString &name)
{
    setTheme(name);

    updateUiState();

    return;

    // Not sure how this would be invoked
    if (!store.themeNames().contains(name))
        return;

    const KeyboardMap map = store.getTheme(name);

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
void KeyboardThemeDialog::createNewTheme()
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

    ui->themeDescriptionEdit->clear();
    ui->themeNameEdit->setFocus();
    ui->themeNameEdit->setText(newName);

    KeyboardMap newMap;
    newMap.name = newName;
    newMap.mappings = currentTheme->mappings;

    themes.insert(newMap.name, newMap);

    ui->keyboardThemes->addItem(newName);
    ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->findText(newName));

    dirty = true;
    unapplied = true;

    updateUiState();
}

/**
 * @brief   KeyboardThemeDialog::validateThemeName - validate the modified theme name
 * @param   name - the new name
 *
 * @details Signalled when the user modifies the theme name in the input box. The Save and Apply
 *          buttons are disabled when a duplicate is detected, or the input is empty.
 */
void KeyboardThemeDialog::validateThemeName(const QString &name)
{
    bool duplicate = store.themeNames().contains(name, Qt::CaseSensitive);
    bool empty = name.trimmed().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!empty && !duplicate);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(!empty && !duplicate);

    // TODO: Implement visual feedback across other dialogs for error fields

    /*
    QPalette pal = ui->themeNameEdit->palette();
    pal.setColor(QPalette::Base, (empty || duplicate) ? QColor(255, 230, 230) : Qt::white);
    ui->themeNameEdit->setPalette(pal);
    */
}

/**
 * @brief   KeyboardThemeDialog::saveTheme - save the themes to disk
 *
 * @details Invoked when the user clicks 'Save' in the dialog box. All themes are written to disk.
 */
void KeyboardThemeDialog::saveTheme()
{
    const QString name = ui->themeNameEdit->text().trimmed();

    if (name.isEmpty())
        return; // Should be prevented by validation

    // Update the shared store in-memory and persist the change
    KeyboardMap map = ui->KeyboardMap->currentMappings();

    map.name = name;
    themes[name] = map;
    store.setTheme(name, map);
    store.saveAllThemes();

    // Refresh dropdown if new theme
    if (!(ui->keyboardThemes->findText(name, Qt::MatchExactly) >= 0))
        ui->keyboardThemes->addItem(name);

    ui->keyboardThemes->setCurrentText(name);

    if (unapplied)
        emit themesApplied(name);

    dirty = false;
    unapplied = false;

    updateUiState();
}

/**
 * @brief   KeyboardThemeDialog::applyTheme - apply the theme to the in-memory store.
 *
 * @details Called when the user clicks the 'Apply' button. Stores any modified theme in the in-memory store,
 *          and emits a signal in case the modified theme in the active one.
 */
void KeyboardThemeDialog::applyTheme()
{
    // Update the shared store in-memory without persisting to disk
    store.setThemes(themes);
    restoreThemes = themes;

    emit themesApplied(currentTheme->name);

    unapplied = false;

    updateUiState();
}

/**
 * @brief   KeyboardThemeDialog::revertTheme - revert to the previously applied theme
 *
 * @details Called when the user clicks the 'Revert' button. Reverts any modifications to the previously
 *          applied version of the theme.
 */
void KeyboardThemeDialog::revertTheme()
{
    themes = restoreThemes;
    restoreThemeName = currentTheme ? currentTheme->name : QStringLiteral("Factory");

    setTheme(restoreThemeName);

    unapplied = false;
    dirty = false;

    updateUiState();
}

/**
 * @brief   KeyboardThemeDialog::removeTheme - remove a theme from the in-memory store
 *
 * @details Called when the user clicks the 'Delete' button. The theme is removed from the in-memory store,
 *          and another theme displayed.
 */
void KeyboardThemeDialog::removeTheme()
{
    const QString toDelete = ui->keyboardThemes->currentText();
    const int toDeleteIx = ui->keyboardThemes->currentIndex();

    themes.remove(toDelete);
    store.removeTheme(toDelete);

    ui->keyboardThemes->removeItem(toDeleteIx);

    const int currentThemeIx = ui->keyboardThemes->currentIndex();

    if (currentThemeIx >=0)
    {
        setTheme(ui->keyboardThemes->currentText());
    }
    else
    {
        currentTheme = nullptr;
    }

    dirty = true;
    unapplied = false;

    updateUiState();
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
    if (currentTheme->name == "Factory")
        return;

    currentTheme->setKeyMapping(
        ui->KeyboardFunctionList->currentText(),
        ui->keySequenceEdit->keySequence()
        );

    ui->KeyboardMap->setTheme(*currentTheme);

    dirty = true;
    unapplied = true;

    updateUiState();
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

void KeyboardThemeDialog::updateUiState()
{
    // Dirty means there are unsaved modifications
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(unapplied);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(dirty);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(unapplied);

    // While dirty, prevent switching themes or creating new ones until changes are resolved
    ui->keyboardThemes->setDisabled(unapplied);
    ui->newThemeButton->setDisabled(unapplied);

    // Disable editing for Factory theme
    bool isFactory = (currentTheme->name == "Factory");

    ui->deleteThemeButton->setDisabled(isFactory);
    ui->themeNameEdit->setDisabled(isFactory);
    ui->themeDescriptionEdit->setDisabled(isFactory);
//    ui->KeyboardMap->setReadOnly(isFactory);

}
