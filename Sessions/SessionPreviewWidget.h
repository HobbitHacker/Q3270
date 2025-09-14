#ifndef SESSIONPREVIEWWIDGET_H
#define SESSIONPREVIEWWIDGET_H

#include <QWidget>
#include "ui_SessionPreview.h"
#include "Session.h"

class SessionPreviewWidget : public QWidget, private Ui::SessionPreviewWidget
{
    Q_OBJECT
public:
    explicit SessionPreviewWidget(QWidget *parent = nullptr);
    void setSession(const Session &session);
    void clear();
};
#endif // SESSIONPREVIEWWIDGET_H
