#include "ColourTheme.h"
#include "ui_ColourTheme.h"

ColourTheme::ColourTheme(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColourTheme)
{
    ui->setupUi(this);
    ui->colourScheme->clear();

    Colours palette;
    Colours factory;

    // Factory defaults
    palette.base.append(QColor(128,128,255));        /* Basic Blue */
    palette.base.append(QColor(255,0,0));            /* Basic Red */
    palette.base.append(QColor(0,255,0));            /* Basic Green */
    palette.base.append(QColor(255,255,255));        /* Basic White */

    palette.extended.append(QColor(0,0,0));          /* Black */
    palette.extended.append(QColor(128,128,255));    /* Blue */
    palette.extended.append(QColor(255,0,0));        /* Red */
    palette.extended.append(QColor(255,0, 255));     /* Magenta */
    palette.extended.append(QColor(0,255,0));        /* Green */
    palette.extended.append(QColor(0,255,255));      /* Cyan */
    palette.extended.append(QColor(255,255,0));      /* Yellow */
    palette.extended.append(QColor(255,255,255));    /* White */

    schemes.insert("Factory", palette);
    ui->colourScheme->addItem("Factory");

    factory = palette;

    // Populate schemes map with settings from config file
    QSettings s;

    int schemeCount = s.beginReadArray("ColourSchemes");

    for (int sc = 0; sc < schemeCount; sc++)
    {
            s.setArrayIndex(sc);

            QString schemeName = s.value("Name").toString();

            palette.base[UNPROTECTED_NORMAL]      = QColor(s.value("UnprotectedNormal").toString());
            palette.base[UNPROTECTED_INTENSIFIED] = QColor(s.value("UnprotectedIntensified").toString());
            palette.base[PROTECTED_NORMAL]        = QColor(s.value("ProtectedNormal").toString());
            palette.base[PROTECTED_INTENSIFIED]   = QColor(s.value("ProtectedIntensified").toString());

            palette.extended[BLACK]     = QColor(s.value("Black").toString());
            palette.extended[BLUE]      = QColor(s.value("Blue").toString());
            palette.extended[RED]       = QColor(s.value("Red").toString());
            palette.extended[MAGENTA]   = QColor(s.value("Magenta").toString());
            palette.extended[GREEN]     = QColor(s.value("Green").toString());
            palette.extended[CYAN]      = QColor(s.value("Cyan").toString());
            palette.extended[YELLOW]    = QColor(s.value("Yellow").toString());
            palette.extended[NEUTRAL]   = QColor(s.value("Neutral").toString());

            schemes.insert(schemeName, palette);
            ui->colourScheme->addItem(schemeName);
    }

    s.endArray();

    setScheme("Factory");
    currentSchemeIndex = 0;

    // Set colour buttons
    connect(ui->baseProtected, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->baseUnprotectedIntensify, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->baseUnprotected, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->baseProtectedIntensify, &QPushButton::clicked, this, &ColourTheme::setColour);

    connect(ui->colourBlack, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourBlue, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourRed, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourPink, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourGreen, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourTurq, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourYellow, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->colourWhite, &QPushButton::clicked, this, &ColourTheme::setColour);

    // Detect scheme selection change
    connect(ui->colourScheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColourTheme::schemeChanged);

    // Create a pop-up dialog for new scheme names
    QVBoxLayout *vbox = new QVBoxLayout(&newSchemePopUp);
    newSchemePopUpButtons.setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    newSchemeName.setText("New Scheme");
    newSchemeMessage.setText("");
    vbox->addWidget(&newSchemeName);
    vbox->addWidget(&newSchemeMessage);
    vbox->addWidget(&newSchemePopUpButtons);
    newSchemePopUp.setWindowTitle("New Colour Scheme Name");

    // Set up connections for pop-up features
    connect(&newSchemeName, &QLineEdit::textChanged, this, &ColourTheme::checkDuplicate);
    connect(&newSchemePopUpButtons, &QDialogButtonBox::accepted, &newSchemePopUp, &QDialog::accept);
    connect(&newSchemePopUpButtons, &QDialogButtonBox::rejected, &newSchemePopUp, &QDialog::reject);
}

ColourTheme::~ColourTheme()
{
    delete ui;
}

void ColourTheme::setScheme(QString schemeName)
{
    // Set dialog to factory defaults

    qDebug() << "Changing colours to " << schemeName;

    if (schemes.find(schemeName) == schemes.end())
    {
        currentScheme = schemes.constFind("Factory").value();
        currentSchemeName = "Factory";
    }
    else
    {
        currentScheme = schemes.constFind(schemeName).value();
        currentSchemeName = schemeName;
    }

    // Change colour swatches
    ui->colourBlack->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(BLACK).name()));
    ui->colourBlue->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(BLUE).name()));
    ui->colourRed->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(RED).name()));
    ui->colourPink->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(MAGENTA).name()));
    ui->colourGreen->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(GREEN).name()));
    ui->colourTurq->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(CYAN).name()));
    ui->colourYellow->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(YELLOW).name()));
    ui->colourWhite->setStyleSheet(QString("background-color: %1;").arg(currentScheme.extended.at(NEUTRAL).name()));

    ui->baseUnprotected->setStyleSheet(QString("background-color: %1;").arg(currentScheme.base.at(UNPROTECTED_NORMAL).name()));
    ui->baseUnprotectedIntensify->setStyleSheet(QString("background-color: %1;").arg(currentScheme.base.at(UNPROTECTED_INTENSIFIED).name()));
    ui->baseProtected->setStyleSheet(QString("background-color: %1;").arg(currentScheme.base.at(PROTECTED_NORMAL).name()));
    ui->baseProtectedIntensify->setStyleSheet(QString("background-color: %1;").arg(currentScheme.base.at(PROTECTED_INTENSIFIED).name()));

    // Disable editing for Factory scheme
    if (!schemeName.compare("Factory"))
    {
        ui->extendedColours->setEnabled(false);
        ui->baseColours->setEnabled(false);
    }
    else
    {
        ui->extendedColours->setEnabled(true);
        ui->baseColours->setEnabled(true);
    }

}


void ColourTheme::setColour()
{
    // Process a change of colour

    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());

    buttonSender->clearFocus();

    QString button = buttonSender->objectName();

    if (!button.compare("colourBlack"))
    {
        colourDialog(currentScheme.extended[BLACK], buttonSender);
    }
    else if (!button.compare("colourBlue"))
    {
        colourDialog(currentScheme.extended[BLUE], buttonSender);
    }
    else if (!button.compare("colourRed"))
    {
        colourDialog(currentScheme.extended[RED], buttonSender);
    }
    else if (!button.compare("colourPink"))
    {
        colourDialog(currentScheme.extended[MAGENTA], buttonSender);
    }
    else if (!button.compare("colourGreen"))
    {
        colourDialog(currentScheme.extended[GREEN], buttonSender);
    }
    else if (!button.compare("colourTurq"))
    {
        colourDialog(currentScheme.extended[CYAN], buttonSender);
    }
    else if (!button.compare("colourYellow"))
    {
        colourDialog(currentScheme.extended[YELLOW], buttonSender);
    }
    else if (!button.compare("colourWhite"))
    {
        colourDialog(currentScheme.extended[NEUTRAL], buttonSender);
    }
    else if (!button.compare("baseProtected"))
    {
        colourDialog(currentScheme.base[PROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseUnprotectedIntensify"))
    {
        colourDialog(currentScheme.extended[UNPROTECTED_INTENSIFIED], buttonSender);
    }
    else if (!button.compare("baseUnprotected"))
    {
        colourDialog(currentScheme.extended[UNPROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseProtectedIntensify"))
    {
        colourDialog(currentScheme.extended[PROTECTED_INTENSIFIED], buttonSender);
    }

    // Update the map with the new colour
    schemes.find(currentSchemeName).value() = currentScheme;
}

void ColourTheme::colourDialog(QColor &c, QPushButton *b)
{
    // Display colour picker and set scheme colour accordingly, along with button
    const QColor dialogColour = QColorDialog::getColor(c, this, "Select Color");

    // Store colour in lists, and update colour button
    if (dialogColour.isValid())
    {
        qDebug() << "New colour chosen: " << dialogColour.name();

        if (c != dialogColour)
        {
            b->setStyleSheet(QString("background-color: %1;").arg(dialogColour.name()));
            qDebug() << c.name();
            c = dialogColour;
            qDebug() << c.name();
        }
    }
}


void ColourTheme::schemeChanged(int index)
{
    qDebug() << "This index: " << index << " Current Index: " << ui->colourScheme->currentIndex() << " Our Index: " << currentSchemeIndex;

    // When the combobox is changed, update the palette colours
    setScheme(ui->colourScheme->itemText(index));

    // Save the new index
    currentSchemeIndex = index;

    // Disable delete for Factory scheme
    if (currentSchemeIndex == 0)
    {
        ui->colourDelete->setEnabled(false);
    }
    else
    {
        ui->colourDelete->setEnabled(true);
    }
}

void ColourTheme::addScheme()
{
    QString newName = "New Scheme";

    // Create unique name for new scheme
    if (schemes.find(newName) != schemes.end())
    {
        int i = 1;
        while(schemes.find(newName + " " + QString::number(i)) != schemes.end())
        {
            i++;
        }
        newName = "New Scheme " + QString::number(i);
    }

    // Allow user to modify name, disallowing existing names
    if(newSchemePopUp.exec() == QDialog::Accepted)
    {
        // Save scheme name
        newName = newSchemeName.text();
        schemes.insert(newName, currentScheme);

        qDebug() << "Added " << newName << " Total items: " << schemes.count();

        ui->colourScheme->addItem(newName);
        ui->colourScheme->setCurrentIndex(ui->colourScheme->count() - 1);
    }
}

void ColourTheme::checkDuplicate()
{
    // Check if new scheme name being entered is a unique value
    if (schemes.find(newSchemeName.text()) == schemes.end())
    {
        newSchemePopUpButtons.button(QDialogButtonBox::Ok)->setEnabled(true);
        newSchemeMessage.setText("");
    }
    else
    {
        newSchemePopUpButtons.button(QDialogButtonBox::Ok)->setEnabled(false);
        newSchemeMessage.setText("Duplicate scheme name");
    }
}

void ColourTheme::deleteScheme()
{
    // Remove scheme from lists
    schemes.remove(ui->colourScheme->currentText());
    ui->colourScheme->removeItem(currentSchemeIndex);
}
