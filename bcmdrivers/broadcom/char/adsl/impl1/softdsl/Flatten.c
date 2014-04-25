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
 * Flatten.c -- Flatten/Unflatten command/status
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: Flatten.c,v 1.1 2008/08/25 06:41:02 l65130 Exp $
 *
 * $Log: Flatten.c,v $
 * Revision 1.1  2008/08/25 06:41:02  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:21  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:20  z30370
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
 * Revision 1.76  2005/11/23 00:53:46  ilyas
 * Added parameter to ExecutionDelay test command
 *
 * Revision 1.75  2005/10/19 18:53:13  ilyas
 * Added margin level parameters (spec) to PLNStart command
 *
 * Revision 1.74  2005/10/12 20:59:10  ilyas
 * ADSL driver change: Pass data pointer for long statuses iso data copying
 *
 * Revision 1.73  2005/09/08 21:00:10  ilyas
 * Improved showtime SNR status conversion
 *
 * Revision 1.72  2005/09/08 20:57:39  ilyas
 * For Adsl driver: Fixed address conversion for print data status, ifdef'ed out PHY only code from driver build
 *
 * Revision 1.71  2005/02/11 06:47:09  ilyas
 * Added support for DslOs
 *
 * Revision 1.70  2004/09/28 23:14:54  mprahlad
 * Remove dependence on G992P3 flag, for reporting ChannelResponse (so that ADSL1
 * builds can also report Hlog/HLin)
 *
 * Revision 1.69  2004/09/02 01:27:29  ilyas
 * Fixed parameter passing for SADSL
 *
 * Revision 1.68  2004/06/24 03:12:31  ilyas
 * Use un-cached read pinter in status write
 *
 * Revision 1.67  2004/06/02 22:24:10  ilyas
 * Added ATM counters for G.992.3
 *
 * Revision 1.66  2004/05/20 00:58:23  ilyas
 * Pass G992p3ConnectInfo via status buffer
 *
 * Revision 1.65  2004/05/13 19:14:18  ilyas
 * Added new statuses for ADSL2
 *
 * Revision 1.64  2004/05/01 01:33:37  ilyas
 * Simplified PowerMgr command/status processing
 *
 * Revision 1.63  2004/05/01 01:12:21  ilyas
 * Added power management command and statuses
 *
 * Revision 1.62  2004/03/29 23:06:39  ilyas
 * Added status for BG table update
 *
 * Revision 1.61  2004/02/18 20:46:33  ilyas
 * Fixed macro definition to build with DslDiags
 *
 * Revision 1.60  2004/02/05 03:11:59  ilyas
 * Integrated changes from ADSL driver
 *
 * Revision 1.59  2004/02/04 19:42:24  linyin
 * Support adsl2plus
 *
 * Revision 1.58  2004/02/03 19:14:40  gsyu
 * Support parameter passing for G992P5
 *
 * Revision 1.57  2004/01/19 19:39:06  ilyas
 * #ifdef'ed address conversion for OLR table
 *
 * Revision 1.56  2004/01/17 00:21:53  ilyas
 * Added commands and statuses for OLR
 *
 * Revision 1.55  2003/11/20 01:02:47  yongbing
 * Merge ADSL2 functionalities into Annex A branch
 *
 * Revision 1.54  2003/11/01 01:05:47  linyin
 * Add annexI carrierInfo
 *
 * Revision 1.53  2003/10/22 03:23:32  ilyas
 * Added QuietLine noise
 *
 * Revision 1.52  2003/10/22 03:19:32  ilyas
 * Added QuietLine noise status support
 *
 * Revision 1.51  2003/10/22 01:04:49  ilyas
 * Pass buffer pointer for channel response statuses
 *
 * Revision 1.50  2003/10/16 00:15:20  ilyas
 * Added G992p3 parameters
 *
 * Revision 1.49  2003/08/22 22:40:24  liang
 * The #endif statements in last checkin were in the wrong locations.
 *
 * Revision 1.48  2003/08/12 23:16:00  khp
 * - for Haixiang: added support for ADSL_MARGIN_TWEAK_TEST
 *
 * Revision 1.47  2003/07/18 04:53:46  ilyas
 * Fixed flattening of clEoc messages
 *
 * Revision 1.46  2003/06/07 00:38:36  ilyas
 * Added conditions to compile for DslDiags
 *
 * Revision 1.45  2003/06/07 00:00:08  ilyas
 * Added flattening of command and status for AFE standalone tests
 * Added support for little endian Host MIPS (in ADSL driver) (for WinCE)
 *
 * Revision 1.44  2003/03/10 20:35:09  ilyas
 * Fixed more compiler warnings
 *
 * Revision 1.43  2003/03/10 20:33:01  ilyas
 * Fixed compiler warning
 *
 * Revision 1.42  2003/03/06 01:05:18  ilyas
 * Added support for SetStatusBuffer command
 *
 * Revision 1.41  2003/02/27 06:33:03  ilyas
 * Improved free space checking in command buffer (became a problem with
 * 2 commands SetXmtgain and StartPhy)
 *
 * Revision 1.40  2003/01/11 01:27:07  ilyas
 * Improved checking for available space in status buffer
 *
 * Revision 1.39  2002/12/12 03:12:33  ilyas
 * Fixed kDslOemDataAddrStatus unflatten bug
 *
 * Revision 1.38  2002/10/04 23:23:46  liang
 * Flatten and unflatten AOC exchange rcv/xmt info data for bitswap.
 *
 * Revision 1.37  2002/09/15 04:36:56  ilyas
 * Fixed compiler warning
 *
 * Revision 1.36  2002/09/15 04:27:42  ilyas
 * Fixed copying of showtime counters
 *
 * Revision 1.35  2002/09/13 21:17:12  ilyas
 * Added pointers to version and build string to OEM interface structure
 *
 * Revision 1.34  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.33  2002/08/02 22:22:08  liang
 * Enable G.lite code when G.dmt annex A is used for G.lite.
 *
 * Revision 1.32  2002/07/11 03:16:48  ilyas
 * Fixed Flatten bug for ShowtimeSNR status
 *
 * Revision 1.31  2002/07/11 01:30:59  ilyas
 * Changed status for ShowtimeMargin reporting
 *
 * Revision 1.30  2002/07/09 19:22:12  ilyas
 * Added support for ShowtimeSNRMargin status
 *
 * Revision 1.29  2002/07/02 01:00:08  ilyas
 * Added support for kDslTestCmd
 *
 * Revision 1.28  2002/06/15 05:18:44  ilyas
 * Support format string from SDRAM in SoftDslPrintf
 *
 * Revision 1.27  2002/05/31 00:55:10  liang
 * Fix Linux OS compiler warning.
 *
 * Revision 1.26  2002/05/30 19:59:40  ilyas
 * Added status for ADSL MIPS exception
 *
 * Revision 1.25  2002/05/17 18:00:27  liang
 * Add codes for Annex A S=1/2.
 *
 * Revision 1.24  2002/04/02 10:06:09  ilyas
 * Added BERT statuses
 *
 * Revision 1.23  2002/04/01 19:06:38  linyin
 * reverse the last checkin
 *
 * Revision 1.21  2002/03/07 22:08:14  georgep
 * Use appropriate variables for annex A, B or C
 *
 * Revision 1.20  2002/02/08 04:47:59  ilyas
 * Completed LOG file support
 *
 * Revision 1.19  2002/01/30 07:19:06  ilyas
 * Moved showtime code to LMEM
 *
 * Revision 1.18  2002/01/28 21:53:42  ilyas
 * Added progress pointer to HostDma handlers
 *
 * Revision 1.17  2002/01/19 01:03:33  ilyas
 * Fixed compiler warning
 *
 * Revision 1.16  2002/01/17 04:44:39  ilyas
 * Moved address adjustment to CommandWrite (was in CommandRead before)
 *
 * Revision 1.15  2002/01/16 19:59:20  ilyas
 * Changed error condition handling
 *
 * Revision 1.14  2002/01/15 22:30:15  ilyas
 * Extended StatusWrite to handle read pointer (updated by the Host MIPS) via
 * uncached memory address
 *
 * Revision 1.13  2002/01/10 07:18:23  ilyas
 * Added status for printf (mainly for ADSL core debugging)
 *
 * Revision 1.12  2002/01/02 19:12:15  liang
 * Make sure number of bytes for BlockByteMove is a multiple of 4.
 *
 * Revision 1.11  2001/12/21 01:53:35  ilyas
 * Fixed compiler warning
 *
 * Revision 1.10  2001/12/21 00:29:17  ilyas
 * Fixed G992CodingParams flatten problem
 *
 * Revision 1.9  2001/12/13 02:38:52  ilyas
 * Added support for G997 and Clear EOC
 *
 * Revision 1.8  2001/11/30 05:56:37  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.7  2001/10/09 22:35:14  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.1.2.3  2001/10/04 00:54:19  liang
 * Flaten/Unflaten G994 xmt & rcv msg, TEQ coef, and PSD info.
 *
 * Revision 1.1.2.2  2001/10/03 01:45:04  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.6  2001/09/21 19:47:05  ilyas
 * Fixed compiler warnings for VxWorks build
 *
 * Revision 1.5  2001/08/30 18:52:23  ilyas
 * Commented out unused code for ADSLCORE_ONLY
 *
 * Revision 1.4  2001/08/29 19:02:58  ilyas
 * Fixed compiling and linking problems for G992 targets
 *
 * Revision 1.3  2001/08/29 02:56:01  ilyas
 * Added tests for flattening/unflatenning command and statuses (dual mode)
 *
 * Revision 1.2  2001/08/08 01:19:17  ilyas
 * Added support for EC coefficients status
 *
 * Revision 1.1  2001/04/24 21:41:21  ilyas
 * Implemented status flattening/unflattaning to transfer statuses between
 * modules asynchronously through the circular buffer
 *
 *****************************************************************************/

#include "SoftDsl.h"
#include "Flatten.h"
#include "BlockUtil.h"

#include <stdarg.h>

#ifdef G997_1_FRAMER
#include "DslFramer.h"
#endif

#ifndef ADSL_PHY_SUPPORT
#define	ADSL_PHY_SUPPORT(f)		(1)
#endif

#ifndef ADSLCORE_ONLY

#ifdef FLATTEN_ADDR_ADJUST
extern	long	flattenAddrAdj;
#endif

Public int FlattenCommand (dslCommandStruct *cmd, ulong *dstPtr, ulong nAvail)
{
#define	CHECK_CMD_BUF_AVAIL(szReq,szAvail)	if ((long)(szReq) > (long)(szAvail)) return 0;

	ulong	*dstPtr0  = dstPtr;
	ulong	statAvail = (nAvail >> 2);

	*dstPtr++ = cmd->command;
	switch (cmd->command) {
		case kDslStartPhysicalLayerCmd:
#ifdef G992_ATUC
			CHECK_CMD_BUF_AVAIL(27,statAvail-1);
#else
			CHECK_CMD_BUF_AVAIL(16,statAvail-1);
#endif
			*dstPtr++ = (long)cmd->param.dslModeSpec.direction;
			*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.modulations;
			*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.minDataRate;
			*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.maxDataRate;
#if	defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
			if (ADSL_PHY_SUPPORT(kAdslPhyG992p2Init)) {
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMinCarr;
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMaxCarr;
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMinCarr;
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMaxCarr;
			}
#endif
#ifdef	G992P1_ANNEX_A
			if (ADSL_PHY_SUPPORT(kAdslPhyAnnexA)) {
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.subChannelInfo;
			}
#endif
#ifdef	G992P1_ANNEX_B
			if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB) || ADSL_PHY_SUPPORT(kAdslPhySADSL)) {
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexB;
			}
#endif
#ifdef	G992_ANNEXC
			*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC;
#endif
#ifdef	G992P1_ANNEX_I
			*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMaxCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMinCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexI;
#endif
#ifdef	G992P5
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
				*dstPtr++ = (long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr;
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.subChannelInfop5;
			}
#endif
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.features;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.auxFeatures;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.demodCapabilities;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.noiseMargin;
#ifdef G992_ATUC /* last please, for ATU-C only */
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.xmtRSf;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.xmtRS;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.xmtS;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.xmtD;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.rcvRSf;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.rcvRS;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.rcvS;
			*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.rcvD;
#endif
#ifdef G992P3
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = (long)SDRAM_ADDR_TO_ADSL(cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA);
#else
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA;
#endif
			}
#endif
#ifdef G992P5
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = (long)SDRAM_ADDR_TO_ADSL(cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA);
#else
				*dstPtr++ =	(long)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA;
#endif
			}
#endif
			break;

		case kDslDiagSetupCmd:
			CHECK_CMD_BUF_AVAIL(4,statAvail-1);
			*dstPtr++ =	(long)cmd->param.dslDiagSpec.setup;
			*dstPtr++ =	(long)cmd->param.dslDiagSpec.eyeConstIndex1;
			*dstPtr++ =	(long)cmd->param.dslDiagSpec.eyeConstIndex2;
			*dstPtr++ =	(long)cmd->param.dslDiagSpec.logTime;
			break;

		case kDslTestCmd:
			*dstPtr++ =	(long)cmd->param.dslTestSpec.type;
			if (kDslTestToneSelection == cmd->param.dslTestSpec.type) {
				long	len1, len2;

				len1 = (cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones + 7) >> 3;
				len2 = (cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones + 7) >> 3;

				CHECK_CMD_BUF_AVAIL(4+((len1 + len2 + 3) >> 2),statAvail-2);

				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.toneSelectSpec.xmtStartTone;
				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones;
				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.toneSelectSpec.rcvStartTone;
				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones;

				BlockByteMove ((len1 + 3) & ~0x3, cmd->param.dslTestSpec.param.toneSelectSpec.xmtMap, (char*)dstPtr);
				BlockByteMove ((len2 + 3) & ~0x3, cmd->param.dslTestSpec.param.toneSelectSpec.rcvMap, ((char*)dstPtr) + len1);
				dstPtr += (len1 + len2 + 3) >> 2;
			}
			else if (kDslTestExecuteDelay==cmd->param.dslTestSpec.type) {
		        *dstPtr++ =	(long) cmd->param.dslTestSpec.param.value;
			}
#ifdef ADSL_MARGIN_TWEAK_TEST
			else if (kDslTestMarginTweak == cmd->param.dslTestSpec.type) {
				long	len1;

				len1 = (cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers + 3) >> 2;
				CHECK_CMD_BUF_AVAIL(2+len1,statAvail-2);

				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.marginTweakSpec.extraPowerRequestQ4dB;
				*dstPtr++ =	(long)cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers;
				BlockByteMove (len1 << 2, cmd->param.dslTestSpec.param.marginTweakSpec.marginTweakTableQ4dB, (char*)dstPtr);
				dstPtr += len1;
			}
#endif
			break;

		case kDslSetStatusBufferCmd:
			*dstPtr++ = (ulong) cmd->param.dslStatusBufSpec.pBuf;
			*dstPtr++ = cmd->param.dslStatusBufSpec.bufSize;
			break;

		case kDslAfeTestCmd:
			CHECK_CMD_BUF_AVAIL(5,statAvail-1);
			*dstPtr++ = (ulong) cmd->param.dslAfeTestSpec.type;
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (ulong) SDRAM_ADDR_TO_ADSL(cmd->param.dslAfeTestSpec.afeParamPtr);
#else
			*dstPtr++ = (ulong) cmd->param.dslAfeTestSpec.afeParamPtr;
#endif
			*dstPtr++ = (ulong) cmd->param.dslAfeTestSpec.afeParamSize;
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (ulong) SDRAM_ADDR_TO_ADSL(cmd->param.dslAfeTestSpec.imagePtr);
#else
			*dstPtr++ = (ulong) cmd->param.dslAfeTestSpec.imagePtr;
#endif
			*dstPtr++ = (ulong) cmd->param.dslAfeTestSpec.imageSize;
			break;

		case kDslPLNControlCmd:
			*dstPtr++ = cmd->param.dslPlnSpec.plnCmd;
			if (kDslPLNControlStart == cmd->param.dslPlnSpec.plnCmd) {
				dstPtr[0] = cmd->param.dslPlnSpec.mgnDescreaseLevelPerBin;
				dstPtr[1] = cmd->param.dslPlnSpec.mgnDescreaseLevelBand;
				dstPtr+=2;
				
			}
			else if(kDslPLNControlDefineInpBinTable==cmd->param.dslPlnSpec.plnCmd) {
			        *dstPtr++=cmd->param.dslPlnSpec.nInpBin;
				*dstPtr++ = (ulong) cmd->param.dslPlnSpec.inpBinPtr;
			}
			else if (kDslPLNControlDefineItaBinTable==cmd->param.dslPlnSpec.plnCmd) {
			        *dstPtr++ = cmd->param.dslPlnSpec.nItaBin;
				*dstPtr++ = (ulong) cmd->param.dslPlnSpec.itaBinPtr;
			}
			  break;

#ifdef G997_1
		case kDslSendEocCommand:
			*dstPtr++ = cmd->param.value;
			if (cmd->param.value >= kDslClearEocFirstCmd)
				{
				*dstPtr++ = cmd->param.dslClearEocMsg.msgType;
				if (cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					long	len;

					len = cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					CHECK_CMD_BUF_AVAIL((len + 3) >> 2,statAvail-3);

					BlockByteMove (len, cmd->param.dslClearEocMsg.dataPtr, (char*)dstPtr);
					dstPtr += (len + 3) >> 2;
					}
				else
#ifdef FLATTEN_ADDR_ADJUST
					*dstPtr++ = (ulong) SDRAM_ADDR_TO_ADSL(cmd->param.dslClearEocMsg.dataPtr);
#else
					*dstPtr++ = (ulong) cmd->param.dslClearEocMsg.dataPtr;
#endif
				}
			break;
#endif

#ifdef G992P3
		case kDslOLRRequestCmd:
			CHECK_CMD_BUF_AVAIL(6,statAvail-1);
			*dstPtr++ = cmd->param.dslOLRRequest.msgType;
			*dstPtr++ = cmd->param.dslOLRRequest.nCarrs;
			BlockByteMove (12, (void *) cmd->param.dslOLRRequest.L, (char*) dstPtr);
			dstPtr += 3;
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (ulong) SDRAM_ADDR_TO_ADSL(cmd->param.dslOLRRequest.carrParamPtr);
#else
			*dstPtr++ = (ulong) cmd->param.dslOLRRequest.carrParamPtr;
#endif
			break;

		case kDslPwrMgrCmd:
			*dstPtr++ = cmd->param.dslPwrMsg.msgType;
			*dstPtr++ = cmd->param.dslPwrMsg.param.msg.msgLen;
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (ulong) SDRAM_ADDR_TO_ADSL(cmd->param.dslPwrMsg.param.msg.msgData);
#else
			*dstPtr++ = (ulong) cmd->param.dslPwrMsg.param.msg.msgData;
#endif
			break;
#endif

		default:
			*dstPtr++ = cmd->param.value;
			break;
	}
	return (char*)dstPtr - (char*)dstPtr0;
}

#endif /* ADSLCORE_ONLY */

#ifndef HOST_ONLY

Public int UnflattenCommand(ulong *srcPtr, dslCommandStruct *cmd)
{
	ulong	*srcPtr0 = srcPtr;

	cmd->command = *srcPtr++;
	
	switch	(cmd->command) {
		case kDslStartPhysicalLayerCmd:
			cmd->param.dslModeSpec.direction = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.modulations = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.minDataRate = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.maxDataRate = (ushort) *srcPtr++;
#if	defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMinCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMaxCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMinCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMaxCarr = (uchar) *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_A
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfo = *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_B
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexB = *srcPtr++;
#endif
#ifdef	G992_ANNEXC
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC = *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_I
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexI = *srcPtr++;
#endif
#ifdef	G992P5
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfop5 = *srcPtr++;
#endif
			cmd->param.dslModeSpec.capabilities.features = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.auxFeatures = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.demodCapabilities = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.noiseMargin = (ushort) *srcPtr++;
#ifdef G992_ATUC /* last please, for ATU-C only */
			cmd->param.dslModeSpec.capabilities.xmtRSf	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtRS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtD	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvRSf	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvRS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvD	= (uchar) *srcPtr++;
#endif
#ifdef G992P3
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = (void *) *srcPtr++;
#endif
#ifdef G992P5
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = (void *) *srcPtr++;
#endif
			break;

		case kDslDiagSetupCmd:
			cmd->param.dslDiagSpec.setup			= *srcPtr++;
			cmd->param.dslDiagSpec.eyeConstIndex1	= *srcPtr++;
			cmd->param.dslDiagSpec.eyeConstIndex2	= *srcPtr++;
			cmd->param.dslDiagSpec.logTime			= *srcPtr++;
			break;

		case kDslTestCmd:
			cmd->param.dslTestSpec.type = *srcPtr++;
			if (kDslTestToneSelection == cmd->param.dslTestSpec.type) {
				long	len1, len2;

				cmd->param.dslTestSpec.param.toneSelectSpec.xmtStartTone	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvStartTone	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones	= *srcPtr++;

				len1 = (cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones + 7) >> 3;
				len2 = (cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones + 7) >> 3;
				cmd->param.dslTestSpec.param.toneSelectSpec.xmtMap = (uchar *) srcPtr;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvMap = ((uchar *) srcPtr) + len1;
				srcPtr += (len1 + len2 + 3) >> 2;
			}
			else if (kDslTestExecuteDelay == cmd->param.dslTestSpec.type) {
				cmd->param.dslTestSpec.param.value                          = *srcPtr++;
			}
#ifdef ADSL_MARGIN_TWEAK_TEST
			else if (kDslTestMarginTweak == cmd->param.dslTestSpec.type) {
				cmd->param.dslTestSpec.param.marginTweakSpec.extraPowerRequestQ4dB	= *srcPtr++;
				cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers			= *srcPtr++;
				cmd->param.dslTestSpec.param.marginTweakSpec.marginTweakTableQ4dB	= (uchar *) srcPtr;

				srcPtr += (cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers + 3) >> 2;
			}
#endif
			break;

		case kDslSetStatusBufferCmd:
			cmd->param.dslStatusBufSpec.pBuf	= (void *) *srcPtr++;
			cmd->param.dslStatusBufSpec.bufSize = *srcPtr++;
			break;

		case kDslAfeTestCmd:
			cmd->param.dslAfeTestSpec.type = *srcPtr++;
			cmd->param.dslAfeTestSpec.afeParamPtr = (void *) *srcPtr++;
			cmd->param.dslAfeTestSpec.afeParamSize = *srcPtr++;
			cmd->param.dslAfeTestSpec.imagePtr = (void *) *srcPtr++;
			cmd->param.dslAfeTestSpec.imageSize = *srcPtr++;
			break;

		case kDslPLNControlCmd:
			cmd->param.dslPlnSpec.plnCmd = *srcPtr++;
			if (kDslPLNControlStart == cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.mgnDescreaseLevelPerBin = srcPtr[0];
				cmd->param.dslPlnSpec.mgnDescreaseLevelBand   = srcPtr[1];
				srcPtr+=2;
			}
			else if(kDslPLNControlDefineInpBinTable==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.nInpBin= srcPtr[0];
				srcPtr++;
				cmd->param.dslPlnSpec.inpBinPtr=(void *)*srcPtr++;
			}
			else if(kDslPLNControlDefineItaBinTable==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.nItaBin=*srcPtr++;
				cmd->param.dslPlnSpec.itaBinPtr=(void *)*srcPtr++;
			}
			break;

#ifdef G997_1
		case kDslSendEocCommand:
			cmd->param.value = *srcPtr++;
			if (cmd->param.value >= kDslClearEocFirstCmd)
				{
				cmd->param.dslClearEocMsg.msgType = *srcPtr++;
				if (cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					cmd->param.dslClearEocMsg.dataPtr = (char*) srcPtr;
					srcPtr += ((cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask) + 3) >> 2;
					}
				else
					cmd->param.dslClearEocMsg.dataPtr = (char*)*srcPtr++;
				}
			break;
#endif
#ifdef G992P3
		case kDslOLRRequestCmd:
			cmd->param.dslOLRRequest.msgType = *srcPtr++;
			cmd->param.dslOLRRequest.nCarrs	 = *srcPtr++;
			BlockByteMove (12, (void *) srcPtr, (void *) cmd->param.dslOLRRequest.L);
			srcPtr += 3;
			cmd->param.dslOLRRequest.carrParamPtr = (void *) *srcPtr++;
			break;

		case kDslPwrMgrCmd:
			cmd->param.dslPwrMsg.msgType = *srcPtr++;
			cmd->param.dslPwrMsg.param.msg.msgLen = *srcPtr++;
			cmd->param.dslPwrMsg.param.msg.msgData = (void *) *srcPtr++;
			break;
#endif
		default:
			cmd->param.value = *srcPtr++;
			break;
	}
	return (char*)srcPtr - (char*)srcPtr0;
}

Private void * FlattenStatusSlow (dslStatusStruct *status, ulong *dstPtr, ulong statAvail, ulong *dstPtr0)
{
#define	CHECK_BUF_AVAIL_SLOW(szReq,szAvail)	if ((long)(szReq) > (long)(szAvail)) return dstPtr0;

	int		size;

	switch	(status->code) {
		case kDslExceptionStatus:
			{
			int		i;

			CHECK_BUF_AVAIL_SLOW(34+status->param.dslException.argc, statAvail);
			BlockLongMove (32, (long *) status->param.dslException.sp, (long*) dstPtr);
			dstPtr += 31;

			*dstPtr++ = (ulong) status->param.dslException.argc;
			for (i = 0; i < status->param.dslException.argc; i++)
				*dstPtr++ = (ulong) status->param.dslException.argv[i];
			}	
			break;

		case kAtmStatus:
			switch (status->param.atmStatus.code) {
				case kAtmStatVcCreated:
					CHECK_BUF_AVAIL_SLOW(6, statAvail);
					*dstPtr++ = status->param.atmStatus.param.vcInfo.vcId;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.vci;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.aalType;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.fwdPeakCellTime;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.backPeakCellTime;
					break;

				case kAtmStatOamF4SegmentCell:
				case kAtmStatOamF4End2EndCell:
				case kAtmStatOamF5SegmentCell:
				case kAtmStatOamF5End2EndCell:
					*dstPtr++ = status->param.atmStatus.param.oamInfo.oamCmd;
					break;

				case kAtmStatRxPacket:
				case kAtmStatTxPacket:
					{
					long			frBytes;

					frBytes = status->param.atmStatus.param.frame.length;
					if  (frBytes > kMaxFlattenFramelength)
						frBytes = kMaxFlattenFramelength;
					size = (frBytes + 3) >> 2;
					CHECK_BUF_AVAIL_SLOW(3+size, statAvail);
					*dstPtr++ = status->param.atmStatus.param.frame.vci;
					*dstPtr++ = status->param.atmStatus.param.frame.aalType;
					*dstPtr++ = status->param.atmStatus.param.frame.length;
					BlockByteMove ((size<<2), status->param.atmStatus.param.frame.framePtr, (char*) dstPtr);
					dstPtr += size;
					}
					break;
			}
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
			*dstPtr++ = status->param.dslOLRRequest.msgType;
			*dstPtr++ = status->param.dslOLRRequest.nCarrs;
			BlockByteMove (12, (void *) status->param.dslOLRRequest.L, (char*) dstPtr);
			dstPtr += 3;
			*dstPtr++ = (ulong) status->param.dslOLRRequest.carrParamPtr;
			break;
		case kDslPwrMgrStatus:
			*dstPtr++ = status->param.dslPwrMsg.msgType;
			*dstPtr++ = status->param.dslPwrMsg.param.msg.msgLen;
			*dstPtr++ = (ulong) status->param.dslPwrMsg.param.msg.msgData;
			break;
#endif
	}

	return dstPtr;
}

Public int FlattenStatus (dslStatusStruct *status, ulong *dstPtr, ulong nAvail) FAST_TEXT;
Public int FlattenStatus (dslStatusStruct *status, ulong *dstPtr, ulong nAvail)
{
#define	CHECK_BUF_AVAIL(szReq,szAvail)	if ((long)(szReq) > (long)(szAvail)) return 0;

	ulong	*dstPtr0 = dstPtr, statAvail = (nAvail >> 2);
	int		size;

	*dstPtr++ = status->code;
	
	switch	(status->code) {
		case		kDslTrainingStatus:
			*dstPtr++ = status->param.dslTrainingInfo.code;
			*dstPtr++ = status->param.dslTrainingInfo.value;
			break;

		case 		kDslDspControlStatus:
			*dstPtr++ = status->param.dslConnectInfo.code;
			*dstPtr++ = status->param.dslConnectInfo.value;
			*dstPtr++ = (long)status->param.dslConnectInfo.buffPtr;
			break;

		case		kDslConnectInfoStatus:
			*dstPtr++ = status->param.dslConnectInfo.code;
			*dstPtr++ = status->param.dslConnectInfo.value;
			switch (status->param.dslConnectInfo.code) {
#ifdef G992
				case	kG992p2XmtToneOrderingInfo:
				case	kG992p2RcvToneOrderingInfo:
#ifdef G992_BIT_SWAP
				case	kG992AocMessageExchangeRcvInfo:
				case	kG992AocMessageExchangeXmtInfo:
#endif
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove (
						(size << 2), 
						(char*)status->param.dslConnectInfo.buffPtr,
						(char*)dstPtr);
					dstPtr += size;
					break;
				case	kG992p2XmtCodingParamsInfo:
				case	kG992p2RcvCodingParamsInfo:
					{
					G992CodingParams	*codingParam;

					size = (sizeof(G992CodingParams) + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					codingParam = (G992CodingParams*)status->param.dslConnectInfo.buffPtr;
					BlockByteMove ((size<<2), (uchar *)codingParam, (uchar *)dstPtr);
					dstPtr += size;
					}
					break;
				case	kG992p2TrainingRcvCarrEdgeInfo:
					{
					int		*carrEdges = (int*)(status->param.dslConnectInfo.buffPtr);

					*dstPtr++ = carrEdges[0];
					*dstPtr++ = carrEdges[1];
					}
					break;
				case	kG992ShowtimeMonitoringStatus:
#ifdef DSL_REPORT_ALL_COUNTERS
					CHECK_BUF_AVAIL(kG992ShowtimeNumOfMonitorCounters, statAvail-3);
#else
					CHECK_BUF_AVAIL(12, statAvail-3);
#endif
					BlockShortMove (
#ifdef DSL_REPORT_ALL_COUNTERS
						kG992ShowtimeNumOfMonitorCounters << 1,
#else
						12 << 1,
#endif
						(short*)status->param.dslConnectInfo.buffPtr,
						(short*)dstPtr);
#ifdef DSL_REPORT_ALL_COUNTERS
					dstPtr += kG992ShowtimeNumOfMonitorCounters;
#else
					dstPtr += 12;
#endif
					break;
#endif	/* G992 */

#if defined(G992) || defined(G994P1)
				case	kG994MessageExchangeRcvInfo:
				case	kG994MessageExchangeXmtInfo:
				case	kG992MessageExchangeRcvInfo:
				case	kG992MessageExchangeXmtInfo:
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove (
						(size << 2), 
						(char*)status->param.dslConnectInfo.buffPtr,
						(char*)dstPtr);
					dstPtr += size;
					break;
#endif	/* defined(G992) || defined(G994P1) */

				case	kDslTEQCoefInfo:
				case	kDslRcvPsdInfo:
				case	kDslRcvCarrierSNRInfo:
					size = (status->param.dslConnectInfo.value + 1) >> 1;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockShortMove (
						(size << 1), 
						(short*)status->param.dslConnectInfo.buffPtr,
						(short*)dstPtr);
					dstPtr += size;
					break;

				case kG992p3XmtCodingParamsInfo:
				case kG992p3RcvCodingParamsInfo:
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove ((size<<2), status->param.dslConnectInfo.buffPtr, (uchar *)dstPtr);
					dstPtr += size;
					break;
				case kDslChannelResponseLog:
				case kDslChannelResponseLinear:
				case kDslChannelQuietLineNoise:
					*dstPtr++ = (ulong)status->param.dslConnectInfo.buffPtr;
					break;
			}
			break;

		case		kDslShowtimeSNRMarginInfo:
			size = (status->param.dslShowtimeSNRMarginInfo.nCarriers + 1) >> 1;
			CHECK_BUF_AVAIL(size+6, statAvail-1);
			BlockShortMove (6*2, (short *) &status->param.dslShowtimeSNRMarginInfo, (short *) dstPtr);
			dstPtr += 6;
			if (size != 0)
				BlockShortMove (
					(size << 1), 
					(short*)status->param.dslShowtimeSNRMarginInfo.buffPtr,
					(short*)dstPtr);
			dstPtr += size;
			break;

#ifdef G997_1
		case kDslReceivedEocCommand:
			*dstPtr++ = status->param.value;
			if (status->param.value >= kDslClearEocFirstCmd)
				{
				*dstPtr++ = status->param.dslClearEocMsg.msgType;
				if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					long	len;

					len = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					size = (len + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove ((size << 2), status->param.dslClearEocMsg.dataPtr, (char*)dstPtr);
					dstPtr += size;
					}
				else
					*dstPtr++ = (ulong) status->param.dslClearEocMsg.dataPtr;
				}
			break;

#ifdef G997_1_FRAMER
		case kDslG997Status:
			*dstPtr++ = status->param.g997Status.code;
			switch (status->param.g997Status.code) {
				case kDslFramerRxFrameErr:
				case kDslFramerTxFrameErr:
					*dstPtr++ = status->param.g997Status.param.error;
					break;
			}
			break;
#endif

#endif

		case kAtmStatus:
			*dstPtr++ = status->param.atmStatus.code;
			*dstPtr++ = status->param.atmStatus.param.value;
			if (status->param.atmStatus.code < kAtmLayerStatFirst) {
				if (kAtmStatBertResult == status->param.atmStatus.code)
					*dstPtr++ = status->param.atmStatus.param.bertInfo.errBits;
				else
					dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 3, dstPtr0);
			}
			break;

		case kDslPrintfStatus:
			{
				va_list		ap;
				long		arg, i;

				CHECK_BUF_AVAIL(status->param.dslPrintfMsg.argNum+2, statAvail-1);
				*dstPtr++ = (ulong) status->param.dslPrintfMsg.fmt;
				*dstPtr++ = status->param.dslPrintfMsg.argNum;
				ap = (void *) status->param.dslPrintfMsg.argPtr;
				for (i = 0; i < status->param.dslPrintfMsg.argNum; i++) {
					arg = va_arg(ap, long);
					*dstPtr++ = arg;
				}
			}
			break;

		case kDslExceptionStatus:
			dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 1, dstPtr0);
			break;

		case kDslDataAvailStatus:
			*dstPtr++ = (ulong) status->param.dslDataAvail.dataPtr;
			*dstPtr++ = status->param.dslDataAvail.dataLen;
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
		case kDslPwrMgrStatus:
			dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 1, dstPtr0);
			break;
#endif

		default:
			*dstPtr++ = status->param.value;
			break;
	}
	return (char*)dstPtr - (char*)dstPtr0;
}

#endif /* HOST_ONLY */


#ifndef ADSLCORE_ONLY

static	void	*statDataPtr = NULL;
static	ulong	statDataLen = 0;

Public int UnflattenStatus (ulong *srcPtr, dslStatusStruct *status)
{
	ulong	*srcPtr0 = srcPtr;
	ulong	tmp;
	Boolean	bStatPtrSet = false;
#ifdef  ADSLDRV_LITTLE_ENDIAN
	int		i;
#endif
	status->code = *srcPtr++;
	
	switch	(status->code) {
		case		kDslTrainingStatus:
			status->param.dslTrainingInfo.code  = srcPtr[0];
			status->param.dslTrainingInfo.value = srcPtr[1];
			srcPtr += 2;
			break;

		case 		kDslDspControlStatus:
			status->param.dslConnectInfo.code  = srcPtr[0];
			status->param.dslConnectInfo.value = srcPtr[1];
			status->param.dslConnectInfo.buffPtr = ADSL_ADDR_TO_HOST(srcPtr[2]);
			srcPtr += 3;
			if (kDslStatusBufferInfo == status->param.dslConnectInfo.code) {
				statDataPtr = status->param.dslConnectInfo.buffPtr;
				statDataLen = status->param.dslConnectInfo.value;
				bStatPtrSet = true;
			}
			break;

		case		kDslConnectInfoStatus:
			status->param.dslConnectInfo.code  = srcPtr[0];
			status->param.dslConnectInfo.value = srcPtr[1];
			srcPtr += 2;
			switch (status->param.dslConnectInfo.code) {
#ifdef G992
				case	kG992p2XmtToneOrderingInfo:
				case	kG992p2RcvToneOrderingInfo:
				case	kG992AocMessageExchangeRcvInfo:
				case	kG992AocMessageExchangeXmtInfo:
#ifdef  ADSLDRV_LITTLE_ENDIAN
					for (i = 0; i < ((status->param.dslConnectInfo.value + 3) >> 2); i++)
						srcPtr[i] = ADSL_ENDIAN_CONV_LONG(srcPtr[i]);
#endif
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp= (status->param.dslConnectInfo.value + 3) >> 2;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = statDataLen;
					}
					srcPtr += tmp;
					break;
				case	kG992p2XmtCodingParamsInfo:
				case	kG992p2RcvCodingParamsInfo:
#ifdef  ADSLDRV_LITTLE_ENDIAN
					{
					G992CodingParams	*codingParam = (void *) srcPtr;

					for (i = 0; i < ((sizeof(G992CodingParams) + 3) >> 2); i++)
						srcPtr[i] = ADSL_ENDIAN_CONV_LONG(srcPtr[i]);

					codingParam->K = ADSL_ENDIAN_CONV_SHORT(codingParam->K);
					codingParam->N = ADSL_ENDIAN_CONV_SHORT(codingParam->N);
					codingParam->AS0BI = ADSL_ENDIAN_CONV_SHORT(codingParam->AS0BI);
					}
#endif
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += (sizeof(G992CodingParams) + 3) >> 2;
					break;
				case	kG992p2TrainingRcvCarrEdgeInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += 2;
					break;
				case	kG992ShowtimeMonitoringStatus:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
#ifdef DSL_REPORT_ALL_COUNTERS
					srcPtr += kG992ShowtimeNumOfMonitorCounters;
#else
					srcPtr += 12;
#endif
					break;
#endif	/* G992 */

#if defined(G992) || defined(G994P1)
				case	kG994MessageExchangeRcvInfo:
				case	kG994MessageExchangeXmtInfo:
				case	kG992MessageExchangeRcvInfo:
				case	kG992MessageExchangeXmtInfo:
#ifdef  ADSLDRV_LITTLE_ENDIAN
					for (i = 0; i < ((status->param.dslConnectInfo.value + 3) >> 2); i++)
						srcPtr[i] = ADSL_ENDIAN_CONV_LONG(srcPtr[i]);
#endif
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp = (status->param.dslConnectInfo.value + 3) >> 2;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = statDataLen;
					}
					srcPtr += tmp;
					break;
#endif	/* defined(G992) || defined(G994P1) */

				case	kDslTEQCoefInfo:
				case	kDslRcvPsdInfo:
				case	kDslRcvCarrierSNRInfo:
#ifdef  ADSLDRV_LITTLE_ENDIAN
					for (i = 0; i < ((status->param.dslConnectInfo.value + 3) >> 2); i++)
						srcPtr[i] = ADSL_ENDIAN_CONV_2SHORTS(srcPtr[i]);
#endif
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp = (status->param.dslConnectInfo.value + 1) >> 1;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = statDataLen;
					}
					srcPtr += tmp;
					break;
#ifdef	G992P3
				case kG992p3XmtCodingParamsInfo:
				case kG992p3RcvCodingParamsInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += (status->param.dslConnectInfo.value + 3) >> 2;
					break;

				case kDslChannelResponseLog:
				case kDslChannelResponseLinear:
				case kDslChannelQuietLineNoise:
					{
					ulong	bufAddr = *srcPtr++;

#ifdef  ADSLDRV_LITTLE_ENDIAN
					bufAddr = ADSL_ENDIAN_CONV_LONG(bufAddr);
#endif
#ifdef FLATTEN_ADDR_ADJUST
					if (ADSL_MIPS_LMEM_ADDR(bufAddr))
						bufAddr = (bufAddr | FLATTEN_ADDR_ADJUST);
					else
						bufAddr = (ulong) SDRAM_ADDR_TO_HOST(bufAddr);
#endif
					status->param.dslConnectInfo.buffPtr = (void *) bufAddr;
					}
					break;
#endif
			}
			break;

		case		kDslShowtimeSNRMarginInfo:
			BlockShortMove (6*2, (short *) srcPtr, (short *) &status->param.dslShowtimeSNRMarginInfo);
			srcPtr += 6;
#ifdef  ADSLDRV_LITTLE_ENDIAN
			for (i = 0; i < ((status->param.dslShowtimeSNRMarginInfo.nCarriers + 1) >> 1); i++)
				srcPtr[i] = ADSL_ENDIAN_CONV_2SHORTS(srcPtr[i]);
#endif
			status->param.dslShowtimeSNRMarginInfo.buffPtr = (void*) srcPtr;
			tmp = (status->param.dslShowtimeSNRMarginInfo.nCarriers + 1) >> 1;
			if (NULL != statDataPtr) {
				status->param.dslShowtimeSNRMarginInfo.nCarriers = statDataLen;
				status->param.dslShowtimeSNRMarginInfo.buffPtr   = statDataPtr;
				srcPtr[-1] = statDataLen;
			}
			srcPtr += tmp;
			break;

#ifdef G997_1
		case kDslReceivedEocCommand:
			status->param.value = *srcPtr++;
			if (status->param.value >= kDslClearEocFirstCmd)
				{
				status->param.dslClearEocMsg.msgType = *srcPtr++;
				if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					int	size = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					size = (size + 3) >> 2;
					status->param.dslClearEocMsg.dataPtr = (char*) srcPtr;
					srcPtr += size;
					}
				else 
					{
					status->param.dslClearEocMsg.dataPtr = (char*) *srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
					if (NULL != status->param.dslClearEocMsg.dataPtr)
						status->param.dslClearEocMsg.dataPtr = ADSL_ADDR_TO_HOST(status->param.dslClearEocMsg.dataPtr);
#endif
					}
				}
			break;

#ifdef G997_1_FRAMER
		case kDslG997Status:
			status->param.g997Status.code = *srcPtr++;
			switch (status->param.g997Status.code) {
				case kDslFramerRxFrameErr:
				case kDslFramerTxFrameErr:
					status->param.g997Status.param.error= *srcPtr++;
					break;
			}
			break;
#endif

#endif

		case		kAtmStatus:
			status->param.atmStatus.code        = *srcPtr++;
			status->param.atmStatus.param.value = *srcPtr++;
			switch (status->param.atmStatus.code) {
				case kAtmStatVcCreated:
					status->param.atmStatus.param.vcInfo.vcId				= *srcPtr++;
					status->param.atmStatus.param.vcInfo.vci				= *srcPtr++;
					status->param.atmStatus.param.vcInfo.aalType			= *srcPtr++;
					status->param.atmStatus.param.vcInfo.fwdPeakCellTime	= *srcPtr++;
					status->param.atmStatus.param.vcInfo.backPeakCellTime	= *srcPtr++;
					break;

				case kAtmStatOamF4SegmentCell:
				case kAtmStatOamF4End2EndCell:
				case kAtmStatOamF5SegmentCell:
				case kAtmStatOamF5End2EndCell:
					status->param.atmStatus.param.oamInfo.oamCmd = *srcPtr++;
					break;

				case kAtmStatRxPacket:
				case kAtmStatTxPacket:
					{
					long			frBytes;

					status->param.atmStatus.param.frame.vci		= *srcPtr++;
					status->param.atmStatus.param.frame.aalType = *srcPtr++;
					status->param.atmStatus.param.frame.length	= *srcPtr++;
					frBytes = status->param.atmStatus.param.frame.length;
					if  (frBytes > kMaxFlattenFramelength)
						frBytes = kMaxFlattenFramelength;
					status->param.atmStatus.param.frame.framePtr= (void*)srcPtr;
					srcPtr += (frBytes + 3) >> 2;
					}
					break;
				case kAtmStatBertResult:
					status->param.atmStatus.param.bertInfo.errBits = *srcPtr++;
					break;
				case kAtmStatCounters:
#ifdef FLATTEN_ADDR_ADJUST
					status->param.atmStatus.param.value = 
						(ADSL_MIPS_LMEM_ADDR(status->param.atmStatus.param.value)) ?
							(status->param.atmStatus.param.value | FLATTEN_ADDR_ADJUST) :
							(ulong) SDRAM_ADDR_TO_HOST(status->param.atmStatus.param.value);
#endif
					break;
			}
			break;

		case kDslPrintfStatus:
			status->param.dslPrintfMsg.fmt = (void *)*srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
			{
			ulong	fmtAddr = (ulong) status->param.dslPrintfMsg.fmt;

			if (ADSL_MIPS_LMEM_ADDR(fmtAddr))
				status->param.dslPrintfMsg.fmt = (void *) (fmtAddr | FLATTEN_ADDR_ADJUST);
			else
				status->param.dslPrintfMsg.fmt = SDRAM_ADDR_TO_HOST(fmtAddr);
			}
#endif
			status->param.dslPrintfMsg.argNum = *srcPtr++;
			status->param.dslPrintfMsg.argPtr = srcPtr;
			srcPtr += status->param.dslPrintfMsg.argNum;
			break;

		case kDslExceptionStatus:
			status->param.dslException.sp = (int) srcPtr;
			srcPtr += 31;

			status->param.dslException.argc = *srcPtr++;
			status->param.dslException.argv = (int*)srcPtr;
			srcPtr += status->param.dslException.argc;
			break;

		case kDslOemDataAddrStatus:
			status->param.value = *srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
			status->param.value = (ulong) SDRAM_ADDR_TO_HOST(status->param.value);
#endif
			break;

		case kDslDataAvailStatus:
			status->param.dslDataAvail.dataPtr = (void *) *srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
			status->param.dslDataAvail.dataPtr = (void *) ((ulong) status->param.dslDataAvail.dataPtr | FLATTEN_ADDR_ADJUST);
#endif
			status->param.dslDataAvail.dataLen = *srcPtr++;
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
			status->param.dslOLRRequest.msgType = *srcPtr++;
			status->param.dslOLRRequest.nCarrs	= *srcPtr++;
			BlockByteMove (12, (void *) srcPtr, (void *) status->param.dslOLRRequest.L);
			srcPtr += 3;
			tmp = *srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
			status->param.dslOLRRequest.carrParamPtr = (void *) ADSL_ADDR_TO_HOST(tmp);
#else
			status->param.dslOLRRequest.carrParamPtr = (void *) tmp;
#endif
			break;
		case kDslPwrMgrStatus:
			status->param.dslPwrMsg.msgType = *srcPtr++;
			status->param.dslPwrMsg.param.msg.msgLen = *srcPtr++;
			tmp = *srcPtr++;
#ifdef FLATTEN_ADDR_ADJUST
			status->param.dslPwrMsg.param.msg.msgData = (void *) ADSL_ADDR_TO_HOST(tmp);
#else
			status->param.dslPwrMsg.param.msg.msgData = (void *) tmp;
#endif
			break;
#endif
		default:
			status->param.value = *srcPtr++;
			break;
	}
	if (!bStatPtrSet)
		statDataPtr = NULL;
	return (char*)srcPtr - (char*)srcPtr0;
}

#endif

/*
**
**	Functions to read and write to command/status strectch buffers
**
*/

#define	BUFFER_ANCHOR		0x55AA1234

#ifndef HOST_ONLY

Public int	FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status) FAST_TEXT;
Public int	FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status)
{
	ulong	statBytesAvail, n, *p;
	char	*rdPtr;
	ulong	critId = SoftDslEnterCritical();

#if defined(bcm47xx) && defined(ADSLCORE_ONLY)
	rdPtr = ((stretchBufferStruct *) ((ulong)fBuf | 0x20000000))->pRead;
#else
	rdPtr = fBuf->pRead;
#endif
	statBytesAvail = _StretchBufferGetWriteAvail(fBuf,rdPtr);
	if (statBytesAvail < 32) {
		SoftDslLeaveCritical(critId);
		return 0;
	}

	p = (ulong *) StretchBufferGetWritePtr(fBuf);
	n = FlattenStatus (status, p, statBytesAvail-4);
	if (n > 0) {
		n = (n + 3) & ~3;
		*(p + (n >> 2)) = BUFFER_ANCHOR;
		n += 4;
		_StretchBufferWriteUpdate (fBuf, rdPtr, n);
	}
	SoftDslLeaveCritical(critId);
	return n;
}

#endif /* HOST_ONLY */

#ifndef ADSLCORE_ONLY

/* #define STATUS_BUFFER_STAT */
#ifdef  STATUS_BUFFER_STAT
static ulong	maxStatLen = 0;
static ulong	maxStatBuf = 0;
#endif

Public void FlattenBufferClearStat(void)
{
#ifdef  STATUS_BUFFER_STAT
	maxStatLen = 0;
	maxStatBuf = 0;
#endif
}

Public int	FlattenBufferStatusRead(stretchBufferStruct *fBuf, dslStatusStruct *status)
{
	ulong	statBytesAvail, n, *p;

	statBytesAvail = StretchBufferGetReadAvail(fBuf);
	if (0 == statBytesAvail)
		return 0;

#ifdef STATUS_BUFFER_STAT
	if (statBytesAvail > maxStatBuf) {
		maxStatBuf = statBytesAvail;
		__SoftDslPrintf(NULL, "FlattenBufferStatusStatT: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif
	p = StretchBufferGetReadPtr(fBuf);
#ifdef FLATTEN_ADDR_ADJUST
	p	= (void *) (((long) p) + flattenAddrAdj);
#endif

	n = UnflattenStatus (p, status);
	n = (n + 3) & ~3;
#ifdef STATUS_BUFFER_STAT
	if (n  > maxStatLen) {
		maxStatLen = n;
		__SoftDslPrintf(NULL, "FlattenBufferStatusStatM: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif
	p += (n >> 2);
	if (BUFFER_ANCHOR == *p)
		return n + 4;

	/* sync lost in status buffer. Try to recover */

	CircBufferGetReadPtr(fBuf) = CircBufferGetWritePtr(fBuf);
	return -(long)(n+4);
}

Public int	FlattenBufferCommandWrite(stretchBufferStruct *fBuf, dslCommandStruct *cmd)
{
	ulong	statBytesAvail, n, *p;

	statBytesAvail = StretchBufferGetWriteAvail(fBuf);
	if (statBytesAvail < 24) {
		return -1;
	}

	p = (ulong *) StretchBufferGetWritePtr(fBuf);
#ifdef FLATTEN_ADDR_ADJUST
	p	= (void *) (((long) p) | FLATTEN_ADDR_ADJUST);
#endif
	n = FlattenCommand (cmd, p, statBytesAvail-4);
	if (n > 0) {
		n = (n + 3) & ~3;
		*(p + (n >> 2)) = BUFFER_ANCHOR;
		StretchBufferWriteUpdate (fBuf, n+4);
		return n+4;
	}
	return 0;
}
#endif /* ADSLCORE_ONLY */

#ifndef HOST_ONLY
Public int	FlattenBufferCommandRead(stretchBufferStruct *fBuf, dslCommandStruct *cmd)
{
	ulong	statBytesAvail, n, *p;

	statBytesAvail = StretchBufferGetReadAvail(fBuf);
	if (0 == statBytesAvail)
		return 0;

	p = StretchBufferGetReadPtr(fBuf);
	n = UnflattenCommand(p, cmd);
	n = (n + 3) & ~3;
	p += (n >> 2);
	if (BUFFER_ANCHOR == *p)
		return n + 4;

	/* sync lost in command buffer. Try to recover */

	CircBufferGetReadPtr(fBuf) = CircBufferGetWritePtr(fBuf);
	return -(long)(n+4);
}
#endif /* HOST_ONLY */
