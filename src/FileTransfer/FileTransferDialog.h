#ifndef FILETRANSFERDIALOG_H
#define FILETRANSFERDIALOG_H

#include <QDialog>

#include "Terminal.h"
#include "Q3270.h"

namespace Ui {
class FileTransferDialog;
}

class FileTransferDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit FileTransferDialog(Terminal *terminal, QWidget *parent = nullptr);
        ~FileTransferDialog();

        bool showForm();

        const QString getCommand();

    private slots:
        void on_browseButton_clicked();
        void on_direction_changed();
        void on_hosttype_changed();
        void on_recfm_changed();
        void on_space_changed();
        void on_ascii_changed();

    private:
        void setOptions();

        Ui::FileTransferDialog *ui;

        Terminal *terminal;
};

#endif // FILETRANSFERDIALOG_H
