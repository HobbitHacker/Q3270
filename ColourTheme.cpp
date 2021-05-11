#include "ColourTheme.h"
#include "ui_ColourTheme.h"
#include "ui_NewTheme.h"

ColourTheme::ColourTheme(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColourTheme)
{
    ui->setupUi(this);
    ui->colourTheme->clear();

    Colours palette;

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

    // Add the factory theme to the list
    themes.insert("Factory", palette);

    // Set the default colour theme to Factory
    ui->colourTheme->addItem("Factory");

    // Populate Themes map with settings from config file
    QSettings s;

    // Begin Themes main group
    s.beginGroup("ColourThemes");

    // Get Themes, and sort them
    QStringList ThemeList = s.childGroups();
    ThemeList.sort(Qt::CaseSensitive);

    // Populate Themes list and combo boxes from config file
    for (int sc = 0; sc < ThemeList.count(); sc++)
    {
        qDebug() << ThemeList.at(sc);

        // Ignore Factory theme (shouldn't be present, but in case of accidents, or user fudging)
        if (ThemeList.at(sc).compare("Factory"))
        {
            qDebug() << "Storing " << ThemeList.at(sc);
            // Begin theme specific group
            s.beginGroup(ThemeList.at(sc));

            // Base colours
            palette[UNPROTECTED_NORMAL]      = QColor(s.value("UnprotectedNormal").toString());
            palette[UNPROTECTED_INTENSIFIED] = QColor(s.value("UnprotectedIntensified").toString());
            palette[PROTECTED_NORMAL]        = QColor(s.value("ProtectedNormal").toString());
            palette[PROTECTED_INTENSIFIED]   = QColor(s.value("ProtectedIntensified").toString());

            // Extended Colours
            palette[BLACK]     = QColor(s.value("Black").toString());
            palette[BLUE]      = QColor(s.value("Blue").toString());
            palette[RED]       = QColor(s.value("Red").toString());
            palette[MAGENTA]   = QColor(s.value("Magenta").toString());
            palette[GREEN]     = QColor(s.value("Green").toString());
            palette[CYAN]      = QColor(s.value("Cyan").toString());
            palette[YELLOW]    = QColor(s.value("Yellow").toString());
            palette[NEUTRAL]   = QColor(s.value("Neutral").toString());

            // Save theme
            themes.insert(ThemeList.at(sc), palette);
            ui->colourTheme->addItem(ThemeList.at(sc));

            // End theme specific group
            s.endGroup();
        }
    }

    // End Themes main group
    s.endGroup();

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

    // Detect theme selection change
    connect(ui->colourTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColourTheme::themeChanged);

    // Create a pop-up dialog for new theme names
    newTheme = new Ui::NewTheme;
    newTheme->setupUi(&newThemePopUp);

    // Set up connections for pop-up features
    connect(newTheme->newName, &QLineEdit::textChanged, this, &ColourTheme::checkDuplicate);
    connect(newTheme->buttonBox, &QDialogButtonBox::accepted, &newThemePopUp, &QDialog::accept);
    connect(newTheme->buttonBox, &QDialogButtonBox::rejected, &newThemePopUp, &QDialog::reject);

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

    qDebug() << themes.keys();

    setTheme("Factory");

    currentThemeIndex = 0;

    for(int i = 0; i < ui->colourTheme->count(); i++)
    {
        qDebug() << "At " << i << " " << ui->colourTheme->itemText(i);
    }
}

ColourTheme::~ColourTheme()
{
    qDebug() << "ColourTheme: Destroyed";
    delete ui;
}

int ColourTheme::exec()
{
    for(int i = 0; i < ui->colourTheme->count(); i++)
    {
        qDebug() << "At " << i << " " << ui->colourTheme->itemText(i);
    }

    return QDialog::exec();
}

void ColourTheme::setTheme(QString ThemeName)
{
    // Set colour theme according to ThemeName, using Factory if not found
    qDebug() << "Changing colours to " << ThemeName;

    if (themes.find(ThemeName) == themes.end())
    {
        currentTheme = themes.constFind("Factory").value();
        currentThemeName = "Factory";
    }
    else
    {
        currentTheme = themes.constFind(ThemeName).value();
        currentThemeName = ThemeName;
    }

    // Disable editing for Factory theme
    if (!ThemeName.compare("Factory"))
    {
        ui->extendedColours->setEnabled(false);
        ui->baseColours->setEnabled(false);
    }
    else
    {
        ui->extendedColours->setEnabled(true);
        ui->baseColours->setEnabled(true);
    }

    setButtonColours(currentTheme, colourButtons);
}

void ColourTheme::setButtonColours(Colours theme, QHash<Colour, QPushButton *> buttons)
{
    // Change colour swatches
    buttons[UNPROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(theme[UNPROTECTED_NORMAL].name()));
    buttons[PROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(theme[PROTECTED_NORMAL].name()));
    buttons[UNPROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(theme[UNPROTECTED_INTENSIFIED].name()));
    buttons[PROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(theme[PROTECTED_INTENSIFIED].name()));

    buttons[BLACK]->setStyleSheet(QString("background-color: %1;").arg(theme[BLACK].name()));
    buttons[BLUE]->setStyleSheet(QString("background-color: %1;").arg(theme[BLUE].name()));
    buttons[RED]->setStyleSheet(QString("background-color: %1;").arg(theme[RED].name()));
    buttons[MAGENTA]->setStyleSheet(QString("background-color: %1;").arg(theme[MAGENTA].name()));
    buttons[GREEN]->setStyleSheet(QString("background-color: %1;").arg(theme[GREEN].name()));
    buttons[CYAN]->setStyleSheet(QString("background-color: %1;").arg(theme[CYAN].name()));
    buttons[YELLOW]->setStyleSheet(QString("background-color: %1;").arg(theme[YELLOW].name()));
    buttons[NEUTRAL]->setStyleSheet(QString("background-color: %1;").arg(theme[NEUTRAL].name()));
}


void ColourTheme::setColour()
{
    // Process a change of colour

    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());

    buttonSender->clearFocus();

    QString button = buttonSender->objectName();

    if (!button.compare("colourBlack"))
    {
        colourDialog(currentTheme[BLACK], buttonSender);
    }
    else if (!button.compare("colourBlue"))
    {
        colourDialog(currentTheme[BLUE], buttonSender);
    }
    else if (!button.compare("colourRed"))
    {
        colourDialog(currentTheme[RED], buttonSender);
    }
    else if (!button.compare("colourPink"))
    {
        colourDialog(currentTheme[MAGENTA], buttonSender);
    }
    else if (!button.compare("colourGreen"))
    {
        colourDialog(currentTheme[GREEN], buttonSender);
    }
    else if (!button.compare("colourTurq"))
    {
        colourDialog(currentTheme[CYAN], buttonSender);
    }
    else if (!button.compare("colourYellow"))
    {
        colourDialog(currentTheme[YELLOW], buttonSender);
    }
    else if (!button.compare("colourWhite"))
    {
        colourDialog(currentTheme[NEUTRAL], buttonSender);
    }
    else if (!button.compare("baseProtected"))
    {
        colourDialog(currentTheme[PROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseUnprotectedIntensify"))
    {
        colourDialog(currentTheme[UNPROTECTED_INTENSIFIED], buttonSender);
    }
    else if (!button.compare("baseUnprotected"))
    {
        colourDialog(currentTheme[UNPROTECTED_NORMAL], buttonSender);
    }
    else if (!button.compare("baseProtectedIntensify"))
    {
        colourDialog(currentTheme[PROTECTED_INTENSIFIED], buttonSender);
    }

    // Update the map with the new colour
    themes.find(currentThemeName).value() = currentTheme;
}

void ColourTheme::colourDialog(QColor &c, QPushButton *b)
{
    // Display colour picker and set theme colour accordingly, along with button
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


void ColourTheme::themeChanged(int index)
{
    qDebug() << "This index: " << index << " Current Index: " << ui->colourTheme->currentIndex() << " Our Index: " << currentThemeIndex;

    // When the combobox is changed, update the palette colours
    setTheme(ui->colourTheme->itemText(index));

    // Save the new index
    currentThemeIndex = index;

    // Disable delete for Factory theme
    if (currentThemeIndex == 0)
    {
        ui->colourDelete->setEnabled(false);
    }
    else
    {
        ui->colourDelete->setEnabled(true);
    }
}

void ColourTheme::addTheme()
{
    QString newName = "New Theme";

    // Create unique name for new theme
    if (themes.find(newName) != themes.end())
    {
        int i = 1;
        while(themes.find(newName + " " + QString::number(i)) != themes.end())
        {
            i++;
        }
        newName = "New Theme " + QString::number(i);
    }

    // Allow user to modify name, disallowing existing names
    if(newThemePopUp.exec() == QDialog::Accepted)
    {
        // Save theme name
        newName = newTheme->newName->text();
        themes.insert(newName, currentTheme);

        qDebug() << "Added " << newName << " Total items: " << themes.count();

        ui->colourTheme->addItem(newName);
        ui->colourTheme->setCurrentIndex(ui->colourTheme->count() - 1);
    }
}

void ColourTheme::checkDuplicate()
{
    // Check if new theme name being entered is a unique value
    if (themes.find(newTheme->newName->text()) == themes.end())
    {
        newTheme->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        newTheme->message->setText("");
    }
    else
    {
        newTheme->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        newTheme->message->setText("Duplicate theme name");
    }
}

void ColourTheme::deleteTheme()
{
    // Remove theme from lists
    themes.remove(ui->colourTheme->currentText());
    ui->colourTheme->removeItem(currentThemeIndex);
}


const ColourTheme::Colours ColourTheme::getTheme(QString theme)
{
    // Return requested theme, if found, or Factory theme if not
    if (themes.find(theme) == themes.end())
    {
        return themes.constFind("Factory").value();
    }

    return themes.constFind(theme).value();
}

QList<QString> ColourTheme::getThemes()
{
    // Return all the Themes
    return themes.keys();
}

void ColourTheme::accept()
{
    // Save settings
    QSettings settings;

    // Group for Colours
    settings.beginGroup("ColourThemes");

    // Clear any existing settings
    settings.remove("");

    QMap<QString, QMap<ColourTheme::Colour, QColor>>::const_iterator i = themes.constBegin();

    while(i != themes.constEnd())
    {
        // Skip Factory theme
        if (i.key().compare("Factory"))
        {
            // Start new group for each theme
            settings.beginGroup(i.key());

            Colours c = i.value();

            // Save base colours
            settings.setValue("UnprotectedNormal", c[UNPROTECTED_NORMAL].name());
            settings.setValue("UnprotectedIntensified", c[UNPROTECTED_INTENSIFIED].name());
            settings.setValue("ProtectedNormal", c[PROTECTED_NORMAL].name());
            settings.setValue("ProtectedIntensified", c[PROTECTED_INTENSIFIED].name());

            // Save Extended colours
            settings.setValue("Black", c[BLACK].name());
            settings.setValue("Blue", c[BLUE].name());
            settings.setValue("Red", c[RED].name());
            settings.setValue("Magenta", c[MAGENTA].name());
            settings.setValue("Green", c[GREEN].name());
            settings.setValue("Cyan", c[CYAN].name());
            settings.setValue("Yellow", c[YELLOW].name());
            settings.setValue("Neutral", c[NEUTRAL].name());

            // End Theme group
            settings.endGroup();
        }

        // Next theme
        i++;

    }

    // Finish ColourThemes group
    settings.endGroup();

    QDialog::accept();
}
