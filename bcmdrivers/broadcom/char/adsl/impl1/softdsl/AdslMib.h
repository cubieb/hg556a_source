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
 * AdslMib.h 
 *
 * Description:
 *	This file contains the exported functions and definitions for AdslMib
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslMib.h,v 1.1 2008/08/25 06:41:01 l65130 Exp $
 *
 * $Log: AdslMib.h,v $
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
 * Revision 1.9  2004/04/12 23:34:52  ilyas
 * Merged the latest ADSL driver chnages for ADSL2+
 *
 * Revision 1.8  2004/03/03 20:14:05  ilyas
 * Merged changes for ADSL2+ from ADSL driver
 *
 * Revision 1.7  2003/10/14 00:55:27  ilyas
 * Added UAS, LOSS, SES error seconds counters.
 * Support for 512 tones (AnnexI)
 *
 * Revision 1.6  2003/07/18 19:07:15  ilyas
 * Merged with ADSL driver
 *
 * Revision 1.5  2002/10/31 20:27:13  ilyas
 * Merged with the latest changes for VxWorks/Linux driver
 *
 * Revision 1.4  2002/07/20 00:51:41  ilyas
 * Merged witchanges made for VxWorks/Linux driver.
 *
 * Revision 1.3  2002/01/13 22:25:40  ilyas
 * Added functions to get channels rate
 *
 * Revision 1.2  2002/01/03 06:03:36  ilyas
 * Handle byte moves tha are not multiple of 2
 *
 * Revision 1.1  2001/12/21 22:39:30  ilyas
 * Added support for ADSL MIB data objects (RFC2662)
 *
 *
 *****************************************************************************/

#ifndef	AdslMibHeader
#define	AdslMibHeader

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#endif

#include "AdslMibDef.h"

/* Interface functions */

typedef	int	(SM_DECL *adslMibNotifyHandlerType)	(void *gDslVars, ulong event);

extern Boolean  AdslMibInit(void *gDslVars, dslCommandHandlerType	commandHandler);
extern void		AdslMibTimer(void *gDslVars, long timeMs);
extern void		AdslMibStatusSnooper (void *gDslVars, dslStatusStruct *status);
extern void		AdslMibSetNotifyHandler(void *gDslVars, adslMibNotifyHandlerType notifyHandlerPtr);
extern int		AdslMibGetModulationType(void *gDslVars);
extern Boolean	AdslMibIsAdsl2Mod(void *gDslVars);
extern int		AdslMibGetActiveChannel(void *gDslVars);
extern int		AdslMibGetGetChannelRate(void *gDslVars, int dir, int channel);
extern Boolean  AdslMibIsLinkActive(void *gDslVars);
extern int		AdslMibPowerState(void *gDslVars);
extern int		AdslMibTrainingState (void *gDslVars);
extern void		AdslMibClearData(void *gDslVars);
extern void		AdslMibClearBertResults(void *gDslVars);
extern void		AdslMibBertStartEx(void *gDslVars, ulong bertSec);
extern void		AdslMibBertStopEx(void *gDslVars);
extern ulong	AdslMibBertContinueEx(void *gDslVars, ulong totalBits, ulong errBits);
extern void		AdslMibSetLPR(void *gDslVars);
extern void		AdslMibSetShowtimeMargin(void *gDslVars, long showtimeMargin);
extern void		AdslMibResetConectionStatCounters(void *gDslVars);
extern void		AdslMibUpdateTxStat(
					void					*gDslVars, 
					adslConnectionDataStat  *adslTxData, 
					adslPerfCounters		*adslTxPerf, 
					atmConnectionDataStat	*atmTxData);

extern void		AdslMibByteMove (int size, void* srcPtr, void* dstPtr);
extern void		AdslMibByteClear(int size, void* dstPtr);
extern int		AdslMibStrCopy(char *srcPtr, char *dstPtr);
extern long		secElapsedInDay;
/* AdslMibGetData dataId codes */

#define	kAdslMibDataAll					0

extern void		*AdslMibGetData (void *gDslVars, int dataId, void *pAdslMibData);

extern int		AdslMibSetObjectValue (
					void	*gDslVars, 
					uchar	*objId, 
					int		objIdLen,
					uchar	*dataBuf,
					ulong	*dataBufLen);
extern int		AdslMibGetObjectValue (
					void	*gDslVars, 
					uchar	*objId, 
					int		objIdLen,
					uchar	*dataBuf,
					ulong	*dataBufLen);

#endif	/* AdslMibHeader */
