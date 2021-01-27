#include "FontSelection.h"

FontSelection::FontSelection(QWidget *parent, QString fontName, QString fontStyle, int fontSize, bool scaling) : QDialog(parent), ui(new Ui::FontSelection)
{
    ui->setupUi(this);

    this->scaling = scaling;
    ui->FontScaling->setCheckState(scaling ? Qt::Checked : Qt::Unchecked);

    initFontList();

    QFont f = fd->font(fontName, fontStyle, fontSize);

    initFontDetails(f.family());

    QList<QListWidgetItem *> ql;

    ql = ui->FontNameList->findItems(f.family(), Qt::MatchExactly);
    if(ql.size() > 0)
    {
        ui->FontNameList->setCurrentItem(ql.first());
        ql = ui->FontStyleList->findItems(f.styleName(),Qt::MatchExactly);
        if(ql.size() > 0)
        {
            ui->FontStyleList->setCurrentItem(ql.first());
        }
        ql = ui->FontSizeList->findItems(QString::number(f.pointSize()),Qt::MatchExactly);
        if (ql.size() > 0)
        {
            ui->FontSizeList->setCurrentItem(ql.first());
        }
    }
}

void FontSelection::initFontList()
{
    fd = new QFontDatabase();

    const QStringList fontFamilies = fd->families();

    for (const QString &family : fontFamilies) {
        ui->FontNameList->addItem(family);
    }

}

void FontSelection::initFontDetails(QString fontname)
{
    const QStringList fontStyles = fd->styles(fontname);
    for (const QString &style : fontStyles)
    {
        ui->FontStyleList->addItem(style);

        const QList<int> smoothSizes = fd->smoothSizes(fontname, style);

        for (int points : smoothSizes)
        {
            printf("FontSelection   : Name %s, StyleName %s, Size %d\n", fontname.toLatin1().data(), style.toLatin1().data(), points);
            fflush(stdout);
            ui->FontSizeList->addItem(QString::number(points));
        }
    }
}

FontSelection::~FontSelection()
{
    delete ui;
}

void FontSelection::fontnameSelected()
{
    QList<QListWidgetItem *> selectedName  = ui->FontNameList->selectedItems();

    QString selectedStyle = "";
    QString selectedSize  = "";

    QList<QListWidgetItem *> ql = ui->FontStyleList->selectedItems();
    if (ql.count() > 0)
    {
        selectedStyle = ql.first()->text();
    }
    ql = ui->FontSizeList->selectedItems();
    if (ql.count() > 0)
    {
        selectedSize = ql.first()->text();
    }

    ui->FontSizeList->clear();
    ui->FontStyleList->clear();

    printf("FontSelection   : font selected %s. Style selected count %d, size selected count %d\n", selectedName.first()->text().toLatin1().data(), selectedStyle.count(), selectedSize.count());
    fflush(stdout);

    initFontDetails(selectedName.first()->text());

    if(selectedStyle != "")
    {
        ql = ui->FontStyleList->findItems(selectedStyle, Qt::MatchExactly);
        if (ql.size() > 0)
        {
            ui->FontStyleList->setCurrentItem(ql.first());
        }
    }

    if (selectedSize != "")
    {
        ql = ui->FontSizeList->findItems(selectedSize, Qt::MatchExactly);
        if (ql.size() > 0)
        {
            ui->FontSizeList->setCurrentItem(ql.first());
        }
    }

    updateSample();
}

void FontSelection::fontstyleSelected()
{
    updateSample();
}

void FontSelection::fontsizeSelected()
{
    updateSample();
}

void FontSelection::fontscalingChanged()
{
    scaling = ui->FontScaling->checkState() == Qt::Checked;
}

bool FontSelection::getScaling()
{
    return scaling;
}


void FontSelection::updateSample()
{
    if (ui->FontNameList->selectedItems().size() > 0 && ui->FontSizeList->selectedItems().size() > 0 && ui->FontStyleList->selectedItems().size() > 0)
    {
        chosenFont = fd->font(ui->FontNameList->selectedItems().first()->text(), ui->FontStyleList->selectedItems().first()->text(), (ui->FontSizeList->selectedItems().first()->text().toInt()));
        ui->SampleTextBox->setFont(chosenFont);
    }
}

void FontSelection::accept()
{
    chosenFont = fd->font(ui->FontNameList->selectedItems().first()->text(), ui->FontStyleList->selectedItems().first()->text(), (ui->FontSizeList->selectedItems().first()->text().toInt()));
    emit setFont("font/name",ui->FontNameList->selectedItems().first()->text());
    emit setFont("font/style", ui->FontStyleList->selectedItems().first()->text());
    emit setFont("font/size", ui->FontSizeList->selectedItems().first()->text());

    QDialog::accept();
}

QFont FontSelection::getFont()
{
    return chosenFont;
}
