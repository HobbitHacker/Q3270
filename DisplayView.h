#ifndef DISPLAYVIEW_H
#define DISPLAYVIEW_H

#include <QGraphicsView>

class DisplayView : public QGraphicsView
{
        Q_OBJECT
    public:
        DisplayView();
        void resizeEvent(QResizeEvent *r);
        void scaleFont(bool scale);

    private:
        bool resizeFont;

};

#endif // DISPLAYVIEW_H
