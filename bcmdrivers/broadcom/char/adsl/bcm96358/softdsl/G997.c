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
 * G997.c -- dslpacket main module
 *
 * Description:
 *	This file contains root functions of G997
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: G997.c,v 1.1.2.1 2009/11/19 06:39:16 l43571 Exp $
 *
 * $Log: G997.c,v $
 * Revision 1.1.2.1  2009/11/19 06:39:16  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:41:05  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:23  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:26  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:56  z45221
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
 * Revision 1.10  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.9  2004/04/27 00:57:57  ilyas
 * Remove debug prints
 *
 * Revision 1.8  2004/04/27 00:27:16  ilyas
 * Implemented double buffering to ensure G.997 HDLC frame is continuous
 *
 * Revision 1.7  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.6  2003/08/04 15:38:38  yongbing
 * Undo the last check-in which will disable G.997 function
 *
 * Revision 1.5  2003/07/26 00:33:40  yjchen
 * temporarily disable G997StatusSnooper to avoid crash
 *
 * Revision 1.4  2003/07/18 18:56:59  ilyas
 * Added support for shared TX buffer (for ADSL driver)
 *
 * Revision 1.3  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.2  2002/01/11 06:48:27  ilyas
 * Added command handler pointer
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "DslFramer.h"
#include "G997.h"
#include "BlockUtil.h"
#include "AdslMibDef.h"

#define	globalVar	gG997Vars

Private void G997EocSend (void *gDslVars);
Private void G997ClearEocSendComplete (void *gDslVars, int msgType, uchar *dataPtr);

/*	G997 HDLC interface functions */

Private Boolean	G997HdlcRxGetPtr(void *gDslVars, dslFramerBufDesc *pBufDesc)
{
	return DslFramerRxGetPtr(gDslVars, &globalVar.dslFramer, pBufDesc);
}

Private void G997HdlcRxDone(void *gDslVars, dslFramerBufDesc *pBufDesc)
{
	DslFramerRxDone(gDslVars, &globalVar.dslFramer, pBufDesc);
}

Private Boolean	G997HdlcTxGetPtr(void *gDslVars, dslFramerBufDesc *pBufDesc)
{
	return DslFramerTxGetPtr(gDslVars, &globalVar.dslFramer, pBufDesc);
}

Private void G997HdlcTxDone(void *gDslVars, dslFramerBufDesc *pBufDesc)
{
	DslFramerTxDone (gDslVars, &globalVar.dslFramer, pBufDesc);
}

Public void G997Reset(void *gDslVars)
{
	HdlcByteReset(gDslVars, &globalVar.hdlcByte);
	DslFramerTxFlush(gDslVars, &globalVar.dslFramer);
	DslFramerRxFlush(gDslVars, &globalVar.dslFramer);
	globalVar.txIdle	= true;
	globalVar.timeCmdOut = 0;
	globalVar.txMsgNum	= 0;
	globalVar.rxMsgNum	= 0;
}

/*
**
**		G997 interface functions
**
*/

Public Boolean  G997Init(
			void					*gDslVars, 
			bitMap					setup, 
			ulong					rxBufNum,
			ulong					rxBufSize,
			ulong					rxPacketNum,
			upperLayerFunctions		*pUpperLayerFunctions,
			dslCommandHandlerType	g997PhyCommandHandler)
{
	Boolean					flag;

	globalVar.setup				   = setup;
	G997SetTxBuffer(gDslVars, kG997MsgBufSize, globalVar.txMsgBuf);

	flag = DslFramerInit(
			gDslVars,
			&globalVar.dslFramer,
			0,
			kDslG997Status,
			FLD_OFFSET (dslStatusStruct, param.g997Status),
			pUpperLayerFunctions->rxIndicateHandlerPtr,
			pUpperLayerFunctions->txCompleteHandlerPtr,
			pUpperLayerFunctions->statusHandlerPtr,
			rxBufNum,
			rxBufSize,
			rxPacketNum);
	if (!flag)
		return false;

	flag = HdlcByteInit(
				gDslVars,
				&globalVar.hdlcByte,
				kHdlcTxIdleStop | kHdlcCrc16,
				G997HdlcRxGetPtr,
				G997HdlcRxDone,
				G997HdlcTxGetPtr,
				G997HdlcTxDone);
	if (!flag)
		DslFramerClose(gDslVars, &globalVar.dslFramer);

	globalVar.commandHandler = g997PhyCommandHandler;
	globalVar.rxMsgNum	= 0;
	globalVar.txMsgNum	= 0;
	globalVar.txIdle	= true;
	globalVar.timeMs	= 0;
	globalVar.timeCmdOut = 0;
	return flag;
}

Public Boolean G997SetTxBuffer(void *gDslVars, ulong len, void *bufPtr)
{
	if ((0 == len) || (NULL == bufPtr))
		return false;

	globalVar.txMsgBufLen = len;
	globalVar.txMsgBufPtr = bufPtr;

	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
		globalVar.txMsgLen		= len >> 1;
		globalVar.txMsgBufs		= 2;
	}
	else {
		globalVar.txMsgLen		= len;
		globalVar.txMsgBufs		= 1;
	}
	globalVar.txMsgBufNum	= 0;

	return true;
}

Public void G997Close(void *gDslVars)
{
	DslFramerClose (gDslVars, &globalVar.dslFramer);
}
	
Public void G997Timer(void *gDslVars, long timeMs)
{
	globalVar.timeMs += timeMs;

	if (!AdslMibIsLinkActive(gDslVars)) {
		globalVar.txIdle = true;
	}

	if (globalVar.timeCmdOut && ((globalVar.timeMs - globalVar.timeCmdOut) > 5000)) {
		__SoftDslPrintf(gDslVars, "G997Timer: CMD pending too long. txIdle=%d", 0, globalVar.txIdle);
		G997ClearEocSendComplete (gDslVars, (globalVar.txMsgNum << 16), (void *) 1);
	}
}
	
Public Boolean G997CommandHandler(void *gDslVars, dslCommandStruct *cmd)
{
	return false;
}
	
Public int G997SendFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	DslFramerSendFrame (gDslVars, &globalVar.dslFramer, pFrame);
	if (globalVar.txIdle)
		G997EocSend (gDslVars);
	return 1;
}

Public int G997ReturnFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	DslFramerReturnFrame (gDslVars, &globalVar.dslFramer, pFrame);
	return 1;
}

Public void * G997GetFramePoolHandler(void *gDslVars)
{
	return DslFramerGetFramePoolHandler (&globalVar.dslFramer);
}

/*
**
**	Clear EOC mesage parsing functions
**
*/

Private void G997IncBufNum(void *gDslVars)
{
	ulong	bufNum = globalVar.txMsgBufNum + 1;

	globalVar.txMsgBufNum = (bufNum == globalVar.txMsgBufs) ? 0 : bufNum;
}

Private void G997EocSend (void *gDslVars)
{
	dslCommandStruct	cmd;
	int					len;
	uchar				*bufPtr;

	bufPtr = globalVar.txMsgBufPtr + globalVar.txMsgBufNum * globalVar.txMsgLen;
	globalVar.txIdle = false;
	len = HdlcByteTx(gDslVars, &globalVar.hdlcByte, globalVar.txMsgLen, bufPtr);
	if (len > 0) {
		G997IncBufNum(gDslVars);
		cmd.command						 = kDslSendEocCommand;
		cmd.param.dslClearEocMsg.msgId   = kDslClearEocSendFrame;
		cmd.param.dslClearEocMsg.msgType = (globalVar.txMsgNum << 16) | len;
		cmd.param.dslClearEocMsg.dataPtr = bufPtr;
		if ((*globalVar.commandHandler)(gDslVars, &cmd)) {
			globalVar.timeCmdOut = globalVar.timeMs;
			if (0 == globalVar.timeCmdOut)
				globalVar.timeCmdOut = (ulong) -1;
		}
		else {
			__SoftDslPrintf(gDslVars, "G997EocSend: Failed to write CMD, type=0x%X", 0, cmd.param.dslClearEocMsg.msgType);
			globalVar.txIdle = true;
		}
	}
	else
		globalVar.txIdle = true;
}

Private void G997ClearEocSendComplete (void *gDslVars, int msgType, uchar *dataPtr)
{
	int		n;

	n = (msgType & kDslClearEocMsgNumMask) >> 16;
	if (dataPtr != NULL) {	/* frame accepted */
#if 0
		if (n != globalVar.txMsgNum)
			AdslDrvPrintf (TEXT("G997ClearEocSendComplete: txMsgNum=%d, msgAck=%d\n"), globalVar.txMsgNum, n);
#endif
		globalVar.txMsgNum = n + 1;
	}
	else {					/* frame rejected */
		globalVar.txMsgNum = n;
		__SoftDslPrintf(gDslVars, "G997ClearEocSendComplete: FRAME REJECTED, txMsgNum=0x%8.8lx\n", globalVar.txMsgNum);
	}

	globalVar.timeCmdOut = 0;
	n = HdlcByteTxIdle(gDslVars, &globalVar.hdlcByte) && DslFramerTxIdle(gDslVars, &globalVar.dslFramer);
	if (n)
		globalVar.txIdle = true;
	else
		G997EocSend (gDslVars);
}

Private void G997ClearEocIndicateRcv (void *gDslVars, int msgType, uchar *dataPtr)
{
	int		n, len;

	n = (msgType & kDslClearEocMsgNumMask) >> 16;
	if (n != globalVar.rxMsgNum) {
		DslFramerRxFlushFrame (gDslVars, &globalVar.dslFramer, kDslFramerRxFrameErrPhy);
		HdlcByteRxFlush(gDslVars, &globalVar.hdlcByte);
	}
	globalVar.rxMsgNum = n + 1;
	len = msgType & kDslClearEocMsgLengthMask;

	HdlcByteRx(gDslVars, &globalVar.hdlcByte, len, dataPtr);
}

Public void G997StatusSnooper (void *gDslVars, dslStatusStruct *status)
{
	if (0 == (globalVar.dslFramer.setup & kDslFramerInitialized))
		return;

	switch (status->code) {
		case kDslEscapeToG994p1Status:
			G997Reset(gDslVars);
			break;

		case	kDslReceivedEocCommand:
			switch (status->param.dslClearEocMsg.msgId) {
				case kDslClearEocSendComplete:
					/* 
					** New PHY will indicate early completion with kDslClearEocSendComplete2
					** and will set kDslClearEocMsgExtraSendComplete for ADSL driver to ignore 
					** kDslClearEocSendComplete message.
					** Old PHY will only use kDslClearEocSendComplete w/o kDslClearEocMsgExtraSendComplete
					*/
					if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgExtraSendComplete)
						break;
					/* proceed if kDslClearEocMsgExtraSendComplete not set */
				case kDslClearEocSendComplete2:
					G997ClearEocSendComplete (gDslVars, status->param.dslClearEocMsg.msgType, status->param.dslClearEocMsg.dataPtr);
					break;
				case kDslClearEocRcvedFrame:
					G997ClearEocIndicateRcv (
						gDslVars,
						status->param.dslClearEocMsg.msgType,
						status->param.dslClearEocMsg.dataPtr);
					break;
			}
			break;

		case kIllegalCommand:
			if (kDslSendEocCommand == status->param.value)
				G997ClearEocSendComplete (gDslVars, globalVar.txMsgNum << 16, NULL);
			break;
		default:
			break;
	}
}
