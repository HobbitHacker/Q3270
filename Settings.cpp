#include "ui_Settings.h"
#include "Settings.h"

Settings::Settings(QWidget *parent, TerminalTab *t) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    this->t = t;

    ui->setupUi(this);

    changeModel(t->getType());

    ui->cursorBlink->setChecked(t->view->getBlink());
    ui->cursorBlinkSpeed->setSliderPosition(t->view->getBlinkSpeed());

    if (t->view->connected)
    {
        ui->terminalCols->setDisabled(true);
        ui->terminalRows->setDisabled(true);
        ui->terminalType->setDisabled(true);
    }

    for(int i = 0; i < 8; i++)
    {
        palette[i] = t->palette[i];
        printf("%d = %s\n", i, t->palette[i].name(QColor::HexRgb).toLatin1().data());
    }
    fflush(stdout);

    ui->colourBlack->setStyleSheet(QString("background-color: %1;").arg(t->palette[0].name(QColor::HexRgb)));
    ui->colourBlue->setStyleSheet(QString("background-color: %1;").arg(t->palette[1].name(QColor::HexRgb)));
    ui->colourRed->setStyleSheet(QString("background-color: %1;").arg(t->palette[2].name(QColor::HexRgb)));
    ui->colourPink->setStyleSheet(QString("background-color: %1;").arg(t->palette[3].name(QColor::HexRgb)));
    ui->colourGreen->setStyleSheet(QString("background-color: %1;").arg(t->palette[4].name(QColor::HexRgb)));
    ui->colourTurq->setStyleSheet(QString("background-color: %1;").arg(t->palette[5].name(QColor::HexRgb)));
    ui->colourYellow->setStyleSheet(QString("background-color: %1;").arg(t->palette[6].name(QColor::HexRgb)));
    ui->colourWhite->setStyleSheet(QString("background-color: %1;").arg(t->palette[7].name(QColor::HexRgb)));

    connect(ui->colourBlack, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourBlue, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourRed, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourPink, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourGreen, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourTurq, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourYellow, &QPushButton::clicked, this, &Settings::setColour);
    connect(ui->colourWhite, &QPushButton::clicked, this, &Settings::setColour);

    QFontDialog *qfd = new QFontDialog();
    qfd->setWindowFlags(Qt::Widget);
    qfd->setOption(QFontDialog::NoButtons);

    ui->verticalLayout_5->addWidget(qfd);
}

Settings::~Settings()
{
    delete ui;
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
            termX = t->terminalWidth();
            termY = t->terminalHeight();
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

void Settings::accept()
{
    t->setType(ui->terminalType->currentIndex());

    t->setSize(ui->terminalCols->value(), ui->terminalRows->value());
    t->view->setBlink(ui->cursorBlink->checkState() == Qt::Checked);
    t->view->setBlinkSpeed(ui->cursorBlinkSpeed->sliderPosition());

    t->setColours(palette);

    QDialog::accept();

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
        buttonSender->setStyleSheet(QString("background-color: %1;").arg(color.name(QColor::HexRgb)));
        palette[thisColour] = color;
    }

}
