#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

#include <Terminal.h>

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(QWidget *parent, Terminal *t);
        ~Settings();

        bool getBlink();
        int getSpeed();
        bool getInherit();

    private slots:
        void changeModel(int m);


    private:
//        using QDialog::accept;
        void accept();

        Terminal *t;

        Ui::Settings *ui;

        int termType;
        int blinkSpeed;
        bool blink;

};

#endif // SETTINGS_H
