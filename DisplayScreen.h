#ifndef DISPLAYDATA_H
#define DISPLAYDATA_H

#include <stdlib.h>
#include <string.h>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QDebug>
#include <QTimer>

#include "text.h"
#include "Buffer.h"
#include "3270.h"

class DisplayScreen : public QGraphicsScene
{
    Q_OBJECT

    public:
        DisplayScreen(int screen_x, int screen_y);
        ~DisplayScreen();

        int width();
        int height();
        qreal gridWidth();
        qreal gridHeight();

        void setChar(int pos, short unsigned int c, bool move);
        void setCharAttr(unsigned char c, unsigned char d);

        void resetExtendedHilite(int pos);
        void resetExtended(int pos);

        void setExtendedColour(int pos, bool foreground, unsigned char c);
        void setExtendedHilite(int pos);
        void setExtendedBlink(int pos);
        void setExtendedReverse(int pos);
        void setExtendedUscore(int pos);
        void setColour(int pos, bool foreground, unsigned char c);
        void setField(int pos, unsigned char c, bool sfe);
        void setGraphicEscape();

        void getModifiedFields(Buffer *buffer);

        int findNextUnprotectedField(int pos);
        int findPrevUnprotectedField(int pos);

        void resetCharAttr();

        bool insertChar(int pos, unsigned char c, bool insertMode);
        void deleteChar(int pos);
        void eraseEOF(int pos);

        void eraseUnprotected(int start, int end);

        void setCursor(int pos);
        void showCursor();
        void setFieldAttrs(int startPos);
        int resetFieldAttrs(int start);

        unsigned char getChar(int pos);
        bool isAskip(int pos);

        void clear();
        void setFont(QFont font);
        void setColourPalette(QColor *c);
        void resetColours();
        void setFontScaling(bool fontScaling);
        void toggleRuler();
        void drawRuler(int x, int y);

        void dumpFields();
        void dumpDisplay();
        void dumpAttrs(int pos);

    public slots:

        void blink();
        void cursorBlink();
        void setStatusXSystem(QString text);
        void showStatusCursorPosition(int x,int y);
        void setStatusInsert(bool ins);

    private:

        const QString EBCDICtoASCIImap[256] =
//        const unsigned char EBCDICtoASCIImap[256] =
        {
                "\u0000", "\u0001", "\u0002", "\u0003", "\u009C", "\u0009", "\u0086", "\u007F", /* 00 to 07 */
                "\u0097", "\u008D", "\u008E", "\u000B", "\u000C", "\u000D", "\u000E", "\u000F", /* 08 to 0F */
                "\u0010", "\u0011", "\u0012", "\u0013", "\u009D", "\u0085", "\u0008", "\u0087", /* 10 to 17 */
                "\u0018", "\u0019", "\u0092", "\u008F", "\u001C", "\u001D", "\u001E", "\u001F", /* 18 to 1F */
                "\u0080", "\u0081", "\u0082", "\u0083", "\u0084", "\u000A", "\u0017", "\u001B", /* 20 to 27 */
                "\u0088", "\u0089", "\u008A", "\u008B", "\u008C", "\u0005", "\u0006", "\u0007", /* 28 to 2F */
                "\u0090", "\u0091", "\u0016", "\u0093", "\u0094", "\u0095", "\u0096", "\u0004", /* 30 to 37 */
                "\u0098", "\u0099", "\u009A", "\u009B", "\u0014", "\u0015", "\u009E", "\u001A", /* 38 to 3F */
                "\u0020", "\u00A0", "\u00E2", "\u00E4", "\u00E0", "\u00E1", "\u00E3", "\u00E5", /* 40 to 47 */
                "\u00E7", "\u00F1", "\u00A2", "\u002E", "\u003C", "\u0028", "\u002B", "\u007C", /* 48 to 4F */
                "\u0026", "\u00E9", "\u00EA", "\u00EB", "\u00E8", "\u00ED", "\u00EE", "\u00EF", /* 50 to 57 */
                "\u00EC", "\u00DF", "\u0021", "\u0024", "\u002A", "\u0029", "\u003B", "\u00AC", /* 58 to 5F */
                "\u002D", "\u002F", "\u00C2", "\u00C4", "\u00C0", "\u00C1", "\u00C3", "\u00C5", /* 60 to 67 */
                "\u00C7", "\u00D1", "\u00A6", "\u002C", "\u0025", "\u005F", "\u003E", "\u003F", /* 68 to 6F */
                "\u00F8", "\u00C9", "\u00CA", "\u00CB", "\u00C8", "\u00CD", "\u00CE", "\u00CF", /* 70 to 77 */
                "\u00CC", "\u0060", "\u003A", "\u0023", "\u0040", "\u0027", "\u003D", "\u0022", /* 78 to 7F */
                "\u00D8", "\u0061", "\u0062", "\u0063", "\u0064", "\u0065", "\u0066", "\u0067", /* 80 to 87 */
                "\u0068", "\u0069", "\u00AB", "\u00BB", "\u00F0", "\u00FD", "\u00FE", "\u00B1", /* 88 to 8F */
                "\u00B0", "\u006A", "\u006B", "\u006C", "\u006D", "\u006E", "\u006F", "\u0070", /* 90 to 97 */
                "\u0071", "\u0072", "\u00AA", "\u00BA", "\u00E6", "\u00B8", "\u00C6", "\u00A4", /* 98 to 9F */
                "\u00B5", "\u007E", "\u0073", "\u0074", "\u0075", "\u0076", "\u0077", "\u0078", /* A0 to A7 */
                "\u0079", "\u007A", "\u00A1", "\u00BF", "\u00D0", "\u005B", "\u00DE", "\u00AE", /* A8 to AF */
                "\u00AC", "\u00A3", "\u00A5", "\u00B7", "\u00A9", "\u00A7", "\u00B6", "\u00BC", /* B0 to B7 */
                "\u00BD", "\u00BE", "\u00DD", "\u00A8", "\u00AF", "\u005D", "\u00B4", "\u00D7", /* B8 to BF */
                "\u007B", "\u0041", "\u0042", "\u0043", "\u0044", "\u0045", "\u0046", "\u0047", /* C0 to C7 */
                "\u0048", "\u0049", "\u00AD", "\u00F4", "\u00F6", "\u00F2", "\u00F3", "\u00F5", /* C8 to CF */
                "\u007D", "\u004A", "\u004B", "\u004C", "\u004D", "\u004E", "\u004F", "\u0050", /* D0 to D7 */
                "\u0051", "\u0052", "\u00B9", "\u00FB", "\u00FC", "\u00F9", "\u00FA", "\u00FF", /* D8 to DF */
                "\u005C", "\u00F7", "\u0053", "\u0054", "\u0055", "\u0056", "\u0057", "\u0058", /* E0 to E7 */
                "\u0059", "\u005A", "\u00B2", "\u00D4", "\u00D6", "\u00D2", "\u00D3", "\u00D5", /* E8 to EF */
                "\u0030", "\u0031", "\u0032", "\u0033", "\u0034", "\u0035", "\u0036", "\u0037", /* F0 to F7 */
                "\u0038", "\u0039", "\u00B3", "\u00DB", "\u00DC", "\u00D9", "\u00DA", "\u009F", /* F8 to FF */
        };

//            0x00, 0x01 ,0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F,  /* 00 - 07 */
//            0x1A, 0x1A ,0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 09 - 0F */
//            0x10, 0x11 ,0x12, 0x13, 0x1A, 0x1A, 0x08, 0x1A,  /* 10 - 17 */
//            0x18, 0x19 ,0x1A, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x0A, 0x17, 0x1B,  /* 20 - 27 */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x05, 0x06, 0x07,  /* 28 - 2F */
//            0x1A, 0x1A ,0x16, 0x1A, 0x1A, 0x1A, 0x1A, 0x04,  /* 30 - 37 */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x14, 0x15, 0x1A, 0x1A,  /* 38 - 3F */
//            0x20, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 40 - 47 */
//           0x1A, 0x1A ,0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,  /* 48 - 4F */
//           0x26, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 50 - 57 */
//            0x1A, 0x1A ,0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,  /* 58 - 5F */
//            0x2D, 0x2F ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 60 - 67 */
//            0x1A, 0x1A ,0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,  /* 68 - 6F */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 70 - 77 */
//            0x1A, 0x60 ,0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,  /* 78 - 7F */
//            0x1A, 0x61 ,0x62, 0x63, 0x64, 0x65, 0x66, 0x67,  /* 80 - 87 */
//            0x68, 0x69 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 88 - 8F */
//            0x1A, 0x6A ,0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,  /* 90 - 97 */
//            0x71, 0x72 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 98 - 9F */
//            0x1A, 0x7E ,0x73, 0x74, 0x75, 0x76, 0x77, 0x78,  /* A0 - A7 */
//            0x79, 0x7A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* A8 - AF */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* B0 - B7 */
//            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* B8 - BF */
//            0x7B, 0x41 ,0x42, 0x43, 0x44, 0x45, 0x46, 0x47,  /* C0 - C7 */
//            0x48, 0x49 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* C8 - CF */
//            0x7D, 0x4A ,0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,  /* D0 - D7 */
//            0x51, 0x52 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* D8 - DF */
//            0x5C, 0x1A ,0x53, 0x54, 0x55, 0x56, 0x57, 0x58,  /* E0 - E7 */
//            0x59, 0x5A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* E8 - EF */
//            0x30, 0x31 ,0x32, 0x33, 0x34, 0x35, 0x36, 0x37,  /* F0 - F7 */
//            0x38, 0x39 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A   /* F8 - FF */

//            0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, /* 00 to 07 */
//            0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, /* 08 to 0F */
//            0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, /* 10 to 17 */
//            0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F, /* 18 to 1F */
//            0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, /* 20 to 27 */
//            0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007, /* 28 to 2F */
//            0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, /* 30 to 37 */
//            0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A, /* 38 to 3F */
//            0x0020, 0x00A0, 0x00E2, 0x00E4, 0x00E0, 0x00E1, 0x00E3, 0x00E5, /* 40 to 47 */
//            0x00E7, 0x00F1, 0x00A2, 0x002E, 0x003C, 0x0028, 0x002B, 0x007C, /* 48 to 4F */
//            0x0026, 0x00E9, 0x00EA, 0x00EB, 0x00E8, 0x00ED, 0x00EE, 0x00EF, /* 50 to 57 */
//            0x00EC, 0x00DF, 0x0021, 0x0024, 0x002A, 0x0029, 0x003B, 0x005E, /* 58 to 5F */
//            0x002D, 0x002F, 0x00C2, 0x00C4, 0x00C0, 0x00C1, 0x00C3, 0x00C5, /* 60 to 67 */
//            0x00C7, 0x00D1, 0x00A6, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F, /* 68 to 6F */
//            0x00F8, 0x00C9, 0x00CA, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, /* 70 to 77 */
//            0x00CC, 0x0060, 0x003A, 0x0023, 0x0040, 0x0027, 0x003D, 0x0022, /* 78 to 7F */
//            0x00D8, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, /* 80 to 87 */
//            0x0068, 0x0069, 0x00AB, 0x00BB, 0x00F0, 0x00FD, 0x00FE, 0x00B1, /* 88 to 8F */
//            0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, /* 90 to 97 */
//            0x0071, 0x0072, 0x00AA, 0x00BA, 0x00E6, 0x00B8, 0x00C6, 0x00A4, /* 98 to 9F */
//            0x00B5, 0x007E, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, /* A0 to A7 */
//            0x0079, 0x007A, 0x00A1, 0x00BF, 0x00D0, 0x005B, 0x00DE, 0x00AE, /* A8 to AF */
//            0x00AC, 0x00A3, 0x00A5, 0x00B7, 0x00A9, 0x00A7, 0x00B6, 0x00BC, /* B0 to B7 */
//            0x00BD, 0x00BE, 0x00DD, 0x00A8, 0x00AF, 0x005D, 0x00B4, 0x00D7, /* B8 to BF */
//            0x007B, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, /* C0 to C7 */
//            0x0048, 0x0049, 0x00AD, 0x00F4, 0x00F6, 0x00F2, 0x00F3, 0x00F5, /* C8 to CF */
//            0x007D, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, /* D0 to D7 */
//            0x0051, 0x0052, 0x00B9, 0x00FB, 0x00FC, 0x00F9, 0x00FA, 0x00FF, /* D8 to DF */
//            0x005C, 0x00F7, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, /* E0 to E7 */
//            0x0059, 0x005A, 0x00B2, 0x00D4, 0x00D6, 0x00D2, 0x00D3, 0x00D5, /* E8 to EF */
//            0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, /* F0 to F7 */
//            0x0038, 0x0039, 0x00B3, 0x00DB, 0x00DC, 0x00D9, 0x00DA, 0x009F  /* F8 to FF */

        uchar ASCIItoEBCDICmap[256] =
        {
            0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F,  /* 00 - 07 */
            0x1A, 0x1A, 0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 08 - 1F */
            0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,  /* 10 - 17 */
            0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
            0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,  /* 20 - 27 */
            0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,  /* 28 - 2F */
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,  /* 30 - 37 */
            0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,  /* 38 - 3F */
            0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  /* 40 - 47 */
            0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,  /* 48 - 4F */
            0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,  /* 50 - 57 */
            0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x5A, 0x5F, 0x6D,  /* 58 - 5F */
            0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,  /* 60 - 67 */
            0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,  /* 68 - 6F */
            0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,  /* 70 - 77 */
            0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,  /* 78 - 7F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 80 - 87 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 80 - 8F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 90 - 97 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 98 - 9F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* A0 - A7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* A8 - AF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* B0 - B7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* B8 - BF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* C0 - C7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* C8 - CF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* D0 - D7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* D8 - DF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* E0 - E7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* E8 - EF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* F0 - F7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F   /* F8 - FF */
        };

        QString EBCDICtoASCIImapge[256] =
        {
  /* 00 - 07 */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 08 - 0F */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 10 - 17 */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 18 - 1F */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 20 - 27 */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 28 - 2F */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 30 - 37 */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 38 - 3F */   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 40 - 47 */   " ",	"𝐴̲", "𝐵̲", "𝐶̲", "𝐷̲", "𝐸̲", "𝐹̲", "𝐺̲",
  /* 48 - 4F */   "𝐻̲", "𝐼̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 50 - 57 */   0x00, "𝐽", "𝐾̲", "𝐿̲", "𝑀̲", "𝑁̲", "𝑂̲", "𝑃̲",
  /* 58 - 5F */    "𝑄̲", "𝑅̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 60 - 67 */   0x00, 0x00, "𝑆̲", "𝑇̲", "𝑈̲", "𝑉̲", "𝑊̲", "𝑋̲",
  /* 68 - 6F */    "𝑌̲", "𝑍̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 70 - 77 */   "⋄", "∧", "¨", "⌻", "⍸", "⍷", "⊢", "⊣",
  /* 78 - 7F */   "∨", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 80 - 87 */   "∼", "║", "═", "⎸", "⎹", "│", /* "│" */ 0x00, 0x00,
  /* 88 - 8F */   0x00, 0x00, "↑", "↓", "≤", "⌈", "⌊", "→",
  /* 90 - 97 */   "⎕", "▌", "▐", "▀", "▄", "█", 0x00, 0x00,
  /* 98 - 9F */   0x00, 0x00, "⊃", "⊂", "⌑", "○", "±", "←",
  /* A0 - A7 */   "‾", "°", "─", "∙", "ₙ", 0x00, 0x00, 0x00,
  /* A8 - AF */   0x00, 0x00, "∩", "∪", "⊥", "[", "≥", "∘",
  /* B0 - B7 */   "⍺", "∊", "⍳", "⍴", "⍵", 0x00, "×", "∖",
  /* B8 - BF */   "÷", 0x00, "∇", "∆", "⊤", "]", "≠", "│",
  /* C0 - C7 */   "{", "⁽", "⁺", "∎", "└", "┌", "├", "┴",
  /* C8 - CF */   "§", 0x00, "⍲", "⍱", "⌷", "⌽", "⍂", "⍉",
  /* D0 - D7 */   "}", "⁾", "⁻", "┼", "┘", "┐", "┤", "┬",
  /* D8 - D8 */   "¶", 0x00, "⌶", "!", "⍒", "⍋", "⍞", "⍝",
  /* E0 - E7 */   "≡", "₁", "₂", "₃", "⍤", "⍥", "⍪", "€",
  /* F8 - EF */   0x00, 0x00, "⌿", "⍀", "∵", "⊖", "⌹", "⍕",
  /* F0 - F7 */   "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷",
  /* F8 - FF */   "⁸", "⁹", 0x00, "⍫", "⍙", "⍟", "⍎", 0x00

        };




        const unsigned char twelveBitBufferAddress[64] = {
            0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  /* 00 0000 to 00 0011 */
            0xC8, 0xC9, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,  /* 00 0100 to 00 1111 */
            0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,  /* 01 0000 to 01 0011 */
            0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  /* 01 0100 to 01 1111 */
            0x60, 0x61, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,  /* 10 0000 to 10 0011 */
            0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,  /* 10 0100 to 10 1111 */
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,  /* 11 0000 to 11 0011 */
            0xF8, 0xF9, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,  /* 11 0100 to 11 1111 */
        };

        QColor palette[12];

        const char *colName[12] = { "black", "blue", "red", "magenta", "green", "cyan", "yellow", "neutral",
                                    "protected", "unprotected,intensfied", "unprotected", "unprotected, intensified"};

        struct Attributes {
                //TODO: Do we need a basic colour as well as extended?
                int colNum;
                bool askip;
                bool num;
                bool mdt;
                bool prot;
                bool fieldStart;
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
        };

        int screen_x;                /* Max Columns */
        int screen_y;                /* Max Rows */
        int screenPos_max;           /* Max position on screen */

        Attributes *attrs;           /* Attributes */

        Text **glyph;                /* Character on screen */
        QVector<QGraphicsRectItem *> cell;    /* Screen slot */
        QGraphicsLineItem **uscore;  /* Underscores */

        bool blinkShow;             /* Whether the character is shown/hidden for a given blink event */
        bool cursorShow;            /* Whether the cursor is shown/hidden for a given blink event */

        bool geActive;              // Next character is Graphic Escape

        // TODO Implement different sorts of crosshairs
        bool ruler;                  /* Whether the crosshairs are shown. */
                                     /* 0 - off, 1 - horizontal, 2 - vertical, 3 - crosshair */

        /* Character Attributes in effect */
        bool useCharAttr;
        struct {
                bool uscore;
                bool uscore_default;

                bool reverse;
                bool reverse_default;

                bool blink;
                bool blink_default;

                QBrush colour;
                int colNum;
                bool colour_default;
        } charAttr;

        QGraphicsRectItem *cursor;

        QGraphicsLineItem *crosshair_X;
        QGraphicsLineItem *crosshair_Y;

        QGraphicsSimpleTextItem *statusXSystem;
        QGraphicsSimpleTextItem *statusCursor;
        QGraphicsSimpleTextItem *statusInsert;

        QFont termFont;
        bool fontScaling;            // Font scales with cell size

        qreal gridSize_X;
        qreal gridSize_Y;

        int findField(int pos);
        int findNextField(int pos);
        int findPrevField(int pos);
};

#endif // DISPLAYDATA_H
