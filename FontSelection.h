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

        FontSelection(QWidget *parent, QString fontName, QString fontStyle, int fontSize, bool scaling = false);
        FontSelection(QWidget *parent);
        ~FontSelection();
        QFont getFont();
        bool getScaling();

    signals:

        void setFont(QString name, QString value);

    private slots:

        void fontnameSelected();
        void fontstyleSelected();
        void fontsizeSelected();
        void fontscalingChanged();
        void accept();

    private:

        void initFontList();
        void initFontDetails(QString fontname);
        void updateSample();

        Ui::FontSelection *ui;

        QFontDatabase *fd;
        QFont chosenFont;

        bool scaling;
};

#endif // FONTSELECTION_H
