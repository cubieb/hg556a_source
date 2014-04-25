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
 * AdslMib.c -- Adsl MIB data manager
 *
 * Description:
 *  This file contains functions for ADSL MIB (RFC 2662) data management
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslMib.c,v 1.1.2.1 2009/11/19 06:39:12 l43571 Exp $
 *
 * $Log: AdslMib.c,v $
 * Revision 1.1.2.1  2009/11/19 06:39:12  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:41:00  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:20  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:10  z30370
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
 * Revision 1.16  2004/06/04 18:56:01  ilyas
 * Added counter for ADSL2 framing and performance
 *
 * Revision 1.15  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.14  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.13  2003/10/17 21:02:12  ilyas
 * Added more data for ADSL2
 *
 * Revision 1.12  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.10  2003/07/18 19:07:15  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.9  2002/11/13 21:32:49  ilyas
 * Added adjustK support for Centillium non-standard framing mode
 *
 * Revision 1.8  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.7  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.6  2002/02/01 06:42:48  ilyas
 * Ignore ASx chaanels for transmit coding parameters
 *
 * Revision 1.5  2002/01/13 22:25:40  ilyas
 * Added functions to get channels rate
 *
 * Revision 1.4  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.3  2002/01/02 19:13:57  liang
 * Fix compiler warning.
 *
 * Revision 1.2  2001/12/22 02:37:30  ilyas
 * Changed memset,memcpy function to BlockByte functions
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#include "SoftDsl.gh"

#include "AdslMib.h"
#include "BlockUtil.h"

#define globalVar   gAdslMibVars

#define k15MinInSeconds             (15*60)
#define k1DayInSeconds              (24*60*60)

#define Q4ToTenth(num)              ((((num) * 10) + 8) >> 4)
#define RestrictValue(n,l,r)        ((n) < (l) ? (l) : (n) > (r) ? (r) : (n))

#define NitroRate(rate)             ((((rate)*53)+48) / 49)
#define ActualRate(rate)    (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression ? NitroRate(rate) : rate)

long    secElapsedInDay;
long    secElapsedShTm;
long    per,rsRate;
int txUpdateStatFlag=0;
int txCntNotRcvYet=0;
#define IncPerfCounterVar(perfEntry, varname) do {                      \
    (*(adslPerfDataEntry*)(perfEntry)).perfTotal.varname++;             \
    (*(adslPerfDataEntry*)(perfEntry)).perfSinceShowTime.varname++;     \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr15Min.varname++;         \
    (*(adslPerfDataEntry*)(perfEntry)).perfCurr1Day.varname++;          \
} while (0)

#define AddBlockCounterVar(chEntry, varname, inc) do {                  \
    (*(adslChanPerfDataEntry *)(chEntry)).perfTotal.varname += inc;     \
    (*(adslChanPerfDataEntry *)(chEntry)).perfCurr15Min.varname += inc; \
    (*(adslChanPerfDataEntry *)(chEntry)).perfCurr1Day.varname += inc;  \
} while (0)

Private int SM_DECL AdslMibNotifyIdle (void *gDslVars, ulong event)
{
    return 0;
}

Public Boolean AdslMibIsAdsl2Mod(void *gDslVars)
{
    int     modType  = AdslMibGetModulationType(gDslVars);

    return ((kAdslModAdsl2 == modType) || (kAdslModAdsl2p == modType) || (kAdslModReAdsl2 == modType));
}

Public void AdslMibByteMove (int size, void* srcPtr, void* dstPtr)
{
    int     len = size & ~1;
    uchar   *sPtr, *dPtr;

    if (len)
        BlockByteMove (len, srcPtr, dstPtr);

    sPtr = ((uchar*)srcPtr) + len;
    dPtr = ((uchar*)dstPtr) + len;
    srcPtr = ((uchar*)srcPtr) + size;
    while (sPtr != srcPtr)
        *dPtr++ = *sPtr++;
}

Public void AdslMibByteClear(int size, void* dstPtr)
{
    uchar   *dPtr = dstPtr;
    uchar   *dPtrEnd = dPtr + size;

    while (dPtr != dPtrEnd)
        *dPtr++ = 0;
}

Public int AdslMibStrCopy(char *srcPtr, char *dstPtr)
{
    char    *sPtr = srcPtr;

    do {
        *dstPtr = *srcPtr++;
    } while (*dstPtr++ != 0);

    return srcPtr - sPtr;
}

Public int AdslMibGetActiveChannel(void *gDslVars)
{
    return (globalVar.adslMib.adslChanFast.adslChanCrcBlockLength ? kAdslFastChannel : kAdslIntlChannel);
}

Private int AdslMibGetFastBytesInFrame(G992CodingParams *param)
{
    int     n = 0;

#ifdef  G992P1_NEWFRAME
    if (param->NF >= 0x0200)    /* old PHY (A023a or earlier) */
        n = param->RSF - param->AS3BF;

    n +=  param->AS0BF;
    n += param->AS1BF;
    n += param->AS2BF;
    n += param->AS3BF;
    n += param->LS0CF;
    n += param->LS1BF;
    n += param->LS2BF;
#else
    n = 0;
#endif
    return n;
}

Private int AdslMibGetIntlBytesInFrame(G992CodingParams *param)
{
    int     n;

#ifdef  G992P1_NEWFRAME
    n =  param->AS0BI;
    n += param->AS1BI;
    n += param->AS2BI;
    n += param->AS3BI;
    n += param->LS0CI;
    n += param->LS1BI;
    n += param->LS2BI;
#else
    n = (param->K - 1);
#endif
    return n;
}

Public int AdslMibGetGetChannelRate(void *gDslVars, int dir, int channel)
{
    G992CodingParams *param;
    int               n;

    param = kAdslRcvDir == dir ? &globalVar.rcvParams : &globalVar.xmtParams;
#ifdef G992P3
    if (AdslMibIsAdsl2Mod(gDslVars)) {
        if (kAdslIntlChannel == channel)
            n = 1000 * ((kAdslRcvDir == dir) ? globalVar.adslMib.adsl2Info.rcvRate : globalVar.adslMib.adsl2Info.xmtRate);
        else
            n = 0;
    }
    else
#endif
    {
        if (kAdslIntlChannel == channel)
            n = AdslMibGetIntlBytesInFrame(param);
        else 
            n = AdslMibGetFastBytesInFrame(param);
        n *= 4000 * 8;

        if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
           n = NitroRate(n);
    }

    return n;
}

Private void AdslMibSetChanEntry(G992CodingParams *param, adslChanEntry *pChFast, adslChanEntry *pChIntl)
{
    int     n;

    pChIntl->adslChanIntlDelay = 4 + (param->S*param->D >> 2) + ((param->S - 1) >> 2);
    pChFast->adslChanPrevTxRate = pChFast->adslChanCurrTxRate;
    pChIntl->adslChanPrevTxRate = pChIntl->adslChanCurrTxRate;
    n = AdslMibGetFastBytesInFrame(param);
    pChFast->adslChanCrcBlockLength = n * 68;
    pChFast->adslChanCurrTxRate = n * 4000 * 8;

    n = AdslMibGetIntlBytesInFrame(param);
    pChIntl->adslChanCrcBlockLength = n * 68;
    pChIntl->adslChanCurrTxRate = n * 4000 * 8;
}

Private void AdslMibSetConnectionInfo(void *gDslVars, G992CodingParams *param, long code, long val, adslConnectionInfo *pConInfo)
{
    adslDataConnectionInfo *pInfo;

    pInfo = (kG992p2XmtCodingParamsInfo == code) ? &pConInfo->xmtInfo : &pConInfo->rcvInfo;
    pInfo->K = param->K;
    pInfo->S = param->S;
    pInfo->R = param->R;
    pInfo->D = param->D;

    pConInfo->trellisCoding = (0 == val) ? kAdslTrellisOff : kAdslTrellisOn;
    pConInfo->chType = AdslMibGetFastBytesInFrame(param) ? kAdslFastChannel : kAdslIntlChannel;
	if (kG992p2XmtCodingParamsInfo == code) 
      if (val != 0)
       pConInfo->trellisCoding2 |= kAdsl2TrellisTxEnabled;
     else
       pConInfo->trellisCoding2 &= ~kAdsl2TrellisTxEnabled;
	else
      if (val != 0)
       pConInfo->trellisCoding2 |= kAdsl2TrellisRxEnabled;
     else
       pConInfo->trellisCoding2 &= ~kAdsl2TrellisRxEnabled;

    /* Centillium NS framing for S = 1/2 */
    if (kG992p2RcvCodingParamsInfo == code) {
        if (((pInfo->K + pInfo->R) > 255) && (6 == globalVar.rsOption[0]))
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 6;
        else 
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
    }
}

Private void Adsl2MibSetConnectionInfo(void *gDslVars, G992p3CodingParams *param, long code, long val, adsl2ConnectionInfo *pConInfo)
{
    adsl2DataConnectionInfo *pInfo;

    pInfo = (kG992p3XmtCodingParamsInfo == code) ? &pConInfo->xmt2Info : &pConInfo->rcv2Info;

    pInfo->Nlp  = param->Nlp;
    pInfo->Nbc  = param->Nbc;
    pInfo->MSGlp= param->MSGlp;
    pInfo->MSGc = param->MSGc;

    pInfo->L = param->L;
    pInfo->M = param->M;
    pInfo->T = param->T;
    pInfo->D = param->D;
    pInfo->R = param->R;
    pInfo->B = param->B;
}

Private void Adsl2MibSetInfoFromGdmt1(void *gDslVars, adsl2DataConnectionInfo *pInfo2, adslDataConnectionInfo *pInfo)
{
    if (pInfo2->L != 0)
        return;

    pInfo2->Nlp = 1;
    pInfo2->Nbc = 1;
    pInfo2->MSGlp= 0;
    pInfo2->MSGc = 1;

    pInfo2->D = pInfo->D;
    pInfo2->R = pInfo->R;
    pInfo2->M = pInfo->S;
    pInfo2->B = pInfo->K;
    pInfo2->L = (pInfo2->B + 1)*8;
    pInfo2->T = 1;
}

Private void Adsl2MibSetInfoFromGdmt(void *gDslVars)
{
    Adsl2MibSetInfoFromGdmt1(gDslVars, &globalVar.adslMib.adsl2Info.rcv2Info, &globalVar.adslMib.adslConnection.rcvInfo);
    Adsl2MibSetInfoFromGdmt1(gDslVars, &globalVar.adslMib.adsl2Info.xmt2Info, &globalVar.adslMib.adslConnection.xmtInfo);
}

Private ulong AdslMibShowtimeSFErrors(ulong *curCnts, ulong *oldCnts)
{
    return (curCnts[kG992ShowtimeSuperFramesRcvdWrong] - oldCnts[kG992ShowtimeSuperFramesRcvdWrong]);
}

Private ulong AdslMibShowtimeRSErrors(ulong *curCnts, ulong *oldCnts)
{
    return (curCnts[kG992ShowtimeRSCodewordsRcvedCorrectable] - oldCnts[kG992ShowtimeRSCodewordsRcvedCorrectable]);
}

Private Boolean AdslMibShowtimeDataError(ulong *curCnts, ulong *oldCnts)
{
    return 
        (curCnts[kG992ShowtimeRSCodewordsRcvedUncorrectable] != 
         oldCnts[kG992ShowtimeRSCodewordsRcvedUncorrectable])   ||
         (AdslMibShowtimeSFErrors(curCnts, oldCnts) != 0);
}

#define AdslMibES(state,cnt)                                        \
do {                                                                \
    if (!state) {                                                   \
        state = true;                                               \
        IncPerfCounterVar(&globalVar.adslMib.adslPerfData, cnt);    \
    }                                                               \
} while (0)

Private void AdslMibConnectionStatUpdate (void *gDslVars, ulong *cntOld, ulong *cntNew)
{
    adslConnectionStat *s1 = &globalVar.adslMib.adslStat;
    adslConnectionStat *s2 = &globalVar.adslMib.adslStatSincePowerOn;
    atmConnectionStat  *a1 = &globalVar.adslMib.atmStat;
    atmConnectionStat  *a2 = &globalVar.adslMib.atmStatSincePowerOn;
    int n;

    n = cntNew[kG992ShowtimeSuperFramesRcvd] - cntOld[kG992ShowtimeSuperFramesRcvd];
    s1->rcvStat.cntSF += n;
    s2->rcvStat.cntSF += n;

    n = cntNew[kG992ShowtimeSuperFramesRcvdWrong] - cntOld[kG992ShowtimeSuperFramesRcvdWrong];
    s1->rcvStat.cntSFErr += n;
    s2->rcvStat.cntSFErr += n;

    n = cntNew[kG992ShowtimeRSCodewordsRcved] - cntOld[kG992ShowtimeRSCodewordsRcved];
    s1->rcvStat.cntRS += n;
    s2->rcvStat.cntRS += n;

    n = cntNew[kG992ShowtimeRSCodewordsRcvedCorrectable] - cntOld[kG992ShowtimeRSCodewordsRcvedCorrectable];
    s1->rcvStat.cntRSCor += n;
    s2->rcvStat.cntRSCor += n;

    n = cntNew[kG992ShowtimeRSCodewordsRcvedUncorrectable] - cntOld[kG992ShowtimeRSCodewordsRcvedUncorrectable];
    s1->rcvStat.cntRSUncor += n;
    s2->rcvStat.cntRSUncor += n;
	
	n = cntNew[kG992ShowtimeNumOfFEBE] - cntOld[kG992ShowtimeNumOfFEBE];
	s2->xmtStat.cntSFErr += n;
	if(txCntNotRcvYet==1)
		s1->xmtStat.cntSFErr =0;
	else
		s1->xmtStat.cntSFErr += n;
	n = cntNew[kG992ShowtimeNumOfFECC] - cntOld[kG992ShowtimeNumOfFECC];
	s2->xmtStat.cntRSCor += n;
	if(txCntNotRcvYet==1)
	{
		txCntNotRcvYet=0;
		s1->xmtStat.cntRSCor = 0;
	}
	else
		s1->xmtStat.cntRSCor += n;

	if(!AdslMibIsAdsl2Mod(gDslVars))
		if (globalVar.adslMib.adslConnection.xmtInfo.R != 0)
			s1->xmtStat.cntRS = (s1->xmtStat.cntSF * 68) / globalVar.adslMib.adslConnection.xmtInfo.S;
		else
			s1->xmtStat.cntRS = 0;
#ifdef DSL_REPORT_ALL_COUNTERS
    n = cntNew[kG992ShowtimeNumOfHEC] - cntOld[kG992ShowtimeNumOfHEC];
    a1->rcvStat.cntHEC += n;
    a2->rcvStat.cntHEC += n;

    n = cntNew[kG992ShowtimeNumOfOCD] - cntOld[kG992ShowtimeNumOfOCD];
    a1->rcvStat.cntOCD += n;
    a2->rcvStat.cntOCD += n;

    n = cntNew[kG992ShowtimeNumOfLCD] - cntOld[kG992ShowtimeNumOfLCD];
    a1->rcvStat.cntLCD += n;
    a2->rcvStat.cntLCD += n;

    if (n != 0)
        AdslMibES(globalVar.currSecondLCD, adslLCDS);

    n = cntNew[kG992ShowtimeNumOfFHEC] - cntOld[kG992ShowtimeNumOfFHEC];
    a1->xmtStat.cntHEC += n;
    a2->xmtStat.cntHEC += n;

    n = cntNew[kG992ShowtimeNumOfFOCD] - cntOld[kG992ShowtimeNumOfFOCD];
    a1->xmtStat.cntOCD += n;
    a2->xmtStat.cntOCD += n;

    n = cntNew[kG992ShowtimeNumOfFLCD] - cntOld[kG992ShowtimeNumOfFLCD];
    a1->xmtStat.cntLCD += n;
    a2->xmtStat.cntLCD += n;
#endif
}

Private void AdslMibUpdateShowtimeErrors(void *gDslVars, ulong nErr)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    if (nErr > 18)
        AdslMibES(globalVar.currSecondSES, adslSES);
}

Private void AdslMibUpdateLOS(void *gDslVars)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    AdslMibES(globalVar.currSecondLOS, adslLOSS);
    AdslMibES(globalVar.currSecondSES, adslSES);
}

Private void AdslMibUpdateLOF(void *gDslVars)
{
    AdslMibES(globalVar.currSecondErrored, adslESs);
    AdslMibES(globalVar.currSecondSES, adslSES);
}

Private void AdslMibUpdateShowtimeRSErrors(void *gDslVars, ulong nErr)
{
    AdslMibES(globalVar.currSecondFEC, adslFECs);
}

Private void AdslMibUpdateIntervalCounters (void *gDslVars, ulong sec)
{
    long    secElapsed, i, n;

    secElapsed  = sec + globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed;
    if (secElapsed >= k15MinInSeconds) {
        n = globalVar.adslMib.adslPerfData.adslPerfValidIntervals;
        if (n < kAdslMibPerfIntervals)
            n++;
        for (i = (n-1); i > 0; i--)
            AdslMibByteMove(
                sizeof(adslPerfCounters), 
                &globalVar.adslMib.adslPerfIntervals[i-1], 
                &globalVar.adslMib.adslPerfIntervals[i]);
        AdslMibByteMove(
            sizeof(adslPerfCounters), 
            &globalVar.adslMib.adslPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslPerfIntervals[0]);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&globalVar.adslMib.adslPerfData.perfCurr15Min);
        globalVar.adslMib.adslPerfData.adslPerfValidIntervals = n;

        n = globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals;
        if (n < kAdslMibChanPerfIntervals)
            n++;
        for (i = (n-1); i > 0; i--) {
            AdslMibByteMove(
                sizeof(adslChanCounters),
                &globalVar.adslMib.adslChanIntlPerfIntervals[i-1], 
                &globalVar.adslMib.adslChanIntlPerfIntervals[i]);
            AdslMibByteMove(
                sizeof(adslChanCounters),
                &globalVar.adslMib.adslChanFastPerfIntervals[i-1], 
                &globalVar.adslMib.adslChanFastPerfIntervals[i]);
        }
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanIntlPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslChanIntlPerfIntervals[0]);
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanFastPerfData.perfCurr15Min, 
            &globalVar.adslMib.adslChanFastPerfIntervals[0]);

        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanIntlPerfData.perfCurr15Min);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanFastPerfData.perfCurr15Min);
        globalVar.adslMib.adslChanIntlPerfData.adslChanPerfValidIntervals = n;
        globalVar.adslMib.adslChanFastPerfData.adslChanPerfValidIntervals = n;

        secElapsed -= k15MinInSeconds;
    }
    globalVar.adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanFastPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanIntlPerfData.adslPerfCurr15MinTimeElapsed = secElapsed;

    secElapsed = sec + globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed;
    if (secElapsed >= k1DayInSeconds) {
        AdslMibByteMove(
            sizeof(adslPerfCounters), 
            &globalVar.adslMib.adslPerfData.perfCurr1Day, 
            &globalVar.adslMib.adslPerfData.perfPrev1Day);
        BlockByteClear(sizeof(adslPerfCounters), (void*)&globalVar.adslMib.adslPerfData.perfCurr1Day);

        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanIntlPerfData.perfCurr1Day, 
            &globalVar.adslMib.adslChanIntlPerfData.perfPrev1Day);
        AdslMibByteMove(
            sizeof(adslChanCounters),
            &globalVar.adslMib.adslChanFastPerfData.perfCurr1Day,
            &globalVar.adslMib.adslChanFastPerfData.perfPrev1Day);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanIntlPerfData.perfCurr1Day);
        BlockByteClear(sizeof(adslChanCounters), (void*)&globalVar.adslMib.adslChanFastPerfData.perfCurr1Day);

        globalVar.adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs = k1DayInSeconds;
        secElapsed -= k1DayInSeconds;
    }
    globalVar.adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanFastPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
    globalVar.adslMib.adslChanIntlPerfData.adslPerfCurr1DayTimeElapsed = secElapsed;
	secElapsedInDay=secElapsed;
}


Public void AdslMibClearData(void *gDslVars)
{
    BlockByteClear (sizeof(adslMibVarsStruct), (void*)&globalVar);

    globalVar.notifyHandlerPtr = AdslMibNotifyIdle;

    globalVar.adslMib.adslLine.adslLineCoding = kAdslLineCodingDMT;
    globalVar.adslMib.adslLine.adslLineType  = kAdslLineTypeFastOrIntl;

    globalVar.adslMib.adslPhys.adslCurrOutputPwr = 130;

    globalVar.adslMib.adslChanIntlAtmPhyData.atmInterfaceOCDEvents = 0;
    globalVar.adslMib.adslChanIntlAtmPhyData.atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
    globalVar.adslMib.adslChanFastAtmPhyData.atmInterfaceOCDEvents = 0;
    globalVar.adslMib.adslChanFastAtmPhyData.atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
    globalVar.adslMib.adslFramingMode = 3;
    globalVar.showtimeMarginThld = -1;
}

Public void AdslMibResetConectionStatCounters(void *gDslVars)
{
    ulong n = globalVar.adslMib.adslStat.fireStat.status;
    BlockByteClear (sizeof(globalVar.adslMib.adslStat), (void *) &globalVar.adslMib.adslStat);
    globalVar.adslMib.adslStat.fireStat.status=n;
    BlockByteClear (sizeof(globalVar.adslMib.atmStat), (void *) &globalVar.adslMib.atmStat);
}

#define kAdslConnected  (kAdslXmtActive | kAdslRcvActive) 

Public Boolean AdslMibIsLinkActive(void *gDslVars)
{
    return (kAdslConnected == (globalVar.linkStatus & kAdslConnected));
}

Public int AdslMibTrainingState (void *gDslVars)
{
    return (globalVar.adslMib.adslTrainingState);
}

Public int AdslMibPowerState(void *gDslVars)
{
    return (globalVar.adslMib.adsl2Info.pwrState);
}

Public void AdslMibSetNotifyHandler(void *gDslVars, adslMibNotifyHandlerType notifyHandlerPtr)
{
    globalVar.notifyHandlerPtr = notifyHandlerPtr ? notifyHandlerPtr : AdslMibNotifyIdle;
}

Private void AdslMibNotify(void *gDslVars, int event)
{
    globalVar.notifyHandlerPtr (gDslVars, event);
}

Public Boolean  AdslMibInit(void *gDslVars, dslCommandHandlerType	commandHandler)
{
    AdslMibClearData(gDslVars);
    globalVar.nTones = kAdslMibToneNum;
	globalVar.cmdHandlerPtr		= commandHandler;
#ifdef G992_ANNEXC
    globalVar.nTones = kAdslMibToneNum*2;
#endif
    AdslMibSetNotifyHandler(gDslVars, NULL);
    return true;
}

Public void AdslMibTimer(void *gDslVars, long timeMs)
{
	long		sec;
	
	globalVar.txShowtimeTime+=timeMs;
	if(globalVar.txShowtimeTime>3000 && txUpdateStatFlag==1 )
	{
		if(secElapsedShTm==0) {
			adsl2DataConnectionInfo p= globalVar.adslMib.adsl2Info.xmt2Info;
			adslDataConnectionInfo p2=globalVar.adslMib.adslConnection.xmtInfo;
			per=(2*p.T*(p.M*(p.B+1)+p.R)*(p.MSGc+6))/(p.M*p.L);
			if(p.R!=0)
				rsRate=(p.L*4000)/8/(p.M*p2.K+p.R);
			else rsRate=0;
			txCntNotRcvYet=1;
		}

		sec = globalVar.txShowtimeTime/1000;
		if(sec > secElapsedShTm+1) {
			secElapsedShTm=sec;
			if(AdslMibIsAdsl2Mod(gDslVars)){
				globalVar.adslMib.adslStat.xmtStat.cntRS=globalVar.txShowtimeTime*rsRate/1000;
				globalVar.adslMib.adslStat.xmtStat.cntSF=globalVar.txShowtimeTime/(per);
			}
			else 
				globalVar.adslMib.adslStat.xmtStat.cntSF=globalVar.txShowtimeTime/17;
		}
	}

    timeMs += globalVar.timeMs;
    sec = timeMs / 1000;
    globalVar.timeSec += sec;
    globalVar.timeMs  = timeMs - sec * 1000;
    if (sec != 0) {
        if (AdslMibIsLinkActive(gDslVars)) {
            if (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS) {
                globalVar.currSecondErrored = true;
                globalVar.currSecondLOS     = true;
                globalVar.currSecondSES     = true;
                globalVar.currSecondFEC     = true;
                globalVar.currSecondLCD     = true;
            }
            else {
                globalVar.currSecondErrored = false;
                globalVar.currSecondLOS     = false;
                globalVar.currSecondSES     = false;
                globalVar.currSecondFEC     = false;
                globalVar.currSecondLCD     = false;
            }
            IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslAS);			
        }
        else {
            globalVar.currSecondErrored = false;
            globalVar.currSecondLOS     = false;
            globalVar.currSecondSES     = false;
            globalVar.currSecondFEC     = false;
            globalVar.currSecondLCD     = false;

            IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslUAS);
        }

        AdslMibUpdateIntervalCounters (gDslVars, sec);
    }
}

Private void AdslMibSetBitAndGain(void *gDslVars, int tblIdx, void *buf, int bufBgSize, Boolean bShared)
{
    int     i;
    uchar   *bufPtr = buf;

    if ((tblIdx + bufBgSize) > kAdslMibMaxToneNum)
        bufBgSize = kAdslMibMaxToneNum - tblIdx;

    for (i = 0; i < bufBgSize; i++) {
        if (!bShared || (0 == (globalVar.bitAlloc[tblIdx + i] | globalVar.gain[tblIdx + i]))) {
            globalVar.bitAlloc[tblIdx + i] = bufPtr[i << 1] & 0xF;
            globalVar.gain[tblIdx + i] = (bufPtr[i << 1] >> 4) | ((ulong)bufPtr[(i << 1) + 1] << 4);
        }
    }
}

Private void AdslMibSetGain(void *gDslVars, int tblIdx, void *buf, int bufBgSize, Boolean bShared)
{
    int     i;
    short   *bufPtr = buf;
    if ((tblIdx + bufBgSize) > kAdslMibMaxToneNum)
        bufBgSize = kAdslMibMaxToneNum - tblIdx;
    for (i = 0; i < bufBgSize; i++) {
        if (!bShared || (0 ==  globalVar.gain[tblIdx + i])) 
            globalVar.gain[tblIdx + i] = bufPtr[i]>>3;
    }
}

#define AdslMibParseBitGain(p,inx,b,g)  do {        \
    inx = p->ind;                                   \
    b   = p->gb & 0xF;                              \
    g   = ((long) p->gain << 4) | (p->gb >> 4);     \
} while (0)

Private void *AdslMibUpdateBitGain(void *gDslVars, void *pOlrEntry, Boolean bAdsl2)
{
    int     index, gain;
    uchar   bits;

    if (bAdsl2) {
        dslOLRCarrParam2p   *p = pOlrEntry;
        AdslMibParseBitGain(p, index, bits, gain);
        pOlrEntry = p + 1;
    }
    else {
        dslOLRCarrParam     *p = pOlrEntry;
        AdslMibParseBitGain(p, index, bits, gain);
        pOlrEntry = p + 1;
    }

    globalVar.bitAlloc[index] = bits;
    globalVar.gain[index]     = gain;

    return pOlrEntry;
}


Private void AdslMibSetBitAllocation(void *gDslVars, int tblIdx, void *buf, int bufSize, Boolean bShared)
{
    int     i;
    uchar   *bufPtr = buf;


    if ((tblIdx + bufSize) > kAdslMibMaxToneNum)
        bufSize = kAdslMibMaxToneNum - tblIdx;

    for (i = 0; i < bufSize; i++) {
        if (!bShared || (0 == (globalVar.bitAlloc[tblIdx + i])))
            globalVar.bitAlloc[tblIdx + i] = bufPtr[i];
    }
}

Public int AdslMibGetModulationType(void *gDslVars)
{
    return globalVar.adslMib.adslConnection.modType & kAdslModMask;
}

Private void AdslMibSetModulationType(void *gDslVars, int mod)
{
    globalVar.adslMib.adslConnection.modType = 
        (globalVar.adslMib.adslConnection.modType & ~kAdslModMask) | mod;
}

Private int AdslMibGetAdsl2Mode(void *gDslVars, ulong mask)
{
    return globalVar.adslMib.adsl2Info.adsl2Mode & mask;
}

Private void AdslMibSetAdsl2Mode(void *gDslVars, ulong mask, int mod)
{
    globalVar.adslMib.adsl2Info.adsl2Mode = 
        (globalVar.adslMib.adsl2Info.adsl2Mode & ~mask) | mod;
}

#define AdslMibGetAnnexMType(gv)        AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexMask)
#define AdslMibSetAnnexMType(gv,mod)    AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexMask,mod)

#define AdslMibGetAnnexLUpType(gv)      AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexLUpMask)
#define AdslMibSetAnnexLUpType(gv,mod)  AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexLUpMask,mod)
#define AdslMibGetAnnexLDnType(gv)      AdslMibGetAdsl2Mode(gv,kAdsl2ModeAnnexLDnMask)
#define AdslMibSetAnnexLDnType(gv,mod)  AdslMibSetAdsl2Mode(gv,kAdsl2ModeAnnexLDnMask,mod)

Private Boolean AdslMibTone32_64(void *gDslVars)
{
#ifdef G992P1_ANNEX_B
    if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB))
        return true;
#endif
    if (AdslMibGetAnnexMType(gDslVars) != 0)
        return true;

    return false;
}

Private int AdslMibGetBitmapMode(void *gDslVars)
{
    return globalVar.adslMib.adslConnection.modType & kAdslBitmapMask;
}

Private void AdslMibSetBitmapMode(void *gDslVars, int mod)
{
    globalVar.adslMib.adslConnection.modType = 
        (globalVar.adslMib.adslConnection.modType & ~kAdslBitmapMask) | mod;
}

Private int AdslMibGetUpstreamMode(void *gDslVars)
{
    return globalVar.adslMib.adslConnection.modType & kAdslUpstreamModeMask;
}

Private void AdslMibSetUpstreamMode(void *gDslVars, int mod)
{
    globalVar.adslMib.adslConnection.modType = 
        (globalVar.adslMib.adslConnection.modType & ~kAdslUpstreamModeMask) | mod;
}

Public void	AdslMibUpdateTxStat(
	void					*gDslVars, 
	adslConnectionDataStat  *adslTxData, 
	adslPerfCounters		*adslTxPerf, 
	atmConnectionDataStat	*atmTxData)
{
	ulong				adslCnt[kG992ShowtimeNumOfMonitorCounters];
	adslMibInfo			*pMib = &globalVar.adslMib;

	AdslMibByteMove(sizeof(globalVar.shtCounters), globalVar.shtCounters, adslCnt);
	adslCnt[kG992ShowtimeNumOfFEBE] = adslTxData->cntSFErr;
	adslCnt[kG992ShowtimeNumOfFECC] = adslTxData->cntRSCor;
	adslCnt[kG992ShowtimeNumOfFHEC] = atmTxData->cntHEC;
	AdslMibConnectionStatUpdate (gDslVars, globalVar.shtCounters, adslCnt);
	AdslMibByteMove(sizeof(globalVar.shtCounters), adslCnt, globalVar.shtCounters);

	pMib->adslTxPerfTotal.adslFECs  = adslTxPerf->adslFECs;
	pMib->adslTxPerfTotal.adslESs   = adslTxPerf->adslESs;
	pMib->adslTxPerfTotal.adslSES   = adslTxPerf->adslSES;
	pMib->adslTxPerfTotal.adslLOSS  = adslTxPerf->adslLOSS;
	pMib->adslTxPerfTotal.adslUAS   = adslTxPerf->adslUAS;

	pMib->atmStat.xmtStat.cntCellTotal = atmTxData->cntCellTotal;
	pMib->atmStat.xmtStat.cntCellData  = atmTxData->cntCellData;
	pMib->atmStat.xmtStat.cntBitErrs   = atmTxData->cntBitErrs; 
}

Public void AdslMibStatusSnooper (void *gDslVars, dslStatusStruct *status)
{
    atmPhyDataEntrty        *pAtmData;
    adslChanPerfDataEntry   *pChanPerfData;
    long                    val, n;

    switch (status->code) {
		case kDslDspControlStatus:
			switch(status->param.dslConnectInfo.code)
			{
				case kFireMonitoringCounters:
				{
					unsigned long *pLong = (unsigned long *)status->param.dslConnectInfo.buffPtr;
					adslFireStat *pFireStat = &globalVar.adslMib.adslStat.fireStat;
					pFireStat->reXmtRSCodewordsRcved = pLong[kFireReXmtRSCodewordsRcved];
					pFireStat->reXmtUncorrectedRSCodewords = pLong[kFireReXmtUncorrectedRSCodewords];
					pFireStat->reXmtCorrectedRSCodewords = pLong[kFireReXmtCorrectedRSCodewords];					
					break;
				}
				case kDslG992RcvShowtimeUpdateGainPtr:
                    {
					val = status->param.dslConnectInfo.value;
                    uchar   *buffPtr = status->param.dslConnectInfo.buffPtr;
                    if (AdslMibTone32_64(gDslVars)) {
                        AdslMibSetGain(gDslVars, 32, buffPtr + 32, 32, true);
                        AdslMibSetGain(gDslVars, 64, buffPtr + 64, val - 32, false);
                    }
                    else
                        AdslMibSetGain(gDslVars, 32, buffPtr+32, val - 32, false);

#ifdef G992_ANNEXC
                    if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                        AdslMibSetGain(gDslVars, kAdslMibToneNum, buffPtr+kAdslMibToneNum, 256, false);
                        AdslMibSetGain(gDslVars, 2*kAdslMibToneNum+32, buffPtr+2*kAdslMibToneNum+32, 224+256, false);
                    }
                    else
                        AdslMibSetGain(gDslVars, kAdslMibToneNum+32, buffPtr+kAdslMibToneNum+32, 224, false);
#endif
                    }
                    break;
				case kDslNLNoise:
					n = (status->param.dslConnectInfo.value <kAdslMibMaxToneNum ? status->param.dslConnectInfo.value : kAdslMibMaxToneNum);
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.distNoisedB);
					break;
				#ifdef ADSL_MIBOBJ_PLN
				case kDslPLNPeakNoiseTablePtr:
					n=status->param.dslConnectInfo.value;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNValueps);
					break;
				case kDslPerBinThldViolationTablePtr:
					n=status->param.dslConnectInfo.value;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNThldCntps);
					break;
				case kDslImpulseNoiseDurationTablePtr:
					n=status->param.dslConnectInfo.value;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNDurationHist);
					break;
				case kDslImpulseNoiseTimeTablePtr:
					n=status->param.dslConnectInfo.value;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNIntrArvlHist);
					if(globalVar.adslMib.adslPLNData.PLNUpdateData==0)
						globalVar.adslMib.adslPLNData.PLNUpdateData=1;
					break;
				case kDslPLNMarginPerBin:
					globalVar.adslMib.adslPLNData.PLNThldPerTone=status->param.dslConnectInfo.value;
					break;
				case kDslPLNMarginBroadband:
					globalVar.adslMib.adslPLNData.PLNThldBB=status->param.dslConnectInfo.value;
					break;
				case kDslPerBinMsrCounter:
					globalVar.adslMib.adslPLNData.PLNPerToneCounter=status->param.dslConnectInfo.value;
					break;
				case kDslBroadbandMsrCounter:
					globalVar.adslMib.adslPLNData.PLNBBCounter=status->param.dslConnectInfo.value;
					break;
				case kDslInpBinTablePtr:
					n=status->param.dslConnectInfo.value;
					globalVar.adslMib.adslPLNData.PLNNbDurBins=n/2;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNDurationBins);
					break;
				case kDslItaBinTablePtr:
					n=status->param.dslConnectInfo.value;
					globalVar.adslMib.adslPLNData.PLNNbIntArrBins=n/2;
					AdslMibByteMove(n,status->param.dslConnectInfo.buffPtr,globalVar.PLNIntrArvlBins);
					break;
				case kDslPlnState:
					globalVar.adslMib.adslPLNData.PLNState=status->param.dslConnectInfo.value;
					break;
				#endif
			}
			break;
					

        case kDslTrainingStatus:
            val = status->param.dslTrainingInfo.value;

            switch (status->param.dslTrainingInfo.code) {
		case kG992RcvDelay:
			globalVar.adslMib.adsl2Info.rcv2DelayInp.delay = (unsigned short)val;
			break;
		case kG992RcvInp:
			globalVar.adslMib.adsl2Info.rcv2DelayInp.inp = (unsigned short)val;
			break;
                case kG992FireState:
                    globalVar.adslMib.adslStat.fireStat.status = val;
                    break;
                case kDslStartedG994p1:
                case kG994p1EventToneDetected:
                case kDslStartedT1p413HS:
                case kDslT1p413ReturntoStartup:
                    globalVar.adslMib.adslStat.fireStat.status=0;
                    globalVar.vendorIdReceived = false;
                    globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
                    globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
                    globalVar.adslMib.adsl2Info.pwrState = 0;
                    globalVar.adslMib.adsl2Info.adsl2Mode = 0;
                    globalVar.rsOptionValid = false;
                    globalVar.rsOption[0] = 0;
                    globalVar.nMsgCnt = 0;
                    n = globalVar.linkStatus;
                    globalVar.linkStatus = 0;
                    if (n != 0)
                        AdslMibNotify(gDslVars, kAdslEventLinkChange);
                    break;
                case kDslG992p2RcvVerifiedBitAndGain:
                    val = Q4ToTenth(val);
                    globalVar.adslMib.adslPhys.adslCurrSnrMgn = RestrictValue(val, -640, 640);
                    break;
                case kDslG992p2TxShowtimeActive:
                    globalVar.txShowtimeTime=0;
					secElapsedShTm=0;
					txUpdateStatFlag=1;
					globalVar.adslMib.adsl2Info.xmtRate = val;
                    if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
                        globalVar.adslMib.adsl2Info.xmtRate = NitroRate(val);
                    globalVar.linkStatus |= kAdslXmtActive;
                    if (AdslMibIsLinkActive(gDslVars)) {
                        globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                        globalVar.adslMib.adslAtucPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                        AdslMibNotify(gDslVars, kAdslEventLinkChange);
                        Adsl2MibSetInfoFromGdmt(gDslVars);
                    }
                    BlockByteClear(sizeof(globalVar.shtCounters), (void*)&globalVar.shtCounters);
#ifndef ADSL_DRV_NO_STAT_RESET
			n = globalVar.adslMib.adslStat.fireStat.status;
                    BlockByteClear(sizeof(globalVar.adslMib.adslStat), (void*)&globalVar.adslMib.adslStat);
			globalVar.adslMib.adslStat.fireStat.status = n;
                    BlockByteClear(sizeof(globalVar.adslMib.atmStat), (void*)&globalVar.adslMib.atmStat);

                    BlockByteClear(
                        sizeof(globalVar.adslMib.adslPerfData.perfSinceShowTime), 
                        (void*)&globalVar.adslMib.adslPerfData.perfSinceShowTime
                        );

                    BlockByteClear(
                        sizeof(globalVar.adslMib.adslPerfData.failSinceShowTime), 
                        (void*)&globalVar.adslMib.adslPerfData.failSinceShowTime
                        );
#endif
                    globalVar.adslMib.adslTrainingState = kAdslTrainingConnected;
                    globalVar.timeConStarted = 0;		
                    break;
                case kDslG992p2RxShowtimeActive:
                    globalVar.adslMib.adsl2Info.rcvRate = val;
                    if (globalVar.adslMib.adslFramingMode & kAtmHeaderCompression)
                        globalVar.adslMib.adsl2Info.rcvRate = NitroRate(val);
                    globalVar.linkStatus |= kAdslRcvActive;
                    if (AdslMibIsLinkActive(gDslVars)) {
                        globalVar.adslMib.adslPhys.adslCurrStatus = kAdslPhysStatusNoDefect;
                        AdslMibNotify(gDslVars, kAdslEventLinkChange);
                        Adsl2MibSetInfoFromGdmt(gDslVars);
                    }
                    break;
				case kDslRetrainReason:
					txUpdateStatFlag=0;
					if(globalVar.adslMib.adslTrainingState > kAdslTrainingG994)
						globalVar.adslMib.adslPerfData.lastShowtimeDropReason=val;
					if(!((val>>kRetrainReasonDslStartPhysicalLayerCmd)&0x1))
						globalVar.adslMib.adslPerfData.lastRetrainReason=val;
					break;
                case kDslFinishedG994p1:
                    memset(
                        globalVar.adslMib.adslPhys.adslVendorID, 
                        0x00, 
                        sizeof(globalVar.adslMib.adslPhys.adslVendorID)
                        );
                    globalVar.adslMib.adslPhys.adslVendorID[0] = 0xb5;
                    globalVar.adslMib.adslPhys.adslVendorID[1] = 0x00;
                    globalVar.adslMib.adslPhys.adslVendorID[2] = 'B';
                    globalVar.adslMib.adslPhys.adslVendorID[3] = 'D';
                    globalVar.adslMib.adslPhys.adslVendorID[4] = 'C';
                    globalVar.adslMib.adslPhys.adslVendorID[5] = 'M';
                    globalVar.adslMib.adslPhys.adslVendorID[6] = 0x00;
                    globalVar.adslMib.adslPhys.adslVendorID[7] = 0x00;

                    switch (val) {
                        case kG992p2AnnexAB:
                        case kG992p2AnnexC:
                            AdslMibSetModulationType(gDslVars, kAdslModGlite);
                            break;
#ifdef G992P1_ANNEX_I
                        case (kG992p1AnnexI>>4):
                            AdslMibSetModulationType(gDslVars, kAdslModAnnexI);
                            break;
#endif
#ifdef G992P3
                        case kG992p3AnnexA:
                        case kG992p3AnnexB:
                            AdslMibSetModulationType(gDslVars, kAdslModAdsl2);
                            break;
#endif
#ifdef G992P5
                        case kG992p5AnnexA:
                        case kG992p5AnnexB:
                            AdslMibSetModulationType(gDslVars, kAdslModAdsl2p);
                            break;
#endif
                        case kG992p1AnnexA:
                        case kG992p1AnnexB:
                        case kG992p1AnnexC:
                        default:
                            AdslMibSetModulationType(gDslVars, kAdslModGdmt);
                            break;
                    }
                    globalVar.nTones = kAdslMibToneNum;
#ifdef G992P5
                    if (kAdslModAdsl2p == AdslMibGetModulationType(gDslVars))
                        globalVar.nTones = kAdslMibToneNum * 2;
#endif
#ifdef G992_ANNEXC
                    globalVar.nTones = kAdslMibToneNum * 
                        ((kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) ? 4 : 2);
#endif
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG992Started;
                    break;
                case kDslG992p3AnnexLMode:
                    if ((val != 0) && (kAdslModAdsl2 == AdslMibGetModulationType(gDslVars))) {
                        AdslMibSetModulationType(gDslVars, kAdslModReAdsl2);
                        if (kG994p1G992p3AnnexLUpNarrowband == (val & 0xFF))
                            AdslMibSetAnnexLUpType(gDslVars, kAdsl2ModeAnnexLUpNarrow);
                        else
                            AdslMibSetAnnexLUpType(gDslVars, kAdsl2ModeAnnexLUpWide);
                        if (kG994p1G992p3AnnexLDownNonoverlap == ((val >> 8) & 0xFF))
                            AdslMibSetAnnexLDnType(gDslVars, kAdsl2ModeAnnexLDnNonOvlap);
                        else
                            AdslMibSetAnnexLDnType(gDslVars, kAdsl2ModeAnnexLDnOvlap);
                    }
                    break;
                case kG992EnableAnnexM:
                    AdslMibSetAnnexMType(gDslVars, val+1);
                    break;
                case kDslFinishedT1p413:
                    AdslMibSetModulationType(gDslVars, kAdslModT1413);
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG992Started;

                    memset(
                        globalVar.adslMib.adslPhys.adslVendorID, 
                        0x00, 
                        sizeof(globalVar.adslMib.adslPhys.adslVendorID)
                        );

                    globalVar.adslMib.adslPhys.adslVendorID[0] = 0x54;
                    globalVar.adslMib.adslPhys.adslVendorID[1] = 0x4d;
                    break;
                case kG992DataRcvDetectLOS:
                    globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOS;
                    globalVar.adslMib.adslPhys.adslCurrStatus &= ~(kAdslPhysStatusLOF | kAdslPhysStatusLOM);
                    IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLoss);
                    AdslMibUpdateLOS(gDslVars);
                    break;
                case kG992DataRcvDetectLOSRecovery:
                    globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOS;
                    break;
                case kG992DecoderDetectRemoteLOS:
                    globalVar.adslMib.adslAtucPhys.adslCurrStatus |= kAdslPhysStatusLOS;
                    break;
                case kG992DecoderDetectRemoteLOSRecovery:
                    globalVar.adslMib.adslAtucPhys.adslCurrStatus &= ~kAdslPhysStatusLOS;
                    break;
                case kG992DecoderDetectRDI:
                    if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS))
                        globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOF;
                    IncPerfCounterVar(&globalVar.adslMib.adslPerfData, adslLofs);
                    AdslMibUpdateLOF(gDslVars);
                    break;
                case kG992DecoderDetectRDIRecovery:
                    globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOF;
                    break;
                case kG992LDCompleted:
                    globalVar.adslMib.adslPhys.adslLDCompleted = (val != 0)? 1: -1;
                    break;
                case kDslG994p1StartupFinished:
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG994;
                    break;
                case kDslG992p2Phase3Started:
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG992ChanAnalysis;
                    break;
                case kDslG992p2Phase4Started:
                    globalVar.adslMib.adslTrainingState = kAdslTrainingG992Exchange;
                    break;
                case kDslG992p2ReceivedBitGainTable:
                    globalVar.nMsgCnt = 0;
                    /* fall through */
                case kDslG992p2ReceivedMsgLD:
                case kDslG992p2ReceivedRates1:
                case kDslG992p2ReceivedMsg1:
                case kDslG992p2ReceivedRatesRA:
                case kDslG992p2ReceivedMsgRA:
                case kDslG992p2ReceivedRates2:
                case kDslG992p2ReceivedMsg2:
                    globalVar.g992MsgType = status->param.dslTrainingInfo.code;
                    break;

                case kDslG992Timeout:
                    globalVar.adslMib.adslPerfData.failTotal.adslInitTo++;                    
                    globalVar.adslMib.adslPerfData.failSinceShowTime.adslInitTo++;
                    break;
                default:
                    break;
            }
            break;

        case kDslConnectInfoStatus:
            val = status->param.dslConnectInfo.value;

            switch (status->param.dslConnectInfo.code) {
#ifdef G994P1
                case    kG994MessageExchangeRcvInfo:
                    {
                    uchar   *msgPtr = ((unsigned char*)status->param.dslConnectInfo.buffPtr);
					int v;
                    if ((msgPtr != NULL) && ((msgPtr[0] == 2) || (msgPtr[0] == 3)) &&   /* CL or CLR message */
                        (val >= (2 + kAdslPhysVendorIdLen)) && !globalVar.vendorIdReceived){
						globalVar.vendorIdReceived=true;	
						AdslMibByteMove(kAdslPhysVendorIdLen, msgPtr+2, globalVar.adslMib.adslAtucPhys.adslVendorID);
						v=(globalVar.adslMib.adslAtucPhys.adslVendorID[6]<<8)+globalVar.adslMib.adslAtucPhys.adslVendorID[7];
						sprintf(globalVar.adslMib.adslAtucPhys.adslVersionNumber,"0x%4x",v);
					}
                    }
                    break;
#endif
                case kG992p2XmtCodingParamsInfo:
                    {
                    G992CodingParams *codingParam = (G992CodingParams*) status->param.dslConnectInfo.buffPtr;

                    codingParam->AS0BF = codingParam->AS1BF = codingParam->AS2BF = codingParam->AS3BF = codingParam->AEXAF = 0;
                    codingParam->AS0BI = codingParam->AS1BI = codingParam->AS2BI = codingParam->AS3BI = codingParam->AEXAI = 0;

                    AdslMibByteMove(sizeof(G992CodingParams), codingParam, &globalVar.xmtParams);
                    AdslMibSetChanEntry(codingParam, &globalVar.adslMib.adslChanFast, &globalVar.adslMib.adslChanIntl);
                    AdslMibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.adslConnection);
                    }
                    break;
                case kG992p2RcvCodingParamsInfo:
                    {
                    G992CodingParams *codingParam = (G992CodingParams*) status->param.dslConnectInfo.buffPtr;

                    AdslMibByteMove(sizeof(G992CodingParams), codingParam, &globalVar.rcvParams);
                    AdslMibSetChanEntry(codingParam, &globalVar.adslMib.adslChanFast, &globalVar.adslMib.adslChanIntl);
                    AdslMibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.adslConnection);
                    }
                    break;

                case kG992p3XmtCodingParamsInfo:
                case kG992p3RcvCodingParamsInfo:
                    {
                    G992p3CodingParams *codingParam = (G992p3CodingParams *) status->param.dslConnectInfo.buffPtr;

                    Adsl2MibSetConnectionInfo(gDslVars, codingParam, status->param.dslConnectInfo.code, val, &globalVar.adslMib.adsl2Info);
                    }
                    break;

                case kG992p3PwrStateInfo:
                    globalVar.adslMib.adsl2Info.pwrState = val;
                    if( 2 == val )
                        AdslCoreIndicateLinkPowerStateL2();
#ifdef DEBUG_L2_RET_L0
                    printk("%s: L%d\n", __FUNCTION__, val);
#endif
                    break;

                case kDslATUAvgLoopAttenuationInfo:
                    val = Q4ToTenth(val);
                    globalVar.adslMib.adslPhys.adslCurrAtn = RestrictValue(val, 0, 1630);
                    globalVar.adslMib.adslPhys.adslSignalAttn = globalVar.adslMib.adslPhys.adslCurrAtn;
                    break;

                case kDslSignalAttenuation:
                    val = Q4ToTenth(val);
                    globalVar.adslMib.adslDiag.signalAttn = val;
                    globalVar.adslMib.adslPhys.adslSignalAttn = val;
                    break;

                case kDslAttainableNetDataRate:
                    globalVar.adslMib.adslDiag.attnDataRate = val;
                    globalVar.adslMib.adslPhys.adslCurrAttainableRate = ActualRate(val);
                    break;

                case kDslHLinScale:
                    globalVar.adslMib.adslDiag.hlinScaleFactor = val;
                    globalVar.adslMib.adslPhys.adslHlinScaleFactor = val;
                    break;

                case kDslATURcvPowerInfo:
                    globalVar.rcvPower = val;
                    break;

                case kDslMaxReceivableBitRateInfo:
                    globalVar.adslMib.adslPhys.adslCurrAttainableRate = ActualRate(val * 1000);
                    break;

                case kDslRcvCarrierSNRInfo:
                    n = kAdslMibToneNum;
#ifdef G992_ANNEXC
                    if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars))
                        n = kAdslMibToneNum << 1;
#endif
#ifdef G992P5
                    if (kAdslModAdsl2p == AdslMibGetModulationType(gDslVars))
                        n = kAdslMibToneNum << 1;
#endif
                  /*  if (val > n)
                        val = n;*/

                    AdslMibByteMove(
                        val * sizeof(globalVar.snr[0]),
                        status->param.dslConnectInfo.buffPtr, 
                        globalVar.snr + globalVar.nMsgCnt * n);
#ifdef G992_ANNEXC
                    globalVar.nMsgCnt ^= 1;
#endif
                    break;

                case kG992p2XmtToneOrderingInfo:
                    {
                    uchar   *buffPtr = status->param.dslConnectInfo.buffPtr;

                    if (AdslMibTone32_64(gDslVars)) {
                        AdslMibSetBitAllocation(gDslVars, 0, buffPtr + 0, 32, false);
                        AdslMibSetBitAllocation(gDslVars, 32, buffPtr + 32, 32, true);
                    }
                    else
                        AdslMibSetBitAllocation(gDslVars, 0, buffPtr, val, false);

#ifdef G992_ANNEXC
                    n = kAdslMibToneNum;
                    if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars))
                        n <<= 1;
                    AdslMibSetBitAllocation(gDslVars, n, buffPtr + 32, 32, false);
#endif
                    }
                    break;
				
                case kG992p2RcvToneOrderingInfo:
                    {
                    uchar   *buffPtr = status->param.dslConnectInfo.buffPtr;

                    if (AdslMibTone32_64(gDslVars)) {
                        AdslMibSetBitAllocation(gDslVars, 32, buffPtr + 32, 32, true);
                        AdslMibSetBitAllocation(gDslVars, 64, buffPtr + 64, val - 32, false);
                    }
                    else
                        AdslMibSetBitAllocation(gDslVars, 32, buffPtr+32, val - 32, false);

#ifdef G992_ANNEXC
                    if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                        AdslMibSetBitAllocation(gDslVars, kAdslMibToneNum, buffPtr+kAdslMibToneNum, 256, false);
                        AdslMibSetBitAllocation(gDslVars, 2*kAdslMibToneNum+32, buffPtr+2*kAdslMibToneNum+32, 224+256, false);
                    }
                    else
                        AdslMibSetBitAllocation(gDslVars, kAdslMibToneNum+32, buffPtr+kAdslMibToneNum+32, 224, false);
#endif
                    }
                    break;
				
                case kG992MessageExchangeRcvInfo:
                    if (kDslG992p2ReceivedMsg2 == globalVar.g992MsgType) {
                        uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;
                        int     n;

                        n = (*msgPtr) | ((*(msgPtr+1) & 1) << 8);
                        globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = ActualRate(n * 4000);
                        globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = (*(msgPtr+2) & 0x1F) * 10;
                        globalVar.adslMib.adslAtucPhys.adslCurrAtn = (*(msgPtr+3) >> 2) * 5;
                    }
                    else if (kDslG992p2ReceivedBitGainTable == globalVar.g992MsgType) {
                        short   *buffPtr = status->param.dslConnectInfo.buffPtr;
                        int     n;

                        globalVar.bitAlloc[0] = 0;
                        globalVar.gain[0] = 0;
                        val = status->param.dslConnectInfo.value >> 1;
                        n   = 0;
#ifdef G992P3
                        if (AdslMibIsAdsl2Mod(gDslVars)) {
                            uchar   *p = (uchar *) buffPtr;
                            int     rate;

                            val -= 7;
                            n    = 7;
                            globalVar.adslMib.adslAtucPhys.adslCurrAtn = (((ulong) p[1] & 0x3) << 8) | p[0];
                            globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = ((long) p[5] << 8) | p[4];
                            rate = ((ulong) p[9] << 24) | ((ulong) p[8] << 16) | ((ulong) p[7] << 8) | p[6];
                            globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = ActualRate(rate);

                            rate = ((long) p[11] << 8) | p[10];
                            globalVar.adslMib.adslPhys.adslCurrOutputPwr = (rate << (32-10)) >> (32-10);
                        }
#endif

#ifdef G992_ANNEXC
                        if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                            AdslMibSetBitAndGain(gDslVars, 1+globalVar.nMsgCnt*2*kAdslMibToneNum, buffPtr + 0, 31, false);
                        }
                        else {
                            AdslMibSetBitAndGain(gDslVars, 1, buffPtr + 0, 31, false);
                            globalVar.bitAlloc[kAdslMibToneNum] = 0;
                            globalVar.gain[kAdslMibToneNum] = 0;
                            AdslMibSetBitAndGain(gDslVars, kAdslMibToneNum + 1, buffPtr + 31, 31, false);
                        }
#else
                        AdslMibSetBitAndGain(gDslVars, 1, buffPtr + n, val, false);
#endif
                    }
                    else if (kDslG992p2ReceivedRatesRA == globalVar.g992MsgType) {
                        uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;
                        
                        for (n = 0; n < 4; n++)
                            globalVar.rsOption[1+n] = msgPtr[n*30 + 21] & 0x3F;
                        globalVar.rsOptionValid = true;
                    }
                    else if (kDslG992p2ReceivedMsgLD == globalVar.g992MsgType) {
                        uchar   *p = (uchar *)status->param.dslConnectInfo.buffPtr;
                        char    *ps = (char *) p;
                        ulong   msgLen;
                        int     i, j;

                        msgLen = status->param.dslConnectInfo.value;
                        switch (*p) {
                          case 0x11:
                            {
                            ulong   n;

                            globalVar.adslMib.adslAtucPhys.adslHlinScaleFactor = ((int) p[3] << 8) + p[2];
                            globalVar.adslMib.adslAtucPhys.adslCurrAtn = ((int) p[5] << 8) + p[4];
                            globalVar.adslMib.adslAtucPhys.adslSignalAttn = ((int) p[7] << 8) + p[6];
                            globalVar.adslMib.adslAtucPhys.adslCurrSnrMgn = ((int) ps[9] << 8) + p[8];
                            globalVar.adslMib.adslPhys.adslCurrOutputPwr = ((int) ps[15] << 8) + p[14];
                            n = ((ulong) p[13] << 24) | ((ulong) p[12] << 16) | ((ulong) p[11] << 8) | p[10];
                            globalVar.adslMib.adslAtucPhys.adslCurrAttainableRate = n;
                            }
                            break;

                          case 0x22:
                            j = 0;
                            for (i = 2; i < msgLen; i += 4) {
                                globalVar.chanCharLin[j].x = ((int) ps[i+1] << 8) + p[i+0];
                                globalVar.chanCharLin[j].y = ((int) ps[i+3] << 8) + p[i+2];
                                j++;
                            }
                            break;
                          case 0x33:
                            j = 0;
                            for (i = 2; i < msgLen; i += 2) {
                                int     n;

                                n = ((int) (p[i+1] & 0x3) << 12) + (p[i] << 4);
                                n = -(n/10) + 6*16;
                                globalVar.chanCharLog[j] = n;
                                j++;
                            }
                            break;
                          case 0x44:
                            for (i = 2; i < msgLen; i++)
                                globalVar.quietLineNoise[i-2] = -(((short) p[i]) << 3) - 23*16;
                            break;
                          case 0x55:
                            for (i = 2; i < msgLen; i++)
                                globalVar.snr[i-2] = (((short) p[i]) << 3) - 32*16;
                            break;

                        }
                    }
                    break;

                case kG992MessageExchangeXmtInfo:
                    if ((1 == status->param.dslConnectInfo.value) && globalVar.rsOptionValid) {
                        static  uchar optNum[16] = { 0, 1, 2, 0, 3, 0,0,0, 4,  0,0,0,0,0,0,0 };
                        uchar   *msgPtr = (uchar *)status->param.dslConnectInfo.buffPtr;

                        globalVar.rsOption[0] = globalVar.rsOption[optNum[*msgPtr & 0xF]];
                        break;
                    }

                    if (kDslG992p2ReceivedBitGainTable == globalVar.g992MsgType) {
                        short * buffPtr = status->param.dslConnectInfo.buffPtr;
                        int     n;

                        val = status->param.dslConnectInfo.value >> 1;
                        n   = 0;
#ifdef G992P3
                        if (AdslMibIsAdsl2Mod(gDslVars)) {
                            val -= 7;
                            n    = 7;
                        }
#endif

#ifdef G992_ANNEXC
                        if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars)) {
                            AdslMibSetBitAndGain(gDslVars, 32+globalVar.nMsgCnt*2*kAdslMibToneNum, buffPtr + 31, 224+256, false);
                        }
                        else {
                            AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31, 224, false);
                            AdslMibSetBitAndGain(gDslVars, kAdslMibToneNum + 32, buffPtr + 255 + 31, 224, false);
                        }
#else
                        if (AdslMibTone32_64(gDslVars)) {
                            AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31 + n, 32, true);
                            AdslMibSetBitAndGain(gDslVars, 64, buffPtr + 63 + n, val - 32, false);
                        }
                        else
#endif
                            AdslMibSetBitAndGain(gDslVars, 32, buffPtr + 31 + n, val > 300 ? 480 : 224, false);
                    }
                    break;

                case    kG992ShowtimeMonitoringStatus:
                    {
                    ulong   *counters = (ulong*) (status->param.dslConnectInfo.buffPtr);

                    pChanPerfData = (kAdslIntlChannel == AdslMibGetActiveChannel(gDslVars) ?
                        &globalVar.adslMib.adslChanIntlPerfData : &globalVar.adslMib.adslChanFastPerfData);

                    if (AdslMibShowtimeDataError(counters, globalVar.shtCounters)) {
                        ulong   nErr = AdslMibShowtimeSFErrors(counters, globalVar.shtCounters);
                        AdslMibUpdateShowtimeErrors(gDslVars, nErr);
                    }
                    n = AdslMibShowtimeRSErrors(counters, globalVar.shtCounters);
                    if (n != 0)
                        AdslMibUpdateShowtimeRSErrors(gDslVars, n);

                    val = counters[kG992ShowtimeSuperFramesRcvd] - globalVar.shtCounters[kG992ShowtimeSuperFramesRcvd];
                    AddBlockCounterVar(pChanPerfData, adslChanReceivedBlks, val);
                    AddBlockCounterVar(pChanPerfData, adslChanTransmittedBlks, val);  /* TBD */

                    val = counters[kG992ShowtimeSuperFramesRcvdWrong] - globalVar.shtCounters[kG992ShowtimeSuperFramesRcvdWrong];
                    AddBlockCounterVar(pChanPerfData, adslChanUncorrectBlks, val);

                    val = counters[kG992ShowtimeRSCodewordsRcvedCorrectable] - globalVar.shtCounters[kG992ShowtimeRSCodewordsRcvedCorrectable];
                    AddBlockCounterVar(pChanPerfData, adslChanCorrectedBlks, val);
					
					if(AdslMibIsAdsl2Mod(gDslVars)) {
						counters[kG992ShowtimeNumOfFEBE] = globalVar.shtCounters[kG992ShowtimeNumOfFEBE];
						counters[kG992ShowtimeNumOfFECC] = globalVar.shtCounters[kG992ShowtimeNumOfFECC];
						counters[kG992ShowtimeNumOfFHEC] = globalVar.shtCounters[kG992ShowtimeNumOfFHEC];
					}
					AdslMibConnectionStatUpdate (gDslVars, globalVar.shtCounters, counters);
                    AdslMibByteMove(sizeof(globalVar.shtCounters), counters, globalVar.shtCounters);
                    }
                    break;

                case kDslChannelResponseLog:
                    n = val << 1;
                    if (n >  sizeof(globalVar.chanCharLog))
                        n = sizeof(globalVar.chanCharLog);
                    AdslMibByteMove(n, status->param.dslConnectInfo.buffPtr, globalVar.chanCharLog);
                    break;

                case kDslChannelResponseLinear:
                    n = val << 1;
                    if (n >  sizeof(globalVar.chanCharLin))
                        n = sizeof(globalVar.chanCharLin);
                    AdslMibByteMove(n, status->param.dslConnectInfo.buffPtr, globalVar.chanCharLin);
                    break;

                case kDslChannelQuietLineNoise:
                    {
                    int     i;
                    uchar   *pNoiseBuf;

                    n = (val < globalVar.nTones ? val : globalVar.nTones);
                    pNoiseBuf = (uchar *) status->param.dslConnectInfo.buffPtr;

                    for (i = 0; i < n; i++)
                        globalVar.quietLineNoise[i] = (-23 << 4) - (pNoiseBuf[i] << 3);
                    }
                    break;

				case kDslNLMaxCritNoise:
						globalVar.adslMib.adslNonLinData.maxCriticalDistNoise= val;
						break;
				case kDslNLAffectedBits:
						globalVar.adslMib.adslNonLinData.distAffectedBits=val;
						break;
                case    kDslATURXmtPowerInfo:
                    globalVar.adslMib.adslPhys.adslCurrOutputPwr = Q4ToTenth(val);
                    break;

                case    kDslATUCXmtPowerInfo:
                    globalVar.adslMib.adslAtucPhys.adslCurrOutputPwr = Q4ToTenth(val);
                    break;

                case    kDslFramingModeInfo:
                    globalVar.adslMib.adslFramingMode = 
                        (globalVar.adslMib.adslFramingMode & ~kAdslFramingModeMask) | (val & kAdslFramingModeMask);
                    break;

                case    kDslATUHardwareAGCObtained:
                    globalVar.adslMib.afeRxPgaGainQ1 = val >> 3;
                    break;

                default:
                    break;
            }
            break;

        case kDslShowtimeSNRMarginInfo:
            if (status->param.dslShowtimeSNRMarginInfo.avgSNRMargin >= globalVar.showtimeMarginThld)
                globalVar.adslMib.adslPhys.adslCurrStatus &= ~kAdslPhysStatusLOM;
            else if (0 == (globalVar.adslMib.adslPhys.adslCurrStatus & kAdslPhysStatusLOS))
                globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLOM;
            globalVar.adslMib.adslPhys.adslCurrSnrMgn = Q4ToTenth(status->param.dslShowtimeSNRMarginInfo.avgSNRMargin);
            val = status->param.dslShowtimeSNRMarginInfo.nCarriers;
            n = kAdslMibToneNum;
#ifdef G992_ANNEXC
            if (kAdslModAnnexI == AdslMibGetModulationType(gDslVars))
                n = kAdslMibToneNum << 1;
#endif
#ifdef G992P5
            if (kAdslModAdsl2p == AdslMibGetModulationType(gDslVars))
                n = kAdslMibToneNum << 1;
#endif
            if (val > n)
                val = n;
            if (val != 0)
                AdslMibByteMove(
                    val * sizeof(globalVar.showtimeMargin[0]),
                    status->param.dslShowtimeSNRMarginInfo.buffPtr, 
                    globalVar.showtimeMargin);
            break;

        case kDslEscapeToG994p1Status:
            switch (globalVar.adslMib.adslTrainingState) {
              case kAdslTrainingConnected:
                globalVar.adslMib.adslPerfData.failTotal.adslRetr++;
                globalVar.adslMib.adslPerfData.failSinceShowTime.adslRetr++;
                n = globalVar.adslMib.adslPhys.adslCurrStatus;
                if (n & kAdslPhysStatusLOS)
                {
                    globalVar.adslMib.adslPerfData.failTotal.adslRetrLos++;
                    globalVar.adslMib.adslPerfData.failSinceShowTime.adslRetrLos++;
                }
                else if (n & kAdslPhysStatusLOF)
                {
                    globalVar.adslMib.adslPerfData.failTotal.adslRetrLof++;                    
                    globalVar.adslMib.adslPerfData.failSinceShowTime.adslRetrLof++;
                }
                else if (n & kAdslPhysStatusLOM)
                {
                    globalVar.adslMib.adslPerfData.failTotal.adslRetrLom++;
                    globalVar.adslMib.adslPerfData.failSinceShowTime.adslRetrLom++;
                }
                else if (n & kAdslPhysStatusLPR)
                {
                    globalVar.adslMib.adslPerfData.failTotal.adslRetrLpr++;
                    globalVar.adslMib.adslPerfData.failSinceShowTime.adslRetrLpr++;
                }
                break;

              case kAdslTrainingG992Started:
              case kAdslTrainingG992ChanAnalysis:
              case kAdslTrainingG992Exchange:
                globalVar.adslMib.adslPerfData.failTotal.adslInitErr++;
                globalVar.adslMib.adslPerfData.failSinceShowTime.adslInitErr++;
                break;
              case kAdslTrainingG994:
                  globalVar.adslMib.adslPerfData.failTotal.adslLineSearch++;
                  globalVar.adslMib.adslPerfData.failSinceShowTime.adslLineSearch++;
                break;  
            }
            if (0 == globalVar.timeConStarted)
                globalVar.timeConStarted = globalVar.timeSec;
            globalVar.adslMib.adslTrainingState = kAdslTrainingIdle;
            globalVar.adslMib.adslRxNonStdFramingAdjustK = 0;
            globalVar.rsOptionValid = false;
            globalVar.rsOption[0] = 0;

            globalVar.nMsgCnt = 0;
            n = globalVar.linkStatus;
            globalVar.linkStatus = 0;
            if (n != 0)
                AdslMibNotify(gDslVars, kAdslEventLinkChange);                      
            break;

        case kDslOLRBitGainUpdateStatus:
            {
            void        *p = status->param.dslOLRRequest.carrParamPtr;
            Boolean     bAdsl2 = (status->param.dslOLRRequest.msgType >= kOLRRequestType4);

            for (n = 0; n < status->param.dslOLRRequest.nCarrs; n++)
                p = AdslMibUpdateBitGain(gDslVars, p, bAdsl2);  
            }   
            break;

        case kAtmStatus:
            pAtmData = (kAdslIntlChannel == AdslMibGetActiveChannel(gDslVars) ?
                        &globalVar.adslMib.adslChanIntlAtmPhyData : &globalVar.adslMib.adslChanFastAtmPhyData);
            switch (status->param.atmStatus.code) {
                case kAtmStatRxHunt:
                case kAtmStatRxPreSync:
                    if (kAtmPhyStateNoAlarm == pAtmData->atmInterfaceTCAlarmState)
                        pAtmData->atmInterfaceOCDEvents++;
                    pAtmData->atmInterfaceTCAlarmState = kAtmPhyStateLcdFailure;
                    break;
                case kAtmStatRxSync:
                    pAtmData->atmInterfaceTCAlarmState = kAtmPhyStateNoAlarm;
                    break;
                case kAtmStatBertResult:
                    globalVar.adslMib.adslBertRes.bertTotalBits = 
                        status->param.atmStatus.param.bertInfo.totalBits;
                    globalVar.adslMib.adslBertRes.bertErrBits = 
                        status->param.atmStatus.param.bertInfo.errBits;
                    break;
                case kAtmStatHdrCompr:
                    if (status->param.atmStatus.param.value)
                        globalVar.adslMib.adslFramingMode |= kAtmHeaderCompression;
                    else
                        globalVar.adslMib.adslFramingMode &= ~kAtmHeaderCompression;
                    break;
                case kAtmStatCounters:
                    {
                    atmPhyCounters  *p = (void *) status->param.atmStatus.param.value;

                    globalVar.adslMib.atmStat.rcvStat.cntCellTotal = p->rxCellTotal;
                    globalVar.adslMib.atmStat.rcvStat.cntCellData  = p->rxCellData;
                    globalVar.adslMib.atmStat.rcvStat.cntCellDrop  = p->rxCellDrop;
                    globalVar.adslMib.atmStat.rcvStat.cntBitErrs   = p->bertBitErrors;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

Public void AdslMibClearBertResults(void *gDslVars)
{
    globalVar.adslMib.adslBertRes.bertTotalBits = 0;
    globalVar.adslMib.adslBertRes.bertErrBits = 0;
}

Public void AdslMibBertStartEx(void *gDslVars, ulong bertSec)
{
    AdslMibClearBertResults(gDslVars);

    globalVar.adslMib.adslBertStatus.bertTotalBits.cntHi = 0;
    globalVar.adslMib.adslBertStatus.bertTotalBits.cntLo = 0;
    globalVar.adslMib.adslBertStatus.bertErrBits.cntHi = 0;
    globalVar.adslMib.adslBertStatus.bertErrBits.cntLo = 0;

    globalVar.adslMib.adslBertStatus.bertSecTotal = bertSec;
    globalVar.adslMib.adslBertStatus.bertSecElapsed = 1;
    globalVar.adslMib.adslBertStatus.bertSecCur = (ulong ) -1;
}

Public void AdslMibBertStopEx(void *gDslVars)
{
    globalVar.adslMib.adslBertStatus.bertSecCur = 0;
}

Private void AdslMibAdd64 (cnt64 *pCnt64, ulong num)
{
    ulong   n;

    n = pCnt64->cntLo + num;
    if ((n < pCnt64->cntLo) || (n < num))
        pCnt64->cntHi++;

    pCnt64->cntLo = n;
}

Public ulong AdslMibBertContinueEx(void *gDslVars, ulong totalBits, ulong errBits)
{
    if (0 == globalVar.adslMib.adslBertStatus.bertSecCur)
        return 0;

    AdslMibAdd64(&globalVar.adslMib.adslBertStatus.bertTotalBits, totalBits);
    AdslMibAdd64(&globalVar.adslMib.adslBertStatus.bertErrBits, errBits);

    globalVar.adslMib.adslBertStatus.bertSecElapsed += globalVar.adslMib.adslBertStatus.bertSecCur;
    if (globalVar.adslMib.adslBertStatus.bertSecElapsed >= globalVar.adslMib.adslBertStatus.bertSecTotal) {
        globalVar.adslMib.adslBertStatus.bertSecCur = 0;
        return 0;
    }
        
    if (AdslMibIsLinkActive(gDslVars)) {
        long    nBits, nSec, nSecLeft;
        
        nBits = AdslMibGetGetChannelRate(gDslVars, kAdslRcvDir, kAdslIntlChannel);
        nBits += AdslMibGetGetChannelRate(gDslVars, kAdslRcvDir, kAdslFastChannel);
        nBits = (nBits * 48) / 53;
        nSec = 0xFFFFFFF0 / nBits;
        nSecLeft = globalVar.adslMib.adslBertStatus.bertSecTotal - globalVar.adslMib.adslBertStatus.bertSecElapsed;
        if (nSec > nSecLeft)
            nSec = nSecLeft;
        if (nSec > 20)
            nSec = 20;

        globalVar.adslMib.adslBertStatus.bertSecCur = nSec;
        return nSec * nBits;
    }
    else {
        globalVar.adslMib.adslBertStatus.bertSecCur = 1;
        return 8000000;
    }
}

Public void AdslMibSetLPR(void *gDslVars)
{
    globalVar.adslMib.adslPhys.adslCurrStatus |= kAdslPhysStatusLPR;
}

Public void AdslMibSetShowtimeMargin(void *gDslVars, long showtimeMargin)
{
    globalVar.showtimeMarginThld = showtimeMargin;
}

Public void  *AdslMibGetData (void *gDslVars, int dataId, void *pAdslMibData)
{
    switch (dataId) {
        case kAdslMibDataAll:
            if (NULL != pAdslMibData)
                AdslMibByteMove (sizeof(adslMibVarsStruct), &globalVar, pAdslMibData);
            else
                pAdslMibData = &globalVar;
            break;

        default:
            pAdslMibData = NULL;
    }   
    return pAdslMibData;
}
