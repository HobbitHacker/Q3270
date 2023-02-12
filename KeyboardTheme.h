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
#include <QTableWidgetItem>


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

        QStringList getThemes();
        KeyboardMap getTheme(QString keyboardThemeName);
        void populateTable(QTableWidget *table, QString mapName);

        int exec();


    private:

        Ui::KeyboardTheme *ui;
        Ui::NewTheme *newTheme;

        QDialog newThemePopUp;

        QMap<QString, KeyboardMap> themes;

        QStringList functionList;

        KeyboardMap theme;
        QString currentTheme;
        int currentThemeIndex;

        int lastRow;
        int lastSeq;

        // Variables used to store state, to be restored should the user press cancel
        QString restoreTheme;
        QMap<QString, KeyboardMap> restoreThemes;
        int restoreThemeIndex;

        void setTheme(QString theme);

    private slots:

        void themeChanged(int index);
        void addTheme();
        void deleteTheme();
        void checkDuplicate();
        void populateKeySequence(QTableWidgetItem *item);
        void setKey();
        void truncateShortcut();

        void accept();
        void reject();

};

#endif // KEYBOARDTHEME_H
