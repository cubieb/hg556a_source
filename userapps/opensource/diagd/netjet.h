//////////////////////////////////////////////////////////////////////
//
//  EthComm2k - Virtual Ethernet COM Port service program 
//              for Windows 2000/XP
//  Copyright (c) 2003. wencai corp.
//
//////////////////////////////////////////////////////////////////////
#if !defined(__NETJET_H__)
#define __NETJET_H__

#define NETJET_COMMAND_OPEN        0x11
#define NETJET_COMMAND_CLOSE       0x12

#define NETJET_COMMAND_SET_LINE_CONTROL  0x13
#define NETJET_COMMAND_SET_BAUD_RATE       0x14

#define NETJET_COMMAND_SET_HARDLINE	        0x16
#define HARDLINE_CLRDTR                   0x00
#define HARDLINE_CLRRTS                   0x01
#define HARDLINE_SETDTR                   0x02
#define HARDLINE_SETRTS                   0x03
#define HARDLINE_SETBREAK               0x06
#define HARDLINE_CLRBREAK               0x07
#define HARDLINE_RESETDEV               0x0C

#define NETJET_COMMAND_REPORT_EVENT	      0x15
#define EVENT_MASK_CTS                    0x01
#define EVENT_MASK_DSR                    0x02
#define EVENT_MASK_RING                  0x04
#define EVENT_MASK_RLSD                  0x08
#define EVENT_MASK_ERR                    0x01

#define EVENT_DATA_CTS                    0x01
#define EVENT_DATA_DSR                    0x02
#define EVENT_DATA_RING                  0x04
#define EVENT_DATA_RLSD                  0x08
#define EVENT_DATA_BREAK                0x01

#define ERR_OE                                    0x02
#define ERR_PE                                    0x04
#define ERR_FE                                    0x08


typedef struct _NETJET_FRAME {
char          Head;
char          ID[8];
char          Type;
char          DataLen[2]; 
char          Data[1];
} NETJET_FRAME, *PNETJET_FRAME;

#endif//!defined(__NETJET_H__)
