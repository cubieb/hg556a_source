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
 * AdslSelfTest.c -- ADSL self test module
 *
 * Description:
 *	ADSL self test module
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslSelfTest.c,v 1.1.2.1 2009/11/19 06:39:09 l43571 Exp $
 *
 * $Log: AdslSelfTest.c,v $
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
 * Revision 1.1  2008/06/20 06:21:16  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:46:54  z30370
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
 * Revision 1.1  2004/04/08 21:24:49  ilyas
 * Initial CVS checkin. Version A2p014
 *
 ****************************************************************************/


#include "AdslMibDef.h"
#include "AdslCore.h"
#include "AdslCoreMap.h"
#include "AdslSelfTest.h"

#ifdef ADSL_SELF_TEST

#include "softdsl/SoftDsl.h"
#include "softdsl/BlockUtil.h"
#include "softdsl/AdslXfaceData.h"

#ifdef ADSL_PHY_FILE
#include "AdslFile.h"
#else

#if defined(CONFIG_BCM963x8)
#include "adslcoreTest6348/adsl_selftest_lmem.h"
#include "adslcoreTest6348/adsl_selftest_sdram.h"
#endif

#endif /* ADSL_PHY_FILE */

/* from AdslCore.c */

extern AdslXfaceData	*pAdslXface;

/* Locals */

int							adslStMode = 0;
int							adslStRes  = 0;

#define RESET_ADSL_CORE(pAdslReg) do {		\
	DISABLE_ADSL_CORE(pAdslReg);			\
	ENABLE_ADSL_CORE(pAdslReg);				\
} while (0);

#define ENABLE_ADSL_CORE(pAdslReg) do {		\
	pAdslReg[ADSL_CORE_RESET] = 0;			\
    HOST_MIPS_STALL(20);					\
} while (0)

#define DISABLE_ADSL_CORE(pAdslReg) do {	\
	pAdslReg[ADSL_CORE_RESET] = 1;			\
    HOST_MIPS_STALL(20);					\
} while (0)

#define ENABLE_ADSL_MIPS(pAdslReg) do {		\
    pAdslReg[ADSL_MIPS_RESET] = 0x2;		\
} while (0)

#define HOST_MIPS_STALL(cnt)				\
    do { int _stall_count = (cnt); while (_stall_count--) ; } while (0)

#define	MEM_PATTERN0	0x55555555
#define	MEM_PATTERN1	0xAAAAAAAA

AC_BOOL AdslLmemTestPattern(ulong pattern1, ulong pattern2)
{
	volatile	ulong	*pLmem	= (void *) ADSL_LMEM_BASE;
#if defined(CONFIG_BCM96348)
	volatile	ulong	*pLmemEnd= (void *) (ADSL_LMEM_BASE + 0x22000);
#else
	volatile	ulong	*pLmemEnd= (void *) (ADSL_LMEM_BASE + 0x18000);
#endif
	volatile	static  ulong	val;
	register	ulong	i, newVal;
	register	AC_BOOL	bTestRes = AC_TRUE;

	AdslDrvPrintf(TEXT("ST: LMEM test started, pattern1 = 0x%lX pattern2 = 0x%lX\n"), pattern1, pattern2);
	do {
		*(pLmem + 0) = pattern1;
		*(pLmem + 1) = pattern2;
#if 0
		if (0 == ((long) pLmem & 0x7FFF))
			*pLmem = 1;
#endif
		pLmem += 2;
	} while (pLmem != pLmemEnd);

	pLmem	= (void *) ADSL_LMEM_BASE;

	do {
		for (i = 0; i < 10; i++)
			val = 0;
		newVal = *(pLmem+0);
#if 0
		__asm__ __volatile__ ("nop\n");
		newVal = *(pLmem+0);
#endif
		if (pattern1 != newVal) {
			AdslDrvPrintf(TEXT("ST: LMEM error at address 0x%lX, pattern=0x%08lX, memVal=0x%08lX(0x%08lX), diff=0x%08lX\n"), 
				(long) pLmem, pattern1, newVal, *(pLmem+0), newVal ^ pattern1);
			bTestRes = AC_FALSE;
		}

		for (i = 0; i < 10; i++)
			val = 0;
		newVal = *(pLmem+1);
#if 0
		__asm__ __volatile__ ("nop\n");
		newVal = *(pLmem+1);
#endif
		if (pattern2 != newVal) {
			AdslDrvPrintf(TEXT("ST: LMEM error at address 0x%lX, pattern=0x%08lX, memVal=0x%08lX(0x%08lX), diff=0x%08lX\n"), 
				(long) (pLmem+1), pattern2, newVal, *(pLmem+1), newVal ^ pattern2);
			bTestRes = AC_FALSE;
		}

		pLmem += 2;
	} while (pLmem != pLmemEnd);
	AdslDrvPrintf(TEXT("ST: LMEM test finished\n"));
	return bTestRes;
}

AC_BOOL AdslLmemTest(void)
{
	AC_BOOL		bTestRes;

	bTestRes  = AdslLmemTestPattern(MEM_PATTERN0, MEM_PATTERN1);
	bTestRes &= AdslLmemTestPattern(MEM_PATTERN1, MEM_PATTERN0);
	return bTestRes;
}

int	AdslSelfTestRun(int stMode)
{
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	volatile ulong	*pAdslLMem = (ulong *) ADSL_LMEM_BASE;
	volatile AdslXfaceData	*pAdslX = (AdslXfaceData *) ADSL_LMEM_XFACE_DATA;
	ulong			*pAdslSdramImage;
	ulong	 i, tmp;

	adslStMode = stMode;
	if (0 == stMode) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}

	adslStRes = kAdslSelfTestInProgress;

	/* take ADSL core out of reset */
	RESET_ADSL_CORE(pAdslEnum);

	pAdslEnum[ADSL_INTMASK_I] = 0;
	pAdslEnum[ADSL_INTMASK_F] = 0;

	if (!AdslLmemTest()) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}

#if 0 && defined(CONFIG_BCM96348)
	adslStRes = kAdslSelfTestCompleted | stMode;
	AdslDrvPrintf(TEXT("ST: Completed. res = 0x%lX\n"), adslStRes);
	RESET_ADSL_CORE(pAdslEnum);
	return adslStRes;
#endif		

	/* Copying ADSL core program to LMEM and SDRAM */
#ifdef ADSL_PHY_FILE
	if (!AdslFileLoadImage("/etc/adsl/adsl_test_phy.bin", pAdslLMem, NULL)) {
		adslStRes = kAdslSelfTestCompleted;
		return adslStRes;
	}
	pAdslSdramImage = AdslCoreGetSdramImageStart();
#else
#ifndef  ADSLDRV_LITTLE_ENDIAN
	BlockByteMove ((sizeof(adsl_selftest_lmem)+0xF) & ~0xF, (void *)adsl_selftest_lmem, (uchar *) pAdslLMem);
#else
	for (tmp = 0; tmp < ((sizeof(adsl_selftest_lmem)+3) >> 2); tmp++)
		pAdslLMem[tmp] = ADSL_ENDIAN_CONV_LONG(((ulong *)adsl_selftest_lmem)[tmp]);
#endif
	pAdslSdramImage = AdslCoreSetSdramImageAddr(((ulong *) adsl_selftest_lmem)[2], sizeof(adsl_selftest_sdram));
	BlockByteMove (AdslCoreGetSdramImageSize(), (void *)adsl_selftest_sdram, (void*)pAdslSdramImage);
#endif

	BlockByteClear (sizeof(AdslXfaceData), (void *)pAdslX);
	pAdslX->sdramBaseAddr = (void *) pAdslSdramImage;

	pAdslEnum[ADSL_HOSTMESSAGE] = stMode;

	ENABLE_ADSL_MIPS(pAdslEnum);

	/* wait for ADSL MIPS to start self-test */
	 
	for (i = 0; i < 10000; i++) {
		tmp = pAdslEnum[ADSL_HOSTMESSAGE];
		if (tmp & (kAdslSelfTestInProgress | kAdslSelfTestCompleted)) {
			break;
		}
		HOST_MIPS_STALL(40);
	}
	AdslDrvPrintf(TEXT("ST: Wait to Start. tmp = 0x%lX\n"), tmp);

	do {
		if (tmp & kAdslSelfTestCompleted) {
			adslStRes = tmp & (~kAdslSelfTestInProgress);
			break;
		}
		if (0 == (tmp & kAdslSelfTestInProgress)) {
			adslStRes = tmp;
			break;
		}

		/* wait for ADSL MIPS to finish self-test */

		for (i = 0; i < 10000000; i++) {
			tmp = pAdslEnum[ADSL_HOSTMESSAGE];
			if (0 == (tmp & kAdslSelfTestInProgress))
				break;
			HOST_MIPS_STALL(40);
		}
		adslStRes = tmp;
	} while (0);
	AdslDrvPrintf(TEXT("ST: Completed. tmp = 0x%lX, res = 0x%lX\n"), tmp, adslStRes);

	RESET_ADSL_CORE(pAdslEnum);
	return adslStRes;
}

int AdslSelfTestGetResults(void)
{
	return adslStRes;
}


#endif /* ADSL_SELF_TEST */
