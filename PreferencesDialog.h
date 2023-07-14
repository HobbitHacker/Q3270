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
#include "ActiveSettings.h"

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
        Q_OBJECT

//        Q_ENUM(RulerStyle);

    public:

        explicit PreferencesDialog(ColourTheme &colours, KeyboardTheme &keyboards, ActiveSettings *activeSettings, QWidget *parent = nullptr);
        ~PreferencesDialog();

        void showForm();

        // FIXME: Remove this somehow
        void populateCodePages(QMap<QString, QString> codepagelist);

    signals:

        void tempFontChange(QFont f);

        // Emitted when hostname field is not blank
        // TODO: Is this the right place for this?
        void connectValid(bool state);

    public slots:

        void connected();
        void disconnected();

    private slots:

        void changeFont(QFont f);
        void terminalModelDropDownChanged(int m);

        void colourThemeDropDownChanged(int index);
        void keyboardThemeDropDownChanged(int index);

        void populateColourThemeNames();
        void populateKeyboardThemeNames();

        void manageColourThemes();
        void manageKeyboardThemes();

    private:

        Ui::PreferencesDialog *ui;

        QFontDialog *qfd;

        ColourTheme &colours;
        KeyboardTheme &keyboards;

        QString colourThemeName;
        ColourTheme::Colours colourTheme;

        QString keyboardThemeName;
        KeyboardTheme::KeyboardMap keyboardTheme;

        ActiveSettings *activeSettings;

        QFont qfdFont;

        QHash<ColourTheme::Colour, QPushButton *> colourButtons;

        // Used to populate the combobox with nice names
        QMap<QString, int> comboRulerStyle;

        void accept();
        void reject();

};

#endif // PREFERENCESDIALOG_H
