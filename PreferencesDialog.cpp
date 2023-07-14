#include "Q3270.h"

#include "ui_PreferencesDialog.h"
#include "PreferencesDialog.h"
#include <QDebug>
#include "PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(ColourTheme &colours, KeyboardTheme &keyboards, ActiveSettings *activeSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    colours(colours),
    keyboards(keyboards),
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

    // Setup Manage Themes buttons
    connect(ui->manageColourThemes, &QPushButton::clicked, this, &PreferencesDialog::manageColourThemes);
    connect(ui->manageKeyboardThemes, &QPushButton::clicked, this, &PreferencesDialog::manageKeyboardThemes);

    // TODO: "Enter" when displaying font selection causes font dialog to vanish from widget

    // Build a QFontDialog for use within our Settings dialog
    qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);
    qfd->setOption(QFontDialog::DontUseNativeDialog);

//    connect(qfd, &QFontDialog::currentFontChanged, this, &Settings::fontChanged);

    ui->verticalLayout_5->addWidget(qfd);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::connected()
{
    ui->terminalCols->setDisabled(true);
    ui->terminalRows->setDisabled(true);
    ui->terminalType->setDisabled(true);

    ui->hostName->setDisabled(true);
    ui->hostPort->setDisabled(true);
    ui->hostLU->setDisabled(true);
}

void PreferencesDialog::disconnected()
{
    ui->terminalType->setEnabled(true);

    // Rows and columns only editable if it's dynamic size
    if (activeSettings->getTerminalModel() == Q3270_TERMINAL_DYNAMIC)
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

void PreferencesDialog::showForm()
{
    ui->hostLU->setText(activeSettings->getHostLU());
    ui->hostName->setText(activeSettings->getHostName());
    ui->hostPort->setText(QString::number(activeSettings->getHostPort()));

    // Terminal model/rows/cols
    ui->terminalType->setCurrentIndex(activeSettings->getTerminalModel());
    ui->terminalCols->setValue(activeSettings->getTerminalX());
    ui->terminalRows->setValue(activeSettings->getTerminalY());


    // Cursor blink enabled & speed
    ui->cursorBlink->setChecked(activeSettings->getCursorBlink());
    ui->cursorBlinkSpeed->setSliderPosition(activeSettings->getCursorBlinkSpeed());

    // Enable blink speed adjustment only if blink is enabled
    ui->cursorBlinkSpeed->setEnabled(ui->cursorBlink->QAbstractButton::isChecked());

    ui->cursorColour->setChecked(activeSettings->getCursorColourInherit());
    ui->stretch->setChecked(activeSettings->getStretchScreen());

    ui->backspaceStop->setChecked(activeSettings->getBackspaceStop());

    qfd->setCurrentFont(activeSettings->getFont());

    populateColourThemeNames();
    populateKeyboardThemeNames();

    // Colour the buttons, based on Settings
    colours.setButtonColours(colourButtons, activeSettings->getColourThemeName());

    ui->CodePages->setCurrentIndex(ui->CodePages->findText(activeSettings->getCodePage()));

    this->exec();
}

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

void PreferencesDialog::changeFont(QFont newFont)
{
    emit tempFontChange(newFont);
}

void PreferencesDialog::accept()
{
    activeSettings->setTerminal(ui->terminalCols->value(), ui->terminalRows->value(), ui->terminalType->currentIndex());
    activeSettings->setCursorBlink(ui->cursorBlink->QAbstractButton::isChecked());
    activeSettings->setCursorBlinkSpeed(ui->cursorBlinkSpeed->value());
    activeSettings->setRulerOn(ui->rulerOn->QAbstractButton::isChecked());
    activeSettings->setRulerStyle(comboRulerStyle.value(ui->crosshair->currentText()));
    activeSettings->setCursorColourInherit(ui->cursorColour->QAbstractButton::isChecked());
    activeSettings->setFont(qfd->currentFont());
    activeSettings->setCodePage(ui->CodePages->currentText());
    activeSettings->setKeyboardTheme(keyboards, ui->keyboardTheme->currentText());
    activeSettings->setColourTheme(ui->colourTheme->currentText());
    activeSettings->setHostAddress(ui->hostName->text(), ui->hostPort->text().toInt(), ui->hostLU->text());
    activeSettings->setStretchScreen(ui->stretch->QAbstractButton::isChecked());

    //emit setStretch(ui->stretch);

    QDialog::accept();

}

void PreferencesDialog::reject()
{
    emit tempFontChange(activeSettings->getFont());

    QDialog::reject();
}

void PreferencesDialog::populateCodePages(QMap<QString, QString> codepagelist)
{
    ui->CodePages->addItems(codepagelist.keys());
}

void PreferencesDialog::colourThemeDropDownChanged([[maybe_unused]] int index)
{
    colours.setButtonColours(colourButtons, ui->colourTheme->currentText());
}

void PreferencesDialog::populateColourThemeNames()
{
    // Refresh the Colour theme names
    ui->colourTheme->clear();
    ui->colourTheme->addItems(colours.getThemes());
}

void PreferencesDialog::manageColourThemes()
{
    // Run the Colour Themes dialog
    colours.exec();

    // Refresh the colour theme names, in case they changed
    populateColourThemeNames();
}

void PreferencesDialog::populateKeyboardThemeNames()
{
    // Refresh the Keyboard theme names
    ui->keyboardTheme->clear();
    ui->keyboardTheme->addItems(keyboards.getThemes());
}

void PreferencesDialog::keyboardThemeDropDownChanged([[maybe_unused]] int index)
{
    // Populate keyboard map table
    keyboards.populateTable(ui->KeyboardMap, ui->keyboardTheme->currentText());
}


void PreferencesDialog::manageKeyboardThemes()
{
    //
}
