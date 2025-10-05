/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef CODEPAGE_H
#define CODEPAGE_H

#include <QString>
#include <QMap>

/**
 * @brief   The CodePage class
 *
 * @details This class is used to convert from EBCDIC to a Unicode character, or from an entered keyboard character
 *          to EBCDIC.
 *
 *          It's probably not the best way to do it, but it works.
 */

class CodePage
{
public:
    CodePage();
    QString getUnicodeChar(uchar ebcdic) const;
    QString getUnicodeGraphicChar(uchar ebcdic) const;
    uchar getEBCDIC(uchar acscii) const;

    void setCodePage(QString codepage);
    const QString getCodePage() const;

    const QStringList getCodePageList() const;

private:

    int currentCodePage;

    QString ebcdic[256];
    uchar *ascii[256];

    typedef struct {
        QString cpName;          // Codepage number - 037, 1024 etc
        QString displayName;     // Display name - IBM-037, ISO8859-1 etc
        bool selectable;         // Whether it will appear in the list on the terminal settings page
        QString fromEBCDIC[256]; // Character table converting from EBCDIC to Unicode
        uchar toEBCDIC[256];     // Character table converting from ASCII to EBCDIC

    } _cpList;

    const _cpList cp_037 = {
        "037", "IBM-037", true,
        {
            /* 00 to 07 */ "\u0000", "\u0001", "\u0002", "\u0003", "\u009C", "\u0009", "\u0086", "\u007F",
            /* 08 to 0F */ "\u0097", "\u008D", "\u008E", "\u000B", "\u000C", "\u000D", "\u000E", "\u000F",
            /* 10 to 17 */ "\u0010", "\u0011", "\u0012", "\u0013", "\u009D", "\u0085", "\u0008", "\u0087",
            /* 18 to 1F */ "\u0018", "\u0019", "\u0092", "\u008F", "\u001C", "\u001D", "\u001E", "\u001F",
            /* 20 to 27 */ "\u0080", "\u0081", "\u0082", "\u0083", "\u0084", "\u000A", "\u0017", "\u001B",
            /* 28 to 2F */ "\u0088", "\u0089", "\u008A", "\u008B", "\u008C", "\u0005", "\u0006", "\u0007",
            /* 30 to 37 */ "\u0090", "\u0091", "\u0016", "\u0093", "\u0094", "\u0095", "\u0096", "\u0004",
            /* 38 to 3F */ "\u0098", "\u0099", "\u009A", "\u009B", "\u0014", "\u0015", "\u009E", "\u001A",
            /* 40 to 47 */ "\u0020", "\u00A0", "\u00E2", "\u00E4", "\u00E0", "\u00E1", "\u00E3", "\u00E5",
            /* 48 to 4F */ "\u00E7", "\u00F1", "\u00A2", "\u002E", "\u003C", "\u0028", "\u002B", "\u007C",
            /* 50 to 57 */ "\u0026", "\u00E9", "\u00EA", "\u00EB", "\u00E8", "\u00ED", "\u00EE", "\u00EF",
            /* 58 to 5F */ "\u00EC", "\u00DF", "\u0021", "\u0024", "\u002A", "\u0029", "\u003B", "\u00AC",
            /* 60 to 67 */ "\u002D", "\u002F", "\u00C2", "\u00C4", "\u00C0", "\u00C1", "\u00C3", "\u00C5",
            /* 68 to 6F */ "\u00C7", "\u00D1", "\u00A6", "\u002C", "\u0025", "\u005F", "\u003E", "\u003F",
            /* 70 to 77 */ "\u00F8", "\u00C9", "\u00CA", "\u00CB", "\u00C8", "\u00CD", "\u00CE", "\u00CF",
            /* 78 to 7F */ "\u00CC", "\u0060", "\u003A", "\u0023", "\u0040", "\u0027", "\u003D", "\u0022",
            /* 80 to 87 */ "\u00D8", "\u0061", "\u0062", "\u0063", "\u0064", "\u0065", "\u0066", "\u0067",
            /* 88 to 8F */ "\u0068", "\u0069", "\u00AB", "\u00BB", "\u00F0", "\u00FD", "\u00FE", "\u00B1",
            /* 90 to 97 */ "\u00B0", "\u006A", "\u006B", "\u006C", "\u006D", "\u006E", "\u006F", "\u0070",
            /* 98 to 9F */ "\u0071", "\u0072", "\u00AA", "\u00BA", "\u00E6", "\u00B8", "\u00C6", "\u00A4",
            /* A0 to A7 */ "\u00B5", "\u007E", "\u0073", "\u0074", "\u0075", "\u0076", "\u0077", "\u0078",
            /* A8 to AF */ "\u0079", "\u007A", "\u00A1", "\u00BF", "\u00D0", "\u00DD", "\u00DE", "\u00AE",
            /* B0 to B7 */ "\u00AC", "\u00A3", "\u00A5", "\u00B7", "\u00A9", "\u00A7", "\u00B6", "\u00BC",
            /* B8 to BF */ "\u00BD", "\u00BE", "\u005B", "\u005D", "\u00AF", "\u00A8", "\u00B4", "\u00D7",
            /* C0 to C7 */ "\u007B", "\u0041", "\u0042", "\u0043", "\u0044", "\u0045", "\u0046", "\u0047",
            /* C8 to CF */ "\u0048", "\u0049", "\u00AD", "\u00F4", "\u00F6", "\u00F2", "\u00F3", "\u00F5",
            /* D0 to D7 */ "\u007D", "\u004A", "\u004B", "\u004C", "\u004D", "\u004E", "\u004F", "\u0050",
            /* D8 to DF */ "\u0051", "\u0052", "\u00B9", "\u00FB", "\u00FC", "\u00F9", "\u00FA", "\u00FF",
            /* E0 to E7 */ "\u005C", "\u00F7", "\u0053", "\u0054", "\u0055", "\u0056", "\u0057", "\u0058",
            /* E8 to EF */ "\u0059", "\u005A", "\u00B2", "\u00D4", "\u00D6", "\u00D2", "\u00D3", "\u00D5",
            /* F0 to F7 */ "\u0030", "\u0031", "\u0032", "\u0033", "\u0034", "\u0035", "\u0036", "\u0037",
            /* F8 to FF */ "\u0038", "\u0039", "\u00B3", "\u00DB", "\u00DC", "\u00D9", "\u00DA", "\u009F"
          },
          {
            /* 00 to 07 */ 0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F,
            /* 08 to 1F */ 0x1A, 0x1A, 0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            /* 10 to 17 */ 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
            /* 18 to 1F */ 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
            /* 20 to 27 */ 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
            /* 28 to 2F */ 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
            /* 30 to 37 */ 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
            /* 38 to 3F */ 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
            /* 40 to 47 */ 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
            /* 48 to 4F */ 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
            /* 50 to 57 */ 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
            /* 58 to 5F */ 0xE7, 0xE8, 0xE9, 0xBA, 0xE0, 0xBB, 0x5F, 0x6D,
            /* 60 to 67 */ 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            /* 68 to 6F */ 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
            /* 70 to 77 */ 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
            /* 78 to 7F */ 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
            /* 80 to 87 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* 80 to 8F */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* 90 to 97 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* 98 to 9F */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* A0 to A7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* A8 to AF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* B0 to B7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* B8 to BF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* C0 to C7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* C8 to CF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* D0 to D7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* D8 to DF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* E0 to E7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* E8 to EF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* F0 to F7 */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
            /* F8 to FF */ 0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
          }
    };

    const _cpList cp_285 = {
        "285", "IBM-285", true,
        {
            /* 00 to 07 */     "\u0000", "\u0001", "\u0002", "\u0003", "\u0000", "\u0009", "\u0000", "\u007F",
            /* 08 to 0F */     "\u0000", "\u0000", "\u0000", "\u000B", "\u000C", "\u000D", "\u000E", "\u000F",
            /* 10 to 17 */     "\u0010", "\u0011", "\u0012", "\u0013", "\u0000", "\u0085", "\u0008", "\u0000",
            /* 18 to 1F */     "\u0018", "\u0019", "\u0000", "\u0000", "\u001C", "\u001D", "\u001E", "\u001F",
            /* 20 to 27 */     "\u0000", "\u0000", "\u0000", "\u0000", "\u0000", "\u000A", "\u0017", "\u001B",
            /* 28 to 2F */     "\u0000", "\u0000", "\u0000", "\u0000", "\u0000", "\u0005", "\u0006", "\u0007",
            /* 30 to 37 */     "\u0000", "\u0000", "\u0016", "\u0000", "\u0000", "\u0000", "\u0000", "\u0004",
            /* 38 to 3F */     "\u0000", "\u0000", "\u0000", "\u0000", "\u0014", "\u0015", "\u0000", "\u001A",
            /* 40 to 47 */     "\u0020", "\u00A0", "\u00E2", "\u00E4", "\u00E0", "\u00E1", "\u00E3", "\u00E5",
            /* 48 to 4F */     "\u00E7", "\u00F1", "\u0024", "\u002E", "\u003C", "\u0028", "\u002B", "\u0000",
            /* 50 to 57 */     "\u0026", "\u00E9", "\u00EA", "\u00EB", "\u00E8", "\u00ED", "\u00EE", "\u00EF",
            /* 58 to 5F */     "\u00EC", "\u00DF", "\u0021", "\u00A3", "\u002A", "\u0029", "\u003B", "\u00AC",
            /* 60 to 67 */     "\u002D", "\u002F", "\u00C2", "\u00C4", "\u00C0", "\u00C1", "\u00C3", "\u00C5",
            /* 68 to 6F */     "\u00C7", "\u00D1", "\u00A6", "\u002C", "\u0025", "\u005F", "\u003E", "\u003F",
            /* 70 to 77 */     "\u00F8", "\u00C9", "\u00CA", "\u00CB", "\u00C8", "\u00CD", "\u00CE", "\u00CF",
            /* 78 to 7F */     "\u00CC", "\u0060", "\u003A", "\u0023", "\u0040", "\u0027", "\u003D", "\u0022",
            /* 80 to 87 */     "\u00D8", "\u0061", "\u0062", "\u0063", "\u0064", "\u0065", "\u0066", "\u0067",
            /* 88 to 8F */     "\u0068", "\u0069", "\u00AB", "\u00BB", "\u00F0", "\u00FD", "\u00FE", "\u00B1",
            /* 90 to 97 */     "\u00B0", "\u006A", "\u006B", "\u006C", "\u006D", "\u006E", "\u006F", "\u0070",
            /* 98 to 9F */     "\u0071", "\u0072", "\u00AA", "\u00BA", "\u00E6", "\u00B8", "\u00C6", "\u00A4",
            /* A0 to A7 */     "\u00B5", "\u00AF", "\u0073", "\u0074", "\u0075", "\u0076", "\u0077", "\u0078",
            /* A8 to AF */     "\u0079", "\u007A", "\u00A1", "\u00BF", "\u00D0", "\u00DD", "\u00DE", "\u00AE",
            /* B0 to B7 */     "\u00A2", "\u005B", "\u00A5", "\u00B7", "\u00A9", "\u00A7", "\u00B6", "\u00BC",
            /* B8 to BF */     "\u00BD", "\u00BE", "\u005E", "\u005D", "\u007E", "\u00A8", "\u00B4", "\u00D7",
            /* C0 to C7 */     "\u007B", "\u0041", "\u0042", "\u0043", "\u0044", "\u0045", "\u0046", "\u0047",
            /* C8 to CF */     "\u0048", "\u0049", "\u00AD", "\u00F4", "\u00F6", "\u00F2", "\u00F3", "\u00F5",
            /* D0 to D7 */     "\u007D", "\u004A", "\u004B", "\u004C", "\u004D", "\u004E", "\u004F", "\u0050",
            /* D8 to DF */     "\u0051", "\u0052", "\u00B9", "\u00FB", "\u00FC", "\u00F9", "\u00FA", "\u00FF",
            /* E0 to E7 */     "\u005C", "\u00F7", "\u0053", "\u0054", "\u0055", "\u0056", "\u0057", "\u0058",
            /* E8 to EF */     "\u0059", "\u005A", "\u00B2", "\u00D4", "\u00D6", "\u00D2", "\u00D3", "\u00D5",
            /* F0 to F7 */     "\u0030", "\u0031", "\u0032", "\u0033", "\u0034", "\u0035", "\u0036", "\u0037",
            /* F8 to FF */     "\u0038", "\u0039", "\u00B3", "\u00DB", "\u00DC", "\u00D9", "\u00DA", "\u0000"
        },
        {
            /* 00 to 07 */     0xFF, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
            /* 08 to 0F */     0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            /* 10 to 17 */     0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
            /* 18 to 1F */     0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
            /* 20 to 27 */     0x40, 0x5A, 0x7F, 0x7B, 0x4A, 0x6C, 0x50, 0x7D,
            /* 28 to 2F */     0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
            /* 30 to 37 */     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
            /* 38 to 3F */     0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
            /* 40 to 47 */     0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
            /* 48 to 4F */     0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
            /* 50 to 57 */     0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
            /* 58 to 5F */     0xE7, 0xE8, 0xE9, 0xB1, 0xE0, 0xBB, 0xBA, 0x6D,
            /* 60 to 67 */     0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            /* 68 to 6F */     0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
            /* 70 to 77 */     0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
            /* 78 to 7F */     0xA7, 0xA8, 0xA9, 0xC0, 0x00, 0xD0, 0xBC, 0x07,
            /* 80 to 87 */     0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
            /* 88 to 8F */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 90 to 97 */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 98 to 9F */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* A0 to A7 */     0x41, 0xAA, 0xB0, 0x5B, 0x9F, 0xB2, 0x6A, 0xB5,
            /* A8 to AF */     0xBD, 0xB4, 0x9A, 0x8A, 0x5F, 0xCA, 0xAF, 0xA1,
            /* B0 to B7 */     0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3,
            /* B8 to BF */     0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,
            /* C0 to C7 */     0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68,
            /* C8 to CF */     0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
            /* D0 to D7 */     0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF,
            /* D8 to DF */     0x80, 0xFD, 0xFE, 0xFB, 0xFC, 0xAD, 0xAE, 0x59,
            /* E0 to E7 */     0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48,
            /* E8 to EF */     0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
            /* F0 to F7 */     0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1,
            /* F8 to FF */     0x70, 0xDD, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF
        }
    };

    // This codepage does not appear in the selectable list on the Terminal Settings dialog
    const _cpList cp_310 = {
        "310", "IBM-310", false,
    // TODO: Tidy up definition (use unicode rather than characters)
        {
            /* 00 to 07 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 08 to 0F */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 10 to 17 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 18 to 1F */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 20 to 27 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 28 to 2F */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 30 to 37 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 38 to 3F */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 40 to 47 */  " ", "\u1D434", "𝐵̲", "𝐶̲", "𝐷̲", "𝐸̲", "𝐹̲", "𝐺̲",
            /* 48 to 4F */  "𝐻̲", "𝐼̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 50 to 57 */  0x00, "𝐽", "𝐾̲", "𝐿̲", "𝑀̲", "𝑁̲", "𝑂̲", "𝑃̲",
            /* 58 to 5F */   "𝑄̲", "𝑅̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 60 to 67 */  0x00, 0x00, "𝑆̲", "𝑇̲", "𝑈̲", "𝑉̲", "𝑊̲", "𝑋̲",
            /* 68 to 6F */   "𝑌̲", "𝑍̲", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 70 to 77 */  "⋄", "∧", "¨", "⌻", "⍸", "⍷", "⊢", "⊣",
            /* 78 to 7F */  "∨", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* 80 to 87 */  "∼", "║", "═", "⎸", "⎹", "│", 0x00, 0x00,
            /* 88 to 8F */  0x00, 0x00, "↑", "↓", "≤", "⌈", "⌊", "→",
            /* 90 to 97 */  "⎕", "▌", "▐", "▀", "▄", "█", 0x00, 0x00,
            /* 98 to 9F */  0x00, 0x00, "⊃", "⊂", "⌑", "○", "±", "←",
            /* A0 to A7 */  "‾", "°", "─", "∙", "ₙ", 0x00, 0x00, 0x00,
            /* A8 to AF */  0x00, 0x00, "∩", "∪", "⊥", "[", "≥", "∘",
            /* B0 to B7 */  "⍺", "∊", "⍳", "⍴", "⍵", 0x00, "×", "∖",
            /* B8 to BF */  "÷", 0x00, "∇", "∆", "⊤", "]", "≠", "│",
            /* C0 to C7 */  "{", "⁽", "⁺", "∎", "└", "┌", "├", "┴",
            /* C8 to CF */  "§", 0x00, "⍲", "⍱", "⌷", "⌽", "⍂", "⍉",
            /* D0 to D7 */  "}", "⁾", "⁻", "┼", "┘", "┐", "┤", "┬",
            /* D8 to D8 */  "¶", 0x00, "⌶", "!", "⍒", "⍋", "⍞", "⍝",
            /* E0 to E7 */  "≡", "₁", "₂", "₃", "⍤", "⍥", "⍪", "€",
            /* F8 to EF */  0x00, 0x00, "⌿", "⍀", "∵", "⊖", "⌹", "⍕",
            /* F0 to F7 */  "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷",
            /* F8 to FF */  "⁸", "⁹", 0x00, "⍫", "⍙", "⍟", "⍎", 0x00
         },
        {}
    };

    const _cpList cp_1047 = {
        "1047", "IBM-1047", true,
        {
            /* 00 to 07 */ "\u0000", "\u0001", "\u0002", "\u0003", "\u009C", "\u0009", "\u0086", "\u007F",
            /* 08 to 0F */ "\u0097", "\u008D", "\u008E", "\u000B", "\u000C", "\u000D", "\u000E", "\u000F",
            /* 10 to 17 */ "\u0010", "\u0011", "\u0012", "\u0013", "\u009D", "\u0085", "\u0008", "\u0087",
            /* 18 to 1F */ "\u0018", "\u0019", "\u0092", "\u008F", "\u001C", "\u001D", "\u001E", "\u001F",
            /* 20 to 27 */ "\u0080", "\u0081", "\u0082", "\u0083", "\u0084", "\u000A", "\u0017", "\u001B",
            /* 28 to 2F */ "\u0088", "\u0089", "\u008A", "\u008B", "\u008C", "\u0005", "\u0006", "\u0007",
            /* 30 to 37 */ "\u0090", "\u0091", "\u0016", "\u0093", "\u0094", "\u0095", "\u0096", "\u0004",
            /* 38 to 3F */ "\u0098", "\u0099", "\u009A", "\u009B", "\u0014", "\u0015", "\u009E", "\u001A",
            /* 40 to 47 */ "\u0020", "\u00A0", "\u00E2", "\u00E4", "\u00E0", "\u00E1", "\u00E3", "\u00E5",
            /* 48 to 4F */ "\u00E7", "\u00F1", "\u00A2", "\u002E", "\u003C", "\u0028", "\u002B", "\u007C",
            /* 50 to 57 */ "\u0026", "\u00E9", "\u00EA", "\u00EB", "\u00E8", "\u00ED", "\u00EE", "\u00EF",
            /* 58 to 5F */ "\u00EC", "\u00DF", "\u0021", "\u0024", "\u002A", "\u0029", "\u003B", "\u005E",
            /* 60 to 67 */ "\u002D", "\u002F", "\u00C2", "\u00C4", "\u00C0", "\u00C1", "\u00C3", "\u00C5",
            /* 68 to 6F */ "\u00C7", "\u00D1", "\u00A6", "\u002C", "\u0025", "\u005F", "\u003E", "\u003F",
            /* 70 to 77 */ "\u00F8", "\u00C9", "\u00CA", "\u00CB", "\u00C8", "\u00CD", "\u00CE", "\u00CF",
            /* 78 to 7F */ "\u00CC", "\u0060", "\u003A", "\u0023", "\u0040", "\u0027", "\u003D", "\u0022",
            /* 80 to 87 */ "\u00D8", "\u0061", "\u0062", "\u0063", "\u0064", "\u0065", "\u0066", "\u0067",
            /* 88 to 8F */ "\u0068", "\u0069", "\u00AB", "\u00BB", "\u00F0", "\u00FD", "\u00FE", "\u00B1",
            /* 90 to 97 */ "\u00B0", "\u006A", "\u006B", "\u006C", "\u006D", "\u006E", "\u006F", "\u0070",
            /* 98 to 9F */ "\u0071", "\u0072", "\u00AA", "\u00BA", "\u00E6", "\u00B8", "\u00C6", "\u00A4",
            /* A0 to A7 */ "\u00B5", "\u007E", "\u0073", "\u0074", "\u0075", "\u0076", "\u0077", "\u0078",
            /* A8 to AF */ "\u0079", "\u007A", "\u00A1", "\u00BF", "\u00D0", "\u005B", "\u00DE", "\u00AE",
            /* B0 to B7 */ "\u00AC", "\u00A3", "\u00A5", "\u00B7", "\u00A9", "\u00A7", "\u00B6", "\u00BC",
            /* B8 to BF */ "\u00BD", "\u00BE", "\u00DD", "\u00A8", "\u00AF", "\u005D", "\u00B4", "\u00D7",
            /* C0 to C7 */ "\u007B", "\u0041", "\u0042", "\u0043", "\u0044", "\u0045", "\u0046", "\u0047",
            /* C8 to CF */ "\u0048", "\u0049", "\u00AD", "\u00F4", "\u00F6", "\u00F2", "\u00F3", "\u00F5",
            /* D0 to D7 */ "\u007D", "\u004A", "\u004B", "\u004C", "\u004D", "\u004E", "\u004F", "\u0050",
            /* D8 to DF */ "\u0051", "\u0052", "\u00B9", "\u00FB", "\u00FC", "\u00F9", "\u00FA", "\u00FF",
            /* E0 to E7 */ "\u005C", "\u00F7", "\u0053", "\u0054", "\u0055", "\u0056", "\u0057", "\u0058",
            /* E8 to EF */ "\u0059", "\u005A", "\u00B2", "\u00D4", "\u00D6", "\u00D2", "\u00D3", "\u00D5",
            /* F0 to F7 */ "\u0030", "\u0031", "\u0032", "\u0033", "\u0034", "\u0035", "\u0036", "\u0037",
            /* F8 to FF */ "\u0038", "\u0039", "\u00B3", "\u00DB", "\u00DC", "\u00D9", "\u00DA", "\u009F"
        },
        {
            /* 00 to 07 */ 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,
            /* 08 to 0F */ 0x16, 0x05, 0x10, 0x11, 0x12, 0x13, 0x3C, 0x0F,
            /* 10 to 17 */ 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,
            /* 18 to 1F */ 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
            /* 20 to 27 */ 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,
            /* 28 to 2F */ 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
            /* 30 to 37 */ 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
            /* 38 to 3F */ 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
            /* 40 to 47 */ 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
            /* 48 to 4F */ 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
            /* 50 to 57 */ 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
            /* 58 to 5F */ 0xE7, 0xE8, 0xE9, 0xAC, 0xE0, 0xA8, 0x5F, 0x6D,
            /* 60 to 67 */ 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            /* 68 to 6F */ 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
            /* 70 to 77 */ 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
            /* 78 to 7F */ 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,

            /* 80 to 87 */ 0x20, 0x21 ,0x22, 0x23, 0x24, 0x15, 0x06, 0x17,
            /* 88 to 8F */ 0x28, 0x29 ,0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
            /* 90 to 97 */ 0x30, 0x31 ,0x1A, 0x33, 0x35, 0x36, 0x37, 0x08,
            /* 98 to 9F */ 0x38, 0x39 ,0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,
            /* A0 to A7 */ 0x41, 0xAA ,0x4A, 0xB1, 0x9F, 0xB2, 0x6A, 0xB5,
            /* A8 to AF */ 0xBB, 0xB4 ,0x9A, 0x8A, 0xB0, 0xCA, 0xAF, 0xBC,
            /* B0 to B7 */ 0x90, 0x8F ,0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3,
            /* B8 to BF */ 0x9D, 0xDA ,0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,
            /* C0 to C7 */ 0x64, 0x65 ,0x66, 0x67, 0x68, 0x69, 0x9E, 0x68,
            /* C8 to CF */ 0x74, 0x71 ,0x72, 0x73, 0x78, 0x75, 0x77, 0x77,
            /* D0 to D7 */ 0xAC, 0x69 ,0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xB7,
            /* D8 to DF */ 0x80, 0xFD ,0xFE, 0xFB, 0xFC, 0xBA, 0xAE, 0x59,
            /* E0 to E7 */ 0x44, 0x45 ,0x42, 0x46, 0x43, 0x47, 0x9C, 0x48,
            /* E8 to EF */ 0x54, 0x51 ,0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
            /* F0 to F7 */ 0x8C, 0x49 ,0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1,
            /* F8 to FF */ 0x70, 0xDD ,0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF
        }
    };


#define CODEPAGE_COUNT 4

    // Codepage 310 is programmatically expected to be at position 0 in the array below

    _cpList cpList[CODEPAGE_COUNT] = { cp_310, cp_037, cp_285, cp_1047 };


};
#endif // CODEPAGE_H
