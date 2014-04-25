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
 * AdslCoreMap.h -- Definitions for ADSL core hardware
 *
 * Description:
 *	Definitions for ADSL core hardware
 *
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: AdslCoreMap.h,v 1.1.2.1 2009/11/19 06:39:09 l43571 Exp $
 *
 * $Log: AdslCoreMap.h,v $
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
#ifndef CONFIG_BCM96368
#define RCVINT_BIT              0x00010000
#define XMTINT_BIT              0x01000000
#define MSGINT_BIT             0x00000020
#define MSGINT_MASK_BIT	MSGINT_BIT
#else
#define MSGINT_BIT             0x00000100
#define MSGINT_MASK_BIT	0x00000001
#endif

#if defined(__KERNEL__)
#include <linux/config.h>
#endif
#include "softdsl/AdslCoreDefs.h"

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358)
#define ADSL_ENUM_BASE          0xFFFE3000
#elif defined(CONFIG_BCM96338)
#define ADSL_ENUM_BASE          0xFFFE1000
#elif defined(CONFIG_BCM96368)
#undef ADSL_ENUM_BASE
#define ADSL_ENUM_BASE		0xB0F56000	/* DHIF */
#else
#error  No definition for ADSL_ENUM_BASE
#endif

#ifndef ADSL_LMEM_BASE
 #if !defined(CONFIG_BCM96368)
 #define ADSL_LMEM_BASE          0xFFF00000
 #else
 #define ADSL_LMEM_BASE          0xB0F80000
 #endif
#endif

#if defined(CONFIG_BCM96358)
#define ATM_REG_BASE		0xFFFE2000
#elif !defined(CONFIG_BCM96368)
#define ATM_REG_BASE		0xFFFE4000
#endif

#if !defined(CONFIG_BCM96368)

/* Backplane Enumeration Space Addresses */
#define ADSL_CTL                (0x000 / 4)
#define ADSL_STATUS             (0x004 / 4)
#define ADSL_INTMASK_I          (0x024 / 4)
#define ADSL_INTMASK_F          (0x02c / 4)
#define ADSL_INTSTATUS_I        (0x020 / 4)
#define ADSL_INTSTATUS_F        (0x028 / 4)
#define ADSL_HOSTMESSAGE        (0x300 / 4)

/* DMA Regs, offset from backplane enumeration space address */

#define XMT_CTL_INTR            (0x200 / 4)
#define XMT_ADDR_INTR           (0x204 / 4)
#define XMT_PTR_INTR            (0x208 / 4)
#define XMT_STATUS_INTR         (0x20c / 4)
#define RCV_CTL_INTR            (0x210 / 4)
#define RCV_ADDR_INTR           (0x214 / 4)
#define RCV_PTR_INTR            (0x218 / 4)
#define RCV_STATUS_INTR         (0x21c / 4)
#define XMT_CTL_FAST            (0x220 / 4)
#define XMT_ADDR_FAST           (0x224 / 4)
#define XMT_PTR_FAST            (0x228 / 4)
#define XMT_STATUS_FAST         (0x22c / 4)
#define RCV_CTL_FAST            (0x230 / 4)
#define RCV_ADDR_FAST           (0x234 / 4)
#define RCV_PTR_FAST            (0x238 / 4)
#define RCV_STATUS_FAST         (0x23c / 4)

#define ADSL_CORE_RESET         (0xf98 / 4)
#define ADSL_MIPS_RESET         (0x000 / 4)

#define US_DESCR_TABLE_ADDR		0xA0004000
#define DS_DESCR_TABLE_ADDR		0xA0005000
#define DS_STATUS_SIZE			16

#define HDMA_INIT_FLAG_ADDR		0xA0006000
#define US_CURR_DSCR_ADDR		0xA0006004
#define US_LAST_DSCR_ADDR		0xA0006008
#define DS_CURR_DSCR_ADDR		0xA000600c
#define DS_LAST_DSCR_ADDR		0xA0006010

#else	/* defined(CONFIG_BCM96368) */

#define ADSL_INTMASK_I		(0x03c / 4)
#define ADSL_INTSTATUS_I		ADSL_INTMASK_I
#define ADSL_HOSTMESSAGE	(0x038 / 4)

#define ADSL_CORE_RESET         (0x0e0 / 4)
#define ADSL_MIPS_RESET 		ADSL_CORE_RESET

#endif /* !defined(CONFIG_BCM96368) */
