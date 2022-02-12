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
#include "CodePage.h"

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
        Q_OBJECT

    public:

        explicit Settings(ColourTheme *colours, KeyboardTheme *keyboards, CodePage *codepage, QWidget *parent = nullptr);
        ~Settings();

        void showForm(bool connected);

        bool getBlink()                            { return blink; }
        void setBlink(bool blink);

        int getBlinkSpeed()                        { return blinkSpeed; }
        void setBlinkSpeed(int blinkSpeed);

        bool getInherit()                          { return cursorInherit; }
        void setInherit(bool inherit);

        int getTermX();
        int getTermY();
        QString getTermName();

        QFont getFont();
        void setFont(QFont font);

        QString getCodePage();
        void setCodePage(QString codepage);
        CodePage *codePage();

        ColourTheme::Colours getColours();

        bool getFontScaling()                      { return fontScaling; }
        void setFontScaling(bool scale);

        bool getStretch()                          { return stretchScreen; }

        bool getRulerOn()                          { return rulerOn; }
        void setRulerOn(bool rulerOn);

        DisplayScreen::RulerStyle getRulerStyle()  { return ruler; }
        void setRulerStyle(DisplayScreen::RulerStyle r);

        QString getAddress();
        void setAddress(QString address);

        QString getColourTheme()                   { return colourThemeName; }
        QString getKeyboardTheme()                 { return keyboardThemeName; }
        QString getModel()                         { return terms[termType].name; }

        void setTerminalModel(QString model);
        void setTerminalSize(int x, int y);

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
        void codePageChanged();

        void rulerChanged(bool showRuler);
        void rulerStyle(DisplayScreen::RulerStyle r);

        // Emitted when hostname field is not blank
        void connectValid(bool state);

    private slots:

        void changeFont(QFont f);
        void setTerminalModel(int m);

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
        CodePage *codepage;

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
        int formCodePage;

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

};

#endif // SETTINGS_H
