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
 * DslFramer.h 
 *
 * Description:
 *	This file contains common DSL framer definitions
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: DslFramer.h,v 1.1 2008/08/25 06:41:01 l65130 Exp $
 *
 * $Log: DslFramer.h,v $
 * Revision 1.1  2008/08/25 06:41:01  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:21  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:19  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:55  z45221
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
 * Revision 1.2  2004/04/12 23:41:10  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *
 *****************************************************************************/

#ifndef	DslFramerHeader
#define	DslFramerHeader

#include "DList.h"

#define	kDslFramerInitialized			0x80000000

/* status codes */

#define	kDslFramerRxFrame				1
#define	kDslFramerRxFrameErr			2
#define kDslFramerTxFrame				3
#define	kDslFramerTxFrameErr			4

#define	kDslFramerRxFrameErrFlushed		1
#define	kDslFramerRxFrameErrAbort		2
#define	kDslFramerRxFrameErrPhy			3

#define	kDslFramerTxFrameErrFlushed		1


typedef	struct _dslFramerBufDesc {
	long		pkId;
	long		bufFlags;
	void		*bufPtr;
	long		bufLen;
} dslFramerBufDesc;

/* data bufDesc flags */

#define kDslFramerStartNewFrame			1
#define kDslFramerEndOfFrame			2
#define kDslFramerAbortFrame			4

#define kDslFramerExtraByteShift		3
#define kDslFramerExtraByteMask			(0x7 << kDslFramerExtraByteShift)

typedef struct _dslFramerControl {
	bitMap					setup;
	dslFrameHandlerType		rxIndicateHandlerPtr;
	dslFrameHandlerType		txCompleteHandlerPtr;
	dslStatusHandlerType	statusHandlerPtr;
	ulong					statusCode;
	ulong					statusOffset;

	int						nRxBuffers;
	int						nRxBufSize;
	int						nRxPackets;

	dslFrame				*freeBufListPtr;
	void					*freeBufPool;
	void					*pBufMemory;

	dslFrame				*freePacketListPtr;
	void					*freePacketPool;

	/* RX working data set */

	dslFrame				*pRxFrame;
	dslFrameBuffer			*pRxBuf;
	uchar					*pRxBufData;
	uchar					*pRxBufDataEnd;
	int						rxFrameLen;

	/* TX working data set */

	DListHeader				dlistTxWaiting;
	dslFrame				*pTxFrame;
	dslFrameBuffer			*pTxBuf;
	uchar					*pTxBufData;
	uchar					*pTxBufDataEnd;

	/* stats data */

	ulong					dslByteCntRxTotal;
	ulong					dslByteCntTxTotal;

	ulong					dslFrameCntRxTotal;
	ulong					dslFrameCntRxErr;
	ulong					dslFrameCntTxTotal;
	
} dslFramerControl;


extern Boolean  DslFramerInit(
			void					*gDslVars,
			dslFramerControl		*dfCtrl,
			bitMap					setup,
			ulong					statusCode,
			ulong					statusOffset,
			dslFrameHandlerType		rxIndicateHandlerPtr,
			dslFrameHandlerType		txCompleteHandlerPtr,
			dslStatusHandlerType	statusHandlerPtr,
			ulong					rxBufNum,
			ulong					rxBufSize,
			ulong					rxPacketNum);
extern void DslFramerClose(void *gDslVars, dslFramerControl *dfCtrl);
extern void DslFramerSendFrame(void *gDslVars, dslFramerControl *dfCtrl, dslFrame *pFrame);
extern void DslFramerReturnFrame(void *gDslVars, dslFramerControl *dfCtrl, dslFrame *pFrame);


extern Boolean DslFramerRxGetPtr(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern void	DslFramerRxDone  (void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern Boolean	DslFramerTxGetPtr(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern void	DslFramerTxDone(void *gDslVars, dslFramerControl *dfCtrl, dslFramerBufDesc *pBufDesc);
extern Boolean DslFramerTxIdle (void *gDslVars, dslFramerControl *dfCtrl);
extern void DslFramerTxFlush(void *gDslVars, dslFramerControl *dfCtrl);

extern void * DslFramerGetFramePoolHandler(dslFramerControl *dfCtrl);
extern void DslFramerClearStat(dslFramerControl *dfCtrl);

extern void DslFramerRxFlushFrame (void *gDslVars, dslFramerControl *dfCtrl, int errCode);
extern void DslFramerRxFlush(void *gDslVars, dslFramerControl *dfCtrl);

#endif	/* DslFramerHeader */
