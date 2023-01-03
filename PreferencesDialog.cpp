#include "Q3270.h"

#include "ui_PreferencesDialog.h"
#include "PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(ColourTheme *colours, KeyboardTheme *keyboards, ActiveSettings *activeSettings, CodePage *codepage, QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->TabsWidget->setCurrentIndex(0);

    QSettings applicationSettings;

    // Store theme dialogs
    this->colours = colours;
    this->keyboards = keyboards;
    this->codepage = codepage;
    this->activeSettings = activeSettings;

    // Set up vector to coordinate combox box for crosshair types
    comboRulerStyle.insert("Crosshairs", Q3270_RULER_CROSSHAIR);
    comboRulerStyle.insert("Vertical", Q3270_RULER_VERTICAL);
    comboRulerStyle.insert("Horizontal", Q3270_RULER_HORIZONTAL);

    // Populate combo box from vector keys
    ui->crosshair->addItems(comboRulerStyle.keys());

    // Factory default settings from here

    // Terminal Model type
    termX = 80;
    termY = 24;
    setTerminalModel("Model2");

    // Default font
    termFont = QFont("ibm3270", 8);

    // Font scaling
    ui->FontScaling->setCheckState(Qt::Checked);
    fontScaling = true;

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

    // Colour the buttons, based on Settings
    colourThemeName = applicationSettings.value("ColourTheme", "Factory").toString();
    colourTheme = colours->getTheme(colourThemeName);
    colours->setButtonColours(colourTheme, colourButtons);

    // Setup Manage Themes buttons
    connect(ui->manageColourThemes, &QPushButton::clicked, this, &PreferencesDialog::manageColourThemes);
    connect(ui->manageKeyboardThemes, &QPushButton::clicked, this, &PreferencesDialog::manageKeyboardThemes);

    // Not connected to start with
    ui->terminalCols->setEnabled(true);
    ui->terminalRows->setEnabled(true);
    ui->terminalType->setEnabled(true);

    // TODO: "Enter" when displaying font selection causes font dialog to vanish from widget

    // Build a QFontDialog for use within our Settings dialog
    qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);
    qfd->setOption(QFontDialog::DontUseNativeDialog);

    // Populate code page list
    ui->CodePages->addItems(codepage->getCodePageList().keys());

//    connect(qfd, &QFontDialog::currentFontChanged, this, &Settings::fontChanged);

    ui->verticalLayout_5->addWidget(qfd);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::showForm(bool connected)
{
    // Certain fields can't be changed without a disconnect/reconnect
    if (connected)
    {
        ui->terminalCols->setDisabled(true);
        ui->terminalRows->setDisabled(true);
        ui->terminalType->setDisabled(true);

        ui->hostName->setDisabled(true);
        ui->hostPort->setDisabled(true);
        ui->hostLU->setDisabled(true);
    }
    else
    {
        ui->terminalCols->setEnabled(true);
        ui->terminalRows->setEnabled(true);
        ui->terminalType->setEnabled(true);

        ui->hostName->setEnabled(true);
        ui->hostPort->setEnabled(true);
        ui->hostLU->setEnabled(true);
    }

    // Cursor blink enabled & speed
    ui->cursorBlink->setChecked(activeSettings->getCursorBlink());
    ui->cursorBlinkSpeed->setSliderPosition(activeSettings->getCursorBlinkSpeed());
    ui->cursorColour->setChecked(activeSettings->getCursorInherit());

    ui->cursorBlinkSpeed->setEnabled(ui->cursorBlink->QAbstractButton::isChecked());

    // No changes to colours or keyboards
    colourThemeChangeFlag = false;
    keyboardThemeChangeFlag = false;

    qfd->setCurrentFont(termFont);

    populateColourThemeNames();
    populateKeyboardThemeNames();

    ui->CodePages->setCurrentIndex(ui->CodePages->findText(codepage->getCodePage()));
    formCodePage = ui->CodePages->currentIndex();

    this->exec();
}

void PreferencesDialog::setAddress(QString newAddress)
{

    // Determine if the supplied address contains an '@' denoting the LU to be used.
    //
    // Addresses can be of the form:
    //
    //   0700@localhost:3270
    //   console@1.2.3.4:23
    //   1.2.3.4:23

    if (newAddress.contains("@"))
    {
        ui->hostLU->setText(newAddress.section("@", 0, 0));
        ui->hostName->setText(newAddress.section("@", 1, 1).section(":", 0, 0));
        ui->hostPort->setText(newAddress.section(":", 1, 1));
    } else
    {
        ui->hostLU->setText("");
        ui->hostName->setText(newAddress.section(":", 0, 0));
        ui->hostPort->setText(newAddress.section(":", 1, 1));
    }

    hostName = ui->hostName->text();
    hostLU = ui->hostLU->text();
    hostPort = ui->hostPort->text().toInt();

    if (hostName.isEmpty())
    {
        emit connectValid(false);
    }
    else
    {
        emit connectValid(true);
    }
}

QString PreferencesDialog::getAddress()
{
    // Return empty string if we don't have a hostname
    if (hostName.isEmpty())
    {
        return "";
    }

    // Return address, allowing for optional LU name
    if (hostLU.isEmpty())
    {
        return hostName + ":" + QString::number(hostPort);
    } else
    {
        return hostLU + "@" + hostName + ":" + QString::number(hostPort);
    }
}

CodePage* PreferencesDialog::codePage()
{
    return codepage;
}


void PreferencesDialog::setTerminalModel(int model)
{
    /* Model types from dialog are 0 - 4 where 0 - 3 are Models 2 - 5 and 4 is dynamic */
    printf("Model = %d", model);
    fflush(stdout);

    termType = model;
    int termX, termY;

    ui->terminalCols->setDisabled(true);
    ui->terminalRows->setDisabled(true);

    switch(model)
    {
        case 1:
            termX = 80;
            termY = 32;
            break;
        case 2:
            termX = 80;
            termY = 43;
            break;
        case 3:
            termX = 132;
            termY = 27;
            break;
        case 4:
            termX = this->termX;
            termY = this->termY;
            ui->terminalCols->setEnabled(true);
            ui->terminalRows->setEnabled(true);
            break;
        default:
            termX = 80;
            termY = 24;
            break;
    }

    ui->terminalType->setCurrentIndex(model);

    ui->terminalCols->setValue(termX);
    ui->terminalRows->setValue(termY);
}

void PreferencesDialog::setTerminalModel(QString type)
{
    for (int i = 0; i < 5; i++)
    {
        if (type == terms[i].name)
        {
            termType = i;
            ui->terminalType->setCurrentIndex(i);
            ui->terminalCols->setValue(termX);
            ui->terminalRows->setValue(termY);
            return;
        }
    }

    termType = 0;
}

void PreferencesDialog::setTerminalSize(int x, int y)
{
    if (termType != 4)
    {
        return;
    }

    terms[4].x = x;
    terms[4].y = y;

    termX = x;
    termY = y;
}

void PreferencesDialog::changeFont(QFont newFont)
{
    emit tempFontChange(newFont);
}

void PreferencesDialog::accept()
{

    hostName = ui->hostName->text();
    hostPort = ui->hostPort->text().toInt();
    hostLU   = ui->hostLU->text();

    if (ui->terminalType->currentIndex() != termType || termX != ui->terminalCols->value() || termY != ui->terminalRows->value())
    {
        termType = ui->terminalType->currentIndex();
        termX = ui->terminalCols->value();
        termY = ui->terminalRows->value();
        emit terminalChanged(termType, termX, termY);
    }

    activeSettings->setCursorBlink(ui->cursorBlink->QAbstractButton::isChecked());
    activeSettings->setCursorBlinkSpeed(ui->cursorBlinkSpeed->value());
    activeSettings->setRulerOn(ui->rulerOn->QAbstractButton::isChecked());
    activeSettings->setRulerStyle(comboRulerStyle.value(ui->crosshair->currentText()));
    activeSettings->setCursorInherit(ui->cursorColour->QAbstractButton::isChecked());

    if (colourThemeChangeFlag)
    {
        emit coloursChanged(colourTheme);
    }

    printf("Before: %s - %s - %d\n", termFont.family().toLatin1().data(), termFont.styleName().toLatin1().data(), termFont.pointSize());
    printf("qfd selected: %s - %s - %d\n", qfd->selectedFont().family().toLatin1().data(), qfd->selectedFont().styleName().toLatin1().data(), qfd->selectedFont().pointSize());
    printf("qfd current: %s - %s - %d\n", qfd->currentFont().family().toLatin1().data(), qfd->currentFont().styleName().toLatin1().data(), qfd->currentFont().pointSize());
    fflush(stdout);

    if (qfd->currentFont() != termFont)
    {
        termFont = qfd->currentFont();
        printf("%s\n%s\n%d\n", termFont.family().toLatin1().data(), termFont.styleName().toLatin1().data(), termFont.pointSize());
        fflush(stdout);
        emit fontChanged();
    }

    if (ui->FontScaling->QAbstractButton::isChecked() != fontScaling)
    {
        fontScaling = ui->FontScaling->QAbstractButton::isChecked();
        emit fontScalingChanged(fontScaling);
    }

    // Signal keyboard update needed if user changed the keyboard theme
    if (keyboardThemeChangeFlag)
    {
        emit setKeyboardTheme(keyboardTheme);
    }

    if (ui->CodePages->currentIndex() != formCodePage)
    {
        codepage->setCodePage(ui->CodePages->currentText());
        emit codePageChanged();
    }

    if (!hostName.isEmpty())
    {
        emit connectValid(true);
    }

    emit setStretch(ui->stretch);

    QDialog::accept();

}

void PreferencesDialog::reject()
{
    emit tempFontChange(termFont);

    QDialog::reject();
}

int PreferencesDialog::getTermX()
{
    return termX;
}

int PreferencesDialog::getTermY()
{
    return termY;
}

QFont PreferencesDialog::getFont()
{
    printf("%s\n%s\n%d\n", termFont.family().toLatin1().data(), termFont.styleName().toLatin1().data(), termFont.pointSize());
    fflush(stdout);
    return termFont;
}

QString PreferencesDialog::getTermName()
{
    return terms[termType].term;
}

void PreferencesDialog::setFont(QFont font)
{
    termFont = font;
    emit fontChanged();
}

void PreferencesDialog::setFontScaling(bool scale)
{
    fontScaling = scale;
}

ColourTheme::Colours PreferencesDialog::getColours()
{
    return colourTheme;
}

QString PreferencesDialog::getCodePage()
{
    return codepage->getCodePage();
}

void PreferencesDialog::setCodePage(QString codepage)
{
    this->codepage->setCodePage(codepage);
    emit codePageChanged();
}

void PreferencesDialog::colourThemeChanged(int index)
{
    colourThemeName = ui->colourTheme->currentText();
    colourTheme = colours->getTheme(colourThemeName);
    colours->setButtonColours(colourTheme, colourButtons);

    // Signify that if the user pressed OK, a colour theme change occurred
    colourThemeChangeFlag = true;
}

void PreferencesDialog::populateColourThemeNames()
{
    // Refresh the Colour theme names
    ui->colourTheme->clear();
    ui->colourTheme->addItems(colours->getThemes());
}

void PreferencesDialog::manageColourThemes()
{
    // Run the Colour Themes dialog
    colours->exec();

    // Refresh the colour theme names, in case they changed
    populateColourThemeNames();

    // Refresh the current colour theme, in case that changed
    emit coloursChanged(colourTheme);
}

void PreferencesDialog::populateKeyboardThemeNames()
{
    // Refresh the Keyboard theme names
    ui->keyboardThemes->clear();
    ui->keyboardThemes->addItems(keyboards->getThemes());
}

void PreferencesDialog::keyboardThemeChanged(int index)
{
    // Store the newly selected named
    keyboardThemeName = ui->keyboardThemes->currentText();

    // Get the newly selected theme from the KeyboardTheme object
    keyboardTheme = keyboards->getTheme(keyboardThemeName);

    // Populate keyboard map table
    keyboards->populateTable(ui->KeyboardMap, keyboardTheme);

    // Signify that if the user pressed OK, a colour theme change occurred
    keyboardThemeChangeFlag = true;
}


void PreferencesDialog::manageKeyboardThemes()
{
    //
}
