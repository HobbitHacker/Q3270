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
#include "ColourTheme.h"
#include "ui_ColourTheme.h"
#include "ui_NewTheme.h"

/**
 * @brief   ColourTheme::ColourTheme - Dialog for choosing the colours for 3270
 * @param   parent
 *
 * @details ColourTheme is used to display the swatches for the different colours used by
 *          the 3270 display.
 *
 *          There are two sets of colours; the basic 4 colour (blue, green, red, white), and
 *          the standard 7 colour (black, blue, red, magenta or pink, green, cyan or turquoise,
 *          yellow and white or neutral).
 *
 *          The theme names are picked up from the config file; Factory is always present
 *          internally, and ignored if also found in the config file. These are used to build up
 *          a list of available themes.
 */
ColourTheme::ColourTheme(ColourStore &store, QWidget *parent) :
    QDialog(parent),
    store(store),
    ui(new Ui::ColourTheme)
{
    ui->setupUi(this);
    ui->colourTheme->clear();

    for (const QString &tn : store.themeNames())
        themes.insert(tn, store.getTheme(tn));

    // Get Themes, and add them to the combobox
    ui->colourTheme->addItems(store.themeNames());
    ui->colourTheme->setCurrentIndex(0);

    setTheme("Factory");

    // Wire up signals
    connect(ui->themeName,   &QLineEdit::textChanged,        this, &ColourTheme::checkThemeName);
    connect(ui->colourTheme, &QComboBox::currentTextChanged, this, &ColourTheme::handleThemeChanged);
    connect(ui->colourNew,   &QPushButton::clicked,          this, &ColourTheme::createNewTheme);

    connect(ui->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &QDialog::close);
    connect(ui->buttonBox->button(QDialogButtonBox::Save),  &QPushButton::clicked, this, &ColourTheme::saveTheme);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ColourTheme::applyTheme);
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ColourTheme::revertTheme);

    connect(ui->deleteThemeButton, &QPushButton::clicked, this, &ColourTheme::removeTheme);

    connect(ui->colourSwatchWidget, &ColourSwatchWidget::colourChanged, this, &ColourTheme::handleColourModified);

    ui->buttonBox->button(QDialogButtonBox::Reset)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);

    dirty = false;
    unapplied = false;
}

/**
 * @brief   ColourTheme::~ColourTheme - destructor
 *
 * @details Destructor.
 */
ColourTheme::~ColourTheme()
{
    delete ui;
}

/**
 * @brief   ColourTheme::exec - dialog display.
 * @return  OK or Cancel
 */
int ColourTheme::exec()
{
    // Save state in the event of a cancel
    restoreThemes = themes;
    restoreThemeName = "Factory";

    setTheme("Factory");
    updateUiState();

    return QDialog::exec();
}

/**
 * @brief   ColourTheme::setTheme - set colour theme
 * @param   ThemeName - string name of theme to be set
 *
 * @details setTheme changes the theme to the one specified, changing the colours on the
 *          swatches to match. If the passed theme name isn't found in the list, the Factory
 *          internal one is used instead.
 */
void ColourTheme::setTheme(const QString &ThemeName)
{
    auto it = themes.find(ThemeName);

    if (it == themes.end())
        it = themes.find("Factory");

    currentTheme = &it.value();

    ui->colourSwatchWidget->setTheme(*currentTheme);

    // Only block signals for the block in braces
    {
        QSignalBlocker blocker(ui->colourTheme);
        ui->colourTheme->setCurrentIndex(ui->colourTheme->findText(currentTheme->name));
    }

    ui->themeName->setText(it.key());
}

/**
 * @brief   ColourTheme::themeChanged - update the colours when the theme is changed
 * @param   index - the index of the selected theme
 *
 * @details themeChanged is triggered when the drop-down box of theme names is updated to show
 *          a different theme. If the theme chosen is not the internal Factory theme, the theme
 *          can be deleted.
 */
void ColourTheme::handleThemeChanged(const QString &name)
{
    // When the combobox is changed, update the palette colours
    setTheme(name);

    updateUiState();
}

void ColourTheme::handleColourModified(Q3270::Colour role, QColor newColour)
{
    if (!currentTheme)
        return;

    currentTheme->setColour(role, newColour);

    unapplied = true;
    dirty = true;

    updateUiState();
}

/**
 * @brief   ColourTheme::addTheme - a pop-up dialog box to create a new colour theme
 *
 * @details addTheme displays a dialog box to the user allowing them to enter a new name for
 *          a new theme. The default name of the new theme is 'New Theme' and a number, increasing by
 *          one for each "New Theme n" that is already present.
 *
 *          When the OK button is pressed, the new theme is added to the list of selectable
 *          themes.
 */
void ColourTheme::createNewTheme()
{
    QString newName = "New Theme";

    // Create unique name for new theme
    if (store.exists(newName))
    {
        int i = 1;
        while(store.exists(newName + " " + QString::number(i)))
        {
            i++;
        }
        newName = "New Theme " + QString::number(i);
    }

    ui->themeDescription->clear();
    ui->themeName->setText(newName);
    ui->themeName->setFocus();

    Colours newTheme;
    newTheme.name = newName;
    newTheme.map = currentTheme->map;

    themes.insert(newTheme.name, newTheme);

    ui->colourTheme->addItem(newName);
    ui->colourTheme->setCurrentIndex(ui->colourTheme->findText(newName));

    dirty = true;
    unapplied = false;

    updateUiState();
}

/**
 * @brief   ColourTheme::checkDuplicate - check for duplicate theme names when adding a new theme
 *
 * @details checkDuplicate is triggered when the user modifies the input box for the new theme name.
 *          When the theme name does not match any existing theme name, the OK button is enabled,
 *          otherwise a "Duplicate theme name" message is shown.
 */
void ColourTheme::checkThemeName(const QString &name)
{
    // Check if new theme name being entered is a unique value
    bool duplicate = store.exists(name);
    bool empty = name.trimmed().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!empty && !duplicate);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(!empty && !duplicate);
    //    newTheme->message->setText(unique ? QString() : QStringLiteral("Duplicate theme name"));
}

void ColourTheme::saveTheme()
{
    const QString name = ui->themeName->text().trimmed();

    if (name.isEmpty())
        return; // Should be prevented by validation

    // Update the shared store in-memory and persist the change
    Colours thisTheme = ui->colourSwatchWidget->currentTheme();

    thisTheme.name = name;
    themes[name] = thisTheme;

    store.setTheme(name, thisTheme);
    store.saveAllThemes();

    // Refresh dropdown if new theme
    if (!(ui->colourTheme->findText(name, Qt::MatchExactly) >= 0))
        ui->colourTheme->addItem(name);

    ui->colourTheme->setCurrentText(name);

    if (unapplied)
        emit themesApplied(thisTheme.name);

    dirty = false;
    unapplied = false;

    updateUiState();
}

void ColourTheme::applyTheme()
{
    // Update the shared store in-memory without persisting to disk

    store.setThemes(themes);
    restoreThemes = themes;

    emit themesApplied(currentTheme->name);

    unapplied = false;

    updateUiState();
}

void ColourTheme::revertTheme()
{
    themes = restoreThemes;
    restoreThemeName = currentTheme ? currentTheme->name : QStringLiteral("Factory");

    setTheme(restoreThemeName);

    unapplied = false;
    dirty = false;

    updateUiState();
}

/**
 * @brief   ColourTheme::deleteTheme - remove a theme
 *
 * @details deleteTheme removes an entry from the theme lists. It is triggered by the delete button.
 */
void ColourTheme::removeTheme()
{
    // Remove theme from lists
    const QString toDelete = ui->colourTheme->currentText();
    const int toDeleteIx = ui->colourTheme->currentIndex();

    if (toDelete.isEmpty())
        return;

    themes.remove(toDelete);
    store.removeTheme(toDelete);

    ui->colourTheme->removeItem(toDeleteIx);

    const int currentThemeIx = ui->colourTheme->currentIndex();

    if (currentThemeIx >= 0)
    {
        setTheme(ui->colourTheme->currentText());
    }
    else
    {
        currentTheme = nullptr;
    }

    dirty = true;
    unapplied = false;

    updateUiState();
}

void ColourTheme::updateUiState()
{
    // Dirty means there are unsaved modifications
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(unapplied);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(dirty);
    ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(unapplied);

    // While dirty, prevent switching themes or creating new ones until changes are resolved
    ui->colourTheme->setDisabled(unapplied);
    ui->colourNew->setDisabled(unapplied);

    // Disable editing for Factory theme
    bool isFactory = (currentTheme->name == "Factory");

    ui->deleteThemeButton->setDisabled(isFactory);
    ui->themeName->setDisabled(isFactory);
    ui->themeDescription->setDisabled(isFactory);
    ui->colourSwatchWidget->setReadOnly(isFactory);

}
