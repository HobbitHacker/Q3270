#include "ui_NewTheme.h"
#include "KeyboardTheme.h"
#include "ui_KeyboardTheme.h"

KeyboardTheme::KeyboardTheme(QWidget *parent) : QDialog(parent), ui(new Ui::KeyboardTheme)
{   
    ui->setupUi(this);

    // Set up factory map
    theme.clear();

    theme.insert("Enter",      { "Enter", "RCtrl" });
    theme.insert("Reset",      { "LCtrl" });
    theme.insert("Insert",     { "Insert" });
    theme.insert("Delete",     { "Delete" });
    theme.insert("Up",         { "Up" });
    theme.insert("Down",       { "Down" });
    theme.insert("Left",       { "Left" });
    theme.insert("Right",      { "Right" });

    theme.insert("Backspace",  { "Backspace" });

    theme.insert("Tab",        { "Tab" });
    theme.insert("Backtab",    { "Backtab", "Shift+Tab", "Shift+Backtab" });

    theme.insert("Home",       { "Home" });
    theme.insert("EraseEOF",   { "End" });
    theme.insert("NewLine",    { "Return" });
    theme.insert("EndLine",    { "Ctrl+End" });

    theme.insert("F1",         { "F1" });
    theme.insert("F2",         { "F2" });
    theme.insert("F3",         { "F3" });
    theme.insert("F4",         { "F4" });
    theme.insert("F5",         { "F5" });
    theme.insert("F6",         { "F6" });
    theme.insert("F7",         { "F7", "PgUp" });
    theme.insert("F8",         { "F8", "PgDown" });
    theme.insert("F9",         { "F9" });
    theme.insert("F10",        { "F10" });
    theme.insert("F11",        { "F11" });
    theme.insert("F12",        { "F12" });

    theme.insert("F13",        { "Shift+F1" });
    theme.insert("F14",        { "Shift+F2" });
    theme.insert("F15",        { "Shift+F3" });
    theme.insert("F16",        { "Shift+F4" });
    theme.insert("F17",        { "Shift+F5" });
    theme.insert("F18",        { "Shift+F6" });
    theme.insert("F19",        { "Shift+F7" } );
    theme.insert("F20",        { "Shift+F8" });
    theme.insert("F21",        { "Shift+F9" });
    theme.insert("F22",        { "Shift+F10" });
    theme.insert("F23",        { "Shift+F11" });
    theme.insert("F24",        { "Shift+F12" });

    theme.insert("PA1",        { "Alt+1" });
    theme.insert("PA2",        { "Alt+2" });
    theme.insert("PA3",        { "Alt+3" });

    theme.insert("Attn",       { "Escape" });

    theme.insert("ToggleRuler", { "Ctrl+Home" });

    theme.insert("Clear",       { "Pause" });

    theme.insert("Copy",        { "Ctrl+C" });
    theme.insert("Paste",       { "Ctrl+V" });

    // Add factory theme to list of themes
    themes.insert("Factory", theme);

    // Store all functions: this is the definitive list of Q3270 functions
    functionList = theme.keys();
    ui->KeyboardFunctionList->addItems(functionList);

    // Populate drop-down list with Factory keyboard layout
    ui->keyboardThemes->addItem("Factory");

    // Now add those from the config file
    QSettings s;

    // Keyboard themes are all stored under the KeyboardThemes group
    s.beginGroup("KeyboardThemes");

    // Get a list of sub-groups
    QStringList themeList = s.childGroups();

    themeList.sort(Qt::CaseSensitive);

    // Populate schemes list and combo boxes from config file
    for (int sc = 0; sc < themeList.count(); sc++)
    {
        qDebug() << themeList.at(sc);

        // Ignore Factory theme (shouldn't be present, but in case of accidents, or user fudging)
        if (themeList.at(sc).compare("Factory"))
        {
            qDebug() << "Storing " << themeList.at(sc);
            // Begin theme specific group
            s.beginGroup(themeList.at(sc));

            // All keyboard mappings for this theme
            QStringList keys = s.childKeys();

            // Clear existing mappings from temporary map
            theme.clear();

            for (int kb = 0; kb < keys.count(); kb++)
            {
                // Keyboard maps stored as Shift+F1=F1; extract Q3270 function into thisKey
                QString thisKey = s.value(keys.at(kb)).toString();

                // Ensure it's a known function before we store it
                if (functionList.contains(thisKey))
                {

                    // Append keyboard mapping to Q3270 function; if it doesn't exist, QMap creates one first
                    theme[thisKey].append(keys.at(kb));
                }
            }

            // Save theme, assuming it had valid mappings
            if (theme.count() > 0)
            {
                themes.insert(themeList.at(sc), theme);
                ui->keyboardThemes->addItem(themeList.at(sc));
            }

            // End theme specific group
            s.endGroup();
        }

    }

    // End themes main group
    s.endGroup();

    setTheme("Factory");

    // Popup dialog for new themes
    newTheme = new Ui::NewTheme();

    newTheme->setupUi(&newThemePopUp);

    // Map the controls we're interested in
    connect(newTheme->newName, &QLineEdit::textChanged, this, &KeyboardTheme::checkDuplicate);
    connect(newTheme->buttonBox, &QDialogButtonBox::accepted, &newThemePopUp, &QDialog::accept);
    connect(newTheme->buttonBox, &QDialogButtonBox::rejected, &newThemePopUp, &QDialog::reject);

}

void KeyboardTheme::setTheme(QString newTheme)
{
    // If we don't know the name of the theme, fall back to Factory. This allows users to delete themes, but
    // still leave them referenced in session configurations.
    if (themes.find(newTheme)  == themes.end())
    {
        currentTheme = "Factory";
    }
    else
    {
        currentTheme = newTheme;
    }

    // Convenience variable to extract specified map
    KeyboardMap thisMap = themes.find(currentTheme).value();

    QMap<QString, QStringList>::ConstIterator i = thisMap.constBegin();

    // Clear keyboard map table in dialog
    ui->KeyboardMap->setRowCount(0);

    int row = 0;

    // Ensure that the "Unassigned" function is available to map keys
    ui->KeyboardFunctionList->addItem("Unassigned");

    while(i != thisMap.constEnd())
    {
        // Insert new row into table widget, and add details
        ui->KeyboardMap->insertRow(row);
        ui->KeyboardMap->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->KeyboardMap->setItem(row, 1, new QTableWidgetItem(i.value().join(", ")));
        i++;
    }

    ui->KeyboardMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void KeyboardTheme::themeChanged(int index)
{

    // Save new index
    currentThemeIndex = index;

    // Save the new name
    currentTheme = ui->keyboardThemes->itemText(index);

    // Update the keyboard map when the combobox changes
    setTheme(currentTheme);

    // Disable delete for Factory theme
    if (currentThemeIndex == 0)
    {
        ui->deleteThemeButton->setDisabled(true);
    }
    else
    {
        ui->deleteThemeButton->setEnabled(true);
    }
}

void KeyboardTheme::addTheme()
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
        newName = "New Scheme " + QString::number(i);
    }

    // Populate dialog box name
    newTheme->newName->setText(newName);

    // Allow user to modify name, disallowing existing names
    if(newThemePopUp.exec() == QDialog::Accepted)
    {
        // Save theme name
        newName = newTheme->newName->text();
        theme = themes[currentTheme];
        themes.insert(newName, theme);
        setTheme(newName);

        qDebug() << "Added " << newName << " Total items: " << themes.count();

        ui->keyboardThemes->addItem(newName);
        ui->keyboardThemes->setCurrentIndex(ui->keyboardThemes->count() - 1);
    }
}


void KeyboardTheme::checkDuplicate()
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

void KeyboardTheme::deleteTheme()
{

}

void KeyboardTheme::accept()
{

}
