#include "ui_settings.h"
#include "settings.h"

Settings::Settings(QWidget *parent, TerminalTab *t) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    this->t = t;
    ui->setupUi(this);
    changeModel(t->getType());
    ui->cursorBlink->setChecked(t->term->getBlink());
    printf("Settings        : Blink speed from terminal: %d\n", t->term->getBlinkSpeed());
    fflush(stdout);
    ui->cursorBlinkSpeed->setSliderPosition(t->term->getBlinkSpeed());

//    setFixedSize(size());
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
        case 4:
            ui->terminalCols->setEnabled(true);
            ui->terminalRows->setEnabled(true);
            return;
        case 0:
            termX = 80;
            termY = 24;
            break;
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
    }

    ui->terminalCols->setValue(termX);
    ui->terminalRows->setValue(termY);
}

void Settings::accept()
{
    t->setType(ui->terminalType->currentIndex());

    t->setSize(ui->terminalCols->value(), ui->terminalRows->value());
    t->term->setBlink(ui->cursorBlink);
    t->term->setBlinkSpeed(ui->cursorBlinkSpeed->sliderPosition());

    QDialog::accept();

}
