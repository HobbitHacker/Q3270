#ifndef HOST_H
#define HOST_H

#include <QDialog>

namespace Ui {
    class Host;
}

class Host : public QDialog
{
        Q_OBJECT

    public:
        explicit Host(QWidget *parent = nullptr);
        ~Host();

        void updateAddress(QString newAddress);

        QString hostName;
        int port;
        QString luName;


    private slots:

        void accept();
        void fieldsChanged();

    private:
        Ui::Host *ui;
};

#endif // HOST_H
