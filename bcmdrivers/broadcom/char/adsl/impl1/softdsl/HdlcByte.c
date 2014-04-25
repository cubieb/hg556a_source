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
 * HdlcByte.c -- HDLC byte framer 
 *
 * Description:
 *	This file contains root HDLC byte framer functions
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: HdlcByte.c,v 1.1 2008/08/25 06:41:05 l65130 Exp $
 *
 * $Log: HdlcByte.c,v $
 * Revision 1.1  2008/08/25 06:41:05  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:24  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:27  z30370
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
 * Revision 1.4  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.3  2003/07/18 18:51:05  ilyas
 * Added mode (default) to pass address and control field
 *
 * Revision 1.2  2001/12/13 21:44:05  liang
 * Add newline to end of file to prevent linux compiler warning.
 *
 * Revision 1.1  2001/12/13 02:28:27  ilyas
 * Added common framer (DslPacket and G997) and G997 module
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "DslFramer.h"
#include "HdlcFramer.h"
#include "BlockUtil.h"

Public void HdlcByteReset(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	hbyCtrl->pTxData				= NULL;
	hbyCtrl->pTxDataEnd				= NULL;

	hbyCtrl->pRxData				= NULL;
	hbyCtrl->pRxDataEnd				= NULL;

	HdlcFramerTxFrameInit(hbyCtrl);
	HdlcFramerRxFrameInit(hbyCtrl);
}

Public Boolean HdlcByteInit(
	void							*gDslVars, 
	hdlcByteControl					*hbyCtrl,
	bitMap							setup, 
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler,
	dslFramerDataDoneHandlerType	rxDataDoneHandler,
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler,
	dslFramerDataDoneHandlerType	txDataDoneHandler)
{
	hbyCtrl->setup					= setup;
	hbyCtrl->rxDataGetPtrHandler	= rxDataGetPtrHandler;
	hbyCtrl->rxDataDoneHandler		= rxDataDoneHandler;
	hbyCtrl->txDataGetPtrHandler	= txDataGetPtrHandler;
	hbyCtrl->txDataDoneHandler		= txDataDoneHandler;

	HdlcByteReset(gDslVars, hbyCtrl);
	return true;
}

Private uchar * HdlcByteRxData(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *srcPtr, int nBytes, Boolean *pEof)
{
	uchar	*srcEndPtr, *dstPtr, b;
	ulong	crc;

	srcEndPtr   = srcPtr + nBytes;
	dstPtr		= hbyCtrl->pRxData;
	crc			= hbyCtrl->rxCrc;
	*pEof		= false;

	while (srcPtr != srcEndPtr) {
		b = *srcPtr++;
		if (HDLC_BYTE_FLAG == b) {
			*pEof = true;
			break;
		}

		if (HDLC_BYTE_ESC == b) {
			if (srcPtr == srcEndPtr) {
				hbyCtrl->rxEscChar = true;
				break;
			}
			else
				b = *srcPtr++ ^ 0x20;
		}
		*dstPtr++ = b;
		crc = Hdlc16UpdateCrc(crc, b);
	}

	hbyCtrl->rxCrc		= crc;
	hbyCtrl->pRxData	= dstPtr;
	return srcPtr;
}

Public int HdlcByteRx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *srcPtr)
{
	uchar					*srcEndPtr, frameState, b;
	int						n;
	Boolean					flag;
	dslFramerBufDesc		bufDesc;

	srcEndPtr  = srcPtr + nBytes;
	frameState = hbyCtrl->rxFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
				b = *srcPtr++;
				if (HDLC_BYTE_FLAG == b)
					frameState		= HDLC_STATE_ADDRESS;
				break;
			case HDLC_STATE_ADDRESS:
				if (0 == (hbyCtrl->setup & kHdlcSpecialAddrCtrl)) {
					hbyCtrl->rxCrc = HDLC16_CRC_INIT;
					goto hdlcByteStartData;
				}

				b = *srcPtr++;
				if (HDLC_BYTE_FLAG != b) {
					hbyCtrl->rxCrc = Hdlc16UpdateCrc(HDLC16_CRC_INIT, b);
					frameState  = HDLC_STATE_CONTROL;
				}
				break;
			case HDLC_STATE_CONTROL:
				b = *srcPtr++;
				if (HDLC_BYTE_FLAG == b) {
					frameState = HDLC_STATE_ADDRESS;
					break;
				}
				hbyCtrl->rxCrc = Hdlc16UpdateCrc(hbyCtrl->rxCrc, b);

hdlcByteStartData:
				if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
					HdlcFramerRxGetData(hbyCtrl, kDslFramerStartNewFrame);

				if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
					frameState = HDLC_STATE_START_FLAG;
				else {
					frameState = HDLC_STATE_DATA;
					hbyCtrl->rxFrameLen= 0;
					hbyCtrl->rxEscChar	= false;
				}
				break;

			case HDLC_STATE_DATA:
				if (hbyCtrl->rxEscChar) {
					b = *srcPtr++ ^ 0x20;
					*hbyCtrl->pRxData++ = b;
					hbyCtrl->rxCrc = Hdlc16UpdateCrc(hbyCtrl->rxCrc, b);
					hbyCtrl->rxEscChar = false;
					break;
				}

				n = hbyCtrl->pRxDataEnd - hbyCtrl->pRxData;
				if (n > (srcEndPtr - srcPtr))
					n = srcEndPtr - srcPtr;

				srcPtr = HdlcByteRxData(gDslVars, hbyCtrl, srcPtr, n, &flag);
				if (flag) {
					bufDesc.bufFlags = (HDLC16_GOOD_CRC == hbyCtrl->rxCrc ? 
						kDslFramerEndOfFrame : kDslFramerAbortFrame);
					bufDesc.bufFlags |= 2 << kDslFramerExtraByteShift;
					bufDesc.bufLen = hbyCtrl->rxDataLen - (hbyCtrl->pRxDataEnd - hbyCtrl->pRxData);

					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
					frameState = HDLC_STATE_ADDRESS;
					hbyCtrl->pRxDataEnd = hbyCtrl->pRxData;
				}
				else if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd) {
					bufDesc.bufLen = hbyCtrl->rxDataLen;
					bufDesc.bufFlags = 0;
					(hbyCtrl->rxDataDoneHandler) (gDslVars, &bufDesc);
					HdlcFramerRxGetData(hbyCtrl, 0);
					if (hbyCtrl->pRxData == hbyCtrl->pRxDataEnd)
						frameState = HDLC_STATE_START_FLAG;
				}
				break;
		}
	} while (srcPtr != srcEndPtr);

	hbyCtrl->rxFrameState = frameState;

	return nBytes;
}

Public void HdlcByteRxFlush(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	hbyCtrl->rxFrameState = HDLC_STATE_START_FLAG;
	hbyCtrl->pRxData = hbyCtrl->pRxDataEnd;
}


Private uchar * HdlcByteTxData(void *gDslVars, hdlcByteControl *hbyCtrl, uchar *dstPtr, int nBytes)
{
	uchar	*dstEndPtr, *srcPtr, b;
	ulong	crc;

	dstEndPtr   = dstPtr + nBytes;
	srcPtr		= hbyCtrl->pTxData;
	crc			= hbyCtrl->txCrc;

	do {
		b = *srcPtr++;
		crc = Hdlc16UpdateCrc(crc, b);
		if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
			*dstPtr++ = HDLC_BYTE_ESC;
			if (dstPtr == dstEndPtr) {
				hbyCtrl->txCharPending = b ^ 0x20;
				break;
			}
			else
				*dstPtr = b ^ 0x20;
		}
		else
			*dstPtr = b;
	} while (++dstPtr != dstEndPtr);

	hbyCtrl->txCrc		= crc;
	hbyCtrl->pTxData	= srcPtr;
	return	dstPtr;
}

Public int HdlcByteTx(void *gDslVars, hdlcByteControl *hbyCtrl, int nBytes, uchar *dstPtr)
{
	uchar	*dstEndPtr, b;
	uchar	frameState;
	int		n;

	dstEndPtr  = dstPtr + nBytes;
	frameState = hbyCtrl->txFrameState;

	do {
		switch (frameState) {
			case HDLC_STATE_START_FLAG:
			case HDLC_STATE_END_FLAG:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
					break;
				}

				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd)
					HdlcFramerTxGetData(hbyCtrl);

				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) {
					if (hbyCtrl->setup & kHdlcTxIdleStop) {
						*dstPtr++  = HDLC_BYTE_FLAG;
						frameState = HDLC_STATE_START_FLAG;
						nBytes = dstPtr - (dstEndPtr - nBytes);
					}
					else 
						BlockByteFill(dstEndPtr-dstPtr, HDLC_BYTE_FLAG, dstPtr);

					dstPtr = dstEndPtr;
				}
				else {
					*dstPtr++ = HDLC_BYTE_FLAG;
					frameState= HDLC_STATE_ADDRESS;
				}
				break;

			case HDLC_STATE_ADDRESS:
				if (0 == (hbyCtrl->setup & kHdlcSpecialAddrCtrl)) {
					hbyCtrl->txCrc = HDLC16_CRC_INIT;
					frameState = HDLC_STATE_DATA;
					break;
				}

				*dstPtr++ = HDLC_ADDR;
				hbyCtrl->txCrc = Hdlc16UpdateCrc(HDLC16_CRC_INIT, HDLC_ADDR);
				frameState = HDLC_STATE_CONTROL;
				break;
			case HDLC_STATE_CONTROL:
				*dstPtr++ = HDLC_CTRL;
				hbyCtrl->txCrc = Hdlc16UpdateCrc(hbyCtrl->txCrc, HDLC_CTRL);
				frameState = HDLC_STATE_DATA;
				break;
			case HDLC_STATE_FCS1:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
				}
				else {
					hbyCtrl->txCrc = HDLC16_CRC_FINAL(hbyCtrl->txCrc);
					b = hbyCtrl->txCrc & 0xFF;
					if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
						hbyCtrl->txCharPending = b ^ 0x20;
						b = HDLC_BYTE_ESC;
					}
					*dstPtr++ = b;
					frameState = HDLC_STATE_FCS2;
				}
				break;
			case HDLC_STATE_FCS2:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
				}
				else {
					b= hbyCtrl->txCrc >> 8;
					if ((HDLC_BYTE_FLAG == b) || (HDLC_BYTE_ESC == b)) {
						hbyCtrl->txCharPending = b ^ 0x20;
						b = HDLC_BYTE_ESC;
					}
					*dstPtr++ = b;
					frameState = HDLC_STATE_END_FLAG;
				}
				break;

			case HDLC_STATE_DATA:
				if (hbyCtrl->txCharPending >= 0) {
					*dstPtr++ = hbyCtrl->txCharPending;
					hbyCtrl->txCharPending = -1;
					break;
				}

				n = hbyCtrl->pTxDataEnd - hbyCtrl->pTxData;
				if (n > (dstEndPtr - dstPtr))
					n = dstEndPtr - dstPtr;
				
				dstPtr = HdlcByteTxData(gDslVars, hbyCtrl, dstPtr, n);
				if (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) {
					dslFramerBufDesc	bufDesc;

					bufDesc.bufLen = hbyCtrl->txDataLen;
					(hbyCtrl->txDataDoneHandler) (gDslVars, &bufDesc);
					if (0 == (bufDesc.bufFlags & kDslFramerEndOfFrame)) {
						HdlcFramerTxGetData(hbyCtrl);
					}
					else
						frameState = HDLC_STATE_FCS1;
				}
				break;
		}
	} while (dstPtr != dstEndPtr);

	hbyCtrl->txFrameState = frameState;

	return nBytes;
}

Public Boolean HdlcByteTxIdle(void *gDslVars, hdlcByteControl *hbyCtrl)
{
	return (hbyCtrl->pTxData == hbyCtrl->pTxDataEnd) && (HDLC_STATE_START_FLAG == hbyCtrl->txFrameState);
}
