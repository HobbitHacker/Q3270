#include "ui_Settings.h"
#include "Settings.h"

Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::Settings)
{
    // All initial settings moved to here, extracted from QSettings, or set to default as needed.
    // Other classes refer to this as needed.
    // Separate option to Save Settings

    ui->setupUi(this);
    ui->TabsWidget->setCurrentIndex(0);

    QSettings applicationSettings;

    // Model type
    if (applicationSettings.contains("terminal/model"))
    {
        termX = applicationSettings.value("terminal/width").toInt();
        termY = applicationSettings.value("terminal/height").toInt();
        changeModel(applicationSettings.value("terminal/model").toString());
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

    // Colours
    if (applicationSettings.beginReadArray("colours") > 0)
    {
        for (int i = 0; i < 12; i++)
        {
            applicationSettings.setArrayIndex(i);
            QString c = applicationSettings.value("colour").toString();
            if (c == "")
            {
                palette[i] = default_palette[i];
            }
            else
            {
                palette[i] = QColor(applicationSettings.value("colour").toString());
            }
        }
        applicationSettings.endArray();
    }
    else
    {
        for(int i = 0; i < 12; i++)
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

    ui->baseProtected->setStyleSheet(QString("background-color: %1;").arg(palette[8].name(QColor::HexRgb)));
    ui->baseUnprotectedIntensify->setStyleSheet(QString("background-color: %1;").arg(palette[9].name(QColor::HexRgb)));
    ui->baseUnprotected->setStyleSheet(QString("background-color: %1;").arg(palette[10].name(QColor::HexRgb)));
    ui->baseProtectedIntensify->setStyleSheet(QString("background-color: %1;").arg(palette[11].name(QColor::HexRgb)));

    connect(ui->baseProtected, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->baseUnprotectedIntensify, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->baseUnprotected, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->baseProtectedIntensify, &QPushButton::clicked, this, &Settings::setColour);

    connect(ui->KeyboardMap, &QTableWidget::itemClicked, this, &Settings::populateKeySequence);
    connect(ui->keySequenceEdit, &QKeySequenceEdit::editingFinished, this, &Settings::truncateShortcut);
    connect(ui->setKeyboardMap, &QPushButton::clicked, this, &Settings::setKey);

    ui->cursorColour->setCheckState(cursorInherit == true ? Qt::Checked : Qt::Unchecked);

    lastRow = -1;
    lastSeq = -1;

    keyboardChanged = false;

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

    ui->KeyboardFunctionList->addItem("Unassigned");

    while(i != map.constEnd())
    {
        ui->KeyboardMap->insertRow(row);
        ui->KeyboardMap->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->KeyboardMap->setItem(row, 1, new QTableWidgetItem(i.value().join(", ")));
        ui->KeyboardFunctionList->addItem(i.key());
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
    else if (!button.compare("colourWhite"))
    {
        thisColour = 7;
    }
    else if (!button.compare("baseProtected"))
    {
        thisColour = 8;
    }
    else if (!button.compare("baseUnprotectedIntensify"))
    {
        thisColour = 9;
    }
    else if (!button.compare("baseUnprotected"))
    {
        thisColour = 10;
    }
    else if (!button.compare("baseUnprotectedIntensify"))
    {
        thisColour = 11;
    }
    else
    {
        return;
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

    qs.beginWriteArray("colours");
    for(int i = 0; i < 12; i++)
    {
        qs.setArrayIndex(i);
        qs.setValue("colour", palette[i].name(QColor::HexRgb));
    }
    qs.endArray();

    emit newMap(keyboardMap);
    emit saveKeyboardSettings();

    QMessageBox saved;
    saved.setText("Settings saved");
    saved.exec();

}

void Settings::populateKeySequence(QTableWidgetItem *item)
{
    //NOTE: This doesn't handle the custom left-ctrl/right-ctrl stuff
    int thisRow = item->row();
    QStringList keyList = ui->KeyboardMap->item(thisRow, 1)->text().split(", ");

    if (thisRow != lastRow)
    {
        lastSeq = 0;
    }
    else
    {
        if (++lastSeq >= keyList.size())
            lastSeq = 0;
    }

    ui->KeyboardFunctionList->setCurrentIndex(ui->KeyboardFunctionList->findText(ui->KeyboardMap->item(thisRow, 0)->text()));

    if (keyList.size() != 0)
    {
        ui->keySequenceEdit->setKeySequence(QKeySequence(keyList[lastSeq]));
    }
    else
    {
        ui->keySequenceEdit->clear();
    }

    if (keyList.size() == 1)
    {
        ui->LabelMultipleKeys->setText("");
    }
    else
    {
        ui->LabelMultipleKeys->setText("Click again for next key mapping");
    }

    lastRow = thisRow;

}

void Settings::setKey()
{
    QMap<QString, QStringList>::iterator i = keyboardMap.begin();

    while(i != keyboardMap.end())
    {
        for (int s = 0; s < i.value().size(); s++)
        {
            if (QKeySequence(i.value()[s]) == ui->keySequenceEdit->keySequence())
            {
                printf("Found %s as %s - removing\n", i.value()[s].toLatin1().data(), i.key().toLatin1().data());
                fflush(stdout);
                // Remove existing entry
                i.value().removeAt(s);
            }
        }
        if (ui->KeyboardFunctionList->currentIndex() != 0 && !i.key().compare(ui->KeyboardFunctionList->currentText()))
        {
            printf("Adding to %s\n", i.key().toLatin1().data());
            fflush(stdout);
            i.value().append(ui->keySequenceEdit->keySequence().toString());
        }
        i++;
    }

    setKeyboardMap(keyboardMap);

    keyboardChanged = true;
}

void Settings::truncateShortcut()
{
    int value = ui->keySequenceEdit->keySequence()[0];
    QKeySequence shortcut(value);
    ui->keySequenceEdit->setKeySequence(shortcut);
}


