#ifndef FONTSELECTION_H
#define FONTSELECTION_H

#include "ui_FontSelection.h"
#include <QDialog>
#include <QFontDatabase>
#include <QPushButton>

namespace Ui {
    class FontSelection;
}

class FontSelection : public QDialog
{
        Q_OBJECT

    public:

        FontSelection(QWidget *parent, QString fontName, QString fontStyle, int fontSize);
        FontSelection(QWidget *parent);
        ~FontSelection();
        QFont getFont();

    signals:

        void setFont(QString name, QString value);

    private slots:

        void fontnameSelected();
        void accept();


    private:

        void initFontList();
        void initFontDetails(QString fontname);

        Ui::FontSelection *ui;

        QFontDatabase *fd;
        QFont chosenFont;
};

#endif // FONTSELECTION_H
