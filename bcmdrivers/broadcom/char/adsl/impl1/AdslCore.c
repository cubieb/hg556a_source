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
 * AdslCore.c -- Bcm ADSL core driver
 *
 * Description:
 *	This file contains BCM ADSL core driver 
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslCore.c,v 1.1 2008/08/25 06:40:53 l65130 Exp $
 *
 * $Log: AdslCore.c,v $
 * Revision 1.1  2008/08/25 06:40:53  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:15  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:46:52  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:51  z45221
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
 * Revision 1.7  2004/07/20 23:45:48  ilyas
 * Added driver version info, SoftDslPrintf support. Fixed G.997 related issues
 *
 * Revision 1.6  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.5  2004/05/06 20:03:51  ilyas
 * Removed debug printf
 *
 * Revision 1.4  2004/05/06 03:24:02  ilyas
 * Added power management commands
 *
 * Revision 1.3  2004/04/30 17:58:01  ilyas
 * Added framework for GDB communication with ADSL PHY
 *
 * Revision 1.2  2004/04/27 00:33:38  ilyas
 * Fixed buffer in shared SDRAM checking for EOC messages
 *
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#endif

#include "AdslCore.h"
#include "AdslCoreMap.h"

#ifdef ADSL_SELF_TEST
#include "AdslSelfTest.h"
#endif

#include "softdsl/SoftDsl.h"
#include "softdsl/CircBuf.h"
#include "softdsl/BlankList.h"
#include "softdsl/BlockUtil.h"
#include "softdsl/Flatten.h"
#include "softdsl/AdslXfaceData.h"
#if defined(__KERNEL__) || defined(_CFE_)

#include <linux/version.h>
#include "bcm_common.h"
#include "bcm_map.h"
#include "bcm_intr.h"

#elif defined(VXWORKS)

#if defined(CONFIG_BCM96348)
#include "6348_common.h"
#elif defined(CONFIG_BCM96338)
#include "6338_common.h"
#elif defined(CONFIG_BCM96358)
#include "6358_common.h"
#elif defined(CONFIG_BCM96368)
#include "6368_common.h"
#endif

#endif /* VXWORKS */

#include "AdslCoreFrame.h"

#if defined(ADSL_PHY_FILE) || defined(ADSL_PHY_FILE2)
#include "AdslFile.h"
#else

#if defined(CONFIG_BCM963x8)
#ifdef ADSL_ANNEXC
#include "adslcore6348C/adsl_lmem.h"
#include "adslcore6348C/adsl_sdram.h"
#elif defined(ADSL_ANNEXB)
#include "adslcore6348B/adsl_lmem.h"
#include "adslcore6348B/adsl_sdram.h"
#elif defined(ADSL_SADSL)
#include "adslcore6348SA/adsl_lmem.h"
#include "adslcore6348SA/adsl_sdram.h"
#else
#include "adslcore6348/adsl_lmem.h"
#include "adslcore6348/adsl_sdram.h"
#endif
#endif /* of CONFIG_BCM963x8 */

#endif /* ADSL_PHY_FILE */

#ifdef G997_1_FRAMER
#include "softdsl/G997.h"
#endif

#ifdef ADSL_MIB
#include "softdsl/AdslMib.h"
#endif

#ifdef G992P3
#include "softdsl/G992p3OvhMsg.h"
#undef	G992P3_DEBUG
#define	G992P3_DEBUG
#endif

#include "softdsl/SoftDsl.gh"

#include <stdarg.h>

#undef	SDRAM_HOLD_COUNTERS

#ifdef CONFIG_BCM963x8

#ifdef DEBUG_L2_RET_L0
#include "BcmOs.h"
static ulong	gL2Start				= 0;
#endif

static ulong	gTimeInL2Ms			= 0;
static ulong	gL2SwitchL0TimeMs	= (ulong)-1;
static ulong	gTimeToWarn 		= 0;
#define		kMemPrtyWarnTime	(5 * 1000)	/* 5 Seconds */

#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96348)

#define 		kMemEnPrtySetMsk	(MEMC_EN_PRIOR | MEMC_ADSL_HPE)
#define		kMemPrtyRegAddr		(MEMC_BASE+MEMC_PRIOR)

#elif defined(CONFIG_BCM96358) || defined(CONFIG_BCM96368)

#define		kMemEnPrtySetMsk	(MEMC_SEL_PRIORITY | MEMC_HIPRRTYQEN | MEMC_MIPS1HIPREN)
#define		kMemPrtyRegAddr		(MEMC_BASE+MEMC_CONFIG)

#else

#error 		"Unknown 963x8 chip"

#endif

#endif /* CONFIG_BCM963x8 */

extern void BcmAdslCoreDiagWriteStatusString(char *fmt, ...);
extern ulong BcmAdslCoreGetCycleCount(void);
extern ulong BcmAdslCoreCycleTimeElapsedUs(ulong cnt1, ulong cnt0);

typedef ulong (*adslCoreStatusCallback) (dslStatusStruct *status, char *pBuf, int len);

/* Local vars */

void * AdslCoreGetOemParameterData (int paramId, int **ppLen, int *pMaxLen);

static ulong AdslCoreIdleStatusCallback (dslStatusStruct *status, char *pBuf, int len)
{
	return 0;
}

adslPhyInfo			adslCorePhyDesc = { 
	0xA0000000 | (ADSL_SDRAM_TOTAL_SIZE - ADSL_PHY_SDRAM_PAGE_SIZE), 0, 0, ADSL_PHY_SDRAM_START_4,
	0, 0, 0, 0,
	NULL, {0,0,0,0}
};
AdslXfaceData		* volatile pAdslXface	= NULL;
AdslOemSharedData	* volatile pAdslOemData = NULL;
adslCoreStatusCallback	pAdslCoreStatusCallback = AdslCoreIdleStatusCallback;
uchar				*pAdslSharedMemAlloc = NULL;
int					adslCoreSelfTestMode = kAdslSelfTestLMEM;
Boolean				adslCoreShMarginMonEnabled = AC_FALSE;
ulong				adslCoreLOMTimeout = (ulong) -1;
long				adslCoreLOMTime = -1;
Boolean				adslCoreOvhMsgPrintEnabled = AC_FALSE;

dslVarsStruct	acDslVars;
#define			gDslVars	(&acDslVars)
long			flattenAddrAdj = FLATTEN_ADDR_ADJUST - 0x19000000;
Boolean			acBlockStatusRead = AC_FALSE;
#define			ADSL_SDRAM_RESERVED			32
#define			ADSL_INIT_MARK				0xDEADBEEF
#define			ADSL_INIT_TIME				(30*60*1000)
typedef  struct {
	ulong		initMark;
	ulong		timeCnt;
	/* add more fields here */
} sdramReservedAreaStruct;
sdramReservedAreaStruct		*pSdramReserved = NULL;
ulong			adslCoreEcUpdateMask = 0;

#ifdef VXWORKS
int				ejtagEnable = 0;
#endif

static unsigned long timeUpdate=0;
static int pendingFrFlag=0;
/*
**
**		ADSL Core SDRAM memory functions
**
*/

void *AdslCoreGetPhyInfo(void)
{
	return &adslCorePhyDesc;
}

void *AdslCoreGetSdramPageStart(void)
{
	return (void *) adslCorePhyDesc.sdramPageAddr;
}

void *AdslCoreGetSdramImageStart(void)
{
	return (void *) adslCorePhyDesc.sdramImageAddr;
}

unsigned long AdslCoreGetSdramImageSize(void)
{
	return adslCorePhyDesc.sdramImageSize;
}

void * AdslCoreSetSdramImageAddr(ulong lmem2, ulong sdramSize)
{
	if (0 == lmem2) {
		lmem2 = (sdramSize > 0x40000) ? 0x20000 : 0x40000;
		adslCorePhyDesc.sdramPhyImageAddr = ADSL_PHY_SDRAM_START + lmem2;
	}
	else
		adslCorePhyDesc.sdramPhyImageAddr = lmem2;
	lmem2 &= (ADSL_PHY_SDRAM_PAGE_SIZE-1);
	adslCorePhyDesc.sdramImageAddr = adslCorePhyDesc.sdramPageAddr + lmem2;
#if (ADSL_PHY_SDRAM_PAGE_SIZE == 0x200000)
	if ((lmem2 & 0x00FFFFFF) < 0x100000)	/* old 256K PHY over orig. 2M */
		adslCorePhyDesc.sdramImageAddr += (0x200000 - 0x80000);
#endif
	adslCorePhyDesc.sdramImageSize = (sdramSize+0xF) & ~0xF;
	pSdramReserved = (void *) (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - sizeof(sdramReservedAreaStruct));
	AdslDrvPrintf(TEXT("pSdramPHY=0x%X, 0x%X 0x%X\n"), (int) pSdramReserved, pSdramReserved->timeCnt, pSdramReserved->initMark);
	adslCoreEcUpdateMask = 0;
	if (ADSL_INIT_MARK != pSdramReserved->initMark) {
		pSdramReserved->timeCnt = 0;
		pSdramReserved->initMark = ADSL_INIT_MARK;
	}
	if (pSdramReserved->timeCnt >= ADSL_INIT_TIME)
		adslCoreEcUpdateMask |= kDigEcShowtimeFastUpdateDisabled;
	return (void *) adslCorePhyDesc.sdramImageAddr;
}

static Boolean AdslCoreIsPhySdramAddr(void *ptr)
{
	ulong	addr = ((ulong) ptr) | 0xA0000000;

	return (addr >= adslCorePhyDesc.sdramImageAddr) && (addr < (adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE));
}

#define ADSL_PHY_SDRAM_SHARED_START		(adslCorePhyDesc.sdramPageAddr + ADSL_PHY_SDRAM_PAGE_SIZE - ADSL_SDRAM_RESERVED)

#define SHARE_MEM_REQUIRE 			2048	/* 936 bytes worst case, but will use 2KB */
int	adslPhyShareMemIsCalloc		= 0;
int	adslPhyShareMemSizeAllow		= 0;
void * adslPhyShareMemStart			= NULL;

void AdslCoreSharedMemInit(void)
{
	int shareMemAvailable = ADSL_SDRAM_IMAGE_SIZE - ADSL_SDRAM_RESERVED - AdslCoreGetSdramImageSize();
	
	if( adslPhyShareMemIsCalloc == 0 ) { /* If calloc() earlier, then will continure to use it; might be from Diags download */
		if(  shareMemAvailable < SHARE_MEM_REQUIRE ) {
			adslPhyShareMemStart = calloc(1,SHARE_MEM_REQUIRE);
			adslPhyShareMemStart = (void *)(0xA0000000 | ((ulong)adslPhyShareMemStart  + SHARE_MEM_REQUIRE));
			adslPhyShareMemIsCalloc = 1;
			adslPhyShareMemSizeAllow = SHARE_MEM_REQUIRE;
		}
		else {
			adslPhyShareMemStart = (void *) ADSL_PHY_SDRAM_SHARED_START;
			adslPhyShareMemSizeAllow = shareMemAvailable;
		}
	}

	pAdslSharedMemAlloc = adslPhyShareMemStart;
	
}

void AdslCoreSharedMemFree(void *p)
{
	pAdslSharedMemAlloc = adslPhyShareMemStart;	
}

/* Note: If called from User context, make sure to block bottom half switching to prevent a potential problem
             in this simple implementation */
void *AdslCoreSharedMemAlloc(long size)
{
	ulong	addr;

	addr = ((ulong) pAdslSharedMemAlloc - size) & ~3;
	if (addr < (adslPhyShareMemStart - adslPhyShareMemSizeAllow)) {
		AdslDrvPrintf(TEXT("***No shared SDRAM ptr=0x%X size=%d\n"), (int) pAdslSharedMemAlloc, size);
		AdslCoreSharedMemInit();
		addr = ((ulong) pAdslSharedMemAlloc - size) & ~3;
	}

	pAdslSharedMemAlloc = (void *) addr;

	return pAdslSharedMemAlloc;
}

void *AdslCoreGdbAlloc(long size)
{
	uchar	*p;

	p = (void *) (adslCorePhyDesc.sdramImageAddr + adslCorePhyDesc.sdramImageSize);
	adslCorePhyDesc.sdramImageSize += (size + 0xF) & ~0xF;
	return p;
}

void AdslCoreGdbFree(void *p)
{
	adslCorePhyDesc.sdramImageSize = ((ulong) p - adslCorePhyDesc.sdramImageAddr);
}


/*
**
**		ADSL Core Status/Command functions 
**
*/

void AdslCoreSetL2Timeout(ulong val)
{
	if( 0 == val)
		gL2SwitchL0TimeMs = (ulong)-1;
	else
		gL2SwitchL0TimeMs = val * 1000;	/* Convert # of Sec to Ms */
}
void AdslCoreIndicateLinkPowerStateL2(void)
{
	gTimeInL2Ms = 0;
#ifdef DEBUG_L2_RET_L0
	bcmOsGetTime(&gL2Start);
#endif
}

void AdslCoreIndicateLinkDown(void)
{
	dslStatusStruct status;

	status.code = kDslEscapeToG994p1Status;
	AdslMibStatusSnooper(gDslVars, &status);
}

void AdslCoreIndicateLinkUp(void)
{
	dslStatusStruct status;

	status.code = kDslTrainingStatus;
	status.param.dslTrainingInfo.code = kDslG992p2RxShowtimeActive;
	status.param.dslTrainingInfo.value= 0;
	AdslMibStatusSnooper(gDslVars, &status);

	status.code = kDslTrainingStatus;
	status.param.dslTrainingInfo.code = kDslG992p2TxShowtimeActive;
	status.param.dslTrainingInfo.value= 0;

	AdslMibStatusSnooper(gDslVars, &status);
}

void AdslCoreIndicateLinkPowerState(int pwrState)
{
	dslStatusStruct status;

	status.code = kDslConnectInfoStatus;
	status.param.dslConnectInfo.code = kG992p3PwrStateInfo;
	status.param.dslConnectInfo.value= pwrState;
	AdslMibStatusSnooper(gDslVars, &status);
}

AC_BOOL AdslCoreCommandWrite(dslCommandStruct *cmdPtr)
{
	int				n;

	n = FlattenBufferCommandWrite(&pAdslXface->sbCmd, cmdPtr);
	if (n > 0) {
#if 0 && !defined(BCM_CORE_NO_HARDWARE)
		volatile ulong	*pAdslEnum;

		pAdslEnum = (ulong *) ADSL_ENUM_BASE;
		pAdslEnum[ADSL_HOSTMESSAGE] = 1;
#endif
		return AC_TRUE;
	}
	return AC_FALSE;		
}

int AdslCoreFlattenCommand(void *cmdPtr, void *dstPtr, ulong nAvail)
{
	return FlattenCommand (cmdPtr, dstPtr, nAvail);
}

AC_BOOL AdslCoreCommandIsPending(void)
{
	return StretchBufferGetReadAvail(&pAdslXface->sbCmd) ? AC_TRUE : AC_FALSE;
}

AC_BOOL AdslCoreCommandHandler(void *cmdPtr)
{
	dslCommandStruct	*cmd = (dslCommandStruct *) cmdPtr;
#ifdef G992P3
	g992p3DataPumpCapabilities	*pG992p3Cap = cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA;
	g992p3DataPumpCapabilities	*pTmpG992p3Cap;
	int							n;
#ifdef G992P5
	g992p3DataPumpCapabilities	*pG992p5Cap = cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA;
#endif
#endif
	AC_BOOL				bRes;

	switch (cmd->command) {
#ifdef ADSL_MIB
		case kDslDiagStartBERT:
			AdslMibClearBertResults(gDslVars);
			break;
		case kDslDiagStopBERT:
			{
			adslMibInfo			*pMibInfo;
			ulong				mibLen;

			mibLen = sizeof(adslMibInfo);
			pMibInfo = (void *) AdslCoreGetObjectValue (NULL, 0, NULL, &mibLen);
			if (pMibInfo->adslBertStatus.bertSecCur != 0)
				AdslCoreBertStopEx();
			}
			break;
		case kDslDyingGaspCmd:
			AdslMibSetLPR(gDslVars);
			break;
#endif
#ifdef G992P3
		case kDslStartPhysicalLayerCmd:

			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
				n = sizeof(g992p3DataPumpCapabilities);
#ifdef G992P5
				if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p))
					n += sizeof(g992p3DataPumpCapabilities);
#endif
				pTmpG992p3Cap = AdslCoreSharedMemAlloc(n+0x10);
				if (NULL != pTmpG992p3Cap) {
					*pTmpG992p3Cap = *pG992p3Cap;
#ifdef G992P5
					if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
						pTmpG992p3Cap++;
						*pTmpG992p3Cap = *pG992p5Cap;
						pTmpG992p3Cap--;
					}
#endif
				}
				else
					return AC_FALSE;
				cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = pTmpG992p3Cap;
#ifdef G992P5
				cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = pTmpG992p3Cap+1;
#endif
			}

			AdslCoreIndicateLinkPowerState(0);
			break;

		case kDslTestCmd:
			AdslCoreIndicateLinkPowerState(0);
			break;

		case kDslOLRRequestCmd:
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2) && !AdslCoreIsPhySdramAddr(cmd->param.dslOLRRequest.carrParamPtr)) {
				if (kAdslModAdsl2p == AdslMibGetModulationType(gDslVars))
					n = cmd->param.dslOLRRequest.nCarrs * sizeof(dslOLRCarrParam2p);
				else
					n = cmd->param.dslOLRRequest.nCarrs * sizeof(dslOLRCarrParam);
				if (NULL != (pTmpG992p3Cap = AdslCoreSharedMemAlloc(n))) {
					memcpy (pTmpG992p3Cap, cmd->param.dslOLRRequest.carrParamPtr, n);
					cmd->param.dslOLRRequest.carrParamPtr = pTmpG992p3Cap;
				}
				else
					return AC_FALSE;
			}
			break;
#endif

		case kDslIdleCmd:
			AdslCoreIndicateLinkDown();
			break;

		case kDslLoopbackCmd:
			AdslCoreIndicateLinkUp();
			break;

		default:
			break;
	}
	bRes = AdslCoreCommandWrite(cmd);

#ifdef DIAG_DBG
	if(bRes == AC_FALSE)
		printk("%s: AdslCoreCommandWrite failed!", __FUNCTION__);		
#endif

#ifdef G992P3
	if (kDslStartPhysicalLayerCmd == cmd->command) {
		cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = pG992p3Cap;
#ifdef G992P5
		cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = pG992p5Cap;
#endif
	}
#endif

	return bRes;
}


int AdslCoreStatusRead (dslStatusStruct *status)
{
	int		n;

	if (acBlockStatusRead)
		return 0;

	n = FlattenBufferStatusRead(&pAdslXface->sbSta, status);

	if (n < 0) {
		AdslDrvPrintf (TEXT("Status read failure: len=%d, st.code=%lu, st.value=%ld\n"),
			-n, (unsigned long) status->code, status->param.value);
		n = 0;
	}

	return n;
}

void AdslCoreStatusReadComplete (int nBytes)
{
	FlattenBufferReadComplete (&pAdslXface->sbSta, nBytes);
}

AC_BOOL AdslCoreStatusAvail (void)
{
	return StretchBufferGetReadAvail(&pAdslXface->sbSta) ? AC_TRUE : AC_FALSE;
}

void *AdslCoreStatusReadPtr (void)
{
	void *p = StretchBufferGetReadPtr(&pAdslXface->sbSta);

#ifdef FLATTEN_ADDR_ADJUST
	p	= (void *) (((long) p) + flattenAddrAdj);
#endif
	return p;
}

static AC_BOOL AdsCoreStatBufInitialized(void *bufPtr)
{
    volatile int		cnt;

	if (bufPtr != StretchBufferGetStartPtr(&pAdslXface->sbSta))
		return AC_FALSE;

	if (bufPtr != StretchBufferGetReadPtr(&pAdslXface->sbSta))
		return AC_FALSE;

	cnt = 20;
	do {
	} while (--cnt != 0);

	return AC_TRUE;
}

#define AdsCoreStatBufAssigned()	(pAdslXface->sbSta.pExtraEnd != NULL)
#define TmElapsedUs(cnt0)			BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cnt0)


AC_BOOL AdslCoreSetStatusBuffer(void *bufPtr, int bufSize)
{
	dslCommandStruct	cmd;
	
	if (NULL == bufPtr)
		bufPtr = (void *) (adslCorePhyDesc.sdramImageAddr + adslCorePhyDesc.sdramImageSize);
	bufPtr = SDRAM_ADDR_TO_ADSL(bufPtr);
	cmd.command = kDslSetStatusBufferCmd;
	cmd.param.dslStatusBufSpec.pBuf = bufPtr;
	cmd.param.dslStatusBufSpec.bufSize = bufSize;

	acBlockStatusRead = AC_TRUE;

	if (!AdslCoreCommandWrite(&cmd)) {
		acBlockStatusRead = AC_FALSE;
		return AC_FALSE;
	}

	do {
	} while (AdsCoreStatBufInitialized(bufPtr));

	flattenAddrAdj = (long) SDRAM_ADDR_TO_HOST(0);
	acBlockStatusRead = AC_FALSE;
	return AC_TRUE;
}

/*
**
**		G.997 callback and interface functions
**
*/

#ifdef G997_1_FRAMER

#define	kG997MaxRxPendingFrames		16
#define	kG997MaxTxPendingFrames		16

#define AdslCoreOvhMsgSupported(gDslV)		AdslMibIsAdsl2Mod(gDslV)

circBufferStruct	g997RxFrCB;
void *				g997RxFrBuf[kG997MaxRxPendingFrames];
dslFrameBuffer		*g997RxCurBuf = NULL;

typedef struct {
	dslFrame		fr;
	dslFrameBuffer	frBuf;
#ifdef G992P3
	dslFrameBuffer	frBufHdr;
	uchar			eocHdr[4];
#endif
} ac997FramePoolItem;

ac997FramePoolItem	g997TxFrBufPool[kG997MaxTxPendingFrames];
void *				g997TxFrList = NULL;

Boolean AdslCoreCommonCommandHandler(void *gDslV, dslCommandStruct *cmdPtr)
{
	return AdslCoreCommandWrite(cmdPtr);
}

void AdslCoreCommonStatusHandler(void *gDslV, dslStatusStruct *status)
{
	switch (status->code) {
		case kDslConnectInfoStatus:
			BcmAdslCoreDiagWriteStatusString("AdslCoreCommonSH (ConnInfo): code=%d, val=%d", 
				status->param.dslConnectInfo.code, status->param.dslConnectInfo.value);
			if ((kG992p3PwrStateInfo == status->param.dslConnectInfo.code) && 
				(3 == status->param.dslConnectInfo.value)) {
				AdslCoreIndicateLinkPowerState(3);
				BcmAdslCoreNotify(ACEV_LINK_POWER_L3);
			}	
			break;
		case kDslGetOemParameter:
			{
			void	*pOemData;
			int		maxLen, *pLen;

			pOemData = AdslCoreGetOemParameterData (status->param.dslOemParameter.paramId, &pLen, &maxLen);
			status->param.dslOemParameter.dataLen = *pLen;
			status->param.dslOemParameter.dataPtr = pOemData;
			if (0 == status->param.dslOemParameter.dataLen)
				status->param.dslOemParameter.dataPtr = NULL;
			}
			break;
		default:
			break;
	}
}

void AdslCoreStatusHandler(void *gDslV, dslStatusStruct *status)
{
	(*pAdslCoreStatusCallback) (status, NULL, 0);
}

void __SoftDslPrintf(void *gDslV, char *fmt, int argNum, ...)
{
	dslStatusStruct		status;
	va_list				ap;

	va_start(ap, argNum);

	status.code = kDslPrintfStatus;
	status.param.dslPrintfMsg.fmt = fmt;
	status.param.dslPrintfMsg.argNum = 0;
	status.param.dslPrintfMsg.argPtr = (void *)ap;
	va_end(ap);

	(*pAdslCoreStatusCallback) (&status, NULL, 0);
}
 
Boolean AdslCoreG997CommandHandler(void *gDslV, dslCommandStruct *cmdPtr)
{
	if ((kDslSendEocCommand == cmdPtr->command) && (kDslClearEocSendFrame == cmdPtr->param.dslClearEocMsg.msgId)) {
		if (AdslCoreIsPhySdramAddr(cmdPtr->param.dslClearEocMsg.dataPtr))
			cmdPtr->param.dslClearEocMsg.msgType &= ~kDslClearEocMsgDataVolatileMask;
		else 
			cmdPtr->param.dslClearEocMsg.msgType |= kDslClearEocMsgDataVolatileMask;
	}
	return AdslCoreCommandWrite(cmdPtr);
}

void AdslCoreG997StatusHandler(void *gDslV, dslStatusStruct *status)
{
}

#ifdef G992P3_DEBUG
void AdslCorePrintDebugData()
{
	BcmAdslCoreDiagWriteStatusString("gDslVars=0x%X, gG997Vars=0x%X, gG992p3OvhMsgVars=0x%X, gAdslMibVars=0x%X", 
				gDslVars, &gG997Vars, &gG992p3OvhMsgVars, &gAdslMibVars);
}

void AdslCoreG997PrintFrame(void *gDslV, char *hdr, dslFrame *pFrame)
{
	dslFrameBuffer		*pBuf;
	uchar			*pData;
	int				len, i;
	char				str[1024], *pStr;
	Boolean			bFirstBuf = true;

	if (!adslCoreOvhMsgPrintEnabled) {
		BcmAdslCoreWriteOvhMsg(gDslV, hdr, pFrame);
		return;
	}

	BcmAdslCoreDiagWriteStatusString(" G.997 frame %s: pFr = 0x%X, len = %ld", hdr, (int) pFrame, DslFrameGetLength(gDslV, pFrame));
	pBuf = DslFrameGetFirstBuffer(gDslV, pFrame);

	while (NULL != pBuf) {
		len   = DslFrameBufferGetLength(gDslV, pBuf);
		pData = DslFrameBufferGetAddress(gDslV, pBuf);

		if (bFirstBuf)
			pStr = str + sprintf(str, "  frameBuf: addr=0x%X, len=%d, (%s PRI%d %s %d) data = ", (int) pData, len, hdr,
				pData[0] & 3, (pData[1] & 2) ? "RSP" : "CMD", pData[1] & 1);
		else
			pStr = str + sprintf(str, "  frameBuf: addr=0x%X, len=%d, data = ", (int) pData, len);
		bFirstBuf = false;
		for (i = 0; i < len; i++) {
			pStr += sprintf(pStr, "0x%X ", *pData++);
			if ((str + sizeof(str) - pStr) < 10) {
				BcmAdslCoreDiagWriteStatusString(str);
				pStr = str + sprintf(str, "  Cont data[%d-] = ", i+1);
			}
		}
		BcmAdslCoreDiagWriteStatusString(str);

#if 0
		{
		extern int g_nAdslExit;
		
		if (pBuf == DslFrameGetNextBuffer(gDslV, pBuf)) 
			{
			BcmAdslCoreDiagWriteStatusString("AdslCoreG997PrintFrame: Failure: Loop in buffer list");
			AdslCorePrintDebugData();
			g_nAdslExit = 1;
			pBuf->length = 1;
			pBuf->next = NULL;
			return;
			}
		}
#endif
		pBuf = DslFrameGetNextBuffer(gDslV, pBuf);
	}
}
#endif

void AdslCoreG997Init(void)
{
	uchar	*p;
	int		i;

	CircBufferInit (&g997RxFrCB, g997RxFrBuf, sizeof(g997RxFrBuf));
	g997RxCurBuf = NULL;

	g997TxFrList = NULL;
	p = (void *) &g997TxFrBufPool;
	for (i = 0; i < kG997MaxTxPendingFrames; i++) {
		BlankListAdd(&g997TxFrList, (void*) p);
		p += sizeof(g997TxFrBufPool[0]);
	}
}

int __AdslCoreG997SendComplete(void *gDslV, void *userVc, ulong mid, dslFrame *pFrame)
{
	ac997FramePoolItem	*pFB = (ac997FramePoolItem	*)pFrame;
	char * pBuf = DslFrameBufferGetAddress(gDslVars, &pFB->frBuf);

	if( NULL != pBuf )
		free(pBuf);	
	BlankListAdd(&g997TxFrList, pFrame);
	BcmAdslCoreNotify(ACEV_G997_FRAME_SENT);
	
	return 1;
}

int AdslCoreG997SendComplete(void *gDslV, void *userVc, ulong mid, dslFrame *pFrame)
{
#ifdef G992P3
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2) && (kG992p3OvhMsgFrameBufCnt == pFrame->bufCnt)) {
		G992p3OvhMsgSendCompleteFrame(gDslV, userVc, mid, pFrame);
#if 0 && defined(G992P3_DEBUG)
		BcmAdslCoreDiagWriteStatusString ("Frame sent (from G992p3OvhMsg) pFrame = 0x%lX\n", (long) pFrame);
#endif
		return 1;
	}
#endif
#if 0 && defined(G992P3_DEBUG)
	BcmAdslCoreDiagWriteStatusString ("Frame sent (NOT from G992p3OvhMsg) pFrame = 0x%lX\n", (long) pFrame);
#endif
	return __AdslCoreG997SendComplete(gDslV, userVc, mid, pFrame);
}

#ifdef G992P3_DEBUG
int TstG997SendFrame (void *gDslV, void *userVc, ulong mid, dslFrame *pFrame)
{
	AdslCoreG997PrintFrame(gDslV, "TX", pFrame);
	return G997SendFrame(gDslVars, userVc, mid, pFrame);
}
#endif

int AdslCoreG997IndicateRecevice (void *gDslV, void *userVc, ulong mid, dslFrame *pFrame)
{
	void	**p;

#ifdef G992P3
#ifdef G992P3_DEBUG
	AdslCoreG997PrintFrame(gDslV, "RX", pFrame);
#endif
	if (AdslCoreOvhMsgSupported(gDslV)) {
		if (G992p3OvhMsgIndicateRcvFrame(gDslV, NULL, 0, pFrame)) {
			G997ReturnFrame(gDslVars, NULL, 0, pFrame);
			return 1;
		}
		// __SoftDslPrintf(NULL, "AdslCoreG997IndicateRecevice: ClearEOC: pFr=0x%X\n", 0, pFrame);
	}
	else
		*(((ulong*) DslFrameGetLinkFieldAddress(gDslV,pFrame)) + 2) = 0;
#endif
	if (CircBufferGetWriteAvail(&g997RxFrCB) > 0) {
	        if( pendingFrFlag==0){
		  pendingFrFlag=1;
		  timeUpdate=0;
		}
	       
		p = CircBufferGetWritePtr(&g997RxFrCB);
		*p = pFrame;
		CircBufferWriteUpdate(&g997RxFrCB, sizeof(void *));

		BcmAdslCoreNotify(ACEV_G997_FRAME_RCV);
	}
	else 
		BcmAdslCoreDiagWriteStatusString ("Frame Received but cannot be read as Buffer is full");
	
	return 1;
}

void AdslCoreSetL3(void)
{
	if (AdslCoreOvhMsgSupported(gDslVars))
		G992p3OvhMsgSetL3(gDslVars);
}

void AdslCoreSetL0(void)
{
	if (AdslCoreOvhMsgSupported(gDslVars))
		G992p3OvhMsgSetL0(gDslVars);
}

/* */

int AdslCoreG997SendFrame (dslFrame * pFrame)
{       
	return G997SendFrame(gDslVars, NULL, 0, pFrame);
}

AC_BOOL AdslCoreG997ReturnFrame (dslFrame * pFrame)
{
	dslFrame **p;
	AC_BOOL	 res = AC_FALSE;	

	if (CircBufferGetReadAvail(&g997RxFrCB) > 0) {
		p = CircBufferGetReadPtr(&g997RxFrCB);
#ifdef G992P3
		G992p3OvhMsgReturnFrame(gDslVars, NULL, 0, *p);
#endif
		res = G997ReturnFrame(gDslVars, NULL, 0, *p);
		CircBufferReadUpdate(&g997RxFrCB, sizeof(void *));
	}
	return res;
}

AC_BOOL AdslCoreG997SendData(void *buf, int len)
{
	ac997FramePoolItem	*pFB;
	AC_BOOL	 res = AC_FALSE;

	pFB = BlankListGet(&g997TxFrList);
	if (NULL == pFB)
		return res;

	DslFrameInit (gDslVars, &pFB->fr);
	DslFrameBufferSetAddress (gDslVars, &pFB->frBuf, buf);
	DslFrameBufferSetLength (gDslVars, &pFB->frBuf, len);
#ifdef G992P3
	if (AdslCoreOvhMsgSupported(gDslVars)) {
		DslFrameBufferSetAddress (gDslVars, &pFB->frBufHdr, pFB->eocHdr);
		DslFrameBufferSetLength (gDslVars, &pFB->frBufHdr, 4);
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBufHdr);
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBuf);
		res = G992p3OvhMsgSendClearEocFrame(gDslVars, &pFB->fr);
	}
	else
#endif
	{        
	       
		DslFrameEnqueBufferAtBack (gDslVars, &pFB->fr, &pFB->frBuf);
		AdslCoreG997PrintFrame(gDslVars,"TX",&pFB->fr);
		res = G997SendFrame(gDslVars, NULL, 0, &pFB->fr);
	}

	if( AC_FALSE == res )
		BlankListAdd(&g997TxFrList, &pFB->fr);

	return res;
}

#if 0
void TestG997SendData(void)
{
	static	uchar buf[1024] = { 0xFF, 0x3 };
	int			  i;

	__SoftDslPrintf(gDslVars, "TestG997SendData:", 0);
	for (i = 0; i < 266; i++)
		buf[2+i] = i & 0xFF;
	AdslCoreG997SendData(buf, 260);
}

static	dslFrame		frRcv;
static  dslFrameBuffer	frRcvBuf;
static  uchar			frRcvData[512];

void TestG997RcvDataInv(int param)
{
	__SoftDslPrintf(gDslVars, "TestG997RcvDataInv: param=%d\n", 0, param);

	frRcvData[0] = 1;		/* kG992p3PriorityNormal */
	frRcvData[1] = 0;		/* kG992p3Cmd */
	frRcvData[2] = 0x43;	/* kG992p3OvhMsgCmdClearEOC */
	frRcvData[3] = 1;		/* kG992p3OvhMsgCmdInvId */

	DslFrameInit (gDslVars, &frRcv);
	DslFrameBufferSetAddress (gDslVars, &frRcvBuf, frRcvData);
	DslFrameBufferSetLength (gDslVars, &frRcvBuf, 4);
	DslFrameEnqueBufferAtBack (gDslVars, &frRcv, &frRcvBuf);

	AdslCoreG997IndicateRecevice (gDslVars, NULL, 0, &frRcv);
}

void TestG997RcvData(int msgLen)
{
	int				i, frLen;
	dslFrame		*pFrame;
	dslFrameBuffer	*pFrBuf;
	uchar			*pData;

	__SoftDslPrintf(gDslVars, "TestG997RcvData: msgLen=%d\n", 0, msgLen);
	if (0 == msgLen)
		msgLen = 64;
	if (msgLen > sizeof(frRcvData))
		msgLen = sizeof(frRcvData);

	frRcvData[0] = 1;	/* kG992p3PriorityNormal */
	frRcvData[1] = 0;	/* kG992p3Cmd */
	frRcvData[2] = 0x8;	/* kG992p3OvhMsgCmdClearEOC */
	frRcvData[3] = 1;

	for (i = 0; i < msgLen; i++)
		frRcvData[4+i] = i & 0xFF;

	DslFrameInit (gDslVars, &frRcv);
	DslFrameBufferSetAddress (gDslVars, &frRcvBuf, frRcvData);
	DslFrameBufferSetLength (gDslVars, &frRcvBuf, msgLen+4);
	DslFrameEnqueBufferAtBack (gDslVars, &frRcv, &frRcvBuf);

	AdslCoreG997IndicateRecevice (gDslVars, NULL, 0, &frRcv);

	/* check how the frame was received */

	if (CircBufferGetReadAvail(&g997RxFrCB) == 0) {
		__SoftDslPrintf(gDslVars, "TestG997RcvData FAILED: no frame received\n", 0);
		return;
	}

	pFrame = *(dslFrame **)CircBufferGetReadPtr(&g997RxFrCB);
	if (pFrame != &frRcv) {
		__SoftDslPrintf(gDslVars, "TestG997RcvData FAILED: rcvFr=0x%X sentFr=0x%X\n", 0, pFrame, &frRcv);
		return;
	}
	pFrBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	if (pFrBuf != &frRcvBuf) {
		__SoftDslPrintf(gDslVars, "TestG997RcvData FAILED: rcvFrBuf=0x%X sentFrBuf=0x%X\n", 0, pFrBuf, &frRcvBuf);
		return;
	}
	frLen   = DslFrameBufferGetLength(gDslV, pFrBuf);
	pData = DslFrameBufferGetAddress(gDslV, pFrBuf);
	__SoftDslPrintf(gDslVars, "TestG997RcvData: pData=0x%X len=%d\n", 0, pData, frLen);
	if ((pData != (frRcvData+4)) || (frLen != msgLen)) {
		__SoftDslPrintf(gDslVars, "TestG997RcvData FAILED: pData=0x%X len=%d\n", 0, pData, frLen);
		return;
	}

	/* return frame through G992p3 */

	CircBufferReadUpdate(&g997RxFrCB, sizeof(void *));
	G992p3OvhMsgReturnFrame(gDslVars, NULL, 0, pFrame);
	pFrBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	frLen   = DslFrameBufferGetLength(gDslV, pFrBuf);
	pData = DslFrameBufferGetAddress(gDslV, pFrBuf);
	__SoftDslPrintf(gDslVars, "TestG997RcvData(RET): pData=0x%X len=%d\n", 0, pData, frLen);
	if ((pData != frRcvData) || (frLen != (msgLen+4))) {
		__SoftDslPrintf(gDslVars, "TestG997RcvData(RET) FAILED: pData=0x%X len=%d\n", 0, pData, frLen);
		return;
	}
}
#endif

int AdslCoreG997FrameReceived(void)
{
	return(CircBufferGetReadAvail(&g997RxFrCB) > 0);
}

void *AdslCoreG997BufGet(dslFrameBuffer	*pBuf, int *pLen)
{
	if (NULL == pBuf)
		return NULL;

	*pLen = DslFrameBufferGetLength(gDslVars, pBuf);
	return DslFrameBufferGetAddress(gDslVars, pBuf);
}

void *AdslCoreG997FrameGet(int *pLen)
{
	dslFrame *pFrame;
	
	if (CircBufferGetReadAvail(&g997RxFrCB) == 0)
		return NULL;

	pFrame = *(dslFrame **)CircBufferGetReadPtr(&g997RxFrCB);
	g997RxCurBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);
	pendingFrFlag=0;
	timeUpdate=0;
	return AdslCoreG997BufGet(g997RxCurBuf, pLen);
}

void *AdslCoreG997FrameGetNext(int *pLen)
{
	if (NULL == g997RxCurBuf)
		return NULL;

	g997RxCurBuf = DslFrameGetNextBuffer(gDslVars, g997RxCurBuf);
	return AdslCoreG997BufGet(g997RxCurBuf, pLen);
}

void AdslCoreG997FrameFinished(void)
{
	AdslCoreG997ReturnFrame (NULL);
}


#endif /* G997_1_FRAMER */

/*
**
**		ADSL MIB functions
**
*/

#ifdef ADSL_MIB

int AdslCoreMibNotify(void *gDslV, ulong event)
{
	if (event & kAdslEventLinkChange) {
		BcmAdslCoreNotify (AdslCoreLinkState() ? ACEV_LINK_UP : ACEV_LINK_DOWN);
	}
	return 0;
}

void * AdslCoreMibGetData (int dataId, void *pAdslMibData)
{
	return AdslMibGetData (gDslVars, dataId, pAdslMibData);
}

#endif

/*
**
**		Interface functions
**
*/

void setParam (ulong *addr, ulong val)
{
	*addr = val;
}

ulong getParam (ulong *addr)
{
	return *addr;
} 

#if 0 && defined(CONFIG_BCM963x8)

#define AFE_REG_BASE		0xFFF58000
#define	AFE_REG_DATA		(AFE_REG_BASE + 0xC)
#define	AFE_REG_CTRL		(AFE_REG_BASE + 0x8)

void writeAFE(ulong reg, ulong val)
{
	ulong	cycleCnt0;

	setParam((ulong *) AFE_REG_DATA, val & 0xff);
	/* need to wait 16 usecs here */
	cycleCnt0 = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cycleCnt0) < 16)	;
	setParam((ulong *) AFE_REG_CTRL, (reg << 8) | 0x05);
	while (getParam((ulong *) AFE_REG_CTRL) & 0x01) ;
	/* need to wait 16 usecs here */
	cycleCnt0 = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cycleCnt0) < 16)	;
	setParam((ulong *) AFE_REG_CTRL, (reg << 8) | 0x01);
	while (getParam((ulong *) AFE_REG_CTRL) & 0x01) ;
}

ulong readAFE(ulong reg)
{
	setParam((ulong *) AFE_REG_CTRL, (reg << 8) | 0x01);
	while (getParam((ulong *) AFE_REG_CTRL) & 0x01) ;
	return getParam((ulong *) AFE_REG_DATA);
}

#endif /* 0 && CONFIG_BCM963x8 */

static __inline void writeAdslEnum(int offset, int value)
{
    volatile unsigned int *penum = ((unsigned int *)ADSL_ENUM_BASE);
    penum[offset] = value;
    return;
}

#define RESET_STALL \
    do { int _stall_count = 20; while (_stall_count--) ; } while (0)

#define RESET_ADSL_CORE \
    writeAdslEnum(ADSL_CORE_RESET, 0x1); \
    RESET_STALL;						 \
    writeAdslEnum(ADSL_CORE_RESET, 0x0); \
    RESET_STALL

#define ENABLE_ADSL_CORE \
    writeAdslEnum(ADSL_CORE_RESET, 0x0); \
    RESET_STALL

#ifndef CONFIG_BCM96368
#define ENABLE_ADSL_MIPS \
    writeAdslEnum(ADSL_MIPS_RESET, 0x2)
#else
#define ENABLE_ADSL_MIPS \
    writeAdslEnum(ADSL_MIPS_RESET, 0x0)
#endif
    
#define DISABLE_ADSL_MIPS \
    writeAdslEnum(ADSL_MIPS_RESET, 0x3)

static AC_BOOL AdslCoreLoadImage(void)
{
	volatile ulong	*pAdslLMem = (ulong *) ADSL_LMEM_BASE;

#ifdef ADSL_PHY_FILE2
	if (!AdslFileReadFile("/etc/adsl/adsl_lmem.bin", pAdslLMem))
		return AC_FALSE;
	{
	void *pSdramImage;

	pSdramImage = AdslCoreSetSdramImageAddr(pAdslLMem[2], 0);
	return (AdslFileReadFile("/etc/adsl/adsl_sdram.bin", pSdramImage) != 0);
	}
#elif defined(ADSL_PHY_FILE)
	return (AdslFileLoadImage("/etc/adsl/adsl_phy.bin", pAdslLMem, NULL) != 0);
#else
	/* Copying ADSL core program to LMEM and SDRAM */
#ifndef  ADSLDRV_LITTLE_ENDIAN
	BlockByteMove ((sizeof(adsl_lmem)+0xF) & ~0xF, (void *)adsl_lmem, (uchar *) pAdslLMem);
#else
	for (tmp = 0; tmp < ((sizeof(adsl_lmem)+3) >> 2); tmp++)
		pAdslLMem[tmp] = ADSL_ENDIAN_CONV_LONG(((ulong *)adsl_lmem)[tmp]);
#endif

	AdslCoreSetSdramImageAddr(((ulong *) adsl_lmem)[2], sizeof(adsl_sdram));
	BlockByteMove (AdslCoreGetSdramImageSize(), (void *)adsl_sdram, AdslCoreGetSdramImageStart());
	return AC_TRUE;
#endif
}

AdslOemSharedData		adslOemDataSave;
Boolean					adslOemDataSaveFlag = AC_FALSE;

static int StrCmp(char *s1, char *s2, int n)
{
	while (n > 0) {
		if (*s1++ != *s2++)
			return 1;
		n--;
	}
	return 0;
}

static int Str2Num(char *s, char **psEnd)
{
	int		n = 0;

	while ((*s >= '0') && (*s <= '9'))
		n = n*10 + (*s++ - '0');

	*psEnd = s;
	return n;
}

static char *StrSkip(char *pVer)
{
	while ((*pVer == ' ') || (*pVer == '\t') || (*pVer == '_'))
		pVer++;
	return pVer;
}

static char *AdslCoreParseStdCaps(char *pVer, adslPhyInfo *pInfo)
{
	if ('2' == pVer[0]) {
		ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAdsl2);
		ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAdslReAdsl2);
		if ('p' == pVer[1]) {
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAdsl2p);
			pVer++;
		}
		pVer++;
	}
	return pVer;
}

static int AdslCoreParseVersionString(char *sVer, adslPhyInfo *pInfo)
{
	static char adslVerStrAnchor[] = "Version";
	char  *pVer = sVer;

	while (StrCmp(pVer, adslVerStrAnchor, sizeof(adslVerStrAnchor)-1) != 0) {
		pVer++;
		if (0 == *pVer)
			return 0;
	}

	pVer += sizeof(adslVerStrAnchor)-1;
	pVer = StrSkip(pVer);
	if (0 == *pVer)
		return 0;

	pInfo->pVerStr	= pVer;
	pInfo->chipType	= kAdslPhyChipUnknown;
	switch (*pVer) {
		case 'a':
		case 'A':
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAnnexA);
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyG992p2Init);
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyT1P413);
			if ('0' == pVer[1]) {
				pInfo->chipType	= kAdslPhyChip6345;
				pVer += 2;
			}
			else
				pVer = AdslCoreParseStdCaps(pVer+1, pInfo);
			break;

		case 'B':
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAnnexB);
			pVer = AdslCoreParseStdCaps(pVer+1, pInfo);
			break;

		case 'S':
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhySADSL);
			pVer += 1;
			break;

		case 'I':
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAnnexI);
		case 'C':
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyAnnexC);
			ADSL_PHY_SET_SUPPORT(pInfo, kAdslPhyG992p2Init);
			pVer += 1;
			break;
	}

	pVer = StrSkip(pVer);
	if (0 == *pVer)
		return 0;

	if (kAdslPhyChipUnknown == pInfo->chipType) {
		switch (*pVer) {
			case 'a':
			case 'A':
				pInfo->chipType	= kAdslPhyChip6345 | (pVer[1] - '0');
				pVer += 2;
				break;
			case 'b':
			case 'B':
				pInfo->chipType	= kAdslPhyChip6348 | (pVer[1] - '0');
				pVer += 2;
				break;
		}
	}

	pVer = StrSkip(pVer);
	if (0 == *pVer)
		return 0;

	pInfo->mjVerNum = Str2Num(pVer, &pVer);
	if (0 == *pVer)
		return 1;

	if ((*pVer >= 'a') && (*pVer <= 'z')) {
		int		n;

		pInfo->mnVerNum = (*pVer - 'a' + 1) * 100;
		n = Str2Num(pVer+1, &pVer);
		pInfo->mnVerNum += n;
	}
	return 1;
}

void AdslCoreExtractPhyInfo(AdslOemSharedData *pOemData, adslPhyInfo *pInfo)
{
	char				*pVer;
	int					i;
	Boolean				bPhyFeatureSet;
	dslCommandStruct	cmd;

	pInfo->fwType		= 0;
	pInfo->chipType		= kAdslPhyChipUnknown;
	pInfo->mjVerNum		= 0;
	pInfo->mnVerNum		= 0;
	pInfo->pVerStr		= NULL;
	for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]);i++)
		pInfo->features[i]	= 0;

	if (NULL == pAdslOemData)
		return;

	if (NULL == (pVer = AdslCoreGetVersionString()))
		return;

	AdslCoreParseVersionString(pVer, pInfo);

	bPhyFeatureSet = false;
	for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]); i++)
		if (pAdslXface->gfcTable[i] != 0) {
			bPhyFeatureSet = true;
			break;
		}

	if (bPhyFeatureSet) {
		for (i = 0; i < sizeof(pInfo->features)/sizeof(pInfo->features[0]); i++) {
			pInfo->features[i] = pAdslXface->gfcTable[i];
			pAdslXface->gfcTable[i] = 0;
		}

		cmd.command = kDslAtmVcMapTableChanged;
		cmd.param.value = 0;
		AdslCoreCommandHandler(&cmd);
	}
}

void AdslCoreSaveOemData(void)
{
	adslOemDataSaveFlag = false;
	if (NULL != pAdslOemData) {
		BlockByteMove ((sizeof(adslOemDataSave) + 3) & ~0x3, (void *)pAdslOemData, (void *) &adslOemDataSave);
		adslOemDataSaveFlag = true;
		AdslDrvPrintf(TEXT("Saving OEM data from 0x%X\n"), (int) pAdslOemData);
	}
}

void AdslCoreRestoreOemData(void)
{
	if ((NULL != pAdslOemData) && adslOemDataSaveFlag) {
		BlockByteMove ((FLD_OFFSET(AdslOemSharedData,g994RcvNonStdInfo) + 3) & ~0x3, (void *) &adslOemDataSave, (void *)pAdslOemData);
		AdslDrvPrintf(TEXT("Restoring OEM data from 0x%X\n"), (int) pAdslOemData);
	}
}

void AdslCoreProcessOemDataAddrMsg(dslStatusStruct *status)
{
	dslCommandStruct	cmd;

	pAdslOemData = (void *) status->param.value;
	AdslCoreExtractPhyInfo(pAdslOemData, &adslCorePhyDesc);
	AdslCoreRestoreOemData();
	adslOemDataSaveFlag = AC_FALSE;
#ifdef G992P3
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
		if (NULL != pAdslOemData->clEocBufPtr)
			pAdslOemData->clEocBufPtr = ADSL_ADDR_TO_HOST(pAdslOemData->clEocBufPtr);
		G997SetTxBuffer(gDslVars, pAdslOemData->clEocBufLen, pAdslOemData->clEocBufPtr);
	}
#endif

	cmd.command = kDslFilterSNRMarginCmd;
	cmd.param.value = 0xFF;
	AdslCoreCommandHandler(&cmd);
}

void AdslCoreStop(void)
{
	AdslCoreIndicateLinkDown();
	AdslCoreSaveOemData();
	writeAdslEnum(ADSL_INTMASK_I, 0);
#ifndef CONFIG_BCM96368		
	writeAdslEnum(ADSL_INTMASK_F, 0);
#endif
	DISABLE_ADSL_MIPS;
	RESET_ADSL_CORE;
}

//#define BCM6348_ADSL_MIPS_213MHz
#define BCM6348_ADSL_MIPS_240MHz

#if defined(CONFIG_BCM96348)

#define	PLL_VALUE(n1,n2,m1r,m2r,m1c,m1b,m2b)	(n1<<20) | (n2<<15) | (m1r<<12) | (m2r<<9) | (m1c<<6) | (m1b<<3) | m2b

void AdslCoreSetPllClockA0(void)
{
	ulong	reg, cnt;

#if defined(BCM6348_ADSL_MIPS_213MHz) || defined(BCM6348_ADSL_MIPS_240MHz)
	*((ulong volatile *) 0xFFFE0038) = 0x0073B548;
#else
	*((ulong volatile *) 0xFFFE0038) = 0x0073F5C8;
#endif
	reg = *((ulong volatile *) 0xFFFE0028);
	*((ulong volatile *) 0xFFFE0028) = reg & ~0x00000400;
	cnt = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cnt) < 16) ;
	*((ulong volatile *) 0xFFFE0028) = reg |  0x00000400;
#if defined(BCM6348_ADSL_MIPS_213MHz) || defined(BCM6348_ADSL_MIPS_240MHz)
	*((ulong volatile *) 0xFFFE3F94) = 0x80;
#endif
}

void AdslCoreSetPllClockA1(void)
{
#if defined(BCM6348_ADSL_MIPS_213MHz) || defined(BCM6348_ADSL_MIPS_240MHz)
	ulong	reg, cnt;

#ifdef BCM6348_ADSL_MIPS_213MHz
	*((ulong volatile *) 0xFFFE0038) = PLL_VALUE(3,7, 3,2, 2, 0,0);
#endif
#ifdef BCM6348_ADSL_MIPS_240MHz
	*((ulong volatile *) 0xFFFE0038) = PLL_VALUE(5,7, 3,2, 3, 0,2);
#endif

	reg = *((ulong volatile *) 0xFFFE0028);
	*((ulong volatile *) 0xFFFE0028) = reg & ~0x00000400;
	cnt = BcmAdslCoreGetCycleCount();
	while (TmElapsedUs(cnt) < 16) ;
	*((ulong volatile *) 0xFFFE0028) = reg |  0x00000400;

#ifdef BCM6348_ADSL_MIPS_213MHz
	*((ulong volatile *) 0xFFFE3F94) = 0x80;
#endif
#ifdef BCM6348_ADSL_MIPS_240MHz
	*((ulong volatile *) 0xFFFE3F94) = 0x8C;
#endif

#endif
}

#endif /* CONFIG_BCM96348 */

void AdslCoreSetPllClock(void)
{
	ulong	reg, chipId;

#if defined(__KERNEL__) || defined(_CFE_)
    reg = PERF->RevID;
#else
	reg = *((ulong volatile *) 0xFFFE0000);
#endif
	chipId = reg >> 16;

#if defined(CONFIG_BCM96338) || defined(CONFIG_BCM96358)
    if ((chipId == 0x6338) || (chipId == 0x6358)) {
		*((ulong volatile *) (ADSL_ENUM_BASE + 0xF94)) = 0x294;
	}
#endif

#if defined(CONFIG_BCM96348)
	if ((reg & 0xFFFF0000) == 0x63480000) {
	  if (reg >= 0x634800A1)
		AdslCoreSetPllClockA1();
	  else
		AdslCoreSetPllClockA0();
	}
#endif
}

#ifdef	SDRAM_HOLD_COUNTERS
#define	rd_shift			11
#define	rt_shift			16
#define	rs_shift			21

#define INSTR_CODE(code)	#code
#define GEN_MFC_INSTR(code,rd)	__asm__ __volatile__( ".word\t" INSTR_CODE(code) "\n\t" : : : "$" #rd)
#define GEN_MTC_INSTR(code)		__asm__ __volatile__( ".word\t" INSTR_CODE(code) "\n\t")

#define opcode_MTC0			0x40800000
#define opcode_MFC0			0x40000000

#define MTC0_SEL(rd,rt,sel)	GEN_MTC_INSTR(opcode_MTC0 | (rd << rd_shift) | (rt << rt_shift) | (sel))
#define MFC0_SEL(rd,rt,sel)	GEN_MFC_INSTR(opcode_MFC0 | (rd << rd_shift) | (rt << rt_shift) | (sel), rt)


void AdslCoreStartSdramHoldCounters(void)
{
	__asm__ __volatile__("li\t$6,0x80000238\n\t" : :: "$6");
	MTC0_SEL(25,6, 6);

	__asm__ __volatile__("li\t  $7,0x7\n\t" : :: "$7");
	/* __asm__ __volatile__("li\t  $7,0x8\n\t" : :: "$7"); */
	__asm__ __volatile__("sll\t $8, $7, 2\n\t" : :: "$8");
	__asm__ __volatile__("sll\t $9, $7, 18\n\t" : :: "$9");
	__asm__ __volatile__("li\t  $10, 0x80008100\n\t" : :: "$10");
	__asm__ __volatile__("or\t  $10, $10, $8\n\t" : :: "$10");
	__asm__ __volatile__("or\t  $10, $10, $9\n\t" : :: "$10");

	MTC0_SEL(25,0, 0);
	MTC0_SEL(25,0, 1);
	MTC0_SEL(25,0, 2);
	MTC0_SEL(25,0, 3);

	MTC0_SEL(25,10,4);

/*
li    $6, 0x8000_0238
mtc0    $6, $25, 6        // select sys_events to test_mux

li    $7, 0x7                // pick a value between [15:6], that corresponds to sys_event[9:0]
sll    $8, $7, 2
sll    $9, $7, 18
li    $10, 0x8000_8100     // counter1 counts active HIGH, counter0 counts positive edge
or    $10, $10, $8
or    $10, $10, $9

mtc0    $0, $25, 0        // initialize counters to ZEROs
mtc0    $0, $25, 1
mtc0    $0, $25, 2
mtc0    $0, $25, 3
 
mtc0    $10, $25, 4    // start counting
*/
}
 
ulong AdslCoreReadtSdramHoldEvents(void)
{
	MFC0_SEL(25,2,0);
}

ulong AdslCoreReadtSdramHoldTime(void)
{
	MFC0_SEL(25,2,1);
}

void AdslCorePrintSdramCounters(void)
{
	BcmAdslCoreDiagWriteStatusString("SDRAM Hold: Events = %d, Total Time = %d", 
		AdslCoreReadtSdramHoldEvents(),
		AdslCoreReadtSdramHoldTime());
}
#endif /* SDRAM_HOLD_COUNTERS */

#ifdef ADSLPHY_CHIPTEST_SUPPORT
#define	HW_RESET_DELAY	0
#else
#define	HW_RESET_DELAY	1000000
#endif
static AC_BOOL __AdslCoreHwReset(AC_BOOL bCoreReset)
{
#ifndef BCM_CORE_NO_HARDWARE
	volatile ulong			*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	volatile AdslXfaceData	*pAdslX = (AdslXfaceData *) ADSL_LMEM_XFACE_DATA;
	ulong				to, tmp, cycleCnt0;

	AdslCoreIndicateLinkDown();
	if (!adslOemDataSaveFlag)
		AdslCoreSaveOemData();

	flattenAddrAdj = FLATTEN_ADDR_ADJUST - 0x19000000;

	if (bCoreReset) {
		/* take ADSL core out of reset */
		RESET_ADSL_CORE;

		if (!(tmp = AdslCoreLoadImage())) {
			AdslDrvPrintf(TEXT("AdslCoreHwReset:  Failed to load ADSL PHY image (%ld)\n"), tmp);
			return AC_FALSE;
		}
	}

	{
	extern void FlattenBufferClearStat(void);
	FlattenBufferClearStat();
	}

	BlockByteClear (sizeof(AdslXfaceData), (void *)pAdslX);
	pAdslX->sdramBaseAddr = (void *) adslCorePhyDesc.sdramImageAddr;
	AdslCoreSharedMemInit();

	/* now take ADSL core MIPS out of reset */
#ifdef VXWORKS
	if (ejtagEnable) {
		int *p = (int *)0xfff0001c;
		*p = 1;
	}
#endif

	pAdslXface = (AdslXfaceData *) pAdslX;
	pAdslOemData = NULL;

	/* clear and enable interrupts */

	tmp = pAdslEnum[ADSL_INTSTATUS_I];
	pAdslEnum[ADSL_INTSTATUS_I] = tmp;
	tmp = pAdslEnum[ADSL_INTMASK_I];
	pAdslEnum[ADSL_INTMASK_I] = tmp & ~MSGINT_MASK_BIT;
	
#ifndef CONFIG_BCM96368	
	tmp = pAdslEnum[ADSL_INTSTATUS_F];
	pAdslEnum[ADSL_INTSTATUS_F] = tmp;
	pAdslEnum[ADSL_INTMASK_F] = 0;	
#endif

	ENABLE_ADSL_MIPS;

#ifdef VXWORKS
	if (ejtagEnable) {
		int read(int fd, void *buf, int n);

		int *p = (int *)0xfff0001c;
		char ch[16];
		printf("Enter any key (for EJTAG)\n");
		read(0, ch, 1);
		*p = 0;
	}
#endif

	/* wait for ADSL core to initialize */

#if 0
	while (0x55 != (tmp = pAdslEnum[ADSL_HOSTMESSAGE]))
		;
#endif

	cycleCnt0 = BcmAdslCoreGetCycleCount();
#if defined(ADSLPHY_CHIPTEST_SUPPORT)
	while (!AdsCoreStatBufAssigned() && (pAdslEnum[ADSL_HOSTMESSAGE] != 1) && (TmElapsedUs(cycleCnt0) < HW_RESET_DELAY) ) ;
	if (!AdsCoreStatBufAssigned() && (pAdslEnum[ADSL_HOSTMESSAGE] != 1))
#else
	while (!AdsCoreStatBufAssigned() && ((to = TmElapsedUs(cycleCnt0)) < 1000000)) ;
	if (!AdsCoreStatBufAssigned())
#endif
	{
		AdslDrvPrintf(TEXT("AdslCoreHwReset:  ADSL PHY initialization timeout %lu usec\n"), to);
#if 1
		{
		volatile ulong	*pAdslRes = (ulong *) ADSL_LMEM_BASE;
		AdslDrvPrintf(TEXT("AdslCoreHwReset:  TestRes=0x%lX\n"), *pAdslRes);
		}
#endif
#ifndef PHY_BLOCK_TEST
		DISABLE_ADSL_MIPS;
		return AC_FALSE;
#endif		
	}

	cycleCnt0 = BcmAdslCoreGetCycleCount();
#if 1 || defined(ADSLPHY_CHIPTEST_SUPPORT)
	if (pAdslEnum[ADSL_HOSTMESSAGE] != 1)
#endif
	{
	  dslStatusStruct status;

	  do {
		status.code = 0;
		if ((tmp = AdslCoreStatusRead (&status)) != 0) {
			AdslCoreStatusReadComplete (tmp);

			if (kDslOemDataAddrStatus == status.code) {
				AdslDrvPrintf(TEXT("AdslCoreHwReset: AdslOemDataAddr = 0x%lX\n"), status.param.value);
				AdslCoreProcessOemDataAddrMsg(&status);
				break;
			}
			else
				status.code = 0;
		}
	  } while ((to = TmElapsedUs(cycleCnt0)) < HW_RESET_DELAY);
	  
	  if (kDslOemDataAddrStatus != status.code) {
		AdslDrvPrintf(TEXT("AdslCoreHwReset:  ADSL PHY OemData status read timeout %lu usec\n"), to);
#ifndef PHY_BLOCK_TEST
		DISABLE_ADSL_MIPS;
		return AC_FALSE;
#endif		
	  }
	}

	tmp = pAdslEnum[ADSL_INTMASK_I];
	pAdslEnum[ADSL_INTMASK_I] = tmp | MSGINT_MASK_BIT;

#endif
	return AC_TRUE;
}

AC_BOOL AdslCoreHwReset(AC_BOOL bCoreReset)
{
	int		cnt = 0;

	do {
	  if (__AdslCoreHwReset(bCoreReset))
		return AC_TRUE;
	  cnt++;
	} while (cnt < 3);
	return AC_FALSE;
}

AC_BOOL AdslCoreInit(void)
{
	volatile ulong	*pAdslEnum;
	volatile ulong	*pAdslLMem;

#ifndef BCM_CORE_NO_HARDWARE
	if (NULL != pAdslXface)
		return AC_FALSE;
#endif

	acDslVars.dataPumpCommandHandlerPtr = AdslCoreCommonCommandHandler;
	acDslVars.externalStatusHandlerPtr  = AdslCoreStatusHandler;
	DslFrameAssignFunctions (acDslVars.DslFrameFunctions, DslFrameNative);
	AdslCoreAssignDslFrameFunctions (acDslVars.DslFrameFunctions);

#ifdef G997_1_FRAMER
	{
		upperLayerFunctions		g997UpperLayerFunctions;

		AdslCoreG997Init();

		g997UpperLayerFunctions.rxIndicateHandlerPtr = AdslCoreG997IndicateRecevice;
		g997UpperLayerFunctions.txCompleteHandlerPtr = AdslCoreG997SendComplete;
		g997UpperLayerFunctions.statusHandlerPtr	 = AdslCoreG997StatusHandler;
		G997Init(gDslVars,0,  32, 128, 8, &g997UpperLayerFunctions, AdslCoreG997CommandHandler);
	}
#endif
#ifdef ADSL_MIB
	AdslMibInit(gDslVars,AdslCoreCommonCommandHandler);
	AdslMibSetNotifyHandler(gDslVars, AdslCoreMibNotify);
#endif
#ifdef G992P3
#ifdef G992P3_DEBUG
	G992p3OvhMsgInit(gDslVars, 0, G997ReturnFrame, TstG997SendFrame, __AdslCoreG997SendComplete, AdslCoreCommonCommandHandler, AdslCoreCommonStatusHandler);
#else
	G992p3OvhMsgInit(gDslVars, 0, G997ReturnFrame, G997SendFrame, __AdslCoreG997SendComplete, AdslCoreCommonCommandHandler, AdslCoreCommonStatusHandler);
#endif

#endif

	adslCoreLOMTime = -1;

	pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	pAdslLMem = (ulong *) ADSL_LMEM_BASE;

#ifdef ADSL_SELF_TEST
	{
	ulong	stRes;

	AdslSelfTestRun(adslCoreSelfTestMode);
	stRes = AdslCoreGetSelfTestResults();
	AdslDrvPrintf (TEXT("AdslSelfTestResults = 0x%X %s\n"), stRes, 
		(stRes == (kAdslSelfTestCompleted | adslCoreSelfTestMode)) ? "PASSED" : "FAILED");
	}
#endif

	AdslCoreSetPllClock();
	return AdslCoreHwReset(AC_TRUE);
}

void AdslCoreUninit(void)
{
#ifndef BCM_CORE_NO_HARDWARE
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;

	pAdslEnum[ADSL_MIPS_RESET] = 0x1;
	pAdslEnum[ADSL_CORE_RESET] = 0x1;
#endif

#ifdef G997_1_FRAMER
	G997Close(gDslVars);
#endif
	pAdslXface = NULL;
}

AC_BOOL AdslCoreSetSDRAMBaseAddr(void *pAddr)
{
	if (NULL != pAdslXface)
		return AC_FALSE;

	adslCorePhyDesc.sdramPageAddr = 0xA0000000 | (((ulong) pAddr) & ~(ADSL_PHY_SDRAM_PAGE_SIZE-1));
	return AC_TRUE;
}

void * AdslCoreGetSDRAMBaseAddr()
{
	return (void *) adslCorePhyDesc.sdramPageAddr;
}

AC_BOOL AdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp)
{
	dslCommandStruct	cmd;

	if ((NULL == pAdslXface) || (gfc <= 0) || (gfc > 15))
		return AC_FALSE;

#ifndef  ADSLDRV_LITTLE_ENDIAN
	pAdslXface->gfcTable[gfc-1] = (vpi << 20) | ((vci & 0xFFFF) << 4) | (pti_clp & 0xF);
#else
	{
	ulong	tmp = (vpi << 20) | ((vci & 0xFFFF) << 4) | (pti_clp & 0xF);
	pAdslXface->gfcTable[gfc-1] = ADSL_ENDIAN_CONV_LONG(tmp);
	}
#endif

	cmd.command = kDslAtmVcMapTableChanged;
	cmd.param.value = gfc;
	AdslCoreCommandHandler(&cmd);

	return AC_TRUE;
}

void AdslCoreSetStatusCallback(void *pCallback)
{
	if (NULL == pCallback)
		pAdslCoreStatusCallback = AdslCoreIdleStatusCallback;
	else
		pAdslCoreStatusCallback = pCallback;
}

void *AdslCoreGetStatusCallback(void)
{
	return (AdslCoreIdleStatusCallback == pAdslCoreStatusCallback ? NULL : pAdslCoreStatusCallback);
}

void AdslCoreSetShowtimeMarginMonitoring(long showtimeMargin, int monEnabled, long lomTimeSec)
{
	AdslMibSetShowtimeMargin(gDslVars, showtimeMargin);
	adslCoreShMarginMonEnabled = (monEnabled != 0);
	adslCoreLOMTimeout = ((ulong) lomTimeSec) * 1000;
}

void AdslCoreG997FrameBufferFlush(void)
{
	char* frameReadPtr;
    int bufLen;         
    frameReadPtr=(char*)AdslCoreG997FrameGet(&bufLen);
	
	while(frameReadPtr!=NULL){	
		AdslCoreG997ReturnFrame(NULL); 
		frameReadPtr=(char*)AdslCoreG997FrameGet(&bufLen);				
		BcmAdslCoreDiagWriteStatusString("G997Frame returned to free-pool as it was not being read");
	}
	timeUpdate=0;
	pendingFrFlag=0;
}

void AdslCoreTimer (ulong tMs)
{
	if(pendingFrFlag){
		timeUpdate+=tMs;
		if (timeUpdate > 5000)
		  AdslCoreG997FrameBufferFlush();
	}

	if (pSdramReserved->timeCnt < ADSL_INIT_TIME) {
		pSdramReserved->timeCnt += tMs;
		if (pSdramReserved->timeCnt >= ADSL_INIT_TIME) {
			dslCommandStruct	cmd;

			adslCoreEcUpdateMask |= kDigEcShowtimeFastUpdateDisabled;
			AdslDrvPrintf (TEXT("AdslCoreEcUpdTmr: timeMs=%d ecUpdMask=0x%X\n"), pSdramReserved->timeCnt, adslCoreEcUpdateMask);

			cmd.command = kDslSetDigEcShowtimeUpdateModeCmd;
			cmd.param.value = kDslSetDigEcShowtimeUpdateModeSlow;
			AdslCoreCommandHandler(&cmd);
		}
	}

#ifdef G997_1_FRAMER
	G997Timer(gDslVars, tMs);
#endif
#ifdef ADSL_MIB
	AdslMibTimer(gDslVars, tMs);
	if (adslCoreLOMTime >= 0)
		adslCoreLOMTime += tMs;
#endif
#ifdef G992P3
	if (AdslCoreOvhMsgSupported(gDslVars))
		G992p3OvhMsgTimer(gDslVars, tMs);
#endif

#ifdef CONFIG_BCM963x8
	gTimeToWarn += tMs;
	if( gTimeToWarn > kMemPrtyWarnTime ) {
		gTimeToWarn = 0;
		if( (*(ulong *)kMemPrtyRegAddr &  kMemEnPrtySetMsk) != kMemEnPrtySetMsk ) {
			AdslDrvPrintf(TEXT("AdslDrv: WARNING!!! Memory Priority is not correctly set!\n"));
			__SoftDslPrintf(gDslVars,"AdslDrv: WARNING!!! Memory Priority is not correctly set!\n",0);
		}
	}
#endif	

	if( 2 == AdslMibPowerState(gDslVars) ) {
		gTimeInL2Ms += tMs;
		if(gTimeInL2Ms >= gL2SwitchL0TimeMs) {
#ifdef DEBUG_L2_RET_L0
			bcmOsGetTime(&gTimeInL2Ms);
			printk("L2 -> L0: %d Sec\n",	((gTimeInL2Ms-gL2Start)*BCMOS_MSEC_PER_TICK)/1000);
#endif			
			AdslCoreSetL0();
			gTimeInL2Ms = 0;
		}
	}
		
}

AC_BOOL AdslCoreIntHandler(void)
{
#ifndef BCM_CORE_NO_HARDWARE
	{
		volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
		ulong		tmp;

		tmp = pAdslEnum[ADSL_INTSTATUS_I];
		pAdslEnum[ADSL_INTSTATUS_I] = tmp;
		
		if( !(tmp & MSGINT_BIT) )
			return 0;
	}
#endif

	return AdslCoreStatusAvail ();
}

extern dslCommandStruct	adslCoreConnectionParam;
static ulong			lastStatTimeMs = 0;
#ifdef SDRAM_HOLD_COUNTERS
static ulong			lastSdramHoldTimeMs = 0;
#endif

void Adsl2UpdateCOErrCounter(dslStatusStruct *status)
{
  adslMibInfo *pMib;
  ulong mibLen;
  ulong *counters=(ulong *)( status->param.dslConnectInfo.buffPtr);

  mibLen=sizeof(adslMibInfo);
  pMib=(void *) AdslMibGetObjectValue(gDslVars,NULL,0,NULL,&mibLen);
  counters[kG992ShowtimeNumOfFEBE]=pMib->adslStat.xmtStat.cntSFErr;
  counters[kG992ShowtimeNumOfFECC]=pMib->adslStat.xmtStat.cntRSCor;
  counters[kG992ShowtimeNumOfFHEC]=pMib->atmStat.xmtStat.cntHEC;
}

int AdslCoreIntTaskHandler(int nMaxStatus)
{
	ulong              	tmp, tMs;
	dslStatusStruct	status;
	int                 	loop = 0;
	
	tMs = lastStatTimeMs;
	if (!AdslCoreCommandIsPending())
		AdslCoreSharedMemFree(NULL);
	
	tmp = AdslCoreStatusRead (&status);
	
	while (tmp > 0) {
		if (AdslMibIsAdsl2Mod(gDslVars) && (status.code == kDslConnectInfoStatus) && (status.param.dslConnectInfo.code == kG992ShowtimeMonitoringStatus))
		  Adsl2UpdateCOErrCounter(&status);

		tMs = (*pAdslCoreStatusCallback) (&status, AdslCoreStatusReadPtr(), tmp-4);
		
#ifdef ADSL_MIB
		AdslMibStatusSnooper(gDslVars, &status);

		if (adslCoreShMarginMonEnabled) {
			adslMibInfo		*pMibInfo;
			ulong			mibLen;
			dslCommandStruct	cmd;
			
			mibLen = sizeof(adslMibInfo);
			pMibInfo = (void *) AdslMibGetObjectValue (gDslVars, NULL, 0, NULL, &mibLen);
			
			if (pMibInfo->adslPhys.adslCurrStatus & kAdslPhysStatusLOM) {
				/* AdslDrvPrintf (TEXT("LOM detected = 0x%lX\n"), pMibInfo->adslPhys.adslCurrStatus); */
				if (adslCoreLOMTime < 0)
					adslCoreLOMTime = 0;
				if (adslCoreLOMTime > adslCoreLOMTimeout) {
					adslCoreLOMTime = -1;

					cmd.command = kDslStartRetrainCmd;
					AdslCoreCommandHandler(&cmd);
				}
			}
			else
				adslCoreLOMTime = -1;
		}
#endif
#ifdef G997_1_FRAMER
		G997StatusSnooper (gDslVars, &status);
#endif
#ifdef G992P3
		if (AdslCoreOvhMsgSupported(gDslVars))
			G992p3OvhMsgStatusSnooper (gDslVars, &status);
#endif

		switch (status.code) {
			case kDslEscapeToG994p1Status:
#if 0
				AdslCoreHwReset(AC_FALSE);
				AdslCoreCommandHandler(&adslCoreConnectionParam);
#endif
				break;

			case kDslOemDataAddrStatus:
				AdslDrvPrintf(TEXT("Regular AdslOemDataAddr = 0x%lX\n"), status.param.value);
				AdslCoreProcessOemDataAddrMsg(&status);
				break;

			case kAtmStatus:
				if (kAtmStatBertResult == status.param.atmStatus.code) {
					ulong	nBits;

					nBits = AdslMibBertContinueEx(gDslVars, 
						status.param.atmStatus.param.bertInfo.totalBits,
						status.param.atmStatus.param.bertInfo.errBits);
					if (nBits != 0)
						AdslCoreBertStart(nBits);
				}
				break;
		}
		AdslCoreStatusReadComplete (tmp);

		if( ++loop >= nMaxStatus )
			break;

		tmp = AdslCoreStatusRead (&status);
	}

	if (tMs != lastStatTimeMs) {
		AdslCoreTimer(tMs - lastStatTimeMs);
		lastStatTimeMs = tMs;
	}

#ifdef SDRAM_HOLD_COUNTERS
	if ((tMs - lastSdramHoldTimeMs) > 2000) {
		AdslDrvPrintf(TEXT("SDRAM Hold: Time=%d ms, HoldEvents=%d, HoldTime=%d\n"), 
			tMs - lastSdramHoldTimeMs,
			AdslCoreReadtSdramHoldEvents(),
			AdslCoreReadtSdramHoldTime());
		AdslCoreStartSdramHoldCounters();
		lastSdramHoldTimeMs = tMs;
	}
#endif

	return (loop < nMaxStatus) ? 0:1;

}

AC_BOOL AdslCoreLinkState ()
{
	return AdslMibIsLinkActive(gDslVars);
}

int	AdslCoreLinkStateEx ()
{
	return AdslMibTrainingState(gDslVars);
}

void AdslCoreGetConnectionRates (AdslCoreConnectionRates *pAdslRates)
{
    pAdslRates->fastUpRate = AdslMibGetGetChannelRate (gDslVars, kAdslXmtDir, kAdslFastChannel);
    pAdslRates->fastDnRate = AdslMibGetGetChannelRate (gDslVars, kAdslRcvDir, kAdslFastChannel);
    pAdslRates->intlUpRate = AdslMibGetGetChannelRate (gDslVars, kAdslXmtDir, kAdslIntlChannel);
    pAdslRates->intlDnRate = AdslMibGetGetChannelRate (gDslVars, kAdslRcvDir, kAdslIntlChannel);

#if !defined(__KERNEL__) && !defined(TARG_OS_RTEMS) && !defined(_CFE_)
    if (0 == pAdslRates->intlDnRate) {
            pAdslRates->intlDnRate = pAdslRates->fastDnRate;
            pAdslRates->fastDnRate = 0;
    }
    if (0 == pAdslRates->intlUpRate) {
            pAdslRates->intlUpRate = pAdslRates->fastUpRate;
            pAdslRates->fastUpRate = 0;
    }
#endif
}

int	AdslCoreSetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	return AdslMibSetObjectValue (gDslVars, objId, objIdLen, dataBuf, dataBufLen);
}

int	AdslCoreGetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	return AdslMibGetObjectValue (gDslVars, objId, objIdLen, dataBuf, dataBufLen);
}

AC_BOOL AdslCoreBertStart(ulong totalBits)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagStartBERT;
	cmd.param.value = totalBits;
	return AdslCoreCommandHandler(&cmd);
}

AC_BOOL AdslCoreBertStop(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagStopBERT;
	return AdslCoreCommandHandler(&cmd);
}

void AdslCoreBertStartEx(ulong bertSec)
{
	ulong nBits;

	AdslMibBertStartEx(gDslVars, bertSec);
	nBits = AdslMibBertContinueEx(gDslVars, 0, 0);
	if (nBits != 0)
		AdslCoreBertStart(nBits);
}

void AdslCoreBertStopEx(void)
{
	AdslMibBertStopEx(gDslVars);
}

void AdslCoreResetStatCounters(void)
{
#ifdef ADSL_MIB
	AdslMibResetConectionStatCounters(gDslVars);
#endif
}


void * AdslCoreGetOemParameterData (int paramId, int **ppLen, int *pMaxLen)
{
	int		maxLen = 0,i;
	ulong	*pLen = NULL;
	char	*pData = NULL;

	switch (paramId) {
		case kDslOemG994VendorId:
			pLen = &pAdslOemData->g994VendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->g994VendorId;
			__SoftDslPrintf(gDslVars,"AdslCoreGetOemParameter paramId:%d pLen:%d",0,paramId,pAdslOemData->g994VendorIdLen);
			for(i=0;i<*pLen;i++)
				__SoftDslPrintf(gDslVars,"AdslCoreGetOemParameter pData[i]=%c",0,pData[i]);
			break;
		case kDslOemG994XmtNSInfo:
			pLen = &pAdslOemData->g994XmtNonStdInfoLen;
			maxLen = kAdslOemNonStdInfoMaxSize;
			pData = pAdslOemData->g994XmtNonStdInfo;
			break;
		case kDslOemG994RcvNSInfo:
			pLen = &pAdslOemData->g994RcvNonStdInfoLen;
			maxLen = kAdslOemNonStdInfoMaxSize;
			pData = pAdslOemData->g994RcvNonStdInfo;
			break;
		case kDslOemEocVendorId:
			pLen = &pAdslOemData->eocVendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->eocVendorId;
			break;
		case kDslOemEocVersion:
			pLen = &pAdslOemData->eocVersionLen;
			maxLen=  kAdslOemVersionMaxSize;
			pData = pAdslOemData->eocVersion;
			break;
		case kDslOemEocSerNum:
			pLen = &pAdslOemData->eocSerNumLen;
			maxLen= kAdslOemSerNumMaxSize;
			pData = pAdslOemData->eocSerNum;
			break;
		case kDslOemT1413VendorId:
			pLen = &pAdslOemData->t1413VendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->t1413VendorId;
			break;
		case kDslOemT1413EocVendorId:
			pLen = &pAdslOemData->t1413EocVendorIdLen;
			maxLen = kAdslOemVendorIdMaxSize;
			pData = pAdslOemData->t1413EocVendorId;
			break;

	}

	*ppLen = (int *)pLen;
	*pMaxLen = maxLen;
	return pData;
}

int AdslCoreGetOemParameter (int paramId, void *buf, int len)
{
	int		maxLen, paramLen,i;
	int		*pLen;
	char	*pData;

	if (NULL == pAdslOemData)
		return 0;

	pData = AdslCoreGetOemParameterData (paramId, &pLen, &maxLen);
	__SoftDslPrintf(gDslVars,"AdslCoreGetOemParameter paramId:%d pLen:%d",0,paramId,*pLen);
	for(i=0;i<*pLen;i++)
		__SoftDslPrintf(gDslVars,"AdslCoreGetOemParameter pData[i]=%c",0,pData[i]);
	if (NULL == buf)
		return maxLen;

	paramLen = (NULL != pLen ? *pLen : 0);
	if (len > paramLen)
		len = paramLen;
	if (len > 0)
		memcpy (buf, pData, len);
	return len;
}

int AdslCoreSetOemParameter (int paramId, void *buf, int len)
{
	int		maxLen;
	int		*pLen;
	char	*pData;
	char    *str;
	str=(char *)buf;
	__SoftDslPrintf(gDslVars,"AdslCoreSetOemParameter paramId=%d len=%d buf=%c%c..%c%c",0,paramId,len,*str,*(str+1),*(str+len-2),*(str+len-1));
	if (NULL == pAdslOemData)
		return 0;

	pData = AdslCoreGetOemParameterData (paramId, &pLen, &maxLen);
	if (len > maxLen)
		len = maxLen;
	if (len > 0)
		memcpy (pData, buf, len);
	*pLen = len;
	return len;
}

char * AdslCoreGetVersionString(void)
{
	return ADSL_ADDR_TO_HOST(pAdslOemData->gDslVerionStringPtr);
}

char * AdslCoreGetBuildInfoString(void)
{
	return ADSL_ADDR_TO_HOST(pAdslOemData->gDslBuildDataStringPtr);
}

/*
**
**		ADSL_SELF_TEST functions
**
*/

int  AdslCoreGetSelfTestMode(void)
{
	return adslCoreSelfTestMode;
}

void AdslCoreSetSelfTestMode(int stMode)
{
	adslCoreSelfTestMode = stMode;
}

#ifdef ADSL_SELF_TEST

int  AdslCoreGetSelfTestResults(void)
{
	return AdslSelfTestGetResults();
}

#else  /* ADSL_SELF_TEST */

int  AdslCoreGetSelfTestResults(void)
{
	return adslCoreSelfTestMode | kAdslSelfTestCompleted;
}

#endif /* ADSL_SELF_TEST */

