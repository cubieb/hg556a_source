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
#ifndef _BCMENET_H_
#define _BCMENET_H_

#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <bcm_map.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "boardparms.h"
#include <bcmnet.h>

/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
/* In order for ATM shaping to work correctly,
 * the number of receive BDS/buffers = # tx queues * # buffers per tx queue
 * (80 ENET buffers = 8 tx queues * 10 buffers per tx queue)
 */
#define ENET_MAX_MTU_SIZE       (1528 + 4)    /* Body(1500) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) */
#define DMA_MAX_BURST_LENGTH    16      /* in 32 bit words */
#define RX_ENET_SKB_RESERVE     (176 + 48)
#define SKB_ALIGNED_SIZE        ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define RX_BUF_LEN              ((ENET_MAX_MTU_SIZE + (DMA_MAX_BURST_LENGTH * 4)) & ~((DMA_MAX_BURST_LENGTH * 4)-1))
#define RX_BUF_SIZE             (SKB_DATA_ALIGN(RX_BUF_LEN + RX_ENET_SKB_RESERVE) + sizeof(struct skb_shared_info))

/* misc. configuration */
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        (NR_RX_BDS / 2)
/* IEEE 802.3 Ethernet constant */
#define ETH_CRC_LEN             4
#define ETH_MULTICAST_BIT       0x01

#define CACHE_TO_NONCACHE(x)    KSEG1ADDR(x)
#define NONCACHE_TO_CACHE(x)    KSEG0ADDR(x)

#define ERROR(x)        printk x
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__)); 
#endif

#if defined(DUMP_TRACE)
#define TRACE(x)        printk x
#else
#define TRACE(x)
#endif

#define NUM_CHANS_PER_EMAC          2
#if defined (CONFIG_BCM96348) || defined (CONFIG_BCM96358)
#define MAX_EMACS                   2
#else
#define MAX_EMACS                   1
#endif
#define EMAC1_RX_CHAN               0
#define EMAC1_TX_CHAN               1
#define EMAC2_RX_CHAN               2
#define EMAC2_TX_CHAN               3

typedef struct ethsw_info_t {
    int cid;                            /* Current chip ID */
    int page;                           /* Current page */
    int type;                           /* Ethernet Switch type */
} ethsw_info_t;

typedef struct emac_pm_addr_t {
    BOOL                valid;          /* 1 indicates the corresponding address is valid */
    unsigned int        ref;            /* reference count */
    unsigned char       dAddr[ETH_ALEN];/* perfect match register's destination address */
    char                unused[2];      /* pad */
} emac_pm_addr_t;                    
#define MAX_PMADDR          4           /* # of perfect match address */
/*
 * device context
 */ 

#pragma pack(1)
typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint32 brcm_tag;
} BcmEnet_hdr;
#pragma pack()

/*start of ÒÔÌ«Íø 3.1.2 porting by l39225 20060504 modify
#define NUM_OF_VPORTS         4
*/
#define NUM_OF_VPORTS       5
/*end of ÒÔÌ«Íø 3.1.2 porting by l39225 20060504 modify*/

/*add of support ETHWAN by l39225 20061218*/
/*if ETHWAN  protocol mode ,the max number of wan is 2.
   if ETHWAN  vlan mode ,the max number of wan is 4. */
#define MAX_NUM_WAN        2
/*I think that this is the  right max number of wan*/
//define MAX_NUM_WAN   MAX_PMADDR
/*end of support ETHWAN by l39225 20061218*/


#define BRCM_TAG_LEN        6
#define BRCM_TYPE           0x8874
#define BRCM_TAG_UNICAST    0x00000000
#define BRCM_TAG_MULTICAST  0x20000000
#define BRCM_TAG_EGRESS     0x40000000
#define BRCM_TAG_INGRESS    0x60000000

typedef struct BcmEnet_devctrl {
    struct net_device *dev;             /* ptr to net_device */
    struct net_device *next_dev;        /* next device */
    int             unit;               /* device control index */
    struct timer_list timer;            /* used by Tx reclaim */
    int             linkstatus_polltimer;
    int             linkstatus_phyport;
    int             linkstatus_holder;

    /*start of ÒÔÌ«Íø 3.1.4 porting by l39225 20060504*/
    spinlock_t      lock;  
   /*end of ÒÔÌ«Íø 3.1.4 porting by l39225 20060504*/
    struct net_device_stats stats;      /* statistics used by the kernel */
    IOCTL_MIB_INFO MibInfo;

    volatile EmacRegisters *emac;       /* EMAC register base address */

    volatile DmaRegs *dmaCtrl;          /* EMAC DMA register base address */

    /* transmit variables */
    volatile DmaChannelCfg *txDma;      /* location of transmit DMA register set */
    volatile DmaDesc *txBds;            /* Memory location of tx Dma BD ring */
    struct sk_buff *txSkb[NR_TX_BDS];   /* list of SKBs pending transition */
    int      txFreeBds;                 /* # of free transmit bds */
    int      txHeadIndex;
    int      txTailIndex;

    /* receive variables */
    volatile DmaChannelCfg *rxDma;      /* location of receive DMA register set */
    volatile DmaDesc *rxBds;            /* Memory location of rx bd ring */
    int      rxAssignedBds;             /* # of BDs given to HW */
    int      rxHeadIndex;
    int      rxTailIndex;
    unsigned char   *buf_pool[NR_RX_BDS]; /* rx buffer pool */

    uint16          chipId;             /* chip's id */
    uint16          chipRev;            /* step */
    int             rxIrq;              /* rx dma irq */
    emac_pm_addr_t  pmAddr[MAX_PMADDR]; /* perfect match address */
    struct tasklet_struct task;         /* tasklet */
    int             linkState;          /* link status */
    ethsw_info_t    ethSwitch;          /* external switch */
    ETHERNET_MAC_INFO EnetInfo;
    unsigned int    vid;
    struct sk_buff *freeSkbList;
    unsigned char skbs[(NR_RX_BDS * SKB_ALIGNED_SIZE) + 0x10];
} BcmEnet_devctrl;

// BD macros
#define IncTxBDptr(x, s) if (x == ((BcmEnet_devctrl *)s)->txLastbdPtr) \
                             x = ((BcmEnet_devctrl *)s)->txFirstbdPtr; \
                      else x++

#define IncRxBDptr(x, s) if (x == ((BcmEnet_devctrl *)s)->rxLastBdPtr) \
                             x = ((BcmEnet_devctrl *)s)->rxFirstBdPtr; \
                      else x++

#define CARDNAME    "BCM63xx_ENET"

#endif /* _BCMENET_H_ */

