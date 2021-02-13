#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QColorDialog>

#include <TerminalTab.h>

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(QWidget *parent, TerminalTab *t);
        ~Settings();

        bool getBlink();
        int getSpeed();
        bool getInherit();

    private slots:
        void changeModel(int m);
        void setColour();

    private:
//        using QDialog::accept;
        void accept();

        TerminalTab *t;

        Ui::Settings *ui;

        QColor palette[8];

        int termType;
        int blinkSpeed;
        bool blink;

};

#endif // SETTINGS_H
