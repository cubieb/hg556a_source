/*
<:copyright-broadcom 
 
 Copyright (c) 2003 Broadcom Corporation 
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
/**************************************************************************
 * File Name  : bcmusb.c
 *
 * Description: This file contains the implementation for a USB
 *              network interface driver that executes on the BCM963xx
 *              reference platforms.  It requires one of the following host
 *              drivers.
 *
 *              * Remote NDIS - Used on Windows with a Microsoft RNDIS host
 *                driver.
 *
 *              * USB Class Definitions for Communications Devices 1.1 
 *                (CDC 1.1) specification, Ethernet Control Model - Used
 *                on Windows with a Broadcom supplied host driver, Linux PCs
 *                and Macintosh.
 *
 * Updates    : 02/19/2002  lat.   Created for Linux.
 *              11/04/2003         Changes for BCM6348.
 ***************************************************************************/


/* Defines. */
#define CARDNAME    "BCM63XX_USB"
#define VERSION     "0.4"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__


/* Includes. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/delay.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>

#include <asm/uaccess.h>

#include <bcmnet.h>
#include <bcmtypes.h>
#include <bcm_map.h>
#include <bcm_common.h>
#include <bcm_intr.h>
#include <board.h>
#include "rndis.h"
#include "bcmusb.h"

#if !defined(LOCAL)
#define LOCAL static
#endif

/* Undefine LOCAL for debugging. */
#if defined(LOCAL)
#undef LOCAL
#define LOCAL
#endif

/* Globals. */
PUSBNIC_DEV_CONTEXT g_pDevCtx = NULL;


/* Externs. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
extern irqreturn_t RndisCtrlRxIsr(int irq, void *dev_id, struct pt_regs *regs);
#else
extern void RndisCtrlRxIsr( int irq, void *dev_id, struct pt_regs *regs );
#endif
extern void RndisDoGetEncapsultedResponse( PUSBNIC_DEV_CONTEXT pDevCtx,
    UINT16 usLength );
extern void RndisProcessMsg( PUSBNIC_DEV_CONTEXT pDevCtx );


/* Prototypes. */
#if defined(MODULE)
LOCAL
#endif
int __init bcmusbnic_probe( void );
LOCAL int UsbNicInit( PUSBNIC_DEV_CONTEXT pDevCtx );
LOCAL void SetMacAddr( char *pStrBuf, UINT8 *pucMacAddr );
LOCAL void UsbNicReset( UsbConfiguration *pUsbCfg, UINT32 ulUsbCfgSize );
LOCAL void UsbNicEnable( volatile DmaRegs *pDmaRegs,
    volatile DmaChannelCfg *pRxDma, volatile DmaChannelCfg *pTxDma,
    volatile DmaChannelCfg *pRxCtrlDma, unsigned int ulParam );
LOCAL void bcmusbnic_cleanup(void);
LOCAL void UsbNicDisable( volatile DmaChannelCfg *pRxDma,
    volatile DmaChannelCfg *pTxDma );
LOCAL int bcmusbnic_open( struct net_device *dev );
LOCAL int bcmusbnic_close(struct net_device *dev);
LOCAL void bcmusbnic_timeout(struct net_device *dev);
LOCAL struct net_device_stats *bcmusbnic_query(struct net_device *dev);
LOCAL int bcmusbnic_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
LOCAL int bcmusbnic_xmit(struct sk_buff *skb, struct net_device *dev);
LOCAL void usbnic_timer( PUSBNIC_DEV_CONTEXT pDevCtx );
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
LOCAL irqreturn_t bcmBulkRxIsr(int irq, void *dev_id, struct pt_regs *regs);
LOCAL irqreturn_t bcmUsbIsr(int irq, void *dev_id, struct pt_regs *regs);
#else
LOCAL void bcmBulkRxIsr(int irq, void * dev_id, struct pt_regs * regs);
LOCAL void bcmUsbIsr(int irq, void *dev_id, struct pt_regs *regs);
#endif
LOCAL void usbnic_bulkrx( PUSBNIC_DEV_CONTEXT pDevCtx );
LOCAL void UsbNicProcessSetupCommand(PUSBNIC_DEV_CONTEXT pDevCtx);
LOCAL void ConvertAnsiToUnicode( char *pszOut, char *pszIn );
void UsbNicSubmitControlPacket( PUSBNIC_DEV_CONTEXT pDevCtx, UINT8 *pBuf,
    UINT32 ulLength );
UINT32 NotifyHost( const UINT32 *pulData );
/* Ethtool support */
LOCAL int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr);

/*******************************************************************************
*
* bcmusbnic_probe
*
* Initial driver entry point.
*
* RETURNS: 0 - success, < 0 - error
*/

#if defined(MODULE)
LOCAL
#endif
int __init bcmusbnic_probe( void )
{
    /* string descriptors (by index), index 0 = LANGID */
    StringTable StrTbl[STRING_TABLE_SIZE] =
        {
        {{0x09, 0x04, 0}},     /* LANGID - English = 0x0409 (or 0x0109 (old)) */
        {"Broadcom Corporation"},
        {"USB Network Interface"},
        {"001018000000"},  /* serial number */
        {"USB Ethernet Configuration"},
        {"Communication Interface Class"},
        {"Data Interface Class"}
        };

    UsbConfiguration UsbCfg =
        {
        /* DeviceDesc */
        {0x12, 0x01, 0x1001, 0x02, 0x00, 0x00, 0x20, 0x5c0a, 0x0063,
         0x0101, 0x01, 0x02, 0x03, 0x02},

        /**** Remote NDIS Configuration ****/

        /* ConfigurationDesc */
        {0x09, 0x02, 0x43, 0x00, 0x02, HOST_DRIVER_RNDIS, 0x04, 0xc0, 0x00},

        /* Communication Class Interface descriptor */
        {0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0xff, 0x00},

        /* Communication Class (ethernet specific) descriptor */
        {0x05, 0x24, 0x00, 0x10, 0x01,
         0x05, 0x24, 0x01, 0x00, 0x00,
         0x04, 0x24, 0x02, 0x00,
         0x05, 0x24, 0x06, 0x00, 0x01},

        /* Endpoint Descriptor (Notification) */
        {0x07, 0x05, 0x85, 0x03, 0x08, 0x00, 0x64 },

        /* Data Class Interface Descriptor */
        {0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x06},

        /* Endpoint Descriptor (Endpoint 1 IN: Bulk Data from Modem) */
        {0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00},

        /* Endpoint Descriptor (Endpoint 2 OUT: Bulk Data to Modem) */
        {0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00},

        /**** Ethernet Control Model Configuration (CDC 1.10) ****/

        /* ConfigurationDesc */
        {0x09, 0x02, 0x50, 0x00, 0x02, HOST_DRIVER_CDC, 0x04, 0xc0, 0x00},

        /* Communication Class Interface descriptor */
        {0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x06, 0x00, 0x05},

        /* Communication Class (ethernet specific) descriptor */
        {0x05, 0x24, 0x00, 0x10, 0x01,
         0x05, 0x24, 0x06, 0x00, 0x01,
         0x0d, 0x24, 0x0f, 0x03, 0x00, 0x00,0x00,0x00,0xea,0x05,0x00,0x00,0x00},

        /* Endpoint Descriptor (Notification) */
        {0x07, 0x05, 0x85, 0x03, 0x08, 0x00, 0x40},

        /* Data Class Interface Descriptor, Intf 0, Alt 0 */
        {0x09, 0x04, 0x01, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x06},

        /* Data Class Interface Descriptor, Intf 0, Alt 1 */
        {0x09, 0x04, 0x01, 0x01, 0x02, 0x0a, 0x00, 0x00, 0x06},

        /* Endpoint Descriptor (Endpoint 1 IN: Bulk Data from Modem) */
        {0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00},

        /* Endpoint Descriptor (Endpoint 2 OUT: Bulk Data to Modem) */
        {0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00}
        };

    static int nProbed = 0;
    int nStatus = 0;

    if( nProbed == 0 )
    {
        PUSBNIC_DEV_CONTEXT pDevCtx = NULL;
        struct net_device *dev = NULL;
        UINT16 usChipId;
        UINT16 usChipRev;
        UINT8 *p;

        nProbed = 1;

        usChipId  = (PERF->RevID & 0xFFFF0000) >> 16;
        usChipRev = (PERF->RevID & 0xFF);

        if( nStatus == 0 )
        {
            /* Allocate device context structure */
            p = (UINT8 *) kmalloc( sizeof(USBNIC_DEV_CONTEXT) +
                sizeof(StrTbl) + sizeof(UsbConfiguration) + 32, GFP_KERNEL );

            if( p == NULL )
                nStatus = -ENOMEM;
        }

        if( nStatus == 0 )
        {
            g_pDevCtx = pDevCtx = (PUSBNIC_DEV_CONTEXT) p;
            memset( pDevCtx, 0x00, sizeof(USBNIC_DEV_CONTEXT) );
            pDevCtx->pStrTbl = (StringTable *)
                (((UINT32) p + sizeof(USBNIC_DEV_CONTEXT) + 0x0f) & ~0x0f);
            pDevCtx->pUsbCfg = (UsbConfiguration *) (((UINT32) p +
                sizeof(USBNIC_DEV_CONTEXT) + sizeof(StrTbl) + 0x0f) & ~0x0f);
            memcpy( pDevCtx->pStrTbl, StrTbl, sizeof(StrTbl) );
            memcpy(pDevCtx->pUsbCfg, &UsbCfg, sizeof(UsbCfg));
            pDevCtx->ulUsbCfgSize = sizeof(UsbCfg);
            pDevCtx->usPacketFilterBitmap = NDIS_PACKET_TYPE_MULTICAST |
                NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED |
                NDIS_PACKET_TYPE_ALL_MULTICAST | NDIS_PACKET_TYPE_PROMISCUOUS;
            pDevCtx->ulFlags = 0;
            pDevCtx->ulHostDriverType = HOST_DRIVER_CDC; /* default value */

            /* Print the chip id and module version. */
            printk("Broadcom BCM%X%X USB Network Device ", usChipId, usChipRev);
            printk(VER_STR "\n");

            /* Initialize a timer that calls a function to free transmit
             * buffers.
             */
            init_timer(&pDevCtx->ReclaimTimer);
            pDevCtx->ReclaimTimer.data = (unsigned long)pDevCtx;
            pDevCtx->ReclaimTimer.function = (void *) usbnic_timer;

            /* Initialize bottom half for handling data received on the bulk
             * endpoint and RNDIS messages.
             */
#ifdef USE_BH
            tasklet_init(&(pDevCtx->BhBulkRx), (void *) usbnic_bulkrx, (unsigned long) pDevCtx);
            tasklet_init(&(pDevCtx->BhRndisMsg), (void *) RndisProcessMsg, (unsigned long) pDevCtx);
#endif

            /* Initialize this device as a network device. */    
            if ((dev = alloc_netdev(sizeof(*pDevCtx), "usb%d", ether_setup)) != NULL) {
                nStatus = 0;
            } else {
                printk(KERN_ERR CARDNAME": alloc_netdev failed\n");
                nStatus = -ENOMEM;
            }
        }

        if( nStatus == 0 )
        {
            dev_alloc_name(dev, dev->name);
            SET_MODULE_OWNER(dev);

            dev->priv = pDevCtx;
            pDevCtx->pDev = dev;

            /* Read and display the USB network device MAC address. */
            dev->dev_addr[0] = 0xff;
            kerSysGetMacAddress( dev->dev_addr, dev->ifindex + 50 );
            if( (dev->dev_addr[0] & 0x01) == 0x01 )
            {
                printk( KERN_ERR CARDNAME": Unable to read MAC address from "
                    "persistent storage.  Using default address.\n" );
                memcpy( dev->dev_addr, "\x40\x10\x18\x02\x00\x01", 6 );
            }

            printk("%s: MAC Address: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                dev->name, dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
                dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5] );

            kerSysGetMacAddress( pDevCtx->ucPermanentHostMacAddr,
                dev->ifindex + 100 );

            if( (pDevCtx->ucPermanentHostMacAddr[0] & 0x01) == 0x01 )
            {
                printk( KERN_ERR CARDNAME": Unable to read host MAC address "
                    "from persistent storage.  Using default address.\n" );
                memcpy( pDevCtx->ucPermanentHostMacAddr,
                    "\x40\x10\x18\x82\x00\x01", ETH_ALEN );
            }

            memcpy( pDevCtx->ucCurrentHostMacAddr,
                pDevCtx->ucPermanentHostMacAddr, ETH_ALEN );

            printk("%s: Host MAC Address: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
                dev->name, pDevCtx->ucPermanentHostMacAddr[0],
                pDevCtx->ucPermanentHostMacAddr[1],
                pDevCtx->ucPermanentHostMacAddr[2],
                pDevCtx->ucPermanentHostMacAddr[3],
                pDevCtx->ucPermanentHostMacAddr[4],
                pDevCtx->ucPermanentHostMacAddr[5] );

            /* Initialize buffer descriptors. */
            nStatus = UsbNicInit( pDevCtx );

#if 0 /* DEBUG */
            printk("USBDBG: pDevCtx=0x%8.8lx\n", (UINT32) pDevCtx );
            printk("USBDBG: pRxBdBase=0x%8.8lx\n", (UINT32) pDevCtx->pRxBdBase);
            printk("USBDBG: pTxBdBase=0x%8.8lx\n", (UINT32) pDevCtx->pTxBdBase);
            printk("USBDBG: pRxCtrlBdBase=0x%8.8lx\n", (UINT32) pDevCtx->pRxCtrlBdBase);
            printk("USBDBG: pCtrlPktsBase=0x%8.8lx\n", (UINT32) pDevCtx->pCtrlPktsBase);
            printk("USBDBG: &pCtrlPktsBase=0x%8.8lx\n", (UINT32) &pDevCtx->pCtrlPktsBase);
            printk("USBDBG: &pRxDma=0x%8.8lx\n", (UINT32) ((DmaChannel *) DMA_BASE + USB_BULK_RX_CHAN));
            printk("USBDBG: &pTxDma=0x%8.8lx\n", (UINT32) ((DmaChannel *) DMA_BASE + USB_BULK_TX_CHAN));
#endif
        }

        if( nStatus == 0 )
        {
            /* Setup the callback functions. */
            dev->open               = bcmusbnic_open;
            dev->stop               = bcmusbnic_close;
            dev->hard_start_xmit    = bcmusbnic_xmit;
            dev->tx_timeout         = bcmusbnic_timeout;
            dev->watchdog_timeo     = USB_TIMEOUT;
            dev->get_stats          = bcmusbnic_query;
            dev->set_multicast_list = NULL;
            dev->do_ioctl           = &bcmusbnic_ioctl;

            /* Don't reset or enable the device yet. "Open" does that. */
            nStatus = register_netdev(dev);
            if (nStatus != 0) 
            {
                printk(KERN_ERR CARDNAME": register_netdev failed\n");
                kfree(pDevCtx);
                free_netdev(dev);
            }
        }
        else
            if( pDevCtx )
                kfree(pDevCtx);
    }
    else
        nStatus = -ENXIO;

    return( nStatus );
} /* bcmusbnic_probe */


/*******************************************************************************
*
* UsbNicInit
*
* This routine initializes resources used by the USB NIC driver.
*
* RETURNS: 0 - success, < 0 - error.
*/

LOCAL int UsbNicInit( PUSBNIC_DEV_CONTEXT pDevCtx )
{
    int nStatus = 0;
    UINT32 i, ulLen;
    volatile DmaDesc *pBd;
    UINT8 *p;
    volatile DmaChannelCfg *pRxDma, *pTxDma;
    PUSB_TX_HDR pTxHdr;
    PCTRL_PKT_BUFS pCtrlPkt;
    UINT8 *pSockBuf;
    UINT16 usVid = (UINT16) BE_SWAP2(USB_VENDOR_ID);
    UINT16 usPid = (UINT16) BE_SWAP2(USB_DSL_PRODUCT_ID);

    /* Make sure USB clock is on. */
    PERF->blkEnables |= USBS_CLK_EN;

    ulLen = (USB_NUM_RX_PKTS * RX_USB_ALLOC_TOTAL_SIZE) + 16;
    if( (pDevCtx->pRxBufs = (char *) kmalloc(ulLen, GFP_KERNEL)) == NULL )
        nStatus = -ENOMEM;

    if( nStatus == 0 )
    {
        /* Make BDs paragraph (16 byte) aligned in uncached memory */
        p = (UINT8 *) KSEG1ADDR(&pDevCtx->RxBds[0]);
        pDevCtx->pRxBdBase = (DmaDesc *) USB_NIC_ALIGN(p, 16);
        pDevCtx->pRxBdAssign = pDevCtx->pRxBdRead = pDevCtx->pRxBdBase;

        p = (UINT8 *) KSEG1ADDR(&pDevCtx->TxBds[0]);
        pDevCtx->pTxBdBase = (DmaDesc *) USB_NIC_ALIGN(p, sizeof(long));
        pDevCtx->pTxBdNext = pDevCtx->pTxBdBase;

        p = (UINT8 *) KSEG1ADDR(&pDevCtx->RxCtrlBds[0]);
        pDevCtx->pRxCtrlBdBase = (DmaDesc *) USB_NIC_ALIGN(p, 16);
        pDevCtx->pRxCtrlBdCurr = pDevCtx->pRxCtrlBdNext = pDevCtx->pRxCtrlBdBase;

        p = (UINT8 *) &pDevCtx->CtrlPkts[0];
        pDevCtx->pCtrlPktsBase = (PCTRL_PKT_BUFS) USB_NIC_ALIGN(p, 16);

        p = (UINT8 *) KSEG1ADDR(&pDevCtx->TxCtrlBd[0]);
        pDevCtx->pTxCtrlBdBase = (DmaDesc *) USB_NIC_ALIGN(p, 16);

        dma_cache_wback_inv((unsigned long) pDevCtx->pRxBufs, ulLen);

        /* Initialize RX ring pointer variables. */
        for( i = 0, pBd = pDevCtx->pRxBdBase,
             p = (unsigned char *) USB_NIC_ALIGN(pDevCtx->pRxBufs, 16);
             i < USB_NR_RX_BDS; i++, pBd++, p += RX_USB_ALLOC_TOTAL_SIZE )
        {
            pBd->status = DMA_OWN;
            pBd->length = RX_USB_NIC_BUF_SIZE;
            pBd->address = (UINT32) VIRT_TO_PHY(p + RX_USB_SKB_RESERVE);
        }

        pDevCtx->pRxBdBase[USB_NR_RX_BDS - 1].status |= DMA_WRAP;

        /* Initialize TX ring pointer variables. */
        for( i = 0, pBd = pDevCtx->pTxBdBase; i < USB_NR_TX_BDS; i++, pBd++ )
        {
            pBd->status = 0;
            pBd->length = 0;
            pBd->address = 0;
        }

        pDevCtx->pTxBdBase[USB_NR_TX_BDS - 1].status = DMA_WRAP;

        /* Initialize RX control endpoint ring pointer variables. */
        for( i = 0, pBd = pDevCtx->pRxCtrlBdBase,
            pCtrlPkt = pDevCtx->pCtrlPktsBase;
            i < NR_RX_CTRL_BDS; i++, pBd++, pCtrlPkt++ )
        {
            pBd->status = DMA_OWN;
            pBd->length = MAX_CTRL_PKT_SIZE;
            pBd->address = (UINT32) VIRT_TO_PHY(pCtrlPkt->ulRxBuf);
            pCtrlPkt->pRxBd = pBd;
        }

        pDevCtx->pRxCtrlBdBase[NR_RX_CTRL_BDS - 1].status |= DMA_WRAP;

        /* Make the transmit header structures long word aligned */
        p = (char *) pDevCtx->TxHdrs;
        pDevCtx->pTxHdrBase = (PUSB_TX_HDR) USB_NIC_ALIGN(p, sizeof(long));

        /* Chain USB_TX_HDR structures together. */
        for(i = 0, pTxHdr = pDevCtx->pTxHdrBase; i < NR_TX_HDRS-1; i++, pTxHdr++)
        {
            pTxHdr->pNext = pTxHdr + 1;
        }

        /* Chain socket buffers. */
        for( i = 0, pSockBuf = (unsigned char *)
             (((unsigned long) pDevCtx->SockBufs + 0x0f) & ~0x0f);
             i < USB_NUM_RX_PKTS; i++, pSockBuf += SKB_ALIGNED_SIZE )
        {
            ((struct sk_buff *) pSockBuf)->retfreeq_context =
                pDevCtx->pFreeSockBufList;
            pDevCtx->pFreeSockBufList = (struct sk_buff *) pSockBuf;
        }

        pDevCtx->pTxHdrBase[NR_TX_HDRS - 1].pNext = NULL;

        pDevCtx->pTxHdrFreeHead = pDevCtx->pTxHdrBase; /* first tx header */
        pDevCtx->pTxHdrFreeTail = pTxHdr; /* last tx header */
        pDevCtx->pTxHdrReclaimHead = NULL;
        pDevCtx->pTxHdrReclaimTail = NULL;

        /* Disable interrupts. */
        BcmHalInterruptDisable(INTERRUPT_ID_USB_BULK_RX_DMA);
        BcmHalInterruptDisable(INTERRUPT_ID_USB_BULK_TX_DMA);
        BcmHalInterruptDisable(INTERRUPT_ID_USB_CNTL_RX_DMA);
        BcmHalInterruptDisable(INTERRUPT_ID_USBS);

        /* Set USB Vendor ID and Product ID. */
        pDevCtx->pUsbCfg->deviceDesc.idVendor = usVid;
        pDevCtx->pUsbCfg->deviceDesc.idProduct = usPid;

        SetMacAddr( pDevCtx->pStrTbl[SERIAL_NUM_INDEX].string,
            (char *) pDevCtx->ucPermanentHostMacAddr);

        /*------------------------------------------------------------------
         * initialize the DMA channels
         *------------------------------------------------------------------*/

        /* clear State RAM */
        pDevCtx->pDmaRegs = (DmaRegs *) USB_DMA_BASE;
        memset( (char *) &pDevCtx->pDmaRegs->stram.s[FIRST_USB_DMA_CHANNEL],
            0x00, sizeof(DmaStateRam) * NR_USB_DMA_CHANNELS );

        /* Bulk endpoint DMA initialization. */
        pDevCtx->pRxDma = pRxDma = pDevCtx->pDmaRegs->chcfg + USB_BULK_RX_CHAN;
        pDevCtx->pTxDma = pTxDma = pDevCtx->pDmaRegs->chcfg + USB_BULK_TX_CHAN;

        /* bulk endpoint transmit */
        pTxDma->cfg = 0;    /* initialize first (will enable later) */
        pTxDma->maxBurst = USB_DMA_MAX_BURST_LENGTH;
        pTxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE; /* clear */
        pTxDma->intMask = DMA_DONE; /* configure */
        pDevCtx->pDmaRegs->stram.s[USB_BULK_TX_CHAN].baseDescPtr =
            (UINT32) VIRT_TO_PHY(pDevCtx->pTxBdBase);

        /* bulk endpoint receive */
        pRxDma->cfg = 0;    /* initialize first (will enable later) */
        pRxDma->maxBurst = USB_DMA_MAX_BURST_LENGTH;
        pRxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE; /* clear */
        pRxDma->intMask = DMA_DONE | DMA_NO_DESC; /* configure */
        pDevCtx->pDmaRegs->stram.s[USB_BULK_RX_CHAN].baseDescPtr =
            (UINT32) VIRT_TO_PHY(pDevCtx->pRxBdBase);

        /* Control endpoint DMA initialization. */
        pDevCtx->pRxCtrlDma=pRxDma=pDevCtx->pDmaRegs->chcfg + USB_CNTL_RX_CHAN;
        pDevCtx->pTxCtrlDma=pTxDma=pDevCtx->pDmaRegs->chcfg + USB_CNTL_TX_CHAN;

        /* control endpoint transmit */
        pTxDma->cfg = 0;    /* initialize first (will enable later) */
        pTxDma->maxBurst = USB_DMA_MAX_BURST_LENGTH;
        pTxDma->intStat = DMA_DONE;
        pTxDma->intMask = 0;  /* mask all ints */
        pDevCtx->pDmaRegs->stram.s[USB_CNTL_TX_CHAN].baseDescPtr =
            (UINT32) VIRT_TO_PHY(pDevCtx->pTxCtrlBdBase);

        /* control endpoint receive */
        pRxDma->cfg = 0;    /* initialize first (will enable later) */
        pRxDma->maxBurst = USB_DMA_MAX_BURST_LENGTH;
        pRxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        pRxDma->intMask = 0;  /* mask all ints */
        pDevCtx->pDmaRegs->stram.s[USB_CNTL_RX_CHAN].baseDescPtr =
            (UINT32) VIRT_TO_PHY(pDevCtx->pRxCtrlBdBase);

        /* Enable USB to send and receive data. */
        UsbNicReset( pDevCtx->pUsbCfg, pDevCtx->ulUsbCfgSize );
        UsbNicEnable( pDevCtx->pDmaRegs, pDevCtx->pRxDma, pDevCtx->pTxDma,
            pDevCtx->pRxCtrlDma, (unsigned int) pDevCtx );

        /* Start a timer that reclaims transmitted packets. */
        pDevCtx->ReclaimTimer.expires = jiffies + USB_TIMEOUT;
        add_timer(&pDevCtx->ReclaimTimer);
    }

    return( nStatus );
} /* UsbNicInit */


/******************************************************************************
*
* SetMacAddr
*
* This routine copies a MAC address as an ANSI string.
*
* RETURNS: None.
*/

LOCAL void SetMacAddr( char *pStrBuf, UINT8 *pucMacAddr )
{
    char szHexChars[] = "0123456789abcdef";
    UINT8 ch;
    int i;
    for( i = 0; i < MAC_ADDR_LEN; i++ )
    {
        ch = pucMacAddr[i];
        *pStrBuf++ = szHexChars[ch >> 4];
        *pStrBuf++ = szHexChars[ch & 0x0f];
    }
    *pStrBuf = '\0'; /* null terminate string  */
} /* SetMacAddr */


/******************************************************************************
*
* UsbNicReset
*
* This routine Writes to USB configuration to the USB MAC.
*
* RETURNS: None.
*/

LOCAL void UsbNicReset( UsbConfiguration *pUsbCfg, UINT32 ulUsbCfgSize )
{
    UINT32 *pulCfg;
    int i, nErrors;

    /* soft reset */
    USB->usb_cntl = USB_SOFT_RESET;

    /* Delay for reset to complete (at least 32 clks). */
    mdelay(2);

    /* Program the conf_mem space with descriptors. */
    USB->conf_mem_ctl = 0;
    USB->conf_mem_read_address = 0;

    /* If the configuration is not divisible by a long word, increase the
     * size by one more long word.
     */
    if( (ulUsbCfgSize & (sizeof(long) - 1)) != 0 )
        ulUsbCfgSize += sizeof(long);
    ulUsbCfgSize /= sizeof(long);

    /* Write the configuration out in long words. */
    pulCfg = (UINT32 *) pUsbCfg;
    for( i = 0; i < ulUsbCfgSize; i++ )
    {
        USB->conf_mem_write_address = i;
        USB->confmem_write_port = LE_SWAP4(*pulCfg);
        pulCfg++;
    }

    /* Read back the USB configuration. */
    USB->conf_mem_ctl = USB_CONF_MEM_RD;
    pulCfg = (UINT32 *) pUsbCfg;
    nErrors = 0;

    for( i = 0; i< ulUsbCfgSize; i++ )
    {
        USB->conf_mem_read_address = i;
        if( USB->confmem_read_port != LE_SWAP4(*pulCfg) )
            nErrors++;
        pulCfg++;
    }

    USB->conf_mem_ctl = 0;

    if( nErrors )
        printk((KERN_CRIT "USB Configuration not written correctly.\n"));
} /* UsbNicReset */


/******************************************************************************
*
* UsbNicEnable
*
* This routine enables the USB device.
*
* RETURNS: None.
*/

LOCAL void UsbNicEnable( volatile DmaRegs *pDmaRegs,
    volatile DmaChannelCfg *pRxDma, volatile DmaChannelCfg *pTxDma,
    volatile DmaChannelCfg *pRxCtrlDma, unsigned int ulParam )
{
    USB->irq_status = 0;  /* clr any pending interrupts then set mask */

    /* Register the interrupt service handlers and enable interrupts. */
    BcmHalInterruptDisable( INTERRUPT_ID_USB_BULK_RX_DMA );
    BcmHalInterruptDisable( INTERRUPT_ID_USB_CNTL_RX_DMA );
    BcmHalInterruptDisable( INTERRUPT_ID_USBS );

    BcmHalMapInterrupt( bcmBulkRxIsr, ulParam,
        INTERRUPT_ID_USB_BULK_RX_DMA );
    BcmHalMapInterrupt( RndisCtrlRxIsr, ulParam,
        INTERRUPT_ID_USB_CNTL_RX_DMA );
    BcmHalMapInterrupt( bcmUsbIsr, ulParam, INTERRUPT_ID_USBS );

    BcmHalInterruptEnable( INTERRUPT_ID_USB_BULK_RX_DMA );
    BcmHalInterruptEnable( INTERRUPT_ID_USB_CNTL_RX_DMA );
    BcmHalInterruptEnable( INTERRUPT_ID_USBS );

    /* Enable DMA on the control enpoint. */
    USB->endpt_cntl &= ~(USB_RX_CNTL_DMA_EN | USB_TX_CNTL_DMA_EN);

    /* Configure DMA channels and enable Rx. */
    pTxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    pTxDma->intMask = DMA_DONE;
    pTxDma->cfg = 0;

    pDmaRegs->controller_cfg |= DMA_MASTER_EN;

    pRxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    pRxDma->intMask = DMA_DONE | DMA_NO_DESC;
    pRxDma->cfg = DMA_ENABLE;

    pRxCtrlDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    pRxCtrlDma->intMask = DMA_DONE;
    pRxCtrlDma->cfg = DMA_ENABLE;

    /* USB Ready. Enable configuration memory. */
    if( (USB->status_frameNum & USB_LINK) == USB_LINK )
        USB->conf_mem_ctl |= USB_CONF_MEM_RDY;
    else
        USB->conf_mem_ctl &= ~USB_CONF_MEM_RDY;

    /* Clear any pending interrupts then set mask. */
    USB->irq_status = 0;
    USB->irq_mask = USB_SETUP_COMMAND_RECV | USB_OUT_FIFO_OV | USB_RESET_RECV |
        USB_SUSPEND_RECV | USB_FIFO_REWIND;
} /* UsbNicEnable */


/*******************************************************************************
*
* bcmusbnic_cleanup
*
* Called when the driver is no longer used.
*
* RETURNS: None.
*/

LOCAL void bcmusbnic_cleanup(void)
{
    PUSBNIC_DEV_CONTEXT pDevCtx = g_pDevCtx;

    if( pDevCtx && pDevCtx->pDev && pDevCtx->pDev->name[0] != '\0' )
    {
        PUSB_TX_HDR pReclaimHead = pDevCtx->pTxHdrReclaimHead;

        del_timer_sync(&pDevCtx->ReclaimTimer);

        UsbNicDisable( pDevCtx->pRxDma, pDevCtx->pTxDma );

        // release the memory
        if (pDevCtx->pRxBufs)
            kfree(pDevCtx->pRxBufs);
        
        /* No need to check MOD_IN_USE, as sys_delete_module() checks. */
        unregister_netdev( pDevCtx->pDev );

        while( pReclaimHead )
        {
            dev_kfree_skb_any( pReclaimHead->pSockBuf );
            pReclaimHead = pReclaimHead->pNext;
        }

        kfree( pDevCtx);
    }
} /* bcmusbnic_cleanup */


/******************************************************************************
*
* UsbNicDisable
*
* This routine disables the USB device.
*
* RETURNS: None.
*/

LOCAL void UsbNicDisable( volatile DmaChannelCfg *pRxDma,
    volatile DmaChannelCfg *pTxDma )
{
    /* Disable interrupts. */
    USB->irq_mask = 0;

    /* Take down USB_READY */
    USB->conf_mem_ctl = 0;

    /* Stop DMA channels. */
    pTxDma->intMask = 0;
    pRxDma->intMask = 0;
} /* UsbNicDisable */


/*******************************************************************************
*
* bcmusbnic_open
*
* Makes the USB device operational.  Called due to the shell command,
* "ifconfig ethX up"
*
* RETURNS: None.
*/

LOCAL int bcmusbnic_open( struct net_device *dev )
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#else
    MOD_INC_USE_COUNT;
#endif

    pDevCtx->ulFlags |= USB_FLAGS_OPENED;

    /* Start the network engine. */
    netif_start_queue(dev);

    return( 0 );
} /* bcmusbnic_open */



/*******************************************************************************
*
* bcmusbnic_close
*
* Stops the USB device.  Called due to the shell command,
* "ifconfig ethX down"
*
* RETURNS: None.
*/

LOCAL int bcmusbnic_close(struct net_device *dev)
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv;

    netif_stop_queue(dev);
 
    pDevCtx->ulFlags &= ~USB_FLAGS_OPENED;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#else
    MOD_DEC_USE_COUNT;
#endif

    return( 0 );
} /* bcmusbnic_close */


/******************************************************************************
*
* bcmusbnic_timeout
*
* This routine processes a transmit timeout. 
*
* RETURNS: None.
*/

LOCAL void bcmusbnic_timeout(struct net_device *dev)
{
    /* Someone thinks we stalled. Now what??? */
    printk(KERN_WARNING CARDNAME ": transmit timed out\n");

    dev->trans_start = jiffies;

    netif_wake_queue(dev);
} /* bcmusbnic_timeout */


/******************************************************************************
*
* bcmusbnic_query
*
* This routine returns the device statistics structure.
*
* RETURNS: device statistics structure
*/

LOCAL struct net_device_stats *bcmusbnic_query(struct net_device *dev)
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv;
    return( &pDevCtx->DevStats );
} /* bcmusbnic_query */


/* Ethtool support */
LOCAL int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr)
{
    /* PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv; */
    u32 ethcmd;

    if (copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)))
       return -EFAULT;

    switch (ethcmd) {
    /* get driver-specific version/etc. info */
    case ETHTOOL_GDRVINFO: {
        struct ethtool_drvinfo info = {ETHTOOL_GDRVINFO};

        strncpy(info.driver, CARDNAME, sizeof(info.driver)-1);
        strncpy(info.version, VERSION, sizeof(info.version)-1);
        if (copy_to_user(useraddr, &info, sizeof(info)))
           return -EFAULT;
        return 0;
        }
    /* get settings */
    case ETHTOOL_GSET: {
        struct ethtool_cmd ecmd = { ETHTOOL_GSET };

        ecmd.speed = SPEED_10;
 
        if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
           return -EFAULT;
        return 0;
        }
    }

    return -EOPNOTSUPP;    
}

/******************************************************************************
*
* bcmusbnic_ioctl
*
* This routine return nonzero if link up, otherwise return 0
*
* RETURNS: 0 = link down, !0 = link up
*/
LOCAL int bcmusbnic_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv;
    int *data=(int*)rq->ifr_data;
    int status;

    /* we can add sub-command in ifr_data if we need to in the future */
    switch (cmd)
    {
    case SIOCGLINKSTATE:
        if( (pDevCtx->ulFlags & USB_FLAGS_PLUGGED_IN) != 0 )
            status = LINKSTATE_UP;
        else
            status = LINKSTATE_DOWN;
        if (copy_to_user((void*)data, (void*)&status, sizeof(int)))
            return -EFAULT;
        break;

    case SIOCSCLEARMIBCNTR:
        memset(&pDevCtx->DevStats, 0, sizeof(struct net_device_stats));
        break;

    case SIOCMIBINFO:
        if (copy_to_user((void*)data, (void*)&pDevCtx->MibInfo,
            sizeof(pDevCtx->MibInfo)))
        {
            return -EFAULT;
        }
        break;

    case SIOCETHTOOL:
        return netdev_ethtool_ioctl(dev, (void *) rq->ifr_data);

    }

    return 0;
} /* bcmusbnic_ioctl */


/*******************************************************************************
*
* UsbEthernetPacketFilter - Filter out packets before sending
*
* This routine uses a filter mask passed in the SET_ETHERNET_PACKET_FILTER
* class command to filter out some types of packets.
*
* RETURNS: 1 = filter packet, 0 = do not filter packet
*/

LOCAL inline int UsbNicEthernetPacketFilter( UINT16 usPacketFilterBitmap,
    UINT8 *pDestAddr, UINT8 *pMacAddr, UINT8 *pucMcList, UINT8 ucMcListSize )
{
    int nFilterOn = 1;

    if( ((usPacketFilterBitmap & NDIS_PACKET_TYPE_PROMISCUOUS) != 0) )
    {
        nFilterOn = 0;
    }
    else
    {
        if( ((usPacketFilterBitmap & NDIS_PACKET_TYPE_DIRECTED) != 0 &&
            *(UINT32 *) &pDestAddr[0] == *(UINT32 *) &pMacAddr[0] &&
            *(UINT16 *) &pDestAddr[4] == *(UINT16 *) &pMacAddr[4]) )
        {
            nFilterOn = 0;
        }
        else
        {
            if( (pDestAddr[0] & 0x01) == 0x01 )
            {
                if( *(UINT32 *) &pDestAddr[0] == 0xffffffff &&
                    *(UINT16 *) &pDestAddr[4] == 0xffff )
                {
                    if((usPacketFilterBitmap & NDIS_PACKET_TYPE_BROADCAST) != 0)
                        nFilterOn = 0;
                }
                else
                {
                    if((usPacketFilterBitmap&NDIS_PACKET_TYPE_ALL_MULTICAST)!=0)
                        nFilterOn = 0;
                }
            }

            if( nFilterOn == 1 &&
                (usPacketFilterBitmap & NDIS_PACKET_TYPE_MULTICAST) != 0 )
            {
                UINT8 i;
                for( i = 0; i < ucMcListSize; i++, pucMcList += ETH_ALEN )
                {
                    if( *(UINT32 *) &pDestAddr[0] == *(UINT32 *) &pucMcList[0] &&
                        *(UINT16 *) &pDestAddr[4] == *(UINT16 *) &pucMcList[4] )
                    {
                        nFilterOn = 0;
                        break;
                    }
                }
            }
        }
    }

    return( nFilterOn );
} /* UsbNicEthernetPacketFilter */


/*******************************************************************************
*
* UsbNicSendNow - Prepare to add data to the transmit queue.
*
* This routine fills in a USB_TX_HDR structure for preparing to send data.
*
* RETURNS: 0 - success, 1 - error
*/

LOCAL inline int UsbNicSendNow(struct sk_buff *skb, PUSBNIC_DEV_CONTEXT pDevCtx)
{
    int nStatus = 0;
    PUSB_TX_HDR pTxHdr = pDevCtx->pTxHdrFreeHead;

    if( pTxHdr )
    {
        volatile DmaDesc *pBd = pDevCtx->pTxBdNext;
        volatile DmaDesc *pBdPktHdr = NULL;
        UINT32 ulAlignLen = 0;

        if( pDevCtx->ulHostDriverType == HOST_DRIVER_RNDIS )
        {
            UINT32 ulHdrLen = sizeof(RNDIS_PACKET) + RNDIS_PACKET_OVERHEAD;
            UINT32 ulTotalLen = ulHdrLen + skb->len;
            UINT32 ulOffset = sizeof(RNDIS_PACKET);

            /* The buffer length of this BD must be a mulitple of 4 and the
             * starting buffer address of the next BD must be a multiple of 4.
             */
            ulAlignLen = (UINT32) skb->data & (sizeof(long) - 1);
            if( ulAlignLen )
            {
                UINT32 ulAlignDifference = sizeof(long) - ulAlignLen;
                memcpy((char *) &pTxHdr->PktHdr + ulHdrLen + ulAlignDifference,
                    skb->data, ulAlignLen);
                ulHdrLen += sizeof(long);
                ulTotalLen += ulAlignDifference;
                ulOffset += ulAlignDifference;
            }

            pTxHdr->PktHdr.NdisMessageType = BE_SWAP4(REMOTE_NDIS_PACKET_MSG);
            pTxHdr->PktHdr.MessageLength = BE_SWAP4(ulTotalLen);
            pTxHdr->PktHdr.Message.Packet.DataLength = BE_SWAP4(skb->len);
            pTxHdr->PktHdr.Message.Packet.DataOffset = BE_SWAP4(ulOffset);

            pBdPktHdr = pBd;

            dma_cache_wback_inv((unsigned long) &pTxHdr->PktHdr, ulHdrLen );
            pBdPktHdr->address = (UINT32) VIRT_TO_PHY(&pTxHdr->PktHdr);
            pBdPktHdr->length = ulHdrLen;

            /* Advance BD pointer to next in the chain. */
            if( (pBdPktHdr->status & DMA_WRAP) != 0 )
                pDevCtx->pTxBdNext = pDevCtx->pTxBdBase;
            else
                pDevCtx->pTxBdNext++;

            pBd = pDevCtx->pTxBdNext;
        }

        pDevCtx->pTxHdrFreeHead = pTxHdr->pNext;
        pTxHdr->pBdAddr = pBd;
        pTxHdr->pSockBuf = skb;

        dma_cache_wback_inv((unsigned long)skb->data, skb->len);
        pBd->address = (UINT32) VIRT_TO_PHY(skb->data + ulAlignLen);
        pBd->length = skb->len - ulAlignLen;

        /* Advance BD pointer to next in the chain. */
        if( (pBd->status & DMA_WRAP) != 0 )
            pDevCtx->pTxBdNext = pDevCtx->pTxBdBase;
        else
            pDevCtx->pTxBdNext++;

        /* Packet was enqueued correctly.  Advance to the next TXHDR needing to
         * be assigned to a BD.
         */
        if( pDevCtx->pTxHdrReclaimHead == NULL )
            pDevCtx->pTxHdrReclaimHead = pTxHdr;
        else
            pDevCtx->pTxHdrReclaimTail->pNext = pTxHdr;

        pDevCtx->pTxHdrReclaimTail = pTxHdr;
        pTxHdr->pNext = NULL;

        /* Start DMA transmit by setting own bit. */
        if( pBdPktHdr ) /* RNDIS */
        {
            pBd->status = (pBd->status & DMA_WRAP) | DMA_EOP | DMA_OWN;
            pBdPktHdr->status = (pBdPktHdr->status&DMA_WRAP) | DMA_SOP | DMA_OWN;
        }
        else /* CDC */
            pBd->status = (pBd->status & DMA_WRAP) | DMA_SOP | DMA_EOP | DMA_OWN;

        /* Enable DMA for this channel */
        pDevCtx->pTxDma->cfg |= DMA_ENABLE;

        pDevCtx->pDev->trans_start = jiffies;

        /* Update record of total bytes sent. */
        pDevCtx->DevStats.tx_bytes += skb->len;
        if( pBdPktHdr )
            pDevCtx->DevStats.tx_bytes += sizeof(RNDIS_MESSAGE);

        nStatus = 0;
    }
    else
    {
        /* No more transmit headers. */
        pDevCtx->DevStats.tx_aborted_errors++;
        nStatus = 1;
    }

    return( nStatus );
} /* UsbNicSendNow */


/******************************************************************************
*
* bcmusbnic_xmit
*
* This routine sends a MAC frame over the USB.
*
* RETURNS: 0 - sucess, < 0 - error
*/

LOCAL int bcmusbnic_xmit(struct sk_buff *skb, struct net_device *dev)
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev->priv;
    PUSB_TX_HDR pReclaimHead = pDevCtx->pTxHdrReclaimHead;
    PUSB_TX_HDR pTxHdr;
    PENET_HDR pEh;

    /* Obtain exclusive access to transmitter.  This is necessary because
     * we might have more than one stack transmitting at once.
     */
    netif_stop_queue(pDevCtx->pDev);

    /* Reclaim transmitted buffers.  Loop here for each Tx'ed packet. */
    if( pReclaimHead && (pReclaimHead->pBdAddr->status & DMA_OWN) == 0 )
    {
        do
        {
            /* Indicate to the upper levels that this ethernet packet was sent. */
            dev_kfree_skb_any( pReclaimHead->pSockBuf );

            /* Finally, return the transmit header to the free list. */
            pTxHdr = pReclaimHead;
            pReclaimHead = pReclaimHead->pNext;
            pTxHdr->pNext = pDevCtx->pTxHdrFreeHead;
            pDevCtx->pTxHdrFreeHead = pTxHdr;
            if( pDevCtx->pTxHdrFreeTail == NULL )
                pDevCtx->pTxHdrFreeTail = pTxHdr;
        } while(pReclaimHead && (pReclaimHead->pBdAddr->status & DMA_OWN) == 0);

        /* Assign current reclaim head and tail pointers. */
        if( pReclaimHead == NULL )
            pDevCtx->pTxHdrReclaimHead = pDevCtx->pTxHdrReclaimTail = NULL;
        else
            pDevCtx->pTxHdrReclaimHead = pReclaimHead;
    }

    if( skb )
    {
        if( (pDevCtx->ulFlags & (USB_FLAGS_CONNECTED | USB_FLAGS_OPENED)) == 
            (USB_FLAGS_CONNECTED | USB_FLAGS_OPENED) )
        {
            pDevCtx->DevStats.tx_packets++;

            /* Send the packet if it is not filtered */
            pEh = (PENET_HDR) skb->data;
            if( UsbNicEthernetPacketFilter( pDevCtx->usPacketFilterBitmap,
                pEh->dest, pDevCtx->ucCurrentHostMacAddr,
                pDevCtx->ucRndisMulticastList,
                pDevCtx->ucRndisNumMulticastListEntries ) )
            {
                dev_kfree_skb_any (skb);
            }
            else
                if( UsbNicSendNow( skb, pDevCtx ) != 0 )
                    dev_kfree_skb_any (skb);
        }
        else
        {
            dev_kfree_skb_any (skb);
            /* pDevCtx->DevStats.tx_dropped++; */
        }
    }

    netif_wake_queue(dev);

    return( 0 );
} /* bcmusbnic_xmit */


/*******************************************************************************
*
* usbnic_timer - free tranmit buffer complete and process receive buffers.
*
* This routine reclaims transmit frames which have been sent and processes
* receive buffers.  This is not a normal execution path.  Typically, transmit
* buffers a freed when sending the next frame and receive buffers are received
* due to an interrupt.  This function is used for special cases that are not
* caught by those conditions.
*
* RETURNS: None.
*/

LOCAL void usbnic_timer( PUSBNIC_DEV_CONTEXT pDevCtx )
{
    /* Free transmitted buffers.  */
    if( netif_queue_stopped(pDevCtx->pDev) == 0 )
        bcmusbnic_xmit(NULL, pDevCtx->pDev);

    /* Check for link up/down. */
    if( (pDevCtx->ulFlags & USB_FLAGS_PLUGGED_IN) == USB_FLAGS_PLUGGED_IN &&
        (USB->status_frameNum & USB_LINK) == 0 )
    {
        USB->irq_mask = 0;
        pDevCtx->ulFlags &= ~USB_FLAGS_PLUGGED_IN;
        USB->conf_mem_ctl &= ~USB_CONF_MEM_RDY;
        pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
        pDevCtx->MibInfo.ulIfSpeed = 0;
        printk("USB Link DOWN.\r\n");

        /* Wake up the user mode application that monitors link status. */
        kerSysWakeupMonitorTask();
    }
    else
        if( (pDevCtx->ulFlags & USB_FLAGS_PLUGGED_IN) == 0 &&
            (USB->status_frameNum & USB_LINK) == USB_LINK )
        {
            pDevCtx->ulFlags |= USB_FLAGS_PLUGGED_IN;
            USB->conf_mem_ctl |= USB_CONF_MEM_RDY;
            USB->irq_status = 0;
            USB->irq_mask = USB_SETUP_COMMAND_RECV | USB_OUT_FIFO_OV |
                USB_RESET_RECV | USB_SUSPEND_RECV | USB_FIFO_REWIND;
            pDevCtx->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;
            pDevCtx->MibInfo.ulIfSpeed = 12000000; /* 12 Mb */
            printk("USB Link UP.\r\n");

            /* Wake up the user mode application that monitors link status. */
            kerSysWakeupMonitorTask();
        }

    /* Restart the timer. */
    pDevCtx->ReclaimTimer.expires = jiffies + USB_TIMEOUT;
    add_timer(&pDevCtx->ReclaimTimer);
} /* usbnic_timer */


/*******************************************************************************
*
* bcmBulkRxIsr - DMA Interrupt Service Routine for the USB bulk rx endpoint
*
* This routine is called when data has been received on the bulk endpoint.
* It schedules a function to run in the bottom half to process the received
* data.
*
* RETURNS: None.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
LOCAL irqreturn_t bcmBulkRxIsr(int irq, void *dev_id, struct pt_regs *regs)
#else
LOCAL void bcmBulkRxIsr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev_id;
    volatile DmaChannelCfg *pRxDma = pDevCtx->pRxDma;
    UINT32 ulRxIntStat = pRxDma->intStat;

    if( (ulRxIntStat & DMA_DONE) != 0 )
        pRxDma->intStat = DMA_DONE | DMA_BUFF_DONE;  /* clear interrupt */

#ifdef USE_BH
    tasklet_schedule(&pDevCtx->BhBulkRx);
#else
    usbnic_bulkrx(pDevCtx);
#endif    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    return IRQ_HANDLED;
#endif 
} /* bcmBulkRxIsr */

/*******************************************************************************
*
* usbnic_freeskbordata - Put socket buffer header to data back onto a free list.
*
* RETURNS: None.
*/

LOCAL void usbnic_freeskbordata( PUSBNIC_DEV_CONTEXT pDevCtx, void *pObj,
    int nFlag )
{
    local_bh_disable();

    if( nFlag == FREE_SKB )
    {
        struct sk_buff *skb = (struct sk_buff *) pObj;

        skb->retfreeq_context = pDevCtx->pFreeSockBufList;
        pDevCtx->pFreeSockBufList = skb;
    }
    else
    {
        volatile DmaDesc *pAssign = pDevCtx->pRxBdAssign;
        volatile DmaChannelCfg *pRxDma = pDevCtx->pRxDma;

        /* Some cache lines might be dirty. If these lines are flushed after the
         * DMA puts data into SDRAM, the data will be corrupted. Invalidating
         * the buffer will clear the dirty bit(s) and prevent this problem.
         */
        dma_cache_wback_inv((unsigned long) pObj, RX_USB_NIC_BUF_SIZE);

        if( pAssign->address == 0 )
        {
            pAssign->address =
                VIRT_TO_PHY((unsigned long) pObj + RX_USB_SKB_RESERVE);
            pAssign->length  = RX_USB_NIC_BUF_SIZE;
            pAssign->status = (pAssign->status & DMA_WRAP) | DMA_OWN;

            if( (pAssign->status & DMA_WRAP) != 0 )
                pDevCtx->pRxBdAssign = pDevCtx->pRxBdBase;
            else
                pDevCtx->pRxBdAssign++;
        }
        else
        {
            /* This should not happen. */
            printk("USB: No place to put free buffer.\n");
        }

        /* Restart DMA in case the DMA ran out of descriptors, and
         * is currently shut down.
         */
        if( (pRxDma->intStat & DMA_NO_DESC) != 0 )
        {
            pDevCtx->DevStats.rx_errors++;
            pRxDma->intStat = DMA_NO_DESC;
            pRxDma->cfg = DMA_ENABLE;
        }
    }

    __local_bh_enable();
} /* usbnic_freeskbordata */


/*******************************************************************************
*
* usbnic_bulkrx - Process received packets
*
* This routine runs in the context of a bottom half.  It processes packets
* received on USB bulk out endpoint.  It assumes/expects that one complete
* packet is received in one DMA buffer descriptor.
*
* RETURNS: None.
*/

LOCAL void usbnic_bulkrx( PUSBNIC_DEV_CONTEXT pDevCtx )
{
    struct sk_buff *skb;
    struct net_device *dev = pDevCtx->pDev;
    UINT8 *pBuf;
    UINT16 usLength, usMinLen;
    volatile DmaDesc *pRxBd = pDevCtx->pRxBdRead;

    if( pDevCtx->ulHostDriverType == HOST_DRIVER_RNDIS )
        usMinLen = sizeof(RNDIS_PACKET) + RNDIS_PACKET_OVERHEAD;
    else
        usMinLen = 0;

    /* Loop here for each received packet. */
    while( (pRxBd->status & DMA_OWN) == 0 && pRxBd->address != 0 )
    {
        pBuf = (UINT8 *) KSEG0ADDR(pRxBd->address);

        usLength = pRxBd->length;

        pRxBd->address = 0;
        pRxBd->status &= DMA_WRAP;

        if( (pRxBd->status & DMA_WRAP) != 0 )
            pRxBd = pDevCtx->pRxBdBase;
        else
            pRxBd++;

        if( pDevCtx->pFreeSockBufList && usLength >= usMinLen )
        {
            skb = pDevCtx->pFreeSockBufList;
            pDevCtx->pFreeSockBufList =
                pDevCtx->pFreeSockBufList->retfreeq_context;

            skb_hdrinit( RX_USB_SKB_RESERVE,
                RX_USB_ALLOC_BUF_SIZE - RX_USB_SKB_RESERVE, skb, pBuf,
                (void *) usbnic_freeskbordata, pDevCtx, 0 );
            __skb_trim(skb, usLength);

            if( pDevCtx->ulHostDriverType == HOST_DRIVER_RNDIS )
                __skb_pull(skb, sizeof(RNDIS_PACKET) + RNDIS_PACKET_OVERHEAD);

            /* Finish setting up the received SKB and send it to the kernel */
            skb->dev = dev;

            /* Set the protocol type */
            skb->protocol = eth_type_trans(skb, dev);

            pDevCtx->DevStats.rx_packets++;
            pDevCtx->DevStats.rx_bytes += usLength;

            /* Notify kernel if device is opened. */
            if( (pDevCtx->ulFlags & (USB_FLAGS_CONNECTED | USB_FLAGS_OPENED)) ==
                (USB_FLAGS_CONNECTED | USB_FLAGS_OPENED) )
            {
                netif_rx(skb);
            }
            else
            {
                /* pDevCtx->DevStats.rx_dropped++; */
                dev_kfree_skb_any( skb );
            }

            dev->last_rx = jiffies;
        }
        else
        {
            pDevCtx->DevStats.rx_dropped++;
            usbnic_freeskbordata( pDevCtx, pBuf, FREE_DATA );
        }
    }

    pDevCtx->pRxBdRead = pRxBd;

    /* Re-enable interrupts to resume packet reception. */
    BcmHalInterruptEnable(INTERRUPT_ID_USB_BULK_RX_DMA);
} /* usbnic_bulkrx */


/*******************************************************************************
*
* bcmUsbIsr - USB Interrupt Service Routine
*
* This routine schedules a function to run in the tNetTask context to process
* an event for the USB MAC.
*
* RETURNS: ISR handle status.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
LOCAL irqreturn_t bcmUsbIsr(int irq, void *dev_id, struct pt_regs *regs)
#else
LOCAL void bcmUsbIsr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    PUSBNIC_DEV_CONTEXT pDevCtx = dev_id;
    UINT32 ulIrqMask = USB->irq_mask;
    UINT32 ulIrqStatus = USB->irq_status;

    /* Mask all. */
    USB->irq_mask = 0;

    if( (ulIrqStatus & USB_NEW_CONFIG) != 0 )
    {
        USB->irq_status = USB_NEW_CONFIG; /* clear interrupt */
        pDevCtx->ulHostDriverType = USB->current_config >> 4;
        pDevCtx->ulFlags |= USB_FLAGS_PLUGGED_IN;
    }

    if( (ulIrqStatus & USB_SETUP_COMMAND_RECV) != 0 )
    {
        USB->irq_status = USB_SETUP_COMMAND_RECV; /* clear interrupt */
        UsbNicProcessSetupCommand( pDevCtx );
        pDevCtx->ulFlags |= USB_FLAGS_PLUGGED_IN;
    }

    if( (ulIrqStatus & USB_OUT_FIFO_OV) != 0 )
    {
        /* Clear interrupt condition */
        USB->irq_status = USB_OUT_FIFO_OV;
        USB->fifo_ovf_count = 0;
        ulIrqMask &= ~USB_OUT_FIFO_OV;
    }

    if( (ulIrqStatus & USB_FIFO_REWIND) != 0 )
    {
        /* Clear interrupt condition */
        USB->irq_status = USB_FIFO_REWIND;
        USB->fifo_rewind_cnt = 0;
        ulIrqMask &= ~USB_FIFO_REWIND;
    }

    if( (ulIrqStatus & USB_RESET_RECV) != 0 )
    {
        /* Clear all stalled endpoints. */
        USB->endpt_stall_reset |= (byte)(USB->endpt_prnt & USB->endpt_status);

        /* Clear interrupt condition. */
        USB->irq_status = USB_RESET_RECV | USB_FIFO_REWIND;

        ulIrqMask |= USB_OUT_FIFO_OV;
    }

    if( (ulIrqStatus & USB_SUSPEND_RECV) != 0 )
    {
        USB->irq_status = USB_SUSPEND_RECV; /* clear interrupt */
        pDevCtx->ulFlags &= ~USB_FLAGS_CONNECTED;
        pDevCtx->pRxCtrlBdCurr = pDevCtx->pRxCtrlBdNext;
    }

    USB->irq_mask = ulIrqMask;
    BcmHalInterruptEnable(INTERRUPT_ID_USBS);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    return IRQ_HANDLED;    
#endif
} /* bcmUsbIsr */


/*******************************************************************************
*
* UsbNicProcessSetupCommand - Process a received setup command.
*
* This routine processes a control pipe request from the USB host.
*
* RETURNS: None.
*/

void UsbNicProcessSetupCommand(PUSBNIC_DEV_CONTEXT pDevCtx )
{
    const int nSetupCommandSize = 8;
    UINT8 ucSetupMsg[nSetupCommandSize];
    USB_setup *pSetupMsg = (USB_setup *) ucSetupMsg;
    StringDesc *pStrDesc = &pDevCtx->StrDesc;
    UINT32 ulIndex;
    int i;
    int errFlag = 0;

    /* Read setup command */
    for( i = 0; i < sizeof(ucSetupMsg); i++ )
    {
        USB->dev_req_bytesel = i;
        ucSetupMsg[i] = (UINT8) USB->ext_dev_data;
    }

    /* Endian conversion. */
    pSetupMsg->wValue = BE_SWAP2(pSetupMsg->wValue);
    pSetupMsg->wIndex = BE_SWAP2(pSetupMsg->wIndex);
    pSetupMsg->wLength = BE_SWAP2(pSetupMsg->wLength);

    switch( (pSetupMsg->bmRequestType & RT_TYPE_MASK) )
    {
    case RT_TYPE_STANDARD:
        switch( pSetupMsg->bRequest)
        {
        case GET_DESCRIPTOR:
            switch( pSetupMsg->wValue >> 8 )
            {
            case STRING_DESC:
                pStrDesc->bDescriptorType = STRING_DESC;
                if((ulIndex = (pSetupMsg->wValue & 0x00ff)) < STRING_TABLE_SIZE)
                {
                    if( ulIndex == 0 )
                    {
                        pStrDesc->bLength =
                            strlen( pDevCtx->pStrTbl[ulIndex].string ) + 2;

                        strcpy( pStrDesc->genericString,
                            pDevCtx->pStrTbl[ulIndex].string );
                    }
                    else
                    {
                        /* Handle specific string descriptor, unicode form */
                        pStrDesc->bLength =
                            strlen( pDevCtx->pStrTbl[ulIndex].string ) * 2 + 2;

                        ConvertAnsiToUnicode( (char *) pStrDesc->genericString,
                            pDevCtx->pStrTbl[ulIndex].string );

                        /* The USB cable is plugged in.  Data is ready to be
                         * received if the CDC host driver is being used.
                         */
                        if( pDevCtx->ulHostDriverType == HOST_DRIVER_CDC )
                            pDevCtx->ulFlags |= USB_FLAGS_CONNECTED;
                    }
                }
                else
                {
                    /* String index is greater than string table size.  Signal
                     * error and abort processing.
                     */
                    pStrDesc->bLength = 3;
                    pStrDesc->genericString[0] = 0;
                    errFlag = 1;
                }

                /* Send string descriptor. */
                UsbNicSubmitControlPacket( pDevCtx, (UINT8 *) pStrDesc, (UINT16)
                    (pSetupMsg->wLength > pStrDesc->bLength)
                    ? pStrDesc->bLength : pSetupMsg->wLength );
                break;

            case DEVICE_DESC:
            case CONFIGURATION_DESC:
            case INTERFACE_DESC:
            case ENDPOINT_DESC:
                break;

            default:
                /* Invalid descriptor type. Signal error and abort processing.*/
                errFlag = 1;
                break;
            }
            break;

        case GET_STATUS:
        case CLEAR_FEATURE:
        case SET_FEATURE:
        case SET_ADDRESS:
        case SET_DESCRIPTOR:
        case GET_CONFIGURATION:
        case SET_CONFIGURATION:
        case GET_INTERFACE:
        case SET_INTERFACE:
            break;

        case SYNCH_FRAME:
            /* dg-mod, currently unsupported (signal error) */
            errFlag = 1;
            break;

        default:
            /* Invalid setup request.  Signal error and abort processing. */
            errFlag = 1;
            break;
        }
        break;

    case RT_TYPE_CLASS:
        switch( pSetupMsg->bRequest)
        {
        case SEND_ENCAPSULATED_COMMAND:
            /* This command is processed by the receive control endpoint
             * buffer done ISR, RndisCtrlRxIsr.
             */
            break;

        case GET_ENCAPSULATED_RESPONSE:
            /* Sent by host Remote NDIS driver. */
            RndisDoGetEncapsultedResponse( pDevCtx, pSetupMsg->wLength );
            break;

        case SET_ETHERNET_PACKET_FILTER:
            /* Should be sent by host CDC driver. */
            pDevCtx->usPacketFilterBitmap = 0;

            if( (pSetupMsg->wValue & USB_PACKET_TYPE_MULTICAST) != 0 )
                pDevCtx->usPacketFilterBitmap |= NDIS_PACKET_TYPE_MULTICAST;

            if( (pSetupMsg->wValue & USB_PACKET_TYPE_BROADCAST) != 0 )
                pDevCtx->usPacketFilterBitmap |= NDIS_PACKET_TYPE_BROADCAST;

            if( (pSetupMsg->wValue & USB_PACKET_TYPE_DIRECTED) != 0 )
                pDevCtx->usPacketFilterBitmap |= NDIS_PACKET_TYPE_DIRECTED;

            if( (pSetupMsg->wValue & USB_PACKET_TYPE_ALL_MULTICAST) != 0 )
                pDevCtx->usPacketFilterBitmap |= NDIS_PACKET_TYPE_ALL_MULTICAST;

            if( (pSetupMsg->wValue & USB_PACKET_TYPE_PROMISCUOUS) != 0 )
                pDevCtx->usPacketFilterBitmap |= NDIS_PACKET_TYPE_PROMISCUOUS;
            break;

        default:
            /* Invalid setup request.  Signal error and abort processing. */
            errFlag = 1;
            break;
        }
        break;

    default:
        errFlag = 1;
        break;
    }

    /* Signal end of command processing. */
    if( errFlag == 1 )
    {
        USB->usb_cntl = USB_COMMAND_ERR;
        USB->endpt_cntl = USB_ENDPOINT_0; /* issue stall on endpoint 0 */
        USB->usb_cntl = (USB_COMMAND_OVER | USB_COMMAND_ERR);
    }
    else
        USB->usb_cntl |= USB_COMMAND_OVER;

} /* UsbNicProcessSetupCommand */


/*******************************************************************************
*
* ConvertAnsiToUnicode
*
* This routine onverts an ANSI string to a Unicode string by inserting a NULL
* character between each ANSI character.
*
* RETURNS: None.
*/

LOCAL void ConvertAnsiToUnicode( char *pszOut, char *pszIn )
{
    while( *pszIn )
    {
        *pszOut++ = *pszIn++;
        *pszOut++ = 0;
    }
} /* ConvertAnsiToUnicode */


/*******************************************************************************
*
* UsbNicSubmitControlPacket
*
* This routine sends a USB packet on the control pipe.  The packet buffer must
* be long word (32 bit) aligned.
*
* RETURNS: None.
*/

void UsbNicSubmitControlPacket( PUSBNIC_DEV_CONTEXT pDevCtx, UINT8 *pBuf,
    UINT32 ulLength )
{
    UINT32 ulStatus;
    volatile DmaChannelCfg *pTxDma = pDevCtx->pTxCtrlDma;
    volatile DmaDesc *pBd = pDevCtx->pTxCtrlBdBase;

    if( (pTxDma->cfg & DMA_ENABLE) == 0 )
    {
        dma_cache_wback_inv((unsigned long) pBuf, ulLength);
        ulStatus = pTxDma->intStat;
        pTxDma->intStat = ulStatus;

        pBd->address = (UINT32) VIRT_TO_PHY(pBuf);
        pBd->length = ulLength;
        pBd->status =  DMA_WRAP | DMA_SOP | DMA_EOP | DMA_OWN;

        pTxDma->cfg = DMA_ENABLE;
    }
    else
    {
        /* This should not happen. */
        printk(KERN_DEBUG CARDNAME" Last control endpoint tramsmit has not "
            "completed.\n");
    }
} /* UsbNicSubmitControlPacket */


/*******************************************************************************
*
* NotifyHost
*
* This routine sends 8-byte chunks of data to the Host using the Interrupt
* endpoint.
*
* RETURNS: None.
*/

UINT32 NotifyHost( const UINT32 *pulData )
{
    const UINT32 ulIntrPipeEmpty = 0x80;

    int i;
    UINT32 ulSuccess = 1;

    /* Make sure INT FIFO is empty before continuing. */
    for( i = 0; i < MAX_USB_FIFO_WAIT_CNT &&
        (USB->fifo_status & ulIntrPipeEmpty) == 0; i++ )
        ;

    if( (USB->fifo_status & ulIntrPipeEmpty) != 0 )
    {
        ulSuccess = 0;

        /* Write the 8-byte INT endpoint as 2 32-bit words.
         * The MSB is xmitted first. We don't need an EOP.
         */
        USB->tx_ram_write_port = (UINT32) LE_SWAP4(*pulData);
        pulData++;
        USB->tx_ram_write_port = (UINT32) LE_SWAP4(*pulData);
    }

    return( ulSuccess );
}


/* --------------------------------------------------------------------------
  MACRO to call driver initialization and cleanup functions.
-------------------------------------------------------------------------- */
module_init(bcmusbnic_probe);
module_exit(bcmusbnic_cleanup);
MODULE_LICENSE("Proprietary");
