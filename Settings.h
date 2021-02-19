#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>

#include "3270.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(QWidget *parent);
        ~Settings();

        void showForm(bool connected);

        bool getBlink();
        int getSpeed();
        bool getInherit();
        int getTermX();
        int getTermY();
        QString getTermName();
        QFont getFont();
        QColor *getColours();

    signals:

        void terminalChanged(int type, int x, int y);
        void cursorBlinkChanged(bool blink, int blinkSpeed);
        void coloursChanged(QColor palette[8]);

    private slots:

        void changeModel(int m);
        void setColour();

    private:
//        using QDialog::accept;
        void accept();

        void changeModel(QString model);
        void changeSize(int x, int y);

        Ui::Settings *ui;

        QColor palette[8];
        QFont termFont;

        int termType;
        int termX;
        int termY;

        int blinkSpeed;
        bool blink;

        bool paletteChanged;

        struct termTypes
        {
            QString term;
            int x, y;
        };

        termTypes terms[5] = {
            { "IBM-3279-2-E", 80, 24 },
            { "IBM-3279-3-E", 80, 32 },
            { "IBM-3279-4-E", 80, 43 },
            { "IBM-3279-5-E", 132, 27 },
            { "IBM-DYNAMIC", 0, 0}
        };


};

#endif // SETTINGS_H
