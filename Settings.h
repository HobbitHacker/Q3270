#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTableWidgetItem>

#include "Q3270.h"
#include "ColourTheme.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(QWidget *parent, ColourTheme *colours);
        ~Settings();

        void showForm(bool connected);

        bool getBlink();
        int getBlinkSpeed();
        bool getInherit();
        int getTermX();
        int getTermY();
        QString getTermName();
        QFont getFont();
        ColourTheme::Colours getColours();
        bool getFontScaling();
        bool getStretch();
        void setKeyboardMap(QMap<QString, QStringList> map);

    signals:

        void terminalChanged(int type, int x, int y);
        void cursorBlinkChanged(bool blink, int blinkSpeed);
        void cursorBlinkSpeedChanged(int blinkSpeed);
        void coloursChanged(ColourTheme::Colours);
        void fontChanged();
        void newMap(QMap<QString, QStringList> newMap);
        void fontScalingChanged(bool fontScaling);
        void tempFontChange(QFont f);
        void saveKeyboardSettings();
        void setCursorColour(bool inherit);
        void setStretch(bool stretch);

    private slots:

        void changeFont(QFont f);
        void changeModel(int m);
        void populateKeySequence(QTableWidgetItem *item);
        void setKey();
        void truncateShortcut();
        void saveSettings();
        void colourSchemeChanged(int index);
        void populateSchemeNames();

    private:

        Ui::Settings *ui;

        QFontDialog *qfd;

        ColourTheme *colours;

        QString colourSchemeName;
        ColourTheme::Colours colourScheme;

        QFont termFont;
        QFont qfdFont;

        QHash<ColourTheme::Colour, QPushButton *> colourButtons;

        QMap<QString, QStringList> keyboardMap;

        int termType;
        int termX;
        int termY;

        int blinkSpeed;
        bool blink;
        bool fontScaling;
        bool cursorInherit;
        bool stretchScreen;

        bool paletteChanged;
        bool keyboardChanged;

        int lastRow;
        int lastSeq;

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

        void accept();
        void reject();

        void changeModel(QString model);
        void changeSize(int x, int y);

};

#endif // SETTINGS_H
