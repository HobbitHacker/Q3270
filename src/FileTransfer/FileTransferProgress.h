#ifndef FILETRANSFERPROGRESS_H
#define FILETRANSFERPROGRESS_H

#include <QDialog>

namespace Ui{
    class FileTransferProgress;
}

class FileTransferProgress : public QDialog
{
    Q_OBJECT

    public:
        explicit FileTransferProgress(QWidget *parent = nullptr);
        ~FileTransferProgress();

        void reset();
        void complete();

    public slots:
        void updateStats(const unsigned long long bytesReceived, const unsigned long long blocksReceived);
        void updateMessage(const QString &message);

    private:
        Ui::FileTransferProgress *ui;
};

#endif // FILETRANSFERPROGRESS_H
