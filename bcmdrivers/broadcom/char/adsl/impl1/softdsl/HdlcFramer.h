/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * HdlcFramer.h 
 *
 * Description:
 *	This file contains common HDLC definitions for bit/byte stuffing
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: HdlcFramer.h,v 1.1 2008/08/25 06:41:05 l65130 Exp $
 *
 * $Log: HdlcFramer.h,v $
 * Revision 1.1  2008/08/25 06:41:05  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:24  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:29  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:57  z45221
 * 合入人:
 * s48571
 *
 * 合入内容：
 * ADSL驱动更新版本至A2x020h
 * ADSLfirmware版本更换版本至a2pb023k/b2pb023k
 *
 * 修改文件列表：
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCore.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCore.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreFrame.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreFrame.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslCoreMap.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslDrvVersion.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslFile.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslFile.h
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslSelfTest.c
 * /bcmdrivers/broadcom/char/adsl/impl1/AdslSelfTest.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslCore.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslCore.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiag.h
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagLinux.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagNone.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmAdslDiagRtems.c
 * /bcmdrivers/broadcom/char/adsl/impl1/BcmOs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/Makefile
 * /bcmdrivers/broadcom/char/adsl/impl1/adsl.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adsldrv.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_defs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_lmem.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_lmem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_sdram.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348/adsl_sdram.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_defs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_lmem.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_lmem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_sdram.c
 * /bcmdrivers/broadcom/char/adsl/impl1/adslcore6348B/adsl_sdram.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslCoreDefs.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMib.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMibObj.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslMibOid.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdslXfaceData.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/BlockUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/CircBuf.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/DslFramer.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/DslFramer.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/Flatten.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/Flatten.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G992p3OvhMsg.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/G997.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcByte.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcFramer.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/HdlcFramer.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MathUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MipsAsm.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/MiscUtil.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftAtmVc.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDsl.gh
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDsl.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftDslFrame.c
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftModem.h
 * /bcmdrivers/broadcom/char/adsl/impl1/softdsl/SoftModemTypes.h
 *
 * /bcmdrivers/broadcom/includeAdslMibDef.h
 * /bcmdrivers/broadcom/includeDiagDef.h
 * /bcmdrivers/broadcom/includeadsldrv.h
 * /bcmdrivers/broadcom/includebcmadsl.h
 *
 * /userapps/broadcom/adslctl/adslctl.c
 * /userapps/broadcom/adslctl/adsluser.c
 *
 * /userapps/broadcom/cfm/html/statsadsl.html
 * /userapps/broadcom/cfm/html/countrysupport/italy_vdf/info.html
 * /userapps/broadcom/cfm/inc/syscall.h
 * /userapps/broadcom/cfm/main/adslcfgmngr/adslctlapi.c
 * /userapps/broadcom/cfm/util/system/syscall.c
 * /userapps/broadcom/cfm/web/cgimain.c
 * /userapps/broadcom/cfm/web/cgimain.h
 *
 * Revision 1.3  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.2  2003/07/18 18:51:05  ilyas
 * Added mode (default) to pass address and control field
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *
 *****************************************************************************/

#ifndef	HdlcFramerHeader
#define	HdlcFramerHeader

/* setup bitmap definitions */

#define	kHdlcSetupShift			16
#define	kHdlcSetupMask			((long)0xFFFF << kHdlcSetupShift)

#define	kHdlcCrcMask			0x00030000
#define	kHdlcCrcNone			0x00000000
#define	kHdlcCrc16				0x00010000
#define	kHdlcCrc32				0x00020000

#define	kHdlcTxIdleStop			0x00040000
#define	kHdlcSpecialAddrCtrl	0x00080000

extern	ushort HdlcCrc16Table[];

#define HDLC16_CRC_INIT			0xFFFF
#define HDLC16_CRC_FINAL(crc)	((crc) ^ 0xFFFF)
#define HDLC16_GOOD_CRC			0xF0B8
#define	Hdlc16UpdateCrc(crc,b)	((crc) >> 8) ^ HdlcCrc16Table[((crc) ^ (b)) & 0xFF]

extern	ulong  HdlcCrc32Table[];

#define HDLC32_CRC_INIT			0xFFFFFFFF
#define HDLC32_CRC_FINAL(crc)	((crc) ^ 0xFFFFFFFF)
#define HDLC32_GOOD_CRC			0xDEBB20E3
#define	Hdlc32UpdateCrc(crc,b)	((crc) >> 8) ^ HdlcCrc32Table[((crc) ^ (b)) & 0xFF]

extern	ulong  HdlcCrc32Table[];

/* HDLC common fields */

#define	HDLC_ADDR				0xFF
#define	HDLC_CTRL				0x3

#define	HDLC_BYTE_FLAG			0x7E
#define	HDLC_BYTE_ESC			0x7D

/* HDLC frame assembly states */

#define	HDLC_STATE_START_FLAG	0
#define	HDLC_STATE_ADDRESS		(HDLC_STATE_START_FLAG + 1)
#define	HDLC_STATE_CONTROL		(HDLC_STATE_START_FLAG + 2)
#define	HDLC_STATE_DATA			(HDLC_STATE_START_FLAG + 3)
#define	HDLC_STATE_FCS1			(HDLC_STATE_START_FLAG + 4)
#define	HDLC_STATE_FCS2			(HDLC_STATE_START_FLAG + 5)
#define	HDLC_STATE_END_FLAG		(HDLC_STATE_START_FLAG + 6)


/* HDLC common types */

typedef struct _hdlcByteControl {
	bitMap					setup;
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;

	/* RX working data set */

	uchar					rxFrameState;
	int						rxFrameLen;
	ulong					rxCrc;
	Boolean					rxEscChar;

	uchar					*pRxData;
	uchar					*pRxDataEnd;
	long					rxDataLen;

	/* TX working data set */

	uchar					txFrameState;
	int						txFrameLen;
	ulong					txCrc;
	int						txCharPending;

	uchar					*pTxData;
	uchar					*pTxDataEnd;
	long					txDataLen;
} hdlcByteControl;

typedef struct _hdlcBitControl {
	bitMap					setup;
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;

	/* RX working data set */

	uchar					rxFrameState;
	int						rxFrameLen;
	ulong					rxCrc;
	int						rxNibblePending;
	int						rxOutVal;
	uchar					rxOutBits;
	uchar					rxRem1Bits;
	Boolean					rxEscChar; /* ???? */

	uchar					*pRxData;
	uchar					*pRxDataEnd;
	long					rxDataLen;

	/* TX working data set */

	uchar					txFrameState;
	int						txFrameLen;
	ulong					txCrc;
	int						txOutVal;
	uchar					txOutBits;
	uchar					txLast1Bits;
	int						txCharPending;	/* ???? */

	uchar					*pTxData;
	uchar					*pTxDataEnd;
	long					txDataLen;
} hdlcBitControl;

/* HDLC common functions */

#define HdlcFramerTxFrameInit(ctrl)	do {			\
	ctrl->txFrameState = HDLC_STATE_START_FLAG;		\
	ctrl->txFrameLen   = 0;							\
	ctrl->txCrc		   = HDLC16_CRC_INIT;			\
	ctrl->txCharPending= -1;						\
} while (0)

#define HdlcFramerRxFrameInit(ctrl) do {			\
	ctrl->rxFrameState = HDLC_STATE_START_FLAG;		\
	ctrl->rxFrameLen   = 0;							\
	ctrl->rxCrc		   = HDLC16_CRC_INIT;			\
	ctrl->rxEscChar	   = false;						\
} while (0)

#define HdlcFramerTxGetData(ctrl)  do {								\
	dslFramerBufDesc	bufDesc;									\
																	\
	if ((ctrl->txDataGetPtrHandler) (gDslVars, &bufDesc)) {			\
		ctrl->pTxData = bufDesc.bufPtr;								\
		ctrl->pTxDataEnd = (uchar*)bufDesc.bufPtr + bufDesc.bufLen;	\
		ctrl->txDataLen	 = bufDesc.bufLen;							\
		if (bufDesc.bufFlags & kDslFramerStartNewFrame)				\
			HdlcFramerTxFrameInit(ctrl);							\
	}																\
	else {															\
		HdlcFramerTxFrameInit(ctrl);								\
	}																\
} while (0);

#define HdlcFramerRxGetData(ctrl, frFlag)  do {						\
	dslFramerBufDesc		bufDesc;								\
																	\
	bufDesc.bufFlags = frFlag;										\
	if ((ctrl->rxDataGetPtrHandler) (gDslVars, &bufDesc)) {			\
		ctrl->pRxData = bufDesc.bufPtr;								\
		ctrl->pRxDataEnd = (uchar*)bufDesc.bufPtr + bufDesc.bufLen;	\
		ctrl->rxDataLen	 = bufDesc.bufLen;							\
	}																\
} while (0);

/* HDLC functions headers */

extern Boolean HdlcByteInit(
	void							*gDslVars, 
	hdlcByteControl					*hbyCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler);

extern void HdlcByteReset(void *gDslVars, hdlcByteControl *hbyCtrl);
extern void HdlcByteRxFlush(void *gDslVars, hdlcByteControl *hbyCtrl);
extern int HdlcByteRx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr) FAST_TEXT;
extern int HdlcByteTx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr) FAST_TEXT;
extern Boolean HdlcByteTxIdle(void *gDslVars, hdlcByteControl *hbyCtrl);

extern Boolean HdlcBitInit(
	void							*gDslVars, 
	hdlcBitControl					*hbiCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler);

extern void HdlcBitReset(void *gDslVars, hdlcByteControl *hbiCtrl);
extern int HdlcBitRx(void *gDslVars, hdlcBitControl *hbiCtrl, int nBytes, uchar *srcPtr) FAST_TEXT;
extern int HdlcBitTx(void *gDslVars, hdlcBitControl *hbiCtrl, int nBytes, uchar *dstPtr) FAST_TEXT;

#endif	/* HdlcFramerHeader */
