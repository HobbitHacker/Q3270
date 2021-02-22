#include "ui_Settings.h"
#include "Settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings)
{
    // All initial settings moved to here, extracted from QSettings, or set to default as needed.
    // Other classes refer to this as needed.
    // Separate option to Save Settings

    ui->setupUi(this);

    QSettings *applicationSettings = new QSettings();

    // Model type
    if (applicationSettings->contains("terminal/model"))
    {
        termX = applicationSettings->value("terminal/width").toInt();
        termY = applicationSettings->value("terminal/height").toInt();
        changeModel(applicationSettings->value("terminal/model").toString());
    }
    else
    {
        changeModel(2);
    }

    // Cursor blink enabled & speed
    if (applicationSettings->contains("terminal/cursorblink"))
    {
        blink = applicationSettings->value("terminal/cursorblink").toBool();
        blinkSpeed = applicationSettings->value("terminal/cursorblinkspeed").toInt();
        ui->cursorBlink->setEnabled(blink);
        ui->cursorBlinkSpeed->setSliderPosition(blinkSpeed);

    }
    else
    {
        blink = true;
        blinkSpeed = 5;
        ui->cursorBlink->setChecked(true);
        ui->cursorBlinkSpeed->setSliderPosition(5);
    }

    termFont = QFont("ibm3270", 8);

    // Font
    if (applicationSettings->contains("font/name"))
    {
        printf("%s\n%s\n%d\n", applicationSettings->value("font/name").toString().toLatin1().data(), applicationSettings->value("font/style").toString().toLatin1().data(), applicationSettings->value("font/size").toInt());
        fflush(stdout);
        termFont.setFamily(applicationSettings->value("font/name").toString());
        termFont.setStyleName(applicationSettings->value("font/style").toString());
        termFont.setPointSize(applicationSettings->value("font/size").toInt());
    }
    else
    {
        termFont.setFamily("ibm3270");
        termFont.setStyleName("Regular");
        termFont.setPointSize(8);
    }

    // Font scaling
    if (applicationSettings->contains("font/scale"))
    {
        ui->FontScaling->setCheckState(applicationSettings->value("font/scale").toString() == "true" ?  Qt::Checked : Qt::Unchecked);
    }
    else
    {
        ui->FontScaling->setCheckState(Qt::Checked);
    }

    // Colours
    if (applicationSettings->beginReadArray("colours") > 0)
    {
        for (int i = 0; i < 8; i++)
        {
            applicationSettings->setArrayIndex(i);
            palette[i] = QColor(applicationSettings->value("colour").toString());
        }
        applicationSettings->endArray();
    }
    else
    {
        for(int i = 0; i < 8; i++)
        {
            palette[i] = default_palette[i];
        }
    }

    // Not connected to start with
    ui->terminalCols->setEnabled(true);
    ui->terminalRows->setEnabled(true);
    ui->terminalType->setEnabled(true);


    ui->colourBlack->setStyleSheet(QString("background-color: %1;").arg(palette[0].name(QColor::HexRgb)));
    ui->colourBlue->setStyleSheet(QString("background-color: %1;").arg(palette[1].name(QColor::HexRgb)));
    ui->colourRed->setStyleSheet(QString("background-color: %1;").arg(palette[2].name(QColor::HexRgb)));
    ui->colourPink->setStyleSheet(QString("background-color: %1;").arg(palette[3].name(QColor::HexRgb)));
    ui->colourGreen->setStyleSheet(QString("background-color: %1;").arg(palette[4].name(QColor::HexRgb)));
    ui->colourTurq->setStyleSheet(QString("background-color: %1;").arg(palette[5].name(QColor::HexRgb)));
    ui->colourYellow->setStyleSheet(QString("background-color: %1;").arg(palette[6].name(QColor::HexRgb)));
    ui->colourWhite->setStyleSheet(QString("background-color: %1;").arg(palette[7].name(QColor::HexRgb)));

    connect(ui->colourBlack, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourBlue, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourRed, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourPink, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourGreen, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourTurq, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourYellow, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourWhite, &QPushButton::clicked, this, &Settings::setColour);

    qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);

//    connect(qfd, &QFontDialog::currentFontChanged, this, &Settings::fontChanged);

    ui->verticalLayout_5->addWidget(qfd);
}

Settings::~Settings()
{
    delete ui;
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

    paletteChanged = false;

    qfd->setCurrentFont(termFont);

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
        if (type == terms[i].term)
        {
            termType = i;
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
    if (ui->terminalType->currentIndex() != termType || termX != ui->terminalCols->value() | termY != ui->terminalRows->value())
    {
        termType = ui->terminalType->currentIndex();
        termX = ui->terminalCols->value();
        termY = ui->terminalRows->value();
        emit terminalChanged(termType, termX, termY);
    }

    if (ui->cursorBlink->QAbstractButton::isChecked() != blink || ui->cursorBlinkSpeed->value() != blinkSpeed)
    {
        blink = ui->cursorBlink->QAbstractButton::isChecked();
        blinkSpeed = ui->cursorBlinkSpeed->value();
        emit cursorBlinkChanged(blink, blinkSpeed);
    }

    if (paletteChanged)
    {
        emit coloursChanged(palette);
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

    QDialog::accept();

}

void Settings::reject()
{
    emit tempFontChange(termFont);

    QDialog::reject();
}

void Settings::setColour()
{

    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());

    buttonSender->clearFocus();

    QString button = buttonSender->objectName();

    int thisColour;

    if (!button.compare("colourBlack"))
    {
        thisColour = 0;
    }
    else if (!button.compare("colourBlue"))
    {
        thisColour = 1;
    }
    else if (!button.compare("colourRed"))
    {
        thisColour = 2;
    }
    else if (!button.compare("colourPink"))
    {
        thisColour = 3;
    }
    else if (!button.compare("colourGreen"))
    {
        thisColour = 4;
    }
    else if (!button.compare("colourTurq"))
    {
        thisColour = 5;
    }
    else if (!button.compare("colourYellow"))
    {
        thisColour = 6;
    }
    else
    {
        thisColour = 7;
    }

    const QColor color = QColorDialog::getColor(palette[thisColour], this, "Select Color");

    if (color.isValid())
    {
        if (palette[thisColour] != color)
        {
            paletteChanged = true;
            buttonSender->setStyleSheet(QString("background-color: %1;").arg(color.name(QColor::HexRgb)));
            palette[thisColour] = color;
        }
    }
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

QColor *Settings::getColours()
{
    return palette;
}
