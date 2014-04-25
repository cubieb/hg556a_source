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
 * AdslCore.h -- Internal definitions for ADSL core driver
 *
 * Description:
 *	Internal definitions for ADSL core driver
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslCore.h,v 1.1.2.1 2009/11/19 06:39:09 l43571 Exp $
 *
 * $Log: AdslCore.h,v $
 * Revision 1.1.2.1  2009/11/19 06:39:09  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:40:53  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:15  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:46:54  z30370
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
 * Revision 1.4  2004/05/06 03:24:02  ilyas
 * Added power management commands
 *
 * Revision 1.3  2004/04/30 17:58:01  ilyas
 * Added framework for GDB communication with ADSL PHY
 *
 * Revision 1.2  2004/04/13 01:07:19  ilyas
 * Merged the latest ADSL driver changes for RTEMS
 *
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/

#undef	ADSLPHY_CHIPTEST_SUPPORT
#undef	PHY_BLOCK_TEST

#ifdef PHY_BLOCK_TEST
#define	ADSLPHY_CHIPTEST_SUPPORT
#endif

typedef unsigned char		AC_BOOL;
#define	AC_TRUE				1
#define	AC_FALSE			0

/* Adsl Core events */

#define	ACEV_LINK_UP							1
#define	ACEV_LINK_DOWN							2
#define	ACEV_G997_FRAME_RCV						4
#define	ACEV_G997_FRAME_SENT					8
#define	ACEV_SWITCH_RJ11_PAIR					0x10
#define ACEV_G994_NONSTDINFO_RECEIVED			0x20
#define ACEV_LINK_POWER_L3						0x40

typedef struct _AdslCoreConnectionRates {
    int		fastUpRate;
    int		fastDnRate;
    int		intlUpRate;
    int		intlDnRate;
} AdslCoreConnectionRates;

void AdslCoreSetL2Timeout(unsigned long val);
void AdslCoreIndicateLinkPowerStateL2(void);
AC_BOOL AdslCoreSetSDRAMBaseAddr(void *pAddr);
void	*AdslCoreGetSDRAMBaseAddr(void);
AC_BOOL AdslCoreSetVcEntry (int gfc, int port, int vpi, int vci, int pti_clp);
void	AdslCoreSetStatusCallback(void *pCallback);
void	*AdslCoreGetStatusCallback(void);
void	AdslCoreSetShowtimeMarginMonitoring(long showtimeMargin, int monEnabled, long lomTimeSec);
AC_BOOL AdslCoreSetStatusBuffer(void *bufPtr, int bufSize);
AC_BOOL AdslCoreLinkState (void);
int		AdslCoreLinkStateEx (void);
void	AdslCoreGetConnectionRates (AdslCoreConnectionRates *pAdslRates);
int		AdslCoreSetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
int		AdslCoreGetObjectValue (char *objId, int objIdLen, char *dataBuf, long *dataBufLen);
AC_BOOL AdslCoreBertStart(unsigned long totalBits);
AC_BOOL AdslCoreBertStop(void);
void	AdslCoreBertStartEx(unsigned long bertSec);
void	AdslCoreBertStopEx(void);
void	AdslCoreResetStatCounters(void);
int		AdslCoreGetOemParameter (int paramId, void *buf, int len);
int		AdslCoreSetOemParameter (int paramId, void *buf, int len);
char	*AdslCoreGetVersionString(void);
char	*AdslCoreGetBuildInfoString(void);
int		AdslCoreGetSelfTestMode(void);
void	AdslCoreSetSelfTestMode(int stMode);
int		AdslCoreGetSelfTestResults(void);
void	*AdslCoreSetSdramImageAddr(unsigned long lmem2, unsigned long sdramSize);
int		AdslCoreFlattenCommand(void *cmdPtr, void *dstPtr, unsigned long nAvail);
void	AdslCoreSetL3(void);
void	AdslCoreSetL0(void);

void	*AdslCoreGetPhyInfo(void);
void	*AdslCoreGetSdramPageStart(void);
void	*AdslCoreGetSdramImageStart(void);
unsigned long AdslCoreGetSdramImageSize(void);
void	AdslCoreSharedMemInit(void);
void	*AdslCoreSharedMemAlloc(long size);
void	AdslCoreSharedMemFree(void *p);
void	*AdslCoreGdbAlloc(long size);
void	AdslCoreGdbFree(void *p);

AC_BOOL AdslCoreInit(void);
void	AdslCoreUninit(void);
AC_BOOL AdslCoreHwReset(AC_BOOL bCoreReset);
void AdslCoreStop(void);

AC_BOOL AdslCoreIntHandler(void);
int	AdslCoreIntTaskHandler(int nMaxStatus);
void	AdslCoreTimer (unsigned long tMs);
AC_BOOL AdslCoreCommandHandler(void *cmdPtr);

AC_BOOL AdslCoreG997SendData(void *buf, int len);

int AdslCoreG997FrameReceived(void);
void	*AdslCoreG997FrameGet(int *pLen);
void	*AdslCoreG997FrameGetNext(int *pLen);
void	AdslCoreG997FrameFinished(void);

void	BcmAdslCoreNotify(long acEvent);
AC_BOOL BcmAdslCoreLogEnabled(void);
int		BcmAdslCoreDiagWrite(void *pBuf, int len);
void	BcmAdslCoreDiagClose(void);

/* debug macros */

#ifdef VXWORKS
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf		printf
#elif defined(__KERNEL__)
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf		printk
#define	PHY_PROFILE_SUPPORT
#elif defined(TARG_OS_RTEMS)
#define	TEXT(__str__)		__str__
#include "BcmOs.h"
#define AdslDrvPrintf           BcmOsEventLog 
#elif defined(_WIN32_WCE)
void NKDbgPrintfW(void *pwStr, ...);
#ifndef TEXT
#define TEXT(quote)			L##quote
#endif
#define	AdslDrvPrintf		NKDbgPrintfW
#elif defined(_CFE_)
#include "lib_printf.h"
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf		printf
#endif
