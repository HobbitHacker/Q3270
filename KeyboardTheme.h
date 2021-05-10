#ifndef KEYBOARDTHEME_H
#define KEYBOARDTHEME_H

#include <QDialog>
#include <QObject>
#include <QMap>


namespace Ui {
    class KeyboardTheme;
}

class KeyboardTheme : public QDialog
{
        Q_OBJECT

    public:

        typedef QMap<QString, QStringList> KeyboardMap;

        KeyboardTheme();

    private:

        Ui::KeyboardTheme *ui;

        QMap<QString, KeyboardMap> themes;

        KeyboardMap theme;
};

#endif // KEYBOARDTHEME_H
