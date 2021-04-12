#ifndef GLYPH_H
#define GLYPH_H

#include <QRectF>
#include <QObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSimpleTextItem>

#include "Q3270.h"

class Glyph : public QObject, public QGraphicsSimpleTextItem
{

    Q_OBJECT

    public:
        Glyph(int x, int y, QGraphicsItem* parent);
        QRectF boundingRect() const;

        enum { Type = UserType + 1 };

        int type() const;

        void setText(const QString text, uchar ebcdic, bool graphic);
        void copyTextFrom(Glyph *source);

        // Getters, inline for speed
        inline uchar getEBCDIC()   { return ebcdic; };
        inline int getX()          { return pos_x; };
        inline int getY()          { return pos_y; };
        inline int getColour()     { return colNum; };

        inline bool isFieldStart() { return fieldStart; };
        inline bool isAutoSkip()   { return prot & num; };
        inline bool isNumeric()    { return num; };
        inline bool isGraphic()    { return graphic; };
        inline bool isMdtOn()      { return mdt; };
        inline bool isProtected()  { return prot; };
        inline bool isDisplay()    { return display; };
        inline bool isPenSelect()  { return pen; };
        inline bool isIntensify()  { return intensify; };
        inline bool isExtended()   { return extended; };
        inline bool isUScore()     { return uscore; };
        inline bool isReverse()    { return reverse; };
        inline bool isBlink()      { return blink; };

        inline bool hasCharAttrs() { return charAttr; };

        // Setters, inline for speed
        inline void setColour(int c)         { colNum = c; };
        inline void setFieldStart(bool fs)   { fieldStart = fs; };
//        inline void setAutoSkip(bool as)     { askip = as; };
        inline void setNumeric(bool n)       { askip = n; };
        inline void setGraphic(bool ge)      { graphic = ge; };
        inline void setMDT(bool m)           { mdt = m; };
        inline void setProtected(bool p)     { prot = p; };
        inline void setDisplay(bool d)       { display = d; };
        inline void setPenSelect(bool p)     { pen = p; };
        inline void setIntensify(bool i)     { intensify = i; };
        inline void setExtended(bool e)      { extended = e; };
        inline void setUScore(bool u)        { uscore = u; };
        inline void setReverse(bool r)       { reverse = r; };
        inline void setBlink(bool b)         { blink = b; };

        inline void setCharAttrs(bool c)     { charAttr = c; };

    private:

        int pos_x;              // 3270 screen position x
        int pos_y;              // 3270 screen position y
        QString contents;
        uchar ebcdic;

        // Is this a GE character?
        bool graphic;

        // Is this a field start?
        bool fieldStart;

        // Field attributes
        bool askip;
        bool num;
        bool mdt;
        bool prot;
        bool display;
        bool pen;
        bool intensify;

        /* Extended Attributes */
        bool extended;
        bool uscore;
        bool reverse;
        bool blink;

        /* Character Attribute in effect */
        bool charAttr;

        // Colour of glyph
        int colNum;

};

#endif // GLYPH_H
