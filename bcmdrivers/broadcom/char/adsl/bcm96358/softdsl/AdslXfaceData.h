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
 * AdslXfaceData.h -- ADSL Core interface data structure
 *
 * Description:
 *	To be included both in SoftDsl and BcmAdslCore driver
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
 * $Id: AdslXfaceData.h,v 1.1.2.1 2009/11/19 06:39:13 l43571 Exp $
 *
 * $Log: AdslXfaceData.h,v $
 * Revision 1.1.2.1  2009/11/19 06:39:13  l43571
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
 * Revision 1.1  2008/01/14 02:47:16  z30370
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
 * Revision 1.9  2004/02/03 02:57:22  ilyas
 * Added PHY feature settings
 *
 * Revision 1.8  2003/07/18 04:50:21  ilyas
 * Added shared buffer for clEoc messages to avoid copying thru command buffer
 *
 * Revision 1.7  2003/02/25 00:46:32  ilyas
 * Added T1.413 EOC vendor ID
 *
 * Revision 1.6  2003/02/21 23:29:13  ilyas
 * Added OEM vendor ID parameter for T1.413 mode
 *
 * Revision 1.5  2002/09/13 21:17:12  ilyas
 * Added pointers to version and build string to OEM interface structure
 *
 * Revision 1.4  2002/09/07 04:16:29  ilyas
 * Fixed HOST to ADSL MIPS SDRAM address translation for relocatable images
 *
 * Revision 1.3  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.2  2002/01/22 19:03:10  khp
 * -put sdramBaseAddr at end of Xface struct
 *
 * Revision 1.1  2002/01/15 06:25:08  ilyas
 * Initial implementation of ADSL core firmware
 *
 ****************************************************************************/

#ifndef	AdslXfaceDataHeader
#define AdslXfaceDataHeader

#include "CircBuf.h"

typedef struct _AdslXfaceData {
	stretchBufferStruct sbSta;
	stretchBufferStruct sbCmd;
	unsigned long		gfcTable[15];
	void				*sdramBaseAddr;
} AdslXfaceData;

/* Shared SDRAM configuration data */

#define	kAdslOemVendorIdMaxSize		8
#define	kAdslOemVersionMaxSize		32
#define	kAdslOemSerNumMaxSize		32
#define	kAdslOemNonStdInfoMaxSize	64

typedef struct _AdslOemSharedData {
	unsigned long		g994VendorIdLen;
	unsigned long		g994XmtNonStdInfoLen;
	unsigned long		g994RcvNonStdInfoLen;
	unsigned long		eocVendorIdLen;
	unsigned long		eocVersionLen;
	unsigned long		eocSerNumLen;
	unsigned char		g994VendorId[kAdslOemVendorIdMaxSize];
	unsigned char		eocVendorId[kAdslOemVendorIdMaxSize];
	unsigned char		eocVersion[kAdslOemVersionMaxSize];
	unsigned char		eocSerNum[kAdslOemSerNumMaxSize];
	unsigned char		g994XmtNonStdInfo[kAdslOemNonStdInfoMaxSize];
	unsigned char		g994RcvNonStdInfo[kAdslOemNonStdInfoMaxSize];
	char				*gDslVerionStringPtr;
	char				*gDslBuildDataStringPtr;
	unsigned long		t1413VendorIdLen;
	unsigned char		t1413VendorId[kAdslOemVendorIdMaxSize];
	unsigned long		t1413EocVendorIdLen;
	unsigned char		t1413EocVendorId[kAdslOemVendorIdMaxSize];
	unsigned long		clEocBufLen;
	unsigned char		*clEocBufPtr;
} AdslOemSharedData;

/* feature list */ 

#define	kAdslPhyAnnexA				0
#define	kAdslPhyAnnexB				1
#define	kAdslPhyAnnexC				2
#define	kAdslPhySADSL				3
#define	kAdslPhyAdsl2				4
#define	kAdslPhyAdslG992p3			4
#define	kAdslPhyAdsl2p				5
#define	kAdslPhyAdslG992p5			5
#define	kAdslPhyAnnexI				6
#define	kAdslPhyAdslReAdsl2			7
#define	kAdslPhyG992p2Init			8
#define	kAdslPhyT1P413				9

#define	AdslFeatureSupported(fa,f)	((fa)[(f) >> 5] & (1 << ((f) & 0x1F)))
#define	AdslFeatureSet(fa,f)		(fa)[(f) >> 5] |= (1 << ((f) & 0x1F))

#endif /* AdslXfaceDataHeader */
