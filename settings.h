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

    private slots:
        void changeModel(int m);

    private:
//        using QDialog::accept;
        void accept();

        Terminal *t;

        Ui::Settings *ui;

        int termType;

};

#endif // SETTINGS_H
