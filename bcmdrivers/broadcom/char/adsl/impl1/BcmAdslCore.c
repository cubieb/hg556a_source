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
 * BcmCoreTest.c -- Bcm ADSL core driver main
 *
 * Description:
 *	This file contains BCM ADSL core driver system interface functions
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: BcmAdslCore.c,v 1.1 2008/08/25 06:40:53 l65130 Exp $
 *
 * $Log: BcmAdslCore.c,v $
 * Revision 1.1  2008/08/25 06:40:53  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:16  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:46:56  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:52  z45221
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
 * Revision 1.6  2004/07/20 23:45:48  ilyas
 * Added driver version info, SoftDslPrintf support. Fixed G.997 related issues
 *
 * Revision 1.5  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.4  2004/04/30 17:58:01  ilyas
 * Added framework for GDB communication with ADSL PHY
 *
 * Revision 1.3  2004/04/28 20:30:38  ilyas
 * Test code for GDB frame processing
 *
 * Revision 1.2  2004/04/12 23:20:03  ilyas
 * Merged RTEMS changes
 *
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#ifdef VXWORKS
#define RTOS	VXWORKS
#endif

#ifdef _WIN32_WCE
#include <windows.h>
#include <types.h>
#endif

#include "softdsl/AdslCoreDefs.h"

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#endif

/* Includes. */
#ifdef __KERNEL__
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#define	LINUX_KERNEL_2_6
#endif
#endif /* __KERNEL__ */

#include "BcmOs.h"
#if defined(CONFIG_BCM96338)
#include "6338_common.h"
#include "6338_map.h"
#elif defined(CONFIG_BCM96348)
#include "6348_common.h"
#include "6348_map.h"
#elif defined(CONFIG_BCM96358)
#include "6358_common.h"
#include "6358_map.h"
#elif defined(CONFIG_BCM96368)
#include "6368_common.h"
#include "6368_map.h"
#endif

#ifdef VXWORKS
#include "interrupt.h"
#ifdef ADSL_IRQ
#undef	INTERRUPT_ID_ADSL
#define INTERRUPT_ID_ADSL   ADSL_IRQ
#endif
#elif defined(TARG_OS_RTEMS)
#include <stdarg.h>
#include "bspcfg.h"
#ifndef BD_BCM63XX_TIMER_CLOCK_INPUT
#define BD_BCM63XX_TIMER_CLOCK_INPUT BD_BCM6345_TIMER_CLOCK_INPUT
#endif
#define FPERIPH BD_BCM63XX_TIMER_CLOCK_INPUT
#else
#include "boardparms.h"
#endif /* VXWORKS */

#include "board.h"

#if !defined(TARG_OS_RTEMS) && !defined(VXWORKS)
#include "bcm_intr.h"
#endif

#include "bcmadsl.h"
#include "BcmAdslCore.h"
#include "AdslCore.h"
#include "AdslCoreMap.h"

#define EXCLUDE_CYGWIN32_TYPES
#include "softdsl/SoftDsl.h"
#include "softdsl/SoftDsl.gh"

#include "BcmAdslDiag.h"
#include "DiagDef.h"

#include "AdslDrvVersion.h"

#ifdef HMI_QA_SUPPORT
#include "BcmXdslHmi.h"
#endif

#ifdef CONFIG_BCM96348
#define BCM6348_PLL_WORKAROUND
#endif

#undef	ADSLDRV_ENABLE_PROFILING
#undef	ADSLCORE_ENABLE_LONG_REACH
/* #define  DTAG_UR2 */

#define ADSL_MIPS_STATUS_TIMEOUT_MS		60000

#if (ADSL_OEM_G994_VENDOR_ID != kDslOemG994VendorId)
#error Inconsistent ADSL_OEM_G994_VENDOR_ID definition
#endif
#if (ADSL_OEM_G994_XMT_NS_INFO != kDslOemG994XmtNSInfo)
#error Inconsistent ADSL_OEM_G994_XMT_NS_INFO definition
#endif
#if (ADSL_OEM_G994_RCV_NS_INFO != kDslOemG994RcvNSInfo)
#error Inconsistent ADSL_OEM_G994_RCV_NS_INFOdefinition
#endif
#if (ADSL_OEM_EOC_VENDOR_ID != kDslOemEocVendorId)
#error Inconsistent ADSL_OEM_EOC_VENDOR_ID definition
#endif
#if (ADSL_OEM_EOC_VERSION != kDslOemEocVersion)
#error Inconsistent ADSL_OEM_EOC_VERSION definition
#endif
#if (ADSL_OEM_EOC_SERIAL_NUMBER != kDslOemEocSerNum)
#error Inconsistent ADSL_OEM_EOC_SERIAL_NUMBER definition
#endif
#if (ADSL_OEM_T1413_VENDOR_ID != kDslOemT1413VendorId)
#error Inconsistent ADSL_OEM_T1413_VENDOR_ID definition
#endif
#if (ADSL_OEM_T1413_EOC_VENDOR_ID != kDslOemT1413EocVendorId)
#error Inconsistent ADSL_OEM_T1413_EOC_VENDOR_ID definition
#endif
#if (ADSL_XMT_GAIN_AUTO != kDslXmtGainAuto)
#error Inconsistent ADSL_XMT_GAIN_AUTO definition
#endif


/* External vars */

extern int	g_nAdslExit;
extern long	adslCoreEcUpdateMask;
//LGD_FOR_TR098
extern unsigned long g_ShowtimeStartTicks;
#if defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)
extern int atmbonding_enabled ;
#endif


/* Local vars */

#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
LOCAL OS_SEMID	irqSemId = (OS_SEMID)  0;
LOCAL OS_TASKID	IrqTid	 = (OS_TASKID) 0;
#else
LOCAL void *	irqDpcId = NULL;
#endif

LOCAL OS_TICKS	pingLastTick, statLastTick;
LOCAL OS_TICKS	intrTicks, dpcTicks;
LOCAL long		acPendingEvents = 0;

LOCAL long		acL3Requested = 0;
LOCAL OS_TICKS	acL3StartTick = 0;

void (*bcmNotify)(void) = BcmAdsl_Notify;

dslCommandStruct	adslCoreConnectionParam;
adslCfgProfile		adslCoreCfgProfile, *pAdslCoreCfgProfile = NULL;
#ifdef G992P3
g992p3DataPumpCapabilities	g992p3Param;
#endif
#ifdef G992P5
g992p3DataPumpCapabilities	g992p5Param;
#endif

Bool			adslCoreInitialized = AC_FALSE;
Bool			adslCoreConnectionMode = AC_FALSE;
ulong			adslCoreIntrCnt		= 0;
ulong			adslCoreIsrTaskCnt	= 0;
long			adslCoreXmtGain	    = ADSL_XMT_GAIN_AUTO;
Bool			adslCoreXmtGainChanged = AC_FALSE;
#ifdef VXWORKS
Bool			adslCoreAlwaysReset = AC_FALSE;
#else
Bool			adslCoreAlwaysReset = AC_TRUE;
#endif
Bool			adslCoreStarted = AC_FALSE;
Bool			adslCoreResetPending = AC_FALSE;
ulong			adslCoreCyclesPerMs = 0;
Bool			adslCoreMuteDiagStatus = AC_FALSE;
ulong			adslCoreHsModeSwitchTime = 0;
int			gConsoleOutputEnable = 1;
Bool			gDiagDataLog = AC_FALSE;

struct {
	ulong		rcvCtrl, rcvAddr, rcvPtr;
	ulong		xmtCtrl, xmtAddr, xmtPtr;
} adslCoreDiagState;

DiagDebugData	diagDebugCmd = { 0, 0 };
#ifdef PHY_PROFILE_SUPPORT
LOCAL void BcmAdslCoreProfilingStart(void);
void BcmAdslCoreProfilingStop(void);
#endif

LOCAL void BcmAdslCoreEnableSnrMarginData(void);
LOCAL void BcmAdslCoreDisableSnrMarginData(void);
ulong BcmAdslCoreStatusSnooper(dslStatusStruct *status, char *pBuf, int len);
void BcmAdslCoreAfeTestStatus(dslStatusStruct *status);
void BcmAdslCoreSendBuffer(ulong statusCode, uchar *bufPtr, ulong bufLen);
void BcmAdslCoreSendDmaBuffers(ulong statusCode, int bufNum);
#ifdef ADSLPHY_CHIPTEST_SUPPORT
void BcmAdslCoreProcessTestCommand(void);
Bool BcmAdslCoreIsTestCommand(void);
void BcmAdslCoreDebugTimer(void);
void BcmAdslCoreDebugSendCommand(char *fileName, ushort cmd, ulong offset, ulong len, ulong bufAddr);
void BcmAdslCoreDebugPlaybackStop(void);
void BcmAdslCoreDebugPlaybackResume(void);
void BcmAdslCoreDebugReset(void);
#endif
void BcmAdslCoreGdbTask(void);
#ifdef LINUX_KERNEL_2_6
static irqreturn_t BcmCoreInterruptHandler(int irq, void * dev_id, struct pt_regs * regs);
#else
UINT32 BcmCoreInterruptHandler (void);
#endif
void BcmCoreAtmVcInit(void);
void BcmCoreAtmVcSet(void);

void BcmDiagDataLogNotify(int set)
{
#ifndef CONFIG_BCM96368
	if(set) {
		gConsoleOutputEnable = 0;
		gDiagDataLog = AC_TRUE;
	}
	else {
		gConsoleOutputEnable = 1;
		gDiagDataLog = AC_FALSE;
	}
#endif	
}

char * BcmAdslCoreDiagScrambleString(char *s)
{
	char	*p = s;

	while (*p != 0) {
		*p = ~(*p);
		p++;
	}

	return s;
}


LOCAL Bool BcmAdslCoreCanReset(void)
{
#ifdef VXWORKS
	if (adslCoreAlwaysReset)
		return 1;
	return (BcmAdslCoreStatusSnooper == AdslCoreGetStatusCallback());
#else
	return adslCoreAlwaysReset;
#endif
}

ulong BcmAdslCoreGetCycleCount(void)
{
	ulong	cnt; 
#ifdef _WIN32_WCE
	cnt = 0;
#else
	__asm volatile("mfc0 %0, $9":"=d"(cnt));
#endif
	return(cnt); 
}

ulong BcmAdslCoreCycleTimeElapsedUs(ulong cnt1, ulong cnt0)
{
	cnt1 -= cnt0;

	if (cnt1 < ((ulong) -1) / 1000)
		return (cnt1 * 1000 / adslCoreCyclesPerMs);
	else
		return (cnt1 / (adslCoreCyclesPerMs / 1000));
}

ulong BcmAdslCoreCalibrate(void)
{
	OS_TICKS	tick0, tick1;
	ulong		cnt; 

	if (adslCoreCyclesPerMs != 0)
		return adslCoreCyclesPerMs;

	bcmOsGetTime(&tick1);
	do {
		bcmOsGetTime(&tick0);
	} while (tick0 == tick1);

	cnt = BcmAdslCoreGetCycleCount();
	do {
		bcmOsGetTime(&tick1);
		tick1 = (tick1 - tick0) * BCMOS_MSEC_PER_TICK;
	} while (tick1 < 60);
	cnt = BcmAdslCoreGetCycleCount() - cnt;
	adslCoreCyclesPerMs = cnt / tick1;
	return adslCoreCyclesPerMs;
}

void BcmAdslCoreDelay(ulong timeMs)
{
	OS_TICKS	tick0, tick1;

	bcmOsGetTime(&tick0);
	do {
		bcmOsGetTime(&tick1);
		tick1 = (tick1 - tick0) * BCMOS_MSEC_PER_TICK;
	} while (tick1 < timeMs);
}

void BcmAdslCoreSetWdTimer(ulong timeUs)
{
	TIMER->WatchDogDefCount = timeUs * (FPERIPH/1000000);
	TIMER->WatchDogCtl = 0xFF00;
	TIMER->WatchDogCtl = 0x00FF;
}

/***************************************************************************
** Function Name: BcmAdslCoreCheckBoard
** Description  : Checks the presense of Bcm ADSL core
** Returns      : 1
***************************************************************************/
Bool BcmAdslCoreCheckBoard()
{
	return 1;
}

void BcmAdslCoreStop(void)
{
#ifndef CONFIG_BCM96368
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
#endif
	bcmOsGetTime(&statLastTick);
	BcmAdslDiagDisable();
#ifndef CONFIG_BCM96368	
	adslCoreDiagState.rcvAddr = pAdslEnum[RCV_ADDR_INTR];
	adslCoreDiagState.rcvCtrl = pAdslEnum[RCV_CTL_INTR];
	adslCoreDiagState.rcvPtr =  pAdslEnum[RCV_PTR_INTR];
	adslCoreDiagState.xmtAddr = pAdslEnum[XMT_ADDR_INTR];
	adslCoreDiagState.xmtCtrl = pAdslEnum[XMT_CTL_INTR];
	adslCoreDiagState.xmtPtr =  pAdslEnum[XMT_PTR_INTR];
#endif
	AdslCoreStop();
	adslCoreStarted = AC_FALSE;
}

void __BcmAdslCoreStart(int diagDataMap, Bool bRestoreImage)
{
#ifndef CONFIG_BCM96368	
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
#endif

	adslCoreResetPending = AC_FALSE;
	bcmOsGetTime(&statLastTick);
	pingLastTick = statLastTick;
	AdslCoreHwReset(bRestoreImage);
#ifndef CONFIG_BCM96368
	pAdslEnum[RCV_ADDR_INTR] = adslCoreDiagState.rcvAddr;
	pAdslEnum[RCV_CTL_INTR]  = adslCoreDiagState.rcvCtrl;
	pAdslEnum[RCV_PTR_INTR]  = adslCoreDiagState.rcvPtr;
	pAdslEnum[XMT_ADDR_INTR] = adslCoreDiagState.xmtAddr;
	pAdslEnum[XMT_CTL_INTR]  = adslCoreDiagState.xmtCtrl;
	pAdslEnum[XMT_PTR_INTR]  = adslCoreDiagState.xmtPtr;
#endif
	BcmCoreCommandHandlerSync(&adslCoreConnectionParam);

	adslCoreStarted = AC_TRUE;
}

#ifdef  BCM6348_PLL_WORKAROUND

Bool			adslCoreReverbPwrRcv = AC_FALSE;
ulong			adslCoreReverbPwr = 0;
void AdslCoreSetPllClock(void);

void BcmAdslCoreReverbPwrClear(void)
{
	adslCoreReverbPwrRcv = AC_FALSE;
	adslCoreReverbPwr = 0;
}

void BcmAdslCoreReverbPwrSet(ulong pwr)
{
	adslCoreReverbPwrRcv = AC_TRUE;
	adslCoreReverbPwr = pwr;
}

Bool BcmAdslCoreReverbPwrGet(ulong *pPwr)
{
	*pPwr = adslCoreReverbPwr;
	return adslCoreReverbPwrRcv;
}

void BcmAdslCoreHandlePLL(ulong to)
{
	ulong		pwr, tryCnt;
	OS_TICKS	tick0, tick1;

	tryCnt = 0;
	do {
		BcmAdslCoreDelay(10);
		BcmAdslCoreSetTestMode(kDslTestEstimatePllPhase);
		BcmAdslCoreDelay(300);
		BcmAdslCoreReverbPwrClear();
		BcmAdslCoreSetTestMode(kDslTestReportPllPhaseStatus);
		bcmOsGetTime(&tick0);
		do {
			bcmOsSleep (10/BCMOS_MSEC_PER_TICK);
			bcmOsGetTime(&tick1);
			tick1 = (tick1 - tick0) * BCMOS_MSEC_PER_TICK;
			if (tick1 > to)
				break;
		} while (!BcmAdslCoreReverbPwrGet(&pwr));
		if (!BcmAdslCoreReverbPwrGet(&pwr)) {
			BcmAdslCoreDiagWriteStatusString("No PLL test status");
			break;
		}

		if (pwr < 5000) {
			BcmAdslCoreDiagWriteStatusString("PLL test passed, pwr = %d", pwr);
			break;
		}
		
		BcmAdslCoreDiagWriteStatusString(
			"PLL test failed, pwr = %d\n"
			"Resetting PLL and ADSL PHY", pwr);
		AdslCoreSetPllClock();

		BcmAdslCoreStop();
		__BcmAdslCoreStart(-1, AC_TRUE);
	} while (++tryCnt < 5);
}

void BcmAdslCoreHandlePLLReset(void)
{
	if (PERF->RevID >= 0x634800A1)
		return;
	adslCoreMuteDiagStatus = AC_TRUE;
	BcmAdslCoreDiagWriteStatusString("<<<Start 6348 PLL check");
	BcmAdslCoreHandlePLL(20);
	adslCoreMuteDiagStatus = AC_FALSE;
	BcmAdslCoreDiagWriteStatusString(">>>6348 PLL check complete");
}

void BcmAdslCoreHandlePLLInit(void)
{
	if (PERF->RevID >= 0x634800A1)
		return;
	BcmCoreCommandHandlerSync(&adslCoreConnectionParam);
	BcmAdslCoreHandlePLLReset();
}

#endif

void BcmAdslCoreStart(int diagDataMap, Bool bRestoreImage)
{
#ifdef ADSLPHY_CHIPTEST_SUPPORT
	BcmAdslCoreDebugReset();
#endif
	__BcmAdslCoreStart(diagDataMap, bRestoreImage);
#if 1 && defined(BCM6348_PLL_WORKAROUND)
	BcmAdslCoreHandlePLLReset();
#endif
	// BcmAdslCoreDelay(50);
	BcmAdslDiagReset(diagDataMap);
	BcmAdslDiagEnable();
	BcmAdslCoreLogWriteConnectionParam(&adslCoreConnectionParam);
	BcmCoreAtmVcSet();
}

/***************************************************************************
** Function Name: BcmAdslCoreReset
** Description  : Completely resets ADSL MIPS 
** Returns      : None.
***************************************************************************/
void BcmAdslCoreReset(int diagDataMap)
{
	BcmAdslCoreStop();
	BcmAdslCoreStart(diagDataMap, AC_TRUE);
}

/***************************************************************************
** Function Name: BcmAdslCoreStatusSnooper
** Description  : Some DSL status processing
** Returns      : 1
***************************************************************************/
ulong BcmAdslCoreStatusSnooper(dslStatusStruct *status, char *pBuf, int len)
{
	long				val;
	adslMibInfo			*pMibInfo;
	ulong				mibLen;
	static	Bool		bCheckBertEx = false;

	bcmOsGetTime(&statLastTick);
	pingLastTick = statLastTick;

	if (!adslCoreMuteDiagStatus || (kDslExceptionStatus == status->code))
		BcmAdslCoreDiagStatusSnooper(status, pBuf, len);

	if (bCheckBertEx) {
		bCheckBertEx = false;
		mibLen = sizeof(adslMibInfo);
		pMibInfo = (void *) AdslCoreGetObjectValue (NULL, 0, NULL, &mibLen);
		if (0 == pMibInfo->adslBertStatus.bertSecCur)
			BcmAdslCoreDiagWriteStatusString(
				"BERT_EX results: totalBits=0x%08lX%08lX, errBits=0x%08lX%08lX",
				pMibInfo->adslBertStatus.bertTotalBits.cntHi, 
				pMibInfo->adslBertStatus.bertTotalBits.cntLo,
				pMibInfo->adslBertStatus.bertErrBits.cntHi, 
				pMibInfo->adslBertStatus.bertErrBits.cntLo);
	}

	switch (status->code) {
		case kDslExceptionStatus:
#if 0 || defined(ADSL_SELF_TEST)
			{
			ulong	*sp, spAddr;
			int		i, stackSize;

			AdslDrvPrintf (TEXT("DSL Exception:\n"));
			sp = (ulong*) status->param.dslException.sp;
			for (i = 0; i < 28; i += 4)
				AdslDrvPrintf (TEXT("R%d=0x%08lX\tR%d=0x%08lX\tR%d=0x%08lX\tR%d=0x%08lX\n"),
					i+1, sp[i], i+2, sp[i+1], i+3, sp[i+2], i+4, sp[i+3]);
			AdslDrvPrintf (TEXT("R29=0x%08lX\tR30=0x%08lX\tR31=0x%08lX\n"), sp[28], sp[29], sp[30]);

			sp = (ulong*) status->param.dslException.argv;
			AdslDrvPrintf (TEXT("argv[0] (EPC) = 0x%08lX\n"), sp[0]);
			for (i = 1; i < status->param.dslException.argc; i++)
				AdslDrvPrintf (TEXT("argv[%d]=0x%08lX\n"), i, sp[i]);
			AdslDrvPrintf (TEXT("Exception stack dump:\n"));

			sp = (ulong*) status->param.dslException.sp;
			spAddr = sp[28];
#ifdef FLATTEN_ADDR_ADJUST
			sp = (ulong *) (spAddr | FLATTEN_ADDR_ADJUST);
			stackSize = 64;
#else
			sp = (ulong *) status->param.dslException.stackPtr;
			stackSize = status->param.dslException.stackLen;
#endif
			for (i = 0; i < stackSize; i += 8)
				{
				AdslDrvPrintf (TEXT("%08lX: %08lX %08lX %08lX %08lX %08lX %08lX %08lX %08lX\n"),
					spAddr + (i*4), sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6], sp[7]);
				sp += 8;
				}
			}
#endif
			if (BcmAdslCoreCanReset())
				adslCoreResetPending = AC_TRUE;
			else
				adslCoreConnectionMode = AC_FALSE;
			break;
		case kDslTrainingStatus:
			val = status->param.dslTrainingInfo.value;
			switch (status->param.dslTrainingInfo.code) {
				case kDslG992p2RxShowtimeActive:
					BcmAdslCoreEnableSnrMarginData();
					break;
				case kDslG994p1RcvNonStandardInfo:
					BcmAdslCoreNotify(ACEV_G994_NONSTDINFO_RECEIVED);
					break;
				default:
					break;
			}
			break;

		case kDslShowtimeSNRMarginInfo:
			if (status->param.dslShowtimeSNRMarginInfo.nCarriers != 0)
				BcmAdslCoreDisableSnrMarginData();
			break;
		case kDslDataAvailStatus:
			BcmAdslCoreAfeTestStatus(status);
			break;
		case kAtmStatus: 
			switch (status->param.atmStatus.code) {
				case kAtmStatBertResult:
					bCheckBertEx = true;
					break;
				default:
					break;
			}
			break;
#ifdef BCM6348_PLL_WORKAROUND
		case kDslTestPllPhaseResult:
			BcmAdslCoreReverbPwrSet(status->param.value);
			break;
#endif
		default:
			break;
	}
	return statLastTick * BCMOS_MSEC_PER_TICK;
}

/***************************************************************************
** Function Name: BcmCoreInterruptHandler
** Description  : Interrupt service routine that is called when there is an
**                core ADSL PHY interrupt.  Signals event to the task to
**                process the interrupt condition.
** Returns      : 1
***************************************************************************/
#ifdef LINUX_KERNEL_2_6
static irqreturn_t BcmCoreInterruptHandler(int irq, void * dev_id, struct pt_regs * regs)
#else
UINT32 BcmCoreInterruptHandler (void)
#endif
{
	Bool	bRunTask;
	
#ifdef	ADSLDRV_ENABLE_PROFILING
	OS_TICKS	tick0, tick1;
	bcmOsGetTime(&tick0);
#endif

	adslCoreIntrCnt++;
	bRunTask = BcmAdslCoreDiagIntrHandler();
	bRunTask |= AdslCoreIntHandler();

#ifdef ADSLPHY_CHIPTEST_SUPPORT
	if (BcmAdslCoreIsTestCommand())
		bRunTask = AC_TRUE;		
#endif

	if (bRunTask)
#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
		bcmOsSemGive (irqSemId);
#else
		bcmOsDpcEnqueue(irqDpcId);
#endif	
	
#ifndef BCM_CORE_TEST
	BcmHalInterruptEnable (INTERRUPT_ID_ADSL);
#endif

#ifdef ADSLDRV_ENABLE_PROFILING
	bcmOsGetTime(&tick1);
	intrTicks += (tick1 - tick0);
#endif

#ifdef LINUX_KERNEL_2_6
	return IRQ_HANDLED;
#else
	return 1;
#endif
}

/***************************************************************************
** Function Name: BcmCoreDpc
** Description  : Processing of ADSL PHY interrupt 
** Returns      : None.
***************************************************************************/
LOCAL void BcmCoreDpc(void * arg)
{
	int	nMaxStatus, n;
#ifdef ADSLDRV_ENABLE_PROFILING
	OS_TICKS	tick0, tick1;
	bcmOsGetTime(&tick0);
#endif

	if( g_nAdslExit == 1 )
		return;

	adslCoreIsrTaskCnt++;
	BcmAdslCoreDiagIsrTask();
	BcmAdslCoreGdbTask();
	
#if defined( __KERNEL__)
 #ifdef ADSLPHY_CHIPTEST_SUPPORT
 	nMaxStatus = 2;
 #else
	if( gDiagDataLog )
		nMaxStatus = 2;
	else
		nMaxStatus = 0xffff;
 #endif
#else
	nMaxStatus = 0xffff;
#endif	
	n = AdslCoreIntTaskHandler(nMaxStatus);
	
#if defined( __KERNEL__)
	if(n != 0)
		bcmOsDpcEnqueue(irqDpcId);
#endif
	
#ifdef ADSLPHY_CHIPTEST_SUPPORT
	BcmAdslCoreProcessTestCommand();
#endif
#ifdef ADSLDRV_ENABLE_PROFILING
	bcmOsGetTime(&tick1);
	dpcTicks += (tick1 - tick0);
#endif
}

#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
/***************************************************************************
** Function Name: BcmCoreIsrTask
** Description  : Runs in a separate thread of execution. Returns from blocking
**                on an event when an core ADSL PHY interrupt.  
** Returns      : None.
***************************************************************************/
LOCAL void BcmCoreIsrTask(void)
{
	while (TRUE) {
		bcmOsSemTake(irqSemId, OS_WAIT_FOREVER);

        if( g_nAdslExit == 1 )
            break;

		BcmCoreDpc(NULL);
	}
}
#endif

#if defined(G992P3) || defined(G992P5)
int BcmAdslCoreLogSaveAdsl2Capabilities(long *cmdData, g992p3DataPumpCapabilities *pCap)
{
	cmdData[0]  = (long)pCap->rcvNTREnabled;
	cmdData[1]  = (long)pCap->shortInitEnabled;
	cmdData[2]  = (long)pCap->diagnosticsModeEnabled;
	cmdData[3]  = (long)pCap->featureSpectrum;
	cmdData[4]  = (long)pCap->featureOverhead;
	cmdData[5]  = (long)pCap->featureTPS_TC[0];
	cmdData[6]  = (long)pCap->featurePMS_TC[0];
	cmdData[7]  = (long)pCap->featureTPS_TC[1];
	cmdData[8]  = (long)pCap->featurePMS_TC[1];
	cmdData[9]  = (long)pCap->featureTPS_TC[2];
	cmdData[10] = (long)pCap->featurePMS_TC[2];
	cmdData[11] = (long)pCap->featureTPS_TC[3];
	cmdData[12] = (long)pCap->featurePMS_TC[3];
	cmdData[13] = (long)pCap->readsl2Upstream;
	cmdData[14] = (long)pCap->readsl2Downstream;
	cmdData[15] = (long)pCap->sizeIDFT;
	cmdData[16] = (long)pCap->fillIFFT;
	cmdData[17] = (long)pCap->minDownOverheadDataRate;
	cmdData[18] = (long)pCap->minUpOverheadDataRate;
	cmdData[19] = (long)pCap->maxDownATM_TPSTC;
	cmdData[20] = (long)pCap->maxUpATM_TPSTC;
	cmdData[21] = (long)pCap->minDownATM_TPS_TC[0];
	cmdData[22] = (long)pCap->maxDownATM_TPS_TC[0];
	cmdData[23] = (long)pCap->minRevDownATM_TPS_TC[0];
	cmdData[24] = (long)pCap->maxDelayDownATM_TPS_TC[0];
	cmdData[25] = (long)pCap->maxErrorDownATM_TPS_TC[0];
	cmdData[26] = (long)pCap->minINPDownATM_TPS_TC[0];
	cmdData[27] = (long)pCap->minUpATM_TPS_TC[0];
	cmdData[28] = (long)pCap->maxUpATM_TPS_TC[0];
	cmdData[29] = (long)pCap->minRevUpATM_TPS_TC[0];
	cmdData[30] = (long)pCap->maxDelayUpATM_TPS_TC[0];
	cmdData[31] = (long)pCap->maxErrorUpATM_TPS_TC[0];
	cmdData[32] = (long)pCap->minINPUpATM_TPS_TC[0];
	cmdData[33] = (long)pCap->maxDownPMS_TC_Latency[0];
	cmdData[34] = (long)pCap->maxUpPMS_TC_Latency[0];
	return 35;
}
#endif

void BcmAdslCoreLogWriteConnectionParam(dslCommandStruct *pDslCmd)
{
	long	cmdData[256];
	int		n;

	if (0 == (n = AdslCoreFlattenCommand(pDslCmd, cmdData, sizeof(cmdData))))
		return;
	n >>= 2;

#ifdef	G992P3
#ifdef	G992P5
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p))
		n--;		/* undo the pointer to carrierInfoG992p5AnnexA */
#endif
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
		n--;		/* undo the pointer to carrierInfoG992p3AnnexA */
#endif

#ifdef	G992P3
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2))
		n += BcmAdslCoreLogSaveAdsl2Capabilities(cmdData+n, pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA);
#endif
#ifdef	G992P5
	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p))
		n += BcmAdslCoreLogSaveAdsl2Capabilities(cmdData+n, pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA);
#endif
	BcmAdslCoreDiagWriteLog(commandInfoData, cmdData, n);
}

/***************************************************************************
** Function Name: BcmAdslCoreSetConnectionParam
** Description  : Sets default connection parameters
** Returns      : None.
***************************************************************************/
LOCAL void BcmAdslCoreSetConnectionParam(dslCommandStruct *pDslCmd, adslCfgProfile *pAdslCfg)
{
	ulong	phyCfg, modCfg;

#if 1
	if (NULL != pAdslCfg) {
		AdslDrvPrintf (TEXT("AnnexCParam=0x%08X AnnexAParam=0x%08X adsl2=0x%08X\n"),
			(int)pAdslCfg->adslAnnexCParam, (int)pAdslCfg->adslAnnexAParam, (int) pAdslCfg->adsl2Param);
	}
	else
		AdslDrvPrintf (TEXT("BcmAdslCoreSetConnectionParam: no profile\n"));
#endif

	if (NULL != pAdslCfg) {
#ifdef G992_ANNEXC
		phyCfg = pAdslCfg->adslAnnexCParam;
#else
		phyCfg = pAdslCfg->adslAnnexAParam;
#endif
		adslCoreHsModeSwitchTime = pAdslCfg->adslHsModeSwitchTime;
		modCfg = phyCfg & kAdslCfgModMask;
	}
	else {
		phyCfg = 0;
		modCfg = kAdslCfgModAny;
	}

	pDslCmd->command = kDslStartPhysicalLayerCmd;
	pDslCmd->param.dslModeSpec.direction = kATU_R;
	pDslCmd->param.dslModeSpec.capabilities.modulations = kG994p1;
	pDslCmd->param.dslModeSpec.capabilities.minDataRate = 1;
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 48;
	pDslCmd->param.dslModeSpec.capabilities.features = 0;
	pDslCmd->param.dslModeSpec.capabilities.auxFeatures = (0x00
				| kG994p1PreferToExchangeCaps
				| kG994p1PreferToDecideMode
				);
	pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 = 0;
#if 1
	if ((NULL != pAdslCfg) && (phyCfg & kAdslCfgPwmSyncClockOn))
		AfePwmSetSyncClockFreq(pDslCmd->param.dslModeSpec.capabilities.auxFeatures, pAdslCfg->adslPwmSyncClockFreq);
#else
		AfePwmSetSyncClockFreq(pDslCmd->param.dslModeSpec.capabilities.auxFeatures, 7);
#endif
	pDslCmd->param.dslModeSpec.capabilities.demodCapabilities = (
				kSoftwareTimeErrorDetectionEnabled |
				kHardwareAGCEnabled |
				kDigitalEchoCancellorEnabled |
				kHardwareTimeTrackingEnabled);

	if((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask))
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |=  kDslBitSwapEnabled;

	if (phyCfg & kAdslCfgExtraMask) {
		if (kAdslCfgTrellisOn == (phyCfg & kAdslCfgTrellisMask))
			pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslTrellisEnabled;
		if (0 == (phyCfg & kAdslCfgTrellisMask))
			pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslTrellisEnabled;
		if (kAdslCfgLOSMonitoringOff == (phyCfg & kAdslCfgLOSMonitoringMask))
			pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslAutoRetrainDisabled;

		pDslCmd->param.dslModeSpec.capabilities.noiseMargin = 
			(kAdslCfgDefaultTrainingMargin != pAdslCfg->adslTrainingMarginQ4) ? 
				pAdslCfg->adslTrainingMarginQ4 : 0x60;
		AdslCoreSetShowtimeMarginMonitoring(
			pAdslCfg->adslShowtimeMarginQ4, 
			phyCfg & kAdslCfgMarginMonitoringOn ? AC_TRUE : AC_FALSE, 
			pAdslCfg->adslLOMTimeThldSec);
	}
	else {
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslTrellisEnabled;
		pDslCmd->param.dslModeSpec.capabilities.noiseMargin = 0x60;
	}

#ifdef G992P1

#ifdef G992P1_ANNEX_A	
  if (ADSL_PHY_SUPPORT(kAdslPhyAnnexA)) {
	pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslG994AnnexAMultimodeEnabled;
	pDslCmd->param.dslModeSpec.capabilities.modulations &= ~(kG994p1 | kT1p413);
	if (kAdslCfgModAny == (modCfg & kAdslCfgModMask))
		modCfg = kAdslCfgModGdmtOnly | kAdslCfgModGliteOnly | kAdslCfgModT1413Only;
#ifdef G994P1
	if (modCfg & (kAdslCfgModGdmtOnly | kAdslCfgModGliteOnly))
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG994p1;
	if (modCfg & kAdslCfgModGdmtOnly)
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p1AnnexA;
#endif
#ifdef T1P413
	if (ADSL_PHY_SUPPORT(kAdslPhyT1P413) && (modCfg & kAdslCfgModT1413Only))
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kT1p413 /* | kG992p1AnnexA */;
#endif
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 255;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMinCarr = 33;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMaxCarr = 254;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMinCarr = 6;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMaxCarr = 31;
	pDslCmd->param.dslModeSpec.capabilities.features |= (kG992p1ATM | kG992p1RACK1);
#if 1
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 400;
	pDslCmd->param.dslModeSpec.capabilities.features |= kG992p1HigherBitRates;
#endif
	if((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask))
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslAturXmtPowerCutbackEnabled;
	pDslCmd->param.dslModeSpec.capabilities.subChannelInfo = (
				kSubChannelLS0Upstream | kSubChannelASODownstream);
#ifdef G992P1_ANNEX_A_USED_FOR_G992P2
	if (ADSL_PHY_SUPPORT(kAdslPhyG992p2Init)) {
		if (modCfg & kAdslCfgModGliteOnly)
			pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p2AnnexAB;
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMinCarr = 33;
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMaxCarr = 126;
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMinCarr = 6;
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMaxCarr = 31;
		pDslCmd->param.dslModeSpec.capabilities.features |= kG992p2RACK1;
	}
#endif
  }
#endif /* G992P1_ANNEX_A */

#ifdef G992P1_ANNEX_B
  if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB) || ADSL_PHY_SUPPORT(kAdslPhySADSL)) {
  	pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslG994AnnexAMultimodeEnabled;
	if (kAdslCfgModAny == (modCfg & kAdslCfgModMask))
		modCfg = kAdslCfgModGdmtOnly | kAdslCfgModGliteOnly | kAdslCfgModT1413Only;
	pDslCmd->param.dslModeSpec.capabilities.modulations &= ~(kG994p1 | kT1p413);
	pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p1AnnexB;
	if (modCfg & (kAdslCfgModGdmtOnly | kAdslCfgModGliteOnly))
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG994p1;
	if (modCfg & kAdslCfgModT1413Only)
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kT1p413;
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 255;
#ifdef SADSL_DEFINES
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr = 59;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr = 254;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr = 7;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr = 58;
#else
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr = 61;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr = 254;
#ifdef DTAG_UR2
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr = 33;
#else
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr = 28;
#endif
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr = 59;
#endif
	pDslCmd->param.dslModeSpec.capabilities.features |= (kG992p1AnnexBATM | kG992p1AnnexBRACK1) ; 
	pDslCmd->param.dslModeSpec.capabilities.features |= kG992p1AnnexBUpstreamTones1to32;
#if 1
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 400;
	pDslCmd->param.dslModeSpec.capabilities.features |= kG992p1HigherBitRates;
#endif
	if((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask))
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslAturXmtPowerCutbackEnabled;

	pDslCmd->param.dslModeSpec.capabilities.subChannelInfoAnnexB = (
				kSubChannelLS0Upstream | kSubChannelASODownstream);
#ifdef G992P1_ANNEX_A_USED_FOR_G992P2
	if (ADSL_PHY_SUPPORT(kAdslPhyG992p2Init)) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p2AnnexAB;
		pDslCmd->param.dslModeSpec.capabilities.features |= kG992p2RACK1;
	}
#endif
  }
#endif	/* G992P1_ANNEX_B */

#ifdef G992_ANNEXC
	if (kAdslCfgModAny == (modCfg & kAdslCfgModMask))
		modCfg = kAdslCfgModGdmtOnly | kAdslCfgModGliteOnly;
	pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p1AnnexC;
	if (modCfg & kAdslCfgModGliteOnly)
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p2AnnexC;
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 400;
	/* pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 255; */

	if (pAdslCfg && (pAdslCfg->adslAnnexCParam & kAdslCfgCentilliumCRCWorkAroundMask))
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslCentilliumCRCWorkAroundEnabled;
#if	1
#ifndef G992P1_ANNEX_I
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr = 33;
#else
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr = 6;
#endif

	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr = 254;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr = 6;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr = 31;
	pDslCmd->param.dslModeSpec.capabilities.features |= (kG992p1ATM | kG992p1RACK1);
	if (pAdslCfg && (kAdslCfgFBM == (pAdslCfg->adslAnnexCParam & kAdslCfgBitmapMask)))
		pDslCmd->param.dslModeSpec.capabilities.features |= kG992p1AnnexCDBM;

	pDslCmd->param.dslModeSpec.capabilities.features |= kG992p1HigherBitRates;
	pDslCmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC = (
				kSubChannelLS0Upstream | kSubChannelASODownstream);
#if defined(ADSLCORE_ENABLE_LONG_REACH) && defined(G992_ANNEXC_LONG_REACH) 
	pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslAnnexCPilot48 | kDslAnnexCReverb33_63; 
#endif

#else
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr = 33;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr = 254;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr = 6;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr = 29;
	pDslCmd->param.dslModeSpec.capabilities.features |= (kG992p1ATM | kG992p1RACK1) <<
				(kG992p1AnnexCFeaturesNPar2Shift - kG992p1FeaturesNPar2Shift);
	pDslCmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC = (
				kSubChannelLS0Upstream | kSubChannelASODownstream);
#endif
#endif /* G992_ANNEXC */
#ifdef G992P1_ANNEX_I
	pDslCmd->param.dslModeSpec.capabilities.auxFeatures &= ~kG994p1PreferToDecideMode;
	pDslCmd->param.dslModeSpec.capabilities.auxFeatures |= kG994p1PreferToMPMode;
	pDslCmd->param.dslModeSpec.capabilities.modulations |= (kG992p1AnnexI>>4);
	/* pDslCmd->param.dslModeSpec.capabilities.modulations &= ~kG992p2AnnexC; */
	pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 1023;
	/* pDslCmd->param.dslModeSpec.capabilities.maxDataRate = 255; */

	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr = 33;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMaxCarr = 511;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMinCarr = 6;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr = 31;

	if (NULL == pAdslCfg)
		modCfg |= kAdslCfgUpstreamMax;

	if (kAdslCfgUpstreamMax == (modCfg & kAdslCfgUpstreamModeMask))
		modCfg |= kAdslCfgUpstreamDouble;

	if (modCfg & kAdslCfgUpstreamDouble)
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr = 
			(modCfg & kAdslCfgNoSpectrumOverlap) ? 62 : 63;
	else
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr = 31;

	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr = 
		(modCfg & kAdslCfgNoSpectrumOverlap ? 
					pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr + 2 :
					7);

	pDslCmd->param.dslModeSpec.capabilities.subChannelInfoAnnexI = (
				kSubChannelLS0Upstream | kSubChannelASODownstream);
#endif
#ifdef G992P3
  if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2))	
	{
	g992p3DataPumpCapabilities	*pG992p3Cap = pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA;
	ulong						adsl2Cfg;

	if (NULL == pAdslCfg)
		modCfg = kAdslCfgModAny;
	else
		modCfg = pAdslCfg->adslAnnexAParam & kAdslCfgModMask;

	if (kAdslCfgModAny == (modCfg & kAdslCfgModMask)) {
		modCfg   = kAdslCfgModMask;	/* all enabled */
		adsl2Cfg = kAdsl2CfgReachExOn /* | kAdsl2CfgAnnexMEnabled */;
	}
	else
		adsl2Cfg = pAdslCfg->adsl2Param;

	if (modCfg & kAdslCfgModAdsl2Only) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p3AnnexA | kG994p1;
		if (adsl2Cfg & kAdsl2CfgAnnexMEnabled) {
			pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p3AnnexM;
			pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 = (adsl2Cfg & kAdsl2CfgAnnexMPsdMask) >> kAdsl2CfgAnnexMPsdShift;
		}
	}

	pG992p3Cap->rcvNTREnabled = 0;
	pG992p3Cap->shortInitEnabled = 0;
	pG992p3Cap->diagnosticsModeEnabled = 0;
	pG992p3Cap->featureSpectrum = 0x10;
	pG992p3Cap->featureOverhead = 0x0f;
	pG992p3Cap->featureTPS_TC[0] = 0x0C;
	pG992p3Cap->featurePMS_TC[0] = 0x03;
	pG992p3Cap->featureTPS_TC[1] = 0;
	pG992p3Cap->featurePMS_TC[1] = 0;
	pG992p3Cap->featureTPS_TC[2] = 0;
	pG992p3Cap->featurePMS_TC[2] = 0;
	pG992p3Cap->featureTPS_TC[3] = 0;
	pG992p3Cap->featurePMS_TC[3] = 0;
	
#if	0	/* def	G992_APPLY_SSVI */ /* do not init unless CO wants it in CL */
	pG992p3Cap->featureSpectrum |= kG994p1G992p3AnnexASpectrumShapingUpstream;
	pG992p3Cap->numUsSubcarrier = 7;
	pG992p3Cap->usSubcarrierIndex[0] = 0;
	pG992p3Cap->usLog_tss[0] = -24;		/* -12dB */
	pG992p3Cap->usSubcarrierIndex[1] = 4;
	pG992p3Cap->usLog_tss[1] = -24;
	pG992p3Cap->usSubcarrierIndex[2] = 5;
	pG992p3Cap->usLog_tss[2] = -6;
	pG992p3Cap->usSubcarrierIndex[3] = 6;
	pG992p3Cap->usLog_tss[3] = 0;
	pG992p3Cap->usSubcarrierIndex[4] = 24;
	pG992p3Cap->usLog_tss[4] = 0;
	pG992p3Cap->usSubcarrierIndex[5] = 25;
	pG992p3Cap->usLog_tss[5] = -24;
	pG992p3Cap->usSubcarrierIndex[6] = 63;
	pG992p3Cap->usLog_tss[6] = -24;
#endif

#ifdef	READSL2
	if (ADSL_PHY_SUPPORT(kAdslPhyAdslReAdsl2) && (modCfg & kAdslCfgModAdsl2Only) && (adsl2Cfg & kAdsl2CfgReachExOn)) {
		pG992p3Cap->featureSpectrum |= kG994p1G992p3AnnexLReachExtended;
		if (0 == (adsl2Cfg & kAdsl2CfgAnnexLMask))
			adsl2Cfg |= kAdsl2CfgAnnexLUpWide | kAdsl2CfgAnnexLUpNarrow;
		pG992p3Cap->readsl2Upstream   = 0;
		pG992p3Cap->readsl2Downstream = kG994p1G992p3AnnexLDownNonoverlap;
		if (adsl2Cfg & kAdsl2CfgAnnexLUpWide)
			pG992p3Cap->readsl2Upstream |= kG994p1G992p3AnnexLUpWideband;
		if (adsl2Cfg & kAdsl2CfgAnnexLUpNarrow)
			pG992p3Cap->readsl2Upstream |= kG994p1G992p3AnnexLUpNarrowband;
		if (adsl2Cfg & kAdsl2CfgAnnexLDnOvlap)
			pG992p3Cap->readsl2Downstream &= ~kG994p1G992p3AnnexLDownNonoverlap;
	}
#endif

	pG992p3Cap->sizeIDFT = 8;
	pG992p3Cap->fillIFFT = 2;
	pG992p3Cap->minDownOverheadDataRate = 3;
	pG992p3Cap->minUpOverheadDataRate = 3;
	pG992p3Cap->maxDownATM_TPSTC = 1;
	pG992p3Cap->maxUpATM_TPSTC = 1;
	pG992p3Cap->minDownATM_TPS_TC[0] = 1;
#if (defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)) && !defined(NO_BITRATE_LIMIT)
        if (atmbonding_enabled == 1)
        {
            //pG992p3Cap->maxDownATM_TPS_TC[0] = 3050;
	         pG992p3Cap->maxDownATM_TPS_TC[0] = 3800 ;
        }
        else
        {
	         pG992p3Cap->maxDownATM_TPS_TC[0] = 4095;
        }
#else
	pG992p3Cap->maxDownATM_TPS_TC[0] = 4095;
#endif          
	pG992p3Cap->minRevDownATM_TPS_TC[0] = 8;
	pG992p3Cap->maxDelayDownATM_TPS_TC[0] = 32;
	pG992p3Cap->maxErrorDownATM_TPS_TC[0] = 2;
	pG992p3Cap->minINPDownATM_TPS_TC[0] = 0;
	pG992p3Cap->minUpATM_TPS_TC[0] = 1;
	pG992p3Cap->maxUpATM_TPS_TC[0] = 2000;
	pG992p3Cap->minRevUpATM_TPS_TC[0] = 8;
	pG992p3Cap->maxDelayUpATM_TPS_TC[0] = 20;
	pG992p3Cap->maxErrorUpATM_TPS_TC[0] = 2;
	pG992p3Cap->minINPUpATM_TPS_TC[0] = 0;
	pG992p3Cap->maxDownPMS_TC_Latency[0] = 4095;
	pG992p3Cap->maxUpPMS_TC_Latency[0] = 4095;
	}
#endif /* G992P3 */
#ifdef G992P5
  if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p))
	{
	g992p3DataPumpCapabilities	*pG992p3Cap = pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA;
	ulong						adsl2Cfg;

	if (NULL == pAdslCfg)
		modCfg = kAdslCfgModAny;
	else
		modCfg = pAdslCfg->adslAnnexAParam & kAdslCfgModMask;

	if (kAdslCfgModAny == (modCfg & kAdslCfgModMask)) {
		modCfg   = kAdslCfgModMask;	/* all enabled */
		adsl2Cfg = kAdsl2CfgReachExOn /* | kAdsl2CfgAnnexMEnabled */;
	}
	else
		adsl2Cfg = pAdslCfg->adsl2Param;

	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMinCarr = 33;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMaxCarr = 511;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr = 6;
	pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr = 31;
#ifdef G992P1_ANNEX_B
	if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB)) {
#ifdef DTAG_UR2
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr = 33;
#endif
		pDslCmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr = 59;
	}
#endif
	if (modCfg & kAdslCfgModAdsl2Only) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p3AnnexA | kG994p1;
		if (adsl2Cfg & kAdsl2CfgAnnexMEnabled) {
			pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p3AnnexM;
			pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 = (adsl2Cfg & kAdsl2CfgAnnexMPsdMask) >> kAdsl2CfgAnnexMPsdShift;
		}
	}
	if (modCfg & kAdslCfgModAdsl2pOnly) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p5AnnexA | kG994p1;
		if (adsl2Cfg & kAdsl2CfgAnnexMEnabled) {
			pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p5AnnexM;
			pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 = (adsl2Cfg & kAdsl2CfgAnnexMPsdMask) >> kAdsl2CfgAnnexMPsdShift;
		}
	}

	pG992p3Cap->rcvNTREnabled = 0;
	pG992p3Cap->shortInitEnabled = 0;
	pG992p3Cap->diagnosticsModeEnabled = 0;
	pG992p3Cap->featureSpectrum = 0x10;
	pG992p3Cap->featureOverhead = 0x0f;
	pG992p3Cap->featureTPS_TC[0] = 0x0C;
	pG992p3Cap->featurePMS_TC[0] = 0x03;
	pG992p3Cap->featureTPS_TC[1] = 0;
	pG992p3Cap->featurePMS_TC[1] = 0;
	pG992p3Cap->featureTPS_TC[2] = 0;
	pG992p3Cap->featurePMS_TC[2] = 0;
	pG992p3Cap->featureTPS_TC[3] = 0;
	pG992p3Cap->featurePMS_TC[3] = 0;
	
#if	0	/* def	G992_APPLY_SSVI */ /* do not init unless CO wants it in CL */
	pG992p3Cap->featureSpectrum |= kG994p1G992p3AnnexASpectrumShapingUpstream;
	pG992p3Cap->numUsSubcarrier = 7;
	pG992p3Cap->usSubcarrierIndex[0] = 0;
	pG992p3Cap->usLog_tss[0] = -24;		/* -12dB */
	pG992p3Cap->usSubcarrierIndex[1] = 4;
	pG992p3Cap->usLog_tss[1] = -24;
	pG992p3Cap->usSubcarrierIndex[2] = 5;
	pG992p3Cap->usLog_tss[2] = -6;
	pG992p3Cap->usSubcarrierIndex[3] = 6;
	pG992p3Cap->usLog_tss[3] = 0;
	pG992p3Cap->usSubcarrierIndex[4] = 24;
	pG992p3Cap->usLog_tss[4] = 0;
	pG992p3Cap->usSubcarrierIndex[5] = 25;
	pG992p3Cap->usLog_tss[5] = -24;
	pG992p3Cap->usSubcarrierIndex[6] = 63;
	pG992p3Cap->usLog_tss[6] = -24;
#endif

#ifdef	READSL2
	if (ADSL_PHY_SUPPORT(kAdslPhyAdslReAdsl2) && (modCfg & kAdslCfgModAdsl2Only) && (adsl2Cfg & kAdsl2CfgReachExOn)) {
		pG992p3Cap->featureSpectrum |= kG994p1G992p3AnnexLReachExtended;
		if (0 == (adsl2Cfg & kAdsl2CfgAnnexLMask))
			adsl2Cfg |= kAdsl2CfgAnnexLUpWide | kAdsl2CfgAnnexLUpNarrow;
		pG992p3Cap->readsl2Upstream   = 0;
		pG992p3Cap->readsl2Downstream = kG994p1G992p3AnnexLDownNonoverlap;
		if (adsl2Cfg & kAdsl2CfgAnnexLUpWide)
			pG992p3Cap->readsl2Upstream |= kG994p1G992p3AnnexLUpWideband;
		if (adsl2Cfg & kAdsl2CfgAnnexLUpNarrow)
			pG992p3Cap->readsl2Upstream |= kG994p1G992p3AnnexLUpNarrowband;
		if (adsl2Cfg & kAdsl2CfgAnnexLDnOvlap)
			pG992p3Cap->readsl2Downstream &= ~kG994p1G992p3AnnexLDownNonoverlap;
	}
#endif

	pG992p3Cap->sizeIDFT = 9;
	pG992p3Cap->fillIFFT = 2;
	pG992p3Cap->minDownOverheadDataRate = 3;
	pG992p3Cap->minUpOverheadDataRate = 3;
	pG992p3Cap->maxDownATM_TPSTC = 1;
	pG992p3Cap->maxUpATM_TPSTC = 1;
	pG992p3Cap->minDownATM_TPS_TC[0] = 1;
#if (defined(CONFIG_BCM_ATM_BONDING_ETH) || defined(CONFIG_BCM_ATM_BONDING_ETH_MODULE)) && !defined(NO_BITRATE_LIMIT)
        if (atmbonding_enabled == 1)
        {
          //pG992p3Cap->maxDownATM_TPS_TC[0] = 1525;
          pG992p3Cap->maxDownATM_TPS_TC[0] = 2600;
        }
        else
        {
	pG992p3Cap->maxDownATM_TPS_TC[0] = 4095;
        }
#else
	pG992p3Cap->maxDownATM_TPS_TC[0] = 4095;
#endif
	pG992p3Cap->minRevDownATM_TPS_TC[0] = 8;
	pG992p3Cap->maxDelayDownATM_TPS_TC[0] = 32;
	pG992p3Cap->maxErrorDownATM_TPS_TC[0] = 2;
	pG992p3Cap->minINPDownATM_TPS_TC[0] = 0;
	pG992p3Cap->minUpATM_TPS_TC[0] = 1;
	pG992p3Cap->maxUpATM_TPS_TC[0] = 2000;
	pG992p3Cap->minRevUpATM_TPS_TC[0] = 8;
	pG992p3Cap->maxDelayUpATM_TPS_TC[0] = 20;
	pG992p3Cap->maxErrorUpATM_TPS_TC[0] = 2;
	pG992p3Cap->minINPUpATM_TPS_TC[0] = 0;
	pG992p3Cap->maxDownPMS_TC_Latency[0] = 4095;
	pG992p3Cap->maxUpPMS_TC_Latency[0] = 4095;
	}
#endif /* G992P5 */

#endif /* G992P1 */

#if 1
	if ((NULL != pAdslCfg) && 
		(kAdslCfgModAny == (pAdslCfg->adslAnnexAParam & kAdslCfgModMask)) &&
		(pAdslCfg->adsl2Param & (kAdsl2CfgReachExOn | kAdsl2CfgAnnexMEnabled))) {
	  pDslCmd->param.dslModeSpec.capabilities.modulations = 0;
	  if (pAdslCfg->adsl2Param & kAdsl2CfgAnnexMEnabled) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG992p3AnnexM | kG992p5AnnexM | kG994p1;
		pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 = (pAdslCfg->adsl2Param & kAdsl2CfgAnnexMPsdMask) >> kAdsl2CfgAnnexMPsdShift;
	  }		
	  if (pAdslCfg->adsl2Param & kAdsl2CfgReachExOn ) {
		pDslCmd->param.dslModeSpec.capabilities.modulations |= kG994p1;
	  }		
	}
#endif

	if((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask) )
		pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 |= (kDslFireDsSupported | kDsl24kbyteInterleavingEnabled);

	if ((NULL != pAdslCfg) && ((phyCfg & kAdslCfgDemodCapMask)|| ~((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask)) ))  {
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= 
			pAdslCfg->adslDemodCapMask & pAdslCfg->adslDemodCapValue;
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities &= 
			~(pAdslCfg->adslDemodCapMask & ~pAdslCfg->adslDemodCapValue);
	}

	pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 |= adslCoreEcUpdateMask;

#if 0	/* enables 30sec T1.413 - G.994 switching */
	if (pDslCmd->param.dslModeSpec.capabilities.modulations & kT1p413) {
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities &= ~kDslG994AnnexAMultimodeEnabled;
		pDslCmd->param.dslModeSpec.capabilities.demodCapabilities |= kDslMultiModesPreferT1p413 | kDslEnableRoundUpDSLoopAttn;
		adslCoreHsModeSwitchTime = 60;    // mode switch timeout in seconds
	}
#endif

#if 0
	AdslDrvPrintf (TEXT("cmd.modulations =0x%08lX\n"), pDslCmd->param.dslModeSpec.capabilities.modulations);
#endif
	if ((NULL != pAdslCfg) && ((phyCfg & kAdslCfgDemodCap2Mask) || ~((phyCfg & kAdslCfgDemodCapMask) || (phyCfg & kAdslCfgDemodCap2Mask)) )) {
		pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5  |= 
			pAdslCfg->adslDemodCap2Mask & pAdslCfg->adslDemodCap2Value;
		pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5 &= 
			~(pAdslCfg->adslDemodCap2Mask & ~pAdslCfg->adslDemodCap2Value);
	}

	adslCoreCfgProfile.adslDemodCapValue = pDslCmd->param.dslModeSpec.capabilities.demodCapabilities;
	adslCoreCfgProfile.adslDemodCapMask = adslCoreCfgProfile.adslDemodCapValue;
	adslCoreCfgProfile.adslDemodCap2Value = pDslCmd->param.dslModeSpec.capabilities.subChannelInfop5;
	adslCoreCfgProfile.adslDemodCap2Mask = adslCoreCfgProfile.adslDemodCap2Value;
}

/***************************************************************************
** Function Name: BcmAdslCoreConnectionReset
** Description  : Restarts ADSL connection if core is initialized
** Returns      : None.
***************************************************************************/
void BcmAdslCoreConnectionReset(void)
{
	if (adslCoreConnectionMode) {
#if 0
		BcmAdslCoreReset(-1);
#else
		OS_TICKS	tick0, tick1;
	
		BcmAdslCoreConnectionStop();
		bcmOsGetTime(&tick0);
		do {
			bcmOsGetTime(&tick1);
			tick1 = (tick1 - tick0) * BCMOS_MSEC_PER_TICK;
		} while (tick1 < 40);
		BcmAdslCoreConnectionStart();
#endif
	}
}

void BcmAdslCoreSendXmtGainCmd(int gain)
{
	dslCommandStruct	cmd;

	cmd.command = kDslSetXmtGainCmd;
	cmd.param.value = gain;
	BcmCoreCommandHandlerSync(&cmd);
}

/**************************************************************************
** Function Name: BcmAdslCoreConfigure
** Description  : This function is called by ADSL driver change ADSL PHY
** Returns      : None.
**************************************************************************/
void BcmAdslCoreConfigure(adslCfgProfile *pAdslCfg)
{
	long	pair;

	if (NULL == pAdslCfg)
		pair = kAdslCfgLineInnerPair;
	else {
#if defined(G992P1_ANNEX_A)
		pair = pAdslCfg->adslAnnexAParam & kAdslCfgLinePairMask;
#elif defined(G992_ANNEXC)
		pair = pAdslCfg->adslAnnexCParam & kAdslCfgLinePairMask;
#else
		pair = kAdslCfgLineInnerPair;
#endif
	}
	BcmAdsl_ConfigureRj11Pair(pair);

#ifdef G992P3
	adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = &g992p3Param;
#endif
#ifdef G992P5
	adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = &g992p5Param;
#endif
	if (NULL != pAdslCfg) {
		adslCoreCfgProfile = *pAdslCfg;
		pAdslCoreCfgProfile = &adslCoreCfgProfile;
	}
	else 
		pAdslCoreCfgProfile = NULL;
	BcmAdslCoreSetConnectionParam(&adslCoreConnectionParam, pAdslCfg);
	BcmAdslCoreConnectionReset();
}


LOCAL int StrCpy(char *dst, char *src)
{
	char	*dst0 = dst;

	while (*src != 0)
		*dst++ = *src++;
	*dst = 0;
	return (dst - dst0);
}

/**************************************************************************
** Function Name: BcmAdslCoreGetVersion
** Description  : Changes ADSL version information
** Returns      : STS_SUCCESS 
**************************************************************************/
void BcmAdslCoreGetVersion(adslVersionInfo *pAdslVer)
{
	adslPhyInfo		*pInfo = AdslCoreGetPhyInfo();
	int				phyVerLen = 0, n;

	pAdslVer->phyMjVerNum = pInfo->mjVerNum;
	pAdslVer->phyMnVerNum = pInfo->mnVerNum;

	pAdslVer->phyVerStr[0] = 0;
	if (NULL != pInfo->pVerStr)
		phyVerLen = StrCpy(pAdslVer->phyVerStr, pInfo->pVerStr);
	
	if (ADSL_PHY_SUPPORT(kAdslPhyAnnexA))
		pAdslVer->phyType = kAdslTypeAnnexA;
	else if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB))
		pAdslVer->phyType = kAdslTypeAnnexB;
	else if (ADSL_PHY_SUPPORT(kAdslPhySADSL))
		pAdslVer->phyType = kAdslPhySADSL;
	else if (ADSL_PHY_SUPPORT(kAdslPhyAnnexC))
		pAdslVer->phyType = kAdslTypeAnnexC;
	else
		pAdslVer->phyType = kAdslTypeUnknown;

	pAdslVer->drvMjVerNum = ADSL_DRV_MJ_VERNUM;
	pAdslVer->drvMnVerNum = (ushort) ((ADSL_DRV_MN_VERSYM - 'a') << (16 - 5)) | ADSL_DRV_MN_VERNUM;
	StrCpy(pAdslVer->drvVerStr, ADSL_DRV_VER_STR);
#if 1
	n = StrCpy(pAdslVer->phyVerStr + phyVerLen, ".d");
	StrCpy(pAdslVer->phyVerStr + phyVerLen + n, pAdslVer->drvVerStr);
#endif
}

/**************************************************************************
** Function Name: BcmAdslCoreInit
** Description  : This function is called by ADSL driver to init core
**                ADSL PHY
** Returns      : None.
**************************************************************************/
unsigned char ctlmG994p1ControlXmtNonStdInfoMsg[] = {
    0x02, 0x12, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x50, 0xC0, 0xA8, 0x01, 0x01,
    0x03, 0x0E, 0x00, 0x00, 0x0E, 0xB5, 0x00, 0x50,
    0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x07, 0x02,
    0x09, 
#if defined(ADSLCORE_ENABLE_LONG_REACH) && defined(G992_ANNEXC_LONG_REACH)
	0x01,
#else
	0x00,
#endif
	0x00
};

void BcmAdslCoreInit(void)
{
	int				diagMap;

	BcmAdslCoreCalibrate();
	adslCoreConnectionMode = AC_FALSE;
	adslCoreMuteDiagStatus = AC_FALSE;
	acPendingEvents = 0;
	bcmOsGetTime(&statLastTick);
	intrTicks = dpcTicks = 0;
#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
	bcmOsSemCreate(NULL, &irqSemId);
#if defined(TARG_OS_RTEMS)
	bcmOsTaskCreate("DSLC", 20*1024, 255, BcmCoreIsrTask, 0, &IrqTid);
#else
	bcmOsTaskCreate("BcmCoreIrq", 20*1024, 6, BcmCoreIsrTask, 0, &IrqTid);
#endif
#else /* Linux */
	irqDpcId = bcmOsDpcCreate(BcmCoreDpc, NULL);
#endif

#ifndef BCM_CORE_TEST
	BcmHalMapInterrupt((void*)BcmCoreInterruptHandler, 0, INTERRUPT_ID_ADSL);
	BcmHalInterruptEnable (INTERRUPT_ID_ADSL);
#endif
#ifndef CONFIG_BCM96348
	PERF->blkEnables |= ADSL_CLK_EN;
#endif
	
	diagMap = BcmAdslDiagTaskInit();
	BcmCoreAtmVcInit();

	if (NULL == AdslCoreGetStatusCallback())
		AdslCoreSetStatusCallback(BcmAdslCoreStatusSnooper);

	adslCoreResetPending = AC_FALSE;
	if (!AdslCoreInit()) 
		BcmAdslCoreUninit();

	BcmAdslCoreConfigure(pAdslCoreCfgProfile);

#if 1 && defined(BCM6348_PLL_WORKAROUND)
	BcmAdslCoreHandlePLLInit();
#endif

	BcmAdslDiagInit(diagMap);

#if 0 || defined(ADSLCORE_ENABLE_LONG_REACH)
	/* BcmAdslCoreSetOemParameter (ADSL_OEM_G994_VENDOR_ID, "\x00""\x00""\x00""\xF0""\x00\x00""\x00\x00", 8); */
	BcmAdslCoreSetOemParameter (
		ADSL_OEM_G994_XMT_NS_INFO, 
		ctlmG994p1ControlXmtNonStdInfoMsg, 
		sizeof(ctlmG994p1ControlXmtNonStdInfoMsg));
#endif
#ifdef G992P1_ANNEX_I
	BcmAdslCoreSetOemParameter (ADSL_OEM_G994_VENDOR_ID, "\xB5""\x00""BDCM""\x54\x4D", 8);
	//BcmAdslCoreSetOemParameter (ADSL_OEM_G994_VENDOR_ID, "\xB5""\x00""BDCM""\x00\x01", 8);
#endif

#ifdef HMI_QA_SUPPORT
	BcmAdslCoreHmiInit();
#endif

	adslCoreInitialized = AC_TRUE;
	adslCoreStarted = AC_TRUE;
}

/**************************************************************************
** Function Name: BcmAdslCoreConnectionStart
** Description  : This function starts ADSL PHY normal connection operations
** Returns      : None.
**************************************************************************/
void BcmAdslCoreConnectionStart(void)
{
	if (!adslCoreInitialized)
		return;

	BcmAdslCoreLogWriteConnectionParam(&adslCoreConnectionParam);
	if (adslCoreXmtGainChanged) {
		adslCoreXmtGainChanged = AC_FALSE;
		BcmAdslCoreSendXmtGainCmd(adslCoreXmtGain);
	}

	BcmCoreDpcSyncEnter();
	if ((adslCoreConnectionParam.param.dslModeSpec.capabilities.modulations & kT1p413) && 
		!(adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities & kDslG994AnnexAMultimodeEnabled))
	{
		dslCommandStruct cmd;

		cmd.command = kDslSetG994p1T1p413SwitchTimerCmd;
		cmd.param.value = adslCoreHsModeSwitchTime;
		AdslCoreCommandHandler(&cmd);
	}
	
	AdslCoreCommandHandler(&adslCoreConnectionParam);
#ifdef DTAG_UR2
	{
	dslCommandStruct	cmd;
	uchar xmtToneMap[8] = {0,0,0,0, 0xFE, 0xFF, 0xFF, 0xFF};
	uchar rcvToneMap = 0xFF;


	cmd.command = kDslTestCmd;
	cmd.param.dslTestSpec.type = kDslTestToneSelection;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtStartTone = 0;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones = 60;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvStartTone = 60;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones = 8;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap = xmtToneMap;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvMap = &rcvToneMap;
	AdslCoreCommandHandler(&cmd);
	}
#endif
	adslCoreConnectionMode = AC_TRUE;
	BcmCoreDpcSyncExit();
}

/**************************************************************************
** Function Name: BcmAdslCoreConnectionStop
** Description  : This function stops ADSL PHY connection operations and 
**				  puts ADSL PHY in idle mode
** Returns      : None.
**************************************************************************/
void BcmAdslCoreConnectionStop(void)
{
	dslCommandStruct	cmd;

	if (!adslCoreInitialized)
		return;

	adslCoreConnectionMode = FALSE;
	cmd.command = kDslIdleCmd;
 	BcmCoreCommandHandlerSync(&cmd);
}

/**************************************************************************
** Function Name: BcmAdslCoreConnectionUninit
** Description  : This function disables ADSL PHY
** Returns      : None.
**************************************************************************/
void BcmAdslCoreUninit()
{
#ifndef BCM_CORE_TEST
	BcmHalInterruptDisable (INTERRUPT_ID_ADSL);
#ifdef __KERNEL__
	free_irq(INTERRUPT_ID_ADSL, 0);
#endif
#endif

	if (adslCoreInitialized)
		AdslCoreUninit();

	BcmAdslDiagTaskUninit();

#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
	if (irqSemId != 0 )
	    bcmOsSemGive (irqSemId);
#endif
	adslCoreInitialized = AC_FALSE;
}

/***************************************************************************
** Function Name: BcmAdslCoreGetConnectionInfo
** Description  : This function is called by ADSL driver to obtain
**                connection info from core ADSL PHY
** Returns      : None.
***************************************************************************/
void BcmAdslCoreGetConnectionInfo(PADSL_CONNECTION_INFO pConnectionInfo)
{
	AdslCoreConnectionRates	acRates;
	OS_TICKS				ticks;
	int 					tMs, tPingMs;
#ifdef ADSLDRV_ENABLE_PROFILING
	static OS_TICKS			printTicks = 0;
#endif

#ifdef ADSLPHY_CHIPTEST_SUPPORT
	BcmAdslCoreDebugTimer();
#endif

	if (!adslCoreConnectionMode) {
		pConnectionInfo->LinkState = BCM_ADSL_LINK_DOWN;
		pConnectionInfo->ulFastDnStreamRate = 0;
		pConnectionInfo->ulInterleavedDnStreamRate = 0;
		pConnectionInfo->ulFastUpStreamRate = 0;
		pConnectionInfo->ulInterleavedUpStreamRate = 0;
		return;
	}

	if (adslCoreResetPending) {
		pConnectionInfo->LinkState = BCM_ADSL_LINK_DOWN;
		pConnectionInfo->ulFastDnStreamRate = 0;
		pConnectionInfo->ulInterleavedDnStreamRate = 0;
		pConnectionInfo->ulFastUpStreamRate = 0;
		pConnectionInfo->ulInterleavedUpStreamRate = 0;
#ifdef __KERNEL__
		if (!in_softirq())
#endif
		{
		adslCoreResetPending = AC_FALSE;
		BcmAdslCoreReset(DIAG_DATA_EYE);
		}
		return;
	}

	if (AdslCoreLinkState()) {
		pConnectionInfo->LinkState = BCM_ADSL_LINK_UP;
		AdslCoreGetConnectionRates (&acRates);
		pConnectionInfo->ulFastDnStreamRate = acRates.fastDnRate;
		pConnectionInfo->ulInterleavedDnStreamRate = acRates.intlDnRate;
		pConnectionInfo->ulFastUpStreamRate = acRates.fastUpRate;
		pConnectionInfo->ulInterleavedUpStreamRate = acRates.intlUpRate;
	}
	else {
		pConnectionInfo->LinkState = BCM_ADSL_LINK_DOWN;
		pConnectionInfo->ulFastDnStreamRate = 0;
		pConnectionInfo->ulInterleavedDnStreamRate = 0;
		pConnectionInfo->ulFastUpStreamRate = 0;
		pConnectionInfo->ulInterleavedUpStreamRate = 0;
		switch (AdslCoreLinkStateEx()) {
			case kAdslTrainingG994:	
				pConnectionInfo->LinkState = BCM_ADSL_TRAINING_G994;
				break;
			case kAdslTrainingG992Started:
				pConnectionInfo->LinkState = BCM_ADSL_TRAINING_G992_STARTED;
				break;
			case kAdslTrainingG992ChanAnalysis:
				pConnectionInfo->LinkState = BCM_ADSL_TRAINING_G992_CHANNEL_ANALYSIS;
				break;
			case kAdslTrainingG992Exchange:
				pConnectionInfo->LinkState = BCM_ADSL_TRAINING_G992_EXCHANGE;
				break;
		}
	}

	bcmOsGetTime(&ticks);
	//LGD_FOR_TR098		
	pConnectionInfo->ShowtimeStart = (ticks - g_ShowtimeStartTicks)*BCMOS_MSEC_PER_TICK/1000;
	
#ifdef ADSLDRV_ENABLE_PROFILING
	tMs = (ticks - printTicks) * BCMOS_MSEC_PER_TICK;
	if (tMs >= 10000) {
		extern ulong diagDmaIntrCnt;

		AdslDrvPrintf(TEXT("tMs=%d: intrCnt=%d, dpcCnt=%d, dmaCnt=%d  intrMs=%d, dpcMs=%d\n"), 
			tMs, adslCoreIntrCnt, adslCoreIsrTaskCnt, diagDmaIntrCnt, 
			intrTicks * BCMOS_MSEC_PER_TICK, dpcTicks * BCMOS_MSEC_PER_TICK);
		printTicks = ticks;
	}
#endif

#ifndef PHY_BLOCK_TEST
	if (adslCoreStarted) {
		tMs = (ticks - statLastTick) * BCMOS_MSEC_PER_TICK;
		tPingMs = (ticks - pingLastTick) * BCMOS_MSEC_PER_TICK;
		if ((tMs > ADSL_MIPS_STATUS_TIMEOUT_MS) && (tPingMs > 1000)) {
			dslCommandStruct	cmd;

			BcmAdslCoreDiagWriteStatusString("ADSL MIPS inactive. Sending Ping\n");
			cmd.command = kDslPingCmd;
			BcmCoreCommandHandlerSync(&cmd);
			pingLastTick = ticks;

			if (tMs > (ADSL_MIPS_STATUS_TIMEOUT_MS + 5000)) {
				int i; ulong cycleCnt0;
				
				BcmAdslCoreDiagWriteStatusString("Resetting ADSL MIPS\n");
				for(i = 0; i < 20; i++) {
					BcmAdslCoreDiagWriteStatusString("PC = 0x%08x\n", *(volatile ulong *)(ADSL_LMEM_BASE + 0x5E8));
					cycleCnt0 = BcmAdslCoreGetCycleCount();	
					while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < 5);
				}
				
				if (BcmAdslCoreCanReset())
					adslCoreResetPending = AC_TRUE;
			}
		}
	}
#endif

	if (acL3StartTick != 0) {
		tMs = (ticks - acL3StartTick) * BCMOS_MSEC_PER_TICK;
		if (tMs > 20000) {
			acL3StartTick = 0;
			BcmAdslCoreConnectionStart();
		}
	}
}

LOCAL void BcmAdslCoreEnableSnrMarginData(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslFilterSNRMarginCmd;
	cmd.param.value = 0;
	AdslCoreCommandHandler(&cmd);
}

LOCAL void BcmAdslCoreDisableSnrMarginData(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslFilterSNRMarginCmd;
	cmd.param.value = 1;
	AdslCoreCommandHandler(&cmd);
}

int	BcmAdslCoreSetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	uchar	*oid = (uchar *) objId;
	int		res;
	BcmCoreDpcSyncEnter();	
	res = AdslCoreSetObjectValue (objId, objIdLen, dataBuf, dataBufLen);
	BcmCoreDpcSyncExit();
	return res;
}

int	BcmAdslCoreGetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen)
{
	uchar	*oid = (uchar *) objId;
	int		res;

	BcmCoreDpcSyncEnter();
	if ((objIdLen > 1) && (kOidAdslPrivate == oid[0]) && (kOidAdslPrivShowtimeMargin == oid[1]))
		BcmAdslCoreEnableSnrMarginData();	
	res = AdslCoreGetObjectValue (objId, objIdLen, dataBuf, dataBufLen);
	BcmCoreDpcSyncExit();
	return res;
}

void BcmAdslCoreStartBERT(unsigned long totalBits)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagStartBERT;
	cmd.param.value = totalBits;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreStopBERT(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagStopBERT;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreBertStartEx(unsigned long bertSec)
{
	BcmCoreDpcSyncEnter();
	AdslCoreBertStartEx(bertSec);
	BcmCoreDpcSyncExit();
}

void BcmAdslCoreBertStopEx(void)
{
	BcmCoreDpcSyncEnter();
	AdslCoreBertStopEx();
	BcmCoreDpcSyncExit();
}

#ifndef DYING_GASP_API

#if defined(CONFIG_BCM963x8)
/* The BCM6348 cycles per microsecond is really variable since the BCM6348
 * MIPS speed can vary depending on the PLL settings.  However, an appoximate
 * value of 120 will still work OK for the test being done.
 */
#define	CYCLE_PER_US	120
#endif
#define	DG_GLITCH_TO	(100*CYCLE_PER_US)

#if !defined(__KERNEL__) && !defined(_CFE_)
#define BpGetAdslDyingGaspExtIntr(pIntrNum)		*(ulong *) (pIntrNum) = 0
#endif

Bool BcmAdslCoreCheckPowerLoss(void)
{
	ulong				clk0;
    ulong               ulIntr;

    ulIntr = 0;
	clk0 = BcmAdslCoreGetCycleCount();

    UART->Data = 'D';
	UART->Data = '%';
	UART->Data = 'G';

#if defined(CONFIG_BCM963x8) && !defined(VXWORKS)
    do {
		ulong		clk1;

		clk1 = BcmAdslCoreGetCycleCount();		/* time cleared */
		/* wait a little to get new reading */
		while ((BcmAdslCoreGetCycleCount()-clk1) < CYCLE_PER_US*2)
			;
	} while ((PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET))) && ((BcmAdslCoreGetCycleCount() - clk0) < DG_GLITCH_TO));

	if (!(PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET)))) {
        BcmHalInterruptEnable( INTERRUPT_ID_DG );
		AdslDrvPrintf (TEXT(" - Power glitch detected. Duration: %ld us\n"), (BcmAdslCoreGetCycleCount() - clk0)/CYCLE_PER_US);
		return AC_FALSE;
	}
#endif
	return AC_TRUE;
}
#endif /* DYING_GASP_API */

void BcmAdslCoreSendDyingGasp(int powerCtl)
{
    dslCommandStruct    cmd;

    if (kAdslTrainingConnected == AdslCoreLinkStateEx()) {
        cmd.command = kDslDyingGaspCmd;
        cmd.param.value = powerCtl != 0 ? 1 : 0;

#ifdef __KERNEL__
        if (!in_irq())
#endif
            BcmCoreDpcSyncEnter();

        AdslCoreCommandHandler(&cmd);
        AdslCoreCommandHandler(&cmd);

#ifdef __KERNEL__
        if (!in_irq())
#endif
            BcmCoreDpcSyncExit();
    } 
    else {
        AdslDrvPrintf (TEXT(" - Power failure detected. ADSL Link down.\n"));
    }

#ifndef DYING_GASP_API
    BcmAdslCoreSetWdTimer(1000000);
#if defined(CONFIG_BCM963x8)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | SAR_CLK_EN);
#endif
#endif
}

#ifdef __KERNEL__

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
extern init_module();
#define	DBG_ADDR(a)		((long) (a) < 0 ? (a) : (ulong) (a) + (ulong) init_module)
#else
extern dsl_get_stats();
#define	DBG_ADDR(a)		((long) (a) < 0 ? (a) : (ulong) (a) + (ulong) dsl_get_stats)
#endif

#else	/* __KERNEL__ */
#define	DBG_ADDR(a)		a 
#endif

void BcmAdslCoreDebugReadMem(DiagDebugData *pDbgCmd)
{
	ULONG				n, frCnt, dataSize;
	DiagProtoFrame		*pDiagFrame;
	DiagDebugData		*pDbgRsp;
	char				testFrame[LOG_MAX_BUF_SIZE], *pData, *pMem, *pMemEnd;

	pDiagFrame	= (void *) testFrame;
	pDbgRsp		= (void *) pDiagFrame->diagData;
	pData		= (char *) pDbgRsp->diagData;
	*(short *)pDiagFrame->diagHdr.logProtoId = *(short *) LOG_PROTO_ID;
	pDiagFrame->diagHdr.logPartyId	= LOG_PARTY_CLIENT;
	pDiagFrame->diagHdr.logCommmand = LOG_CMD_DEBUG;

	pDbgRsp->cmd = pDbgCmd->cmd;
	pDbgRsp->cmdId = pDbgCmd->cmdId;
	dataSize = sizeof(testFrame) - (pData - testFrame) - 16;
	pMem = (char *) DBG_ADDR(pDbgCmd->param1);
	pMemEnd = pMem + pDbgCmd->param2;

	frCnt	 = 0;
	while (pMem != pMemEnd) {
		n = pMemEnd - pMem;
		if (n > dataSize)
			n = dataSize;

		pDbgRsp->param1 = (ulong) pMem;
		pDbgRsp->param2 = n;
		memcpy (pData, pMem, n);
		BcmAdslCoreDiagWrite(testFrame, (pData - testFrame) + n);

		pMem += n;
		frCnt = (frCnt + 1) & 0xFF;

		if (0 == (frCnt & 7))
			BcmAdslCoreDelay(40);
	}
}

void BcmAdslCoreDebugCmd(void *pMsg)
{
	DiagDebugData	*pDbgCmd = (DiagDebugData *) pMsg;

	switch (pDbgCmd->cmd) {
		case DIAG_DEBUG_CMD_READ_MEM:
			BcmAdslCoreDebugReadMem(pDbgCmd);
			break;

		case DIAG_DEBUG_CMD_SET_MEM:
			{
			ulong	*pAddr = (ulong *) DBG_ADDR(pDbgCmd->param1);

			*pAddr = pDbgCmd->param2;
			}
			break;

		case DIAG_DEBUG_CMD_RESET_CONNECTION:
			BcmAdslCoreConnectionReset();
			break;

		case DIAG_DEBUG_CMD_RESET_PHY:
			BcmAdslCoreReset(DIAG_DATA_EYE);
			if (DIAG_DEBUG_CMD_LOG_DATA == diagDebugCmd.cmd) {
			  BcmAdslCoreDiagStartLog(diagDebugCmd.param1, diagDebugCmd.param2);
			  diagDebugCmd.cmd = 0;
			}
			break;

#if 1 && defined(BCM6348_PLL_WORKAROUND)
		case 99:
			AdslCoreSetPllClock();
			BcmAdslCoreReset(DIAG_DATA_EYE);
			break;
#endif

		case DIAG_DEBUG_CMD_RESET_CHIP:
			BcmAdslCoreSetWdTimer (1000);
			break;

		case DIAG_DEBUG_CMD_EXEC_FUNC:
			{
			int (*pExecFunc) (ulong param) = (void *) DBG_ADDR(pDbgCmd->param1);
			int	res;

			res = (*pExecFunc) (pDbgCmd->param2);
			BcmAdslCoreDiagWriteStatusString("CMD_EXEC_FUNC at 0x%X param=%d: result=%d", pExecFunc, pDbgCmd->param2, res);
			}
			break;

		case DIAG_DEBUG_CMD_WRITE_FILE:
			BcmAdslCoreSendBuffer(kDslDataAvailStatus, (void*) pDbgCmd->param1, pDbgCmd->param2);
			break;

		case DIAG_DEBUG_CMD_LOG_SAMPLES:
			{
			dslCommandStruct	cmd;
			int					nDmaBlk;

			BcmAdslCoreDiagDmaInit();
			nDmaBlk = BcmAdslCoreDiagGetDmaBlockNum();
			if (nDmaBlk < 3)
				break;

			bcmOsSleep (100/BCMOS_MSEC_PER_TICK);
			cmd.command = kDslDiagSetupCmd;
			cmd.param.dslDiagSpec.setup = kDslDiagEnableEyeData | kDslDiagEnableLogData;
			cmd.param.dslDiagSpec.eyeConstIndex1 = 63;
			cmd.param.dslDiagSpec.eyeConstIndex2 = 64;
			cmd.param.dslDiagSpec.logTime = (ulong) (1 - nDmaBlk);
			BcmCoreCommandHandlerSync(&cmd);
			bcmOsSleep (100/BCMOS_MSEC_PER_TICK);
			BcmAdslCoreSendDmaBuffers(kDslDataAvailStatus, nDmaBlk - 2);
			}
			break;

		case DIAG_DEBUG_CMD_LOG_DATA:
			if (pDbgCmd->cmdId == DIAG_DEBUG_CMD_LOG_AFTER_RESET)
			  diagDebugCmd = *pDbgCmd;
			else
			  BcmAdslCoreDiagStartLog(pDbgCmd->param1, pDbgCmd->param2);
			break;

#ifdef ADSLPHY_CHIPTEST_SUPPORT
		case DIAG_DEBUG_CMD_PLAYBACK_STOP:
			BcmAdslCoreDebugPlaybackStop();
			break;

		case DIAG_DEBUG_CMD_PLAYBACK_RESUME:
			BcmAdslCoreDebugPlaybackResume();
			break;
#endif

		case DIAG_DEBUG_CMD_G992P3_DEBUG:
			{
			extern Boolean adslCoreOvhMsgPrintEnabled;

			adslCoreOvhMsgPrintEnabled = (pDbgCmd->param1 != 0);
			}
			break;
        case DIAG_DEBUG_CMD_CLEAREOC_LOOPBACK:
		    {
			extern int ClearEOCLoopBackEnabled;
                        
		    ClearEOCLoopBackEnabled= (pDbgCmd->param1 != 0);
			}
			break;
		case DIAG_DEBUG_CMD_SET_OEM:
			{
			char str[500],str1[100];
			int i=0,k=0;
			memcpy(str,pDbgCmd,pDbgCmd->param2+sizeof(DiagDebugData));
			for (i=0;i<pDbgCmd->param2+sizeof(DiagDebugData);i++)
			{
				AdslDrvPrintf("DIAG_DEBUG_CMD_SET_OEM str[%d]=  %c",i,*(str+i));
				if(i>=sizeof(DiagDebugData))
					str1[k++]=str[i];
			}
			AdslCoreSetOemParameter(pDbgCmd->param1,str1,(int)pDbgCmd->param2);
			}
			break;
		case DIAG_DEBUG_CMD_G992P3_DIAG_MODE: 
			BcmAdslCoreSetAdslDiagMode(pDbgCmd->param1);
			break;
		case DIAG_DEBUG_CMD_ANNEXM_CFG:
			if (pDbgCmd->param2 != 0) {
				if (-1 == (long) pDbgCmd->param2)
					pDbgCmd->param2 = 0;
				adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA->readsl2Upstream = 
				adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA->readsl2Upstream =
					pDbgCmd->param2 & 0xFF;
				adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA->readsl2Downstream = 
				adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA->readsl2Downstream =
					(pDbgCmd->param2 >> 8) & 0xFF;
			}
			adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5 = pDbgCmd->param1 & (kAdsl2CfgAnnexMPsdMask >> kAdsl2CfgAnnexMPsdShift);
			adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5 |= adslCoreEcUpdateMask;
			BcmAdslCoreConnectionReset();
			break;
		case DIAG_DEBUG_CMD_SET_L2_TIMEOUT:
			AdslCoreSetL2Timeout(pDbgCmd->param1);
			break;
	}
}

void BcmAdslCoreGdbTask(void)
{
}

void BcmAdslCoreGdbCmd(void *pCmd, int cmdLen)
{
	DiagProtoFrame		*pDiagFrame;
	uchar				*pGdbFrame;
	char				testFrame[LOG_MAX_BUF_SIZE];

	BcmAdslCoreDiagWriteStatusString("BcmAdslCoreGdbCmd: cmdLen=%d data[0]=0x%X", cmdLen, ((uchar *)pCmd)[0]);
	pDiagFrame	= (void *) testFrame;
	pGdbFrame	= (void *) pDiagFrame->diagData;
	*(short *)pDiagFrame->diagHdr.logProtoId = *(short *) LOG_PROTO_ID;
	pDiagFrame->diagHdr.logPartyId	= LOG_PARTY_CLIENT;
	pDiagFrame->diagHdr.logCommmand = LOG_CMD_GDB;

	memcpy (pGdbFrame, pCmd, cmdLen);
	BcmAdslCoreDiagWrite(testFrame, cmdLen + sizeof(LogProtoHeader));
}

void BcmAdslCoreResetStatCounters(void)
{
	BcmCoreDpcSyncEnter();
	AdslCoreResetStatCounters();
	BcmCoreDpcSyncExit();
}

void BcmAdslCoreSetTestMode(int testMode)
{
	dslCommandStruct	cmd;

	if (ADSL_TEST_DIAGMODE == testMode) {
		BcmAdslCoreSetAdslDiagMode(1);
		return;
	}

	BcmCoreDpcSyncEnter();
	if (ADSL_TEST_L3 == testMode) {
		acL3Requested = 1;
		AdslCoreSetL3();
	}
	else if (ADSL_TEST_L0 == testMode) {
		AdslCoreSetL0();
	}
	else {
		cmd.command = kDslTestCmd;
		cmd.param.dslTestSpec.type = testMode;
		AdslCoreCommandHandler(&cmd);
	}
	BcmCoreDpcSyncExit();
}
void BcmAdslCoreSetTestExecutionDelay(int testMode, ulong value)
{
	dslCommandStruct	cmd;

    BcmCoreDpcSyncEnter();
	cmd.command = kDslTestCmd;
	cmd.param.dslTestSpec.type =kDslTestExecuteDelay ;
	cmd.param.dslTestSpec.param.value=value;
	AdslCoreCommandHandler(&cmd);
	BcmCoreDpcSyncExit();
}
void BcmAdslCoreSetAdslDiagMode(int diagMode)
{
#ifdef G992P3
	dslCommandStruct	cmd;
	adslMibInfo			*pMibInfo;
	ulong				mibLen;

	if (!adslCoreStarted)
		return;

	if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
		cmd.command = kDslIdleCmd;
 		BcmCoreCommandHandlerSync(&cmd);

		BcmAdslCoreDelay(40);

		pMibInfo = (void *) AdslCoreGetObjectValue (NULL, 0, NULL, &mibLen);
		if (diagMode != 0)
			pMibInfo->adslPhys.adslLDCompleted = 0;

		g992p5Param.diagnosticsModeEnabled = 
		g992p3Param.diagnosticsModeEnabled = diagMode != 0 ? 1 : 0;
		BcmCoreCommandHandlerSync(&adslCoreConnectionParam);
		g992p3Param.diagnosticsModeEnabled = 0;
		g992p5Param.diagnosticsModeEnabled = 0;
	}
#endif
}

void BcmAdslCoreSelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap)
{
	dslCommandStruct	cmd;

	cmd.command = kDslTestCmd;
	cmd.param.dslTestSpec.type = kDslTestToneSelection;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtStartTone = xmtStartTone;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones = xmtNumTones;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvStartTone = rcvStartTone;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones = rcvNumTones;
	cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap = xmtToneMap;
	cmd.param.dslTestSpec.param.toneSelectSpec.rcvMap = rcvToneMap;
#if 1
	{
		int		i;

		AdslDrvPrintf(TEXT("xmtStartTone=%ld, xmtNumTones=%ld, rcvStartTone=%ld, rcvNumTones=%ld \nxmtToneMap="),
			cmd.param.dslTestSpec.param.toneSelectSpec.xmtStartTone,
			cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones,
			cmd.param.dslTestSpec.param.toneSelectSpec.rcvStartTone,
			cmd.param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones);
		for (i = 0; i < ((cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones+7)>>3); i++)
			AdslDrvPrintf(TEXT("%02X "), cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap[i]);
		AdslDrvPrintf(TEXT("\nrcvToneMap="));
		for (i = 0; i < ((cmd.param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones+7)>>3); i++)
			AdslDrvPrintf(TEXT("%02X "), cmd.param.dslTestSpec.param.toneSelectSpec.rcvMap[i]);
	}
	AdslDrvPrintf(TEXT("\n"));
#endif

	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreDiagRearrangeSelectTones(int *xmtStartTone, int *xmtNumTones,int *rcvStartTone, int *rcvNumTones, char *rcvToneMap, char *xmtToneMap)
{
	*rcvStartTone = *xmtNumTones - 8;
	*rcvNumTones  = 512 - *rcvStartTone;
	if ((*rcvToneMap - *xmtToneMap) != (32 >> 3)) {
		int		i;
			for (i = (32 >> 3); i < (*xmtNumTones >> 3); i++)
			xmtToneMap[i] = rcvToneMap[i - (32 >> 3)];
	}
	//rcvToneMap   += (*rcvStartTone - 32) >> 3;
	return;
}

void BcmAdslCoreDiagSelectTones(
	int		xmtStartTone, 
	int		xmtNumTones, 
	int		rcvStartTone,
	int		rcvNumTones, 
	char	*xmtToneMap,
	char	*rcvToneMap)
{
#ifdef G992P1_ANNEX_B
	if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB)) {
		xmtNumTones  = (adslCoreConnectionParam.param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr + 7) & ~7;
		BcmAdslCoreDiagRearrangeSelectTones(&xmtStartTone, &xmtNumTones, &rcvStartTone, &rcvNumTones,rcvToneMap,xmtToneMap);
	}
#endif
	if((adslCoreConnectionParam.param.dslModeSpec.capabilities.modulations & kG992p3AnnexM) ||(adslCoreConnectionParam.param.dslModeSpec.capabilities.modulations & kG992p5AnnexM) )
	{
		xmtNumTones  = 64;
		BcmAdslCoreDiagRearrangeSelectTones(&xmtStartTone, &xmtNumTones, &rcvStartTone, &rcvNumTones, rcvToneMap, xmtToneMap);
		rcvToneMap+=(rcvStartTone - 32) >> 3;
	}
	BcmAdslCoreSelectTones(xmtStartTone,xmtNumTones, rcvStartTone,rcvNumTones, xmtToneMap,rcvToneMap);
}

Bool BcmAdslCoreSetSDRAMBaseAddr(void *pAddr)
{
	return AdslCoreSetSDRAMBaseAddr(pAddr);
}

Bool BcmAdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp)
{
	return AdslCoreSetVcEntry(gfc, port, vpi, vci, pti_clp);
}

Bool BcmAdslCoreSetGfc2VcMapping(Bool bOn)
{
	dslCommandStruct	cmd;

	cmd.command = kDslAtmGfcMappingCmd;
	cmd.param.value = bOn;
	BcmCoreCommandHandlerSync(&cmd);
	return AC_TRUE;
}

Bool BcmAdslCoreSetAtmLoopbackMode(void)
{
	dslCommandStruct	cmd;

#if 1
	BcmAdslCoreReset(DIAG_DATA_EYE);
#else
	BcmAdslCoreConnectionStop();
#endif
	BcmAdslCoreDelay(100);

	cmd.command = kDslLoopbackCmd;
	BcmCoreCommandHandlerSync(&cmd);
	return AC_TRUE;
}

int BcmAdslCoreGetOemParameter (int paramId, void *buf, int len)
{
	return AdslCoreGetOemParameter (paramId, buf, len);
}

int BcmAdslCoreSetOemParameter (int paramId, void *buf, int len)
{
	return AdslCoreSetOemParameter (paramId, buf, len);
}

int BcmAdslCoreSetXmtGain(int gain)
{
	if ((gain != ADSL_XMT_GAIN_AUTO) && ((gain < -22) || (gain > 2)))
		return BCMADSL_STATUS_ERROR;

#if 1
	if (gain != adslCoreXmtGain) {
		adslCoreXmtGain = gain;
		adslCoreXmtGainChanged = AC_TRUE;
		if (adslCoreConnectionMode)
			BcmAdslCoreConnectionReset();
	}
#else
	adslCoreXmtGain = gain;
	BcmAdslCoreSendXmtGainCmd(gain);
#endif
	return BCMADSL_STATUS_SUCCESS;
}


int  BcmAdslCoreGetSelfTestMode(void)
{
	return AdslCoreGetSelfTestMode();
}

void BcmAdslCoreSetSelfTestMode(int stMode)
{
	AdslCoreSetSelfTestMode(stMode);
}

int  BcmAdslCoreGetSelfTestResults(void)
{
	return AdslCoreGetSelfTestResults();
}



ADSL_LINK_STATE BcmAdslCoreGetEvent (void)
{
	int		adslState;

	if (0 == acPendingEvents)
		adslState = -1;
	else if (acPendingEvents & ACEV_LINK_UP) {
		adslState = BCM_ADSL_LINK_UP;
		acPendingEvents &= ~ACEV_LINK_UP;
	}
	else if (acPendingEvents & ACEV_LINK_DOWN) {
		adslState = BCM_ADSL_LINK_DOWN;
		acPendingEvents &= ~ACEV_LINK_DOWN;
	}
	else if (acPendingEvents & ACEV_G997_FRAME_RCV) {
		adslState = BCM_ADSL_G997_FRAME_RECEIVED;
		acPendingEvents &= ~ACEV_G997_FRAME_RCV;
	}
	else if (acPendingEvents & ACEV_G997_FRAME_SENT) {
		adslState = BCM_ADSL_G997_FRAME_SENT;
		acPendingEvents &= ~ACEV_G997_FRAME_SENT;
	}
	else if (acPendingEvents & ACEV_SWITCH_RJ11_PAIR) {
		adslState = ADSL_SWITCH_RJ11_PAIR;
		acPendingEvents &= ~ACEV_SWITCH_RJ11_PAIR;
	}
	else if (acPendingEvents & ACEV_G994_NONSTDINFO_RECEIVED) {
		adslState = BCM_ADSL_G994_NONSTDINFO_RECEIVED;
		acPendingEvents &= ~ACEV_G994_NONSTDINFO_RECEIVED;
	}
	else {
		adslState = BCM_ADSL_EVENT;
		acPendingEvents = 0;
	}

	return adslState;
}


Bool BcmAdslCoreG997SendData(void *buf, int len)
{
	Bool	bRet;

	BcmCoreDpcSyncEnter();
	bRet = AdslCoreG997SendData(buf, len);
	BcmCoreDpcSyncExit();
	return bRet;
}

void *BcmAdslCoreG997FrameGet(int *pLen)
{
	return AdslCoreG997FrameGet(pLen);
}

void *BcmAdslCoreG997FrameGetNext(int *pLen)
{
	return AdslCoreG997FrameGetNext(pLen);
}

void BcmAdslCoreG997FrameFinished(void)
{
	BcmCoreDpcSyncEnter();
	AdslCoreG997FrameFinished();
	BcmCoreDpcSyncExit();
}



void BcmAdslCoreNotify(long acEvent)
{
	if (ACEV_LINK_POWER_L3 == acEvent) {
		dslCommandStruct	cmd;

		cmd.command = kDslIdleCmd;
 		AdslCoreCommandHandler(&cmd);

		if (!acL3Requested) {
			bcmOsGetTime(&acL3StartTick);
			if (0 == acL3StartTick)
				acL3StartTick = 1;
		}
		acL3Requested = 0;
		return;
	}

	acPendingEvents |= acEvent;
	(*bcmNotify)();
}

int BcmAdslCoreGetConstellationPoints (int toneId, ADSL_CONSTELLATION_POINT *pointBuf, int numPoints)
{
	return BcmAdslDiagGetConstellationPoints (toneId, pointBuf, numPoints);
}

/*
**
**	ATM EOP workaround functions
**
*/


void BcmAdslCoreAtmSetPortId(int path, int portId)
{
	dslCommandStruct	cmd;
	
	cmd.command = kDslAtmVcControlCmd;
	if(path==0)
		cmd.param.value = kDslAtmSetIntlPortId | portId;
	else if(path==1)
		cmd.param.value = kDslAtmSetFastPortId | portId;
	BcmCoreCommandHandlerSync(&cmd);
}

ulong   atmVcTable[16] = { 0 };
ulong	atmVcCnt = 0;
#define ATM_VC_SIZE		sizeof(atmVcTable)/sizeof(atmVcTable[0])

void BcmCoreAtmVcInit(void)
{
	atmVcCnt = 0;
	AdslMibByteClear(ATM_VC_SIZE, atmVcTable);
}

void BcmCoreAtmVcSet(void)
{
	dslCommandStruct	cmd;
	int					i;

	cmd.command = kDslAtmVcControlCmd;
	cmd.param.value = kDslAtmVcClear;
	BcmCoreCommandHandlerSync(&cmd);

	for (i = 0; i < atmVcCnt; i++) {
		cmd.command = kDslAtmVcControlCmd;
		cmd.param.value = kDslAtmVcAddEntry | atmVcTable[i];
		BcmCoreCommandHandlerSync(&cmd);
	}
}

void BcmAdslCoreAtmClearVcTable(void)
{
	dslCommandStruct	cmd;

	atmVcCnt = 0;
	AdslMibByteClear(ATM_VC_SIZE, atmVcTable);

	cmd.command = kDslAtmVcControlCmd;
	cmd.param.value = kDslAtmVcClear;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreAtmAddVc(int vpi, int vci)
{
	dslCommandStruct	cmd;
	int					i;
	ulong				vc;

	vc = (vpi << 16) | vci;
	for (i = 0; i < atmVcCnt; i++) {
	  if (vc == atmVcTable[i])
		break;
	}
	if ((i == atmVcCnt) && (atmVcCnt < ATM_VC_SIZE)) {
	  atmVcTable[atmVcCnt] = vc;
	  atmVcCnt++;
	}

	cmd.command = kDslAtmVcControlCmd;
	cmd.param.value = kDslAtmVcAddEntry | vc;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreAtmDeleteVc(int vpi, int vci)
{
	dslCommandStruct	cmd;
	int					i;
	ulong				vc;

	vc = (vpi << 16) | vci;
	for (i = 0; i < atmVcCnt; i++) {
	  if (vc == atmVcTable[i]) {
		atmVcTable[i] = atmVcTable[atmVcCnt];
		atmVcCnt--;
		atmVcTable[atmVcCnt] = 0;
		break;
	  }
	}

	cmd.command = kDslAtmVcControlCmd;
	cmd.param.value = kDslAtmVcDeleteEntry | vc;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreAtmSetMaxSdu(unsigned short maxsdu)
{
	dslCommandStruct	cmd;

	cmd.command = kDslAtmVcControlCmd;
	cmd.param.value = kDslAtmSetMaxSDU | maxsdu;
	BcmCoreCommandHandlerSync(&cmd);
}

#if 0
void AddVcTest(ulong vci)
{
	BcmAdslCoreAtmAddVc(vci >> 16, vci & 0xFFFF);
}

void DelVcTest(ulong vci)
{
	BcmAdslCoreAtmDeleteVc(vci >> 16, vci & 0xFFFF);
}
#endif

/*
**
**	DslDiags common functions
**
*/

static	void	*diagStatDataPtr = NULL;

void DiagWriteDataCont(char *buf0, int len0, char *buf1, int len1)
{
	if (len0 > LOG_MAX_DATA_SIZE) {
		BcmAdslCoreDiagWriteStatusData(statusInfoData-2, buf0, LOG_MAX_DATA_SIZE, NULL, 0);
		DiagWriteDataCont(buf0 + LOG_MAX_DATA_SIZE, len0 - LOG_MAX_DATA_SIZE, buf1, len1);
	}
	else if ((len0 + len1) > LOG_MAX_DATA_SIZE) {
		long	len2 = (LOG_MAX_DATA_SIZE - len0) & ~1;

		BcmAdslCoreDiagWriteStatusData(statusInfoData-2, buf0, len0, buf1, len2);
		DiagWriteDataCont(buf1 + len2, len1 - len2, NULL, 0);
	}
	else
		BcmAdslCoreDiagWriteStatusData(statusInfoData-3, buf0, len0, buf1, len1);
}

void BcmAdslCoreDiagWriteStatus(dslStatusStruct *status, char *pBuf, int len)
{
	static	long	statStrBuf[4096/4];
	char			*statStr, *p1;
	ulong			cmd;
	long			n, n1;

	if (!BcmAdslDiagIsActive())
		return;

	BcmCoreDpcSyncEnter();

	statStr = pBuf;
	n = len;
	cmd = statusInfoData;
	p1 = NULL;
	n1 = 0;

	switch (status->code) {
	  case kDslPrintfStatus:
	  case kDslPrintfStatus1:
		statStr = (char *) statStrBuf;
		n = vsprintf (statStr, status->param.dslPrintfMsg.fmt, status->param.dslPrintfMsg.argPtr) + 1;
		BcmAdslCoreDiagScrambleString(statStr);
		cmd = LOG_CMD_SCRAMBLED_STRING;
		break;

	  case kDslExceptionStatus:
		{
		ulong	*sp;
		sp = (ulong*) status->param.dslException.sp;
#ifdef FLATTEN_ADDR_ADJUST
		sp = (ulong *) (sp[28] | FLATTEN_ADDR_ADJUST);
#endif
		p1 = (void *) sp;
		n1 = 64 * sizeof(long);
		}
		break;
	  case kDslConnectInfoStatus:
		{
		ulong	bufLen = status->param.dslConnectInfo.value;

		  switch (status->param.dslConnectInfo.code) {
			case kDslChannelResponseLog:
			case kDslChannelResponseLinear:
				bufLen <<= 1;
				/* */
			case kDslChannelQuietLineNoise:
				p1 = status->param.dslConnectInfo.buffPtr;
				n1 = bufLen;
				break;
			case kDslRcvCarrierSNRInfo:
				if (NULL != diagStatDataPtr) {
					n  = 3 * 4;
					p1 = diagStatDataPtr;
					n1 = bufLen << 1;
				}
				break;
			case kG992p2XmtToneOrderingInfo:
			case kG992p2RcvToneOrderingInfo:
			case kG994MessageExchangeRcvInfo:
			case kG994MessageExchangeXmtInfo:
			case kG992MessageExchangeRcvInfo:
			case kG992MessageExchangeXmtInfo:
				if (NULL != diagStatDataPtr) {
					n  = 3 * 4;
					p1 = diagStatDataPtr;
					n1 = bufLen;
				}
				break;
			default:
				break;
		  }
		}
		break;
	  case kDslDspControlStatus:
		switch (status->param.dslConnectInfo.code) {
			case kDslG992RcvShowtimeUpdateGainPtr:
			case kDslPLNPeakNoiseTablePtr:
			case kDslPerBinThldViolationTablePtr:
			case kDslImpulseNoiseDurationTablePtr:
			case kDslImpulseNoiseTimeTablePtr:
			case kDslInpBinTablePtr:
			case kDslItaBinTablePtr:  
			case kDslNLNoise:
			case kDslInitializationSNRMarginInfo:
			case kFireMonitoringCounters:
				p1 = status->param.dslConnectInfo.buffPtr;
				n1 = status->param.dslConnectInfo.value;
				break;
		}
		break;

	  case kDslShowtimeSNRMarginInfo:
		if (NULL != diagStatDataPtr) {
			n  = sizeof(status->param.dslShowtimeSNRMarginInfo) + 4 - 4;
			p1 = diagStatDataPtr;
			n1 = status->param.dslShowtimeSNRMarginInfo.nCarriers << 1;
		}
		break;
	  case kDslReceivedEocCommand:
		if ( ((kDslClearEocSendFrame  == status->param.dslClearEocMsg.msgId) ||
			  (kDslClearEocRcvedFrame == status->param.dslClearEocMsg.msgId) ||
			  (kDslGeneralMsgStart <= status->param.dslClearEocMsg.msgId))
		  &&
			 (0 == (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)))
		{
			n -= 4;
			((dslStatusStruct *)statStr)->param.dslClearEocMsg.msgType |= kDslClearEocMsgDataVolatileMask;
			n1 = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
			p1 = status->param.dslClearEocMsg.dataPtr;
		}
		break;
	}

	if (n > LOG_MAX_DATA_SIZE) {
		BcmAdslCoreDiagWriteStatusData(cmd | DIAG_SPLIT_MSG, statStr, LOG_MAX_DATA_SIZE, NULL, 0);
		DiagWriteDataCont(statStr + LOG_MAX_DATA_SIZE, n - LOG_MAX_DATA_SIZE, p1, n1);
	}
	else if ((n + n1) > LOG_MAX_DATA_SIZE) {
		long	len1 = (LOG_MAX_DATA_SIZE - n) & ~1;

		BcmAdslCoreDiagWriteStatusData(cmd | DIAG_SPLIT_MSG, statStr, n, p1, len1);
		DiagWriteDataCont(p1 + len1, n1 - len1, NULL, 0);
	}
	else
		BcmAdslCoreDiagWriteStatusData(cmd, statStr, n, p1, n1);

	BcmCoreDpcSyncExit();
}

void BcmAdslCoreWriteOvhMsg(void *gDslVars, char *hdr, dslFrame *pFrame)
{
	dslStatusStruct status;
	dslFrameBuffer	*pBuf, *pBufNext;
	uchar		*pData;
	int			dataLen, i = 0;
	ulong		cmd = statusInfoData;
	Boolean		bFirstBuf = true;
	
	status.code = kDslDspControlStatus;
	if( 'T' == hdr[0] )
		status.param.dslConnectInfo.code = kDslTxOvhMsg;
	else
		status.param.dslConnectInfo.code = kDslRxOvhMsg;
	status.param.dslConnectInfo.value = DslFrameGetLength(gDslVars, pFrame);
	pBuf = DslFrameGetFirstBuffer(gDslVars, pFrame);

	while (NULL != pBuf) {
		dataLen   = DslFrameBufferGetLength(gDslVars, pBuf);
		pData = DslFrameBufferGetAddress(gDslVars, pBuf);
		pBufNext = DslFrameGetNextBuffer(gDslVars, pBuf);
		if( ++i >= 20 && pBufNext) {
			BcmAdslCoreDiagWriteStatusData(cmd -3,	pData, dataLen, NULL, 0);
			BcmAdslCoreDiagWriteStatusString(" G.997 frame %s: pFr = 0x%X, len = %ld; too many buffer(> %d) in this frame",
				hdr, (int) pFrame, DslFrameGetLength(gDslVars, pFrame), i);
			break;
		}			
		
		if( !bFirstBuf ) {
			if(pBufNext) 
				BcmAdslCoreDiagWriteStatusData(cmd -2, pData, dataLen, NULL, 0);			
			else 
				BcmAdslCoreDiagWriteStatusData(cmd -3,	pData, dataLen, NULL, 0);			
		}
		else {
			if(pBufNext)
				BcmAdslCoreDiagWriteStatusData(cmd | DIAG_SPLIT_MSG,
					&status, sizeof(status.code) + sizeof(status.param.dslConnectInfo),
					pData, dataLen);
			else 
				BcmAdslCoreDiagWriteStatusData(cmd,
					&status, sizeof(status.code) + sizeof(status.param.dslConnectInfo),
					pData, dataLen);
			bFirstBuf = FALSE;	
		}
		pBuf = pBufNext;
	}	
}

void BcmAdslCoreDiagStatusSnooper(dslStatusStruct *status, char *pBuf, int len)
{
	Bool bStatPtrSet = false;

	if (!BcmAdslDiagIsActive())
		return;

	BcmAdslCoreDiagWriteStatus(status, pBuf, len);

	switch (status->code) {
		case kDslDspControlStatus:
			if (kDslStatusBufferInfo == status->param.dslConnectInfo.code) {
				diagStatDataPtr = status->param.dslConnectInfo.buffPtr;
				bStatPtrSet = true;
			}
			break;
		case kDslExceptionStatus:
			BcmAdslCoreDiagWriteStatusString("Resetting ADSL MIPS\n");
			break;
		default:
			break;
	}
	if (!bStatPtrSet)
		diagStatDataPtr = NULL;
}

void BcmAdslCoreDiagWriteStatusString(char *fmt, ...)
{
	dslStatusStruct		status;
	va_list				ap;

	va_start(ap, fmt);

	status.code = kDslPrintfStatus;
	status.param.dslPrintfMsg.fmt = fmt;
	status.param.dslPrintfMsg.argNum = 0;
	status.param.dslPrintfMsg.argPtr = (void *)ap;
	va_end(ap);

	BcmAdslCoreDiagWriteStatus(&status, NULL, 0);
}

/* common diag command handler */

static void BcmDiagPrintConfigCmd (char *hdr, adslCfgProfile *pCfg)
{
	ulong	cfg;

	AdslDrvPrintf (TEXT("DrvDiag: %s: CCFG=0x%lX ACFG=0x%lX"), hdr, pCfg->adslAnnexCParam, pCfg->adslAnnexAParam);
#ifndef G992_ANNEXC
	cfg = pCfg->adslAnnexAParam;
#else
	cfg = pCfg->adslAnnexCParam;
#endif
	if (cfg & kAdslCfgExtraData) {
		AdslDrvPrintf (TEXT(" TrM=0x%lX ShM=0x%lX MgTo=%ld"), pCfg->adslTrainingMarginQ4, pCfg->adslShowtimeMarginQ4, pCfg->adslLOMTimeThldSec);
		if (cfg & kAdslCfgDemodCapOn)
			AdslDrvPrintf (TEXT(", DemodMask=0x%lX DemodVal=0x%lX\n"), pCfg->adslDemodCapMask, pCfg->adslDemodCapValue);
		else
			AdslDrvPrintf (TEXT("\n"));
	}
	else
		AdslDrvPrintf (TEXT("\n"));
}

void BcmAdslCoreDiagCmdCommon(int diagCmd, int len, void *pCmdData)
{
	dslCommandStruct	cmd;
        char*                   pInpBins;
	char*                   pItaBins;
	/* AdslDrvPrintf (TEXT("DrvDiagCmd: %d\n"), diagCmd); */
	
	switch (diagCmd) {
		case LOG_CMD_PING_REQ:
			/* AdslDrvPrintf (TEXT("DrvDiagCmd: PING\n")); */
			BcmAdslCoreDiagWriteStatusData(LOG_CMD_PING_RSP, pCmdData, 2, NULL, 0);
			break;

		case LOG_CMD_MIB_GET: {
			char			cmdData[LOG_MAX_DATA_SIZE];
			uchar			*oid;
			long			size;
			int				res;

			cmdData[0] = len & 0xFF;
			cmdData[1] = (len >> 8) & 0xFF;
			if (len != 0)
				memcpy(cmdData+2, pCmdData, len);
			oid  = cmdData+2;
			size = sizeof(cmdData) - (2+len+2);
			res = BcmAdslCoreGetObjectValue ((0 == len ? NULL : oid), len, oid+len+2, &size);
			if (kAdslMibStatusSuccess == res) {
				oid[len+0] = size & 0xFF;
				oid[len+1] = (size >> 8) & 0xFF;
				BcmAdslCoreDiagWriteStatusData(LOG_CMD_MIB_GET, cmdData, 2+len+2+size, NULL, 0);
			}
			else if ((kAdslMibStatusBufferTooSmall == res) && (kOidAdslPrivate == oid[0])) {
				uchar	*pObj;
				int		n;

				pObj = (void *) BcmAdsl_GetObjectValue ((0 == len ? NULL : oid), len, NULL, (long *)&size);
				n = sizeof(cmdData) - (2+len+2);
				oid[0] = kOidAdslPrivatePartial;
				oid[len+0] = n & 0xFF;
				oid[len+1] = (n >> 8) & 0xFF;
				memcpy (oid+len+2, pObj, n);
				BcmAdslCoreDiagWriteStatusData(LOG_CMD_MIB_GET, cmdData, 2+len+2+n, NULL, 0);

				size -= n;
				oid[len+0] = size & 0xFF;
				oid[len+1] = (size >> 8) & 0xFF;
				memcpy (oid+len+2, pObj+n, size);
				BcmAdslCoreDiagWriteStatusData(LOG_CMD_MIB_GET, cmdData, 2+len+2+size, NULL, 0);

				oid[len+0] = 0;
				oid[len+1] = 0;
				BcmAdslCoreDiagWriteStatusData(LOG_CMD_MIB_GET, cmdData, 2+len+2, NULL, 0);
			}
			else {
				BcmAdslCoreDiagWriteStatusString("MIB error %d: oidLen=%d, oid = %d %d %d %d %d", 
					res, len, oid[0], oid[1], oid[2], oid[3], oid[4]);
				oid[len+0] = 0;
				oid[len+1] = 0;
				BcmAdslCoreDiagWriteStatusData(LOG_CMD_MIB_GET, cmdData, 2+len+2, NULL, 0);
			}
			}
			break;

		case LOG_CMD_EYE_CFG:
			{
			short	*pEyeTones = pCmdData;

			cmd.command = kDslDiagSetupCmd;
			cmd.param.dslDiagSpec.setup = kDslDiagEnableEyeData;
			cmd.param.dslDiagSpec.eyeConstIndex1 = pEyeTones[0];
			cmd.param.dslDiagSpec.eyeConstIndex2 = pEyeTones[1];
			cmd.param.dslDiagSpec.logTime = 0;
			BcmCoreCommandHandlerSync(&cmd);
			}
			break;

		case LOG_CMD_SWITCH_RJ11_PAIR:
			AdslDrvPrintf (TEXT("DrvDiag: LOG_CMD_SWITCH_RJ11_PAIR\n"));
			BcmAdslCoreNotify(ACEV_SWITCH_RJ11_PAIR);
			break;

		case LOG_CMD_CFG_PHY:
		{
			adslPhyCfg	*pCfg = pCmdData;

			AdslDrvPrintf (TEXT("CFG_PHY CMD: demodCapabilities=0x%x  mask=0x%x  demodCap=0x%x\n"),
				adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities,
				pCfg->demodCapMask, pCfg->demodCap);
			if(adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities == pCfg->demodCap) {
				AdslDrvPrintf (TEXT("CFG_PHY CMD: No change, do nothing\n"));
				break;
			}
			adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities |= 
				pCfg->demodCapMask & pCfg->demodCap;
			adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities &= 
				~(pCfg->demodCapMask & ~pCfg->demodCap);

			adslCoreCfgProfile.adslDemodCapValue=adslCoreConnectionParam.param.dslModeSpec.capabilities.demodCapabilities;
			adslCoreCfgProfile.adslDemodCapMask=adslCoreCfgProfile.adslDemodCapValue;			
		}
			goto _cmd_reset;

		case LOG_CMD_CFG_PHY2:
		{
			adslPhyCfg	*pCfg = pCmdData;

			AdslDrvPrintf (TEXT("CFG_PHY2 CMD: subChannelInfop5=0x%x  mask=0x%x  demodCap=0x%x\n"),
				adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5,
				pCfg->demodCapMask, pCfg->demodCap);
			
			if(adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5 == pCfg->demodCap) {
				AdslDrvPrintf (TEXT("CFG_PHY2 CMD: No change, do nothing\n"));
				break;
			}

			adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5 |= 
				pCfg->demodCapMask & pCfg->demodCap;
			adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5 &= 
				~(pCfg->demodCapMask & ~pCfg->demodCap);

			adslCoreCfgProfile.adslDemodCap2Value=adslCoreConnectionParam.param.dslModeSpec.capabilities.subChannelInfop5;
			adslCoreCfgProfile.adslDemodCap2Mask=adslCoreCfgProfile.adslDemodCap2Value;	
		}
		
_cmd_reset:

		case LOG_CMD_RESET:
			BcmAdslCoreConnectionReset();
			break;

		case LOG_CMD_TEST_DATA:
			adslCoreAlwaysReset = (0 != *(long*) pCmdData) ? AC_TRUE : AC_FALSE;
			break;

		case LOG_CMD_LOG_STOP:
			cmd.command = kDslDiagStopLogCmd;
			BcmCoreCommandHandlerSync(&cmd);
			break;

		case LOG_CMD_CONFIG_A:
#ifndef G992_ANNEXC
			{
			adslCfgProfile *pAdslCfg = (adslCfgProfile *) pCmdData;

			BcmDiagPrintConfigCmd  ("LOG_CMD_CONFIG_A", pCmdData);

			if ( (kAdslCfgModAny == (pAdslCfg->adslAnnexAParam & kAdslCfgModMask)) &&
				 (pAdslCfg->adsl2Param == (kAdsl2CfgReachExOn | kAdsl2CfgAnnexLUpWide | kAdsl2CfgAnnexLUpNarrow)) )
				pAdslCfg->adsl2Param = 0;
			BcmAdslCoreConfigure((adslCfgProfile *) pCmdData);
			}
#else
			AdslDrvPrintf (TEXT("DrvDiag: LOG_CMD_CONFIG_A - Can't configure AnnexC modem for AnnexA\n"));
#endif
			break;

		case LOG_CMD_CONFIG_C:
#ifdef G992_ANNEXC
			BcmDiagPrintConfigCmd  ("LOG_CMD_CONFIG_C", pCmdData);
			BcmAdslCoreConfigure((adslCfgProfile *) pCmdData);
#else
			AdslDrvPrintf (TEXT("DrvDiag: LOG_CMD_CONFIG_C - Can't configure AnnexA modem for AnnexC\n"));
#endif
			break;

		case LOG_CMD_BERT_EX:
			BcmAdslCoreBertStartEx(*(long*)pCmdData);
			break;

		case LOG_CMD_GDB:
			BcmAdslCoreGdbCmd(pCmdData, len);
			break;
        case (kDslAfelbTestCmd - kFirstDslCommandCode):
            cmd.param.value = *(long*)pCmdData;
		case (kDslTestQuietCmd - kFirstDslCommandCode):
            cmd.param.value = *(long*)pCmdData;
		case (kDslDiagStartBERT - kFirstDslCommandCode):
		case (kDslFilterSNRMarginCmd - kFirstDslCommandCode):
		case (kDslDyingGaspCmd - kFirstDslCommandCode):
			cmd.param.value = *(long*)pCmdData;

			/* fall through */

		case (kDslStartRetrainCmd - kFirstDslCommandCode):
		case (kDslIdleCmd - kFirstDslCommandCode):
		case (kDslIdleRcvCmd - kFirstDslCommandCode):
		case (kDslIdleXmtCmd - kFirstDslCommandCode):
		case (kDslLoopbackCmd - kFirstDslCommandCode):
		case (kDslDiagStopBERT - kFirstDslCommandCode):
		case (kDslDiagStopLogCmd - kFirstDslCommandCode):
		case (kDslPingCmd - kFirstDslCommandCode):
			cmd.command = diagCmd + kFirstDslCommandCode;
			AdslDrvPrintf (TEXT("DrvDiag: cmd=%ld, val= %ld\n"), cmd.command, cmd.param.value);
			BcmCoreCommandHandlerSync(&cmd);

			if ((kDslDyingGaspCmd - kFirstDslCommandCode) == diagCmd) {
				bcmOsSleep (1000/BCMOS_MSEC_PER_TICK);
				cmd.command = kDslIdleCmd;
				BcmCoreCommandHandlerSync(&cmd);
			}
			break;

		case (kDslTestCmd - kFirstDslCommandCode):
			cmd.command = kDslTestCmd;
			memcpy(&cmd.param, pCmdData, sizeof(cmd.param.dslTestSpec));
			/* AdslDrvPrintf (TEXT("DrvDslTestCmd: testCmd=%ld\n"), cmd.param.dslTestSpec.type); */
			if (kDslTestToneSelection == cmd.param.dslTestSpec.type) {
				cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap = ((char*) pCmdData) +
					FLD_OFFSET(dslCommandStruct,param.dslTestSpec.param.toneSelectSpec.xmtMap) -
					FLD_OFFSET(dslCommandStruct,param);
				cmd.param.dslTestSpec.param.toneSelectSpec.rcvMap = 
					cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap +
					((cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones + 7) >> 3);
				BcmAdslCoreDiagSelectTones(
					cmd.param.dslTestSpec.param.toneSelectSpec.xmtStartTone,
					cmd.param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones,
					cmd.param.dslTestSpec.param.toneSelectSpec.rcvStartTone,
					cmd.param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones,
					cmd.param.dslTestSpec.param.toneSelectSpec.xmtMap,
					cmd.param.dslTestSpec.param.toneSelectSpec.rcvMap);
			}
			else if (kDslTestExecuteDelay== cmd.param.dslTestSpec.type) {
				BcmAdslCoreSetTestExecutionDelay(cmd.param.dslTestSpec.type,cmd.param.dslTestSpec.param.value);
			}
			else 
			        BcmAdslCoreSetTestMode(cmd.param.dslTestSpec.type);
			break;

		case (kDslStartPhysicalLayerCmd - kFirstDslCommandCode):
			cmd.command = kDslStartPhysicalLayerCmd;
			memcpy(&cmd.param, pCmdData, sizeof(cmd.param.dslModeSpec));
			BcmCoreCommandHandlerSync(&cmd);
			break;

		case (kDslSetXmtGainCmd - kFirstDslCommandCode):
			cmd.param.value = *(long*)pCmdData;
			BcmAdslCoreSetXmtGain (cmd.param.value);
			break;

		case (kDslAfeTestCmd - kFirstDslCommandCode):
			BcmAdslCoreAfeTestMsg(pCmdData);
			break;

		case (kDslPLNControlCmd - kFirstDslCommandCode):
			cmd.command = kDslPLNControlCmd;
			cmd.param.dslPlnSpec.plnCmd = ((long*)pCmdData)[0];
			BcmCoreDpcSyncEnter(); /* protect AdslCoreSharedMemAlloc(), called from User context */			
            if (kDslPLNControlStart == cmd.param.dslPlnSpec.plnCmd) {
				cmd.param.dslPlnSpec.mgnDescreaseLevelPerBin = ((long*)pCmdData)[1];
				cmd.param.dslPlnSpec.mgnDescreaseLevelBand   = ((long*)pCmdData)[2];
			}
            if (kDslPLNControlDefineInpBinTable == cmd.param.dslPlnSpec.plnCmd) {
                cmd.param.dslPlnSpec.nInpBin = ((long*)pCmdData)[3];
		if(NULL!=(pInpBins=AdslCoreSharedMemAlloc(2*cmd.param.dslPlnSpec.nInpBin))){
		  memcpy(pInpBins,((char *)pCmdData) + FLD_OFFSET(dslCommandStruct, param.dslPlnSpec.itaBinPtr) - 0*FLD_OFFSET(dslCommandStruct, param),2*cmd.param.dslPlnSpec.nInpBin);
		  cmd.param.dslPlnSpec.inpBinPtr = pInpBins;
		}
                AdslDrvPrintf (TEXT("cmd.param.dslPlnSpec.inpBinPtr: 0x%x, data= 0x%X 0x%X 0x%X\n"), 
					cmd.param.dslPlnSpec.inpBinPtr,
					cmd.param.dslPlnSpec.inpBinPtr[0],
					cmd.param.dslPlnSpec.inpBinPtr[1],
					cmd.param.dslPlnSpec.inpBinPtr[2]);
		}
            if (kDslPLNControlDefineItaBinTable == cmd.param.dslPlnSpec.plnCmd) {
                cmd.param.dslPlnSpec.nItaBin = ((long*)pCmdData)[5];	       	
		if(NULL!=(pItaBins=AdslCoreSharedMemAlloc(2*cmd.param.dslPlnSpec.nItaBin))){
		  memcpy(pItaBins,((char *)pCmdData) + FLD_OFFSET(dslCommandStruct, param.dslPlnSpec.itaBinPtr) - 0*FLD_OFFSET(dslCommandStruct, param),2*cmd.param.dslPlnSpec.nItaBin);
		  cmd.param.dslPlnSpec.itaBinPtr = pItaBins;
                }
                AdslDrvPrintf (TEXT("DrvDslPLNControlCmd: itaBin=%d\n"), cmd.param.dslPlnSpec.nItaBin);
            
	    }
			BcmCoreCommandHandlerSync(&cmd);
			BcmCoreDpcSyncExit();				
			break;
#ifdef PHY_PROFILE_SUPPORT			
		case (kDslProfileControlCmd - kFirstDslCommandCode):

			cmd.param.value = *(long*)pCmdData;
			cmd.command = diagCmd + kFirstDslCommandCode;
			
			if( kDslProfileEnable == cmd.param.value )
				BcmAdslCoreProfilingStart();
			else
				BcmAdslCoreProfilingStop();
			
			BcmCoreCommandHandlerSync(&cmd);			
			break;
#endif			
		default:
			if (diagCmd < 200) {
				cmd.param.value = *(long*)pCmdData;
				cmd.command = diagCmd + kFirstDslCommandCode;
				BcmCoreCommandHandlerSync(&cmd);
			}
			break;
	}	
}

/*
**
**	Support for AFE test
**
*/

typedef struct {
	char		*pSdram;
	ulong		size;
	ulong		cnt;
	ulong		frameCnt;
} afeTestData;

afeTestData		afeParam = { NULL, 0, 0, 0 };
afeTestData		afeImage = { NULL, 0, 0, 0 };

void BcmAdslCoreIdle(int size) {}
void (*bcmLoadBufferCompleteFunc)(int size) = BcmAdslCoreIdle;

void BcmAdslCoreSetLoadBufferCompleteFunc(void *funcPtr)
{
	bcmLoadBufferCompleteFunc = funcPtr ? funcPtr : BcmAdslCoreIdle;
}

void *BcmAdslCoreGetLoadBufferCompleteFunc(void)
{
	return (BcmAdslCoreIdle == bcmLoadBufferCompleteFunc) ? bcmLoadBufferCompleteFunc : NULL;
}

void BcmAdslCoreAfeTestInit(afeTestData *pAfe, void *pSdram, ulong size)
{
	pAfe->pSdram = pSdram;
	pAfe->size	= size;
	pAfe->cnt	= 0;
	pAfe->frameCnt = 0;
}

void BcmAdslCoreAfeTestStart(afeTestData *pAfe, afeTestData *pImage)
{
	dslCommandStruct	cmd;

	cmd.command = kDslAfeTestCmd;
	cmd.param.dslAfeTestSpec.type = kDslAfeTestPatternSend;
	cmd.param.dslAfeTestSpec.afeParamPtr = afeParam.pSdram;
	cmd.param.dslAfeTestSpec.afeParamSize = afeParam.size;
	cmd.param.dslAfeTestSpec.imagePtr = afeImage.pSdram;
	cmd.param.dslAfeTestSpec.imageSize = afeImage.size;
	BcmCoreCommandHandlerSync(&cmd);
}

void BcmAdslCoreAfeTestAck (afeTestData	*pAfeData, ulong frNum, ulong frRcv)
{
	dslStatusStruct		status;

	// AdslDrvPrintf (TEXT("BcmAdslCoreAfeTestAck. frNumA = %d frNumR = %d\n"), frNum, frRcv);
	status.code = kDslDataAvailStatus;
#if 1
	status.param.dslDataAvail.dataPtr = (void *) (frNum | (frRcv << 8) | DIAG_ACK_FRAME_RCV_PRESENT);
#else
	status.param.dslDataAvail.dataPtr = (void *) frNum;
#endif
	status.param.dslDataAvail.dataLen = DIAG_ACK_LEN_INDICATION;
	BcmAdslCoreDiagStatusSnooper(&status, (void *)&status, sizeof(status.code) + sizeof(status.param.dslDataAvail));
}

void BcmAdslCoreAfeTestMsg(void *pMsg)
{
	dslCommandStruct	*pCmd = (void *) (((char *) pMsg) - sizeof(pCmd->command));
	char				*pSdram;
	afeTestData			*pAfeData;
	ulong				frNum = 0, dataLen = 0, n;

	if (kDslAfeTestPhyRun == pCmd->param.dslAfeTestSpec.type) {
		BcmAdslCoreStart(DIAG_DATA_EYE, AC_FALSE);
		return;
	}

	if ( ((void *) -1 == pCmd->param.dslAfeTestSpec.afeParamPtr) &&
		 ((void *) -1 == pCmd->param.dslAfeTestSpec.imagePtr) ) {

		if (-1 == (pCmd->param.dslAfeTestSpec.afeParamSize & pCmd->param.dslAfeTestSpec.imageSize)) {
			BcmAdslCoreAfeTestStart(&afeParam, &afeImage);
			return;
		}

		switch (pCmd->param.dslAfeTestSpec.type) {
			case kDslAfeTestLoadImage:
			case kDslAfeTestLoadImageOnly:
				BcmAdslCoreAfeTestInit(&afeParam, (void *)ADSL_LMEM_BASE, pCmd->param.dslAfeTestSpec.afeParamSize);
				BcmAdslCoreAfeTestInit(&afeImage, NULL, pCmd->param.dslAfeTestSpec.imageSize);
				BcmAdslCoreStop();
				if ((kDslAfeTestLoadImage == pCmd->param.dslAfeTestSpec.type) &&
					(0 == afeParam.size) && (0 == afeImage.size)) {
					BcmAdslCoreStart(DIAG_DATA_EYE, AC_TRUE);
					if (DIAG_DEBUG_CMD_LOG_DATA == diagDebugCmd.cmd) {
					  BcmAdslCoreDiagStartLog(diagDebugCmd.param1, diagDebugCmd.param2);
					  diagDebugCmd.cmd = 0;
					}
				}
				break;

			case kDslAfeTestPatternSend:
				pSdram = (char*) AdslCoreGetSdramImageStart() + AdslCoreGetSdramImageSize();
				BcmAdslCoreAfeTestInit(&afeParam, pSdram, pCmd->param.dslAfeTestSpec.afeParamSize);
				pSdram += (pCmd->param.dslAfeTestSpec.afeParamSize + 0xF) & ~0xF;
				BcmAdslCoreAfeTestInit(&afeImage, pSdram, pCmd->param.dslAfeTestSpec.imageSize);
				break;

			case kDslAfeTestLoadBuffer:
				n = pCmd->param.dslAfeTestSpec.imageSize;	/* buffer address is here */
				pSdram = (((int) n & 0xF0000000) == 0x10000000) ? ADSL_ADDR_TO_HOST(n) : (void *) n;
				BcmAdslCoreAfeTestInit(&afeParam, pSdram, pCmd->param.dslAfeTestSpec.afeParamSize);
				BcmAdslCoreAfeTestInit(&afeImage, 0, 0);
				if (0 == afeParam.size)
					(*bcmLoadBufferCompleteFunc)(0);
				break;
		}
		return;
	}

	if ((0 == afeParam.size) && (0 == afeImage.size))
		return;

	if (pCmd->param.dslAfeTestSpec.afeParamSize != 0) {
		pAfeData = &afeParam;
		frNum = (ulong) (pCmd->param.dslAfeTestSpec.afeParamPtr) & 0xFF;
		dataLen = pCmd->param.dslAfeTestSpec.afeParamSize;
	}
	else if (pCmd->param.dslAfeTestSpec.imageSize != 0) {
		pAfeData = &afeImage;
		if (NULL == pAfeData->pSdram)
			pAfeData->pSdram = AdslCoreSetSdramImageAddr(((ulong*)ADSL_LMEM_BASE)[2], pAfeData->size);
		frNum = (ulong) (pCmd->param.dslAfeTestSpec.imagePtr) & 0xFF;
		dataLen = pCmd->param.dslAfeTestSpec.imageSize;
	}
	else
		pAfeData = NULL;

	if (NULL != pAfeData) {
		if (frNum != pAfeData->frameCnt) {
			BcmAdslCoreAfeTestAck (pAfeData, (pAfeData->frameCnt - 1) & 0xFF, frNum);
			return;
		}
		n = pAfeData->size - pAfeData->cnt;
		if (dataLen > n)
			dataLen = n;
		pSdram = pAfeData->pSdram + pAfeData->cnt;
		memcpy (
			pSdram, 
			((char *) pCmd) + sizeof(pCmd->command) + sizeof(pCmd->param.dslAfeTestSpec),
			dataLen);
		pAfeData->cnt += dataLen;
		pAfeData->frameCnt = (pAfeData->frameCnt + 1) & 0xFF;
		BcmAdslCoreAfeTestAck (pAfeData, frNum, frNum);
	}

	if ((afeParam.cnt == afeParam.size) && (afeImage.cnt == afeImage.size)) {
		switch (pCmd->param.dslAfeTestSpec.type) {
			case kDslAfeTestLoadImageOnly:
				adslCoreAlwaysReset = AC_FALSE;
				break;
			case kDslAfeTestLoadImage:
				adslCoreAlwaysReset = AC_FALSE;
#ifdef G992_ANNEXC
				pAdslCoreCfgProfile->adslAnnexCParam &= ~(kAdslCfgDemodCapOn | kAdslCfgDemodCap2On);
#else
				pAdslCoreCfgProfile->adslAnnexAParam &= ~(kAdslCfgDemodCapOn | kAdslCfgDemodCap2On);
#endif				
				if (DIAG_DEBUG_CMD_LOG_DATA != diagDebugCmd.cmd) {
					BcmAdslCoreStart(DIAG_DATA_EYE, AC_FALSE);
					BcmAdslCoreSetConnectionParam(&adslCoreConnectionParam, pAdslCoreCfgProfile);
					BcmAdslCoreConnectionReset();
				}
				else {
					BcmAdslCoreSetConnectionParam(&adslCoreConnectionParam, pAdslCoreCfgProfile);
					BcmAdslCoreStart(DIAG_DATA_EYE, AC_FALSE);				
					BcmAdslCoreDiagStartLog(diagDebugCmd.param1, diagDebugCmd.param2);
					diagDebugCmd.cmd = 0;
				}				
				break;
			case kDslAfeTestPatternSend:
				BcmAdslCoreAfeTestStart(&afeParam, &afeImage);
				break;
			case kDslAfeTestLoadBuffer:
				(*bcmLoadBufferCompleteFunc)(afeParam.size);
				break;
		}
		afeParam.cnt = 0;
		afeImage.cnt = 0;
	}
}

void BcmAdslCoreSendBuffer(ulong statusCode, uchar *bufPtr, ulong bufLen)
{
	ULONG				n, frCnt, dataCnt, dataSize;
	dslStatusStruct		*pStatus;
	char				testFrame[LOG_MAX_DATA_SIZE], *pData;

	pStatus = (void *) testFrame;
	dataSize = LOG_MAX_DATA_SIZE - sizeof(pStatus->code) - sizeof(pStatus->param.dslDataAvail) - 16;
	pData = testFrame + sizeof(pStatus->code) + sizeof(pStatus->param.dslDataAvail);

	dataCnt  = 0;
	frCnt	 = 0;
	while (dataCnt < bufLen) {
		n = bufLen - dataCnt;
		if (n > dataSize)
			n = dataSize;

		pStatus->code = statusCode;
		pStatus->param.dslDataAvail.dataPtr = (void *) frCnt;
		pStatus->param.dslDataAvail.dataLen	= n;
		memcpy (pData, bufPtr + dataCnt, n);
		BcmAdslCoreDiagStatusSnooper(pStatus, testFrame, (pData - testFrame) + n);

		dataCnt += n;
		frCnt = (frCnt + 1) & 0xFF;

		if (0 == (frCnt & 7))
			BcmAdslCoreDelay(40);
	}

	pStatus->code = statusCode;
	pStatus->param.dslDataAvail.dataPtr = (void *) frCnt;
	pStatus->param.dslDataAvail.dataLen	= 0;
	BcmAdslCoreDiagStatusSnooper(pStatus, testFrame, pData - testFrame);
}

void BcmAdslCoreSendDmaBuffers(ulong statusCode, int bufNum)
{
	int					len, i;
	dslStatusStruct		*pStatus;
	char				testFrame[LOG_MAX_DATA_SIZE], *pFrameData;
	LogProtoHeader		*pLogHdr;

	/* 
	**	Send the first 2 DMA buffers as raw/DMA data 
	**  If DMA'd block aren't sent in interrupt handler DslDiags needs to 
	**	see this to know the sampling rate 512 vs. 1024
	*/

	pLogHdr = ((LogProtoHeader *) BcmAdslCoreDiagGetDmaDataAddr(0)) - 1;
	BcmAdslCoreDiagWriteStatusData(
		pLogHdr->logCommmand, 
		(void *) (pLogHdr + 1),
		BcmAdslCoreDiagGetDmaDataSize(0), 
		NULL, 0);

	pLogHdr = ((LogProtoHeader *) BcmAdslCoreDiagGetDmaDataAddr(1)) - 1;
	BcmAdslCoreDiagWriteStatusData(
		pLogHdr->logCommmand, 
		(void *) (pLogHdr + 1),
		BcmAdslCoreDiagGetDmaDataSize(1), NULL, 0);

	/*	Send captured samples as statuses for loopback test */

	pStatus = (void *) testFrame;
	pFrameData = testFrame + sizeof(pStatus->code) + sizeof(pStatus->param.dslDataAvail);
	len	  = BcmAdslCoreDiagGetDmaDataSize(0);

	pStatus->code = kDslDataAvailStatus;
	pStatus->param.dslDataAvail.dataPtr = (void *) 0xFFF00000;
	pStatus->param.dslDataAvail.dataLen = len * bufNum;
	BcmAdslCoreDiagStatusSnooper(pStatus, testFrame, pFrameData - testFrame);

	for (i = 0; i < bufNum; i++) {
		pStatus->code = statusCode;
		pStatus->param.dslDataAvail.dataPtr = (void *) (i & 0xFF);
		pStatus->param.dslDataAvail.dataLen = len;
		len	  = BcmAdslCoreDiagGetDmaDataSize(i);
		memcpy (pFrameData, BcmAdslCoreDiagGetDmaDataAddr(i), len);
		BcmAdslCoreDiagStatusSnooper(pStatus, testFrame, (pFrameData - testFrame) + len);

		if (7 == (i & 7))
			BcmAdslCoreDelay(40);
	}

	pStatus->code = statusCode;
	pStatus->param.dslDataAvail.dataPtr = (void *) bufNum;
	pStatus->param.dslDataAvail.dataLen	= 0;
	BcmAdslCoreDiagStatusSnooper(pStatus, testFrame, pFrameData - testFrame);
}

void BcmAdslCoreAfeTestStatus(dslStatusStruct *status)
{
	/* Original kDslDataAvailStatus message has already gone to BcmAdslCoreDiagStatusSnooper */

	BcmAdslCoreSendBuffer(status->code, status->param.dslDataAvail.dataPtr, status->param.dslDataAvail.dataLen);
}

/*
**
**	Support for chip test (not standard PHY DSL interface
**
*/

#ifdef ADSLPHY_CHIPTEST_SUPPORT

/* Memory map for Monitor (System Calls) */

#define MEM_MONITOR_ID              0
#define MEM_MONITOR_PARAM_1         1
#define MEM_MONITOR_PARAM_2         2
#define MEM_MONITOR_PARAM_3         3
#define MEM_MONITOR_PARAM_4         4

#define GLOBAL_EVENT_LOAD_IMAGE     2   /* imageId, imageStart */
#define GLOBAL_EVENT_DUMP_IMAGE     3   /* imageId, imageStart, imageEnd */
#define GLOBAL_EVENT_PR_MSG         4   /* num, msgFormat, msgData */
#define GLOBAL_EVENT_PROBE_IMAGE    5   /* imageId, imageStart, imageEnd */
#define GLOBAL_EVENT_READ_FILE      20  /* filename, bufferStart, size */
#define GLOBAL_EVENT_SIM_STOP       8   /* N/A */

Bool		testCmdInProgress = AC_FALSE;
Bool		testFileReadCmdInProgress = AC_FALSE;
Bool		testPlaybackStopRq = AC_FALSE;
OS_TICKS	tFileReadCmd, tTestCmd;

/*
**	File cache control data
*/

// #define	READ_FILE_CACHE_SIZE		0x4000

void  BcmAdslCoreDebugReadFile(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen);
void  __BcmAdslCoreDebugReadFileCached(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen);
void  BcmAdslCoreDebugReadFileCached(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen);
#ifdef READ_FILE_CACHE_SIZE
void  (*bcmDebugReadFileFunc)(char *fName, ulong rcmd, uchar *pBuf, ulong bLen) = BcmAdslCoreDebugReadFileCached;
#else
void  (*bcmDebugReadFileFunc)(char *fName, ulong rcmd, uchar *pBuf, ulong bLen) = BcmAdslCoreDebugReadFile;
#endif
char  *rdFileName = NULL;
char  *dirFileName = NULL;
ulong dirFileOffset = 0;
char  *cacheFileName = NULL;
uchar *cacheBufPtr = NULL;
ulong cacheSize    = 0;
circBufferStruct	cacheCircBuf;

uchar	*rdBufPtr = NULL;
ulong	rdBufLen = 0;

	
void BcmAdslCoreDebugSendCommand(char *fileName, ushort cmd, ulong offset, ulong len, ulong bufAddr)
{
	DiagProtoFrame		*pDiagFrame;
	DiagTestData		*pTestCmd;
	char				testFrame[LOG_MAX_BUF_SIZE];

	pDiagFrame	= (void *) testFrame;
	pTestCmd	= (void *) pDiagFrame->diagData;
	*(short *)pDiagFrame->diagHdr.logProtoId = *(short *) LOG_PROTO_ID;
	pDiagFrame->diagHdr.logPartyId	= LOG_PARTY_CLIENT;
	pDiagFrame->diagHdr.logCommmand = LOG_CMD_DEBUG;

	pTestCmd->cmd		= cmd;
	pTestCmd->cmdId		= 0;
	pTestCmd->offset	= offset;
	pTestCmd->len		= len;
	pTestCmd->bufPtr	= bufAddr;
	memcpy (pTestCmd->fileName, fileName, DIAG_TEST_FILENAME_LEN);
	BcmAdslCoreDiagWrite(testFrame, (pTestCmd->fileName + DIAG_TEST_FILENAME_LEN + 8)- testFrame);
}

void BcmAdslCoreDebugWriteFile(char *fileName, char cmd, uchar *bufPtr, ulong bufLen)
{
	BcmAdslCoreDebugSendCommand(fileName, cmd, 0, bufLen, (ulong) bufPtr);
	BcmAdslCoreSendBuffer(kDslDataAvailStatus, bufPtr, bufLen);
}

void BcmAdslCoreDebugCompleteCommand(void)
{
	ulong	* volatile pAdslLmem = (void *) ADSL_LMEM_BASE;

	pAdslLmem[MEM_MONITOR_ID] ^= (ulong) -1;
	testCmdInProgress = AC_FALSE;
	adslCoreStarted = AC_TRUE;
#ifndef PHY_BLOCK_TEST
#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
	if (irqSemId != 0 )
		bcmOsSemGive (irqSemId);
#else
	if (irqDpcId != NULL)
		bcmOsDpcEnqueue(irqDpcId);
#endif
#endif
}

int __BcmAdslCoreDebugReadFileComplete(int rdSize, Bool bCompleteCmd)
{
	testFileReadCmdInProgress = AC_FALSE;
	if (testPlaybackStopRq) {
		testPlaybackStopRq = AC_FALSE;
		rdSize = 0;
	}

	if (rdFileName == dirFileName)
		dirFileOffset += rdSize;

	/* 
	** if read problem (rdSize == 0) leave command pending and be ready for
	** DslDiags retransmissions (i.e. leave complete function pointer
	*/

	if (rdSize != 0) {
		BcmAdslCoreSetLoadBufferCompleteFunc(NULL);
		if (bCompleteCmd)
			BcmAdslCoreDebugCompleteCommand();
	}
	return rdSize;
}

void BcmAdslCoreDebugReadFileComplete(int rdSize)
{
	__BcmAdslCoreDebugReadFileComplete(rdSize, true);
}

#ifdef READ_FILE_CACHE_SIZE

void BcmAdslCoreDebugReadFileCacheComplete(int rdSize)
{
	CircBufferWriteUpdateContig(&cacheCircBuf, rdSize);
	rdSize = __BcmAdslCoreDebugReadFileComplete(rdSize, false);

	if ((rdBufLen != 0) && (rdSize != 0))
		__BcmAdslCoreDebugReadFileCached(cacheFileName, DIAG_TEST_CMD_READ, rdBufPtr, rdBufLen);
}

#endif

void __BcmAdslCoreDebugReadFile(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen, void *fnComplPtr)
{
	ulong	offset = 0;

	if (NULL == dirFileName) {
		dirFileName = fileName;
		dirFileOffset = 0;
	}
	if (fileName == dirFileName)
		offset = dirFileOffset;
	rdFileName = fileName;

	bcmOsGetTime(&tFileReadCmd);
	testFileReadCmdInProgress = AC_TRUE;
	BcmAdslCoreSetLoadBufferCompleteFunc(fnComplPtr);
	BcmAdslCoreDebugSendCommand(fileName, readCmd, offset, bufLen, (ulong) bufPtr);
}

void BcmAdslCoreDebugReadFile(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen)
{
	return __BcmAdslCoreDebugReadFile(fileName, readCmd, bufPtr, bufLen, BcmAdslCoreDebugReadFileComplete);
}

/*
**
**	File cache control data
**
*/

#ifdef READ_FILE_CACHE_SIZE

void *BcmAdslCoreDebugCacheAlloc(ulong *pBufSize)
{
	void	*pMem;
	ulong	bufSize = *pBufSize;

	do { 
#if defined(VXWORKS)
	    pMem = (void *) malloc(bufSize);
#elif defined(__KERNEL__)
		pMem = (void *) kmalloc(bufSize, GFP_KERNEL);
#endif
	} while ((NULL == pMem) && ((bufSize >> 1) > 0x2000));
	*pBufSize = bufSize;
	return pMem;
}

ulong BcmAdslCoreDebugReadCacheContig(uchar *bufPtr, ulong bufLen)
{
	ulong		n;

	if (0 == bufLen)
		return 0;
	if (0 == (n = CircBufferGetReadContig(&cacheCircBuf))) 
		return 0;

	if (n > bufLen)
		n = bufLen;

	memcpy (ADSL_ADDR_TO_HOST(bufPtr), CircBufferGetReadPtr(&cacheCircBuf), n);
	CircBufferReadUpdateContig(&cacheCircBuf, n);
	return n;
}

void __BcmAdslCoreDebugReadFileCached(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen)
{
	ulong		n;

	do {
	  n = BcmAdslCoreDebugReadCacheContig(bufPtr, bufLen);
	  bufPtr += n;
	  bufLen -= n;
	  if (0 == bufLen)
		  break;

	  n = BcmAdslCoreDebugReadCacheContig(bufPtr, bufLen);
	  bufPtr += n;
	  bufLen -= n;
	} while (0);

	rdBufPtr = bufPtr;
	rdBufLen = bufLen;

	if (!testFileReadCmdInProgress && ((n = CircBufferGetWriteContig(&cacheCircBuf)) != 0))
		__BcmAdslCoreDebugReadFile(fileName, readCmd, CircBufferGetWritePtr(&cacheCircBuf), n, BcmAdslCoreDebugReadFileCacheComplete);
	if (0 == bufLen)
		BcmAdslCoreDebugCompleteCommand();
}

void BcmAdslCoreDebugReadFileCached(char *fileName, ulong readCmd, uchar *bufPtr, ulong bufLen)
{
	if (NULL == cacheFileName) {
		cacheSize = READ_FILE_CACHE_SIZE;
		cacheBufPtr = BcmAdslCoreDebugCacheAlloc(&cacheSize);
		BcmAdslCoreDiagWriteStatusString("AllocateCache: ptr=0x%X, size=%d", (ulong) cacheBufPtr, cacheSize);
		if (NULL == cacheBufPtr) {
			bcmDebugReadFileFunc = BcmAdslCoreDebugReadFile;
			return BcmAdslCoreDebugReadFile(fileName, readCmd, bufPtr, bufLen);
		}
		cacheFileName = fileName;
		CircBufferInit(&cacheCircBuf, cacheBufPtr, cacheSize);
	}
	else if (cacheFileName != fileName)
		return BcmAdslCoreDebugReadFile(fileName, readCmd, bufPtr, bufLen);

	__BcmAdslCoreDebugReadFileCached(fileName, readCmd, bufPtr, bufLen);
}
#endif

void BcmAdslCoreDebugTestComplete(void)
{
	BcmAdslCoreDebugSendCommand("", DIAG_TEST_CMD_TEST_COMPLETE, 0, 0, 0);
}

void BcmAdslCoreDebugPlaybackStop(void)
{
	testPlaybackStopRq = AC_TRUE;
}

void BcmAdslCoreDebugPlaybackResume(void)
{
	if (testCmdInProgress) {
		BcmAdslCoreSetLoadBufferCompleteFunc(NULL);
#ifdef READ_FILE_CACHE_SIZE
		__BcmAdslCoreDebugReadFileCached(cacheFileName, DIAG_TEST_CMD_READ, rdBufPtr, rdBufLen);
#else
		BcmAdslCoreDebugCompleteCommand();
#endif
	}
}

void BcmAdslCoreDebugReset(void)
{
	if (NULL != cacheBufPtr) {
#if defined(VXWORKS)
		free (cacheBufPtr);
#elif defined(__KERNEL__)
		kfree(cacheBufPtr);
#endif
		cacheBufPtr = NULL;
	}
	cacheFileName = NULL;
#ifdef READ_FILE_CACHE_SIZE
	bcmDebugReadFileFunc = BcmAdslCoreDebugReadFileCached;
#else
	bcmDebugReadFileFunc = BcmAdslCoreDebugReadFile;
#endif
	testCmdInProgress = AC_FALSE;
	dirFileName = NULL;
	dirFileOffset = 0;
	rdFileName = NULL;
	BcmAdslCoreDebugSendCommand("", DIAG_TEST_CMD_TEST_COMPLETE, 0, 0, 0);
}

void BcmAdslCoreDebugTimer(void)
{
	ulong	* volatile pAdslLmem = (void *) ADSL_LMEM_BASE;
	char		*pName;
	OS_TICKS	tMs;

	if (testCmdInProgress && testFileReadCmdInProgress) {
	  bcmOsGetTime(&tMs);
	  tMs = (tMs - tFileReadCmd) * BCMOS_MSEC_PER_TICK;
	  if (tMs > 5000) {
		BcmAdslCoreDiagWriteStatusString("BcmAdslCoreDebugTimer: FileReadCmd timeout time=%d", tMs);
		pName = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_1]);
		(*bcmDebugReadFileFunc)(
			pName,
			DIAG_TEST_CMD_READ,
			(uchar *) pAdslLmem[MEM_MONITOR_PARAM_2],
			pAdslLmem[MEM_MONITOR_PARAM_3]);
	  }
	}

	else if (!testCmdInProgress && BcmAdslCoreIsTestCommand()) {
	  bcmOsGetTime(&tMs);
	  tMs = (tMs - tTestCmd) * BCMOS_MSEC_PER_TICK;
	  if (tMs > 100) {
  		BcmAdslCoreDiagWriteStatusString("BcmAdslCoreDebugTimer: TestCmd=%d time=%d", pAdslLmem[MEM_MONITOR_ID], tMs);
#if defined(VXWORKS) || defined(TARG_OS_RTEMS)
	    if (irqSemId != 0 )
		  bcmOsSemGive (irqSemId);
#else
		if (irqDpcId != NULL)
			bcmOsDpcEnqueue(irqDpcId);
#endif
	  }
	}

}

Bool BcmAdslCoreIsTestCommand(void)
{
	ulong	* volatile pAdslLmem = (void *) ADSL_LMEM_BASE;

	if (testCmdInProgress)
		return AC_FALSE;

	return ((pAdslLmem[MEM_MONITOR_ID] & 0xFFFFFF00) == 0);
}

void BcmAdslCoreProcessTestCommand(void)
{
	ulong	* volatile pAdslLmem = (void *) ADSL_LMEM_BASE;
	char	*pName;
	uchar	cmd;

	if (!BcmAdslCoreIsTestCommand())
		return;
	bcmOsGetTime(&tTestCmd);

#if 0
	printk ("%s: cmdId=%d, p1 = 0x%X, p2 = 0x%X, p3 = 0x%X, p4 = 0x%X\n",
		__FUNCTION__, pAdslLmem[MEM_MONITOR_ID],
		pAdslLmem[MEM_MONITOR_PARAM_1], pAdslLmem[MEM_MONITOR_PARAM_2],
		pAdslLmem[MEM_MONITOR_PARAM_3], pAdslLmem[MEM_MONITOR_PARAM_4]);
#endif

	adslCoreStarted = AC_FALSE;
	switch (pAdslLmem[MEM_MONITOR_ID]) {
		case GLOBAL_EVENT_LOAD_IMAGE:
			pName = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_1]);
			testCmdInProgress = AC_TRUE;
			BcmAdslCoreDebugReadFile(
				pName,
				DIAG_TEST_CMD_LOAD,
				(uchar *) pAdslLmem[MEM_MONITOR_PARAM_2],
				pAdslLmem[MEM_MONITOR_PARAM_3]);
			testCmdInProgress = AC_TRUE;
			break;
		case GLOBAL_EVENT_READ_FILE:
			pName = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_1]);
			testCmdInProgress = AC_TRUE;
			(*bcmDebugReadFileFunc)(
				pName,
				DIAG_TEST_CMD_READ,
				(uchar *) pAdslLmem[MEM_MONITOR_PARAM_2],
				pAdslLmem[MEM_MONITOR_PARAM_3]);
			testCmdInProgress = AC_TRUE;
			break;

		case GLOBAL_EVENT_DUMP_IMAGE:
			cmd = DIAG_TEST_CMD_WRITE;
			goto _write_file;
		case GLOBAL_EVENT_PROBE_IMAGE:
			cmd = DIAG_TEST_CMD_APPEND;
_write_file:
			pName = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_1]);
#if 0
			if (testFileReadCmdInProgress)
				break;

			{
			ushort *pSmp = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_2]);
			int		len = pAdslLmem[MEM_MONITOR_PARAM_3] - pAdslLmem[MEM_MONITOR_PARAM_2] + 1;

			BcmAdslCoreDiagWriteStatusString("WriteCmd%d: ptr=0x%X, len=%d, samples=0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", 
				pAdslLmem[MEM_MONITOR_ID], (ulong) pSmp, len,
				pSmp[0], pSmp[1], pSmp[2], pSmp[3], pSmp[4], pSmp[5]);
			}
#endif
			BcmAdslCoreDebugWriteFile(
				pName,
				cmd,
				ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_2]),
				pAdslLmem[MEM_MONITOR_PARAM_3] - pAdslLmem[MEM_MONITOR_PARAM_2] + 1);
			// BcmAdslCoreDelay(40);
			BcmAdslCoreDebugCompleteCommand();
			break;

		case GLOBAL_EVENT_PR_MSG:
			pName = ADSL_ADDR_TO_HOST(pAdslLmem[MEM_MONITOR_PARAM_2]);
			BcmAdslCoreDiagWriteStatusString(
				pName,
				pAdslLmem[MEM_MONITOR_PARAM_3],
				pAdslLmem[MEM_MONITOR_PARAM_4]);
			BcmAdslCoreDebugCompleteCommand();
			break;

		case GLOBAL_EVENT_SIM_STOP:
			BcmAdslCoreDebugSendCommand("", DIAG_TEST_CMD_TEST_COMPLETE, 0, 0, 0);
			BcmAdslCoreDebugCompleteCommand();
			break;

		default:
			AdslDrvPrintf (TEXT("BcmAdslCoreProcessTestCommand: unknown cmd = 0x%lX\n"), pAdslLmem[MEM_MONITOR_ID]);
			break;
	}
	if (!testCmdInProgress)
		adslCoreStarted = AC_TRUE;
}

#endif /* ADSLPHY_CHIPTEST_SUPPORT */

#ifdef PHY_PROFILE_SUPPORT

#ifdef SYS_RANDOM_GEN
#define PROF_TIMER_SEED_INIT	srand(jiffies)
#define PROF_RANDOM32_GEN 		rand()
#else
LOCAL uint32 profileTimerSeed = 0;
LOCAL uint32 random32(uint32 *seed);

LOCAL uint32 random32(uint32 *seed)	/* FIXME: will get a more sophiscated algorithm later */
{
	*seed = 16807*(*seed);
	*seed += (((*seed) >> 16) & 0xffff);
	return *seed;
}

#define PROF_TIMER_SEED_INIT	(profileTimerSeed = jiffies)
#define PROF_RANDOM32_GEN 		random32(&profileTimerSeed)
#endif

LOCAL void BcmAdslCoreProfileTimerFn(void);

LOCAL int profileStarted = 0;
LOCAL struct timer_list profileTimer;

LOCAL void BcmAdslCoreProfilingStart(void)
{
	init_timer(&profileTimer);
	profileTimer.function = BcmAdslCoreProfileTimerFn;
	profileTimer.expires = 2;	/* 10ms */
	profileTimer.data = 0;
	PROF_TIMER_SEED_INIT;
	add_timer(&profileTimer);
	profileStarted = 1;	
}

void BcmAdslCoreProfilingStop(void)
{
	if(profileStarted) {	
		del_timer(&profileTimer);
		profileStarted = 0;
	}
}


LOCAL void BcmAdslCoreProfileTimerFn(void)
{
	ulong 		cycleCnt0;
	ulong 		randomDelay;
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	
	if(!profileStarted)
		return;
	
	/* Wait some random time within 250us or 250* CyclesPerUs */
	randomDelay = PROF_RANDOM32_GEN;
	
#ifdef PROF_RES_IN_US
	randomDelay = randomDelay % 250;
	cycleCnt0 = BcmAdslCoreGetCycleCount();	
	while (BcmAdslCoreCycleTimeElapsedUs(BcmAdslCoreGetCycleCount(), cycleCnt0) < randomDelay);
#else
	randomDelay = randomDelay % (250 *  ((adslCoreCyclesPerMs+500)/1000));
	cycleCnt0 = BcmAdslCoreGetCycleCount();	
	while ((BcmAdslCoreGetCycleCount() - cycleCnt0) < randomDelay);
#endif
	
	/* Initiate PHY interupt */
#if 1
	pAdslEnum[ADSL_HOSTMESSAGE] = 1;
#else
	printk("%s: randomDelay - %d\n", __FUNCTION__, randomDelay);
#endif

	/* Re-schedule the timer */
	mod_timer(&profileTimer, 1 /* 5ms */);
}
#endif /* PHY_PROFILE_SUPPORT */

