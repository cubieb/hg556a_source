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
 * AdslMibObj.c -- Adsl MIB object access functions
 *
 * Description:
 *	This file contains functions for access to ADSL MIB (RFC 2662) data
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslMibObj.c,v 1.1.2.1 2009/11/19 06:39:12 l43571 Exp $
 *
 * $Log: AdslMibObj.c,v $
 * Revision 1.1.2.1  2009/11/19 06:39:12  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:41:01  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:21  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:15  z30370
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
 * Revision 1.8  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.7  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.6  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.5  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.4  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.3  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.2  2002/01/02 19:13:19  liang
 * Change memcpy to BlockByteMove.
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "AdslMib.h"
#include "AdslMibOid.h"
#include "BlockUtil.h"

#define	globalVar	gAdslMibVars
extern adslCfgProfile	adslCoreCfgProfile;
/*
**
**		ATM TC (Transmission Convergence aka PHY) MIB data oblects
**
*/

Private void * MibGetAtmTcObjPtr(void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	atmPhyDataEntrty	*pAtmData;
	void				*pObj = NULL;

	if ((objIdLen < 2) || (objId[0] != kOidAtmMibObjects) || (objId[1] != kOidAtmTcTable))
		return NULL;

	pAtmData = (kAdslIntlChannel == AdslMibGetActiveChannel(gDslVars) ?
				&globalVar.adslMib.adslChanIntlAtmPhyData : &globalVar.adslMib.adslChanFastAtmPhyData);
	if (objIdLen == 2) {
		*objLen = sizeof(atmPhyDataEntrty);
		return pAtmData;
	}
	if (objId[2] != kOidAtmTcEntry)
		return NULL;
		
	if (objIdLen == 3) {
		*objLen = sizeof(atmPhyDataEntrty);
		return pAtmData;
	}

	if (objId[3] > kOidAtmAlarmState)
		return NULL;

	if (kOidAtmOcdEvents == objId[3]) {
		*objLen = sizeof(pAtmData->atmInterfaceOCDEvents);
		pObj = &pAtmData->atmInterfaceOCDEvents;
	}
	else if (kOidAtmAlarmState == objId[3]) {
		*objLen = sizeof(pAtmData->atmInterfaceTCAlarmState);
		pObj = &pAtmData->atmInterfaceTCAlarmState;
	}

	return pObj;
}


/*
**
**		ADSL Line MIB data oblects
**
*/

Private void * MibGetAdslLineTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;	

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslLine);
		return &globalVar.adslMib.adslLine;
	}
	if (objId[1] != kOidAdslLineEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslLine);
		return &globalVar.adslMib.adslLine;
	}

	switch (objId[2]) {
		case kOidAdslLineCoding:
			*objLen = sizeof(globalVar.adslMib.adslLine.adslLineCoding);
			pObj = &globalVar.adslMib.adslLine.adslLineCoding;
			break;
		case kOidAdslLineType:
			*objLen = sizeof(globalVar.adslMib.adslLine.adslLineType);
			pObj = &globalVar.adslMib.adslLine.adslLineType;
			break;
		case kOidAdslLineSpecific:
		case kOidAdslLineConfProfile:
		case kOidAdslLineAlarmConfProfile:
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

Private void * MibGetAdslPhysTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;	

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPhys);
		return &globalVar.adslMib.adslPhys;
	}
	if (objId[0] != kOidAdslPhysEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPhys);
		return &globalVar.adslMib.adslPhys;
	}

	switch (objId[1]) {
		case kOidAdslPhysInvSerialNumber:
		case kOidAdslPhysInvVendorID:
		case kOidAdslPhysInvVersionNumber:
			pObj = NULL;
			break;
			
		case kOidAdslPhysCurrSnrMgn:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrSnrMgn);
			pObj = &globalVar.adslMib.adslPhys.adslCurrSnrMgn;
			break;
		case kOidAdslPhysCurrAtn:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrAtn);
			pObj = &globalVar.adslMib.adslPhys.adslCurrAtn;
			break;
		case kOidAdslPhysCurrStatus:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrStatus);
			pObj = &globalVar.adslMib.adslPhys.adslCurrStatus;
			break;
		case kOidAdslPhysCurrOutputPwr:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrOutputPwr);
			pObj = &globalVar.adslMib.adslPhys.adslCurrOutputPwr;
			break;
		case kOidAdslPhysCurrAttainableRate:
			*objLen = sizeof(globalVar.adslMib.adslPhys.adslCurrAttainableRate);
			pObj = &globalVar.adslMib.adslPhys.adslCurrAttainableRate;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfTableCounterPtr (adslPerfCounters *pPerf, uchar cntId, ulong *objLen)
{
	void	*pObj = NULL;

	switch (cntId) {
		case  kOidAdslPerfLofs:
			*objLen = sizeof(pPerf->adslLofs);
			pObj = &pPerf->adslLofs;
			break;
		case  kOidAdslPerfLoss:
			*objLen = sizeof(pPerf->adslLoss);
			pObj = &pPerf->adslLoss;
			break;
		case  kOidAdslPerfLprs:
			*objLen = sizeof(pPerf->adslLprs);
			pObj = &pPerf->adslLprs;
			break;
		case  kOidAdslPerfESs:
			*objLen = sizeof(pPerf->adslESs);
			pObj = &pPerf->adslESs;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfTableObjPtr (void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfData);
		return &globalVar.adslMib.adslPerfData;
	}
	if (objId[0] != kOidAdslPerfDataEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfData);
		return &globalVar.adslMib.adslPerfData;
	}

	switch (objId[1]) {
		case  kOidAdslPerfLofs:
		case  kOidAdslPerfLoss:
		case  kOidAdslPerfLprs:
		case  kOidAdslPerfESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfTotal, objId[1], objLen);
			break;
		case  kOidAdslPerfValidIntervals:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfValidIntervals);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfValidIntervals;
			break;	
		case  kOidAdslPerfInvalidIntervals:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfInvalidIntervals);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfInvalidIntervals;
			break;	
		case  kOidAdslPerfCurr15MinTimeElapsed: 	
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed;
			break;
		case  kOidAdslPerfCurr15MinLofs:
		case  kOidAdslPerfCurr15MinLoss:
		case  kOidAdslPerfCurr15MinLprs:
		case  kOidAdslPerfCurr15MinESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfCurr15Min, objId[1]-kOidAdslPerfCurr15MinLofs+1, objLen);
			break;	
		case  kOidAdslPerfCurr1DayTimeElapsed:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed);
			pObj = &globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed;
			break;
		case  kOidAdslPerfCurr1DayLofs:
		case  kOidAdslPerfCurr1DayLoss:
		case  kOidAdslPerfCurr1DayLprs:
		case  kOidAdslPerfCurr1DayESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfCurr1Day, objId[1]-kOidAdslPerfCurr1DayLofs+1, objLen);
			break;	
		case  kOidAdslPerfPrev1DayMoniSecs:
			*objLen = sizeof(globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs);
			pObj = &globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs;
			break;
		case  kOidAdslPerfPrev1DayLofs:
		case  kOidAdslPerfPrev1DayLoss:
		case  kOidAdslPerfPrev1DayLprs:
		case  kOidAdslPerfPrev1DayESs:
			pObj = MibGetAdslPerfTableCounterPtr (&globalVar.adslMib.adslPerfData.perfPrev1Day, objId[1]-kOidAdslPerfPrev1DayLofs+1, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslPerfIntervalObjPtr (void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;	
	uchar	ind;

	if (0 == objIdLen) {
		*objLen = sizeof(globalVar.adslMib.adslPerfIntervals);
		return &globalVar.adslMib.adslPerfIntervals;
	}
	if ((objId[0] != kOidAdslPerfIntervalEntry) || (objIdLen < 4))
		return NULL;
	ind = objId[3];
	if ((ind == 0) || (ind > globalVar.adslMib.adslPerfData.adslPerfValidIntervals))
		return NULL;

	switch (objId[1]) {
		case kOidAdslIntervalNumber:
			*objLen = sizeof(int);
			globalVar.scratchData = ind;
			pObj = &globalVar.scratchData;
			break;
		case kOidAdslIntervalLofs:
		case kOidAdslIntervalLoss:
		case kOidAdslIntervalLprs:
		case kOidAdslIntervalESs:
			pObj = MibGetAdslPerfTableCounterPtr(&globalVar.adslMib.adslPerfIntervals[ind-1], objId[1]-kOidAdslIntervalLofs+1, objLen);
			break;

		case kOidAdslIntervalValidData:
			*objLen = sizeof(int);
			globalVar.scratchData = true;
			pObj = &globalVar.scratchData;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslObjPtr (void *gDslVars, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;	

	if ((objIdLen < 3) || (objId[0] != kOidAdslLine) || (objId[1] != kOidAdslMibObjects))
		return NULL;

	switch (objId[2]) {
		case kOidAdslLineTable:
			pObj = MibGetAdslLineTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturPhysTable:
			pObj = MibGetAdslPhysTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAtucPhysTable:
			pObj	= &globalVar.adslMib.adslAtucPhys;
			*objLen = sizeof(globalVar.adslMib.adslAtucPhys);
			break;
		case kOidAdslAturPerfDataTable:
			pObj = MibGetAdslPerfTableObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturPerfIntervalTable:
			pObj = MibGetAdslPerfIntervalObjPtr (gDslVars, objId+3, objIdLen-3, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

/*
**
**		ADSL Channel MIB data oblects
**
*/

Private void * MibGetAdslChanTableObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, ulong *objLen)
{
	void			*pObj = NULL;
	adslChanEntry	*pChan;

	pChan = (kAdslIntlChannel == chId ? &globalVar.adslMib.adslChanIntl: &globalVar.adslMib.adslChanFast);

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanEntry);
		return pChan;
	}
	if (objId[0] != kOidAdslChanEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(adslChanEntry);
		return pChan;
	}

	switch (objId[1]) {
		case kOidAdslChanInterleaveDelay:
			*objLen = sizeof(pChan->adslChanIntlDelay);
			pObj = &pChan->adslChanIntlDelay;
			break;
		case kOidAdslChanCurrTxRate:
			*objLen = sizeof(pChan->adslChanCurrTxRate);
			pObj = &pChan->adslChanCurrTxRate;
			break;
		case kOidAdslChanPrevTxRate:
			*objLen = sizeof(pChan->adslChanPrevTxRate);
			pObj = &pChan->adslChanPrevTxRate;
			break;
		case kOidAdslChanCrcBlockLength:
			*objLen = sizeof(pChan->adslChanCrcBlockLength);
			pObj = &pChan->adslChanCrcBlockLength;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfTableCounterPtr (adslChanCounters *pPerf, uchar cntId, ulong *objLen)
{
	void	*pObj = NULL;

	switch (cntId) {
		case kOidAdslChanReceivedBlks:
			*objLen = sizeof(pPerf->adslChanReceivedBlks);
			pObj = &pPerf->adslChanReceivedBlks;
			break;
		case kOidAdslChanTransmittedBlks:
			*objLen = sizeof(pPerf->adslChanTransmittedBlks);
			pObj = &pPerf->adslChanTransmittedBlks;
			break;
		case kOidAdslChanCorrectedBlks:
			*objLen = sizeof(pPerf->adslChanCorrectedBlks);
			pObj = &pPerf->adslChanCorrectedBlks;
			break;
		case kOidAdslChanUncorrectBlks:
			*objLen = sizeof(pPerf->adslChanUncorrectBlks);
			pObj = &pPerf->adslChanUncorrectBlks;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfTableObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, ulong *objLen)
{
	void					*pObj = NULL;
	adslChanPerfDataEntry	*pChanPerf;

	pChanPerf = (kAdslIntlChannel == chId ? &globalVar.adslMib.adslChanIntlPerfData : &globalVar.adslMib.adslChanFastPerfData);

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanPerfDataEntry);
		return pChanPerf;
	}
	if (objId[0] != kOidAdslChanPerfEntry)
		return NULL;
	if (1 == objIdLen) {
		*objLen = sizeof(adslChanPerfDataEntry);
		return pChanPerf;
	}

	switch (objId[1]) {
		case kOidAdslChanReceivedBlks:
		case kOidAdslChanTransmittedBlks:
		case kOidAdslChanCorrectedBlks:
		case kOidAdslChanUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfTotal, objId[1], objLen);
			break;

		case kOidAdslChanPerfValidIntervals:
			*objLen = sizeof(pChanPerf->adslChanPerfValidIntervals);
			pObj = &pChanPerf->adslChanPerfValidIntervals;
			break;	
		case kOidAdslChanPerfInvalidIntervals:
			*objLen = sizeof(pChanPerf->adslChanPerfInvalidIntervals);
			pObj = &pChanPerf->adslChanPerfInvalidIntervals;
			break;	
		case kOidAdslChanPerfCurr15MinTimeElapsed:
			*objLen = sizeof(pChanPerf->adslPerfCurr15MinTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr15MinTimeElapsed;
			break;	

		case kOidAdslChanPerfCurr15MinReceivedBlks:
		case kOidAdslChanPerfCurr15MinTransmittedBlks:
		case kOidAdslChanPerfCurr15MinCorrectedBlks:
		case kOidAdslChanPerfCurr15MinUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfCurr15Min, objId[1]-kOidAdslChanPerfCurr15MinReceivedBlks+1, objLen);
			break;

		case kOidAdslChanPerfCurr1DayTimeElapsed:
			*objLen = sizeof(pChanPerf->adslPerfCurr1DayTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr1DayTimeElapsed;
			break;	

		case kOidAdslChanPerfCurr1DayReceivedBlks:
		case kOidAdslChanPerfCurr1DayTransmittedBlks:
		case kOidAdslChanPerfCurr1DayCorrectedBlks:
		case kOidAdslChanPerfCurr1DayUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfCurr1Day, objId[1]-kOidAdslChanPerfCurr1DayReceivedBlks+1, objLen);
			break;

		case kOidAdslChanPerfPrev1DayMoniSecs:
			*objLen = sizeof(pChanPerf->adslPerfCurr1DayTimeElapsed);
			pObj = &pChanPerf->adslPerfCurr1DayTimeElapsed;
			break;	

		case kOidAdslChanPerfPrev1DayReceivedBlks:
		case kOidAdslChanPerfPrev1DayTransmittedBlks:
		case kOidAdslChanPerfPrev1DayCorrectedBlks:
		case kOidAdslChanPerfPrev1DayUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr (&pChanPerf->perfPrev1Day, objId[1]-kOidAdslChanPerfPrev1DayReceivedBlks+1, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanPerfIntervalObjPtr(void *gDslVars, uchar chId, uchar *objId, int objIdLen, ulong *objLen)
{
	void				*pObj = NULL;
	adslChanCounters	*pIntervals;
	uchar				ind, nInt;

	if (kAdslIntlChannel == chId) {
		pIntervals = globalVar.adslMib.adslChanIntlPerfIntervals;
		nInt = globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals;
	}
	else {
		pIntervals = globalVar.adslMib.adslChanFastPerfIntervals;
		nInt = globalVar.adslMib.adslChanFastPerfData.adslChanPerfValidIntervals;
	}

	if (0 == objIdLen) {
		*objLen = sizeof(adslChanCounters) * nInt;
		return pIntervals;
	}
	if ((objId[0] != kOidAdslChanIntervalEntry) || (objIdLen < 4))
		return NULL;
	ind = objId[3];
	if ((ind == 0) || (ind > nInt))
		return NULL;

	switch (objId[1]) {
		case kOidAdslChanIntervalNumber:
			*objLen = sizeof(int);
			globalVar.scratchData = ind;
			pObj = &globalVar.scratchData;
			break;
		case kOidAdslChanIntervalReceivedBlks:
		case kOidAdslChanIntervalTransmittedBlks:
		case kOidAdslChanIntervalCorrectedBlks:
		case kOidAdslChanIntervalUncorrectBlks:
			pObj = MibGetAdslChanPerfTableCounterPtr(pIntervals+ind-1, objId[1]-kOidAdslChanIntervalReceivedBlks+1, objLen);
			break;
		case kOidAdslChanIntervalValidData:
			*objLen = sizeof(int);
			globalVar.scratchData = true;
			pObj = &globalVar.scratchData;
			break;
		default:
			pObj = NULL;
			break;
	}
	return pObj;
}

Private void * MibGetAdslChanObjPtr (void *gDslVars, uchar chId, uchar *objId, int objIdLen, ulong *objLen)
{
	void	*pObj = NULL;

	if ((objIdLen < 3) || (objId[0] != kOidAdslLine) || (objId[1] != kOidAdslMibObjects))
		return NULL;

	if (chId != AdslMibGetActiveChannel(gDslVars))
		return NULL;

	switch (objId[2]) {
		case kOidAdslAturChanTable:
			pObj = MibGetAdslChanTableObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturChanPerfTable:
			pObj = MibGetAdslChanPerfTableObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		case kOidAdslAturChanIntervalTable:
			pObj = MibGetAdslChanPerfIntervalObjPtr (gDslVars, chId, objId+3, objIdLen-3, objLen);
			break;
		default:
			pObj = NULL;
			break;
	}

	return pObj;
}

Public int AdslMibSetObjectValue(
				void	*gDslVars, 
				uchar	*objId, 
				int		objIdLen,
				uchar	*dataBuf,
				ulong	*dataBufLen)
{
	dslCommandStruct		cmd;
	BcmAdslCoreDiagWriteStatusString("AdslMibSetObjectValue function \n");
	BcmAdslCoreDiagWriteStatusString("objId=%X %X %X %X ,objIdLen= %d, dataBufLen=%u dataBuf= %X %X",objId[0],objId[1],objId[2],objId[3],objIdLen,*dataBufLen,dataBuf[0],dataBuf[1]);
	switch (objId[0])
	{
		case kOidAdslPrivate:
			switch (objId[1]) {
				case kOidAdslPrivExtraInfo:
					switch (objId[2])
					{
						#ifdef ADSL_MIBOBJ_PLN
						case kOidAdslPrivPLNDurationBins:
							{
							int i,n;ushort* pPLNinpBinIntervalPtr;
							for(i=0;i<kPlnNumberOfDurationBins;i++)
								globalVar.PLNDurationBins[i]=0;
							AdslMibByteMove(*dataBufLen,dataBuf,globalVar.PLNDurationBins);
							BcmAdslCoreDiagWriteStatusString("PLNDurationBins=%X %X %X %X",globalVar.PLNDurationBins[0],globalVar.PLNDurationBins[1],globalVar.PLNDurationBins[2],globalVar.PLNDurationBins[3]);
							n=*dataBufLen;
							if(NULL!=(pPLNinpBinIntervalPtr=AdslCoreSharedMemAlloc(n))){
							memcpy(pPLNinpBinIntervalPtr,&dataBuf[0],n);
							BcmAdslCoreDiagWriteStatusString("PLNDurationBins=%X %X %X %X",pPLNinpBinIntervalPtr[0],pPLNinpBinIntervalPtr[1],pPLNinpBinIntervalPtr[2],pPLNinpBinIntervalPtr[3]);
							cmd.command=kDslPLNControlCmd;
							cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineInpBinTable;
							cmd.param.dslPlnSpec.nInpBin= globalVar.adslMib.adslPLNData.PLNNbDurBins;
							cmd.param.dslPlnSpec.inpBinPtr=pPLNinpBinIntervalPtr;
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd); 
							}
							else BcmAdslCoreDiagWriteStatusString("No Shared Mem");
							BcmAdslCoreDiagWriteStatusString("PLNDurationBins=%d cmd.param.dslPlnSpec.nInpBin=%d",globalVar.PLNDurationBins,cmd.param.dslPlnSpec.nInpBin);
							break;
							}
						case kOidAdslPrivPLNIntrArvlBins:
							{
							int i,n;ushort* pPLNitaBinIntervalPtr;
							for(i=0;i<kPlnNumberOfInterArrivalBins;i++)
								globalVar.PLNIntrArvlBins[i]=0;
							AdslMibByteMove(*dataBufLen,dataBuf,globalVar.PLNIntrArvlBins);
							BcmAdslCoreDiagWriteStatusString("PLNIntrArvlBins=%X %X %X %X",globalVar.PLNIntrArvlBins[0],globalVar.PLNIntrArvlBins[1],globalVar.PLNIntrArvlBins[2],globalVar.PLNIntrArvlBins[3]);
							n=*dataBufLen;
							if(NULL!=(pPLNitaBinIntervalPtr=AdslCoreSharedMemAlloc(n))){
							memcpy(pPLNitaBinIntervalPtr,&dataBuf[0],n);
							cmd.command=kDslPLNControlCmd;
							cmd.param.dslPlnSpec.plnCmd=kDslPLNControlDefineItaBinTable;
							cmd.param.dslPlnSpec.nItaBin=globalVar.adslMib.adslPLNData.PLNNbIntArrBins;
							cmd.param.dslPlnSpec.itaBinPtr= pPLNitaBinIntervalPtr;
							(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
							}
							BcmAdslCoreDiagWriteStatusString( "cmd.param.dslPlnSpec.nItaBin=%d",cmd.param.dslPlnSpec.nItaBin);
							break;
							}
						case kOidAdslExtraPLNData:
							switch (objId[3]){
							case kOidAdslExtraPLNDataThldBB:
									globalVar.adslMib.adslPLNData.PLNThldBB=dataBuf[0];
									globalVar.adslMib.adslPLNData.PLNThldBB<<=8;
									globalVar.adslMib.adslPLNData.PLNThldBB|=dataBuf[1];
									break;
								case kOidAdslExtraPLNDataThldPerTone:
									globalVar.adslMib.adslPLNData.PLNThldPerTone=dataBuf[0];
									globalVar.adslMib.adslPLNData.PLNThldPerTone<<=8;
									globalVar.adslMib.adslPLNData.PLNThldPerTone|=dataBuf[1];
									break;
								case kOidAdslExtraPLNDataPLNState:
									BcmAdslCoreDiagWriteStatusString("State dataBuf=%d \n",dataBuf);
									if(*dataBuf==2){
										BcmAdslCoreDiagWriteStatusString("Start Requested\n");
										globalVar.adslMib.adslPLNData.PLNState=1;
										}
										else if(*dataBuf==3){
											BcmAdslCoreDiagWriteStatusString("Stop Requested\n");
											globalVar.adslMib.adslPLNData.PLNState=0;
										}
									cmd.command=kDslPLNControlCmd;
									cmd.param.value=kDslPLNControlStop;
									(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
									if(globalVar.adslMib.adslPLNData.PLNState==1)
									{
										cmd.command=kDslPLNControlCmd;
										cmd.param.dslPlnSpec.plnCmd=kDslPLNControlStart;
										cmd.param.dslPlnSpec.mgnDescreaseLevelPerBin= globalVar.adslMib.adslPLNData.PLNThldPerTone;
										cmd.param.dslPlnSpec.mgnDescreaseLevelBand=globalVar.adslMib.adslPLNData.PLNThldBB;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd); 
									}
									break;
								case kOidAdslExtraPLNDataNbDurBins:
									if(*dataBuf>kPlnNumberOfDurationBins)
										*dataBuf=kPlnNumberOfDurationBins;
									globalVar.adslMib.adslPLNData.PLNNbDurBins=dataBuf[0];
									BcmAdslCoreDiagWriteStatusString("PLNNbDurBins = %d",globalVar.adslMib.adslPLNData.PLNNbDurBins);
									break;
								case kOidAdslExtraPLNDataNbIntArrBins:
									if(*dataBuf>kPlnNumberOfInterArrivalBins)
										*dataBuf=kPlnNumberOfInterArrivalBins;
									globalVar.adslMib.adslPLNData.PLNNbIntArrBins=dataBuf[0];
									BcmAdslCoreDiagWriteStatusString("PLNNbIntrArrBins = %d",globalVar.adslMib.adslPLNData.PLNNbIntArrBins);
									break;
								case kOidAdslExtraPLNDataUpdate:
									if(*dataBuf==1)
									{
										globalVar.adslMib.adslPLNData.PLNUpdateData=0;
										cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlPeakNoiseGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										 cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlThldViolationGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd;
										cmd.param.value=kDslPLNControlImpulseNoiseEventGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd; 
										cmd.param.value= kDslPLNControlGetStatus;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
										cmd.command=kDslPLNControlCmd; 
										cmd.param.value=kDslPLNControlImpulseNoiseTimeGetPtr;
										(*globalVar.cmdHandlerPtr)(gDslVars, &cmd);
									}
									break;
										
									
							}	
							break;
					#endif
					}
					break;
			}
			break;
	}
return kAdslMibStatusSuccess;
}
									
Public int	AdslMibGetObjectValue (
				void	*gDslVars, 
				uchar	*objId, 
				int		objIdLen,
				uchar	*dataBuf,
				ulong	*dataBufLen)
{
	uchar		*pObj;
	ulong		objLen, bufLen;

	bufLen = *dataBufLen;
	pObj = NULL;
	if ((NULL == objId) || (0 == objIdLen)) {
		pObj = (void *) &globalVar.adslMib;
		objLen = sizeof (globalVar.adslMib);
	}
	else {
		switch (objId[0]) {
			case kOidAdsl:
				pObj = MibGetAdslObjPtr (gDslVars, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAdslInterleave:
				pObj = MibGetAdslChanObjPtr (gDslVars, kAdslIntlChannel, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAdslFast:
				pObj = MibGetAdslChanObjPtr (gDslVars, kAdslFastChannel, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAtm:
				pObj = MibGetAtmTcObjPtr (gDslVars, objId+1, objIdLen-1, &objLen);
				break;
			case kOidAdslPhyCfg:
				pObj=(void *) &adslCoreCfgProfile;
				objLen=sizeof(adslCfgProfile);
				break;
			case kOidAdslPrivate:
				switch (objId[1]) {
					case kOidAdslPrivSNR:
						pObj = (void *) &globalVar.snr;
						objLen = sizeof (globalVar.snr[0]) * 512/*globalVar.nTones*/;
						BcmAdslCoreDiagWriteStatusString("kOidAdslPrivSNR Len=%d",globalVar.nTones);
						break;
					case kOidAdslPrivBitAlloc:
						pObj = (void *) &globalVar.bitAlloc;
						objLen = sizeof (globalVar.bitAlloc[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivGain:
						pObj = (void *) &globalVar.gain;
						objLen = sizeof (globalVar.gain[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivShowtimeMargin:
						pObj = (void *) &globalVar.showtimeMargin;
						objLen = sizeof (globalVar.showtimeMargin[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivChanCharLin:
						pObj = (void *) &globalVar.chanCharLin;
						objLen = sizeof (globalVar.chanCharLin[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivChanCharLog:
						pObj = (void *) &globalVar.chanCharLog;
						objLen = sizeof (globalVar.chanCharLog[0]) * globalVar.nTones;
						break;
					case kOidAdslPrivQuietLineNoise:
						pObj = (void *) &globalVar.quietLineNoise;
						objLen = sizeof (globalVar.quietLineNoise[0]) * globalVar.nTones;
						break;
					#ifdef ADSL_MIBOBJ_PLN
					case kOidAdslPrivPLNDurationBins:
						pObj = (void *) &globalVar.PLNDurationBins;
						objLen = sizeof (globalVar.PLNDurationBins[0])*kPlnNumberOfDurationBins;
						break;
					case kOidAdslPrivPLNIntrArvlBins:
						pObj = (void *) &globalVar.PLNIntrArvlBins;
						objLen = sizeof (globalVar.PLNIntrArvlBins[0])*kPlnNumberOfInterArrivalBins;
						break;
					case kOidAdslPrivPLNValueps :
						pObj = (void *) &globalVar.PLNValueps;
						objLen = sizeof (globalVar.PLNValueps[0])*(kAdslMibToneNum*2-32);
						break;
					case kOidAdslPrivPLNThldCntps  :
						pObj = (void *) &globalVar.PLNThldCntps;
						objLen = sizeof (globalVar.PLNThldCntps[0])*(kAdslMibToneNum*2-32);
						break;
					case kOidAdslPrivPLNDurationHist :
						pObj = (void *) &globalVar.PLNDurationHist;
						objLen = sizeof (globalVar.PLNDurationHist[0])*kPlnNumberOfDurationBins;
						break;
					case kOidAdslPrivPLNIntrArvlHist:
						pObj = (void *) &globalVar.PLNIntrArvlHist;
						objLen = sizeof (globalVar.PLNIntrArvlHist[0])*kPlnNumberOfInterArrivalBins;
						break;
					#endif
					case kOidAdslPrivNLDistNoise:
						pObj = (void *) &globalVar.distNoisedB;
						objLen = sizeof (globalVar.distNoisedB[0])*kAdslMibToneNum*2;
						break;
					case kOidAdslPrivExtraInfo:
					  switch (objId[2]) {
						case kOidAdslExtraConnectionInfo:
							pObj = (void *) &globalVar.adslMib.adslConnection;
							objLen = sizeof (globalVar.adslMib.adslConnection);
							break;
						case kOidAdslExtraConnectionStat:
							pObj = (void *) &globalVar.adslMib.adslStat;
							objLen = sizeof (globalVar.adslMib.adslStat);
							break;
						case kOidAdslExtraFramingMode:
							pObj = (void *) &globalVar.adslMib.adslFramingMode;
							objLen = sizeof (globalVar.adslMib.adslFramingMode);
							break;
						case kOidAdslExtraTrainingState:
							pObj = (void *) &globalVar.adslMib.adslTrainingState;
							objLen = sizeof (globalVar.adslMib.adslTrainingState);
							break;
						case kOidAdslExtraNonStdFramingAdjustK:
							pObj = (void *) &globalVar.adslMib.adslRxNonStdFramingAdjustK;
							objLen = sizeof (globalVar.adslMib.adslRxNonStdFramingAdjustK);
							break;
						case kOidAdslExtraAtmStat:
							pObj = (void *) &globalVar.adslMib.atmStat;
							objLen = sizeof (globalVar.adslMib.atmStat);
							break;
						case kOidAdslExtraDiagModeData:
							pObj = (void *) &globalVar.adslMib.adslDiag;
							objLen = sizeof (globalVar.adslMib.adslDiag);
							break;
						case kOidAdslExtraAdsl2Info:
							pObj = (void *) &globalVar.adslMib.adsl2Info;
							objLen = sizeof (globalVar.adslMib.adsl2Info);
							break;
						case kOidAdslExtraTxPerfCounterInfo:
							pObj = (void *) &globalVar.adslMib.adslTxPerfTotal;
							objLen = sizeof (globalVar.adslMib.adslTxPerfTotal);
							break;
						case kOidAdslExtraNLInfo:
							pObj = (void *) &globalVar.adslMib.adslNonLinData;
							objLen= sizeof(globalVar.adslMib.adslNonLinData);
							break;
						#ifdef ADSL_MIBOBJ_PLN
						case kOidAdslExtraPLNInfo:
							pObj = (void *) &globalVar.adslMib.adslPLNData;
							objLen= sizeof(globalVar.adslMib.adslPLNData);
							break;
						#endif
					  }
					  break;
				}
				break;
		}
	}

	if (NULL == pObj)
		return kAdslMibStatusNoObject;

	*dataBufLen = objLen;
	if (NULL == dataBuf)
		return (int)pObj;

	if (objLen > bufLen)
		return kAdslMibStatusBufferTooSmall;

	AdslMibByteMove(objLen, pObj, dataBuf);
	return kAdslMibStatusSuccess;
}
