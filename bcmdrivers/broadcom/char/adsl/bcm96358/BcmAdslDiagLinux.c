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
 * BcmCoreTest.c -- Bcm ADSL core driver main
 *
 * Description:
 *	This file contains BCM ADSL core driver system interface functions
 *
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.1.2.1 $
 *
 * $Id: BcmAdslDiagLinux.c,v 1.1.2.1 2009/11/19 06:39:10 l43571 Exp $
 *
 * $Log: BcmAdslDiagLinux.c,v $
 * Revision 1.1.2.1  2009/11/19 06:39:10  l43571
 * 【变更分类】
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/08/25 06:40:56  l65130
 * 【变更分类】建立基线
 * 【问题单号】
 * 【问题描述】
 * 【修改说明】
 *
 * Revision 1.1  2008/06/20 06:21:17  z67625
 * *** empty log message ***
 *
 * Revision 1.1  2008/01/14 02:47:01  z30370
 * *** empty log message ***
 *
 * Revision 1.2  2007/12/16 10:09:53  z45221
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
 * Revision 1.3  2004/07/20 23:45:48  ilyas
 * Added driver version info, SoftDslPrintf support. Fixed G.997 related issues
 *
 * Revision 1.2  2004/06/10 00:20:33  ilyas
 * Added L2/L3 and SRA
 *
 * Revision 1.1  2004/04/14 21:11:59  ilyas
 * Inial CVS checkin
 *
 ****************************************************************************/

/* Includes. */
#include "BcmOs.h"

#include <linux/types.h>
#define	_SYS_TYPES_H

#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>

#include <net/ip.h>
#include <net/route.h>
#include <net/arp.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include "bcm_common.h"
#include "bcm_map.h"
#include "bcm_intr.h"
#else
#include "6345_common.h"
#include "6345_map.h"
#include "6345_intr.h"
#endif
#include "board.h"

#include "bcmadsl.h"
#include "BcmAdslCore.h"
#include "AdslCore.h"
#include "AdslCoreMap.h"

#define EXCLUDE_CYGWIN32_TYPES
#include "softdsl/SoftDsl.h"

#include "BcmAdslDiag.h"
#include "DiagDef.h"

//#undef	DIAG_DBG

extern dslCommandStruct	adslCoreConnectionParam;
extern Bool	adslCoreInitialized;
extern void BcmAdslCoreReset(void);

void BcmAdslCoreDiagDataInit(void);
void BcmAdslCoreDiagCommand(void);

#define DMA_INTR_MASK		(ADSL_INT_RCV | ADSL_INT_RCV_FIFO_OF | ADSL_INT_RCV_DESC_UF | \
							ADSL_INT_DESC_PROTO_ERR |ADSL_INT_DATA_ERR |ADSL_INT_DESC_ERR)
#define DMA_ERROR_MASK	(ADSL_INT_RCV_FIFO_OF | ADSL_INT_RCV_DESC_UF | \
							ADSL_INT_DESC_PROTO_ERR |ADSL_INT_DATA_ERR |ADSL_INT_DESC_ERR)
#define PHY_INTR_ENABLE(x)	(x[ADSL_INTMASK_F] |= DMA_INTR_MASK); \
							(x[ADSL_INTMASK_I] |= ADSL_INT_HOST_MSG)
#define PHY_INTR_DISABLE(x)	(x[ADSL_INTMASK_F]  &= ~DMA_INTR_MASK); \
							(x[ADSL_INTMASK_I] &= ~ADSL_INT_HOST_MSG)
							
/* Local vars */

#ifdef DIAG_DBG
ulong			diagRxIntrCnt = 0;
ulong			diagSkipWrDmaBlkCnt =0;
#endif

ulong			diagMaxLpPerSrvCnt =0;
ulong			diagDmaErrorCnt =0;
ulong			diagDmaTotalDataLen = 0;

ulong			diagDataMap = 0;
ulong			diagLogTime = 0;
ulong			diagTotalBufNum = 0;

ulong			diagDmaIntrCnt = 0;
ulong			diagDmaBlockCnt = 0;
ulong			diagDmaOvrCnt = 0;
ulong			diagDmaSeqBlockNum = 0;
ulong			diagDmaSeqErrCnt = 0;
ulong			diagDmaBlockWrCnt = 0;
ulong			diagDmaBlockWrErrCnt = 0;
ulong			diagDmaBlockWrBusy = 0;
ulong			diagDmaLogBlockNum = 0;

int				diagEnableCnt = 0;
void *			diagDpcId = NULL;

/*
**
**	Socket diagnostic support
**
*/

#define	DIAG_SKB_USERS			0x3FFFFFFF

#define	UNCACHED(p)				((void *)((long)(p) | 0x20000000))
#define	CACHED(p)				((void *)((long)(p) & ~0x20000000))

#define kDiagDmaBlockSizeShift	11
#define kDiagDmaBlockSize		(1 << kDiagDmaBlockSizeShift)

#define DIAG_DESC_TBL_ALIGN_SIZE	0x1000
#define DIAG_DESC_TBL_MAX_SIZE		0x800
#define DIAG_DESC_TBL_SIZE(x)		((x) * sizeof(adslDmaDesc))
#define DIAG_DMA_BLK_SIZE			( sizeof(diagDmaBlock) )

struct net_device	*dbgDev = NULL;
struct sk_buff		*skbModel = NULL;
struct sk_buff		*skbModel2 = NULL;

typedef struct _diagIpHeader {
	uchar	ver_hl;			/* version & header length */
	uchar	tos;			/* type of service */
	ushort	len;			/* total length */
	ushort	id;				/* identification */
	ushort	off;			/* fragment offset field */
	uchar	ttl;			/* time to live */
	uchar	proto;			/* protocol */
	ushort	checksum;		/* checksum */
	ulong	srcAddr;
	ulong	dstAddr;		/* source and dest address */
} diagIpHeader;

typedef struct _diagUdpHeader {
	ushort	srcPort;		/* source port */
	ushort	dstPort;		/* destination port */
	ushort	len;			/* udp length */
	ushort	checksum;		/* udp checksum */
} diagUdpHeader;

#define DIAG_FRAME_PAD_SIZE		2
#define DIAG_DMA_MAX_DATA_SIZE	1200
#define DIAG_FRAME_HEADER_LEN	(sizeof(diagSockFrame) - DIAG_DMA_MAX_DATA_SIZE - DIAG_FRAME_PAD_SIZE)

typedef struct _diagSockFrame {
	uchar			pad[DIAG_FRAME_PAD_SIZE];
	struct ethhdr		eth;
	diagIpHeader		ipHdr;
	diagUdpHeader	udpHdr;
	LogProtoHeader	diagHdr;
	uchar			diagData[DIAG_DMA_MAX_DATA_SIZE];
} diagSockFrame;

typedef struct {
	struct sk_buff			skb;
	ulong				len;
	ulong				frameNum;
	ulong				mark;
	LogProtoHeader		diagHdrDma;
	diagSockFrame			dataFrame;
	struct skb_shared_info	skbShareInfo;
} diagDmaBlock;

uchar		*diagBuf, *diagBufUC;
diagDmaBlock	*diagStartBlock = NULL;
diagDmaBlock	*diagWriteBlock = NULL;
diagDmaBlock	*diagCurrBlock = NULL;
diagDmaBlock	*diagEndBlock  = NULL;
diagDmaBlock	*diagEyeBlock = NULL;

typedef struct {
	ulong		flags;
	ulong		addr;
} adslDmaDesc;

adslDmaDesc         *diagDescTbl = NULL;
adslDmaDesc         *diagDescTblUC = NULL;

void                        *diagDescMemStart = NULL;
ulong                       diagDescMemSize = 0;

static ushort DiagIpComputeChecksum(diagIpHeader *ipHdr)
{
	ushort	*pHdr = (ushort	*) ipHdr;
	ushort	*pHdrEnd = pHdr + 10;
	ulong	sum = 0;

	do {
		sum += pHdr[0];
		sum += pHdr[1];
		pHdr += 2;
	} while (pHdr != pHdrEnd);

	while (sum > 0xFFFF)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return sum ^ 0xFFFF;
}

static ushort DiagIpUpdateChecksum(int sum, ushort oldv, ushort newv)
{
	ushort	tmp;

	tmp = (newv - oldv);
	tmp -= (tmp >> 15);
	sum = (sum ^ 0xFFFF) + tmp;
	sum = (sum & 0xFFFF) + (sum >> 16);
	return sum ^ 0xFFFF;
}

static void DiagUpdateDataLen(diagSockFrame *diagFr, int dataLen)
{
	int		n;

	diagFr->udpHdr.len = dataLen + sizeof(LogProtoHeader) + sizeof(diagUdpHeader);
	n = diagFr->udpHdr.len + sizeof(diagIpHeader);

	diagFr->ipHdr.checksum = DiagIpUpdateChecksum(diagFr->ipHdr.checksum, diagFr->ipHdr.len, n);
	diagFr->ipHdr.len = n;
}

#if 0
static void DiagPrintData(struct sk_buff *skb)
{
	int		i;

	printk ("***SKB: dev=0x%X, hd=0x%X, dt=0x%X, tl=0x%X, end=0x%X, len=%d, users=%d\n", 
		(int)skb->dev,
		(int)skb->head, (int)skb->data, (int)skb->tail, (int)skb->end, skb->len, 
		atomic_read(&skb->users));

	for (i = 0; i < skb->len; i++)
		printk("%X ", skb->data[i]);
	printk("\n");
}
#endif

static int __DiagWriteData(struct sk_buff *skbDiag, ulong cmd, char *buf0, int len0, char *buf1, int len1)
{
	diagSockFrame		*dd;
	int					n;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	ulong				flags;
#endif

	dd = (diagSockFrame *) skbDiag->head;
	DiagUpdateDataLen(dd, len0+len1);

	if (cmd & DIAG_MSG_NUM)
		n = diagDmaLogBlockNum++;
	else {
		n = *(short *) LOG_PROTO_ID;
		if (cmd & DIAG_SPLIT_MSG)
			n++;
	}

	*(short *)dd->diagHdr.logProtoId = n;
	dd->diagHdr.logPartyId	= LOG_PARTY_CLIENT;
	dd->diagHdr.logCommmand = cmd & 0xFF;
	memcpy (dd->diagData, buf0, len0);
	if (NULL != buf1)
		memcpy (dd->diagData+len0, buf1, len1);

	skbDiag->data = skbDiag->head + DIAG_FRAME_PAD_SIZE;
	skbDiag->len  = DIAG_FRAME_HEADER_LEN + len0 + len1;
	skbDiag->tail = skbDiag->data + skbDiag->len;

	/* DiagPrintData(skbDiag); */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	local_irq_save(flags);
	local_irq_enable();
#endif
	n = dev_queue_xmit(skbDiag);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	local_irq_restore(flags);
#endif
	return n;
}

int DiagWriteData(struct net_device * dev, ulong cmd, char *buf0, int len0, char *buf1, int len1)
{
	struct sk_buff	 *skb,*skb2;

	if (NULL == dbgDev)
		return 0;
	skb = alloc_skb (DIAG_FRAME_PAD_SIZE + DIAG_FRAME_HEADER_LEN + len0 + len1 + 16, GFP_ATOMIC);
	if (NULL == skb)
		return -ENOMEM;
	if(skbModel != NULL){
	skb->dev = dev;
	skb->protocol = eth_type_trans (skb, dev);
	skb->data = skb->head + DIAG_FRAME_PAD_SIZE;
	memcpy(skb->data, skbModel->data, DIAG_FRAME_HEADER_LEN);
	__DiagWriteData(skb, cmd, buf0, len0, buf1, len1);
	}
	if(skbModel2 != NULL){
	skb2 = alloc_skb (DIAG_FRAME_PAD_SIZE + DIAG_FRAME_HEADER_LEN + len0 + len1 + 16, GFP_ATOMIC);
	if (NULL == skb2)
		return -ENOMEM;

	skb2->dev = dev;
	skb2->protocol = eth_type_trans (skb2, dev);
	skb2->data = skb2->head + DIAG_FRAME_PAD_SIZE;
	memcpy(skb2->data, skbModel2->data, DIAG_FRAME_HEADER_LEN);
	return __DiagWriteData(skb2, cmd, buf0, len0, buf1, len1);
	}
	else return -ENOMEM;
}

void BcmAdslCoreDiagWriteStatusData(ulong cmd, char *buf0, int len0, char *buf1, int len1)
{
	DiagWriteData(dbgDev, cmd, buf0, len0, buf1, len1);
}

int BcmAdslCoreDiagWrite(void *pBuf, int len)
{
	DiagProtoFrame	*pDiagFrame = (DiagProtoFrame *) pBuf;

	return DiagWriteData(dbgDev, pDiagFrame->diagHdr.logCommmand, pDiagFrame->diagData, len - sizeof(LogProtoHeader), NULL, 0);
}

#define DiagWriteMibData(dev,buf,len)		DiagWriteData(dev,LOG_CMD_MIB_GET,buf,len,NULL,0)
#define DiagWriteStatusString(dev,str)		DiagWriteData(dev,LOG_CMD_SCRAMBLED_STRING,str,strlen(str)+1,NULL,0)

#if 0
#include "softdsl/StatusParser.h"
LOCAL void BcmAdslCoreDiagWriteStatusOrig(dslStatusStruct *status)
{
	static	char	statStr[4096];
	static	char	statStrAnnex[] = "\n";
	long			n, len;
	char			ch1 = 0, ch2 = 0, *pStr;

	if (NULL == dbgDev)
		return;

	StatusParser (status, statStr);
	if (statStr[0] == 0)
		return;

	strcat(statStr, statStrAnnex);
	len = strlen(statStr);
	pStr = statStr;

	while (len > (LOG_MAX_DATA_SIZE-1)) {
		n = LOG_MAX_DATA_SIZE-1;
		ch1 = pStr[n-1];
		ch2 = pStr[n];
		pStr[n-1] = 1;
		pStr[n] = 0;

		DiagWriteData(dbgDev, LOG_CMD_STRING_DATA, pStr, n + 1, NULL, 0);

		pStr[n-1] = ch1;
		pStr[n] = ch2;
		pStr += (n-1);
		len -= (n - 1);
	}
	DiagWriteData(dbgDev, LOG_CMD_STRING_DATA, pStr, len + 1, NULL, 0);
}
#endif

void BcmAdslCoreDiagWriteLog(logDataCode logData, ...)
{
	static	char	logDataBuf[512];
	char			*logDataPtr = logDataBuf;
	long			n, i, datum, *pCmdData;
	va_list			ap;

	if ((NULL == dbgDev) || (0 == (diagDataMap & DIAG_DATA_LOG)))
		return;

	va_start(ap, logData);

	switch	(logData) {
		case	commandInfoData:
			logDataPtr += sprintf(logDataPtr, "%d:\t", (int)logData);	
			pCmdData = (void *) va_arg(ap, long);
			n = va_arg(ap, long);
			for (i = 0; i < n ; i++)
				logDataPtr += sprintf(logDataPtr, "%ld ", pCmdData[i]);	
			logDataPtr += sprintf(logDataPtr, "\n");	
			break;
		case (inputSignalData - 2):
			datum = va_arg(ap, long);
			*logDataPtr++ = (char) datum;	
			break;
		default:
			break;
	}

	if (logDataPtr != logDataBuf)
		DiagWriteData(dbgDev, logData | DIAG_MSG_NUM, logDataBuf, (logDataPtr - logDataBuf), NULL, 0);
	va_end(ap);
}

static void DiagUpdateSkbForDmaBlock(diagDmaBlock *db, int len)
{
	DiagUpdateDataLen(&db->dataFrame, len);
	*(ulong*)&db->dataFrame.diagHdr = *(ulong*)&db->diagHdrDma;

	db->skb.data = db->skb.head + DIAG_FRAME_PAD_SIZE;
	db->skb.len  = DIAG_FRAME_HEADER_LEN + len;
	db->skb.tail = db->skb.data + DIAG_FRAME_HEADER_LEN + len;

	/* DiagPrintData(skbDiag); */
}
#if 0
static int DiagWriteDmaBlock(diagDmaBlock *db)
{
	int	n;

	/* pretest if the device is busy */
	if (db->skb.dev->xmit_lock_owner != -1) {
		diagDmaBlockWrBusy++;
		return -1;
	}
	
	n = dev_queue_xmit(&db->skb);

	if (n != 0) {
		diagDmaBlockWrErrCnt++;
#ifdef DIAG_DBG
		printk("%s: Error=%d, packetID=0x%X\n", __FUNCTION__, n, db->dataFrame.ipHdr.id);
#endif
		atomic_set(&db->skb.users, DIAG_SKB_USERS);
	}
	
	return n;
}
#endif

struct net_device * BcmAdslCoreGuiInit(PADSL_DIAG pAdslDiag)
{
	char				*diagDevNames[] = { "eth0", "br0" };
	struct net_device	*dev;
	struct in_device	*in_dev;
	struct in_ifaddr	**ifap;
	struct in_ifaddr	*ifa;
	struct rtable		rtbl;
	diagSockFrame		*dd;
	int					n, dstPort;

	dstPort = ((ulong) pAdslDiag->diagMap) >> 16;
	if (0 == dstPort)
		dstPort = LOG_FILE_PORT2;
	printk ("DrvGuiCmd: CONNECT map=0x%X, logTime=%d srvIpAddr=0x%X gwIpAddr=0x%X dstPort=%d\n", 
		pAdslDiag->diagMap, pAdslDiag->logTime, pAdslDiag->srvIpAddr, pAdslDiag->gwIpAddr, dstPort);
	/* find DIAG interface device */

	ifa = NULL;
	for (n = 0; n < sizeof(diagDevNames)/sizeof(diagDevNames[0]); n++) {
		dev = __dev_get_by_name(diagDevNames[n]);
		if (NULL == dev)
			continue;

		printk ("dev = %s(0x%X) hwAddr=%X:%X:%X:%X:%X:%X\n", 
			diagDevNames[n], (int) dev,
			dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
			dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

		/* get device IP address */

		in_dev = __in_dev_get(dev);
		if (NULL == in_dev)
			continue;

		for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
			printk ("ifa_label = %s, ipAddr = 0x%X mask = 0x%X\n", ifa->ifa_label, ifa->ifa_local, ifa->ifa_mask);
			if (strcmp(diagDevNames[n], ifa->ifa_label) == 0)
				break;
		}

		if (ifa != NULL)
			break;
	}

	if (ifa == NULL)
		return NULL;

	/* get remote MAC address for Diag srvIpAddr */

	if (NULL == skbModel2) {
		skbModel2 = alloc_skb (DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
		if (skbModel2 == NULL)
			return NULL;
	}
	skbModel2->dev = dev;
	skbModel2->protocol = eth_type_trans (skbModel2, dev);
	dd = (diagSockFrame *) skbModel2->head;
	dd->eth.h_proto = htons(ETH_P_IP);
	memcpy(dd->eth.h_source, dev->dev_addr, ETH_ALEN);
	memset(dd->eth.h_dest, 0, ETH_ALEN);
	skbModel2->dst = (void *) &rtbl;

	if ((ifa->ifa_local & ifa->ifa_mask) == (pAdslDiag->srvIpAddr & ifa->ifa_mask)) {
		printk ("Diag server is on the same subnet\n");
		rtbl.rt_gateway = pAdslDiag->srvIpAddr;
		n = arp_find(dd->eth.h_dest, skbModel2);
	}
	else if (pAdslDiag->gwIpAddr != 0) {
		printk ("Diag server is outside subnet, using gateway IP address = 0x%X\n", pAdslDiag->gwIpAddr);
		rtbl.rt_gateway = pAdslDiag->gwIpAddr;
		n = arp_find(dd->eth.h_dest, skbModel2);
	}
	else {
		printk ("Diag server is outside subnet and no gateway specified. Diag support impossible\n");
		n = 1;
	}

	if (n != 0) {
		skbModel2 = NULL;
		return NULL;
	}
	printk ("srvMACAddr = %X:%X:%X:%X:%X:%X\n", 
		dd->eth.h_dest[0], dd->eth.h_dest[1], dd->eth.h_dest[2],
		dd->eth.h_dest[3], dd->eth.h_dest[4], dd->eth.h_dest[5]);

	/* check dd->eth.h_dest[0..5] != 0 (TBD) */

	dd->ipHdr.ver_hl = 0x45;
	dd->ipHdr.tos = 0;
	dd->ipHdr.len = 0;			/* changes for frames */
	dd->ipHdr.id = 0x2000;
	dd->ipHdr.off = 0;
	dd->ipHdr.ttl =128;
	dd->ipHdr.proto = 0x11;		/* always UDP */
	dd->ipHdr.checksum = 0;		/* changes for frames */
	dd->ipHdr.srcAddr = ifa->ifa_local;
	dd->ipHdr.dstAddr = pAdslDiag->srvIpAddr;
	dd->ipHdr.checksum = DiagIpComputeChecksum(&dd->ipHdr);

	dd->udpHdr.srcPort = LOG_FILE_PORT2;
	dd->udpHdr.dstPort = dstPort;
	dd->udpHdr.len = 0;			/* changes for frames */
	dd->udpHdr.checksum = 0;	/* not used */

	/* to prevent skb from deallocation */

	skbModel2->data = skbModel2->head + DIAG_FRAME_PAD_SIZE;
	atomic_set(&skbModel2->users, DIAG_SKB_USERS);

	diagDataMap = pAdslDiag->diagMap & 0xFFFF;
	diagLogTime = pAdslDiag->logTime;
	diagEnableCnt = 1;

	return dev;
}

struct net_device * BcmAdslCoreDiagInit(PADSL_DIAG pAdslDiag)
{
	char				*diagDevNames[] = { "eth0", "br0" };
	struct net_device	*dev;
	struct in_device	*in_dev;
	struct in_ifaddr	**ifap;
	struct in_ifaddr	*ifa;
	struct rtable		rtbl;
	diagSockFrame		*dd;
	int					n, dstPort;

	dstPort = ((ulong) pAdslDiag->diagMap) >> 16;
	if (0 == dstPort)
		dstPort = LOG_FILE_PORT;
	printk ("%s: map=0x%X, logTime=%d srvIpAddr=0x%X gwIpAddr=0x%X dstPort=%d\n", 
		__FUNCTION__, pAdslDiag->diagMap, pAdslDiag->logTime, pAdslDiag->srvIpAddr, pAdslDiag->gwIpAddr, dstPort);
	/* find DIAG interface device */

	ifa = NULL;
	for (n = 0; n < sizeof(diagDevNames)/sizeof(diagDevNames[0]); n++) {
		dev = __dev_get_by_name(diagDevNames[n]);
		if (NULL == dev)
			continue;

		printk ("dev = %s(0x%X) hwAddr=%X:%X:%X:%X:%X:%X\n", 
			diagDevNames[n], (int) dev,
			dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
			dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

		/* get device IP address */

		in_dev = __in_dev_get(dev);
		if (NULL == in_dev)
			continue;

		for (ifap=&in_dev->ifa_list; (ifa=*ifap) != NULL; ifap=&ifa->ifa_next) {
			printk ("ifa_label = %s, ipAddr = 0x%X mask = 0x%X\n", ifa->ifa_label, ifa->ifa_local, ifa->ifa_mask);
			if (strcmp(diagDevNames[n], ifa->ifa_label) == 0)
				break;
		}

		if (ifa != NULL)
			break;
	}

	if (ifa == NULL)
		return NULL;

	/* get remote MAC address for Diag srvIpAddr */

	if (NULL == skbModel) {
		skbModel = alloc_skb (DIAG_FRAME_HEADER_LEN + 32, GFP_ATOMIC);
		if (skbModel == NULL)
			return NULL;
	}
	skbModel->dev = dev;
	skbModel->protocol = eth_type_trans (skbModel, dev);
	dd = (diagSockFrame *) skbModel->head;
	dd->eth.h_proto = htons(ETH_P_IP);
	memcpy(dd->eth.h_source, dev->dev_addr, ETH_ALEN);
	memset(dd->eth.h_dest, 0, ETH_ALEN);
	skbModel->dst = (void *) &rtbl;

	if ((ifa->ifa_local & ifa->ifa_mask) == (pAdslDiag->srvIpAddr & ifa->ifa_mask)) {
		printk ("Diag server is on the same subnet\n");
		rtbl.rt_gateway = pAdslDiag->srvIpAddr;
		n = arp_find(dd->eth.h_dest, skbModel);
	}
	else if (pAdslDiag->gwIpAddr != 0) {
		printk ("Diag server is outside subnet, using gateway IP address = 0x%X\n", pAdslDiag->gwIpAddr);
		rtbl.rt_gateway = pAdslDiag->gwIpAddr;
		n = arp_find(dd->eth.h_dest, skbModel);
	}
	else {
		printk ("Diag server is outside subnet and no gateway specified. Diag support impossible\n");
		n = 1;
	}

	if (n != 0) {
		skbModel = NULL;
		return NULL;
	}
	printk ("srvMACAddr = %X:%X:%X:%X:%X:%X\n", 
		dd->eth.h_dest[0], dd->eth.h_dest[1], dd->eth.h_dest[2],
		dd->eth.h_dest[3], dd->eth.h_dest[4], dd->eth.h_dest[5]);

	/* check dd->eth.h_dest[0..5] != 0 (TBD) */

	dd->ipHdr.ver_hl = 0x45;
	dd->ipHdr.tos = 0;
	dd->ipHdr.len = 0;			/* changes for frames */
	dd->ipHdr.id = 0x2000;
	dd->ipHdr.off = 0;
	dd->ipHdr.ttl =128;
	dd->ipHdr.proto = 0x11;		/* always UDP */
	dd->ipHdr.checksum = 0;		/* changes for frames */
	dd->ipHdr.srcAddr = ifa->ifa_local;
	dd->ipHdr.dstAddr = pAdslDiag->srvIpAddr;
	dd->ipHdr.checksum = DiagIpComputeChecksum(&dd->ipHdr);

	dd->udpHdr.srcPort = LOG_FILE_PORT;
	dd->udpHdr.dstPort = dstPort;
	dd->udpHdr.len = 0;			/* changes for frames */
	dd->udpHdr.checksum = 0;	/* not used */

	/* to prevent skb from deallocation */

	skbModel->data = skbModel->head + DIAG_FRAME_PAD_SIZE;
	atomic_set(&skbModel->users, DIAG_SKB_USERS);

	diagDataMap = pAdslDiag->diagMap & 0xFFFF;
	diagLogTime = pAdslDiag->logTime;
	diagEnableCnt = 1;

	return dev;
}

#ifndef CONFIG_BCM96368
void BcmAdslCoreDiagDataFlush(void)
{
	int				i, n = 0;
	diagDmaBlock	*diagPtr;

	do {
		diagPtr = (void *) diagStartBlock;
		diagPtr = CACHED(diagPtr);

		for (i = 0; i < diagTotalBufNum; i++) {
			n = atomic_read(&diagPtr->skb.users);
			if ((diagPtr->dataFrame.pad[0] != 0) && (DIAG_SKB_USERS == n))
				break;

			diagPtr = (void *) (((char *) diagPtr)  + DIAG_DMA_BLK_SIZE);
		}

		if (i >= diagTotalBufNum)
			break;
				
#ifdef DIAG_DBG
		printk ("DiagDataFlush waiting for block %d to be sent. users = 0x%X\n", i, n);
#endif		
	} while (1);
}
#endif

void BcmAdslCoreDiagCommand(void)
{
	dslCommandStruct	cmd;

	cmd.command = kDslDiagSetupCmd;
	cmd.param.dslDiagSpec.setup = 0;
	if (diagDataMap & DIAG_DATA_EYE)
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableEyeData;
	if (diagDataMap & DIAG_DATA_LOG) {
		cmd.param.dslDiagSpec.setup |= kDslDiagEnableLogData;
		diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagWriteLog(inputSignalData - 2, AC_TRUE);
		diagDmaLogBlockNum = 0;
	}
	cmd.param.dslDiagSpec.eyeConstIndex1 = 63; 
	cmd.param.dslDiagSpec.eyeConstIndex2 = 64;
	cmd.param.dslDiagSpec.logTime = diagLogTime;
	BcmCoreCommandHandlerSync(&cmd);
}

LOCAL void __BcmAdslCoreDiagDmaInit(void)
{
#ifndef CONFIG_BCM96368
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;

#ifdef DIAG_DBG
	diagRxIntrCnt = 0;
	diagSkipWrDmaBlkCnt = 0;
#endif
	diagDmaTotalDataLen = 0;
	diagMaxLpPerSrvCnt = 0;	
	diagDmaErrorCnt = 0;
	diagDmaIntrCnt = 0;

	diagDmaSeqBlockNum = 0;
	diagDmaBlockCnt = 0;
	diagDmaOvrCnt = 0;
	diagDmaSeqErrCnt = 0;
	diagDmaBlockWrCnt = 0;
	diagDmaBlockWrErrCnt = 0;
	diagDmaBlockWrBusy = 0;

	diagStartBlock = (void*)diagBufUC;
	diagCurrBlock = diagStartBlock;
	diagWriteBlock = diagStartBlock;
	diagEyeBlock = diagStartBlock;
	diagEndBlock  = (void *) (((char*) diagCurrBlock) + diagTotalBufNum*DIAG_DMA_BLK_SIZE);

	pAdslEnum[ADSL_INTMASK_F] = 0;
	pAdslEnum[RCV_PTR_FAST] = 0;
	pAdslEnum[RCV_CTL_FAST] = 0;
	
	pAdslEnum[RCV_ADDR_FAST] = (ulong) diagDescTbl & 0x1FFFFFFF;
	pAdslEnum[RCV_CTL_FAST] = 1 | ((FLD_OFFSET(diagDmaBlock, dataFrame.diagData) - FLD_OFFSET(diagDmaBlock, len)) << 1);
	pAdslEnum[RCV_PTR_FAST] = diagTotalBufNum << 3;
	pAdslEnum[ADSL_INTMASK_F] |= DMA_INTR_MASK;
#endif	
}

void BcmAdslCoreDiagDmaInit(void)
{
	if (0 == diagDataMap) {
		diagDataMap = DIAG_DATA_EYE;
		diagLogTime = 0;
		BcmAdslCoreDiagDataInit();
	}
	else
		__BcmAdslCoreDiagDmaInit();
}

void BcmAdslCoreDiagStartLog(ulong map, ulong time)
{
	dslCommandStruct	cmd;

	diagDataMap = 0;
	if (map & kDslDiagEnableEyeData)
		diagDataMap |= DIAG_DATA_EYE;
	if (map & (kDslDiagEnableLogData | kDslDiagEnableDebugData)) {
		diagDataMap |= DIAG_DATA_LOG;
		diagDmaLogBlockNum = 0;
		BcmAdslCoreDiagWriteLog(inputSignalData - 2, AC_TRUE);
		diagDmaLogBlockNum = 0;
		BcmDiagDataLogNotify(1);
	}
	else
		BcmDiagDataLogNotify(0);
	
	diagLogTime = time;
	BcmAdslCoreDiagDataInit();
#ifndef CONFIG_BCM96368
	printk("%s: map=0x%X, time=%d, diagTotalBufNum=%d\n", __FUNCTION__, (int)map, (int)time, (int)diagTotalBufNum);
#else
	printk("%s: map=0x%X, time=%d\n", __FUNCTION__, (int)map, (int)time);
#endif

	cmd.command = kDslDiagSetupCmd;
	cmd.param.dslDiagSpec.setup = map;
	cmd.param.dslDiagSpec.eyeConstIndex1 = 0; 
	cmd.param.dslDiagSpec.eyeConstIndex2 = 0;
	cmd.param.dslDiagSpec.logTime = time;
	BcmCoreCommandHandlerSync(&cmd);
}

#ifndef CONFIG_BCM96368
LOCAL void BcmAdslCoreDiagDmaUninit(void)
{
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;

	pAdslEnum[ADSL_INTMASK_F] = 0;
	pAdslEnum[RCV_PTR_FAST] = 0;
	pAdslEnum[RCV_CTL_FAST] = 0;

	BcmAdslCoreDiagDataFlush();
}

void * BcmAdslCoreDiagGetDmaDataAddr(int descNum)
{
	diagDmaBlock	*diagDmaPtr;

	diagDmaPtr = (void *) ((diagDescTblUC[descNum].addr - FLD_OFFSET(diagDmaBlock, len)) | 0xA0000000);
	return &diagDmaPtr->dataFrame.diagData;
}

int BcmAdslCoreDiagGetDmaDataSize(int descNum)
{
	diagDmaBlock	*diagDmaPtr;

	diagDmaPtr = (void *) ((diagDescTblUC[descNum].addr - FLD_OFFSET(diagDmaBlock, len)) | 0xA0000000);
	return diagDmaPtr->len;
}

int	 BcmAdslCoreDiagGetDmaBlockNum(void)
{
	return diagTotalBufNum;
}


ulong  BcmAdslCoreDiagBufRequired(ulong map)
{
	ulong	nBuf = 0;

	if (map & DIAG_DATA_EYE)
		nBuf = 28;
	if (map & DIAG_DATA_LOG)
#if (LINUX_FW_VERSION >= 307)
		nBuf= 256;
#else
		nBuf= 28;
#endif
	
	return nBuf;
}

void *BcmAdslCoreDiagAllocDmaMem(ulong *pBufNum)
{
	void	*pTbl;
	int		nBuf = *pBufNum;

	pTbl = (void *) kmalloc(nBuf * DIAG_DMA_BLK_SIZE + (DIAG_DESC_TBL_ALIGN_SIZE + DIAG_DESC_TBL_SIZE(nBuf)), GFP_KERNEL|__GFP_NOWARN);

	while ((NULL == pTbl) && (nBuf > 0)) {
		nBuf--;
		pTbl = (void *) kmalloc(nBuf * DIAG_DMA_BLK_SIZE + (DIAG_DESC_TBL_ALIGN_SIZE + DIAG_DESC_TBL_SIZE(nBuf)), GFP_KERNEL|__GFP_NOWARN);
	}
	diagDescMemStart = pTbl;
	if (NULL == pTbl) {
		*pBufNum = 0;
		return NULL;
	}
	pTbl = (void *) (((ulong) pTbl + 0xFFF) & ~0xFFF);
	if (diagDescMemStart == pTbl)
		nBuf += 2;

	diagDescMemSize  = nBuf * DIAG_DMA_BLK_SIZE + DIAG_DESC_TBL_SIZE(nBuf);

	printk ("diagDmaBlkSz=%d, diagTotalBuf=%d, diagDescTbl=0x%X(0x%X), size=%d\n", 
		sizeof(diagDmaBlock), (int)nBuf, (int) pTbl, (int) diagDescMemStart, (int) diagDescMemSize);

	*pBufNum = nBuf;
	return pTbl;
}
#endif / * !CONFIG_BCM96368 */

void BcmAdslCoreDiagDataInit(void)
{
#ifndef CONFIG_BCM96368
	int					i;
	ushort				ipId;
	struct sk_buff		*skb;

	if (0 == diagDataMap)
		return;

	if (NULL == diagDescTbl) {
		diagTotalBufNum = BcmAdslCoreDiagBufRequired(diagDataMap);
		diagDescTbl = BcmAdslCoreDiagAllocDmaMem(&diagTotalBufNum);
		if (NULL == diagDescTbl) {
			diagDataMap = 0;
			return;
		}
	}
	else {
		ulong	nReq;

		BcmAdslCoreDiagDmaUninit();
		nReq = BcmAdslCoreDiagBufRequired(diagDataMap);
		if (nReq > diagTotalBufNum) {
			kfree(diagDescMemStart);
			diagTotalBufNum = nReq;
			diagDescTbl = BcmAdslCoreDiagAllocDmaMem(&diagTotalBufNum);
		}
	}

	diagDescTblUC = UNCACHED(diagDescTbl);
	diagBuf = (uchar *)diagDescTbl + DIAG_DESC_TBL_SIZE(diagTotalBufNum);
	diagBufUC = UNCACHED(diagBuf);
	diagCurrBlock = (void*) diagBuf;

	for (i = 0; i < diagTotalBufNum; i++) {
		diagDescTblUC[i].flags = DIAG_DMA_BLK_SIZE - FLD_OFFSET(diagDmaBlock, len) - sizeof(struct skb_shared_info);
		diagDescTblUC[i].addr  = ((ulong)diagBuf & 0x1FFFFFFF) + FLD_OFFSET(diagDmaBlock, len);

		diagCurrBlock = (void*) diagBuf;
		skb = &diagCurrBlock->skb;
		memset(skb, 0, sizeof(*skb));

		/* memcpy(skb, skbModel, sizeof(struct sk_buff)); */

		skb->head = (void *)&diagCurrBlock->dataFrame;
		skb->data = (void *)&diagCurrBlock->dataFrame.eth;
		skb->tail = skb->data;
		skb->end  = (void *)&diagCurrBlock->skbShareInfo;
		skb->len = 0;
		skb->data_len = 0;

		skb->truesize = (skb->end - skb->head) + sizeof(struct sk_buff);
		skb->cloned = 0;
		atomic_set(&skb->users, DIAG_SKB_USERS); 
		atomic_set(&(skb_shinfo(skb)->dataref), 1);
		skb_shinfo(skb)->nr_frags = 0;
		skb_shinfo(skb)->frag_list = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		skb_shinfo(skb)->tso_size = 0;
		skb_shinfo(skb)->tso_segs = 0;
#endif
		skb->dev = dbgDev;
		if (NULL != dbgDev)
			skb->protocol = eth_type_trans (skb, skb->dev);
		skb->ip_summed = CHECKSUM_NONE;

		if (NULL != skbModel)
			memcpy(skb->data, skbModel->data, DIAG_FRAME_HEADER_LEN);
		ipId = diagCurrBlock->dataFrame.ipHdr.id;
		diagCurrBlock->dataFrame.ipHdr.id = 0x4000 + i;
		diagCurrBlock->dataFrame.ipHdr.checksum = DiagIpUpdateChecksum(
												diagCurrBlock->dataFrame.ipHdr.checksum,
												ipId,
												diagCurrBlock->dataFrame.ipHdr.id);
		
		diagCurrBlock->len = 0;
		diagCurrBlock->frameNum = 0;
		diagCurrBlock->mark   = 0;
		diagCurrBlock->dataFrame.pad[0] = 0;

#if 0
		diagCurrBlock->diagHdrDma.logProtoId[0] = '*';
		diagCurrBlock->diagHdrDma.logProtoId[1] = 'L';
		diagCurrBlock->diagHdrDma.logPartyId = 1;
#if 0
		diagCurrBlock->dataFrame.eth.h_proto = 0x2A4C;
		diagCurrBlock->diagHdrDma.logCommmand = 0xF;
		diagCurrBlock->dataFrame.diagData[0] = 0;
		diagCurrBlock->dataFrame.diagData[1] = 0;
		diagCurrBlock->dataFrame.diagData[2] = (i >> 8) & 0xFF;
		diagCurrBlock->dataFrame.diagData[3] = i & 0xFF;
#else
		diagCurrBlock->diagHdrDma.logCommmand = LOG_CMD_STRING_DATA;
		diagCurrBlock->dataFrame.diagData[0] = 'A' + (i & 0xFF);
		diagCurrBlock->dataFrame.diagData[1] = 'B' + (i & 0xFF);
		diagCurrBlock->dataFrame.diagData[2] = 'C' + (i & 0xFF);
		diagCurrBlock->dataFrame.diagData[3] = 0;
#endif
		DiagUpdateSkbForDmaBlock(diagCurrBlock, 4);
		DiagWriteDmaBlock (diagCurrBlock);
		/* DiagPrintData(skb); */

		printk("diagBuf=%p, diagDescTblUC[%d].addr=%08x, dataFrame.diagData=%p, skb->head=%p, skb->data=%p, skb->end=%p\n",
				diagBuf,
				i,
				diagDescTblUC[i].addr,
				diagCurrBlock->dataFrame.diagData,
				skb->head,
				skb->data,
				skb->end);
#endif

		diagBuf		+= DIAG_DMA_BLK_SIZE;
		diagBufUC	+= DIAG_DMA_BLK_SIZE;
		
	}
	diagDescTblUC[diagTotalBufNum-1].flags |= 0x10000000;

	diagBuf = (uchar *)diagDescTbl + DIAG_DESC_TBL_SIZE(diagTotalBufNum);
	diagBufUC = UNCACHED(diagBuf);

	__BcmAdslCoreDiagDmaInit();
#endif	
	return;
}

#ifndef CONFIG_BCM96368
int BcmAdslCoreDiagWriteBlocks(void)
{
	diagDmaBlock	*diagPtr;
	int			reschedule = 0;

	ASSERT(NULL != dbgDev);

	while (diagWriteBlock != diagCurrBlock) {
		diagPtr = CACHED(diagWriteBlock);
		if ((diagPtr->dataFrame.pad[0] != 0) && (DIAG_SKB_USERS == atomic_read(&diagPtr->skb.users))) {
#ifdef DIAG_DBG
			diagSkipWrDmaBlkCnt++;
#endif
			reschedule = 1;
			break;
		}
		
		diagPtr->dataFrame.pad[0] = 0;
		atomic_set(&diagPtr->skb.users, DIAG_SKB_USERS);
		diagPtr->skb.dev = dbgDev;
		
		reschedule = dev_queue_xmit(&diagPtr->skb);
		
		if ( 0 != reschedule) {
			diagDmaBlockWrErrCnt++;
#ifdef DIAG_DBG
			printk("%s: Error=%d, packetID=0x%X\n", __FUNCTION__, n, diagPtr->dataFrame.ipHdr.id);
#endif
			break;
		}
		
		diagPtr->dataFrame.pad[0] = 1;
		diagDmaBlockWrCnt++;
		
		if (diagWriteBlock->diagHdrDma.logCommmand != kDslLogComplete)
			diagDmaTotalDataLen += diagWriteBlock->len;
		else
			BcmDiagDataLogNotify(0);
		
		diagWriteBlock = (void *) (((char*) diagWriteBlock) + DIAG_DMA_BLK_SIZE);
		if (diagWriteBlock == diagEndBlock)
			diagWriteBlock = diagStartBlock;

	}

	return reschedule;
}

LOCAL int BcmCoreDiagDpc(void * arg)
{
	int		descIndex;
	int		res;
	int 		loopCnt = 0;	
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	diagDmaBlock	*diagDmaPtr, *diagPtr;

	pAdslEnum[ADSL_INTSTATUS_F] = pAdslEnum[ADSL_INTSTATUS_F];

	descIndex =  (pAdslEnum[RCV_STATUS_FAST] & 0xFFF) >> 3;
	diagDmaPtr = (diagDmaBlock*) ((diagDescTblUC[descIndex].addr - FLD_OFFSET(diagDmaBlock, len)) | 0xA0000000);
	
	while (diagCurrBlock != diagDmaPtr) {
		diagDmaBlockCnt++;
		diagPtr = CACHED(diagCurrBlock);
		if ((diagPtr->dataFrame.pad[0] != 0) && (DIAG_SKB_USERS == atomic_read(&diagPtr->skb.users)))
			diagDmaOvrCnt++;

		if (diagCurrBlock->frameNum != diagDmaSeqBlockNum) {
#ifdef DIAG_DBG
			BcmAdslCoreDiagWriteStatusString ("blkNum=%ld, diagP=%X, dmaP=0x%X, len=%ld,anum=%ld,mark=0x%lX,lHdr=0x%lX,enum=%ld\n", 
				diagDmaBlockCnt, (int)diagCurrBlock,
				(int)diagDmaPtr, diagCurrBlock->len,
				diagCurrBlock->frameNum, diagCurrBlock->mark,
				*(ulong*)&diagCurrBlock->diagHdrDma,diagDmaSeqBlockNum);
#endif
			diagDmaSeqErrCnt++;
			diagDmaSeqBlockNum = diagCurrBlock->frameNum;
		}
		
		diagDmaSeqBlockNum++;

		/* update log frame number */
		if (diagCurrBlock->diagHdrDma.logCommmand != eyeData)
			*(ushort *)diagCurrBlock->diagHdrDma.logProtoId = (ushort) diagDmaLogBlockNum++;		
		
		/* update skb for this DMA block */
		diagPtr->diagHdrDma = diagCurrBlock->diagHdrDma;
		DiagUpdateSkbForDmaBlock(diagPtr, diagCurrBlock->len);		

		diagCurrBlock->mark = 0;
		diagCurrBlock = (void *) (((char*) diagCurrBlock) + DIAG_DMA_BLK_SIZE);
		
		if (diagCurrBlock == diagEndBlock)
			diagCurrBlock = diagStartBlock;

		if (diagCurrBlock == diagWriteBlock)
			diagDmaOvrCnt++;

		res = BcmAdslCoreDiagWriteBlocks();

		loopCnt++;

		descIndex =  (pAdslEnum[RCV_STATUS_FAST] & 0xFFF) >> 3;
		diagDmaPtr = (diagDmaBlock*) ((diagDescTblUC[descIndex].addr - FLD_OFFSET(diagDmaBlock, len)) | 0xA0000000);
	}

	res = BcmAdslCoreDiagWriteBlocks();

	if( (0 != res) && (NULL != diagDpcId) )
		bcmOsDpcEnqueue(diagDpcId);
	else
		PHY_INTR_ENABLE(pAdslEnum);

	if(loopCnt > diagMaxLpPerSrvCnt)
		diagMaxLpPerSrvCnt=loopCnt;

	return 0;
}
#endif /* #ifndef CONFIG_BCM96368 */

Bool BcmAdslCoreDiagIntrHandler(void)
{
#ifndef CONFIG_BCM96368
	ulong		intStatus;
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;

	PHY_INTR_DISABLE(pAdslEnum);
	
	intStatus = pAdslEnum[ADSL_INTSTATUS_F];
	pAdslEnum[ADSL_INTSTATUS_F] = intStatus;

	if ((0 == diagDataMap) || (diagEnableCnt <= 0)) {
		PHY_INTR_ENABLE(pAdslEnum);
		return 0;
	}
	
	if (intStatus & DMA_INTR_MASK) {
		diagDmaIntrCnt++;
		
		if(intStatus & DMA_ERROR_MASK)
			diagDmaErrorCnt++;
		
		if(intStatus & ADSL_INT_RCV) {
#ifdef DIAG_DBG
			diagRxIntrCnt++;
#endif
			if( NULL != diagDpcId ) {
				bcmOsDpcEnqueue(diagDpcId);
				return 0;
			}
		}
	}
	
	PHY_INTR_ENABLE(pAdslEnum);
#endif /* #ifndef CONFIG_BCM96368 */	
	return 0;
}


void BcmAdslCoreDiagIsrTask(void)
{
}

int BcmAdslDiagGetConstellationPoints (int toneId, void *pointBuf, int numPoints)
{
#ifndef CONFIG_BCM96368
	volatile ulong	*pAdslEnum = (ulong *) ADSL_ENUM_BASE;
	diagDmaBlock	*diagDmaBlockPtr;
	ulong			*pSrc, *pDst;
	int				i;
#endif

	if (0 == diagDataMap) {
		diagDataMap = DIAG_DATA_EYE;
		diagLogTime = 0;
		BcmAdslCoreDiagDataInit();
		BcmAdslCoreDiagCommand();
		return 0;
	}
	
#ifndef CONFIG_BCM96368
	diagDmaBlockPtr = (diagDmaBlock*) ((diagDescTblUC[ (pAdslEnum[RCV_STATUS_FAST] & 0xFFF) >> 3].addr - FLD_OFFSET(diagDmaBlock, len)) | 0xA0000000);
	while (diagEyeBlock != diagDmaBlockPtr) {
		if (eyeData == diagEyeBlock->diagHdrDma.logCommmand)
			break;

		diagEyeBlock = (void *) (((char*) diagEyeBlock) + DIAG_DMA_BLK_SIZE);
		if (diagEyeBlock == diagEndBlock)
			diagEyeBlock = diagStartBlock;
	}
	if (diagEyeBlock == diagDmaBlockPtr)
		return 0;

	if (numPoints > (diagEyeBlock->len >> 3))
		numPoints = diagEyeBlock->len >> 3;

	pSrc = ((ulong *)diagEyeBlock->dataFrame.diagData) + (toneId != 0 ? 1 : 0);
	pDst = (ulong *) pointBuf;
	for (i = 0; i < numPoints; i++) {
		*pDst++ = *pSrc;
		pSrc += 2;
	}

	diagEyeBlock = (void *) (((char*) diagEyeBlock) + DIAG_DMA_BLK_SIZE);	
	if (diagEyeBlock == diagEndBlock)
		diagEyeBlock = diagStartBlock;
#endif	
	return numPoints;
}

#ifdef PHY_PROFILE_SUPPORT
extern void BcmAdslCoreProfilingStop(void);
#endif

void BcmAdslCoreDiagCmd(PADSL_DIAG pAdslDiag)
{
	ulong				origDiagMap;
	int					dstPort;
	dstPort = ((ulong) pAdslDiag->diagMap) >> 16;
	switch (pAdslDiag->diagCmd) {
		case LOG_CMD_CONNECT:
			origDiagMap = diagDataMap;
				dstPort = ((ulong) pAdslDiag->diagMap) >> 16;
			printk("%s: CONNECT dstPort=%d\n", __FUNCTION__, dstPort);
			if (pAdslDiag->diagMap & DIAG_DATA_GUI_ID)
				dbgDev = BcmAdslCoreGuiInit(pAdslDiag);
			else	
				dbgDev = BcmAdslCoreDiagInit(pAdslDiag);
			if (adslCoreInitialized && (NULL != dbgDev)) {
				adslVersionInfo		verInfo;

				if (diagDataMap & DIAG_DATA_LOG) {
					BcmAdslCoreReset();
					BcmDiagDataLogNotify(1);
				}
				else {
					BcmAdslCoreDiagDataInit();
					if (0 == origDiagMap) {
						BcmAdslCoreDiagCommand();
					}
				}

				BcmAdslCoreGetVersion(&verInfo);
				BcmAdslCoreDiagWriteStatusString("ADSL version info: PHY=%s, Drv=%s. Built on "__DATE__" " __TIME__,
					verInfo.phyVerStr, verInfo.drvVerStr);
			}
			break;

		case LOG_CMD_DISCONNECT:
			printk ("%s: DISCONNECT dstPort=%d\n", __FUNCTION__, dstPort);
			//dbgDev = NULL;
			diagDataMap = 0;
			BcmDiagDataLogNotify(0);
			BcmAdslCoreDiagCommand();
#ifdef PHY_PROFILE_SUPPORT
			BcmAdslCoreProfilingStop();
#endif
			break;

		case LOG_CMD_DEBUG:
			{
			DiagDebugData	*pDbgCmd = (void *)pAdslDiag->diagMap;
			switch (pDbgCmd->cmd) {
				case DIAG_DEBUG_CMD_CLEAR_STAT:
					diagDmaIntrCnt	= 0;
					diagDmaBlockCnt = 0;
					diagDmaOvrCnt = 0;
					diagDmaSeqErrCnt = 0;
					diagDmaBlockWrCnt = 0;
					diagDmaBlockWrErrCnt = 0;
					diagDmaBlockWrBusy = 0;
					break;
				case DIAG_DEBUG_CMD_PRINT_STAT:
					BcmAdslCoreDiagWriteStatusString(
						"DiagLinux Statistics:\n"
						"   diagIntrCnt		= %d\n"
						"   dmaBlockCnt	= %d\n"
						"   dmaSeqNum	= %d\n"
						"   ethWrCnt		= %d\n"						
						"   dmaOvrCnt		= %d\n"
						"   dmaSqErrCnt	= %d\n"
						"   ethErrWrCnt	= %d\n"
						"   ethBusyCnt	= %d\n"
#ifdef DIAG_DBG
						"   dmaRxIntrCnt		= %d\n"
						"   dmaSkipWrBlkCnt	= %d\n"
#endif
						"   dmaMaxLpPerSrvCnt	= %d\n"
						"   dmaErrorCnt		= %d\n"
						"   dmaTotalDataLen	= %lu\n",
						diagDmaIntrCnt,
						diagDmaBlockCnt,
						diagDmaSeqBlockNum,
						diagDmaBlockWrCnt,						
						diagDmaOvrCnt,
						diagDmaSeqErrCnt,
						diagDmaBlockWrErrCnt,
						diagDmaBlockWrBusy,
#ifdef DIAG_DBG
						diagRxIntrCnt,
						diagSkipWrDmaBlkCnt,
#endif
						diagMaxLpPerSrvCnt,
						diagDmaErrorCnt,
						diagDmaTotalDataLen);
					break;
			}
			BcmAdslCoreDebugCmd(pDbgCmd);
			}
			break;

		default:
			BcmAdslCoreDiagCmdCommon(pAdslDiag->diagCmd, pAdslDiag->logTime, (void*) pAdslDiag->diagMap);
			break;
	}
}

/***************************************************************************
** Function Name: BcmAdslDiagDisable/BcmAdslDiagEnable
** Description  : This function enables/disables diag interrupt processing
** Returns      : None.
***************************************************************************/
int BcmAdslDiagDisable(void)
{
	diagEnableCnt--;
	return diagEnableCnt;
}

int BcmAdslDiagEnable(void)
{
	diagEnableCnt++;
	return diagEnableCnt;
}

Bool BcmAdslDiagIsActive(void)
{
	return (NULL != dbgDev);
}

/***************************************************************************
** Function Name: BcmAdslDiagReset
** Description  : This function resets diag support after ADSL MIPS reset
** Returns      : None.
***************************************************************************/
void BcmAdslDiagReset(int map)
{
	BcmAdslDiagDisable();
	diagDataMap &= map & (DIAG_DATA_LOG | DIAG_DATA_EYE);
	if (diagDataMap & (DIAG_DATA_LOG | DIAG_DATA_EYE)) {
		diagDmaSeqBlockNum = 0;
		BcmAdslCoreDiagDataInit();
	}
	BcmAdslDiagEnable();
	if (diagDataMap & (DIAG_DATA_LOG | DIAG_DATA_EYE))
		BcmAdslCoreDiagCommand();
}

/***************************************************************************
** Function Name: BcmAdslDiagInit
** Description  : This function intializes diag support on Host and ADSL MIPS
** Returns      : None.
***************************************************************************/
int BcmAdslDiagInit(int map)
{
#ifndef CONFIG_BCM96368
	diagDpcId = bcmOsDpcCreate(BcmCoreDiagDpc, NULL);
#endif
	BcmAdslCoreDiagDataInit();
	BcmAdslCoreDiagCommand();
	return 0;
}



/**************************************************************************
** Function Name: BcmAdslDiagTaskInit
** Description  : This function intializes ADSL driver Diag task
** Returns      : None.
**************************************************************************/
int BcmAdslDiagTaskInit(void)
{
	return 0;
}

/**************************************************************************
** Function Name: BcmAdslDiagTaskUninit
** Description  : This function unintializes ADSL driver Diag task
** Returns      : None.
**************************************************************************/
void BcmAdslDiagTaskUninit(void)
{
}

