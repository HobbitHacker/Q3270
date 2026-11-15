#ifndef FILETRANSFERDIALOG_H
#define FILETRANSFERDIALOG_H

#include <QDialog>

#include "Q3270.h"

namespace Ui {
class FileTransferDialog;
}

class FileTransferDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit FileTransferDialog(QWidget *parent = nullptr);
        ~FileTransferDialog();

        bool showForm();

        const QString getCommand();
        Q3270::TransferDirection getDirection();
        QString getLocalFile();
        bool isAscii();
        bool isCRLF();
        bool isDFTEnabled();
        quint16 getDFTBlockSize();

    private slots:

        void on_browseButton_clicked();
        void on_direction_changed();
        void on_hosttype_changed();
        void on_recfm_changed();
        void on_space_changed();
        void on_ascii_changed();
        void on_dftEnabled_changed();

    private:
        void setOptions();

        Ui::FileTransferDialog *ui;
};

#endif // FILETRANSFERDIALOG_H
