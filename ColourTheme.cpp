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
    palette[PROTECTED_NORMAL]        = QColor(128,128,255);        /* Basic Blue */
    palette[UNPROTECTED_INTENSIFIED] = QColor(255,0,0);            /* Basic Red */
    palette[UNPROTECTED_NORMAL]      = QColor(0,255,0);            /* Basic Green */
    palette[PROTECTED_INTENSIFIED]   = QColor(255,255,255);        /* Basic White */

    palette[BLACK]   = QColor(0,0,0);          /* Black */
    palette[BLUE]    = QColor(128,128,255);    /* Blue */
    palette[RED]     = QColor(255,0,0);        /* Red */
    palette[MAGENTA] = QColor(255,0, 255);     /* Magenta */
    palette[GREEN]   = QColor(0,255,0);        /* Green */
    palette[CYAN]    = QColor(0,255,255);      /* Cyan */
    palette[YELLOW]  = QColor(255,255,0);      /* Yellow */
    palette[NEUTRAL] = QColor(255,255,255);    /* White */

    schemes.insert("Factory", palette);

    factory = palette;

    // Populate schemes map with settings from config file
    QSettings s;

    int schemeCount = s.beginReadArray("ColourSchemes");

    for (int sc = 0; sc < schemeCount; sc++)
    {
            s.setArrayIndex(sc);

            QString schemeName = s.value("Name").toString();

            palette[UNPROTECTED_NORMAL]      = QColor(s.value("UnprotectedNormal").toString());
            palette[UNPROTECTED_INTENSIFIED] = QColor(s.value("UnprotectedIntensified").toString());
            palette[PROTECTED_NORMAL]        = QColor(s.value("ProtectedNormal").toString());
            palette[PROTECTED_INTENSIFIED]   = QColor(s.value("ProtectedIntensified").toString());

            palette[BLACK]     = QColor(s.value("Black").toString());
            palette[BLUE]      = QColor(s.value("Blue").toString());
            palette[RED]       = QColor(s.value("Red").toString());
            palette[MAGENTA]   = QColor(s.value("Magenta").toString());
            palette[GREEN]     = QColor(s.value("Green").toString());
            palette[CYAN]      = QColor(s.value("Cyan").toString());
            palette[YELLOW]    = QColor(s.value("Yellow").toString());
            palette[NEUTRAL]   = QColor(s.value("Neutral").toString());

            schemes.insert(schemeName, palette);
            ui->colourScheme->addItem(schemeName);
    }

    s.endArray();

    // Set colour buttons actions
    connect(ui->baseUnprotected, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->baseProtected, &QPushButton::clicked, this, &ColourTheme::setColour);
    connect(ui->baseUnprotectedIntensify, &QPushButton::clicked, this, &ColourTheme::setColour);
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

    // Set up a list of buttons for use in setButtonColours
    colourButtons[UNPROTECTED_NORMAL]      = ui->baseUnprotected;
    colourButtons[PROTECTED_NORMAL]        = ui->baseProtected;
    colourButtons[UNPROTECTED_INTENSIFIED] = ui->baseUnprotectedIntensify;
    colourButtons[PROTECTED_INTENSIFIED]   =  ui->baseProtectedIntensify;

    colourButtons[BLACK]        = ui->colourBlack;
    colourButtons[BLUE]         = ui->colourBlue;
    colourButtons[RED]          = ui->colourRed;
    colourButtons[MAGENTA]      = ui->colourPink;
    colourButtons[GREEN]        = ui->colourGreen;
    colourButtons[CYAN]         = ui->colourTurq;
    colourButtons[YELLOW]       = ui->colourYellow;
    colourButtons[NEUTRAL]      = ui->colourWhite;

    // Set the default colour scheme to Factory
    ui->colourScheme->addItem("Factory");

    setScheme("Factory");

    currentSchemeIndex = 0;

}

ColourTheme::~ColourTheme()
{
    delete ui;
}

void ColourTheme::setScheme(QString schemeName)
{
    // Set colour theme according to schemeName, using Factory if not found
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

    setButtonColours(currentScheme, colourButtons);
}

void ColourTheme::setButtonColours(Colours scheme, QHash<Colour, QPushButton *> buttons)
{
    // Change colour swatches
    buttons[UNPROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(scheme[UNPROTECTED_NORMAL].name()));
    buttons[PROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(scheme[UNPROTECTED_INTENSIFIED].name()));
    buttons[UNPROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(scheme[PROTECTED_NORMAL].name()));
    buttons[PROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(scheme[PROTECTED_INTENSIFIED].name()));

    buttons[BLACK]->setStyleSheet(QString("background-color: %1;").arg(scheme[BLACK].name()));
    buttons[BLUE]->setStyleSheet(QString("background-color: %1;").arg(scheme[BLUE].name()));
    buttons[RED]->setStyleSheet(QString("background-color: %1;").arg(scheme[RED].name()));
    buttons[MAGENTA]->setStyleSheet(QString("background-color: %1;").arg(scheme[MAGENTA].name()));
    buttons[GREEN]->setStyleSheet(QString("background-color: %1;").arg(scheme[GREEN].name()));
    buttons[CYAN]->setStyleSheet(QString("background-color: %1;").arg(scheme[CYAN].name()));
    buttons[YELLOW]->setStyleSheet(QString("background-color: %1;").arg(scheme[YELLOW].name()));
    buttons[NEUTRAL]->setStyleSheet(QString("background-color: %1;").arg(scheme[NEUTRAL].name()));
}


void ColourTheme::setColour()
{
    // Process a change of colour

    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());

    buttonSender->clearFocus();

    QString button = buttonSender->objectName();

    if (!button.compare("colourBlack"))
    {
        colourDialog(currentScheme[BLACK], buttonSender);
    }
    else if (!button.compare("colourBlue"))
    {
        colourDialog(currentScheme[BLUE], buttonSender);
    }
    else if (!button.compare("colourRed"))
    {
        colourDialog(currentScheme[RED], buttonSender);
    }
    else if (!button.compare("colourPink"))
    {
        colourDialog(currentScheme[MAGENTA], buttonSender);
    }
    else if (!button.compare("colourGreen"))
    {
        colourDialog(currentScheme[GREEN], buttonSender);
    }
    else if (!button.compare("colourTurq"))
    {
        colourDialog(currentScheme[CYAN], buttonSender);
    }
    else if (!button.compare("colourYellow"))
    {
        colourDialog(currentScheme[YELLOW], buttonSender);
    }
    else if (!button.compare("colourWhite"))
    {
        colourDialog(currentScheme[NEUTRAL], buttonSender);
    }
    else if (!button.compare("baseProtected"))
    {
        colourDialog(currentScheme[PROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseUnprotectedIntensify"))
    {
        colourDialog(currentScheme[UNPROTECTED_INTENSIFIED], buttonSender);
    }
    else if (!button.compare("baseUnprotected"))
    {
        colourDialog(currentScheme[UNPROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseProtectedIntensify"))
    {
        colourDialog(currentScheme[PROTECTED_INTENSIFIED], buttonSender);
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


const ColourTheme::Colours ColourTheme::getScheme(QString scheme)
{
    // Return requested scheme, if found, or Factory scheme if not
    if (schemes.find(scheme) == schemes.end())
    {
        return schemes.constFind("Factory").value();
    }

    return schemes.constFind(scheme).value();
}

QList<QString> ColourTheme::getSchemes()
{
    // Return all the schemes
    return schemes.keys();
}
