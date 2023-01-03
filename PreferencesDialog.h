#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QMap>

#include "ColourTheme.h"
#include "KeyboardTheme.h"
#include "CodePage.h"
#include "ActiveSettings.h"

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
        Q_OBJECT

//        Q_ENUM(RulerStyle);

    public:

        explicit PreferencesDialog(ColourTheme *colours, KeyboardTheme *keyboards, ActiveSettings *activeSettings, CodePage *codepage, QWidget *parent = nullptr);
        ~PreferencesDialog();

        void showForm(bool connected);

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

        QString getAddress();
        void setAddress(QString address);

        QString getColourTheme()                   { return colourThemeName; }
        QString getKeyboardTheme()                 { return keyboardThemeName; }
        QString getModel()                         { return terms[termType].name; }

        void setTerminalModel(QString model);
        void setTerminalSize(int x, int y);

    signals:

        void terminalChanged(int type, int x, int y);
        void coloursChanged(ColourTheme::Colours);
        void fontChanged();
        void setKeyboardTheme(KeyboardTheme::KeyboardMap newTheme);
        void fontScalingChanged(bool fontScaling);
        void tempFontChange(QFont f);
        void setStretch(bool stretch);
        void codePageChanged();

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

        Ui::PreferencesDialog *ui;

        QFontDialog *qfd;

        ColourTheme *colours;
        KeyboardTheme *keyboards;
        CodePage *codepage;

        QString colourThemeName;
        ColourTheme::Colours colourTheme;

        QString keyboardThemeName;
        KeyboardTheme::KeyboardMap keyboardTheme;

        ActiveSettings *activeSettings;

        QFont termFont;
        QFont qfdFont;

        QHash<ColourTheme::Colour, QPushButton *> colourButtons;

        // Host address parts
        QString hostName;
        int hostPort;
        QString hostLU;

        // Used to populate the combobox with nice names
        QMap<QString, int> comboRulerStyle;

        int termType;
        int termX;
        int termY;

        // Terminal behaviours
        bool fontScaling;                   // Scale font to Window

        bool stretchScreen;                 // Whether to stretch the 3270 screen to fit the window
        bool backSpaceStop;                 // Whether backspace stops at the field start position

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

#endif // PREFERENCESDIALOG_H
