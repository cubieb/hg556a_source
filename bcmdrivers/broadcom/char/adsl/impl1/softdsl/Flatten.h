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
 * Flatten.h -- Header for Flatten/Unflatten command/status
 *
 * Copyright (c) 1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: Flatten.h,v 1.1 2008/08/25 06:41:04 l65130 Exp $
 *
 * $Log: Flatten.h,v $
 * Revision 1.1  2008/08/25 06:41:04  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:23  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:22  z30370
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
 * Revision 1.14  2004/03/04 19:48:52  linyin
 * Support adsl2plus
 *
 * Revision 1.13  2003/10/17 22:45:14  yongbing
 * Increase buffer size for large B&G table of G992P3
 *
 * Revision 1.12  2003/08/12 23:16:26  khp
 * - for Haixiang: added support for ADSL_MARGIN_TWEAK_TEST
 *
 * Revision 1.11  2003/02/27 06:33:03  ilyas
 * Improved free space checking in command buffer (became a problem with
 * 2 commands SetXmtgain and StartPhy)
 *
 * Revision 1.10  2003/01/11 01:27:07  ilyas
 * Improved checking for available space in status buffer
 *
 * Revision 1.9  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.8  2002/05/16 00:01:52  khp
 * -added missing #endif
 *
 * Revision 1.7  2002/05/15 00:04:48  mprahlad
 * increase the status buffer size - prevent memory overflow for annexC cases
 *
 * Revision 1.6  2002/04/05 04:10:33  linyin
 * -hack to fit in Annex C firmware in LMEM
 *
 * Revision 1.5  2002/04/05 02:45:25  linyin
 * Make the buffer side larger for annexC
 *
 * Revision 1.4  2002/01/30 07:19:06  ilyas
 * Moved showtime code to LMEM
 *
 * Revision 1.3  2001/08/29 02:56:01  ilyas
 * Added tests for flattening/unflatenning command and statuses (dual mode)
 *
 * Revision 1.2  2001/04/25 00:30:54  ilyas
 * Adjusted MaxFrameLen
 *
 * Revision 1.1  2001/04/24 21:41:21  ilyas
 * Implemented status flattening/unflattaning to transfer statuses between
 * modules asynchronously through the circular buffer
 *
 *
 *****************************************************************************/

#ifndef _Flatten_H_
#define _Flatten_H_

#include	"CircBuf.h"

#ifdef ADSL_MARGIN_TWEAK_TEST
#define kMaxFlattenedCommandSize	272		/* maximum no. of bytes in flattened cmd */
#else
#define kMaxFlattenedCommandSize	128		/* maximum no. of bytes in flattened cmd */
#endif
#if	defined(G992_ANNEXC) || defined(G992P3)
#if defined(G992P5)
#define kMaxFlattenedStatusSize		2200   	/* maximum no. of bytes in flattened status */
#else
#define kMaxFlattenedStatusSize		1100   	/* maximum no. of bytes in flattened status */
#endif
#else
#define kMaxFlattenedStatusSize		 550   	/* maximum no. of bytes in flattened status */
#endif

#define	kMaxFlattenFramelength		(kMaxFlattenedStatusSize - (4*sizeof(long)) - 20)

extern int	SM_DECL FlattenCommand	(dslCommandStruct *cmd, ulong *dstPtr, ulong nAvail);
extern int	SM_DECL UnflattenCommand(ulong *srcPtr, dslCommandStruct *cmd);
extern int	SM_DECL FlattenStatus	(dslStatusStruct *status, ulong *dstPtr, ulong nAvail);
extern int	SM_DECL UnflattenStatus	(ulong *srcPtr, dslStatusStruct *status);

#define	FlattenBufferInit(fb,fbData,bufSize,itemSize)		\
	StretchBufferInit(fb, fbData, bufSize, itemSize)
	
extern int	SM_DECL FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status);
extern int	SM_DECL FlattenBufferStatusRead(stretchBufferStruct *fBuf, dslStatusStruct *status);

extern int	SM_DECL FlattenBufferCommandWrite(stretchBufferStruct *fBuf, dslCommandStruct *cmd);
extern int	SM_DECL FlattenBufferCommandRead(stretchBufferStruct *fBuf, dslCommandStruct *cmd);

#define FlattenBufferReadComplete(fb,nBytes)				\
	StretchBufferReadUpdate (fb, nBytes)

#endif /* _Flatten_H_ */

