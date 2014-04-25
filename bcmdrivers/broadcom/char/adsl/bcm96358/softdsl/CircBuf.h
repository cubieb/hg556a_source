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
 * CircBuf -- Generic Circular Buffer
 *
 * Description:
 *	Implementation of generic circular buffer algorithms
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: CircBuf.h,v 1.1.2.1 2009/11/19 06:39:14 l43571 Exp $
 *
 * $Log: CircBuf.h,v $
 * Revision 1.1.2.1  2009/11/19 06:39:14  l43571
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
 * Revision 1.1  2008/01/14 02:47:17  z30370
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
 * Revision 1.14  2004/06/24 03:10:37  ilyas
 * Added extra macro to be able to use un-cached variable (for status write)
 *
 * Revision 1.13  2004/02/09 23:47:02  ilyas
 * Fixed last change
 *
 * Revision 1.12  2004/02/06 22:52:58  ilyas
 * Improved stretch buffer write
 *
 * Revision 1.11  2002/12/30 23:27:55  ilyas
 * Added macro for HostDma optimizations
 *
 * Revision 1.10  2002/10/26 02:15:02  ilyas
 * Optimized and added new macros for HostDma
 *
 * Revision 1.9  2002/01/22 23:59:29  ilyas
 * Added paraenthesis around macro argument
 *
 * Revision 1.8  2002/01/15 22:28:38  ilyas
 * Extended macro to support readPtr from uncached address
 *
 * Revision 1.7  2001/09/21 19:47:05  ilyas
 * Fixed compiler warnings for VxWorks build
 *
 * Revision 1.6  2001/06/07 18:47:56  ilyas
 * Added more macros for circular buffer arithmetics
 *
 * Revision 1.5  2001/04/18 03:58:34  ilyas
 * Added LOG file write granularity
 *
 * Revision 1.4  2001/01/19 04:34:12  ilyas
 * Added more macros to circular buffer implementation
 *
 * Revision 1.3  2001/01/06 04:01:41  ilyas
 * Changed the way we write status messages
 *
 * Revision 1.2  2001/01/04 05:52:21  ilyas
 * Added implementation of stretchable circular buffer used in LOG and Status
 * handlers
 *
 * Revision 1.1  2000/05/03 03:45:55  ilyas
 * Original implementation
 *
 *
 *****************************************************************************/

#ifndef	CircBufHeader_H_
#define	CircBufHeader_H_


typedef struct {
	char	*pStart;
	char	*pEnd;
	char	*pRead;
	char	*pWrite;
} circBufferStruct;

/* Initialize circular buffer */

#define	CircBufferInit(pCB,buf,size)	do {			\
	(pCB)->pStart = (char *) (buf);						\
	(pCB)->pRead = (pCB)->pWrite = (pCB)->pStart;		\
	(pCB)->pEnd = (pCB)->pStart + size;					\
} while (0)

#define	CircBufferGetSize(pCB)			((pCB)->pEnd - (pCB)->pStart)
#define	CircBufferGetStartPtr(pCB)		((void *) (pCB)->pStart)
#define	CircBufferGetEndPtr(pCB)		((void *) (pCB)->pEnd)

#define	CircBufferReset(pCB)			(pCB)->pRead = (pCB)->pWrite = (pCB)->pStart


#define	CircBufferGetReadPtr(pCB)		((void *) (pCB)->pRead)
#define	CircBufferGetWritePtr(pCB)		((void *) (pCB)->pWrite)


#ifndef bcm47xx
#define	CircBufferDistance(pCB,p1,p2,d)	((char*)(p2) - (char*)(p1) - d >= 0 ?			\
											(char*)(p2) - (char*)(p1) - d :			\
											((char*)(p2)- (char*)(p1) - d + ((pCB)->pEnd - (pCB)->pStart)))

#define	CircBufferAddContig(pCB,p,n)	((char*)(p) + (n) == (pCB)->pEnd ? (pCB)->pStart : (char*)(p) + (n))
#else
static __inline int CircBufferDistance(circBufferStruct *pCB, char *p1, char *p2, int d)
{
	int tmp = p2 - p1 - d;

	return (tmp >= 0 ? tmp : tmp + (pCB->pEnd - pCB->pStart));
}

static __inline char * CircBufferAddContig(circBufferStruct *pCB, char *p, int n)
{
	p += n;
	return (p == pCB->pEnd ? pCB->pStart : p);
}
#endif

#define	CircBufferAdd(pCB,p,n)			((char*)(p) + (n) >= (pCB)->pEnd ?						\
											(pCB)->pStart + ((char*)(p) + (n) - (pCB)->pEnd) :	\
											(char*)(p) + (n))

#define	CircBufferReadUpdate(pCB,n)		(pCB)->pRead = CircBufferAdd(pCB,(pCB)->pRead,n)
#define	CircBufferWriteUpdate(pCB,n)	(pCB)->pWrite= CircBufferAdd(pCB,(pCB)->pWrite,n)

#define	CircBufferReadUpdateContig(pCB,n)	(pCB)->pRead = CircBufferAddContig(pCB,(pCB)->pRead,n)
#define	CircBufferWriteUpdateContig(pCB,n)	(pCB)->pWrite= CircBufferAddContig(pCB,(pCB)->pWrite,n)

#define	CircBufferGetReadAvail(pCB)		CircBufferDistance(pCB,(pCB)->pRead,(pCB)->pWrite,0)
#define	CircBufferIsReadEmpty(pCB)		((pCB)->pRead == (pCB)->pWrite)
#define	CircBufferGetWriteAvail(pCB)	CircBufferDistance(pCB,(pCB)->pWrite,(pCB)->pRead,1)
#define	CircBufferGetWriteAvailN(pCB,n)	CircBufferDistance(pCB,(pCB)->pWrite,(pCB)->pRead,n)

#define	CircBufferGetReadContig(pCB)	((unsigned long)(pCB)->pWrite >= (unsigned long) (pCB)->pRead ?	\
											(pCB)->pWrite - (pCB)->pRead :		\
											(pCB)->pEnd	  - (pCB)->pRead)

#define	CircBufferGetWriteContig(pCB)	((pCB)->pEnd - (pCB)->pWrite > CircBufferGetWriteAvail(pCB) ?	\
											CircBufferGetWriteAvail(pCB) :		\
											(pCB)->pEnd - (pCB)->pWrite)

/*
**
**		structure and macros for "strectch" buffer
**
*/

typedef struct {
	char	*pStart;
	char	*pEnd;
	char	*pExtraEnd;
	char	*pStretchEnd;
	char	*pRead;
	char	*pWrite;
} stretchBufferStruct;

#define	StretchBufferInit(pSB,buf,size,extra)	do {	\
	(pSB)->pStart = (char *) (buf);						\
	(pSB)->pRead = (pSB)->pWrite = (pSB)->pStart;		\
	(pSB)->pEnd = (pSB)->pStart + (size);				\
	(pSB)->pStretchEnd = (pSB)->pEnd;					\
	(pSB)->pExtraEnd = (pSB)->pEnd+(extra);				\
} while (0)

#define	StretchBufferGetSize(pSB)		((pSB)->pEnd - (pSB)->pStart)
#define	StretchBufferGetStartPtr(pSB)	((void *) (pSB)->pStart)
#define	StretchBufferGetReadPtr(pSB)	((void *) (pSB)->pRead)
#define	StretchBufferGetWritePtr(pSB)	((void *) (pSB)->pWrite)
#define	StretchBufferReset(pSB)			((pSB)->pRead = (pSB)->pWrite = (pSB)->pStart)

#define	StretchBufferGetReadToEnd(pSB)	((pSB)->pStretchEnd - (pSB)->pRead)

#define	StretchBufferGetReadAvail(pSB)		((pSB)->pWrite - (pSB)->pRead >= 0 ?	\
											(pSB)->pWrite - (pSB)->pRead   :		\
											(pSB)->pStretchEnd - (pSB)->pRead)
#define	_StretchBufferGetWriteAvail(pSB,rd) ((rd) - (pSB)->pWrite > 0 ?				\
											(rd) - (pSB)->pWrite - 1 :				\
											((pSB)->pExtraEnd - (pSB)->pWrite))
#define	StretchBufferGetWriteAvail(pSB)		_StretchBufferGetWriteAvail(pSB, (pSB)->pRead)

#define	StretchBufferReadUpdate(pSB,n)		do {								\
	char	*p;																	\
																				\
	p = (pSB)->pRead + (n);														\
	(pSB)->pRead = (p >= (pSB)->pStretchEnd ? (pSB)->pStart : p);				\
} while (0)

#define	_StretchBufferWriteUpdate(pSB,rd,n)	do {								\
	char	*p;																	\
																				\
	p = (pSB)->pWrite + (n);													\
	if (p >= (pSB)->pEnd) {														\
	  if ((rd) != (pSB)->pStart) {												\
		(pSB)->pStretchEnd = p;													\
		(pSB)->pWrite = (pSB)->pStart;											\
	  }																			\
	}																			\
	else																		\
		(pSB)->pWrite = p;														\
} while (0)

#define	StretchBufferWriteUpdate(pSB,n)		_StretchBufferWriteUpdate(pSB,(pSB)->pRead,n)	

#endif	/* CircBufHeader_H_ */



