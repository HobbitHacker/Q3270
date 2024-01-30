#include "Q3270.h"
#include "ColourTheme.h"
#include "ui_ColourTheme.h"
#include "ui_NewTheme.h"

/**
 * @brief   ColourTheme::ColourTheme - Dialog for choosing the colours for 3270
 * @param   parent
 *
 * @details ColourTheme is used to display the swatches for the different colours used by
 *          the 3270 display.
 *
 *          There are two sets of colours; the basic 4 colour (blue, green, red, white), and
 *          the standard 7 colour (black, blue, red, magenta or pink, green, cyan or turquoise,
 *          yellow and white or neutral).
 *
 *          The theme names are picked up from the config file; Factory is always present
 *          internally, and ignored if also found in the config file. These are used to build up
 *          a list of available themes.
 */
ColourTheme::ColourTheme(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColourTheme)
{
    ui->setupUi(this);
    ui->colourTheme->clear();

    Colours palette;

    // Factory defaults
    palette[UNPROTECTED_NORMAL]      = QColor(0,255,0);            /* Basic Green */
    palette[PROTECTED_NORMAL]        = QColor(128,128,255);        /* Basic Blue */
    palette[UNPROTECTED_INTENSIFIED] = QColor(255,0,0);            /* Basic Red */
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

    QSettings settings(Q3270_SETTINGS);

    // Begin Themes main group
    settings.beginGroup("ColourThemes");

    // Get Themes, and sort them
    QStringList ThemeList = settings.childGroups();
    ThemeList.sort(Qt::CaseSensitive);

    qDebug() << "ColourTheme Settings:" << settings.organizationName() << "," << settings.applicationName();
    qDebug() << "Groups: " << settings.childGroups();
    qDebug() << "Keys: " << settings.childKeys();

    // Populate Themes list and combo boxes from config file
    for (int sc = 0; sc < ThemeList.count(); sc++)
    {
        // Ignore Factory theme (shouldn't be present, but in case of accidents, or user fudging)
        if (ThemeList.at(sc).compare("Factory"))
        {
            qDebug() << "Storing " << ThemeList.at(sc);
            // Begin theme specific group
            settings.beginGroup(ThemeList.at(sc));

            // Base colours
            palette[UNPROTECTED_NORMAL]      = QColor(settings.value("UnprotectedNormal").toString());
            palette[UNPROTECTED_INTENSIFIED] = QColor(settings.value("UnprotectedIntensified").toString());
            palette[PROTECTED_NORMAL]        = QColor(settings.value("ProtectedNormal").toString());
            palette[PROTECTED_INTENSIFIED]   = QColor(settings.value("ProtectedIntensified").toString());

            // Extended Colours
            palette[BLACK]     = QColor(settings.value("Black").toString());
            palette[BLUE]      = QColor(settings.value("Blue").toString());
            palette[RED]       = QColor(settings.value("Red").toString());
            palette[MAGENTA]   = QColor(settings.value("Magenta").toString());
            palette[GREEN]     = QColor(settings.value("Green").toString());
            palette[CYAN]      = QColor(settings.value("Cyan").toString());
            palette[YELLOW]    = QColor(settings.value("Yellow").toString());
            palette[NEUTRAL]   = QColor(settings.value("Neutral").toString());

            // Save theme
            themes.insert(ThemeList.at(sc), palette);
            ui->colourTheme->addItem(ThemeList.at(sc));

            // End theme specific group
            settings.endGroup();
        }
    }

    // End Themes main group
    settings.endGroup();

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
}

/**
 * @brief   ColourTheme::~ColourTheme - destructor
 *
 * @details Destructor.
 */
ColourTheme::~ColourTheme()
{
    qDebug() << "ColourTheme: Destroyed";
    delete ui;
}

/**
 * @brief   ColourTheme::exec - dialog display.
 * @return  OK or Cancel
 */
int ColourTheme::exec()
{
    // Save state in the event of a cancel
    restoreThemes = themes;
    restoreThemeName = currentThemeName;
    restoreThemeIndex = currentThemeIndex;

    return QDialog::exec();
}

/**
 * @brief   ColourTheme::setTheme - set colour theme
 * @param   ThemeName - string name of theme to be set
 *
 * @details setTheme changes the theme to the one specified, changing the colours on the
 *          swatches to match. If the passed theme name isn't found in the list, the Factory
 *          internal one is used instead.
 */
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

    setButtonColours(currentThemeName);
}

/**
 * @brief   ColourTheme::setButtonColours - set the swatches to the colours specified
 * @param   themeName - the name of theme
 *
 * @details setButtonColours is used to change the colours on the swatches of the dialog.
 */
void ColourTheme::setButtonColours(QString themeName)
{
    Colours thisTheme = getTheme(themeName);

    // Change colour swatches
    colourButtons[UNPROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[UNPROTECTED_NORMAL].name()));
    colourButtons[PROTECTED_NORMAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[PROTECTED_NORMAL].name()));
    colourButtons[UNPROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[UNPROTECTED_INTENSIFIED].name()));
    colourButtons[PROTECTED_INTENSIFIED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[PROTECTED_INTENSIFIED].name()));

    colourButtons[BLACK]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[BLACK].name()));
    colourButtons[BLUE]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[BLUE].name()));
    colourButtons[RED]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[RED].name()));
    colourButtons[MAGENTA]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[MAGENTA].name()));
    colourButtons[GREEN]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[GREEN].name()));
    colourButtons[CYAN]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[CYAN].name()));
    colourButtons[YELLOW]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[YELLOW].name()));
    colourButtons[NEUTRAL]->setStyleSheet(QString("background-color: %1;").arg(thisTheme[NEUTRAL].name()));
}

/**
 * @brief   ColourTheme::setColour - the slot called when the user clicks on a swatch
 *
 * @details setColour is called when the swatch button is clicked, which enables the user to
 *          modify the colour. As every swatch is connected to this slot, it compares the
 *          button name to known values to display the colour selection dialog with that colour
 *          selected.
 */
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

/**
 * @brief   ColourTheme::colourDialog - display the colour collection dialog
 * @param   c - the original colour of the button
 * @param   b - the button that was pressed
 *
 * @details colourDialog presents the user with a colour selection dialog, and will
 *          update the colour of the button that was pressed when the OK button is pressed.
 */
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


/**
 * @brief   ColourTheme::themeChanged - update the colours when the theme is changed
 * @param   index - the index of the selected theme
 *
 * @details themeChanged is triggered when the drop-down box of theme names is updated to show
 *          a different theme. If the theme chosen is not the internal Factory theme, the theme
 *          can be deleted.
 */
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

/**
 * @brief   ColourTheme::addTheme - a pop-up dialog box to create a new colour theme
 *
 * @details addTheme displays a dialog box to the user allowing them to enter a new name for
 *          a new theme. The default name of the new theme is 'New Theme' and a number, increasing by
 *          one for each "New Theme n" that is already present.
 *
 *          When the OK button is pressed, the new theme is added to the list of selectable
 *          themes.
 */
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

/**
 * @brief   ColourTheme::checkDuplicate - check for duplicate theme names when adding a new theme
 *
 * @details checkDuplicate is triggered when the user modifies the input box for the new theme name.
 *          When the theme name does not match any existing theme name, the OK button is enabled,
 *          otherwise a "Duplicate theme name" message is shown.
 */
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

/**
 * @brief   ColourTheme::deleteTheme - remove a theme
 *
 * @details deleteTheme removes an entry from the theme lists. It is triggered by the delete button.
 */
void ColourTheme::deleteTheme()
{
    // Remove theme from lists
    themes.remove(ui->colourTheme->currentText());
    ui->colourTheme->removeItem(currentThemeIndex);
}

/**
 * @brief   ColourTheme::getThemes - return a list of theme names
 * @return  the list of available theme names
 *
 * @details Extract a list of available colour themes; the Factory theme is always first.
 */
QList<QString> ColourTheme::getThemes()
{
    QList<QString> tk = themes.keys();

    // Factory theme should always be at the top
    tk.removeAt(tk.indexOf("Factory"));
    tk.prepend("Factory");

    return tk;
}

/**
 * @brief   ColourTheme::getTheme - return the colour theme identified by theme
 * @param   theme - the colour theme to be returned
 * @return  the list of colours
 *
 * @details getTheme is called when the colour theme is changed.
 */
const ColourTheme::Colours ColourTheme::getTheme(QString theme)
{
    // Return requested theme, if found, or Factory theme if not
    if (themes.find(theme) == themes.end())
    {
        return themes.constFind("Factory").value();
    }

    return themes.constFind(theme).value();
}

/**
 * @brief   ColourTheme::accept - save colour theme changes
 *
 * @details accept is triggered by the OK button, and saves the colour themes to the config file.
 *          The internal Factory theme is skipped.
 */
void ColourTheme::accept()
{

    QSettings settings(Q3270_SETTINGS);

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

/**
 * @brief   ColourTheme::reject - revert any changes made
 *
 * @details reject is triggered when the cancel button is pressed to undo any changes the user has done.
 */
void ColourTheme::reject()
{
    // Restore state to before dialog displayed
    themes = restoreThemes;
    currentThemeName = restoreThemeName;

    ui->colourTheme->clear();
    ui->colourTheme->addItems(themes.keys());

    setTheme(currentThemeName);

    QDialog::reject();
}
