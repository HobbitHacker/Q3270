#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>

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
        int getBlinkSpeed();
        bool getInherit();
        int getTermX();
        int getTermY();
        QString getTermName();
        QFont getFont();
        QColor *getColours();
        bool getFontScaling();

    signals:

        void terminalChanged(int type, int x, int y);
        void cursorBlinkChanged(bool blink, int blinkSpeed);
        void cursorBlinkSpeedChanged(int blinkSpeed);
        void coloursChanged(QColor palette[8]);
        void fontChanged();
        void fontScalingChanged(bool fontScaling);
        void tempFontChange(QFont f);
        void saveKeyboardSettings();

    private slots:

        void changeFont(QFont f);
        void changeModel(int m);
        void setColour();
        void saveSettings();

    private:
//        using QDialog::accept;
        void accept();
        void reject();

        void changeModel(QString model);
        void changeSize(int x, int y);

        Ui::Settings *ui;

        QFontDialog *qfd;

        QColor palette[8];
        QFont termFont;
        QFont qfdFont;

        int termType;
        int termX;
        int termY;

        int blinkSpeed;
        bool blink;
        bool fontScaling;

        bool paletteChanged;

        struct termTypes
        {
            QString name;
            QString term;
            int x, y;
        };

        termTypes terms[5] = {
            { "Model2", "IBM-3279-2-E", 80, 24 },
            { "Model3", "IBM-3279-3-E", 80, 32 },
            { "Model4", "IBM-3279-4-E", 80, 43 },
            { "Model5", "IBM-3279-5-E", 132, 27 },
            { "Dynamic", "IBM-DYNAMIC", 0, 0}
        };


};

#endif // SETTINGS_H
