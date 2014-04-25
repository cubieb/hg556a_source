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
 * AdslCoreFrame.c -- Frame allcation/freeing functions
 *
 * Description:
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: AdslCoreFrame.c,v 1.1 2008/08/25 06:40:53 l65130 Exp $
 *
 * $Log: AdslCoreFrame.c,v $
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
 *
 *****************************************************************************/

#include "softdsl/SoftDsl.h"
#include "softdsl/BlankList.h"

#include "AdslCoreFrame.h"

#if defined(__KERNEL__) || defined(TARG_OS_RTEMS) || defined(_CFE_)
#include "BcmOs.h"
#else
#include <stdlib.h>
#endif

Public void * AdslCoreFrameAllocMemForFrames(ulong frameNum)
{
	void	**pFreeFrameList;
	uchar	*frPtr;
	ulong	i;

	/* allocate memory to hold the head and frameNum dslFrame buffers */

	pFreeFrameList = (void *) calloc (1, sizeof (void *) + sizeof(dslFrame) * frameNum);
	if (NULL == pFreeFrameList)
		return NULL;

	/* make a list of free dslFrame buffers */

	*pFreeFrameList = NULL;
	frPtr = (uchar *) (pFreeFrameList + 1) + sizeof(dslFrame) * (frameNum - 1);
	for (i = 0; i < frameNum; i++) {
		BlankListAdd(pFreeFrameList, (void*)frPtr);
		frPtr -= sizeof(dslFrame);
	}
	return pFreeFrameList;
}

Public void AdslCoreFrameFreeMemForFrames(void *hMem)
{
	free (hMem);
}

Public dslFrame * AdslCoreFrameAllocFrame(void *handle)
{
	dslFrame	*pFr;

	pFr = BlankListGet(handle);
	return pFr;
}

Public void AdslCoreFrameFreeFrame(void *handle, dslFrame *pFrame)
{
	BlankListAdd(handle, pFrame);
}

Public void * AdslCoreFrameAllocMemForBuffers(void **ppMemPool, ulong bufNum, ulong memSize)
{
	void	**pFreeBufList;
	uchar	*frPtr;
	ulong	i;
	
	/* allocate memory to hold the head and bufNum dslFrame buffers */

	pFreeBufList = (void *) calloc (1, sizeof (void *) + sizeof(dslFrameBuffer) * bufNum + memSize);
	if (NULL == pFreeBufList)
		return NULL;

	/* make a list of free dslFrame buffers */

	*pFreeBufList = NULL;
	frPtr = (uchar *) (pFreeBufList + 1) + sizeof(dslFrameBuffer) * (bufNum - 1);
	for (i = 0; i < bufNum; i++) {
		BlankListAdd(pFreeBufList, (void*)frPtr);
		frPtr -= sizeof(dslFrameBuffer);
	}

	*ppMemPool = (void *) pFreeBufList;
	return (uchar *) (pFreeBufList + 1) + sizeof(dslFrameBuffer) * bufNum;
}

Public void AdslCoreFrameFreeMemForBuffers(void *hMem, ulong memSize, void *pMemPool)
{
	free (pMemPool);
}

Public dslFrameBuffer * AdslCoreFrameAllocBuffer(void *handle, void *pMem, ulong length)
{
	dslFrameBuffer	*pBuf;

	pBuf = BlankListGet(handle);
	pBuf->pData  = pMem;
	pBuf->length = length;

	return pBuf;
}

Public void AdslCoreFrameFreeBuffer(void *handle, dslFrameBuffer *pBuf)
{
	BlankListAdd(handle, pBuf);
}

Public ulong AdslCoreFrame2Id(void *handle, dslFrame *pFrame)
{
	return ((uchar *)pFrame - (uchar *)handle - sizeof (void *)) / sizeof(dslFrame);
}

Public void * AdslCoreFrameId2Frame(void *handle, ulong frameId)
{
	return (uchar *)handle + sizeof(void *) + sizeof(dslFrame)*frameId;
}
