#ifndef KEYBOARDTHEME_H
#define KEYBOARDTHEME_H

#include <QDialog>
#include <QObject>
#include <QMap>
#include <QSettings>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>


namespace Ui {
    class KeyboardTheme;
    class NewTheme;
}

class KeyboardTheme : public QDialog
{
        Q_OBJECT

    public:

        typedef QMap<QString, QStringList> KeyboardMap;

        KeyboardTheme(QWidget *parent = nullptr);

    private:

        Ui::KeyboardTheme *ui;
        Ui::NewTheme *newTheme;

        QDialog newThemePopUp;

        QMap<QString, KeyboardMap> themes;

        QStringList functionList;

        KeyboardMap theme;
        QString currentTheme;
        int currentThemeIndex;

        void setTheme(QString theme);

    private slots:

        void themeChanged(int index);
        void addTheme();
        void deleteTheme();
        void checkDuplicate();

        void accept();

};

#endif // KEYBOARDTHEME_H
