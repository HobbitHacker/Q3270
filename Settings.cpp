
#include "ui_Settings.h"
#include "Settings.h"

Settings::Settings(ColourTheme *colours, QWidget *parent) : QDialog(parent), ui(new Ui::Settings)
{
    // All initial settings moved to here, extracted from QSettings, or set to default as needed.
    // Other classes refer to this as needed.
    // Separate option to Save Settings

    ui->setupUi(this);
    ui->TabsWidget->setCurrentIndex(0);

    QSettings applicationSettings;

    this->colours = colours;

    // Model type
    if (applicationSettings.contains("terminal/model"))
    {
        termX = applicationSettings.value("terminal/width").toInt();
        termY = applicationSettings.value("terminal/height").toInt();
        changeModel(applicationSettings.value("terminal/model").toString());
        stretchScreen = applicationSettings.value("terminal/stretch").toBool();

    }
    else
    {
        termX = 80;
        termY = 24;
        changeModel("Model2");
    }

    // Cursor blink enabled & speed
    if (applicationSettings.contains("terminal/cursorblink"))
    {
        blink = applicationSettings.value("terminal/cursorblink").toBool();
        blinkSpeed = applicationSettings.value("terminal/cursorblinkspeed").toInt();
        ui->cursorBlink->setChecked(blink);
        ui->cursorBlinkSpeed->setSliderPosition(blinkSpeed);

    }
    else
    {
        blink = true;
        blinkSpeed = 4;
        ui->cursorBlink->setChecked(true);
        ui->cursorBlinkSpeed->setSliderPosition(4);
    }

    // Cursor colour inheritance
    if (applicationSettings.contains("terminal/cursorinheritcolour"))
    {
        cursorInherit = applicationSettings.value("terminal/cusorinheritcolour").toBool();
    }
    else
    {
        cursorInherit = true;
    }

    termFont = QFont("ibm3270", 8);

    // Font
    if (applicationSettings.contains("font/name"))
    {
        printf("%s\n%s\n%d\n", applicationSettings.value("font/name").toString().toLatin1().data(), applicationSettings.value("font/style").toString().toLatin1().data(), applicationSettings.value("font/size").toInt());
        fflush(stdout);
        termFont.setFamily(applicationSettings.value("font/name").toString());
        termFont.setStyleName(applicationSettings.value("font/style").toString());
        termFont.setPointSize(applicationSettings.value("font/size").toInt());
    }
    else
    {
        termFont.setFamily("ibm3270");
        termFont.setStyleName("Regular");
        termFont.setPointSize(32);
    }

    // Font scaling
    if (applicationSettings.contains("font/scale"))
    {
        fontScaling = applicationSettings.value("font/scale").toString() == "true" ? true : false;
        ui->FontScaling->setCheckState(fontScaling ?  Qt::Checked : Qt::Unchecked);

    }
    else
    {
        ui->FontScaling->setCheckState(Qt::Checked);
        fontScaling = true;
    }

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

    colours = new ColourTheme();

    // Colour the buttons, based on Settings
    colourThemeName = applicationSettings.value("ColourTheme", "Factory").toString();
    colourTheme = colours->getTheme(colourThemeName);
    colours->setButtonColours(colourTheme, colourButtons);

    // Setup Manage Themes button
    connect(ui->manageColourThemes, &QPushButton::clicked, this, &Settings::manageColourThemes);

    // Not connected to start with
    ui->terminalCols->setEnabled(true);
    ui->terminalRows->setEnabled(true);
    ui->terminalType->setEnabled(true);

    ui->cursorColour->setCheckState(cursorInherit == true ? Qt::Checked : Qt::Unchecked);

    keyboardChanged = false;

    // Build a QFontDialog for use within our Settings dialog
    qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);
    qfd->setOption(QFontDialog::DontUseNativeDialog);

//    connect(qfd, &QFontDialog::currentFontChanged, this, &Settings::fontChanged);

    ui->verticalLayout_5->addWidget(qfd);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::setKeyboardMap(QMap<QString, QStringList> map)
{
    keyboardMap = map;

    QMap<QString, QStringList>::ConstIterator i = map.constBegin();

    ui->KeyboardMap->setRowCount(0);

    int row = 0;

    while(i != map.constEnd())
    {
        ui->KeyboardMap->insertRow(row);
        ui->KeyboardMap->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->KeyboardMap->setItem(row, 1, new QTableWidgetItem(i.value().join(", ")));
        printf("Function name %s mapped to %s\n",  i.key().toLatin1().data(), i.value().join(",").toLatin1().data());
        fflush(stdout);
        i++;
    }

    ui->KeyboardMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void Settings::showForm(bool connected)
{
    if (connected)
    {
        ui->terminalCols->setDisabled(true);
        ui->terminalRows->setDisabled(true);
        ui->terminalType->setDisabled(true);
    }
    else
    {
        ui->terminalCols->setEnabled(true);
        ui->terminalRows->setEnabled(true);
        ui->terminalType->setEnabled(true);
    }

    ui->cursorBlinkSpeed->setEnabled(ui->cursorBlink->QAbstractButton::isChecked());

    paletteChanged = false;

    qfd->setCurrentFont(termFont);

    populateThemeNames();

    this->exec();
}

void Settings::changeModel(int model)
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

void Settings::changeModel(QString type)
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

void Settings::changeSize(int x, int y)
{
    if (termType != 4)
    {
        return;
    }

    terms[4].x = x;
    terms[4].y = y;
}

void Settings::changeFont(QFont newFont)
{
    emit tempFontChange(newFont);
}


void Settings::accept()
{
    if (ui->terminalType->currentIndex() != termType || termX != ui->terminalCols->value() || termY != ui->terminalRows->value())
    {
        termType = ui->terminalType->currentIndex();
        termX = ui->terminalCols->value();
        termY = ui->terminalRows->value();
        emit terminalChanged(termType, termX, termY);
    }

    if (ui->cursorBlink->QAbstractButton::isChecked() != blink)
    {
        blink = ui->cursorBlink->QAbstractButton::isChecked();
        blinkSpeed = ui->cursorBlinkSpeed->value();
        emit cursorBlinkChanged(blink, blinkSpeed);
    }

    if (ui->cursorBlinkSpeed->value() != blinkSpeed)
    {
        blinkSpeed = ui->cursorBlinkSpeed->value();
        emit cursorBlinkSpeedChanged(blinkSpeed);
    }

    if (paletteChanged)
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

    if (keyboardChanged)
    {
        emit newMap(keyboardMap);
    }

    if (ui->cursorColour->QAbstractButton::isChecked() != cursorInherit)
    {
        cursorInherit = ui->cursorColour->QAbstractButton::isChecked();

        emit setCursorColour(cursorInherit);
    }

    emit setStretch(ui->stretch);

    QDialog::accept();

}

void Settings::reject()
{
    emit tempFontChange(termFont);

    QDialog::reject();
}

int Settings::getTermX()
{
    return termX;
}

int Settings::getTermY()
{
    return termY;
}

QFont Settings::getFont()
{
    printf("%s\n%s\n%d\n", termFont.family().toLatin1().data(), termFont.styleName().toLatin1().data(), termFont.pointSize());
    fflush(stdout);
    return termFont;
}

QString Settings::getTermName()
{
    return terms[termType].term;
}

int Settings::getBlinkSpeed()
{
    return blinkSpeed;
}

bool Settings::getBlink()
{
    return blink;
}

bool Settings::getFontScaling()
{
    return fontScaling;
}

bool Settings::getStretch()
{
    return stretchScreen;
}

ColourTheme::Colours Settings::getColours()
{
    return colourTheme;
}

void Settings::saveSettings()
{
    QSettings qs;

    qs.setValue("terminal/model", terms[termType].name);
    qs.setValue("terminal/width", termX);
    qs.setValue("terminal/height", termY);

    qs.setValue("terminal/cursorblink", blink);
    qs.setValue("terminal/cursorblinkspeed", blinkSpeed);

    qs.setValue("font/name", termFont.family());
    qs.setValue("font/style", termFont.styleName());
    qs.setValue("font/size", termFont.pointSize());

    qs.setValue("font/scale", ui->FontScaling->QAbstractButton::isChecked());

    qs.setValue("terminal/stretch", ui->stretch->QAbstractButton::isChecked());

    emit newMap(keyboardMap);
    emit saveKeyboardSettings();

    QMessageBox saved;
    saved.setText("Settings saved");
    saved.exec();

}

void Settings::colourThemeChanged(int index)
{
    colourThemeName = ui->colourTheme->currentText();
    colourTheme = colours->getTheme(colourThemeName);
    colours->setButtonColours(colourTheme, colourButtons);

    paletteChanged = true;
}


void Settings::populateThemeNames()
{
    ui->colourTheme->clear();
    ui->colourTheme->addItems(colours->getThemes());
}

void Settings::manageColourThemes()
{
    // Run the Colour Themes dialog
    colours->exec();
    populateThemeNames();
    emit coloursChanged(colourTheme);
}
