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
 * SoftDslFrame.c -- Software Modem main module
 *
 *
 * Description:
 *	This file contains native frame functions 
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1 $
 *
 * $Id: SoftDslFrame.c,v 1.1 2008/08/25 06:41:08 l65130 Exp $
 *
 * $Log: SoftDslFrame.c,v $
 * Revision 1.1  2008/08/25 06:41:08  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:26  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:43  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:58  z45221
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
 * Revision 1.9  2004/04/13 00:18:50  ilyas
 * Added standard header for shared ADSL driver files
 *
 * Revision 1.8  2001/02/10 03:03:09  ilyas
 * Added one more DslFrame function
 *
 * Revision 1.7  2000/07/23 20:52:52  ilyas
 * Added xxxFrameBufSetAddress() function for ATM framer layers
 * Rearranged linkLayer functions in one structure which is passed as a
 * parameter to xxxLinkLayerInit() function to be set there
 *
 * Revision 1.6  2000/07/18 20:03:24  ilyas
 * Changed DslFrame functions definitions to macros,
 * Removed gDslVars from their parameter list
 *
 * Revision 1.5  2000/07/17 21:08:15  lkaplan
 * removed global pointer
 *
 * Revision 1.4  2000/06/09 18:32:56  liang
 * Fixed Irix compiler warnings.
 *
 * Revision 1.3  2000/05/03 03:57:04  ilyas
 * Added LOG file support for writing ATM data
 *
 * Revision 1.2  2000/04/19 00:31:47  ilyas
 * Added global SoftDsl functions for Vc, added OOB info functions
 *
 * Revision 1.1  2000/04/04 01:47:21  ilyas
 * Implemented abstract dslFrame and dslFrameBuffer objects
 *
 *
 ******************************************************************************/

#include "SoftDsl.h"
#include "Que.h"

/*
**
**		Frame buffer processing functions
**
*/

Public ulong DslFrameNativeBufferGetLength(dslFrameBuffer *fb)
{
	return fb->length;
}

Public void * DslFrameNativeBufferGetAddress(dslFrameBuffer *fb)	
{
	/* return ((uchar *)&fb->length + sizeof(fb->length)); */
	return fb->pData;
}

Public void DslFrameNativeBufferSetLength(dslFrameBuffer *fb, ulong l)
{
	fb->length = l;
}

Public void DslFrameNativeBufferSetAddress(dslFrameBuffer *fb, void *p)	
{
	fb->pData = p;
}

/*
**
**		Frame processing functions
**
*/

Public void DslFrameNativeInit(dslFrame *f)
{
	f->totalLength	= 0;
	f->bufCnt		= 0;
	f->head			= NULL;
	f->tail			= NULL;
}

Public ulong DslFrameNativeGetLength (dslFrame *pFrame)
{
	return pFrame->totalLength;
}

Public ulong DslFrameNativeGetBufCnt(dslFrame *pFrame)
{
	return pFrame->bufCnt;
}

Public dslFrameBuffer * DslFrameNativeGetFirstBuffer(dslFrame *pFrame)
{
	return pFrame->head;
}

Public dslFrameBuffer * DslFrameNativeGetNextBuffer(dslFrameBuffer *pFrBuffer)
{
	return pFrBuffer->next;
}

Public void DslFrameNativeSetNextBuffer(dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext)
{
	pFrBuf->next = pFrBufNext;
}

Public dslFrameBuffer * DslFrameNativeGetLastBuffer(dslFrame *pFrame)
{
	return pFrame->tail;
}


Public void * DslFrameNativeGetLinkFieldAddress(dslFrame *f)
{
	return (void *) &f->Reserved;
}

Public dslFrame* DslFrameNativeGetFrameAddressFromLink(void *lnk)
{
	return (void *) ((uchar *)lnk - FLD_OFFSET(dslFrame, Reserved));
}

Public Boolean DslFrameNativeGetOobInfo (dslFrame *f, dslOobFrameInfo *pOobInfo)
{
	return false;
}

Public Boolean DslFrameNativeSetOobInfo (dslFrame *f, dslOobFrameInfo *pOobInfo)
{
	return true;
}

/*
**
**		Queueing frame functions
**
*/

Public void DslFrameNativeEnqueBufferAtBack(dslFrame *f, dslFrameBuffer *b)
{
	QueAdd(&f->head, b);
	f->totalLength += b->length;
	f->bufCnt++;
}

Public void DslFrameNativeEnqueFrameAtBack(dslFrame *fMain, dslFrame *f)
{
	QueMerge(&fMain->head,&f->head);
	fMain->totalLength += f->totalLength;
	fMain->bufCnt	 += f->bufCnt;
}

Public void DslFrameNativeEnqueBufferAtFront(dslFrame *f, dslFrameBuffer *b)
{
	QueAddFirst(&f->head,b);
	f->totalLength += b->length;
	f->bufCnt++;
}

Public void DslFrameNativeEnqueFrameAtFront(dslFrame *fMain, dslFrame *f)
{
	QueMerge(&f->head, &fMain->head);
	QueCopy(&fMain->head,&f->head);
	fMain->totalLength += f->totalLength;
	fMain->bufCnt	 += f->bufCnt;
}


Public dslFrameBuffer * DslFrameNativeDequeBuffer(dslFrame *pFrame)
{
	dslFrameBuffer	*pBuf;

	pBuf = (dslFrameBuffer *) QueFirst((QueHeader *)&pFrame->head);
	if (NULL != pBuf) {
		pFrame->totalLength -= DslFrameNativeBufferGetLength(pBuf);
		pFrame->bufCnt--;
		QueRemove((QueHeader *)&pFrame->head);
	}

	return pBuf;
}

/*
**
**		Frame allocation functions
**
*/

Public void * DslFrameNativeAllocMemForFrames(ulong frameNum)
{
	return NULL;
}

Public void DslFrameNativeFreeMemForFrames(void *hMem)
{
}

Public dslFrame * DslFrameNativeAllocFrame(void *handle)
{
	return NULL;
}

Public void DslFrameNativeFreeFrame(void *handle, dslFrame *pFrame)
{
}

Public void * DslFrameNativeAllocMemForBuffers(void **ppMemPool, ulong bufNum, ulong memSize)
{
	return NULL;
}

Public void DslFrameNativeFreeMemForBuffers(void *hMem, ulong memSize, void *pMemPool)
{
}

Public dslFrameBuffer * DslFrameNativeAllocBuffer(void *handle, void *pMem, ulong length)
{
	return NULL;
}

Public void DslFrameNativeFreeBuffer(void *handle, dslFrameBuffer *pBuf)
{
}

Public ulong DslFrameNative2Id (void *handle, dslFrame *pFrame)
{
	return 0;
}

Public void * DslFrameNativeId2Frame (void *handle, ulong frameId)
{
	return NULL;
}
