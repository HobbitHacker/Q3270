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

#include "ui_PreferencesDialog.h"
#include "PreferencesDialog.h"
#include <QDebug>
#include "PreferencesDialog.h"

#include "Q3270.h"

/**
 * @brief   PreferencesDialog::PreferencesDialog - The preferences dialog
 * @param   colours         - the shared ColourTheme object
 * @param   keyboards       - the shared KeyboardTheme object
 * @param   activeSettings  - the shared currently active settings object
 * @param   parent          - the parent window
 *
 * @details Initialise the Preferences dialog.
 */
PreferencesDialog::PreferencesDialog(ColourTheme &colours, KeyboardTheme &keyboards, CodePage &codepages, ActiveSettings &activeSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    colours(colours),
    keyboards(keyboards),
    codepages(codepages),
    activeSettings(activeSettings)
{
    ui->setupUi(this);
    ui->TabsWidget->setCurrentIndex(0);

    // Set up vector to coordinate combox box for crosshair types
    comboRulerStyle.insert("Crosshairs", Q3270_RULER_CROSSHAIR);
    comboRulerStyle.insert("Vertical", Q3270_RULER_VERTICAL);
    comboRulerStyle.insert("Horizontal", Q3270_RULER_HORIZONTAL);

    // Populate combo box from vector keys
    ui->crosshair->addItems(comboRulerStyle.keys());

    // Set up a list of buttons for use in setButtonColours
    colourButtons[ColourTheme::UNPROTECTED_NORMAL]      = ui->baseUnprotected;
    colourButtons[ColourTheme::PROTECTED_NORMAL]        = ui->baseProtected;
    colourButtons[ColourTheme::UNPROTECTED_INTENSIFIED] = ui->baseUnprotectedIntensify;
    colourButtons[ColourTheme::PROTECTED_INTENSIFIED]   = ui->baseProtectedIntensify;

    colourButtons[ColourTheme::BLACK]        = ui->colourBlack;
    colourButtons[ColourTheme::BLUE]         = ui->colourBlue;
    colourButtons[ColourTheme::RED]          = ui->colourRed;
    colourButtons[ColourTheme::MAGENTA]      = ui->colourPink;
    colourButtons[ColourTheme::GREEN]        = ui->colourGreen;
    colourButtons[ColourTheme::CYAN]         = ui->colourTurq;
    colourButtons[ColourTheme::YELLOW]       = ui->colourYellow;
    colourButtons[ColourTheme::NEUTRAL]      = ui->colourWhite;

    // Populate code page list
    ui->CodePages->addItems(codepages.getCodePageList());

    // TODO: "Enter" when displaying font selection causes font dialog to vanish from widget

    // Build a QFontDialog for use within our Settings dialog
    qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);
    qfd->setOption(QFontDialog::DontUseNativeDialog);

//    connect(qfd, &QFontDialog::currentFontChanged, this, &Settings::fontChanged);

    ui->verticalLayout_5->addWidget(qfd);
}

/**
 * @brief   PreferencesDialog::~PreferencesDialog - destructor
 *
 * @details Delete objects acquired by 'new'.
 */
PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

/**
 * @brief   PreferencesDialog::connected - disable some settings when connected to a host
 *
 * @details When connected to a host, there are some settngs that cannot be changed. This is because
 *          these settings are only of use during connection negotiation and are not used unless the
 *          connection is closed and reopened.
 */
void PreferencesDialog::connected()
{
    ui->terminalCols->setDisabled(true);
    ui->terminalRows->setDisabled(true);
    ui->terminalType->setDisabled(true);

    ui->hostName->setDisabled(true);
    ui->hostPort->setDisabled(true);
    ui->hostLU->setDisabled(true);
}

/**
 * @brief   PreferencesDialog::disconnected - enable some settings when not connected to a host
 *
 * @details When connected to a host, there are some settngs that cannot be changed. This is because
 *          these settings are only of use during connection negotiation and are not used unless the
 *          connection is closed and reopened.
 */
void PreferencesDialog::disconnected()
{
    ui->terminalType->setEnabled(true);

    // Rows and columns only editable if it's dynamic size
    if (activeSettings.getTerminalModel() == Q3270_TERMINAL_DYNAMIC)
    {
        ui->terminalCols->setEnabled(true);
        ui->terminalRows->setEnabled(true);
    }
    else
    {
        ui->terminalCols->setEnabled(false);
        ui->terminalRows->setEnabled(false);
    }

    ui->hostName->setEnabled(true);
    ui->hostPort->setEnabled(true);
    ui->hostLU->setEnabled(true);
}

/**
 * @brief   PreferencesDialog::showForm - display the Preferences dialog
 *
 * @details Populate the various dialog fields from the active settings and show the form. This
 *          saves the coding required to save/restore settings before the form is displayed and to
 *          restore those settings if the user pressed Cancel.
 */
void PreferencesDialog::showForm()
{
    ui->hostLU->setText(activeSettings.getHostLU());
    ui->hostName->setText(activeSettings.getHostName());
    ui->hostPort->setText(QString::number(activeSettings.getHostPort()));

    // Terminal model/rows/cols
    ui->terminalType->setCurrentIndex(activeSettings.getTerminalModel());
    ui->terminalCols->setValue(activeSettings.getTerminalX());
    ui->terminalRows->setValue(activeSettings.getTerminalY());

    // Cursor blink enabled & speed
    ui->cursorBlink->setChecked(activeSettings.getCursorBlink());
    ui->cursorBlinkSpeed->setSliderPosition(activeSettings.getCursorBlinkSpeed());

    // Enable blink speed adjustment only if blink is enabled
    ui->cursorBlinkSpeed->setEnabled(ui->cursorBlink->QAbstractButton::isChecked());

    ui->cursorColour->setChecked(activeSettings.getCursorColourInherit());
    ui->stretch->setChecked(activeSettings.getStretchScreen());

    ui->backspaceStop->setChecked(activeSettings.getBackspaceStop());

    qfd->setCurrentFont(activeSettings.getFont());

    populateColourThemeNames();
    populateKeyboardThemeNames();

    // Select the right themes in the comboboxes
    ui->colourTheme->setCurrentIndex(ui->colourTheme->findText(activeSettings.getColourThemeName()));
    ui->keyboardTheme->setCurrentIndex(ui->keyboardTheme->findText(activeSettings.getKeyboardThemeName()));

    // Colour the buttons, based on Settings
    colours.setButtonColours(activeSettings.getColourThemeName());

    ui->CodePages->setCurrentIndex(ui->CodePages->findText(activeSettings.getCodePage()));

    this->exec();
}

/**
 * @brief   PreferencesDialog::setButtonColours - set the swatches to the colours specified
 * @param   themeName - the name of theme
 *
 * @details setButtonColours is used to change the colours on the swatches of the dialog. This is a
 *          duplicate of the code in ColourTheme.
 */
void PreferencesDialog::setButtonColours(QString themeName)
{
    ColourTheme::Colours thisTheme = colours.getTheme(themeName);

    // Change colour swatches
    colourButtons[ColourTheme::UNPROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::UNPROTECTED_NORMAL].name()));
    colourButtons[ColourTheme::PROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::PROTECTED_NORMAL].name()));
    colourButtons[ColourTheme::UNPROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::UNPROTECTED_INTENSIFIED].name()));
    colourButtons[ColourTheme::PROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::PROTECTED_INTENSIFIED].name()));

    colourButtons[ColourTheme::BLACK]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::BLACK].name()));
    colourButtons[ColourTheme::BLUE]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::BLUE].name()));
    colourButtons[ColourTheme::RED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::RED].name()));
    colourButtons[ColourTheme::MAGENTA]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::MAGENTA].name()));
    colourButtons[ColourTheme::GREEN]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::GREEN].name()));
    colourButtons[ColourTheme::CYAN]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::CYAN].name()));
    colourButtons[ColourTheme::YELLOW]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::YELLOW].name()));
    colourButtons[ColourTheme::NEUTRAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[ColourTheme::NEUTRAL].name()));
}

/**
 * @brief   PreferencesDialog::terminalModelDropDownChanged - signalled when the terminal model is changed
 * @param   model - the model number
 *
 * @details When the user chooses a different terminal model, the size of the terminal needs to be
 *          updated. If the selected model is the Dynamic one, enable editing of the rows and columns,
 *          otherwise, force them to the standard size of the model.
 */
void PreferencesDialog::terminalModelDropDownChanged(int model)
{
    // If the drop-down isn't enabled, this has been triggered by opening a session and
    // connecting. If we're connected, we can't edit the terminal type.
    if (ui->terminalType->isEnabled())
    {
        switch(model)
        {
            case Q3270_TERMINAL_MODEL2:
                ui->terminalCols->setDisabled(true);
                ui->terminalRows->setDisabled(true);
                ui->terminalCols->setValue(80);
                ui->terminalRows->setValue(24);
                break;
            case Q3270_TERMINAL_MODEL3:
                ui->terminalCols->setDisabled(true);
                ui->terminalRows->setDisabled(true);
                ui->terminalCols->setValue(80);
                ui->terminalRows->setValue(32);
                break;
            case Q3270_TERMINAL_MODEL4:
                ui->terminalCols->setDisabled(true);
                ui->terminalRows->setDisabled(true);
                ui->terminalCols->setValue(80);
                ui->terminalRows->setValue(43);
                break;
            case Q3270_TERMINAL_MODEL5:
                ui->terminalCols->setDisabled(true);
                ui->terminalRows->setDisabled(true);
                ui->terminalCols->setValue(132);
                ui->terminalRows->setValue(27);
                break;
            case Q3270_TERMINAL_DYNAMIC:
                ui->terminalCols->setEnabled(true);
                ui->terminalRows->setEnabled(true);
                break;
        }
    }
}

/**
 * @brief   PreferencesDialog::changeFont - signalled when the user has picked a new font
 * @param   newFont - the new font
 *
 * @details When the Font tab is displayed, and the user has chosen a new font, it would be possible
 *          to dynamically display the changes of font in the terminal as they happen.
 *
 * @note    This feature is not currently enabled.
 */
void PreferencesDialog::changeFont(QFont newFont)
{
    emit tempFontChange(newFont);
}

/**
 * @brief   PreferencesDialog::accept - process the OK button
 *
 * @details Update the active settings when the OK button is pressed.
 */
void PreferencesDialog::accept()
{
    activeSettings.setTerminal(ui->terminalCols->value(), ui->terminalRows->value(), ui->terminalType->currentIndex());
    activeSettings.setCursorBlink(ui->cursorBlink->QAbstractButton::isChecked());
    activeSettings.setCursorBlinkSpeed(ui->cursorBlinkSpeed->value());
    activeSettings.setRulerState(ui->rulerOn->QAbstractButton::isChecked());
    activeSettings.setRulerStyle(comboRulerStyle.value(ui->crosshair->currentText()));
    activeSettings.setCursorColourInherit(ui->cursorColour->QAbstractButton::isChecked());
    activeSettings.setFont(qfd->currentFont());
    activeSettings.setCodePage(ui->CodePages->currentText());
    activeSettings.setKeyboardTheme(ui->keyboardTheme->currentText());
    activeSettings.setColourTheme(ui->colourTheme->currentText());
    activeSettings.setHostAddress(ui->hostName->text(), ui->hostPort->text().toInt(), ui->hostLU->text());
    activeSettings.setStretchScreen(ui->stretch->QAbstractButton::isChecked());

    //emit setStretch(ui->stretch);

    QDialog::accept();

}

/**
 * @brief   PreferencesDialog::reject - process the Cancel button
 *
 * @details Reset the terminal font to the original one in case the user changed it during the
 *          dialog display.
 *
 * @note    Dynamic font change display is not currently enabled.
 */
void PreferencesDialog::reject()
{
    emit tempFontChange(activeSettings.getFont());

    QDialog::reject();
}

/**
 * @brief   PreferencesDialog::colourThemeDropDownChanged - signalled when the colour theme is changed
 * @param   index - the new colour theme index
 *
 * @details Called when the user changes the ColourTheme drop down so the button colours can be updated.
 */
void PreferencesDialog::colourThemeDropDownChanged([[maybe_unused]] int index)
{
   setButtonColours(ui->colourTheme->currentText());
}

/**
 * @brief   PreferencesDialog::populateColourThemeNames - build the list of ColourThemes
 *
 * @details Called to (re-)populate the list of available ColourThemes. This may change during the
 *          life of the display of the Preferences dialog as the option to manage the ColourThemes
 *          can be invoked from the Preferences dialog itself.
 */
void PreferencesDialog::populateColourThemeNames()
{
    // Refresh the Colour theme names
    ui->colourTheme->clear();
    ui->colourTheme->addItems(colours.getThemes());
}

/**
 * @brief   PreferencesDialog::manageColourThemes - invoke the ColourThemes dialog
 *
 * @details Called when the user pressed the Manange Themes button on the Colours tab. The
 *          list of available colour themes is then rebuilt in case the user modified the list.
 */
void PreferencesDialog::manageColourThemes()
{
    // Run the Colour Themes dialog
    colours.exec();

    // Refresh the colour theme names, in case they changed
    populateColourThemeNames();
}

/**
 * @brief   PreferencesDialog::populateKeyboardThemeNames - build the list of KeyboardThemes
 *
 * @details Called to (re-)populate the list of available KeyboardThemes. This may change during the
 *          life of the display of the Preferences dialog as the option to manage the KeyboardThemes
 *          can be invoked from the Preferences dialog itself.
 */
void PreferencesDialog::populateKeyboardThemeNames()
{
    // Refresh the Keyboard theme names
    ui->keyboardTheme->clear();
    ui->keyboardTheme->addItems(keyboards.getThemes());
}

/**
 * @brief   PreferencesDialog::keyboardThemeDropDownChanged - signalled when the keyboard theme is changed
 * @param   index - the new keyboard theme index
 *
 * @details Called when the user changes the KeyboardTheme drop down so the table of keyboard mappings
 *          can be updated.
 */
void PreferencesDialog::keyboardThemeDropDownChanged([[maybe_unused]] int index)
{
    // Populate keyboard map table
    keyboards.populateTable(ui->KeyboardMap, ui->keyboardTheme->currentText());
}

/**
 * @brief   PreferencesDialog::manageKeyboardThemes - invoke the KeyboardThemes dialog
 *
 * @details Called when the user pressed the Manange Themes button on the Keyboard tab. The
 *          list of available keyboard themes is then rebuilt in case the user modified the list.
 *
 * @note    This routine does nothing at present.
 */
void PreferencesDialog::manageKeyboardThemes()
{
    keyboards.exec();

    populateKeyboardThemeNames();
}
