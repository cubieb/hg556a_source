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
 *
 * Copyright (c) 2000-2001  Broadcom Corporation
 * All Rights Reserved
 * No portions of this material may be reproduced in any form without the
 * written permission of:
 *          Broadcom Corporation
 *          16215 Alton Parkway
 *          Irvine, California 92619
 * All information contained in this document is Broadcom Corporation
 * company private, proprietary, and trade secret.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslCoreDefs.h,v 1.1.2.1 2009/11/19 06:39:12 l43571 Exp $
 *
 * $Log: AdslCoreDefs.h,v $
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
 * Revision 1.1  2006/12/20 06:15:44  d55909
 * *** empty log message ***
 *
 * Revision 1.1  2006/11/14 05:46:12  d55909
 * 新建模块
 *
 * Revision 1.3.4.1  2006/10/26 03:21:11  y42304
 * 问题描述：adsl porting(所有修改见总makefile修改注释)
 * 修改人：s48571
 * 时间：2006-10-26
 *
 * Revision 1.3  2006/05/30 09:54:28  l37298
 * 删除ADSL特性
 * 修改文件列表：
 * bcmdrivers\broadcom\char\adsl\impl1\softdsl\AdslCoreDefs.h
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atmapidrv.c
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atmappl.cpp
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atminterface.cpp
 * userapps\broadcom\6510refCode\hmi\relay6348\relayctl.c
 * userapps\broadcom\cfm\util\system\syscall.c
 * userapps\broadcom\cfm\web\cgimain.c
 *
 * Revision 1.2  2006/05/30 09:50:11  l37298
 * bcmdrivers\broadcom\char\adsl\impl1\softdsl\AdslCoreDefs.h
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atmapidrv.c
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atmappl.cpp
 * bcmdrivers\broadcom\char\atmapi\bcm96358\atminterface.cpp
 * userapps\broadcom\6510refCode\hmi\relay6348\relayctl.c
 * userapps\broadcom\cfm\util\system\syscall.c
 * userapps\broadcom\cfm\web\cgimain.c
 *
 * Revision 1.1  2006/04/19 05:11:44  z60003055
 * z60003055：新增BCM306 版本代码基线
 *
 * Revision 1.4  2004/07/20 23:45:48  ilyas
 * Added driver version info, SoftDslPrintf support. Fixed G.997 related issues
 *
 * Revision 1.3  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.2  2004/04/12 23:24:38  ilyas
 * Added default G992P5 PHY definition
 *
 * Revision 1.1  2004/04/08 23:59:15  ilyas
 * Initial CVS checkin
 *
 ****************************************************************************/

#ifndef _ADSL_CORE_DEFS_H
#define _ADSL_CORE_DEFS_H

#if defined(__KERNEL__)
#include <linux/config.h>
#endif

#if defined(CONFIG_BCM96348) || defined(BOARD_bcm96348) || defined(_BCM96348_)
#undef  CONFIG_BCM96348
#define CONFIG_BCM96348
#define	CONFIG_BCM963x8
#endif

#if defined(CONFIG_BCM96338) || defined(BOARD_bcm96338) || defined(_BCM96338_)
#undef  CONFIG_BCM96338
#define	CONFIG_BCM96338
#define	CONFIG_BCM963x8
#endif

#if defined(CONFIG_BCM96358) || defined(BOARD_bcm96358) || defined(_BCM96358_)
#undef	CONFIG_BCM96358
#define	CONFIG_BCM96358
#define	CONFIG_BCM963x8
#endif

#if defined(CONFIG_BCM963x8)
#ifdef ADSL_ANNEXC
#include "../adslcore6348C/adsl_defs.h"
#elif defined(ADSL_ANNEXB)
#include "../adslcore6348B/adsl_defs.h"
#elif defined(ADSL_SADSL)
#include "../adslcore6348SA/adsl_defs.h"
#else
#include "../adslcore6348/adsl_defs.h"
#endif
#endif

#ifdef _WIN32_WCE
#define	ASSERT(a)
#endif

#include "AdslXfaceData.h"

/* adjust some definitions for the HOST */

#undef	GLOBAL_PTR_BIAS
#undef	ADSLCORE_ONLY
#undef	USE_SLOW_DATA 
#undef	USE_FAST_TEXT 
#undef	VP_SIMULATOR 
#undef	bcm47xx 
#undef	ADSL_FRAMER
#undef	ATM
#undef	ATM_I432 
#undef	DSL_OS

#define HOST_ONLY
#define G997_1_FRAMER
#define ADSL_MIB

//#define ADSL_MIBOBJ_PLN

/* definitions for combo PHY (AnnexA(ADSL2) and AnnexB) */
 
#if !(defined(ADSL_SINGLE_PHY) || defined(G992_ANNEXC))


#undef	G992P1_ANNEX_A
#define	G992P1_ANNEX_A
#undef	G992P3
#define	G992P3
#undef	G992P5
#define	G992P5
#define	G992P1_ANNEX_A
#undef	READSL2
#define	READSL2
#undef	G992P1_ANNEX_A_USED_FOR_G992P2
#define	G992P1_ANNEX_A_USED_FOR_G992P2
#undef	T1P413
#define	T1P413

#undef	G992P1_ANNEX_B
#define	G992P1_ANNEX_B

#endif

/* ADSL PHY definition */

typedef struct {
	unsigned long	sdramPageAddr;
	unsigned long	sdramImageAddr;
	unsigned long	sdramImageSize;
	unsigned long	sdramPhyImageAddr;
	unsigned short	fwType;
	unsigned short	chipType;
	unsigned short	mjVerNum;
	unsigned short	mnVerNum;
	char			*pVerStr;
	unsigned long	features[4];
} adslPhyInfo;
extern adslPhyInfo	adslCorePhyDesc;

/* chip list */ 

#define	kAdslPhyChipMjMask			0xFF00
#define	kAdslPhyChipMnMask			0x00FF
#define	kAdslPhyChipUnknown			0
#define	kAdslPhyChip6345			0x100
#define	kAdslPhyChip6348			0x200
#define	kAdslPhyChipRev0			0
#define	kAdslPhyChipRev1			1
#define	kAdslPhyChipRev2			2
#define	kAdslPhyChipRev3			3
#define	kAdslPhyChipRev4			4
#define	kAdslPhyChipRev5			5

#define	ADSL_PHY_SUPPORT(f)			AdslFeatureSupported(adslCorePhyDesc.features,f)
#define	ADSL_PHY_SET_SUPPORT(p,f)	AdslFeatureSet((p)->features,f)

/* ADSL Driver to/from PHY address and data conversion macros */

#ifdef  ADSLDRV_LITTLE_ENDIAN
#define	ADSL_ENDIAN_CONV_LONG(x)	( ((x) << 24) | (((x) << 8) & 0x00FF0000) | (((x) >> 8) & 0x0000FF00) | ((unsigned long)(x) >> 24) )
#define	ADSL_ENDIAN_CONV_SHORT(x)	( ((x) << 8) | ((unsigned short)(x) >> 8) )
#define ADSL_ENDIAN_CONV_2SHORTS(x)	( ((x) << 16) | ((unsigned long)(x) >> 16) )
#else
#define	ADSL_ENDIAN_CONV_LONG(x)	x
#define	ADSL_ENDIAN_CONV_SHORT(x)	x
#define ADSL_ENDIAN_CONV_2SHORTS(x)	x
#endif

#ifndef ADSL_LMEM_BASE
 #if !defined(CONFIG_BCM96368)
 #define ADSL_LMEM_BASE          0xFFF00000
 #else
 #define ADSL_LMEM_BASE          0xB0F80000
#endif
#endif

#ifndef FLATTEN_ADDR_ADJUST
#define FLATTEN_ADDR_ADJUST			ADSL_LMEM_BASE
#endif

#ifndef ADSL_PHY_XFACE_OFFSET
#define ADSL_PHY_XFACE_OFFSET			0x00017F90
#endif
#define ADSL_LMEM_XFACE_DATA			(ADSL_LMEM_BASE | ADSL_PHY_XFACE_OFFSET)

#ifndef ADSL_PHY_SDRAM_START
#define ADSL_PHY_SDRAM_START			0x10000000
#endif
#ifndef ADSL_PHY_SDRAM_BIAS
#define ADSL_PHY_SDRAM_BIAS				0x00040000
#endif
#define	ADSL_PHY_SDRAM_START_4			(ADSL_PHY_SDRAM_START + ADSL_PHY_SDRAM_BIAS)

#ifndef	ADSL_PHY_SDRAM_PAGE_SIZE
#define ADSL_PHY_SDRAM_PAGE_SIZE		0x00080000
#endif

/*Start modify : s48571 2006-10-26 for adsl porting*/
#ifdef CONFIG_BCM_VDSL
/*Start modify : l37298 2006-05-30 for deleting adsl feature*/
#define ADSL_SDRAM_IMAGE_SIZE           (0)
/*End modify : l37298 2006-05-30 for deleting adsl feature*/
#else
#ifdef ADSL_PHY_SDRAM_BIAS
#define ADSL_SDRAM_IMAGE_SIZE			(ADSL_PHY_SDRAM_PAGE_SIZE - ADSL_PHY_SDRAM_BIAS)
#else
#define ADSL_SDRAM_IMAGE_SIZE			(256*1024)
#endif
#endif
/*End modify : s48571 2006-10-26 for adsl porting*/

#ifndef	ADSL_PHY_SDRAM_LINK_OFFSET
#define ADSL_PHY_SDRAM_LINK_OFFSET		0x00040000
#endif

#define ADSL_SDRAM_TOTAL_SIZE			0x00800000
#define ADSL_SDRAM_HOST_MIPS_DEFAULT	(0xA0000000 | (ADSL_SDRAM_TOTAL_SIZE - ADSL_PHY_SDRAM_PAGE_SIZE + ADSL_PHY_SDRAM_BIAS))

#define ADSLXF							((AdslXfaceData *) ADSL_LMEM_XFACE_DATA)

#define ADSL_MIPS_LMEM_ADDR(a)	(((ulong)(a) & 0x19000000) == 0x19000000)
#define	SDRAM_ADDR_TO_HOST(a)	((void *) ((ulong)(a) - adslCorePhyDesc.sdramPhyImageAddr + \
	(ADSLXF->sdramBaseAddr ? (unsigned long) ADSLXF->sdramBaseAddr : ADSL_SDRAM_HOST_MIPS_DEFAULT)))
#define	SDRAM_ADDR_TO_ADSL(a)	(a)
#define ADSL_ADDR_TO_HOST(addr)		ADSL_MIPS_LMEM_ADDR(addr) ? (void *) ((ulong) (addr) | FLATTEN_ADDR_ADJUST) : SDRAM_ADDR_TO_HOST(addr)


#ifndef DEBUG
#define DEBUG	0
#endif

#ifndef __SoftDslPrintf
void __SoftDslPrintf(void *gDslVars, char *fmt, int argNum, ...);
#endif

#endif

