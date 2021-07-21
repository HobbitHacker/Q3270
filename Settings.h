#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QMap>

#include "Q3270.h"
#include "ColourTheme.h"
#include "KeyboardTheme.h"
#include "DisplayScreen.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(ColourTheme *colours, KeyboardTheme *keyboards, QWidget *parent = nullptr);
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

        bool getRulerOn()                          { return rulerOn; }
        DisplayScreen::RulerStyle getRulerStyle()  { return ruler; }

        QString getAddress();
        void setAddress(QString address);

        QString getColourTheme()                   { return colourThemeName; }
        QString getKeyboardTheme()                 { return keyboardThemeName; }
        QString getModel()                         { return terms[termType].name; }

    signals:

        void terminalChanged(int type, int x, int y);
        void cursorBlinkChanged(bool blink, int blinkSpeed);
        void cursorBlinkSpeedChanged(int blinkSpeed);
        void coloursChanged(ColourTheme::Colours);
        void fontChanged();
        void setKeyboardTheme(KeyboardTheme::KeyboardMap newTheme);
        void fontScalingChanged(bool fontScaling);
        void tempFontChange(QFont f);
        void setCursorColour(bool inherit);
        void setStretch(bool stretch);

        void rulerChanged(bool showRuler);
        void rulerStyle(DisplayScreen::RulerStyle r);

        // Emitted when hostname field is not blank
        void connectValid(bool state);

    private slots:

        void changeFont(QFont f);
        void changeModel(int m);

        void colourThemeChanged(int index);
        void keyboardThemeChanged(int index);

        void populateColourThemeNames();
        void populateKeyboardThemeNames();

        void manageColourThemes();
        void manageKeyboardThemes();

    private:

        Ui::Settings *ui;

        QFontDialog *qfd;

        ColourTheme *colours;
        KeyboardTheme *keyboards;

        QString colourThemeName;
        ColourTheme::Colours colourTheme;

        QString keyboardThemeName;
        KeyboardTheme::KeyboardMap keyboardTheme;

        QFont termFont;
        QFont qfdFont;

        QHash<ColourTheme::Colour, QPushButton *> colourButtons;

        // Host address parts
        QString hostName;
        int hostPort;
        QString hostLU;

        // Used to populate the combobox with nice names
        QMap<QString, DisplayScreen::RulerStyle> comboRulerStyle;

        int termType;
        int termX;
        int termY;

        // Terminal behaviours
        int blinkSpeed;                     // Speed of cursor blink
        bool blink;                         // Whether cursor blinks
        bool fontScaling;                   // Scale font to Window
        bool cursorInherit;                 // Whether the cursor colour mirrors the character foreground colour
        bool stretchScreen;                 // Whether to stretch the 3270 screen to fit the window
        bool backSpaceStop;                 // Whether backspace stops at the field start position
        bool rulerOn;                       // Whether crosshairs are shown
        DisplayScreen::RulerStyle ruler;    // Style of crosshairs

        bool colourThemeChangeFlag;
        bool keyboardThemeChangeFlag;

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
