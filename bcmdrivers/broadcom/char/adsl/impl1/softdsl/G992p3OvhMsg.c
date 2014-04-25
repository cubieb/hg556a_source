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
/**********************************************************************
 *
 * G992p3OvhMsg.c -- G992p3 overhead channel message processing module
 *
 * Description:
 *	This file contains mian functions of G992p3 overhead channel message 
 *  processing
 *
 *
 * Copyright (c) 1999-2003 BroadCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: G992p3OvhMsg.c,v 1.1 2008/08/25 06:41:04 l65130 Exp $
 *
 * $Log: G992p3OvhMsg.c,v $
 * Revision 1.1  2008/08/25 06:41:04  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:23  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:22  z30370
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
 * Revision 1.11  2004/09/11 03:52:24  ilyas
 * Added support for overhead message segmentation
 *
 * Revision 1.10  2004/07/21 01:39:41  ilyas
 * Reset entire G.997 state on retrain. Timeout in G.997 if no ACK
 *
 * Revision 1.9  2004/07/07 01:27:20  ilyas
 * Fixed OHC message stuck problem on L2 entry/exit
 *
 * Revision 1.8  2004/06/10 00:13:31  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.7  2004/05/07 02:58:36  ilyas
 * Commented out debug print
 *
 * Revision 1.6  2004/05/06 20:02:39  ilyas
 * Fixed include path for Linux build
 *
 * Revision 1.5  2004/05/06 03:14:44  ilyas
 * Added power management commands. Fixed multibuffer message handling
 *
 * Revision 1.4  2004/04/27 00:27:16  ilyas
 * Implemented double buffering to ensure G.997 HDLC frame is continuous
 *
 * Revision 1.3  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.2  2003/10/14 00:56:35  ilyas
 * Added stubs if G992P3 is not defined
 *
 * Revision 1.1  2003/07/18 19:39:18  ilyas
 * Initial G.992.3 overhead channel message implementation (from ADSL driver)
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "DslFramer.h"
#include "G992p3OvhMsg.h"
#include "AdslMibDef.h"
#include "BlockUtil.h"

#include "../AdslCore.h"

#undef	G992P3_POLL_OVH_MSG
#define	G992P3_POLL_OVH_MSG
#undef	G992P3_OLR_TEST
#undef	G992P3_DBG_PRINT

#define	globalVar	gG992p3OvhMsgVars

/* G.992.3 overhead message definitions */

#define	kG992p3AddrField					0
#define	kG992p3PriorityMask					0x03
#define	kG992p3PriorityHigh					0
#define	kG992p3PriorityNormal				0x01
#define	kG992p3PriorityLow					0x02

#define	kG992p3CtrlField					1
#define	kG992p3MsgNumMask					0x01
#define	kG992p3CmdRspMask					0x02
#define	kG992p3Cmd							0
#define	kG992p3Rsp							0x02

#define	kG992p3DataField					2
#define	kG992p3CmdCode						kG992p3DataField
#define	kG992p3CmdSubCode					(kG992p3CmdCode + 1)
#define	kG992p3CmdId						(kG992p3CmdCode + 2)


/* G.992.3 overhead message commands */

#define	kG992p3OvhMsgCmdNone				0
#define	kG992p3OvhMsgCmdOLR					0x01
#define	  kG992p3OvhMsgCmdOLRReq1			0x01
#define	  kG992p3OvhMsgCmdOLRReq2			0x02
#define	  kG992p3OvhMsgCmdOLRReq3			0x03
#define	  kG992p3OvhMsgCmdOLRReq4			0x04
#define	  kG992p3OvhMsgCmdOLRReq5			0x05
#define	  kG992p3OvhMsgCmdOLRReq6			0x06
#define	  kG992p3OvhMsgCmdOLRRspDefer1		0x81
#define	  kG992p3OvhMsgCmdOLRRspRej2		0x82
#define	  kG992p3OvhMsgCmdOLRRspRej3		0x83

#define	kG992p3OvhMsgCmdEOC					0x41
#define	  kG992p3OvhMsgCmdEOCSelfTest		0x01
#define	  kG992p3OvhMsgCmdEOCUpdTestParam	0x02
#define	  kG992p3OvhMsgCmdEOCStartTxCorCRC	0x03
#define	  kG992p3OvhMsgCmdEOCStopTxCorCRC	0x04
#define	  kG992p3OvhMsgCmdEOCStartRxCorCRC	0x05
#define	  kG992p3OvhMsgCmdEOCStopRxCorCRC	0x06
#define	  kG992p3OvhMsgCmdEOCAck			0x80

#define	kG992p3OvhMsgCmdTime				0x42

#define	kG992p3OvhMsgCmdCtrlRead			0x04

#define	kG992p3OvhMsgCmdInventory			0x43
#define	  kG992p3OvhMsgCmdInvId				0x01
#define	  kG992p3OvhMsgCmdInvAuxId			0x02
#define	  kG992p3OvhMsgCmdInvSelfTestRes	0x03
#define	  kG992p3OvhMsgCmdInvPmdCap			0x04
#define	  kG992p3OvhMsgCmdInvPmsTcCap		0x05
#define	  kG992p3OvhMsgCmdInvTpsTcCap		0x06

#define	kG992p3OvhMsgCmdCntRead				0x05
#define	 kG992p3OvhMsgCmdCntReadId			0x01
#define	 kG992p3OvhMsgCmdCntReadId1		0x0F			/* kG992p3OvhMsgCmdCntReadId + FIRE counters */

#define	kG992p3OvhMsgCmdPower				0x07
#define	kG992p3OvhMsgCmdClearEOC			0x08
#define	  kG992p3OvhMsgCmdClearEOCMsg		0x01
#define	  kG992p3OvhMsgCmdClearEOCAck		0x80
#define	kG992p3OvhMsgCmdNonStdFac			0x3F

#define	kG992p3OvhMsgCmdPMDRead				0x81
#define	  kG992p3OvhMsgCmdPMDSingleRead		0x01
#define	  kG992p3OvhMsgCmdPMDMultiRead		0x02
#define	  kG992p3OvhMsgCmdPMDMultiNext		0x03
#define	  kG992p3OvhMsgCmdPMDReadNACK		0x80
#define	  kG992p3OvhMsgCmdPMDSingleRdRsp	0x81
#define	  kG992p3OvhMsgCmdPMDMultiRdRsp		0x82

#define	    kG992p3OvhMsgCmdPMDChanRspLog	0x01
#define	    kG992p3OvhMsgCmdPMDQLineNoise	0x03
#define	    kG992p3OvhMsgCmdPMDSnr			0x04
#define	    kG992p3OvhMsgCmdPMDPeriod		0x05

#define	    kG992p3OvhMsgCmdPMDLnAttn		0x21
#define	    kG992p3OvhMsgCmdPMDSigAttn		0x22
#define	    kG992p3OvhMsgCmdPMDSnrMgn		0x23
#define	    kG992p3OvhMsgCmdPMDAttnDR		0x24
#define	    kG992p3OvhMsgCmdPMDNeXmtPower	0x25
#define	    kG992p3OvhMsgCmdPMDFeXmtPower	0x26
#define     kDslPLNControlReadPerBinPeakNoise   0x81
#define     kDslPLNControlReadPerBinThresholdCount   0x82
#define     kDslPLNControlReadImpulseDurationStat   0x83
#define     kDslPLNControlReadImpulseInterArrivalTimeStat   0x84
#define     kDslPLNControlStart                      1
#define     kDslPLNControlStop                       2
#define     kDslPLNControlPLNStatus                  0x85

#define	kG992p3OvhMsgCmdNonStdFacLow		0xBF

#define	kG992p3OvhSegMsgAckHi				0xF0
#define	kG992p3OvhSegMsgAckNormal			0xF1
#define	kG992p3OvhSegMsgAckLow				0xF2

/* frame flags */

#define		kG992p3OvhSegMsgMaxLen			1000

#define	kFrameBusy							0x01
#define	kFrameNextSegPending				0x02
#define	kFrameSetCmdTime					0x04
#define	kFrameSetSegTime					0x08
#define	kFrameCmdAckPending					0x10
#define	kFrameClearEoc						0x20
#define	kFramePollCmd						0x40

#define	G992p3GetFrameInfoPtr(gv,f)			(((ulong*) DslFrameGetLinkFieldAddress(gv,f)) + 2)

#define	G992p3GetFrameInfoVal(gv,f)			*G992p3GetFrameInfoPtr(gv,f)
#define	G992p3SetFrameInfoVal(gv,f,val)		*G992p3GetFrameInfoPtr(gv,f) = val;

#define	G992p3FrameIsBitSet(gv,f,mask)		(*G992p3GetFrameInfoPtr(gv,f) & mask)
#define	G992p3FrameBitSet(gv,f,mask)		(*G992p3GetFrameInfoPtr(gv,f) |= mask)
#define	G992p3FrameBitClear(gv,f,mask)		(*G992p3GetFrameInfoPtr(gv,f) &= ~mask)

#define	G992p3IsFrameBusy(gv,f)				G992p3FrameIsBitSet(gv,f,kFrameBusy)
#define	G992p3TakeFrame(gv,f)				G992p3FrameBitSet(gv,f,kFrameBusy)
#define	G992p3ReleaseFrame(gv,f)			G992p3FrameBitClear(gv,f,kFrameBusy)

/* globalVar.txFlags */

#define	kTxCmdWaitingAck					0x0010
#define	kTxCmdL3WaitingAck					0x0020
#define	kTxCmdL3RspWait						0x0040
#define	kTxCmdL0WaitingAck					0x0100

#define G992p3OvhMsgXmtRspBusy()			G992p3IsFrameBusy(gDslVars, &globalVar.txRspFrame)
#define G992p3OvhMsgXmtPwrRspBusy()			G992p3IsFrameBusy(gDslVars, &globalVar.txPwrRspFrame)
#define G992p3OvhMsgXmtCmdBusy()			G992p3IsFrameBusy(gDslVars, &globalVar.txCmdFrame)
#define G992p3OvhMsgXmtOLRCmdBusy()			G992p3IsFrameBusy(gDslVars, &globalVar.txOLRCmdFrame)

/* globalVar.setup flag */
#define kG992p3ClearEocWorkaround                 0x80000000
#define RestrictValue(n,l,r)        ((n) < (l) ? (l) : (n) > (r) ? (r) : (n))

char g992p3OvhMsgVendorId[kDslVendorIDRegisterLength+1] = "\xB5""\x00""BDCM""\x00\x00";


/* Counter read/write macros */

#define WriteCnt16(pData,cnt)	do {				\
	((uchar *)(pData))[0] = ((cnt) >> 8) & 0xFF;	\
	((uchar *)(pData))[1] = (cnt) & 0xFF;			\
} while (0)

#define WriteCnt32(pData,cnt)	do {				\
	((uchar *)(pData))[0] = (cnt) >> 24;			\
	((uchar *)(pData))[1] = ((cnt) >> 16) & 0xFF;	\
	((uchar *)(pData))[2] = ((cnt) >> 8) & 0xFF;	\
	((uchar *)(pData))[3] = (cnt) & 0xFF;			\
} while (0)

#define ReadCnt16(pData)	\
	((ulong) ((uchar *)(pData))[0] << 8) + ((uchar *)(pData))[1]

#define ReadCnt32(pData)					\
	((ulong) ((uchar *)(pData))[0] << 24) + \
	((ulong) ((uchar *)(pData))[1] << 16) + \
	((ulong) ((uchar *)(pData))[2] << 8)  + \
	((uchar *)(pData))[3]

#ifdef G992P3

#define	kPollCmdActive						0x00000001
#define	kPollCmdOnce						0x00000002
#define kPollCmdStayActive					0x00000004

#define	kSnrfReadCmdId			0
#define	kQlnReadCmdId			1
#define	kHlogReadCmdId			2
#define	kAttnReadCmdId			3
#define	kMgntCntReadCmdId		4
#define	kSnrmReadCmdId			5
#define	kMaxRateReadCmdId		6
#define	kXmtPowerReadCmdId		7
#define	kSatnReadCmdId			8
#define	kVendorIdCmdId			9

static Boolean	gPollingRateAdjusted	= false;

uchar	mgntCntReadCmd[] = { kG992p3PriorityNormal, 0, kG992p3OvhMsgCmdCntRead, kG992p3OvhMsgCmdCntReadId};
uchar	attnReadCmd[] = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDLnAttn };
uchar	satnReadCmd[] = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDSigAttn };
uchar	snrmReadCmd[] = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDSnrMgn };
uchar	maxRateReadCmd[] = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDAttnDR };
uchar	xmtPowerReadCmd[] = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDNeXmtPower };
uchar	snrfReadCmd[]  = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDSnr };
uchar	qlnReadCmd[]  = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDQLineNoise };
uchar	hlogReadCmd[]  = { kG992p3PriorityLow, 0, 
	kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDChanRspLog };
uchar	VendorIdCmd[]  = { kG992p3PriorityNormal, 0, 
	kG992p3OvhMsgCmdInventory, kG992p3OvhMsgCmdInvId };

Private void G992p3OvhMsgCompleteClearEocFrame(void *gDslVars, dslFrame *pFrame);
Private void G992p3OvhMsgPollCmdRateChange(void *gDslVars);

#ifdef G992P3_OLR_TEST

extern dslVarsStruct	acDslVars;
extern Boolean G992p3OvhMsgRcvProcessCmd(void *gDslVars, dslFrameBuffer *pBuf, uchar *pData, ulong cmdLen);

void OLRRequestTest(void)
{
	void *gDslVars = &acDslVars;
	int							i;
	dslStatusStruct				status;
	static dslOLRCarrParam2p	carTbl[8];


	__SoftDslPrintf(NULL, "OLRRequestTest:", 0);
	status.code = kDslOLRRequestStatus;
	status.param.dslOLRRequest.msgType = kOLRRequestType4;
	status.param.dslOLRRequest.nCarrs  = 8;
	for (i = 0; i < 4; i++) {
		status.param.dslOLRRequest.L[i] = (i << 8) | (i+2);
		status.param.dslOLRRequest.B[i] = i + 1;
	}
	if (status.param.dslOLRRequest.msgType < kOLRRequestType4) {
	  dslOLRCarrParam		*pCar = (void *) carTbl;
		
	  for (i = 0; i < 8; i++) {
		pCar[i].ind  = 600 + i;
		pCar[i].gain = 16 + i;
		pCar[i].gb   = (i << 4) | (i + 1);
	  }
	}
	else {
	  dslOLRCarrParam2p		*pCar = (void *) carTbl;

	  for (i = 0; i < 8; i++) {
		pCar[i].ind  = 600 + i;
		pCar[i].gain = 16 + i;
		pCar[i].gb   = (i << 4) | (i + 1);
	  }
	}

	status.param.dslOLRRequest.carrParamPtr = (void *) carTbl;
	G992p3OvhMsgStatusSnooper(gDslVars, &status);
}

void OLRResponseTest(void)
{
	void *gDslVars = &acDslVars;
	dslStatusStruct		status;

	status.code = kDslOLRResponseCmd;
	status.param.value = kOLRRejectType3;
	G992p3OvhMsgStatusSnooper(gDslVars, &status);
}

void PwrTestMsgPrint(char *hdr, dslPwrMessage *pMsg)
{
	__SoftDslPrintf(NULL, "%s: type=0x%X, val=0x%X, len=%d ptr=0x%X", 0, hdr, pMsg->msgType,
		pMsg->param.value, pMsg->param.msg.msgLen, pMsg->param.msg.msgData);
}

void PwrTestCmd1(void)
{
	void *gDslVars = &acDslVars;
	dslStatusStruct				status;

	status.code = kDslPwrMgrCmd;
	status.param.dslPwrMsg.msgType = kPwrSimpleRequest;
	status.param.dslPwrMsg.param.value = 0x19;
	PwrTestMsgPrint("PwrTestCmd1", &status.param.dslPwrMsg);
	G992p3OvhMsgStatusSnooper(gDslVars, &status);
}

void PwrTestCmd2(void)
{
	void *gDslVars = &acDslVars;
	dslStatusStruct				status;
	uchar				*p;
	int					i;

	status.code = kDslPwrMgrCmd;
	status.param.dslPwrMsg.msgType = kPwrL2Grant;
	status.param.dslPwrMsg.param.msg.msgLen = 0x40;
	status.param.dslPwrMsg.param.msg.msgData = p = AdslCoreSharedMemAlloc(0x100);
	for (i = 0; i < status.param.dslPwrMsg.param.msg.msgLen; i++)
		p[i] = i & 0xFF;

	PwrTestMsgPrint("PwrTestCmd2", &status.param.dslPwrMsg);
	G992p3OvhMsgStatusSnooper(gDslVars, &status);
}

void OvhMsgSnr(void)
{
	void *gDslVars = &acDslVars;
	char snrCmd[]  = { kG992p3PriorityLow, 0, kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDSnr };
	// char snrCmd[]  = { kG992p3PriorityLow, 0, kG992p3OvhMsgCmdPMDRead, kG992p3OvhMsgCmdPMDSingleRead, kG992p3OvhMsgCmdPMDChanRspLog };
	dslFrameBuffer	frBuf;

	DslFrameNativeBufferSetLength(&frBuf, sizeof(snrCmd));
	DslFrameNativeBufferSetAddress(&frBuf, snrCmd);
	snrCmd[kG992p3CtrlField] = globalVar.rxCmdMsgNum ^ 1;
	G992p3OvhMsgRcvProcessCmd(gDslVars, &frBuf, snrCmd, sizeof(snrCmd));
}
#endif /* G992P3_OLR_TEST */


static int FrameDataCopy(void *gDslVars, dslFrameBuffer	*pBuf, uchar *pSrc, ulong dataLen, uchar *pDest)
{
	uchar			*pData, *pDest0 = pDest, *pDestEnd = pDest + dataLen;
	int				len, n;

	len   = DslFrameBufferGetLength(gDslVars, pBuf);
	pData = DslFrameBufferGetAddress(gDslVars, pBuf);

	n = len - (pSrc - pData);
	if (n > 0) {
		memcpy (pDest, pSrc, n);
		pDest += n;
	}

	pBuf = DslFrameGetNextBuffer(gDslVars, pBuf);
	while (NULL != pBuf) {
		len   = DslFrameBufferGetLength(gDslVars, pBuf);
		pData = DslFrameBufferGetAddress(gDslVars, pBuf);
		if (len > (pDestEnd - pDest))
			len = pDestEnd - pDest;

		memcpy (pDest, pData, len);
		pDest += n;
		if (pDest == pDestEnd)
			break;

		pBuf = DslFrameGetNextBuffer(gDslVars, pBuf);
	}
	return pDest - pDest0;
}

/*
**
**		G992p3OvhMsg interface functions
**
*/

Private void G992p3OvhMsgPollCmdRateChange(void *gDslVars)
{
	g992p3PollCmdStruct *pCmd;
	
	if(gPollingRateAdjusted)
		return;

	pCmd = globalVar.pollCmd + kSnrfReadCmdId;
	pCmd->cmdFlags &= ~kPollCmdActive;
	pCmd = globalVar.pollCmd + kQlnReadCmdId;
	pCmd->cmdFlags &= ~kPollCmdActive;	
	pCmd = globalVar.pollCmd + kHlogReadCmdId;
	pCmd->cmdFlags &= ~kPollCmdActive;
	
	pCmd = globalVar.pollCmd + kAttnReadCmdId;
	pCmd->cmdFlags &= ~kPollCmdActive;
	pCmd = globalVar.pollCmd + kSatnReadCmdId;
	pCmd->cmdFlags &= ~kPollCmdActive;
	
	pCmd = globalVar.pollCmd + kMgntCntReadCmdId;
	pCmd->tmPeriod = 15000;	/* 15 Secs */	
	pCmd = globalVar.pollCmd + kSnrmReadCmdId;
	pCmd->tmPeriod = 20000;	/* 20 Secs */		
	pCmd = globalVar.pollCmd + kMaxRateReadCmdId;
	pCmd->tmPeriod = 60000;	/* 1 min. */
	pCmd = globalVar.pollCmd + kXmtPowerReadCmdId;
	pCmd->tmPeriod = 60000;	/* 1 min. */	
	
	gPollingRateAdjusted = true;
}

Private	void G992p3OvhMsgInitPollCmd(void *gDslVars, int id, uchar *cmd, ulong len, ulong period, ulong flags)
{
	g992p3PollCmdStruct		*pCmd = globalVar.pollCmd + id;

	pCmd->cmd = cmd;
	pCmd->len = len;
	pCmd->tmPeriod = period;
	pCmd->tmLastSent = globalVar.timeMs - period;
	pCmd->cmdFlags = flags;
}

extern void	AdslCoreG997FrameBufferFlush(void);
Public void	G992p3OvhMsgReset(void *gDslVars)
{
	ulong		*pFrFlags;

	globalVar.rxCmdMsgNum = 1;
	globalVar.rxRspMsgNum = 0;
	globalVar.txCmdMsgNum[kG992p3PriorityHigh] = 1;
	globalVar.txCmdMsgNum[kG992p3PriorityNormal] = 1;
	globalVar.txCmdMsgNum[kG992p3PriorityLow] = 1;
	globalVar.txCmdMsgNum[3] = 1;
	globalVar.txRspMsgNum = 1;
	globalVar.txFlags	  = 0;
	globalVar.txL0Rq	  = false;
	globalVar.txL3Rq	  = false;

	globalVar.timeCmdOut  = globalVar.timeMs;
	globalVar.cmdTryCnt	  = 0;
	globalVar.cmdNum	  = (ulong) -1;

	G992p3OvhMsgInitPollCmd(gDslVars, kSnrfReadCmdId, snrfReadCmd,sizeof(snrfReadCmd), 10000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kQlnReadCmdId, qlnReadCmd,sizeof(qlnReadCmd), 10000, kPollCmdActive | kPollCmdOnce);
	G992p3OvhMsgInitPollCmd(gDslVars, kHlogReadCmdId, hlogReadCmd,sizeof(hlogReadCmd), 10000, kPollCmdActive | kPollCmdOnce);
	G992p3OvhMsgInitPollCmd(gDslVars, kAttnReadCmdId, attnReadCmd, sizeof(attnReadCmd), 5000,  kPollCmdActive | kPollCmdOnce);
	G992p3OvhMsgInitPollCmd(gDslVars, kMgntCntReadCmdId, mgntCntReadCmd, sizeof(mgntCntReadCmd), 1000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kSnrmReadCmdId, snrmReadCmd, sizeof(snrmReadCmd), 3000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kMaxRateReadCmdId, maxRateReadCmd, sizeof(maxRateReadCmd), 8000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kXmtPowerReadCmdId, xmtPowerReadCmd, sizeof(xmtPowerReadCmd), 5000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kSatnReadCmdId, satnReadCmd, sizeof(satnReadCmd), 10000, kPollCmdActive);
	G992p3OvhMsgInitPollCmd(gDslVars, kVendorIdCmdId, VendorIdCmd, sizeof(VendorIdCmd), 10000, kPollCmdActive | kPollCmdOnce);
	globalVar.pollCmdNum = 10;

	pFrFlags = G992p3GetFrameInfoPtr(globalVar, &globalVar.txRspFrame);
	*pFrFlags = 0;
	pFrFlags = G992p3GetFrameInfoPtr(globalVar, &globalVar.txCmdFrame);
	*pFrFlags = 0;
	pFrFlags = G992p3GetFrameInfoPtr(globalVar, &globalVar.txOLRCmdFrame);
	*pFrFlags = 0;
	pFrFlags = G992p3GetFrameInfoPtr(globalVar, &globalVar.txPwrRspFrame);
	*pFrFlags = 0;
	globalVar.txSegFrameCtl.segFrame = NULL;
	
	gPollingRateAdjusted = false;
	globalVar.setup &= ~kG992p3ClearEocWorkaround;

	AdslCoreG997FrameBufferFlush();

	while(CircBufferGetReadAvail(&globalVar.txClEocFrameCB) > 0) {
		dslFrame *pFrame, **pFr;

		pFr	= CircBufferGetReadPtr(&globalVar.txClEocFrameCB);
		pFrame = *pFr;
		CircBufferReadUpdate(&globalVar.txClEocFrameCB, sizeof(dslFrame*));
		G992p3OvhMsgCompleteClearEocFrame(gDslVars, pFrame);		
	}
	
}

Public Boolean  G992p3OvhMsgInit(
			void					*gDslVars, 
			bitMap					setup, 
			dslFrameHandlerType		rxReturnFramePtr,
			dslFrameHandlerType		txSendFramePtr,
			dslFrameHandlerType		txSendCompletePtr,
			dslCommandHandlerType	commandHandler,
			dslStatusHandlerType	statusHandler)
{
	globalVar.setup				= setup;
	globalVar.txSendFramePtr	= txSendFramePtr;
	globalVar.txSendCompletePtr	= txSendCompletePtr;
	globalVar.cmdHandlerPtr		= commandHandler;
	globalVar.statusHandlerPtr	= statusHandler;

	DslFrameInit (gDslVars, &globalVar.txRspFrame);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txRspFrBuf, globalVar.txRspBuf);

	DslFrameBufferSetAddress (gDslVars, &globalVar.txPwrRspFrBuf0, globalVar.txPwrRspBuf0);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txPwrRspFrBuf0a, globalVar.txPwrRspBuf0+2);


	DslFrameInit (gDslVars, &globalVar.txCmdFrame);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txCmdFrBuf0, globalVar.txCmdBuf);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txCmdFrBuf0a, globalVar.txCmdBuf+4);
	DslFrameBufferSetLength (gDslVars, &globalVar.txCmdFrBuf0, 0);
	globalVar.lastTxCmdFrame = &globalVar.txCmdFrame;

	DslFrameInit (gDslVars, &globalVar.txOLRCmdFrame);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txOLRCmdFrBuf0, globalVar.txOLRCmdBuf);
	DslFrameBufferSetAddress (gDslVars, &globalVar.txOLRCmdFrBuf0a, globalVar.txOLRCmdBuf+4);
	DslFrameBufferSetLength (gDslVars, &globalVar.txOLRCmdFrBuf0, 0);

	CircBufferInit (&globalVar.txClEocFrameCB, globalVar.txClEocFrame, sizeof(globalVar.txClEocFrame));
	globalVar.PLNMessageReadRspFlag=0;
	globalVar.PLNPerBinMeasurementCounterRcvFlag=0;
	globalVar.PLNBroadbandMsrCounterRcvFlag=0;
	globalVar.kG992p3OvhMsgCmdPLN=0x10;
	globalVar.timeMs = 0;
	globalVar.PLNElapsedTimeMSec=0;
	globalVar.PLNActiveFlag=0;
	G992p3OvhMsgReset(gDslVars);
	return true;
}

Public void G992p3OvhMsgClose(void *gDslVars)
{
}
	
Public Boolean G992p3OvhMsgCommandHandler(void *gDslVars, dslCommandStruct *cmd)
{
	return false;
}

Private Boolean G992p3OvhMsgXmtSendFrame(void *gDslVars, dslFrame *pFrame)
{
	if (G992p3IsFrameBusy(gDslVars, pFrame))
		return false;

	G992p3TakeFrame(gDslVars, pFrame);
	pFrame->bufCnt = kG992p3OvhMsgFrameBufCnt;
	(*globalVar.txSendFramePtr)(gDslVars, NULL, 0, pFrame);
	return true;
}

Private Boolean G992p3OvhMsgXmtSendCurrSeg(void *gDslVars, g992p3SegFrameCtlStruct *pSegCtl)
{
	if (G992p3OvhMsgXmtSendFrame(gDslVars, pSegCtl->segFrame)) {
		pSegCtl->tryCnt++;
		pSegCtl->timeSegOut = globalVar.timeMs;
		G992p3FrameBitSet(gDslVars, pSegCtl->segFrame, kFrameSetSegTime);
		return true;
	}

	pSegCtl->timeSegOut = globalVar.timeMs - 800;
	return false;
}

Private void G992p3OvhMsgXmtSendNextSeg(void *gDslVars, g992p3SegFrameCtlStruct *pSegCtl)
{
	int				n, len, frLen;
	dslFrameBuffer	*pBuf, *pFirstBuf;
	uchar			*pData0, *pData1;
	dslFrame		*pFrame = pSegCtl->segFrame;

	if (G992p3IsFrameBusy(gDslVars, pFrame))
		return;
	G992p3FrameBitClear(gDslVars, pFrame, kFrameNextSegPending);

	if (0 == pSegCtl->segId) {
		pSegCtl->segFrame = NULL;
		return;
	}

	pSegCtl->segId--;
	pFirstBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	pData0    = DslFrameBufferGetAddress (gDslVars, pFirstBuf);
	if ((0 == pSegCtl->segId) || (pSegCtl->segId == (pSegCtl->segTotal-1)))
		n = 0x80 | (pSegCtl->segId << 3);
	else
		n = (pSegCtl->segId << 3);
	pData0[kG992p3CtrlField] &= 0x3;
	pData0[kG992p3CtrlField] |= n;

	DslFrameInit (gDslVars, pFrame);
	DslFrameEnqueBufferAtBack (gDslVars, pFrame, pFirstBuf);

	frLen = 0;
	len   = DslFrameBufferGetLength (gDslVars, pSegCtl->segFrBufCur);
	if ((0 == DslFrameBufferGetLength (gDslVars, &pSegCtl->segFrBuf)) && (len < (kG992p3OvhSegMsgMaxLen-10))) {
		frLen = len;
		pBuf = DslFrameGetNextBuffer(gDslVars, pSegCtl->segFrBufCur);
		DslFrameEnqueBufferAtBack (gDslVars, pFrame, pSegCtl->segFrBufCur);
		pSegCtl->segFrBufCur = pBuf;
		pData0  = DslFrameBufferGetAddress (gDslVars, pBuf);
		DslFrameBufferSetAddress (gDslVars, &pSegCtl->segFrBuf, pData0);
	}

	pData0  = DslFrameBufferGetAddress (gDslVars, pSegCtl->segFrBufCur);
	len     = DslFrameBufferGetLength (gDslVars, pSegCtl->segFrBufCur);
	pData1  = DslFrameBufferGetAddress (gDslVars, &pSegCtl->segFrBuf);
	pData1 += DslFrameBufferGetLength (gDslVars, &pSegCtl->segFrBuf);
	len    -= (pData1 - pData0);
#ifdef G992P3_DBG_PRINT
	__SoftDslPrintf(gDslVars, "G992p3OvhMsgXmtSendNextSeg: pData0=0x%X, pData1=0x%X, len=%d, frLen=%d\n", 0, 
		pData0, pData1, len, frLen);
#endif
	if (len <= 0) {
		if (NULL == (pSegCtl->segFrBufCur = DslFrameGetNextBuffer(gDslVars, pSegCtl->segFrBufCur))) {
			pSegCtl->segFrame = NULL;
			return;
		}
		pData1  = DslFrameBufferGetAddress (gDslVars, pSegCtl->segFrBufCur);
		len     = DslFrameBufferGetLength (gDslVars, pSegCtl->segFrBufCur);
	}
	DslFrameBufferSetAddress (gDslVars, &pSegCtl->segFrBuf, pData1);

	if ((frLen+len) > kG992p3OvhSegMsgMaxLen)
		len = kG992p3OvhSegMsgMaxLen - frLen;
	DslFrameBufferSetLength (gDslVars, &pSegCtl->segFrBuf, len);

	DslFrameEnqueBufferAtBack (gDslVars, pFrame, &pSegCtl->segFrBuf);
	pSegCtl->tryCnt = 0;
	if (G992p3OvhMsgXmtSendCurrSeg(gDslVars, pSegCtl) && (0 == pSegCtl->segId)) {
		if (G992p3FrameIsBitSet(gDslVars, pFrame, kFrameCmdAckPending)) {
			G992p3FrameBitClear(gDslVars, pFrame, kFrameCmdAckPending);
			globalVar.txFlags |= kTxCmdWaitingAck;
		}
		pSegCtl->segFrame = NULL;
	}
}

Private void G992p3OvhMsgXmtSendSegFrame(void *gDslVars, g992p3SegFrameCtlStruct *pSegCtl, dslFrame *pFrame)
{
	int				len0, len;
	dslFrameBuffer	*pBuf;
	uchar			*pData;

	pBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	len0 = DslFrameBufferGetLength(gDslVars, pBuf);
	len  = DslFrameGetLength(gDslVars, pFrame);
	pSegCtl->segTotal = (len + (len0 + kG992p3OvhSegMsgMaxLen - 1)) / (len0 + kG992p3OvhSegMsgMaxLen);
	if (pSegCtl->segTotal > 8)
		return;
	pSegCtl->segId = pSegCtl->segTotal;

	pData = DslFrameBufferGetAddress (gDslVars, pBuf);
	pSegCtl->segFrBufCur = DslFrameGetNextBuffer(gDslVars, pBuf);

	pData = DslFrameBufferGetAddress (gDslVars, pSegCtl->segFrBufCur);
	DslFrameBufferSetAddress (gDslVars, &pSegCtl->segFrBuf, pData);
	DslFrameBufferSetLength (gDslVars, &pSegCtl->segFrBuf, 0);

	globalVar.txSegFrameCtl.segFrame = pFrame;
	G992p3OvhMsgXmtSendNextSeg(gDslVars, pSegCtl);
}

Private void G992p3OvhMsgXmtSendLongFrame(void *gDslVars, dslFrame *pFrame)
{
	if (DslFrameGetLength(gDslVars, pFrame) <= 1024) {
		G992p3OvhMsgXmtSendFrame(gDslVars, pFrame);
		return;
	}

	if (NULL != globalVar.txSegFrameCtl.segFrame) {
		__SoftDslPrintf(gDslVars, "G992p3OvhMsgXmtSendLongFrame: txSegFrameCtl BUSY, fr=0x%X\n", 0, (int) pFrame);
		return;
	}

	G992p3OvhMsgXmtSendSegFrame(gDslVars, &globalVar.txSegFrameCtl, pFrame);
}

Private void G992p3OvhMsgXmtSendRsp(void *gDslVars, int rspLen)
{
	if (rspLen != -1)
		DslFrameBufferSetLength (gDslVars, &globalVar.txRspFrBuf, rspLen);

	DslFrameInit (gDslVars, &globalVar.txRspFrame);
	if (rspLen < 1020) {
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txRspFrame, &globalVar.txRspFrBuf);
		G992p3OvhMsgXmtSendFrame(gDslVars, &globalVar.txRspFrame);
	}
	else {
		DslFrameBufferSetLength (gDslVars, &globalVar.txRspFrBuf, 4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txRspFrame, &globalVar.txRspFrBuf);

		DslFrameBufferSetAddress (gDslVars, &globalVar.txRspFrBuf1, globalVar.txRspBuf + 4);
		DslFrameBufferSetLength (gDslVars, &globalVar.txRspFrBuf1, rspLen - 4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txRspFrame, &globalVar.txRspFrBuf1);
		G992p3OvhMsgXmtSendLongFrame(gDslVars, &globalVar.txRspFrame);
	}
}

Private void G992p3OvhMsgXmtTransferOLRCmd(void * gDslVars, dslFrame * pFrame)
{
	G992p3OvhMsgXmtSendLongFrame(gDslVars, pFrame);

	G992p3FrameBitSet(gDslVars, pFrame, kFrameSetCmdTime);
}

Private void G992p3OvhMsgXmtTransferCmd(void *gDslVars, dslFrame *pFrame)
{
	G992p3OvhMsgXmtSendLongFrame(gDslVars, pFrame);

	G992p3FrameBitSet(gDslVars, pFrame, kFrameSetCmdTime);
	globalVar.timeCmdOut = globalVar.timeMs;
}

Private uchar G992p3OvhMsgGetCmdNum(void *gDslVars, uchar *msgHdr, Boolean bNewCmd)
{
	int		n = msgHdr[kG992p3AddrField] & kG992p3PriorityMask;
	
	if( (globalVar.setup & kG992p3ClearEocWorkaround) != 0 )
		n = 0;

	if (bNewCmd)
		globalVar.txCmdMsgNum[n] ^= 1;
	return globalVar.txCmdMsgNum[n];
}

Private void G992p3OvhMsgXmtResendCmd(void *gDslVars)
{
	if (!G992p3OvhMsgXmtCmdBusy()) {
		if( (globalVar.setup & kG992p3ClearEocWorkaround) != 0 ) {
			dslFrameBuffer			*pBuf;
			uchar					*pData;

			pBuf  = DslFrameGetFirstBuffer(gDslVars, globalVar.lastTxCmdFrame);
			pData = DslFrameBufferGetAddress(gDslVars, pBuf);
			pData[kG992p3CtrlField] = kG992p3Cmd | G992p3OvhMsgGetCmdNum(gDslVars, pData, true);
		}
		globalVar.cmdTryCnt++;
		__SoftDslPrintf(gDslVars, "G992p3OvhMsgXmtResendCmd: cnt=%d, frame=0x%X", 0, globalVar.cmdTryCnt, globalVar.lastTxCmdFrame);
		G992p3OvhMsgXmtTransferCmd(gDslVars, globalVar.lastTxCmdFrame);
	}
}

Private Boolean G992p3OvhMsgXmtSendCmd(
	void	*gDslVars, 
	uchar	*pData0, 
	ulong	cmdLen0, 
	uchar	*pData1, 
	ulong	cmdLen1, 
	Boolean bNewCmd,
	Boolean bWaitRsp
	)
{
	int		i;

	if (G992p3OvhMsgXmtCmdBusy())
		return false;

	if (bWaitRsp) {
		globalVar.txFlags |= kTxCmdWaitingAck;
		if (G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc))
			G992p3OvhMsgCompleteClearEocFrame(gDslVars, globalVar.lastTxCmdFrame);
		globalVar.lastTxCmdFrame = &globalVar.txCmdFrame;
		if (bNewCmd)
			globalVar.cmdTryCnt = 1;
	}
	else
		globalVar.txFlags &= ~kTxCmdWaitingAck;
	G992p3FrameBitClear(gDslVars, globalVar.lastTxCmdFrame, kFramePollCmd);

	globalVar.txCmdBuf[kG992p3AddrField] = pData0[kG992p3AddrField];
	globalVar.txCmdBuf[kG992p3CtrlField] = kG992p3Cmd | G992p3OvhMsgGetCmdNum(gDslVars, pData0, bNewCmd);
	for (i = kG992p3CmdCode; i < cmdLen0; i++)
		globalVar.txCmdBuf[i] = pData0[i];

	DslFrameInit (gDslVars, &globalVar.txCmdFrame);

	if ((cmdLen0 + cmdLen1) <= 1024) {
		DslFrameBufferSetLength (gDslVars, &globalVar.txCmdFrBuf0, cmdLen0);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txCmdFrame, &globalVar.txCmdFrBuf0);
	}
	else {
		DslFrameBufferSetLength (gDslVars, &globalVar.txCmdFrBuf0, 4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txCmdFrame, &globalVar.txCmdFrBuf0);
		DslFrameBufferSetLength (gDslVars, &globalVar.txCmdFrBuf0a, cmdLen0-4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txCmdFrame, &globalVar.txCmdFrBuf0a);
		if (bWaitRsp)
			G992p3FrameBitClear(gDslVars, &globalVar.txCmdFrame, kFrameCmdAckPending);
		globalVar.txFlags &= ~kTxCmdWaitingAck;
	}

	if (cmdLen1 != 0) {
		DslFrameBufferSetAddress (gDslVars, &globalVar.txCmdFrBuf1, pData1);
		DslFrameBufferSetLength (gDslVars, &globalVar.txCmdFrBuf1, cmdLen1);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txCmdFrame, &globalVar.txCmdFrBuf1);
	}

	G992p3OvhMsgXmtTransferCmd(gDslVars, &globalVar.txCmdFrame);
	return true;
}

Private Boolean G992p3OvhMsgXmtSendOLRCmd(
	void	*gDslVars, 
	uchar	*pData0, 
	ulong	cmdLen0, 
	uchar	*pData1, 
	ulong	cmdLen1, 
	Boolean bNewCmd
	)
{
	int		i;

	if (G992p3OvhMsgXmtOLRCmdBusy())
		return false;

	globalVar.txOLRCmdBuf[kG992p3AddrField] = pData0[kG992p3AddrField];
	globalVar.txOLRCmdBuf[kG992p3CtrlField] = kG992p3Cmd | G992p3OvhMsgGetCmdNum(gDslVars, pData0, bNewCmd);
	for (i = kG992p3CmdCode; i < cmdLen0; i++)
		globalVar.txOLRCmdBuf[i] = pData0[i];

	DslFrameInit (gDslVars, &globalVar.txOLRCmdFrame);

	if ((cmdLen0 + cmdLen1) <= 1024) {
		DslFrameBufferSetLength (gDslVars, &globalVar.txOLRCmdFrBuf0, cmdLen0);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txOLRCmdFrame, &globalVar.txOLRCmdFrBuf0);
	}
	else {
		DslFrameBufferSetLength (gDslVars, &globalVar.txOLRCmdFrBuf0, 4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txOLRCmdFrame, &globalVar.txOLRCmdFrBuf0);
		DslFrameBufferSetLength (gDslVars, &globalVar.txOLRCmdFrBuf0a, cmdLen0-4);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txOLRCmdFrame, &globalVar.txOLRCmdFrBuf0a);
	}

	if (cmdLen1 != 0) {
		DslFrameBufferSetAddress (gDslVars, &globalVar.txOLRCmdFrBuf1, pData1);
		DslFrameBufferSetLength (gDslVars, &globalVar.txOLRCmdFrBuf1, cmdLen1);
		DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txOLRCmdFrame, &globalVar.txOLRCmdFrBuf1);
	}

	G992p3OvhMsgXmtTransferOLRCmd(gDslVars, &globalVar.txOLRCmdFrame);
	return true;
}

Private void G992p3OvhMsgSendClearEocCmd(void *gDslVars, dslFrame *pFrame)
{
	dslFrameBuffer			*pBuf;
	uchar					*eocHdr;

	pBuf   = DslFrameGetFirstBuffer(gDslVars, pFrame);
	eocHdr = DslFrameBufferGetAddress(gDslVars, pBuf);

	eocHdr[kG992p3AddrField] = kG992p3PriorityNormal;
	eocHdr[kG992p3CtrlField] = kG992p3Cmd | G992p3OvhMsgGetCmdNum(gDslVars, eocHdr, true);
	eocHdr[kG992p3CmdCode] = kG992p3OvhMsgCmdClearEOC;
	eocHdr[kG992p3CmdSubCode] = 1;

	globalVar.txFlags |= kTxCmdWaitingAck;
	globalVar.lastTxCmdFrame = pFrame;
	globalVar.cmdTryCnt = 1;
	G992p3FrameBitSet(gDslVars, pFrame, kFrameClearEoc);
	G992p3OvhMsgXmtTransferCmd(gDslVars, pFrame);
}

Private void G992p3OvhMsgCompleteClearEocFrame(void *gDslVars, dslFrame *pFrame)
{
	G992p3FrameBitClear(gDslVars, pFrame, kFrameClearEoc);
	(*globalVar.txSendCompletePtr)(gDslVars, NULL, 0, pFrame);
	globalVar.lastTxCmdFrame = &globalVar.txCmdFrame;
	__SoftDslPrintf(gDslVars, "G992p3OvhMsgCompleteClearEocFrame: pFrame=0x%X", 0, pFrame);
}

Public Boolean G992p3OvhMsgSendClearEocFrame(void *gDslVars, dslFrame *pFrame)
{
	dslFrame **pFr;

	if (CircBufferGetWriteAvail(&globalVar.txClEocFrameCB) < sizeof(dslFrame*))
		return false;

	pFr = CircBufferGetWritePtr(&globalVar.txClEocFrameCB);
	*pFr = pFrame;
	CircBufferWriteUpdate(&globalVar.txClEocFrameCB, sizeof(dslFrame*));
	return true;
}

Public void G992p3OvhMsgTimer(void *gDslVars, long timeMs)
{
	dslStatusStruct		status;

	if (!ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
		return;

	globalVar.timeMs += timeMs;
	globalVar.cmdTime += timeMs;
	if(globalVar.PLNActiveFlag==1)
	  globalVar.PLNElapsedTimeMSec+=timeMs;
	if (!AdslMibIsLinkActive(gDslVars))
		return;

	/* Handle pending transition to L3 */

	if ((globalVar.txFlags & kTxCmdL3RspWait) && 
		((globalVar.timeMs - globalVar.timeRspOut) > 1000)) {
		status.code = kDslConnectInfoStatus;
		status.param.dslConnectInfo.code  = kG992p3PwrStateInfo;
		status.param.dslConnectInfo.value = 3;
		(*globalVar.statusHandlerPtr)(gDslVars, &status);
		globalVar.txFlags &= ~kTxCmdL3RspWait;
	}

	if (NULL != globalVar.txSegFrameCtl.segFrame) {
	  if (G992p3FrameIsBitSet(gDslVars, globalVar.txSegFrameCtl.segFrame, kFrameNextSegPending))
		G992p3OvhMsgXmtSendNextSeg(gDslVars, &globalVar.txSegFrameCtl);
	  else if ((globalVar.timeMs - globalVar.txSegFrameCtl.timeSegOut) > 1000) {
		if (globalVar.txSegFrameCtl.tryCnt >= 3)
			globalVar.txSegFrameCtl.segFrame = NULL;
		else
			G992p3OvhMsgXmtSendCurrSeg(gDslVars, &globalVar.txSegFrameCtl);
	  }
	  return;
	}

	if (globalVar.txFlags & kTxCmdWaitingAck) {
		int tmp=0;
		
		if( ((globalVar.setup & kG992p3ClearEocWorkaround) != 0) &&
		     !G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc) &&
		     (CircBufferGetReadAvail(&globalVar.txClEocFrameCB) > 0) )  {
			tmp=globalVar.timeCmdOut;
			globalVar.timeCmdOut = 0;
			globalVar.cmdTryCnt = 3;
		}
		
		if ((globalVar.timeMs - globalVar.timeCmdOut) > 1000) {
			if (globalVar.cmdTryCnt >= 3) {
				if(!tmp)
					__SoftDslPrintf(gDslVars, "G992p3OvhMsg Timeout: cnt=%d, frame=0x%X", 0, globalVar.cmdTryCnt, globalVar.lastTxCmdFrame);
				else {
					__SoftDslPrintf(gDslVars, "G992p3OvhMsg Terminate restransmit of frame=0x%X", 0, globalVar.lastTxCmdFrame);
					globalVar.timeCmdOut = tmp;					
				}
				
				globalVar.cmdTryCnt = 0;
				globalVar.txFlags &= ~kTxCmdWaitingAck;
				if (G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc))
					G992p3OvhMsgCompleteClearEocFrame(gDslVars, globalVar.lastTxCmdFrame);

				if (globalVar.txFlags & kTxCmdL3WaitingAck) {
					status.code = kDslConnectInfoStatus;
					status.param.dslConnectInfo.code  = kG992p3PwrStateInfo;
					status.param.dslConnectInfo.value = 3;
					(*globalVar.statusHandlerPtr)(gDslVars, &status);
					globalVar.txFlags &= ~kTxCmdL3WaitingAck;
				}
					
				globalVar.txFlags &= ~kTxCmdL0WaitingAck;

				if (G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFramePollCmd)
					&& ((ulong) -1 != globalVar.cmdNum && !(globalVar.pollCmd[globalVar.cmdNum].cmdFlags & kPollCmdStayActive))) {
					__SoftDslPrintf(gDslVars, "G992p3OvhMsg (TO) Disable command num=%d", 0, globalVar.cmdNum);
					globalVar.pollCmd[globalVar.cmdNum].cmdFlags &= ~kPollCmdActive;
				}
			}
			else 
			{
				G992p3OvhMsgXmtResendCmd(gDslVars);
			}
		}

		return;
	}

	if (!G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc) &&
		(CircBufferGetReadAvail(&globalVar.txClEocFrameCB) > 0)) {
		dslFrame *pFrame, **pFr;

		pFr	= CircBufferGetReadPtr(&globalVar.txClEocFrameCB);
		pFrame = *pFr;
		CircBufferReadUpdate(&globalVar.txClEocFrameCB, sizeof(dslFrame*));
		G992p3OvhMsgSendClearEocCmd(gDslVars, pFrame);
		return;
	}

#ifdef G992P3_POLL_OVH_MSG
	if ((globalVar.timeMs - globalVar.timeCmdOut) > 200) {
		g992p3PollCmdStruct		*pCmd;
		ulong					lastCmd, cmd;

		cmd = globalVar.cmdNum;
		if (++cmd >= globalVar.pollCmdNum)
			cmd = 0;
		lastCmd = cmd;
		pCmd = &globalVar.pollCmd[cmd];

		while (!((pCmd->cmdFlags & kPollCmdActive) && ((globalVar.timeMs - pCmd->tmLastSent) > pCmd->tmPeriod))) {
			if (++cmd >= globalVar.pollCmdNum)
				cmd = 0;
			if (cmd == lastCmd) {
				cmd = (ulong) -1;
				break;
			}
			pCmd = &globalVar.pollCmd[cmd];
		}
		globalVar.cmdNum = cmd;
		if (((ulong) -1 != cmd) && G992p3OvhMsgXmtSendCmd(gDslVars, pCmd->cmd,pCmd->len, NULL,0, true,true)) {
			G992p3FrameBitSet(gDslVars, globalVar.lastTxCmdFrame, kFramePollCmd);
			pCmd->tmLastSent = globalVar.timeMs;
		}
	}
#endif
}
	
	
Public int G992p3OvhMsgSendCompleteFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	Boolean		bSeg;

	if (!ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
		return 0;
#ifdef G992P3_DBG_PRINT
	__SoftDslPrintf(gDslVars, " G992p3OvhMsgSendCompleteFrame(%s): txFlags=0x%X, pVc=0x%X mid=0x%X pFr=0x%X\n", 0, 
		 (pFrame == &globalVar.txCmdFrame) ? "CMD" : 
		((pFrame == &globalVar.txRspFrame) ? "RSP" : 
		((pFrame == &globalVar.txPwrRspFrame) ? "RSPPWR" : 
		"UNKNOWN")),
		globalVar.txFlags, (int) pVc, mid, (int) pFrame);
#endif
	G992p3ReleaseFrame (gDslVars, pFrame);

	if ((bSeg = G992p3FrameIsBitSet(gDslVars, pFrame, kFrameSetSegTime))) {
		G992p3FrameBitClear(gDslVars, pFrame, kFrameSetSegTime);
		globalVar.txSegFrameCtl.timeSegOut = globalVar.timeMs;
	}
	if (G992p3FrameIsBitSet(gDslVars, pFrame, kFrameSetCmdTime)) {
		if (!bSeg)
			G992p3FrameBitClear(gDslVars, pFrame, kFrameSetCmdTime);
		globalVar.timeCmdOut = globalVar.timeMs;
	}

	return 1;
}

Private Boolean G992p3OvhMsgRcvProcessRsp(void *gDslVars, dslFrameBuffer *pBuf, uchar *pData, ulong rspLen)
{
	uchar					*pMsg;
	ulong					mibLen;
	adslMibInfo				*pMib;
	uchar					rspId;
	g992p3PollCmdStruct		*pCmd = &globalVar.pollCmd[globalVar.cmdNum];
	char					oidStr[] = { kOidAdslPrivate, 0 };
	short					*pToneData;
	uchar					*pMsgEnd;
#if 0 && defined(G992P3_DBG_PRINT)
	__SoftDslPrintf(gDslVars, "G992p3OvhMsgRcvProcessRsp: code=0x%X, subCode=0x%X\n", 0,
		pData[kG992p3CmdCode], pData[kG992p3CmdSubCode]);
#endif
	switch (pData[kG992p3CmdCode]) {
		case kG992p3OvhMsgCmdCntRead:
			if (!(pCmd->cmdFlags & kPollCmdOnce) && !(pCmd->cmdFlags & kPollCmdStayActive))
				pCmd->cmdFlags |= kPollCmdStayActive;

			mibLen = sizeof(adslMibInfo);
			pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);

			if (((kG992p3OvhMsgCmdCntReadId | 0x80) == pData[kG992p3CmdSubCode] ) ||
			     ((pMib->adslStat.fireStat.status & kFireUsEnabled) && ((kG992p3OvhMsgCmdCntReadId1| 0x80) == pData[kG992p3CmdSubCode] )) ) {
				adslConnectionDataStat		adslTxData;
				adslPerfCounters			adslTxPerf;
				atmConnectionDataStat		atmTxData;
				
				pMsg = pData + kG992p3CmdSubCode + 1;

				adslTxData.cntRSCor = ReadCnt32(pMsg+0);
				adslTxData.cntSFErr = ReadCnt32(pMsg+4);

				adslTxPerf.adslFECs  = ReadCnt32(pMsg+8);
				adslTxPerf.adslESs   = ReadCnt32(pMsg+12);
				adslTxPerf.adslSES   = ReadCnt32(pMsg+16);
				adslTxPerf.adslLOSS  = ReadCnt32(pMsg+20);
				adslTxPerf.adslUAS   = ReadCnt32(pMsg+24);

				atmTxData.cntHEC	   = ReadCnt32(pMsg+28);
				atmTxData.cntCellTotal = ReadCnt32(pMsg+32);
				atmTxData.cntCellData  = ReadCnt32(pMsg+36);
				atmTxData.cntBitErrs   = ReadCnt32(pMsg+40);
				AdslMibUpdateTxStat(gDslVars, &adslTxData, &adslTxPerf, &atmTxData);
				
				if( (kG992p3OvhMsgCmdCntReadId1| 0x80) == pData[kG992p3CmdSubCode] ) {
					pMib->adslStat.fireStat.reXmtRSCodewordsRcvedUS = ReadCnt32(pMsg+44);
					pMib->adslStat.fireStat.reXmtUncorrectedRSCodewordsUS= ReadCnt32(pMsg+48);
					pMib->adslStat.fireStat.reXmtCorrectedRSCodewordsUS= ReadCnt32(pMsg+52);
				}
			}
			break;
		case kG992p3OvhMsgCmdInventory:
			if (pCmd->cmdFlags & kPollCmdOnce) {
				pCmd->cmdFlags &= ~kPollCmdActive;
				__SoftDslPrintf(gDslVars, "G992p3OvhMsg (ONCE) Disable command num=%d %x %x", 0, globalVar.cmdNum,pCmd->cmd[kG992p3CmdCode+1],pData[kG992p3CmdCode+1]);
			}
			pMsg = pData + kG992p3CmdSubCode +1;
			pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			switch(pData[kG992p3CmdSubCode]){
			case 0x81:{
				memcpy(pMib->adslAtucPhys.adslVendorID,pMsg,8);
				pMsg+=8;
				memcpy(pMib->adslAtucPhys.adslVersionNumber,pMsg,16);
				pMsg+=16;	
				memcpy(pMib->adslAtucPhys.adslSerialNumber,pMsg,32);
				pMsg+=32;
				}
				break;
			}
			break;
				
		case kG992p3OvhMsgCmdPMDRead:
			if (((ulong) -1 == globalVar.cmdNum) ||
				(kG992p3OvhMsgCmdPMDRead != pCmd->cmd[kG992p3CmdCode]) ||
				(kG992p3OvhMsgCmdPMDSingleRead != pCmd->cmd[kG992p3CmdCode+1]))
				break;

			if (kG992p3OvhMsgCmdPMDReadNACK == pData[kG992p3CmdSubCode] && !(pCmd->cmdFlags & kPollCmdStayActive)) {
				pCmd->cmdFlags &= ~kPollCmdActive;
				__SoftDslPrintf(gDslVars, "G992p3OvhMsg (NACK) Disable command num=%d", 0, globalVar.cmdNum);
				break;
			}

			if (kG992p3OvhMsgCmdPMDSingleRdRsp != pData[kG992p3CmdSubCode])
				break;

			if (pCmd->cmdFlags & kPollCmdOnce) {
				pCmd->cmdFlags &= ~kPollCmdActive;
				__SoftDslPrintf(gDslVars, "G992p3OvhMsg (ONCE) Disable command num=%d", 0, globalVar.cmdNum);
			}
			
			if (!(pCmd->cmdFlags & kPollCmdOnce) && !(pCmd->cmdFlags & kPollCmdStayActive))
				pCmd->cmdFlags |= kPollCmdStayActive;

			pMsg = pData + kG992p3CmdSubCode + 1;
			mibLen = sizeof(adslMibInfo);
			pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			rspId = pCmd->cmd[kG992p3CmdCode+2];
#if 0 && defined(G992P3_DBG_PRINT)
			__SoftDslPrintf(gDslVars, "PMDReadRsp: rspId = 0x%X, rspLen=%d", 0, rspId, rspLen);
#endif
			switch (rspId) {
				case kG992p3OvhMsgCmdPMDLnAttn:
					pMib->adslAtucPhys.adslCurrAtn = ReadCnt16(pMsg);
					__SoftDslPrintf(gDslVars, "US LATN: %d/10", 0, pMib->adslAtucPhys.adslCurrAtn);
					break;
				case kG992p3OvhMsgCmdPMDSigAttn:
					pMib->adslAtucPhys.adslSignalAttn = ReadCnt16(pMsg);
					__SoftDslPrintf(gDslVars, "US SATN: %d/10", 0, pMib->adslAtucPhys.adslSignalAttn);
					break;
				case kG992p3OvhMsgCmdPMDSnrMgn:
					pMib->adslAtucPhys.adslCurrSnrMgn = ReadCnt16(pMsg);
					__SoftDslPrintf(gDslVars, "US SNRM: %d/10", 0, pMib->adslAtucPhys.adslCurrSnrMgn);
					break;
				case kG992p3OvhMsgCmdPMDAttnDR:
					pMib->adslAtucPhys.adslCurrAttainableRate = ReadCnt32(pMsg);
					__SoftDslPrintf(gDslVars, "US ATTNDR: %d Kbps", 0, pMib->adslAtucPhys.adslCurrAttainableRate/1000);
					break;
				case kG992p3OvhMsgCmdPMDNeXmtPower:
					{
					int n=ReadCnt16(pMsg);
					pMib->adslAtucPhys.adslCurrOutputPwr = (n << (32-10)) >> (32-10);
					}
					__SoftDslPrintf(gDslVars, "DS XmtPwr: %d/10", 0, pMib->adslAtucPhys.adslCurrOutputPwr);
					break;

				case kG992p3OvhMsgCmdPMDSnr:
					mibLen = 0;
					oidStr[1] = kOidAdslPrivSNR;
					pToneData = (void *) AdslMibGetObjectValue(gDslVars, oidStr,sizeof(oidStr), NULL, &mibLen);
					pMsg += 2;
					pMsgEnd = pData + rspLen;
					while (pMsg != pMsgEnd) {
						if (*pMsg != 255)
							*pToneData = (((short) pMsg[0]) << 3) - 32*16;
						else
							*pToneData = 0;
						pToneData++;
						pMsg++;
					}
					break;

				case kG992p3OvhMsgCmdPMDQLineNoise:
					mibLen = 0;
					oidStr[1] = kOidAdslPrivQuietLineNoise;
					pToneData = (void *) AdslMibGetObjectValue(gDslVars, oidStr,sizeof(oidStr), NULL, &mibLen);
					pMsg += 2;
					pMsgEnd = pData + rspLen;
					while (pMsg != pMsgEnd) {
						if (*pMsg != 255)
							*pToneData = -(((short) pMsg[0]) << 3) - 23*16;
						else
							*pToneData = -150*16;
						pToneData++;
						pMsg++;
					}
					break;

				case kG992p3OvhMsgCmdPMDChanRspLog:
					mibLen = 0;
					oidStr[1] = kOidAdslPrivChanCharLog;
					pToneData = (void *) AdslMibGetObjectValue(gDslVars, oidStr,sizeof(oidStr), NULL, &mibLen);
					pMsg += 2;
					pMsgEnd = pData + ((rspLen + 1) & ~0x1);
					while (pMsg != pMsgEnd) {
						int		n;

						n = ((int) (pMsg[0] & 0x3) << 12) + (pMsg[1] << 4);
						n = -(n/10) + 6*16;
						*pToneData++ = n;
						pMsg += 2;
					}
					break;
				default:
					break;
			}
			break;

		case kG992p3OvhMsgCmdPower:
			
			if (globalVar.txFlags & kTxCmdL3WaitingAck) {
				dslStatusStruct		status;

				status.code = kDslConnectInfoStatus;
				status.param.dslConnectInfo.code = kG992p3PwrStateInfo;
				if (kPwrGrant == pData[kG992p3CmdSubCode])
					status.param.dslConnectInfo.value = 3;
				else 
					status.param.dslConnectInfo.value = kPwrReject;
				(*globalVar.statusHandlerPtr)(gDslVars, &status);
				globalVar.txFlags &= ~kTxCmdL3WaitingAck;
			}

			globalVar.txFlags &= ~kTxCmdL0WaitingAck;
			
			break;

		default:
			break;
	}

	if(!(G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc))   &&
	    !(globalVar.txFlags & kTxCmdL0WaitingAck) &&
	    !(globalVar.txFlags & kTxCmdL3WaitingAck)) {
	    
	    globalVar.txFlags &= ~kTxCmdWaitingAck;
		
	}
	else if( G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc) &&
		kG992p3OvhMsgCmdClearEOC == pData[kG992p3CmdCode] ) {
		globalVar.txFlags &= ~kTxCmdWaitingAck;
		G992p3OvhMsgCompleteClearEocFrame(gDslVars, globalVar.lastTxCmdFrame);
	}

	return true;
}

Private void G992p3CopyOemData(uchar *pDest, ulong dstLen, uchar *pSrc, ulong srcLen)
{
	if (dstLen <= srcLen)
		AdslMibByteMove (dstLen, pSrc, pDest);
	else {
		AdslMibByteMove (srcLen, pSrc, pDest);
		AdslMibByteClear(dstLen - srcLen, pDest + srcLen);
	}
}

void G992p3GetOemData(void *gDslVars, int paramId, uchar *pDest, ulong dstLen, uchar *pSrc, ulong srcLen)
{
	dslStatusStruct			status;
	int						len;
	
	status.code = kDslGetOemParameter;
	status.param.dslOemParameter.paramId = paramId;
	status.param.dslOemParameter.dataPtr = NULL;
	status.param.dslOemParameter.dataLen = 0;
	(*globalVar.statusHandlerPtr)(gDslVars, &status);
	if (NULL != status.param.dslOemParameter.dataPtr)
		G992p3CopyOemData(pDest, dstLen, status.param.dslOemParameter.dataPtr, status.param.dslOemParameter.dataLen);
	else if (NULL == pSrc)
		AdslMibByteClear(dstLen, pDest);
	else if (0 == srcLen) {
		len = AdslMibStrCopy(pSrc, pDest);
		if (len < dstLen)
			AdslMibByteClear(dstLen - len, pDest + len);
	}
	else  // pSrc != NULL, srcLen != 0
		G992p3CopyOemData(pDest, dstLen, pSrc, srcLen);
}

Boolean G992p3OvhMsgRcvProcessCmd(void *gDslVars, dslFrameBuffer *pBuf, uchar *pData, ulong cmdLen)
{
	uchar					*pMsg;
	Boolean					bNewCmd, bRsp;
	ulong					mibLen, rspLen, i;
	adslMibInfo				*pMib;
	adslChanPerfDataEntry	*pChanPerfData;
	dslCommandStruct		cmd;
	int						modType = AdslMibGetModulationType(gDslVars);
	int						timeSec,timeSec1,timeMin,timeMin1,timeHr,timeHr1;
	int n;
	
	if (G992p3OvhMsgXmtRspBusy())
		return false;
	
	bNewCmd = (pData[kG992p3CtrlField] ^ globalVar.rxCmdMsgNum) & kG992p3MsgNumMask;
	globalVar.rxCmdMsgNum = pData[kG992p3CtrlField] & kG992p3MsgNumMask;
	bRsp = false;
	rspLen = 0;
	globalVar.PLNMessageReadRspFlag=0;
	switch (pData[kG992p3CmdCode]) {
	  
	    case kG992p3OvhMsgCmdTime:
		  if (pData[kG992p3CmdSubCode]==0x01)
		    {
		      globalVar.txRspBuf[kG992p3CmdCode] = pData[kG992p3CmdCode];
		      globalVar.cmdTime=(pData[4]-'0')*36000+(pData[5]-'0')*3600+(pData[7]-'0')*600+(pData[8]-'0')*60+(pData[10]-'0')*10+pData[11]-'0';
		      globalVar.cmdTime*=1000;
		      globalVar.txRspBuf[kG992p3CmdSubCode]=0x80;
		      rspLen=4; 
		      timeSec1=(globalVar.cmdTime+500)/1000;
		      timeMin1=timeSec1/60;
		      timeHr1=timeMin1/60;
		      timeSec1%=60;
		      timeMin1%=60;
		      __SoftDslPrintf(gDslVars, "G992p3Ovh CMD TIME Set as Hr%d Min %d Sec %d", 0, timeHr1,timeMin1,timeSec1);
		    }      
		  else if(pData[kG992p3CmdSubCode]==0x02)
		    {
		      globalVar.txRspBuf[kG992p3CmdCode] = pData[kG992p3CmdCode];
		      globalVar.txRspBuf[kG992p3CmdSubCode]=0x82;
		     
		      timeSec=(globalVar.cmdTime+500)/1000;
	              timeMin=timeSec/60;
	              timeHr=timeMin/60;
		     
		      timeSec%=60;
		      timeMin%=60;
			
		      
		      globalVar.txRspBuf[4]=timeHr/10+'0';
		      globalVar.txRspBuf[5]=timeHr%10+'0';
		      globalVar.txRspBuf[6]=':';
		      globalVar.txRspBuf[7]=timeMin/10+'0';
		      globalVar.txRspBuf[8]=timeMin%10+'0';
		      globalVar.txRspBuf[9]=':';
		      globalVar.txRspBuf[10]=timeSec/10+'0';
		      globalVar.txRspBuf[11]=timeSec%10+'0';
		      rspLen=12;
		      __SoftDslPrintf(gDslVars, "G992p3Ovh CMD TIME Get called Hr%d Min %d Sec %d", 0, timeHr,timeMin,timeSec);
		    }
  		  bRsp = true;
		  break;
	  
		case kG992p3OvhMsgCmdCntRead:
			mibLen = sizeof(adslMibInfo);
			pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			
			if ( (kG992p3OvhMsgCmdCntReadId == pData[kG992p3CmdSubCode]) ||
			      ((pMib->adslStat.fireStat.status & kFireDsEnabled) && (kG992p3OvhMsgCmdCntReadId1== pData[kG992p3CmdSubCode])) ) {
				pChanPerfData = (kAdslIntlChannel == pMib->adslConnection.chType) ?
					&pMib->adslChanIntlPerfData : &pMib->adslChanFastPerfData;

				globalVar.txRspBuf[kG992p3CmdCode] = pData[kG992p3CmdCode];
				globalVar.txRspBuf[kG992p3CmdSubCode] = pData[kG992p3CmdSubCode] ^ 0x80;

				pMsg = &globalVar.txRspBuf[kG992p3CmdSubCode+1];
				WriteCnt32(pMsg+0, pChanPerfData->perfTotal.adslChanCorrectedBlks);
				WriteCnt32(pMsg+4, pChanPerfData->perfTotal.adslChanUncorrectBlks);
				WriteCnt32(pMsg+8,  pMib->adslPerfData.perfTotal.adslFECs);
				WriteCnt32(pMsg+12, pMib->adslPerfData.perfTotal.adslESs);
				WriteCnt32(pMsg+16, pMib->adslPerfData.perfTotal.adslSES);
				WriteCnt32(pMsg+20, pMib->adslPerfData.perfTotal.adslLOSS);
				WriteCnt32(pMsg+24, pMib->adslPerfData.perfTotal.adslUAS);

				WriteCnt32(pMsg+28, pMib->atmStat.rcvStat.cntHEC);
				WriteCnt32(pMsg+32, pMib->atmStat.rcvStat.cntCellTotal);
				WriteCnt32(pMsg+36, pMib->atmStat.rcvStat.cntCellData);
				WriteCnt32(pMsg+40, pMib->atmStat.rcvStat.cntBitErrs);
				if ( kG992p3OvhMsgCmdCntReadId1 != pData[kG992p3CmdSubCode] )
					rspLen = pMsg+44 - globalVar.txRspBuf;					
				else {
					WriteCnt32(pMsg+44, pMib->adslStat.fireStat.reXmtRSCodewordsRcved);
					WriteCnt32(pMsg+48, pMib->adslStat.fireStat.reXmtUncorrectedRSCodewords);
					WriteCnt32(pMsg+52, pMib->adslStat.fireStat.reXmtCorrectedRSCodewords);
					rspLen = pMsg+56 - globalVar.txRspBuf;	
				}
				
				bRsp = true;
			}
			break;

		case kG992p3OvhMsgCmdEOC:
			if (kG992p3OvhMsgCmdEOCUpdTestParam != pData[kG992p3CmdSubCode])
				break;
			globalVar.txRspBuf[kG992p3CmdCode] = kG992p3OvhMsgCmdEOC;
			globalVar.txRspBuf[kG992p3CmdSubCode] = kG992p3OvhMsgCmdEOCAck;
			rspLen = 4;
			bRsp = true;
			break;

		case kG992p3OvhMsgCmdClearEOC:
			if (kG992p3OvhMsgCmdClearEOCMsg == pData[kG992p3CmdSubCode]) {
				globalVar.txRspBuf[kG992p3CmdCode] = kG992p3OvhMsgCmdClearEOC;
				globalVar.txRspBuf[kG992p3CmdSubCode] = kG992p3OvhMsgCmdClearEOCAck;
				rspLen = 4;
				bRsp = true;
			}
			else if (kG992p3OvhMsgCmdClearEOCAck == pData[kG992p3CmdSubCode]) {	/* workaround for Huawei DSLAM */
				globalVar.setup |= kG992p3ClearEocWorkaround;
				G992p3OvhMsgPollCmdRateChange(gDslVars);
				G992p3OvhMsgRcvProcessRsp(gDslVars, pBuf, pData, cmdLen);
				__SoftDslPrintf(gDslVars, "G992p3Ovh using Clear EOC Workaround",0);
			}
			break;

		case kG992p3OvhMsgCmdInventory:
			pMsg = &globalVar.txRspBuf[kG992p3CmdSubCode+1];
			switch (pData[kG992p3CmdSubCode]) {
				case kG992p3OvhMsgCmdInvId:
					{
					adslPhyInfo		*pPhyInfo = AdslCoreGetPhyInfo();

					G992p3GetOemData(gDslVars, kDslOemEocVendorId, pMsg, 8, g992p3OvhMsgVendorId, kDslVendorIDRegisterLength);
					G992p3GetOemData(gDslVars, kDslOemEocVersion, pMsg+8, 16, pPhyInfo->pVerStr, 0);
					G992p3GetOemData(gDslVars, kDslOemEocSerNum, pMsg+24, 32, NULL, 0);
					rspLen = kDslVendorIDRegisterLength + 16 + 32;
					bRsp = true;
					}
					break;
				case kG992p3OvhMsgCmdInvAuxId:
					AdslMibByteMove (kDslVendorIDRegisterLength, g992p3OvhMsgVendorId, pMsg);
					rspLen = kDslVendorIDRegisterLength;
					bRsp = true;
					break;
				case kG992p3OvhMsgCmdInvSelfTestRes:
				case kG992p3OvhMsgCmdInvPmdCap:
				case kG992p3OvhMsgCmdInvPmsTcCap:
				case kG992p3OvhMsgCmdInvTpsTcCap:
				default:
					bRsp = false;
					break;
			}
			if (bRsp) {
			       	globalVar.txRspBuf[kG992p3CmdCode] = pData[kG992p3CmdCode];
				globalVar.txRspBuf[kG992p3CmdSubCode] = pData[kG992p3CmdSubCode] | 0x80;
				rspLen += 4;
			}
			break;

	        case kG992p3OvhMsgCmdPMDRead:
			globalVar.txRspBuf[kG992p3CmdCode] = kG992p3OvhMsgCmdPMDRead;
			if(globalVar.kG992p3OvhMsgCmdPLN==pData[kG992p3CmdSubCode]){
			  __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ,pData[2]=%d pData[3]=%d pData[4]=%d",0,pData[2],pData[3],pData[4]);
			  globalVar.txRspBuf[kG992p3CmdSubCode]= globalVar.kG992p3OvhMsgCmdPLN | 0x80;
			  bRsp=true;
			  switch(pData[kG992p3CmdId]){
			    
			  case kDslPLNControlStart :
			    globalVar.PLNActiveFlag=1;
			    globalVar.PLNNoiseMarginPerBinDecLevel=pData[7];
			    globalVar.PLNNoiseMarginPerBinDecLevel<<=8;
			    globalVar.PLNNoiseMarginPerBinDecLevel=globalVar.PLNNoiseMarginPerBinDecLevel|pData[8];
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN Start,PLNNoiseMarginDecLevel CMDLEN=%u 0x%x 0x%x 0x%x %u",0,globalVar.PLNNoiseMarginPerBinDecLevel,pData[7],pData[8],globalVar.PLNNoiseMarginPerBinDecLevel,cmdLen);
			    globalVar.PLNNoiseMarginBroadbandDecLevel=pData[5];
			    globalVar.PLNNoiseMarginBroadbandDecLevel<<=8;
			    globalVar.PLNNoiseMarginBroadbandDecLevel=globalVar.PLNNoiseMarginBroadbandDecLevel|pData[6];
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN Start,PLNNoiseMarginBroadbandDecLevel=%u 0x%x 0x%x 0x%x",0,globalVar.PLNNoiseMarginBroadbandDecLevel,pData[5],pData[6],globalVar.PLNNoiseMarginBroadbandDecLevel);
			    if(cmdLen>12){
			            globalVar.PLNnInpBin=pData[9];
				    n=2*globalVar.PLNnInpBin;
				    if(NULL!=(globalVar.pPLNinpBinIntervalPtr=AdslCoreSharedMemAlloc(n))){
				            __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN .pPLNinpBinIntervalPtr %X n= %d",0,globalVar.pPLNinpBinIntervalPtr,n);
				            memcpy(globalVar.pPLNinpBinIntervalPtr,&pData[10],n);
				    }
				    //  globalVar.pPLNinpBinIntervalPtr= (ushort*) &pData[10];
			    
				    globalVar.PLNnItaBin=pData[10+2*globalVar.PLNnInpBin];
				    n=2*globalVar.PLNnItaBin;
				    if(NULL!=(globalVar.pPLNitaBinIntervalPtr=AdslCoreSharedMemAlloc(n))){ 
                                            __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN .pPLNitaBinIntervalPtr %X n= %d",0,globalVar.pPLNitaBinIntervalPtr,n);
				            memcpy(globalVar.pPLNitaBinIntervalPtr,&pData[10+2*globalVar.PLNnInpBin+1],n);
				    }
			    }
			    //globalVar.pPLNitaBinIntervalPtr=(ushort*) &pData[10+2*globalVar.PLNnInpBin+1];
			    globalVar.pPLNPeakNoiseTable=NULL;
			    globalVar.pPLNThresholdCountTable=NULL;
			    globalVar.pPLNImpulseDurationTable=NULL;
			    globalVar.pPLNInterArrivalTimeTable=NULL;
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN  Start",0);
			    globalVar.PLNPerBinPeakNoiseMsrCounter=0;
			    globalVar.PLNPerBinThldCounter=0;
			    globalVar.PLNImpulseDurStatCounter=0;
			    globalVar.PLNImpulseInterArrStatCounter=0;
			    globalVar.PLNElapsedTimeMSec=0;
			    
			    //cmd.command=kDslPLNControlCmd;
			    //cmd.param.value=kDslPLNControlClear;
			    //(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    
			    cmd.command=kDslPLNControlCmd;
			    if(cmdLen<13){
			      globalVar.PLNnInpBin=0;
			      globalVar.PLNnItaBin=0; 
			    }
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.value=kDslPLNControlStop;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    
			    cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineInpBinTable;
			    cmd.param.dslPlnSpec.nInpBin= globalVar.PLNnInpBin;
			    cmd.param.dslPlnSpec.inpBinPtr=globalVar.pPLNinpBinIntervalPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd); 
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineItaBinTable;
			    cmd.param.dslPlnSpec.nItaBin=globalVar.PLNnItaBin;
			    cmd.param.dslPlnSpec.itaBinPtr= globalVar.pPLNitaBinIntervalPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.dslPlnSpec.plnCmd=kDslPLNControlStart;
			    cmd.param.dslPlnSpec.mgnDescreaseLevelPerBin= globalVar.PLNNoiseMarginPerBinDecLevel;
			    cmd.param.dslPlnSpec.mgnDescreaseLevelBand=globalVar.PLNNoiseMarginBroadbandDecLevel;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd); 
			   
			 
			    globalVar.txRspBuf[4]=kDslPLNControlStart;
			    rspLen++;
			    break;
			  case kDslPLNControlStop:
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN Read Stop",0);
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.value=kDslPLNControlStop;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    globalVar.txRspBuf[4]=kDslPLNControlStop;
			    globalVar.PLNActiveFlag=0;
			    rspLen++;
			    break;
			
			  case kDslPLNControlReadPerBinPeakNoise:   
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.value=kDslPLNControlPeakNoiseGetPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    bRsp=false;
			    globalVar.PLNMessageReadRspFlag=1;
			    globalVar.txRspMsgNumStored=globalVar.txRspMsgNum^1;
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadPerBinPeakNoise",0);
			    break;
			  case kDslPLNControlReadPerBinThresholdCount:
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.value=kDslPLNControlThldViolationGetPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    bRsp=false;
			    globalVar.PLNMessageReadRspFlag=1;
			    globalVar.txRspMsgNumStored=globalVar.txRspMsgNum^1;
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadPerBinThresholdCount",0);
			    break;
			  case kDslPLNControlReadImpulseDurationStat:
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadImpulseDurationStat",0);
			    cmd.command=kDslPLNControlCmd;
			    cmd.param.value=kDslPLNControlImpulseNoiseEventGetPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    globalVar.PLNMessageReadRspFlag=1;
			    globalVar.txRspMsgNumStored=globalVar.txRspMsgNum^1;
			    bRsp=false;  
			    break;
			  case kDslPLNControlReadImpulseInterArrivalTimeStat:
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadImpulseInterArrivalTimeStat",0);
			    cmd.command=kDslPLNControlCmd; 
			    cmd.param.value=kDslPLNControlImpulseNoiseTimeGetPtr;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    globalVar.PLNMessageReadRspFlag=1;
			    globalVar.txRspMsgNumStored=globalVar.txRspMsgNum^1;
			    bRsp=false;
			    break;
			  case 	kDslPLNControlPLNStatus :
			    __SoftDslPrintf(gDslVars, "G992p3Ovh CMD get PLN Status",0);
			    cmd.command=kDslPLNControlCmd; 
			    cmd.param.value= kDslPLNControlGetStatus;
			    (*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
			    globalVar.PLNMessageReadRspFlag=1;
			    globalVar.txRspMsgNumStored=globalVar.txRspMsgNum^1;
			    bRsp=false;
			    break;
			  }
			  rspLen+=4;
			  break;
			}
	
			if(kG992p3OvhMsgCmdPMDSingleRead != pData[kG992p3CmdSubCode]) {
				globalVar.txRspBuf[kG992p3CmdSubCode] = kG992p3OvhMsgCmdPMDReadNACK;
				rspLen = 4;
				bRsp = true;
				break;
			}

			globalVar.txRspBuf[kG992p3CmdSubCode] = kG992p3OvhMsgCmdPMDSingleRdRsp;
			pMsg = &globalVar.txRspBuf[kG992p3CmdSubCode+1];
			rspLen = 0;
			mibLen = sizeof(adslMibInfo);
			pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			switch (pData[kG992p3CmdId]){ 
				case kG992p3OvhMsgCmdPMDPeriod:
					WriteCnt16(pMsg+0, 0x100);
					rspLen = 2;
					break;

				case kG992p3OvhMsgCmdPMDChanRspLog:
					{
					ulong	chLogLen, val;
					short	*pChLog;
					uchar	chLogOidStr[] = { kOidAdslPrivate, kOidAdslPrivChanCharLog };

					WriteCnt16(pMsg+0, 0x100);
					pMsg+=2;
					pChLog = (void*) AdslMibGetObjectValue (gDslVars, chLogOidStr, sizeof(chLogOidStr), NULL, &chLogLen);
					for (i = 0; i < (chLogLen >> 1); i++) {
						val = (ulong) (((6*16 - pChLog[i]) * 5) >> 3);
						if (val > 0x3FE)
							val = 0x3FF;
						WriteCnt16(pMsg, val);
						pMsg += 2;
					}
					rspLen = chLogLen + 2;
					}
					break;
			
				case kG992p3OvhMsgCmdPMDQLineNoise:
					{
					ulong	qlnLen, qln;
					short	*pQlnInfo;
					uchar	qlnOidStr[] = { kOidAdslPrivate, kOidAdslPrivQuietLineNoise };

					WriteCnt16(pMsg+0, 0x100);
					pMsg+=2;
					pQlnInfo = (void*) AdslMibGetObjectValue (gDslVars, qlnOidStr, sizeof(qlnOidStr), NULL, &qlnLen);
					qlnLen >>= 1;
					for (i = 0; i < qlnLen; i++) {
						qln = (ulong) ((-pQlnInfo[i] - 23*16) >> 3);
						pMsg[i] = (qln > 254) ? 255 : qln;
					}
					rspLen = qlnLen + 2;
					}
					break;

				       
				case kG992p3OvhMsgCmdPMDSnr:
					{
					ulong	snrLen, snr;
					short	*pSnrInfo;
					uchar	snrOidStr[] = { kOidAdslPrivate, kOidAdslPrivSNR };

					WriteCnt16(pMsg+0, 0x100);
					pMsg+=2;
					pSnrInfo = (void*) AdslMibGetObjectValue (gDslVars, snrOidStr, sizeof(snrOidStr), NULL, &snrLen);
					snrLen >>= 1;
					for (i = 0; i < snrLen; i++) {
						snr = (ulong) ((pSnrInfo[i] + 32*16) >> 3);
						pMsg[i] = (snr > 254) ? 255 : snr;
					}
					rspLen = snrLen + 2;
					}
					break;

				case kG992p3OvhMsgCmdPMDLnAttn:
					{
					long val;
					val=pMib->adslPhys.adslCurrAtn;
					val=RestrictValue(val, 0, 1023);
					WriteCnt16(pMsg+0, val);
					rspLen = 2;
					}
					break;
				case kG992p3OvhMsgCmdPMDSigAttn:
					{
					long val;
					val=pMib->adslPhys.adslCurrAtn;
					val=RestrictValue(val, 0, 1023);
					WriteCnt16(pMsg+0, val);
					rspLen = 2;
					}
					break;
				case kG992p3OvhMsgCmdPMDSnrMgn:
					{
					long val;
					val=pMib->adslPhys.adslCurrSnrMgn;
					val=RestrictValue(val, -512, 512);
					if(val==512) val=-512;
					WriteCnt16(pMsg+0, val);
					rspLen = 2;
					}
					break;
				case kG992p3OvhMsgCmdPMDAttnDR:
					WriteCnt32(pMsg+0, pMib->adslPhys.adslCurrAttainableRate);
					rspLen = 4;
					break;
				case kG992p3OvhMsgCmdPMDNeXmtPower:
					{
					long val;
					val=pMib->adslPhys.adslCurrOutputPwr;
					if(val<-310 || val >310)
						val=-512;
					WriteCnt16(pMsg+0,val);
					rspLen = 2;
					}
					break;
				case kG992p3OvhMsgCmdPMDFeXmtPower:
					{
					long val;
					val=pMib->adslAtucPhys.adslCurrOutputPwr;
					if(val<-310 || val >310)
						val=-512;
					WriteCnt16(pMsg+0,val);
					rspLen = 2;
					}
					break;
				default:
					globalVar.txRspBuf[kG992p3CmdSubCode] = kG992p3OvhMsgCmdPMDReadNACK;
					break;
			}
			rspLen += 4;
			
			bRsp = true;
			break;
		
		case kG992p3OvhMsgCmdOLR:
			cmd.command = kDslOLRRequestCmd;
			cmd.param.dslOLRRequest.msgType = pData[kG992p3CmdSubCode];
			pMsg = pData+kG992p3CmdSubCode+1;
			switch (cmd.param.dslOLRRequest.msgType) {
			  case kG992p3OvhMsgCmdOLRReq2:
			  case kG992p3OvhMsgCmdOLRReq3:
			  case kG992p3OvhMsgCmdOLRReq5:
			  case kG992p3OvhMsgCmdOLRReq6:
				cmd.param.dslOLRRequest.L[0] = ReadCnt16(pMsg);
				cmd.param.dslOLRRequest.B[0] = pMsg[2];
				pMsg += 3;
				/* fall through */

			  case kG992p3OvhMsgCmdOLRReq1:
			  case kG992p3OvhMsgCmdOLRReq4:
				if (kAdslModAdsl2p == modType) {
					cmd.param.dslOLRRequest.nCarrs = ReadCnt16(pMsg);
					cmd.param.dslOLRRequest.carrParamPtr = pMsg + 2;
				}
				else {
					cmd.param.dslOLRRequest.nCarrs  = pMsg[0];
					cmd.param.dslOLRRequest.carrParamPtr = pMsg + 1;
				}

				if (cmd.param.dslOLRRequest.nCarrs != 0) {
					int 	n;
					uchar	*pSharedMem;

					n = cmd.param.dslOLRRequest.nCarrs * ((kAdslModAdsl2p == modType) ? 
							sizeof(dslOLRCarrParam2p) : sizeof(dslOLRCarrParam));
					if (NULL != (pSharedMem = AdslCoreSharedMemAlloc(n))) {
						FrameDataCopy(gDslVars, pBuf, cmd.param.dslOLRRequest.carrParamPtr, n, pSharedMem);
						cmd.param.dslOLRRequest.carrParamPtr = pSharedMem;
						(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
						mibLen = sizeof(adslMibInfo);
						pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
						pMib->adslStat.bitswapStat.xmtCnt++;
					}
				}
				break;
			}
			break;

		case kG992p3OvhMsgCmdPower:
			cmd.command = kDslPwrMgrCmd;
			cmd.param.dslPwrMsg.msgType = pData[kG992p3CmdSubCode];
			switch (cmd.param.dslPwrMsg.msgType) {
			  case kPwrL2TrimRequest:
				cmd.param.dslPwrMsg.param.value = pData[kG992p3CmdSubCode+1];
				(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
				break;

			  case kPwrL2Request:
				{
				uchar	*pSharedMem;

				if (NULL == (pSharedMem = AdslCoreSharedMemAlloc(cmdLen)))
					break;

				FrameDataCopy(gDslVars, pBuf, pData+kG992p3CmdSubCode+1, cmdLen-kG992p3CmdSubCode-1, pSharedMem);
				cmd.param.dslPwrMsg.param.msg.msgLen = cmdLen-kG992p3CmdSubCode-1;
				cmd.param.dslPwrMsg.param.msg.msgData = pSharedMem;
				(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
				}
				break;

			  case kPwrSimpleRequest:
				{
				globalVar.txRspBuf[kG992p3CmdCode] = kG992p3OvhMsgCmdPower;

				if (3 == pData[kG992p3CmdSubCode+1]) {
					globalVar.txFlags |= kTxCmdL3RspWait;
					globalVar.timeRspOut = globalVar.timeMs;
					globalVar.txRspBuf[kG992p3CmdSubCode] = kPwrGrant;
					rspLen = 4;
				}
				else {
					globalVar.txRspBuf[kG992p3CmdSubCode]	= kPwrReject;
					globalVar.txRspBuf[kG992p3CmdSubCode+1] = kPwrInvalid;
					rspLen = 5;
				}
				bRsp = true;
				}
				break;
			}
			break;

		default:
			break;
	}

	if (bRsp) {
		//if (bNewCmd)
			globalVar.txRspMsgNum ^= 1;
		globalVar.txRspBuf[kG992p3AddrField] = pData[kG992p3AddrField];
		globalVar.txRspBuf[kG992p3CtrlField] = kG992p3Rsp | globalVar.txRspMsgNum;
		if( (globalVar.setup & kG992p3ClearEocWorkaround) != 0 ) {
			// if (kG992p3OvhMsgCmdClearEOC == pData[kG992p3CmdCode])
				globalVar.txRspBuf[kG992p3CtrlField] = kG992p3Rsp | G992p3OvhMsgGetCmdNum(gDslVars, pData, true);
			globalVar.txRspMsgNum ^= 1;
		}
		G992p3OvhMsgXmtSendRsp(gDslVars, rspLen);
	}

	return bRsp;
}

Boolean G992p3OvhMsgRcvProcessSegAck(void *gDslVars, dslFrameBuffer *pBuf, uchar *pData, ulong cmdLen)
{
#if 1 || defined(G992P3_DBG_PRINT)
	__SoftDslPrintf(gDslVars, "G992p3OvhMsgRcvProcessSegAck: code=0x%X, subCode=0x%X, segId=%d, 0x%X, 0x%X\n", 0,
		pData[kG992p3CmdCode], pData[kG992p3CmdSubCode], pData[kG992p3CmdSubCode+1],
		pData[kG992p3CmdSubCode+2], pData[kG992p3CmdSubCode+3]);
#endif

	if (NULL == globalVar.txSegFrameCtl.segFrame)
		return false;

	if (globalVar.txSegFrameCtl.segId == pData[kG992p3CmdSubCode+1]) {
		if (G992p3IsFrameBusy(gDslVars, globalVar.txSegFrameCtl.segFrame)) {
			G992p3FrameBitSet(gDslVars, globalVar.txSegFrameCtl.segFrame, kFrameNextSegPending);
			__SoftDslPrintf(gDslVars, "G992p3OvhMsgRcvProcessSegAck: Mark pending\n", 0);
		}
		else
			G992p3OvhMsgXmtSendNextSeg(gDslVars, &globalVar.txSegFrameCtl);
		return true;
	}
	return false;
}

Public int G992p3OvhMsgIndicateRcvFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	dslFrameBuffer			*pBuf;
	uchar					*pData, cmd;
	int						len;
	Boolean					bRes;

	if (!ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
		return 0;

	G992p3SetFrameInfoVal(gDslVars, pFrame, 0);
	pBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	if (NULL == pBuf)
		return 1;

	len   = DslFrameBufferGetLength(gDslVars, pBuf);
	pData = DslFrameBufferGetAddress(gDslVars, pBuf);
	if ((len < 4) || (NULL == pData))
		return 1;

	cmd = pData[kG992p3CmdCode];
	if ((kG992p3OvhSegMsgAckHi == cmd) ||
		(kG992p3OvhSegMsgAckNormal == cmd) ||
		(kG992p3OvhSegMsgAckLow == cmd))
		bRes = G992p3OvhMsgRcvProcessSegAck(gDslVars, pBuf, pData, len);
	else if (kG992p3Cmd == (pData[kG992p3CtrlField] & kG992p3CmdRspMask))
		bRes = G992p3OvhMsgRcvProcessCmd(gDslVars, pBuf, pData, len);
	else
		bRes = G992p3OvhMsgRcvProcessRsp(gDslVars, pBuf, pData, len);

	if ((kG992p3OvhMsgCmdClearEOC == pData[kG992p3CmdCode]) &&
		(kG992p3OvhMsgCmdClearEOCMsg == pData[kG992p3CmdSubCode]))
	{
		// __SoftDslPrintf(NULL, "ClearEOC frame received: pFr=0x%X len=%d pData=0x%X\n", 0, pFrame, len, pData);
		DslFrameBufferSetLength(gDslVars, pBuf, len-4);
		DslFrameBufferSetAddress(gDslVars, pBuf, pData+4);
		G992p3SetFrameInfoVal(gDslVars, pFrame, 4);
		return 0;
	}

	return 1;
}

Public void G992p3OvhMsgReturnFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	dslFrameBuffer		*pBuf;
	uchar				*pData;
	int					len;
	ulong				val = G992p3GetFrameInfoVal(gDslVars, pFrame);

	// __SoftDslPrintf(NULL, "ClearEOC frame returned: pFr=0x%X val=%d\n", 0, pFrame, val);
	if (0 == val)
		return;

	pBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	if (NULL == pBuf)
		return;

	len   = DslFrameBufferGetLength(gDslVars, pBuf);
	pData = DslFrameBufferGetAddress(gDslVars, pBuf);
	// __SoftDslPrintf(NULL, "ClearEOC frame returned: len=%d pData=0x%X\n", 0, len, pData);

	DslFrameBufferSetLength(gDslVars, pBuf, len + val);
	DslFrameBufferSetAddress(gDslVars, pBuf, pData - val);
	G992p3SetFrameInfoVal(gDslVars, pFrame, 0);
}

Public void	G992p3OvhMsgSetL3(void *gDslVars)
{
	if (0 == (globalVar.txFlags & kTxCmdL3WaitingAck))
		globalVar.txL3Rq = true;
}

Public void	G992p3OvhMsgSetL0(void *gDslVars)
{
	if ((globalVar.txFlags & kTxCmdL0WaitingAck) || (0 == AdslMibPowerState(gDslVars)))
		return;

	globalVar.txL0Rq = true;
}

Public void PLNReadResponses(void *gDslVars, char PLNTableName)
{
        int rspLen=4;
	uchar	*pMsg;
	ulong zero=0;
	globalVar.txRspBuf[kG992p3AddrField] =kG992p3PriorityLow ;
	globalVar.txRspBuf[kG992p3CtrlField] =kG992p3Rsp | globalVar.txRspMsgNumStored;
	globalVar.txRspBuf[kG992p3CmdCode] = kG992p3OvhMsgCmdPMDRead;
	globalVar.txRspBuf[kG992p3CmdSubCode]= globalVar.kG992p3OvhMsgCmdPLN | 0x80;
	globalVar.PLNMessageReadRspFlag=0;
	switch(PLNTableName)
	        {      
		case kDslPLNPeakNoiseTablePtr:
		  {
		          int j;
			  globalVar.txRspBuf[4]=kDslPLNControlReadPerBinPeakNoise;
			  rspLen++;
			  __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadPerBinPeakNoise Ptr Has returned",0);
			  
			     
			  if(globalVar.pPLNPeakNoiseTable!=NULL){
			    pMsg= &globalVar.txRspBuf[5];
			    if(globalVar.PLNPerBinMeasurementCounterRcvFlag==1){
			      WriteCnt32(pMsg,globalVar.PLNUpdtCount);
			      globalVar.PLNPerBinPeakNoiseMsrCounter=globalVar.PLNUpdtCount;
			    }
			    else WriteCnt32(pMsg,zero);
			    rspLen+=4;
			    //AdslMibByteMove(globalVar.PLNPeakNoiseTableLen,globalVar.pPLNPeakNoiseTable,globalVar.txRspBuf+rspLen);
			    for (j=0;j<globalVar.PLNPeakNoiseTableLen;j++)
			      globalVar.txRspBuf[j+9]=*(globalVar.pPLNPeakNoiseTable + j);
			    rspLen+=globalVar.PLNPeakNoiseTableLen;
			  }
			  globalVar.PLNPerBinMeasurementCounterRcvFlag=0;
		  }
		  break;
		case kDslPerBinThldViolationTablePtr:{
		          int j;
			  globalVar.txRspBuf[4]=kDslPLNControlReadPerBinThresholdCount;
			  rspLen++;
			  __SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadPerBinThresholdCount",0);
			 
			  if(globalVar.pPLNThresholdCountTable!=NULL){
			    pMsg = &globalVar.txRspBuf[5];
			    WriteCnt16(pMsg,globalVar.PLNNoiseMarginPerBinDecLevel);
			    rspLen+=2;
			    pMsg= &globalVar.txRspBuf[7];
			    if(globalVar.PLNPerBinMeasurementCounterRcvFlag==1){
			      WriteCnt32(pMsg,globalVar.PLNUpdtCount);
			      globalVar.PLNPerBinThldCounter=globalVar.PLNUpdtCount;
			    }
			    else WriteCnt32(pMsg,zero);
			    rspLen+=4;
			    //AdslMibByteMove(globalVar.PLNThresholdCountTableLen,globalVar.pPLNThresholdCountTable,globalVar.txRspBuf+rspLen);
			    for (j=0;j<globalVar.PLNThresholdCountTableLen;j++)
			      globalVar.txRspBuf[j+11]=*(globalVar.pPLNThresholdCountTable + j);
			    rspLen+=globalVar.PLNThresholdCountTableLen;
			  }
			  globalVar.PLNPerBinMeasurementCounterRcvFlag=0;
		}
		  break;
		case kDslImpulseNoiseDurationTablePtr:{
		        int j;		 
		        globalVar.txRspBuf[4]=kDslPLNControlReadImpulseDurationStat;
			rspLen++;
			__SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadImpulseDurationStat",0);
		
			if(globalVar.pPLNImpulseDurationTable!=NULL){
			  pMsg = &globalVar.txRspBuf[5]; 
			  WriteCnt16(pMsg,globalVar.PLNNoiseMarginBroadbandDecLevel);
			  rspLen+=2;
			  pMsg = &globalVar.txRspBuf[7];
			  if(globalVar.PLNBroadbandMsrCounterRcvFlag==1){
			    WriteCnt32(pMsg,globalVar.PLNUpdtCount);
			    globalVar.PLNImpulseDurStatCounter=globalVar.PLNUpdtCount;
			  }
			  else WriteCnt32(pMsg,zero);
			  rspLen+=4;
			  //AdslMibByteMove(globalVar.PLNImpulseDurationTableLen,globalVar.pPLNImpulseDurationTable,globalVar.txRspBuf+rspLen);
			  for (j=0;j<globalVar.PLNImpulseDurationTableLen;j++)
			    globalVar.txRspBuf[j+11]=*(globalVar.pPLNImpulseDurationTable + j);
			  rspLen+=globalVar.PLNImpulseDurationTableLen;
			  globalVar.PLNBroadbandMsrCounterRcvFlag=0;
			}
		}
		  break;
		case kDslImpulseNoiseTimeTablePtr:{
	   	        int j;
			globalVar.txRspBuf[4]=kDslPLNControlReadImpulseInterArrivalTimeStat;
			rspLen++;
			__SoftDslPrintf(gDslVars, "G992p3Ovh CMD PLN ReadImpulseInterArrivalTimeStat",0);
		
			if(globalVar.pPLNInterArrivalTimeTable!=NULL){
			  pMsg = &globalVar.txRspBuf[5];
			  WriteCnt16(pMsg,globalVar.PLNNoiseMarginBroadbandDecLevel);
			  rspLen+=2;
			  pMsg = &globalVar.txRspBuf[7];
			  if(globalVar.PLNBroadbandMsrCounterRcvFlag==1){
			    WriteCnt32(pMsg,globalVar.PLNUpdtCount);
			    globalVar.PLNImpulseInterArrStatCounter=globalVar.PLNUpdtCount;
			  }
			  else WriteCnt32(pMsg,zero);
			  rspLen+=4;
			  //AdslMibByteMove(globalVar.PLNInterArrivalTimeTableLen,globalVar.pPLNInterArrivalTimeTable,globalVar.txRspBuf+rspLen); 
			  for (j=0;j<globalVar.PLNInterArrivalTimeTableLen;j++)
			    globalVar.txRspBuf[j+11]=*(globalVar.pPLNInterArrivalTimeTable + j);
			  rspLen+=globalVar.PLNInterArrivalTimeTableLen;
			  globalVar.PLNBroadbandMsrCounterRcvFlag=0;
			}
		}
		  break;
		case  kDslItaBinTablePtr:{
		          int j,k;
			  globalVar.txRspBuf[4]=kDslPLNControlPLNStatus;
			  rspLen++;
			  if(globalVar.PLNActiveFlag==0)
			    globalVar.txRspBuf[5]=0x00;
			  else globalVar.txRspBuf[5]=0x01;
			  rspLen++;
			  pMsg = &globalVar.txRspBuf[6];
			  //globalVar.PLNElapsedTimeSec=(globalVar.PLNElapsedTimeMSec+500)/1000;
			  //WriteCnt32(pMsg,globalVar.PLNElapsedTimeSec);
			  if(globalVar.PLNBroadbandMsrCounterRcvFlag==1){
			    WriteCnt32(pMsg,globalVar.PLNUpdtCount);
			  }
			  else WriteCnt32(pMsg,zero);
			  rspLen+=4;

			  //globalVar.txRspBuf[10]=globalVar.PLNnInpBin;
			  globalVar.txRspBuf[10]=globalVar.PLNinpBinTableLen/2;
			  rspLen++;
			  for (j=0;j<globalVar.PLNinpBinTableLen;j++){
			    globalVar.txRspBuf[j+11]=*(globalVar.pPLNinpBinTablePtr + j);
			    __SoftDslPrintf(gDslVars, "G992p3Ovh PLN Status InpBin Table index %d value %X addr %X",0,j,*(globalVar.pPLNinpBinTablePtr+j),globalVar.pPLNinpBinTablePtr+j);
			  }
			  rspLen+=globalVar.PLNinpBinTableLen;
			 
			  //globalVar.txRspBuf[11+globalVar.PLNinpBinTableLen]=globalVar.PLNnItaBin;
			  globalVar.txRspBuf[11+globalVar.PLNinpBinTableLen]=globalVar.PLNitaBinTableLen/2;
			  rspLen++;
			  k=12+globalVar.PLNinpBinTableLen;
			  for (j=0;j<globalVar.PLNitaBinTableLen;j++){
			    globalVar.txRspBuf[j+k]=*(globalVar.pPLNitaBinTablePtr + j);
			    __SoftDslPrintf(gDslVars, "G992p3Ovh PLN Status InpBin Table index %d value %X addr %X",0,j,*(globalVar.pPLNitaBinTablePtr+j),globalVar.pPLNitaBinTablePtr+j);
			  }
			  rspLen+=globalVar.PLNitaBinTableLen;
			  
		}
		break;
		}
	G992p3OvhMsgXmtSendRsp(gDslVars, rspLen);
}

Public void G992p3OvhMsgStatusSnooper (void *gDslVars, dslStatusStruct *status)
{
	static uchar l3RequestCmd[] = { kG992p3PriorityNormal, 0, kG992p3OvhMsgCmdPower, kPwrSimpleRequest, 3};
	static uchar l0RequestCmd[] = { kG992p3PriorityNormal, 0, kG992p3OvhMsgCmdPower, kPwrSimpleRequest, 0};
	uchar	olrCmd[2+3+2*4+4+1];
	int		olrLen, olrLen1;

	switch (status->code) 
	  {
	  case kDslDspControlStatus:
	    switch(status->param.dslConnectInfo.code)
	      {
	      case kDslPLNPeakNoiseTablePtr:
		globalVar.PLNPeakNoiseTableLen=status->param.dslConnectInfo.value;
		globalVar.pPLNPeakNoiseTable=status->param.dslConnectInfo.buffPtr;
		if(globalVar.PLNMessageReadRspFlag==1)
		  PLNReadResponses(gDslVars,kDslPLNPeakNoiseTablePtr );
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN Peak Noise Table Ptr update Addr %X ",0,status->param.dslConnectInfo.buffPtr);
		break;
	      case kDslPerBinThldViolationTablePtr:
		globalVar.PLNThresholdCountTableLen=status->param.dslConnectInfo.value;
		globalVar.pPLNThresholdCountTable=status->param.dslConnectInfo.buffPtr;
		if(globalVar.PLNMessageReadRspFlag==1)
		  PLNReadResponses(gDslVars,kDslPerBinThldViolationTablePtr); 
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN Threshold Violation Count Table Ptr update Addr %X",0,status->param.dslConnectInfo.buffPtr);
		break;   
	      case kDslImpulseNoiseDurationTablePtr:
		globalVar.PLNImpulseDurationTableLen=status->param.dslConnectInfo.value;
		globalVar.pPLNImpulseDurationTable=status->param.dslConnectInfo.buffPtr;
		if(globalVar.PLNMessageReadRspFlag==1)
		  PLNReadResponses(gDslVars,kDslImpulseNoiseDurationTablePtr);
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN Impulse Duration Table Ptr update Addr %X",0,status->param.dslConnectInfo.buffPtr);
		break;
	      case kDslImpulseNoiseTimeTablePtr:
		globalVar.PLNInterArrivalTimeTableLen=status->param.dslConnectInfo.value;
		globalVar.pPLNInterArrivalTimeTable=status->param.dslConnectInfo.buffPtr;
		if(globalVar.PLNMessageReadRspFlag==1)
		  PLNReadResponses(gDslVars,kDslImpulseNoiseTimeTablePtr);
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN InterArrival Table Ptr update Addr %X",0,status->param.dslConnectInfo.buffPtr);
		break;   
	      case kDslPLNMarginPerBin:
		globalVar.PLNNoiseMarginPerBinDecLevel=status->param.dslConnectInfo.value;
		break;
	      case kDslPLNMarginBroadband:
		globalVar.PLNNoiseMarginBroadbandDecLevel=status->param.dslConnectInfo.value;
		break;
	      case kDslPerBinMsrCounter:
		globalVar.PLNPerBinMeasurementCounterRcvFlag=1;
		globalVar.PLNUpdtCount=status->param.dslConnectInfo.value;
		break;
	      case kDslBroadbandMsrCounter:
		globalVar.PLNBroadbandMsrCounterRcvFlag=1;
		globalVar.PLNUpdtCount=status->param.dslConnectInfo.value;
		break;
	      case kDslInpBinTablePtr:
		globalVar.PLNinpBinTableLen=status->param.dslConnectInfo.value;
		globalVar.PLNnInpBin=globalVar.PLNinpBinTableLen/2;
		globalVar.pPLNinpBinTablePtr=status->param.dslConnectInfo.buffPtr;
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN Status InpBin Table Ptr update Addr %X Length %d ",0,status->param.dslConnectInfo.buffPtr,globalVar.PLNinpBinTableLen);
		break;
	      case kDslItaBinTablePtr:
		globalVar.PLNitaBinTableLen=status->param.dslConnectInfo.value;
		globalVar.PLNnItaBin=globalVar.PLNitaBinTableLen/2;
		globalVar.pPLNitaBinTablePtr=status->param.dslConnectInfo.buffPtr;
		if(globalVar.PLNMessageReadRspFlag==1)
		  PLNReadResponses(gDslVars,kDslItaBinTablePtr);
		__SoftDslPrintf(gDslVars, "G992p3Ovh PLN Status Ita Table Ptr update Addr %X Length %d ",0,status->param.dslConnectInfo.buffPtr,globalVar.PLNitaBinTableLen);
		break;
	      case kDslPlnState:
		globalVar.PLNActiveFlag=status->param.dslConnectInfo.value;
		break;
		
	      }
	    break;
	   
	  case kDslEscapeToG994p1Status:
	    G992p3OvhMsgReset(gDslVars);
	    break;

	  case kDslTrainingStatus:
		switch (status->param.dslTrainingInfo.code) {
		case kG992FireState:
			if( status->param.dslTrainingInfo.value & kFireUsEnabled )
				mgntCntReadCmd[3]=kG992p3OvhMsgCmdCntReadId1;
			break;
		case kDslStartedG994p1:
		case kG994p1EventToneDetected:
		case kDslStartedT1p413HS:
		case kDslT1p413ReturntoStartup:
			mgntCntReadCmd[3]=kG992p3OvhMsgCmdCntReadId;
			break;
		case kDslG992p2TxShowtimeActive:
			break;
		case kDslG992p2RxShowtimeActive:
#if 1
		{
			ulong					mibLen;
			adslMibInfo				*pMib;
			pMib = (void *) AdslMibGetObjectValue (gDslVars,NULL,0,NULL,&mibLen);
			
			if( pMib->adslAtucPhys.adslVendorID[2] == 'G' &&
			    pMib->adslAtucPhys.adslVendorID[3] == 'S' &&
			    pMib->adslAtucPhys.adslVendorID[4] == 'P' &&
			    pMib->adslAtucPhys.adslVendorID[5] == 'N' ) {
			    
			    	G992p3OvhMsgPollCmdRateChange(gDslVars);
				__SoftDslPrintf(gDslVars, "CPE is connecting GSPN CO",0);		
			}
		}
#else
			globalVar.setup &= ~kG992p3ClearEocWorkaround;
#endif
	      break;
	    case kG992SetPLNMessageBase:
	      globalVar.kG992p3OvhMsgCmdPLN=status->param.dslTrainingInfo.value;
	      break;
	    }
	    break;

	  case kDslOLRRequestStatus:
	    {
		uchar		*pOlrCmd;
		adslMibInfo 	*pMib;
		long			mibLen;
			
		olrCmd[kG992p3AddrField] = kG992p3PriorityHigh;
		olrCmd[kG992p3CmdCode]	 = kG992p3OvhMsgCmdOLR;
		olrCmd[kG992p3CmdCode+1] = status->param.dslOLRRequest.msgType;
		olrLen = kG992p3CmdCode+2;
	    	if ((status->param.dslOLRRequest.msgType != kOLRRequestType1) && (status->param.dslOLRRequest.msgType != kOLRRequestType4)) {
			WriteCnt16(olrCmd+kG992p3CmdCode+2, status->param.dslOLRRequest.L[0]);
			olrCmd[kG992p3CmdCode+4] = status->param.dslOLRRequest.B[0];
			olrLen += 3;
	    	}
		pOlrCmd = olrCmd + olrLen;
		if (status->param.dslOLRRequest.msgType < kOLRRequestType4) {  /* G.992.3 request */
			pOlrCmd[0] = status->param.dslOLRRequest.nCarrs;
			olrLen1 = 3*status->param.dslOLRRequest.nCarrs;
			olrLen++;
		}
		else {
			WriteCnt16(pOlrCmd, status->param.dslOLRRequest.nCarrs);
			olrLen1 = 4*status->param.dslOLRRequest.nCarrs;
			olrLen += 2;
		}
	      
		G992p3OvhMsgXmtSendOLRCmd(gDslVars,
			olrCmd, olrLen,
			status->param.dslOLRRequest.carrParamPtr, olrLen1,
			true);
		
		mibLen = sizeof(adslMibInfo);
		pMib = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
		pMib->adslStat.bitswapStat.rcvCnt++;		
		}
	    break;
	    
	  case kDslOLRResponseStatus:
	    if (!G992p3OvhMsgXmtRspBusy()) {
	      globalVar.txRspBuf[kG992p3AddrField] = kG992p3PriorityHigh;
	      globalVar.txRspMsgNum ^= 1;
	      globalVar.txRspBuf[kG992p3CtrlField] = kG992p3Rsp | globalVar.txRspMsgNum;
	      globalVar.txRspBuf[kG992p3CmdCode]	 = kG992p3OvhMsgCmdOLR;
	      globalVar.txRspBuf[kG992p3CmdCode+1] = status->param.value & 0xFFFF; /* reject code */
	      globalVar.txRspBuf[kG992p3CmdCode+2] = status->param.value >> 16;	 /* reason code */
	      G992p3OvhMsgXmtSendRsp(gDslVars, kG992p3CmdCode+3);
	    }
	    break;
	    
	  case kDslPwrMgrStatus:
	    if (G992p3OvhMsgXmtPwrRspBusy())
	      break;
	    
	    DslFrameInit (gDslVars, &globalVar.txPwrRspFrame);
	    globalVar.txPwrRspBuf0[kG992p3AddrField] = kG992p3PriorityNormal;
	    globalVar.txRspMsgNum ^= 1;
	    globalVar.txPwrRspBuf0[kG992p3CtrlField] = kG992p3Rsp | globalVar.txRspMsgNum;
	    globalVar.txPwrRspBuf0[kG992p3CmdCode]	 = kG992p3OvhMsgCmdPower;
	    globalVar.txPwrRspBuf0[kG992p3CmdSubCode]= status->param.dslPwrMsg.msgType;
	    if ((kPwrL2Grant == status->param.dslPwrMsg.msgType) ||
		(kPwrL2Grant2p == status->param.dslPwrMsg.msgType)) {
#ifdef G992P3_DBG_PRINT
	      __SoftDslPrintf(gDslVars, "kDslPwrMgrStatus: msgLen=%d\n", 0, status->param.dslPwrMsg.param.msg.msgLen);
#endif
	      DslFrameBufferSetLength (gDslVars, &globalVar.txPwrRspFrBuf0, 4);
	      DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txPwrRspFrame, &globalVar.txPwrRspFrBuf0);
	      
	      DslFrameBufferSetAddress (gDslVars, &globalVar.txPwrRspFrBuf1, status->param.dslPwrMsg.param.msg.msgData);
	      DslFrameBufferSetLength (gDslVars, &globalVar.txPwrRspFrBuf1, status->param.dslPwrMsg.param.msg.msgLen);
	      DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txPwrRspFrame, &globalVar.txPwrRspFrBuf1);
	      G992p3OvhMsgXmtSendLongFrame(gDslVars, &globalVar.txPwrRspFrame);
	    }
	    else {
	      globalVar.txPwrRspBuf0[kG992p3CmdSubCode+1]= status->param.dslPwrMsg.param.value;
	      olrLen = kG992p3CmdSubCode + 2;
	      DslFrameBufferSetLength (gDslVars, &globalVar.txPwrRspFrBuf0, olrLen);
	      DslFrameEnqueBufferAtBack (gDslVars, &globalVar.txPwrRspFrame, &globalVar.txPwrRspFrBuf0);
	      G992p3OvhMsgXmtSendFrame(gDslVars, &globalVar.txPwrRspFrame);
	    }
	    break;

#if 0
	  case kDslShowtimeSNRMarginInfo:
	    {
	      static uchar stuffData[676 - sizeof(qlnReadCmd)];
	      int			 i;
	      
	      
	      if (10 != stuffData[10])
		for (i = 0; i < sizeof(stuffData); i++)
		  stuffData[i] = i & 0xFF;
	      
	      G992p3OvhMsgXmtSendCmd(gDslVars, qlnReadCmd,sizeof(qlnReadCmd), stuffData, sizeof(stuffData), true, true);
			}
	    break;
#endif
	  default:
	    break;
	  }
	
	if ( globalVar.txL3Rq && !G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc) ) {	
		if(G992p3OvhMsgXmtSendCmd(gDslVars, l3RequestCmd,sizeof(l3RequestCmd), NULL,0, true, true)) {
			globalVar.txL3Rq = false;
			globalVar.txFlags |= kTxCmdL3WaitingAck;
		}
	}
	
	if (0 == AdslMibPowerState(gDslVars))
		globalVar.txFlags &= ~kTxCmdL0WaitingAck;
	else 	if ( globalVar.txL0Rq && !G992p3FrameIsBitSet(gDslVars, globalVar.lastTxCmdFrame, kFrameClearEoc) ) {
		if(G992p3OvhMsgXmtSendCmd(gDslVars, l0RequestCmd,sizeof(l0RequestCmd), NULL,0, true, true)) {
			globalVar.txL0Rq = false;
			globalVar.txFlags |= kTxCmdL0WaitingAck;
		}
	}
}

#else /* G992P3 */

Public Boolean  G992p3OvhMsgInit(
			void					*gDslVars, 
			bitMap					setup, 
			dslFrameHandlerType		rxReturnFramePtr,
			dslFrameHandlerType		txSendFramePtr,
			dslFrameHandlerType		txSendCompletePtr,
			dslCommandHandlerType	commandHandler,
			dslStatusHandlerType	statusHandler)
{
	return true;
}

Public int G992p3OvhMsgIndicateRcvFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	return 0;
}

Public void G992p3OvhMsgReturnFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
}

Public int G992p3OvhMsgSendCompleteFrame(void *gDslVars, void *pVc, ulong mid, dslFrame *pFrame)
{
	return 0;
}

Public void G992p3OvhMsgClose(void *gDslVars)
{
}
	
Public void G992p3OvhMsgTimer(void *gDslVars, long timeQ24ms)
{
}
	
Public Boolean G992p3OvhMsgCommandHandler(void *gDslVars, dslCommandStruct *cmd)
{
	return false;
}

Public void G992p3OvhMsgStatusSnooper (void *gDslVars, dslStatusStruct *status)
{
}

Public void	G992p3OvhMsgSetL3(void *gDslVars)
{
}

Public void	G992p3OvhMsgSetL0(void *gDslVars)
{
}

#endif
