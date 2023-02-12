#ifndef N3270_H
#define N3270_H

#include <QObject>

/* 3270 Write Commands */
#include "qobjectdefs.h"
#define IBM3270_W    0xF1  /* Write */
#define IBM3270_EW   0xF5  /* Erase Write */
#define IBM3270_EWA  0x7E  /* Erase/Write Alternate */
#define IBM3270_RB   0xF2  /* Read Buffer */
#define IBM3270_RM   0xF6  /* Read Modified */
#define IBM3270_RMA  0x6E  /* Read Modified All */
#define IBM3270_EAU  0x6F  /* Erase All Uprotected */
#define IBM3270_WSF  0xF3  /* Write Structured Field */

/* 3270 CCW Write Commands */
#define IBM3270_CCW_W      0x01   /* Write */
#define IBM3270_CCW_EW     0x05   /* Erase/Write */
#define IBM3270_CCW_EWA    0x0D   /* Erase/Write Alternate */
#define IBM3270_CCW_RB     0x02   /* Read Buffer */
#define IBM3270_CCW_RM     0x06   /* Read Modified */
#define IBM3270_CCW_EAU    0x0F   /* Erase All Unprotected */
#define IBM3270_CCW_WSF    0x11   /* Write Structured Field */
#define IBM3270_CCW_NOP    0x03   /* No Operation */
#define IBM3270_CCW_SNS    0x04   /* Sense */
#define IBM3270_CCW_SNSID  0xE4   /* Sense Id */

/* 3270 Orders */
#define IBM3270_SF   0x1D   /* Start Field */
#define IBM3270_SFE  0x29   /* Start Field Extended */
#define IBM3270_SBA  0x11   /* Set Buffer Address */
#define IBM3270_SA   0x28   /* Set Attribute */
#define IBM3270_MF   0x2C   /* Modify field */
#define IBM3270_IC   0x13   /* Insert Cursor */
#define IBM3270_PT   0x05   /* Program Tab */
#define IBM3270_RA   0x3C   /* Repeat to Address */
#define IBM3270_EUA  0x12   /* Erase Unprotected to Address */
#define IBM3270_GE   0x08   /* Graphic Escape */

/* Constants for some EBCDIC chars */
#define IBM3270_CHAR_NULL  0x00
#define IBM3270_CHAR_SPACE 0x40

/* Write Structured Field Commands */
#define IBM3270_WSF_RESET         0x00
#define IBM3270_WSF_READPARTITION 0x01
#define IBM3270_WSF_OB3270DS      0x40

/* Inbound Structured Fields */
#define IBM3270_SF_QUERYREPLY            0x81

#define IBM3270_SF_QUERYREPLY_SUMMARY    0x80
#define IBM3270_SF_QUERYREPLY_USABLE     0x81
#define IBM3270_SF_QUERYREPLY_PARTS      0x84 /* Alphanumeric Partitions */
#define IBM3270_SF_QUERYREPLY_CHARSETS   0x85 /* Graphic Escape supported for example */
#define IBM3270_SF_QUERYREPLY_COLOUR     0x86
#define IBM3270_SF_QUERYREPLY_HIGHLIGHT  0x87
#define IBM3270_SF_QUERYREPLY_IMPPARTS   0xA6 /* Implicit Partitions */

/* Extended Attributes */
#define IBM3270_EXT_DEFAULT       0x00
#define IBM3270_EXT_3270          0xC0
#define IBM3270_EXT_VALIDATION    0xC1
#define IBM3270_EXT_OUTLINE       0xC2
#define IBM3270_EXT_HILITE        0x41
#define IBM3270_EXT_FG_COLOUR     0x42
#define IBM3270_EXT_CHARSET       0x43
#define IBM3270_EXT_BG_COLOUR     0x45
#define IBM3270_TRANSPARENT       0x46

#define IBM3270_EXT_HI_DEFAULT    0x00
#define IBM3270_EXT_HI_NORMAL     0xF0
#define IBM3270_EXT_HI_BLINK      0xF1
#define IBM3270_EXT_HI_REVERSE    0xF2
#define IBM3270_EXT_HI_USCORE     0xF4

/* 3270 AIDs */
#define IBM3270_AID_NOAID 0x60

#define IBM3270_AID_ENTER 0x7D

#define IBM3270_AID_SF    0x88

#define IBM3270_AID_F1    0xF1
#define IBM3270_AID_F2    0xF2
#define IBM3270_AID_F3    0xF3
#define IBM3270_AID_F4    0xF4
#define IBM3270_AID_F5    0xF5
#define IBM3270_AID_F6    0xF6
#define IBM3270_AID_F7    0xF7
#define IBM3270_AID_F8    0xF8
#define IBM3270_AID_F9    0xF9
#define IBM3270_AID_F10   0x7A
#define IBM3270_AID_F11   0x7B
#define IBM3270_AID_F12   0x7C
#define IBM3270_AID_F13   0xC1
#define IBM3270_AID_F14   0xC2
#define IBM3270_AID_F15   0xC3
#define IBM3270_AID_F16   0xC4
#define IBM3270_AID_F17   0xC5
#define IBM3270_AID_F18   0xC6
#define IBM3270_AID_F19   0xC7
#define IBM3270_AID_F20   0xC8
#define IBM3270_AID_F21   0xC9
#define IBM3270_AID_F22   0x4A
#define IBM3270_AID_F23   0x4B
#define IBM3270_AID_F24   0x4C

#define IBM3270_AID_PA1   0x6C
#define IBM3270_AID_PA2   0x6E
#define IBM3270_AID_PA3   0x6B


#define IBM3270_AID_CLEAR 0x6D

#define TELOPT_TN3270E 0x28 /* TN3270 Extended */

#define TN3270E_ASSOCIATE    0x00
#define TN3270E_CONNECT      0x01
#define TN3270E_DEVICE_TYPE  0x02
#define TN3270E_FUNCTIONS    0x03
#define TN3270E_IS           0x04
#define TN3270E_REASON       0x05
#define TN3270E_REJECT       0x06
#define TN3270E_REQUEST      0x07
#define TN3270E_SEND         0x08

#define TN3270E_REASON_CONN_PARTNER      0x00
#define TN3270E_REASON_DEVICE_IN_USE     0x01
#define TN3270E_REASON_INV_ASSOCIATE     0x02
#define TN3270E_REASON_INV_NAME          0x03
#define TN3270E_REASON_INV_DEVICE_TYPE   0x04
#define TN3270E_REASON_TYPE_NAME_ERROR   0x05
#define TN3270E_REASON_UNKNOWN_ERROR     0x06
#define TN3270E_REASON_UNSUPPORTED_REQ   0x07

#define TN3270E_FUNCTION_BIND_IMAGE      0x00
#define TN3270E_FUNCTION_DATA_STREAM_CTL 0x01
#define TN3270E_FUNCTION_RESPONSES       0x02
#define TN3270E_FUNCTION_SCS_CTL_CODES   0x03
#define TN3270E_FUNCTION_SYSREQ          0x04

#define TN3270E_DATATYPE_3270_DATA       0x00
#define TN3270E_DATATYPE_SCS_DATA        0x01
#define TN3270E_DATATYPE_RESPONSE        0x02
#define TN3270E_DATATYPE_BIND_IMAGE      0x03
#define TN3270E_DATATYPE_UNBIND          0x04
#define TN3270E_DATATYPE_NVT_DATA        0x05
#define TN3270E_DATATYPE_REQUEST         0x06
#define TN3270E_DATATYPE_SSCP_LU_DATA    0x07
#define TN3270E_DATATYPE_PRINT_EOJ       0x08

enum Indicators {
    Unlocked,
    SystemLock,
    TerminalWait,
    OvertypeMode,
    InsertMode,
    GoElsewhere
};

#define Q3270_RULER_CROSSHAIR   0
#define Q3270_RULER_VERTICAL    1
#define Q3270_RULER_HORIZONTAL  3

#define Q3270_TERMINAL_MODEL2   0
#define Q3270_TERMINAL_MODEL3   1
#define Q3270_TERMINAL_MODEL4   2
#define Q3270_TERMINAL_MODEL5   3
#define Q3270_TERMINAL_DYNAMIC  4

#endif // N3270_H
