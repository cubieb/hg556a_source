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
//**************************************************************************
// File Name  : bcmenet.c
//
// Description: This is Linux network driver for the BCM63xx Ethernet MACs
//               
//**************************************************************************
//#define DUMP_DAT
#define     MIRROR_DATE

#define VERSION     "0.3"

#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include <linux/mii.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kmod.h>
#include <linux/rtnetlink.h>
#include <net/arp.h>
#include <board.h>
#include <bcm_common.h>
#include <bcm_map.h>
#include <bcm_intr.h>
#include "bcmenet.h"
#include "bcmmii.h"
#include "ifonmii.h"

#include <linux/stddef.h>

#ifdef USE_PROC
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#define PROC_ENTRY_NAME     "driver/eth%dinfo"
#endif

#include <asm/r4kcache.h>
/* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
#include <linux/dyinggasp.h>
/* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */

#define POLLTIME_100MS      (HZ/10)
#define POLLCNT_1SEC        (HZ/POLLTIME_100MS)
#define POLLCNT_FOREVER     ((int) 0x80000000)

#define EMAC_TX_WATERMARK   32

#define VLAN_DISABLE         0
#define VLAN_ENABLE          1

#define ENET_POLL_DONE      0x80000000

 /*start of 以太网 3.1.1 porting by l39225 20060504*/
 #define PAGE_STATUS         0x01/* PHY Port Status registers */
 #define REG_LS_SUM	          0x00
 /*end of 以太网 3.1.1 porting by l39225 20060504*/


/*start of support Enetwan mirror by l39225 20061218*/
extern int rfc2684InMirrorStatus;
extern int rfc2684OutMirrorStatus;
#define MIRROR_DISABLED              1
#define MIRROR_ENABLED               2
/*end of support Enetwan mirror by l39225 20061218*/


typedef enum {
    TX_BD_RING1 = 0,
    TX_BD_RING2,
    RX_BD_RING,
    DMA_DESC_STATUS,
    BUF_POOL,
    EMAC_REGS,
    MISC_REGS,
    MIB_REGS,
}   DUMP_TYPE;

#define MAKE4(x)   ((x[3] & 0xFF) | ((x[2] & 0xFF) << 8) | ((x[1] & 0xFF) << 16) | ((x[0] & 0xFF) << 24))
#define MAKE2(x)   ((x[1] & 0xFF) | ((x[0] & 0xFF) << 8))

/* Prototypes */
static int bcm63xx_enet_open(struct net_device * dev);
static int bcm63xx_enet_close(struct net_device * dev);
static void bcm63xx_enet_timeout(struct net_device * dev);
static void bcm_set_multicast_list(struct net_device * dev);
static struct net_device_stats * bcm63xx_enet_query(struct net_device * dev);
static void bcm63xx_enet_poll_timer(unsigned long arg);
static int bcm63xx_enet_xmit(struct sk_buff * skb, struct net_device * dev);
static irqreturn_t bcm63xx_enet_isr(int irq, void *, struct pt_regs *regs);
static int bcm63xx_enet_poll(struct net_device * dev, int *budget);
static uint32 bcm63xx_rx(void *ptr, uint32 budget);
static void bcm63xx_enet_free_skb_or_data(void *param, void *obj, int free_flag);
static int bcm_set_mac_addr(struct net_device *dev, void *p);
static void write_mac_address(struct net_device *dev);
static void assign_rx_buffers(BcmEnet_devctrl *pDevCtrl, unsigned char *data);
static void perfectmatch_write(volatile EmacRegisters *emac, int reg, const uint8 * pAddr, bool bValid);
static void perfectmatch_update(BcmEnet_devctrl *pDevCtrl, const uint8 * pAddr, bool bValid);
static void clear_mib(volatile EmacRegisters *emac);
static int init_emac(BcmEnet_devctrl *pDevCtrl);
static void init_dma(BcmEnet_devctrl *pDevCtrl);
static int init_buffers(BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl);
#ifdef USE_PROC
static int bcm63xx_enet_dump(BcmEnet_devctrl *pDevCtrl, char *buf, int reqinfo);
static int dev_proc_engine(void *data,loff_t pos, char *buf);
static ssize_t eth_proc_read(struct file *file, char *buf, size_t count, loff_t *pos);
#endif
int __init bcm63xx_enet_probe(void);
#ifdef DYING_GASP_API
void bcm63xx_dying_gasp_handler(void *context);
#endif
static int bcmIsEnetUp(struct net_device *dev);
static void __exit bcmenet_module_cleanup(void);
static int __init bcmenet_module_init(void);
static int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr);
static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);

static struct net_device *eth_root_dev = NULL;
static struct net_device* vnet_dev[NUM_OF_VPORTS] = {NULL, NULL, NULL, NULL};

/* HUAWEI HGW s48571 2008年1月29日 Hardware porting add begin:*/
static bool isEthUp = FALSE;
/* HUAWEI HGW s48571 2008年1月29日 Hardware porting add end.*/


/*add of support ENETWAN protocol by l39225 20061218*/
#ifdef CONFIG_ETHWAN
static struct net_device_stats *wanDev_query(struct net_device * dev);
static int  bcm63xx_create_enetWan(void);

static struct net_device* wan_net_dev[MAX_NUM_WAN] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static struct net_device* wan_dev = NULL;

#define   LAN_MODE     -1
static  int s_iWanMode = LAN_MODE;

#define WAN_UP                          0xf00
#define DEV_BASE                       0xf000
#define PROTO_PPPOE			0
#define PROTO_MER			1
#endif
/*end of support ENETWAN protocol by L39225 20061218*/

struct net_device *get_vnet_dev(int port);
struct net_device *set_vnet_dev(int port, struct net_device *dev);

EXPORT_SYMBOL_NOVERS(get_vnet_dev);
EXPORT_SYMBOL_NOVERS(set_vnet_dev);

struct net_device* get_vnet_dev(int port)
{
    if ((port < 0) || (port >= NUM_OF_VPORTS))
        return NULL;

    return vnet_dev[port];
}

struct net_device *set_vnet_dev(int port, struct net_device *dev)
{
    if ((port < 0) || (port >= NUM_OF_VPORTS))
        return NULL;

    local_bh_disable();

    if (port == 1)
    {
        if (dev != NULL)
            mii_switch_frame_manage_mode(vnet_dev[0]);
        else
        {
            mii_switch_unmanage_mode(vnet_dev[0]);
            /*start of add lsw port seperate by port_vlan base by l39225 2006-8-15*/
            mii_switch_unPort_vlan_base_mode(vnet_dev[0]);
            /*end of add lsw port seperate by port_vlan base by l39225 2006-8-15*/
        }
    }

    vnet_dev[port] = dev;
    local_bh_enable();
    return dev;
}

#ifdef VPORTS
static inline struct sk_buff *bcm63xx_put_tag(struct sk_buff *skb,struct net_device *dev);
static int bcm63xx_header(struct sk_buff *s, struct net_device *d, unsigned short ty, void *da, void *sa, unsigned len);
#define is_vport(dev) ((vnet_dev[0] != NULL) && (dev->hard_header == vnet_dev[0]->hard_header))
#define is_vmode() (vnet_dev[1] != NULL)
#define egress_vport_id_from_dev(dev) ((dev->base_addr == vnet_dev[0]->base_addr) ? 0: (dev->base_addr - 1))
#define ingress_vport_id_from_tag(p) (((unsigned char *)p)[17] & 0x0f)
/*lyc add in order to support ADM6996M LSW*/
#define ingress_vport_id_from_inf_tag(p) (((unsigned char *)p)[14] & 0x07)
#endif
#define is_ewan(dev) ((wan_dev != NULL) && ((dev->base_addr & DEV_BASE) == DEV_BASE))

#define IsArp(p)      ((p[18] == 0x08) && (p[19] == 0x06))
#define IsBcstAddr(p) ((p[0] == 0xff) && (p[1] == 0xff) && (p[2] == 0xff))
#define IsMultAddr(p) ((p[0] == 0x01) && (p[1] == 0x00) && (p[2] == 0x5e))
#define IsComeBack(p) (((p)[0] == 's') && ((p)[1] == 'w') && ((p)[2] == 's') && ((p)[3] == 'w'))

#ifdef DUMP_DATA
static void dumpHexData(unsigned char *head, int len);
/*
 * dumpHexData dump out the hex base binary data
 */
static void dumpHexData(unsigned char *head, int len)
{
    int i;
    unsigned char *curPtr = head;

    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");       
        printk("0x%02X, ", *curPtr++);
    }
    printk("\n");
}
#endif

static inline void cache_wback_inv(unsigned long addr, unsigned long size)
{
    unsigned long end, a;
    unsigned long dc_lsize = current_cpu_data.dcache.linesz;

    a = addr & ~(dc_lsize - 1);
    end = (addr + size - 1) & ~(dc_lsize - 1);
    while (1) {
        flush_dcache_line(a);   /* Hit_Writeback_Inv_D */
        if (a == end)
            break;
        a += dc_lsize;
    }
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_open
 Purpose: Open and Initialize the EMAC on the chip
-------------------------------------------------------------------------- */
static int bcm63xx_enet_open(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

#ifdef VPORTS
    if (is_vport(dev) && (dev != vnet_dev[0]))
    {
        if ((vnet_dev[0]->flags & IFF_UP) == 0)
            return -ENETDOWN;

        netif_start_queue(dev);
        return 0;
    }
#endif

/*add of support ETHWAN protocol by 139225 20061218*/
#ifdef CONFIG_ETHWAN
   #ifdef CONFIG_LSWWAN
         if( is_ewan(dev) && dev  != vnet_dev[LSW_WPORT])
	  {
	         if ((vnet_dev[0]->flags & IFF_UP) == 0)
	    	 {
	    	        return -ENETDOWN;
	    	  }

	        netif_start_queue(dev);
	        return 0; 
	   }
   #else
	   if( is_ewan(dev) && dev  != wan_dev)
	   {
	         if ((wan_dev->flags & IFF_UP) == 0)
	    	 {
	    	        return -ENETDOWN;
	    	  }

	        netif_start_queue(dev);
	        return 0; 
	   }
   #endif
#endif
/*end of support ENETWAN protocol by l39225 200612118*/

    ASSERT(pDevCtrl != NULL);

    TRACE(("%s: bcm63xx_enet_open\n", dev->name));

    pDevCtrl->emac->config |= EMAC_ENABLE;           

    pDevCtrl->dmaCtrl->controller_cfg |= DMA_MASTER_EN;

    pDevCtrl->rxDma->cfg |= DMA_ENABLE;

    BcmHalInterruptEnable(pDevCtrl->rxIrq);

    /* Start the network engine */
    netif_start_queue(dev);
    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_close
 Purpose: Stop communicating with the outside world
    Note: Caused by 'ifconfig ethX down'
-------------------------------------------------------------------------- */
static int bcm63xx_enet_close(struct net_device * dev)
{
    int i;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

#ifdef VPORTS
    if (is_vport(dev) && (dev != vnet_dev[0]))
    {
        //netif_stop_queue(dev);
        return 0;
    }
#endif

/*add of support ENETWAN protocol by l39225 20061218*/
#ifdef CONFIG_ETHWAN
    #ifdef CONFIG_LSWWAN
	   if( is_ewan(dev) && dev  != vnet_dev[LSW_WPORT])
	   {
	    	   //netif_start_queue(dev);
                 return 0; 
	    }
    #else
	    if( is_ewan(dev) && dev  != wan_dev)
	    {
	        //netif_start_queue(dev);
	        return 0; 
	    }
    #endif
#endif
/*end of support ENETWAN protocol by l39225 20061218*/

    ASSERT(pDevCtrl != NULL);

    TRACE(("%s: bcm63xx_enet_close\n", dev->name));

    netif_stop_queue(dev);

    BcmHalInterruptDisable(pDevCtrl->rxIrq);
   
    pDevCtrl->rxDma->cfg &= ~DMA_ENABLE;
    // wait the current packet to complete before turning off EMAC, otherwise memory corruption can occur.
    for(i=0; pDevCtrl->rxDma->cfg & DMA_ENABLE; i++) {
        // put the line below inside - it seems the signal can be lost and DMA never stops
        pDevCtrl->rxDma->cfg &= ~DMA_ENABLE;
        if (i >= 100) { break; }
        udelay(20);
    }

    pDevCtrl->emac->config |= EMAC_DISABLE;
    // wait the shut down process to complete. EMAC registers should't be modified before that.
    for(i=0; pDevCtrl->emac->config & EMAC_DISABLE; i++) {
        if (i >= 100) { break; }
        udelay(20);
    }

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_timeout
 Purpose: 
-------------------------------------------------------------------------- */
static void bcm63xx_enet_timeout(struct net_device * dev)
{
    ASSERT(dev != NULL);

    TRACE(("%s: bcm63xx_enet_timeout\n", dev->name));

    dev->trans_start = jiffies;

    netif_wake_queue(dev);
}

/* --------------------------------------------------------------------------
    Name: bcm_set_multicast_list
 Purpose: Set the multicast mode, ie. promiscuous or multicast
-------------------------------------------------------------------------- */
static void bcm_set_multicast_list(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#ifdef VPORTS
    static int num_of_promisc_vports = 0;

    if (is_vport(dev))
        pDevCtrl = netdev_priv(vnet_dev[0]);
#endif

/*add of support ETHWAN by l39225 200061218*/
#ifdef CONFIG_ETHWAN
     if(is_ewan(dev))
     {
     		  pDevCtrl = netdev_priv(wan_dev);
     }
#endif
/*end of support ETHWAN by l39225 20061218*/

    TRACE(("%s: bcm_set_multicast_list: %08X\n", dev->name, dev->flags));

    /* Promiscous mode */
    if (dev->flags & IFF_PROMISC) {
#ifdef VPORTS
        if (is_vport(dev))
            num_of_promisc_vports ++;
#endif
        pDevCtrl->emac->rxControl |= EMAC_PROM;
    } else {
#ifdef VPORTS
        if (is_vport(dev)) {
            if (-- num_of_promisc_vports == 0)
                pDevCtrl->emac->rxControl &= ~EMAC_PROM;
        } 
        else 
#endif
        {
            pDevCtrl->emac->rxControl &= ~EMAC_PROM;
        }
    }

    /* All Multicast packets */
    if (dev->flags & IFF_ALLMULTI) {
        pDevCtrl->emac->rxControl |= EMAC_ALL_MCAST;
    } else {
        pDevCtrl->emac->rxControl &= ~EMAC_ALL_MCAST;
    }
}

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_query
 Purpose: Return the current statistics. This may be called with the card 
          open or closed.
-------------------------------------------------------------------------- */
static struct net_device_stats *
bcm63xx_enet_query(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

    return &(pDevCtrl->stats);
}

/*
 * bcm63xx_enet_poll_timer: reclaim transmit frames which have been sent out
 */
static void bcm63xx_enet_poll_timer(unsigned long arg)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)arg;
    int linkState;
    unsigned long ulVal = 0;
    //unsigned long ulPval = 0;

    pDevCtrl->linkstatus_polltimer++;
    /* Reclaim transmit Frames which have been sent out */
    bcm63xx_enet_xmit(NULL, pDevCtrl->dev);
 
    if ( pDevCtrl->linkstatus_polltimer >= POLLCNT_1SEC ) {
    	 /*start of 以太网 3.1.3 porting by l39225 20060504*/
    	// pDevCtrl->linkstatus_polltimer = 0;
    	 /*end of 以太网 3.1.3 porting by l39225 20060504*/
        linkState = bcmIsEnetUp(pDevCtrl->dev);

        if (linkState != pDevCtrl->linkState) {
            if (linkState != 0) {
                pDevCtrl->MibInfo.ulIfLastChange = (jiffies * 100) / HZ;

                if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH) {
                    pDevCtrl->MibInfo.ulIfSpeed = SPEED_100MBIT;
                    if (pDevCtrl->linkState == 0)
                    {
                        pDevCtrl->emac->txControl |= EMAC_FD;
                        pDevCtrl->dmaCtrl->flowctl_ch3_alloc = (DMA_BUF_ALLOC_FORCE | 0);
                        pDevCtrl->dmaCtrl->flowctl_ch3_alloc = NR_RX_BDS;
                        pDevCtrl->rxDma->cfg |= DMA_ENABLE;                    
                    }
                } else {
                    unsigned long v = mii_read(pDevCtrl->dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_AUX_CTRL_STATUS);
                    if( (v & MII_AUX_CTRL_STATUS_SP100_INDICATOR) == MII_AUX_CTRL_STATUS_SP100_INDICATOR )
                        pDevCtrl->MibInfo.ulIfSpeed = SPEED_100MBIT;
                    else
                        pDevCtrl->MibInfo.ulIfSpeed = SPEED_10MBIT;
                    
                    /*start of 以太网 3.1.5 porting by l39225 20060504*/
                   // ulPval = mii_read(pDevCtrl->dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_LPA);
                    ulVal = mii_read(pDevCtrl->dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR);
                    if( (v & SP100_INDICATOR) == SP100_INDICATOR)
                    {
                        ulVal |= MII_CR_SPEED_100M;
                    }
                    else
	             {
	                 ulVal &= ~MII_CR_SPEED_100M;
                    }

                   if ((v & FULLDUPLEX_INDICATOR) == FULLDUPLEX_INDICATOR)
                   {
                        ulVal |= MII_CR_FD_MODE;
                    }	
                    else
                   {
                       ulVal &= ~MII_CR_FD_MODE;
                    }

                    mii_write(pDevCtrl->dev, pDevCtrl->EnetInfo.ucPhyAddress, MII_BMCR,  ulVal);
                   /*start of 以太网 3.1.5 porting by l39225 20060504*/
                    
                  if( (v & MII_AUX_CTRL_STATUS_FULL_DUPLEX) != 0) {
                        pDevCtrl->emac->txControl |= EMAC_FD;
                        pDevCtrl->dmaCtrl->flowctl_ch1_alloc = (DMA_BUF_ALLOC_FORCE | 0);
                        pDevCtrl->dmaCtrl->flowctl_ch1_alloc = NR_RX_BDS;
                        pDevCtrl->rxDma->cfg |= DMA_ENABLE;
                    }
                    else
                        pDevCtrl->emac->txControl &= ~EMAC_FD;

#if defined (CONFIG_BCM96338)
                    phy_tuning_mse(pDevCtrl->dev);
#endif
                }
                /* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日
                if (pDevCtrl->linkState == 0)
                    printk((KERN_CRIT "%s Link UP.\n"),pDevCtrl->dev->name);
                */
                if (pDevCtrl->linkState == 0)
                {
                    printk((KERN_CRIT "%s Link UP.\n"),pDevCtrl->dev->name);
                    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
                    isEthUp = TRUE;
                    kerSysLedCtrl(kLedInternet,kLedStateOn);
                    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
                    dg_recordIntfState(pDevCtrl->dev->name, DG_ITF_UP);
                }
                /* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
            } else {
                pDevCtrl->MibInfo.ulIfLastChange = 0;
                pDevCtrl->MibInfo.ulIfSpeed = 0;

                if (pDevCtrl->linkState != 0)
                {
                    pDevCtrl->rxDma->cfg &= ~DMA_ENABLE;
                    printk((KERN_CRIT "%s Link DOWN.\n"),pDevCtrl->dev->name);
                    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
                    isEthUp = FALSE;
                    kerSysLedCtrl(kLedInternet,kLedStateOff);
                    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
                    /* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
                    dg_recordIntfState(pDevCtrl->dev->name, DG_ITF_DOWN);
                    /* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
                }
            }

            /* Wake up the user mode application that monitors link status. */
            pDevCtrl->linkState = linkState;
            /* HUAWEI VHG WangXinfeng 69233 2008-05-22 Add begin */
            /* Add: Support diagnose ethernet port link state requrement */
            kerSysSetLswLinkState(linkState);
            /* HUAWEI VHG WangXinfeng 69233 2008-05-22 Add begin */
            kerSysWakeupMonitorTask();
        }
    }

    pDevCtrl->timer.expires = jiffies + POLLTIME_100MS;
    add_timer(&pDevCtrl->timer);
}

#ifdef VPORTS

static inline struct sk_buff *bcm63xx_put_tag(struct sk_buff *skb,struct net_device *dev)
{
    BcmEnet_hdr *pHdr = (BcmEnet_hdr *)skb->data;
    int headroom;
    int tailroom;

    /*start of support ADM9669M LSW by l39225*/
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    /*end of support ADM9669M LSW by l39225*/

    if (pHdr->brcm_type == BRCM_TYPE)
    {
        headroom = 0;
        tailroom = ETH_ZLEN + BRCM_TAG_LEN - skb->len;
    }
    else
    {
        headroom = BRCM_TAG_LEN;
        tailroom = ETH_ZLEN - skb->len;
    }

    /*add of support ADM6996M LSW by l39225 20061218*/
   if( ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
    {
           tailroom = (tailroom < 0)? 0:  tailroom ;
    }
    /*end of support ADM6996M LSW by l39225 20061218*/
    else
    {
	    tailroom = (tailroom < 0)? ETH_CRC_LEN: ETH_CRC_LEN + tailroom ;
    }

    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
    {
        struct sk_buff *oskb = skb;

        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);

        if (!skb)
            return NULL;
    }
    else if (headroom != 0)
    {
        skb = skb_unshare(skb, GFP_ATOMIC);

        if (!skb)
            return NULL;
    }

    memset(skb->data + skb->len, 0, tailroom);
    skb_put(skb, tailroom);

    if (headroom != 0)
    {
        BcmEnet_hdr *pHdr = (BcmEnet_hdr *)skb_push(skb, headroom);
        memmove(pHdr, skb->data + headroom, ETH_ALEN * 2);
        pHdr->brcm_type = BRCM_TYPE;

        /*add of support ADM6996M LSW by l39225 20061218*/
       if( ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
        {
        	  pHdr->brcm_tag = BRCM_TAG_EGRESS ;
        	  pHdr->brcm_tag  |= (1 << egress_vport_id_from_dev(dev) ) << 24;
        }
         /*end of support ADM6996M LSW by l39225 20061218*/
        else 
        {
		 pHdr->brcm_tag  = ((skb->len - BRCM_TAG_LEN) << 16);
		/* 
		 * start of y42304 delete 20060811: 解决问题:
		 * 两个网口不在同一个vlan，其中一个pvid是管理vlan，可以访问http，另一个不能访问将两个网口先全部拔下，然后再插上，
		 * 将可以访问网关的那个网口连接的pc的arp执行 ：arp -d然后访问网关http或者是ping网关都不通，不能自动恢复。如果将
		 * 另外一条网线拔下，执行过arp -d的那台pc自动接通网关
		 */
		#if 0        
		        if (IsArp(skb->data))
		            return skb;
		#endif           

		   #ifdef CONFIG_KPN_VLAN
		       /* start of y42304 added 20060831: HG510v/HG510vi VLAN brigde 禁用mac地址学习 */
		       if ((skb->nfmark & BCM_TAG_NFMARK) == BCM_TAG_NFMARK)
		       {
		            skb->nfmark &= ~BCM_TAG_NFMARK;
		                       
		            if (vnet_dev[1] != NULL)
		            {
		                pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
		            }            
		       }   
		       else
		       {
		           if(IsMultAddr(skb->data) || IsBcstAddr(skb->data))
		           {            
		               if (vnet_dev[1] != NULL)
		               {
		                   pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
		               }
		           }
		           //lyc add mirror packets
		           if((skb->nfmark  &  BCM_MIRROR_PACKETS) == BCM_MIRROR_PACKETS)
		            {
		                   skb->nfmark &= ~BCM_MIRROR_PACKETS;

		                    if (vnet_dev[1] != NULL)
		                    {
		                          pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
		                    } 
		            }

		           
		            
		       }
		       /* end of y42304 added 20060831 */
		   #else

		       /*
		        * start of y42304 added 20060809: 解决问题单A36D02425: HG510v为bridge模式，一个LAN口接PC拨号进行FTP下载，其他LAN口都能收到PC发出的报文
		        * 解决5325E的bug: 在Broadcom 私有的6个字节的tag最后4个字节加上(BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev))
		        * 后,5325E不会学习mac地址, 从而导致问题单A36D02425所描述的现象
		        */
		        if(IsMultAddr(skb->data) || IsBcstAddr(skb->data))
		        {            
		            if (vnet_dev[1] != NULL)
		            {
		                pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
		            }
		        }  


		         //lyc add mirror packets
		           if((skb->nfmark  & BCM_MIRROR_PACKETS) == BCM_MIRROR_PACKETS)
		            {
		                   skb->nfmark &= ~BCM_MIRROR_PACKETS;

		                    if (vnet_dev[1] != NULL)
		                    {
		                          pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
		                    } 
		            }
		    #endif
	    }
    }
    return skb; 
}

static int bcm63xx_header(struct sk_buff *skb, struct net_device *dev, unsigned short type, void *da, void *sa, unsigned len)
{
    struct ethhdr *eth;
    BcmEnet_hdr *pHdr;
    uint32 total_len;
    uint32 header_len;
    /*add of support ADM6996M LSW by l39225 20061218*/
    BcmEnet_devctrl *pDevCtrl;

    if(is_vmode())
    {
    	   pDevCtrl = netdev_priv(vnet_dev[0]);
    }
    /*end of support ADM6996M LSW by l39225 20061218*/
    header_len = is_vmode() ? dev->hard_header_len: ETH_HLEN;
    eth = (struct ethhdr *)skb_push(skb, header_len);

    if (type != ETH_P_802_3)
        eth->h_proto = htons(type);
    else
        eth->h_proto = htons(len);

    if (!is_vmode())
        goto skip_tag;

    memmove(skb->data + 18, skb->data + 12, 2);

    total_len = ((len + ETH_HLEN) < ETH_ZLEN)? (ETH_ZLEN) : (len + ETH_HLEN);
    total_len += ETH_CRC_LEN;

    pHdr = (BcmEnet_hdr *)eth;
    pHdr->brcm_type = BRCM_TYPE;
    
    /*add of support ADM6996M LSW by l39225 20061218*/
    if( ESW_TYPE_ADM6996M != pDevCtrl->ethSwitch.type)
    {
	    pHdr->brcm_tag  = (total_len << 16);

	    if (IsArp(skb->data))
	        goto skip_tag;

	    if (vnet_dev[1] != NULL)
	        pHdr->brcm_tag |= BRCM_TAG_EGRESS | egress_vport_id_from_dev(dev);
    }
    else 
    {
    	    if (vnet_dev[1] != NULL)
    	    {
    	         pHdr->brcm_tag = BRCM_TAG_EGRESS ;
	         pHdr->brcm_tag  |= (1 << egress_vport_id_from_dev(dev) ) << 24;	
    	    }
    }
   /*end of support ADM6996M LSW by l39225 20061218*/
skip_tag:

    if (sa)
        memcpy(eth->h_source, sa, dev->addr_len);
    else
        memcpy(eth->h_source, dev->dev_addr, dev->addr_len);

    if (dev->flags & (IFF_LOOPBACK | IFF_NOARP)) {
        memset(eth->h_dest, 0, dev->addr_len);
        return header_len;
    }

    if (da) {
        memcpy(eth->h_dest, da, dev->addr_len);
        return header_len;
    }
    return (-1 * header_len);
}

#endif

/* --------------------------------------------------------------------------
    Name: bcm63xx_enet_xmit
 Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */

static int bcm63xx_enet_xmit(struct sk_buff * skb, struct net_device * dev)
{
    unsigned long flags; 

    #ifdef MIRROR_DATE
    /*start add of  support Enetewan mirror packets by l39225 20061218 */
    struct sk_buff * pCloneBuf;
    unsigned long ulClone = 0;
    unsigned long ulFlg    = 0;
    /*end add of  support Enetewan mirror packets by l39225 20061218 */
    #endif

    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

#ifdef VPORTS
    if (is_vport(dev))
        pDevCtrl = netdev_priv(vnet_dev[0]);
#endif

/*add of support  ETHWAN by l39225 20061218*/
#ifdef CONFIG_ETHWAN
	if( is_ewan(dev))
	{
		 pDevCtrl = netdev_priv(wan_dev);
	}
#endif
/*end of support ETHWAN by l39225 20061218*/   

    ASSERT(pDevCtrl != NULL);

   /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
   unsigned long ledFlag = FALSE; 
   /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

   /*start of 以太网 3.1.4 porting by l39225 20060504*/
    //local_bh_disable();
    spin_lock_irqsave(&pDevCtrl->lock, flags);            //加锁禁止硬中断
    /*end of 以太网 3.1.4 porting by l39225 20060504*/



/* Finish setting up the received SKB and send it to the kernel */


    /*
     * Obtain exclusive access to transmitter.  This is necessary because
     * we might have more than one stack transmitting at once.
     */
     
    /* Reclaim transmitted buffers */
    while (pDevCtrl->txFreeBds < NR_TX_BDS)  {
        if (pDevCtrl->txBds[pDevCtrl->txHeadIndex].status & DMA_OWN)
            break;
        if (pDevCtrl->txSkb[pDevCtrl->txHeadIndex] != NULL)
            dev_kfree_skb_any(pDevCtrl->txSkb[pDevCtrl->txHeadIndex]);
        if (++pDevCtrl->txHeadIndex == NR_TX_BDS)
            pDevCtrl->txHeadIndex = 0;

        pDevCtrl->txFreeBds++;
    }

    if (skb == NULL) {
        netif_wake_queue(dev);
        /*start of 以太网 3.1.4 porting by l39225 20060504*/
        //__local_bh_enable();
        spin_unlock_irqrestore(&pDevCtrl->lock, flags);
    
        /*end of 以太网 3.1.4 porting by l39225 20060504*/
        return 0;
    }

#ifdef MIRROR_DATE
/*start add of  support Enetewan mirror packets by l39225 20061218 */
#ifdef VPORTS
if((rfc2684OutMirrorStatus == MIRROR_ENABLED)&& ( (skb->nfmark  &  BCM_MIRROR_PACKETS) != BCM_MIRROR_PACKETS)&& (vnet_dev[1] != NULL) )
{
          
          pCloneBuf = skb_clone(skb, GFP_ATOMIC) ;
          if(NULL == pCloneBuf )
          {
          	printk(" skb clone failed!lan no Moirrioring\n");
          }
          else
          {
                
                pCloneBuf->nfmark |= BCM_MIRROR_PACKETS;
          	  ulClone = 1;
          }
}
#endif
/*end add of  support Enetewan mirror packets by l39225 20061218 */
#endif

out:

    /* If we could not queue this packet, free it */
    if (pDevCtrl->txFreeBds == 0) 
    {
        TRACE(("%s: bcm63xx_enet_xmit low on txFreeBds\n", dev->name));
        #ifdef VPORTS
        if (is_vport(dev) && (dev == vnet_dev[0]))
        {
            netif_stop_queue(dev);
        }
        #else 
            netif_stop_queue(dev);
        #endif
        /*start of 以太网 3.1.4 porting by l39225 20060504*/
        //__local_bh_enable();
       spin_unlock_irqrestore(&pDevCtrl->lock, flags);
	/*end of 以太网 3.1.4 porting by l39225 20060504*/

  #ifdef MIRROR_DATE
	 /*start add of  support Enetewan mirror packets by l39225 20061218 */      
       if(ulFlg == 1)
    	 { 
    	       kfree_skb(skb);
    	       ulFlg = 0;
    	       return 0;
    	 }

       if(ulClone == 1)
       {
             kfree_skb(pCloneBuf);
       }
        /*end add of  support Enetewan mirror packets by l39225 20061218 */   
    #endif
        
	return 1;
    }

#ifdef VPORTS
    if (is_vport(dev) && is_vmode())
    {
        if (IsMultAddr(skb->data) || IsBcstAddr(skb->data))
        {
            if ((skb_headroom(skb) >= 6) && IsComeBack(skb->data - 6))
            {
               kfree_skb(skb);

               #ifdef  MIRROR_DATE
               /*start add of  support Enetewan mirror packets by l39225 20061218 */      
               if(ulClone == 1)
               {
                     kfree_skb(pCloneBuf);
               }
                /*end add of  support Enetewan mirror packets by l39225 20061218 */   
                #endif
                
               /*start of 以太网 3.1.4 porting by l39225 20060504*/
              //__local_bh_enable();
              spin_unlock_irqrestore(&pDevCtrl->lock, flags);
          
             /*end of 以太网 3.1.4 porting by l39225 20060504*/
              return 0;
            }
        }
        skb = bcm63xx_put_tag(skb, dev);
    }

    if (skb == NULL)
    {
        /*start of 以太网 3.1.4 porting by l39225 20060504*/
        //__local_bh_enable();
       spin_unlock_irqrestore(&pDevCtrl->lock, flags);
       /*end of 以太网 3.1.4 porting by l39225 20060504*/
       return 0;
    }
#endif

/*add of support ETHWAN by l39225 20061218*/
#ifdef CONFIG_ETHWAN
       #ifdef CONFIG_LSWWAN
	if( is_ewan(dev))
	{
		  skb = bcm63xx_put_tag(skb, vnet_dev[LSW_WPORT]);
		   if (skb == NULL)
		   {
		        /*start of 以太网 3.1.4 porting by l39225 20060504*/
		        //__local_bh_enable();
		       spin_unlock_irqrestore(&pDevCtrl->lock, flags);
		       /*end of 以太网 3.1.4 porting by l39225 20060504*/
		       return 0;
		   }
	}
      #endif
 #endif
/*end of support ETHWAN by l39225 20061218*/

#ifdef DUMP_DATA
    printk("bcm63xx_enet_xmit  : len %d,name  %s", skb->len,dev->name);
    dumpHexData(skb->data,  skb->len <32? skb->len:32);
#endif

    cache_wback_inv((unsigned long)skb->data, skb->len);

    /* Decrement total BD count */
    pDevCtrl->txFreeBds--;

    if (pDevCtrl->txFreeBds == 0) {
        TRACE(("%s: bcm63xx_enet_xmit no transmit queue space -- stopping queues\n", dev->name));
  
        /* start of liuyongchao 2060727:  */
        #ifdef VPORTS
        if (is_vport(dev) && (dev == vnet_dev[0]))
        {
            netif_stop_queue(dev);
        }
        #else 
            netif_stop_queue(dev);
        #endif
        /* end of liuyongchao 2060727:  */
    }

    /* Track sent SKB, so we can release them latter */
    pDevCtrl->txSkb[pDevCtrl->txTailIndex] = skb;

    pDevCtrl->txBds[pDevCtrl->txTailIndex].address = (uint32)VIRT_TO_PHY(skb->data);
    pDevCtrl->txBds[pDevCtrl->txTailIndex].length  = skb->len;

    /* advance BD pointer to next in the chain. */
    if (pDevCtrl->txTailIndex == (NR_TX_BDS - 1)) {
        pDevCtrl->txBds[pDevCtrl->txTailIndex].status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC | DMA_WRAP;
        pDevCtrl->txTailIndex = 0;
    } else {
        pDevCtrl->txBds[pDevCtrl->txTailIndex].status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC;
        pDevCtrl->txTailIndex++;
    }

    /* Enable DMA for this channel */
    pDevCtrl->txDma->cfg = DMA_ENABLE;

    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
    ledFlag = TRUE;
    /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

    /* update stats */
#ifdef VPORTS
    if (is_vport(dev) && (dev->base_addr > 0) && (dev->base_addr < NUM_OF_VPORTS)) {
        struct net_device_stats *s =
            (struct net_device_stats *)netdev_priv(vnet_dev[dev->base_addr]);

        s->tx_bytes += ((skb->len < ETH_ZLEN) ? ETH_ZLEN : skb->len);
        s->tx_bytes += ETH_CRC_LEN;
        s->tx_packets++;
    }
#endif
    pDevCtrl->stats.tx_bytes += ((skb->len < ETH_ZLEN) ? ETH_ZLEN : skb->len);
    pDevCtrl->stats.tx_bytes += ETH_CRC_LEN;
    pDevCtrl->stats.tx_packets++;
    pDevCtrl->dev->trans_start = jiffies;



    #ifdef MIRROR_DATE
    /*start add of  support Enetewan mirror packets by l39225 20061218 */ 
    if((1== ulClone))
    {
    	   ulClone = 0;
    	   skb = pCloneBuf;
    	   dev = vnet_dev[1];
    	   ulFlg = 1;
    	   goto out;
    }
   /*start add of  support Enetewan mirror packets by l39225 20061218 */ 
  #endif

    /*start of 以太网 3.1.4 porting by l39225 20060504*/
    //__local_bh_enable();
    spin_unlock_irqrestore(&pDevCtrl->lock, flags);
    /*end of 以太网 3.1.4 porting by l39225 20060504*/

/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
    if ((ledFlag == TRUE)&&( isEthUp == TRUE))
        kerSysLedCtrl(kLedInternet, kLedStateFastFlickerOnce);
 
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

    return 0;
}

static int bcm63xx_enet_poll(struct net_device * dev, int * budget)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    uint32 work_to_do = min(dev->quota, *budget);
    uint32 work_done;
    uint32 ret_done;

    work_done = bcm63xx_rx(pDevCtrl, work_to_do);
    ret_done = work_done & ENET_POLL_DONE;
    work_done &= ~ENET_POLL_DONE;

    *budget -= work_done;
    dev->quota -= work_done;

    if (work_done < work_to_do && ret_done != ENET_POLL_DONE) {
        /* Did as much as could, but we are not done yet */
        return 1;
    }

    /* We are done */
    netif_rx_complete(dev);

    /* Reschedule if there are more packets on the DMA ring to be received. */
    if( (pDevCtrl->rxBds[pDevCtrl->rxHeadIndex].status & DMA_OWN) == 0 )
        netif_rx_reschedule(pDevCtrl->dev, work_done);

    BcmHalInterruptEnable(pDevCtrl->rxIrq);
    return 0;
}

/*
 * bcm63xx_enet_phy_isr: Acknowledge interrupts and set flag for poll timer to
 *                       check link state.
 */
static irqreturn_t bcm63xx_enet_phy_isr(int irq, void * dev_id, struct pt_regs * regs)
{
    BcmEnet_devctrl *pDevCtrl = dev_id;
    
    pDevCtrl->linkstatus_polltimer = POLLCNT_1SEC;
    return IRQ_HANDLED;
}

/*
 * bcm63xx_enet_isr: Acknowledge interrupt and check if any packets have
 *                  arrived
 */
static irqreturn_t bcm63xx_enet_isr(int irq, void * dev_id, struct pt_regs * regs)
{
    BcmEnet_devctrl *pDevCtrl = dev_id;

    pDevCtrl->rxDma->intStat = DMA_DONE;  // clr interrupt
    netif_rx_schedule(pDevCtrl->dev);

    return IRQ_HANDLED;
}

/*
 *  bcm63xx_rx: Process all received packets.                  
 */
static uint32 bcm63xx_rx(void *ptr, uint32 budget)
{
    BcmEnet_devctrl *pDevCtrl = ptr;
    struct sk_buff *skb;
    /*lyc add  lan MONITOR*/
   struct sk_buff *  pCloneBuf;
    /*lyc  add end*/
    uint16 dmaFlag;
    unsigned char *pBuf;
    int len;
    int vport = 0;
    uint32 rxpktgood = 0;
    uint32 rxpktprocessed = 0;
    uint32 rxpktmax = budget + (budget / 2);
    int brcm_hdr_len = 0;
    int brcm_fcs_len = 0;

      
    /* Loop here for each received packet */
    while(pDevCtrl->rxAssignedBds != 0) {
        dmaFlag = pDevCtrl->rxBds[pDevCtrl->rxHeadIndex].status;
        if (dmaFlag & DMA_OWN) {
            rxpktgood |= ENET_POLL_DONE;
            break;
        }

        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
        if (TRUE == isEthUp )
        {
            kerSysLedCtrl(kLedInternet, kLedStateFastFlickerOnce);
        }
        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

        rxpktprocessed++;
        pBuf = (unsigned char *)(phys_to_virt(pDevCtrl->rxBds[pDevCtrl->rxHeadIndex].address));
        len = pDevCtrl->rxBds[pDevCtrl->rxHeadIndex].length;

        if (++pDevCtrl->rxHeadIndex == NR_RX_BDS)
            pDevCtrl->rxHeadIndex = 0;

        local_bh_disable();
        pDevCtrl->rxAssignedBds--;
        __local_bh_enable();

        if (dmaFlag & (EMAC_CRC_ERROR | EMAC_OV | EMAC_NO | EMAC_LG)) {
            pDevCtrl->stats.rx_errors++;
            assign_rx_buffers(pDevCtrl, pBuf - RX_ENET_SKB_RESERVE);
            if( rxpktprocessed < rxpktmax )
                continue;
            break;
        }
        else
        {
            if (pDevCtrl->freeSkbList == NULL) {
                pDevCtrl->stats.rx_dropped++;
                assign_rx_buffers(pDevCtrl, pBuf - RX_ENET_SKB_RESERVE);
                if( rxpktprocessed < rxpktmax )
                    continue;
                break;
            }
        }

        rxpktgood++;

        skb = pDevCtrl->freeSkbList;
        pDevCtrl->freeSkbList = pDevCtrl->freeSkbList->retfreeq_context;
 

        skb_hdrinit(RX_ENET_SKB_RESERVE, SKB_DATA_ALIGN(ENET_MAX_MTU_SIZE), skb,
            pBuf, (void *) bcm63xx_enet_free_skb_or_data, pDevCtrl, 0);

        vport = 0;   

#ifdef VPORTS
        if (is_vport(pDevCtrl->dev)&& is_vmode()) {
            unsigned char *p;

            /*add of support ADM6996M lsw by l39225 20061218*/
            if(ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
            {
                  vport = ingress_vport_id_from_inf_tag(pBuf) + 1;
            }
            /*end of support ADM6996M lsw by l39225 20061218*/
            else
            {
                   vport = ingress_vport_id_from_tag(pBuf) + 1;
            }
            vport = (vnet_dev[vport] == NULL) ? 0: vport;

            for (p = pBuf + 11; p >= pBuf; p--)
                *(p + 6) = *p;

            pBuf[0] = 's';
            pBuf[1] = 'w';
            pBuf[2] = 's';
            pBuf[3] = 'w';
            brcm_hdr_len = 6;
            brcm_fcs_len = 4;
        }
#endif
        skb_pull(skb, brcm_hdr_len);
        /*add of support ADM6996M lsw by l39225 20061218*/
        if(ESW_TYPE_ADM6996M == pDevCtrl->ethSwitch.type)
         {
                 skb_trim(skb, len - ETH_CRC_LEN - brcm_hdr_len);
         }
        else 
        {
        	   skb_trim(skb, len - ETH_CRC_LEN - brcm_hdr_len - brcm_fcs_len);
        }
        /*end of support ADM6996M lsw by l39225  20061218*/


#ifdef DUMP_DATA
        printk("bcm63xx_rx : len %d", skb->len);
        dumpHexData(skb->data, skb->len <32 ? skb->len: 32);
#endif


#ifdef MIRROR_DATE
/*start add of  support Enetewan mirror packets by l39225 20061218 */
#ifdef VPORTS
if( (rfc2684InMirrorStatus == MIRROR_ENABLED)&& (vnet_dev[1] != NULL) )
{
          pCloneBuf = skb_clone(skb, GFP_ATOMIC) ;
          if(NULL == pCloneBuf )
          {
          	printk(" skb clone failed!lan no Moirrioring\n");
          }
          else
          {
              pCloneBuf->nfmark |= BCM_MIRROR_PACKETS;
          	  bcm63xx_enet_xmit(pCloneBuf,vnet_dev[1]);
          }
}
#endif
/*end add of  support Enetewan mirror packets by l39225 20061218 */
#endif
/* Finish setting up the received SKB and send it to the kernel */

#ifdef VPORTS
        if ((vport > 0) && (vport < NUM_OF_VPORTS)) {
            struct net_device_stats *s = 
                (struct net_device_stats *)netdev_priv(vnet_dev[vport]);

            skb->dev = vnet_dev[vport];
            skb->protocol = eth_type_trans(skb, vnet_dev[vport]);
 
/*add of support EHTWAN protocol by 139255 20061218*/
 #ifdef CONFIG_ETHWAN
	   #ifdef CONFIG_LSWWAN
	         if((vport == LSW_WPORT) && (s_iWanMode != LAN_MODE))
	          {                 
	                /*oh ,this is very bad!!! modify ?*/
	                // skb->pkt_type = PACKET_HOST;
	                   
					if(  ETH_P_PPP_DISC == skb->protocol ||  ETH_P_PPP_SES == skb->protocol)
					{
						if((wan_net_dev[PROTO_PPPOE]->base_addr & WAN_UP) == WAN_UP)
						{
							skb->dev = wan_net_dev[PROTO_PPPOE];
							/*oh ,this is not must if the wan_net_dev[PROTO_PPPOE] and vnet_dev 
							share the same mac */
							skb->protocol = eth_type_trans(skb,  wan_net_dev[PROTO_PPPOE]);	
						}
					}
					else 
					{
						if((wan_net_dev[PROTO_MER]->base_addr & WAN_UP) == WAN_UP)
						{
							skb->dev = wan_net_dev[PROTO_MER];
							/*oh ,this is not must if the wan_net_dev[PROTO_PPPOE] and vnet_dev 
							share the same mac */
							skb->protocol = eth_type_trans(skb, wan_net_dev[PROTO_MER]);
						}
					}
	             }
			
	     #endif
  #endif
/*end of support ETHWAN protocol by l39225 20061218*/
            s->rx_packets ++;
            s->rx_bytes += len;
        }
        else
#endif
        {
            skb->dev = pDevCtrl->dev;
            skb->protocol = eth_type_trans(skb, pDevCtrl->dev);

   /*add of support ETHWAN by l39225 20061218*/
   #ifdef CONFIG_ETHWAN
        /*LSW 不启用VPORT 无法做WAN 口*/
        #ifndef CONFIG_LSWWAN
	         if((pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY) && (LAN_MODE != s_iWanMode) )
	          {
	                   /*oh ,this is very bad!!! modify ?*/
	                    //skb->pkt_type = PACKET_HOST;
	                    
		            if(  ETH_P_PPP_DISC == skb->protocol ||  ETH_P_PPP_SES == skb->protocol)
		            {
		                   if((wan_net_dev[PROTO_PPPOE]->base_addr & WAN_UP) == WAN_UP)
		                   {
		                        skb->dev = wan_net_dev[PROTO_PPPOE];
		                    	/*oh ,this is not must if the wan_net_dev[PROTO_PPPOE] and vnet_dev 
					            share the same mac */
					            skb->protocol = eth_type_trans(skb,  wan_net_dev[PROTO_PPPOE]);	
		                    }
		            }
		            else 
		            {
		            	  if((wan_net_dev[PROTO_MER]->base_addr & WAN_UP) == WAN_UP)
		                  {
		                          skb->dev = wan_net_dev[PROTO_MER];
		                          /*oh ,this is not must if the wan_net_dev[PROTO_PPPOE] and vnet_dev 
					               share the same mac */
					              skb->protocol = eth_type_trans(skb, wan_net_dev[PROTO_MER]);
		                  }
		            }
	            }
           #endif
     #endif
     /*end of support ETHWAN by l39225 20061218*/
        }


        pDevCtrl->stats.rx_packets++;
        pDevCtrl->stats.rx_bytes += len;

        /* Notify kernel */
        netif_receive_skb(skb);
        if (--budget <= 0)
            break;
    }

    pDevCtrl->dev->last_rx = jiffies;
    return rxpktgood;
}

/*
 * kernel skb free call back
 */
static void bcm63xx_enet_free_skb_or_data(void *param, void *obj, int free_flag)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) param;

    if( free_flag == FREE_SKB ) {
        struct sk_buff *skb = (struct sk_buff *) obj;
        local_bh_disable();
        skb->retfreeq_context = pDevCtrl->freeSkbList;
        pDevCtrl->freeSkbList = skb;
        __local_bh_enable();
    }
    else {
        assign_rx_buffers(pDevCtrl, (unsigned char *) obj);
    }
}

/*
 * Set the hardware MAC address.
 */
static int bcm_set_mac_addr(struct net_device *dev, void *p)
{
     struct sockaddr *addr=p;

    if(netif_running(dev))
        return -EBUSY;

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    write_mac_address(dev);

    return 0;
}

/*
 * write_mac_address: store MAC address into EMAC perfect match registers                   
 */
void write_mac_address(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    volatile uint32 val;
#ifdef VPORTS
    if (is_vport(dev))
        pDevCtrl = netdev_priv(vnet_dev[0]);
#endif

/*add of support ETHWAN by l39225 200061218*/
#ifdef CONFIG_ETHWAN
     if(is_ewan(dev))
     {
     		  pDevCtrl = netdev_priv(wan_dev);
     }
#endif
/*end of support ETHWAN by l39225 20061218*/

    ASSERT(pDevCtrl != NULL);

    val = pDevCtrl->emac->config;
    if (val & EMAC_ENABLE) {
        /* disable ethernet MAC while updating its registers */
        pDevCtrl->emac->config |= EMAC_DISABLE ;           
        while(pDevCtrl->emac->config & EMAC_DISABLE);     
    }

    /* add our MAC address to the perfect match register */
    perfectmatch_update(pDevCtrl, dev->dev_addr, 1);

    if (val & EMAC_ENABLE) {
        pDevCtrl->emac->config = val;
    }
}

/*
 * assign_rx_buffers: Beginning with the first receive buffer descriptor
 *                  used to receive a packet since this function last
 *                  assigned a buffer, reassign buffers to receive
 *                  buffer descriptors, and mark them as EMPTY; available
 *                  to be used again.
 *
 */
static void assign_rx_buffers(BcmEnet_devctrl *pDevCtrl, unsigned char *data)
{
    local_bh_disable();

    cache_wback_inv((unsigned long)data, RX_BUF_SIZE);

    /* assign packet, prepare descriptor, and advance pointer */
    pDevCtrl->rxBds[pDevCtrl->rxTailIndex].address = (uint32)VIRT_TO_PHY(data + RX_ENET_SKB_RESERVE);
    pDevCtrl->rxBds[pDevCtrl->rxTailIndex].length  = RX_BUF_LEN;

    if (pDevCtrl->unit == 0)
        pDevCtrl->dmaCtrl->flowctl_ch1_alloc = 1;
    else
        pDevCtrl->dmaCtrl->flowctl_ch3_alloc = 1;

    if (pDevCtrl->rxTailIndex == (NR_RX_BDS - 1)) {
        pDevCtrl->rxBds[pDevCtrl->rxTailIndex].status = DMA_OWN | DMA_WRAP;
        pDevCtrl->rxTailIndex = 0;
    } else {
        pDevCtrl->rxBds[pDevCtrl->rxTailIndex].status = DMA_OWN;
        pDevCtrl->rxTailIndex++;
    }

    pDevCtrl->rxAssignedBds++;

    pDevCtrl->rxDma->cfg = DMA_ENABLE;

    __local_bh_enable();
}

/*
 * perfectmatch_write: write an address to the EMAC perfect match registers
 */
static void perfectmatch_write(volatile EmacRegisters *emac, int reg, const uint8 * pAddr, bool bValid)
{
    volatile uint32 *pmDataLo;
    volatile uint32 *pmDataHi;
  
    switch (reg) {
    case 0:
        pmDataLo = &emac->pm0DataLo;
        pmDataHi = &emac->pm0DataHi;
        break;
    case 1:
        pmDataLo = &emac->pm1DataLo;
        pmDataHi = &emac->pm1DataHi;
        break;
    case 2:
        pmDataLo = &emac->pm2DataLo;
        pmDataHi = &emac->pm2DataHi;
        break;
    case 3:
        pmDataLo = &emac->pm3DataLo;
        pmDataHi = &emac->pm3DataHi;
        break;
    default:
        return;
    }
    /* Fill DataHi/Lo */
    if (bValid == 1)
        *pmDataLo = MAKE4((pAddr + 2));
    *pmDataHi = MAKE2(pAddr) | (bValid << 16);
}

/*
 * perfectmatch_update: update an address to the EMAC perfect match registers
 */
static void perfectmatch_update(BcmEnet_devctrl *pDevCtrl, const uint8 * pAddr, bool bValid)
{
    int i;
    unsigned int aged_ref;
    int aged_entry;

    /* check if this address is in used */
    for (i = 0; i < MAX_PMADDR; i++) {
        if (pDevCtrl->pmAddr[i].valid == 1) {
            if (memcmp (pDevCtrl->pmAddr[i].dAddr, pAddr, ETH_ALEN) == 0) {
                if (bValid == 0) {
                    /* clear the valid bit in PM register */
                    perfectmatch_write(pDevCtrl->emac, i, pAddr, bValid);
                    /* clear the valid bit in pDevCtrl->pmAddr */
                    pDevCtrl->pmAddr[i].valid = bValid;
                } else {
                    pDevCtrl->pmAddr[i].ref++;
                }
                return;
            }
        }
    }
    if (bValid == 1) {
        for (i = 0; i < MAX_PMADDR; i++) {
            /* find an available entry for write pm address */
            if (pDevCtrl->pmAddr[i].valid == 0) {
                break;
            }
        }
        if (i == MAX_PMADDR) {
            aged_ref = 0xFFFFFFFF;
            aged_entry = 0;
            /* aged out an entry */
            for (i = 0; i < MAX_PMADDR; i++) {
                if (pDevCtrl->pmAddr[i].ref < aged_ref) {
                    aged_ref = pDevCtrl->pmAddr[i].ref;
                    aged_entry = i;
                }
            }
            i = aged_entry;
        }
        /* find a empty entry and add the address */
        perfectmatch_write(pDevCtrl->emac, i, pAddr, bValid);

        /* update the pDevCtrl->pmAddr */
        pDevCtrl->pmAddr[i].valid = bValid;
        memcpy(pDevCtrl->pmAddr[i].dAddr, pAddr, ETH_ALEN);
        pDevCtrl->pmAddr[i].ref = 1;
    }
}

/*
 * clear_mib: Initialize the Ethernet Switch MIB registers
 */
static void clear_mib(volatile EmacRegisters *emac)
{
    int i;
    volatile uint32 *pt;

    pt = (uint32 *)&emac->tx_mib;
    for( i = 0; i < NumEmacTxMibVars; i++ ) {
        *pt++ = 0;
    }

    pt = (uint32 *)&emac->rx_mib;;
    for( i = 0; i < NumEmacRxMibVars; i++ ) {
        *pt++ = 0;
    }
}

/*
 * init_emac: Initializes the Ethernet Switch control registers
 */
static int init_emac(BcmEnet_devctrl *pDevCtrl)
{
    volatile EmacRegisters *emac;
#if defined(CONFIG_BCM96348)
    uint32 gpiomode;
#endif

    TRACE(("bcm63xxenet: init_emac\n"));

#if defined(CONFIG_BCM96348)
    if (pDevCtrl->EnetInfo.ucPhyType != BP_ENET_INTERNAL_PHY) {
        gpiomode = GPIO->GPIOMode;
        gpiomode |= (GROUP3_EXT_MII|GROUP0_EXT_MII); /* External MII */
        if ((pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_1) ||
            (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_2) ||
            (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_3))
            gpiomode &= ~GROUP1_SPI_MASTER;
            gpiomode |= GROUP1_SPI_MASTER;
        GPIO->GPIOMode = gpiomode;
    }
#endif
#if defined(CONFIG_BCM96338)
    SPI->spiClkCfg = SPI_CLK_0_781MHZ;
#endif

    if (pDevCtrl->unit == 0)
        pDevCtrl->emac = EMAC1;
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358)
    else
        pDevCtrl->emac = EMAC2;
#endif

    emac = pDevCtrl->emac;
    /* Initialize the Ethernet Switch MIB registers */
    clear_mib(pDevCtrl->emac);

    /* disable ethernet MAC while updating its registers */
    emac->config = EMAC_DISABLE ;           
    while(emac->config & EMAC_DISABLE);     

    /* issue soft reset, wait for it to complete */
    emac->config = EMAC_SOFT_RESET;
    while (emac->config & EMAC_SOFT_RESET);

    if (mii_init(pDevCtrl->dev))
        return -EFAULT;

    /* Initialize emac registers */
    emac->rxControl = EMAC_FC_EN | EMAC_PROM;   /* allow Promiscuous */

#ifdef MAC_LOOPBACK
    emac->rxControl |= EMAC_LOOPBACK;
#endif
    emac->rxMaxLength = ENET_MAX_MTU_SIZE;
    emac->txMaxLength = ENET_MAX_MTU_SIZE;

    /* tx threshold = abs(63-(0.63*EMAC_DMA_MAX_BURST_LENGTH)) */
    emac->txThreshold = EMAC_TX_WATERMARK;
    emac->mibControl  = EMAC_NO_CLEAR;
    emac->intMask = 0;              /* mask all EMAC interrupts*/

    return 0;
}

/*
 * init_dma: Initialize DMA control register
 */
static void init_dma(BcmEnet_devctrl *pDevCtrl)
{
    DmaStateRam *StateRam;

    TRACE(("bcm63xxenet: init_dma\n"));
    /*
     * clear State RAM
     */
    if (pDevCtrl->unit == 0)
        StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[EMAC1_RX_CHAN];
    else
        StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[EMAC2_RX_CHAN];
    memset(StateRam, 0, sizeof(DmaStateRam) * NUM_CHANS_PER_EMAC);

    /*
     * initialize IUDMA controller register
     */
    if (pDevCtrl->unit == 0) {
        pDevCtrl->dmaCtrl->controller_cfg |= DMA_FLOWC_CH1_EN;
        pDevCtrl->dmaCtrl->flowctl_ch1_thresh_lo = DMA_FC_THRESH_LO;
        pDevCtrl->dmaCtrl->flowctl_ch1_thresh_hi = DMA_FC_THRESH_HI;
        pDevCtrl->dmaCtrl->flowctl_ch1_alloc = (DMA_BUF_ALLOC_FORCE | 0);
    } else {
        pDevCtrl->dmaCtrl->controller_cfg |= DMA_FLOWC_CH3_EN;
        pDevCtrl->dmaCtrl->flowctl_ch3_thresh_lo = DMA_FC_THRESH_LO;
        pDevCtrl->dmaCtrl->flowctl_ch3_thresh_hi = DMA_FC_THRESH_HI;
        pDevCtrl->dmaCtrl->flowctl_ch3_alloc = (DMA_BUF_ALLOC_FORCE | 0);
    }

    /* setup transmit dma register */
    pDevCtrl->txDma->cfg = 0;       /* initialize first (will enable later) */
    pDevCtrl->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
    pDevCtrl->txDma->intMask = 0;   /* mask all ints */
    if (pDevCtrl->unit == 0)
        pDevCtrl->dmaCtrl->stram.s[EMAC1_TX_CHAN].baseDescPtr = 
            (uint32)VIRT_TO_PHY((uint32 *)pDevCtrl->txBds);
    else
        pDevCtrl->dmaCtrl->stram.s[EMAC2_TX_CHAN].baseDescPtr = 
            (uint32)VIRT_TO_PHY((uint32 *)pDevCtrl->txBds);

    /* setup receive dma register */
    pDevCtrl->rxDma->cfg = 0;  // initialize first (will enable later)
    pDevCtrl->rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
    pDevCtrl->rxDma->intMask = 0;   /* mask all ints */
    /* clr any pending interrupts on channel */
    pDevCtrl->rxDma->intStat = DMA_DONE;
    /* set to interrupt on packet complete and no descriptor available */
    pDevCtrl->rxDma->intMask = DMA_DONE;
    if (pDevCtrl->unit == 0)
        pDevCtrl->dmaCtrl->stram.s[EMAC1_RX_CHAN].baseDescPtr = 
            (uint32)VIRT_TO_PHY((uint32 *)pDevCtrl->rxBds);
    else
        pDevCtrl->dmaCtrl->stram.s[EMAC2_RX_CHAN].baseDescPtr = 
            (uint32)VIRT_TO_PHY((uint32 *)pDevCtrl->rxBds);
}

/*
 *  init_buffers: initialize driver's pools of receive buffers
 *  and tranmit headers
 */
static int init_buffers(BcmEnet_devctrl *pDevCtrl)
{
    unsigned char *data;
    int i;

    TRACE(("bcm63xxenet: init_buffers\n"));

    /* allocate recieve buffer pool */
    for (i = 0; i < NR_RX_BDS; i++) {
        data = kmalloc(RX_BUF_SIZE, GFP_KERNEL);
        if (data == NULL) {
            printk(KERN_NOTICE CARDNAME": Low memory.\n");
            break;
        }
        pDevCtrl->buf_pool[i] = data;
        assign_rx_buffers(pDevCtrl, data);
    }

    if (i < NR_RX_BDS) {
        /* release allocated receive buffer memory */
        for (i = 0; i < NR_RX_BDS; i++) {
            if (pDevCtrl->buf_pool[i]) {
                kfree(pDevCtrl->buf_pool[i]);
                pDevCtrl->buf_pool[i] = NULL;
            }
        }
        return -ENOMEM;
    }
    return 0;
}

/*
 * bcm63xx_init_dev: initialize Ethernet MACs,
 * allocate Tx/Rx buffer descriptors pool, Tx header pool.
 */
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl)
{
    void *p;
    int i;
    unsigned char *skb;

    TRACE(("bcm63xxenet: bcm63xx_init_dev\n"));

    /* init rx/tx dma channels */
    pDevCtrl->dmaCtrl = (DmaRegs *)(EMAC_DMA_BASE);
    if (pDevCtrl->unit == 0) {
        pDevCtrl->rxDma = &pDevCtrl->dmaCtrl->chcfg[EMAC1_RX_CHAN];
        pDevCtrl->txDma = &pDevCtrl->dmaCtrl->chcfg[EMAC1_TX_CHAN];
        pDevCtrl->rxIrq = INTERRUPT_ID_EMAC1_RX_DMA;
    } else {
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358)
        pDevCtrl->rxDma = &pDevCtrl->dmaCtrl->chcfg[EMAC2_RX_CHAN];
        pDevCtrl->txDma = &pDevCtrl->dmaCtrl->chcfg[EMAC2_TX_CHAN];
        pDevCtrl->rxIrq = INTERRUPT_ID_EMAC2_RX_DMA;
#endif
    }
    BcmHalInterruptDisable(pDevCtrl->rxIrq);

    /* register the interrupt service handler */
    BcmHalMapInterrupt(bcm63xx_enet_isr, (unsigned int)pDevCtrl, pDevCtrl->rxIrq);

    if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY) {
        BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
        BcmHalMapInterrupt(bcm63xx_enet_phy_isr, (unsigned int)pDevCtrl,
            INTERRUPT_ID_EPHY);
    }

    /* setup buffer/pointer relationships here */
    /* allocate and assign tx buffer descriptors */
    if (!(p = kmalloc(NR_TX_BDS*sizeof(DmaDesc), GFP_KERNEL))) {
        return -ENOMEM;
    }
    memset(p, 0, NR_TX_BDS*sizeof(DmaDesc));
    cache_wback_inv((unsigned long)p, NR_TX_BDS*sizeof(DmaDesc));
    pDevCtrl->txBds = (DmaDesc *)CACHE_TO_NONCACHE(p);   /* tx bd ring */

    /* alloc space for the rx buffer descriptors */
    if (!(p = kmalloc(NR_RX_BDS*sizeof(DmaDesc), GFP_KERNEL))) {
        kfree((void *)NONCACHE_TO_CACHE(pDevCtrl->txBds));
        return -ENOMEM;
    }
    memset(p, 0, NR_RX_BDS*sizeof(DmaDesc));
    cache_wback_inv((unsigned long)p, NR_RX_BDS*sizeof(DmaDesc));
    pDevCtrl->rxBds = (DmaDesc *)CACHE_TO_NONCACHE(p);   /* rx bd ring */

    pDevCtrl->rxAssignedBds = 0;
    pDevCtrl->rxHeadIndex = pDevCtrl->rxTailIndex = 0;

    pDevCtrl->txFreeBds = NR_TX_BDS;
    pDevCtrl->txHeadIndex = pDevCtrl->txTailIndex = 0;

    /* init dma registers */
    init_dma(pDevCtrl);

    /* initialize the receive buffers and transmit headers */
    if (init_buffers(pDevCtrl)) {
        kfree((void *)NONCACHE_TO_CACHE(pDevCtrl->txBds));
        kfree((void *)NONCACHE_TO_CACHE(pDevCtrl->rxBds));
        return -ENOMEM;
    }

    /* Chain socket buffers. */
    for(i = 0, skb = (unsigned char *)
        (((unsigned long) pDevCtrl->skbs + 0x0f) & ~0x0f); i < NR_RX_BDS;
        i++, skb += SKB_ALIGNED_SIZE)
    {
        ((struct sk_buff *) skb)->retfreeq_context = pDevCtrl->freeSkbList;
        pDevCtrl->freeSkbList = (struct sk_buff *) skb;
    }

    /* init switch control registers */
    if (init_emac(pDevCtrl))
        return -EFAULT;

    /* if we reach this point, we've init'ed successfully */
    return 0;
}

/* Uninitialize tx/rx buffer descriptor pools */
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl)
{
    char proc_name[32];
    int i;

    if (pDevCtrl) {
        if (pDevCtrl->timer.function != NULL)
            del_timer_sync(&pDevCtrl->timer);

        /* disable DMA */
        pDevCtrl->txDma->cfg = 0;
        pDevCtrl->rxDma->cfg = 0;
        if (pDevCtrl->unit == 0)
            pDevCtrl->dmaCtrl->flowctl_ch1_alloc = (DMA_BUF_ALLOC_FORCE | 0);
        else
            pDevCtrl->dmaCtrl->flowctl_ch3_alloc = (DMA_BUF_ALLOC_FORCE | 0);

       /* free the irq */
        if (pDevCtrl->rxIrq) {
            BcmHalInterruptDisable(pDevCtrl->rxIrq);
            free_irq(pDevCtrl->rxIrq, pDevCtrl);
        }
        if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY) {
            BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
            free_irq(INTERRUPT_ID_EPHY, pDevCtrl);
        }
        /* free the skbs */
        while (pDevCtrl->txFreeBds < NR_TX_BDS)  {
            pDevCtrl->txFreeBds++;
            dev_kfree_skb_any (pDevCtrl->txSkb[pDevCtrl->txHeadIndex++]);
            if (pDevCtrl->txHeadIndex == NR_TX_BDS)
                pDevCtrl->txHeadIndex = 0;
        }
        /* release allocated receive buffer memory */
        for (i = 0; i < NR_RX_BDS; i++) {
            if (pDevCtrl->buf_pool[i]) {
                kfree(pDevCtrl->buf_pool[i]);
                pDevCtrl->buf_pool[i] = NULL;
            }
        }
        /* free the transmit buffer descriptor */
        if (pDevCtrl->txBds)
            kfree((void *)NONCACHE_TO_CACHE(pDevCtrl->txBds));
        /* free the receive buffer descriptor */
        if (pDevCtrl->rxBds)
            kfree((void *)NONCACHE_TO_CACHE(pDevCtrl->rxBds));
#ifdef USE_PROC
        sprintf(proc_name, PROC_ENTRY_NAME, pDevCtrl->unit);
        remove_proc_entry(proc_name, NULL);
#endif
        if (pDevCtrl->dev) {
            if (pDevCtrl->dev->reg_state != NETREG_UNINITIALIZED)
                unregister_netdev(pDevCtrl->dev);
            free_netdev(pDevCtrl->dev);
        }
    }
   
    return 0;
}

#ifdef USE_PROC
/*
 * bcm63xx_enet_dump - display EMAC information
 */
static int bcm63xx_enet_dump(BcmEnet_devctrl *pDevCtrl, char *buf, int reqinfo)
{
    volatile EmacRegisters *emac;
    DmaStateRam *StateRam;
    int val;
    int i;
    int len = 0;
    int bufcnt;

    switch (reqinfo) {
    case TX_BD_RING1: /* tx DMA BD descriptor ring */
        if (pDevCtrl->unit == 0)
            len += sprintf(&buf[len], "\nEMAC1\n");
        else
            len += sprintf(&buf[len], "\nEMAC2\n");
        len += sprintf(&buf[len], "\ntx buffer descriptor ring status.\n");
        len += sprintf(&buf[len], "BD\tlocation\tlength\tstatus\n");
        for (i = 0; i < NR_TX_BDS/2; i++) {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   pDevCtrl->txBds[i].length,
                   pDevCtrl->txBds[i].status);
        }
        break;

    case TX_BD_RING2: /* tx DMA BD descriptor ring */
        for (i = NR_TX_BDS/2; i < NR_TX_BDS; i++) {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   pDevCtrl->txBds[i].length,
                   pDevCtrl->txBds[i].status);
        }
        break;

    case RX_BD_RING: /* rx DMA BD descriptor ring */
        len += sprintf(&buf[len], "\nrx buffer descriptor ring status.\n");
        len += sprintf(&buf[len], "BD\tlocation\tlength\tstatus\n");
        for (i = 0; i < NR_RX_BDS; i++) {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   pDevCtrl->rxBds[i].length,
                   pDevCtrl->rxBds[i].status);
        }
        break;

    case DMA_DESC_STATUS:  /* DMA descriptors pointer and status */
        len += sprintf(&buf[len], "\nrx pointers:\n");
        len += sprintf(&buf[len], "DmaDesc *rxBds:\t\t\t%08x\n", (int) pDevCtrl->rxBds);
        len += sprintf(&buf[len], "DmaDesc *rxHeadIndex:\t\t%d\n", pDevCtrl->rxHeadIndex);
        len += sprintf(&buf[len], "DmaDesc *rxTailIndex:\t\t%d\n", pDevCtrl->rxTailIndex);
        len += sprintf(&buf[len], "rxAssignedBds (RxBDs %d):\t\t%d\n", NR_RX_BDS, pDevCtrl->rxAssignedBds);

        len += sprintf(&buf[len], "\ntx pointers:\n");
        len += sprintf(&buf[len], "DmaDesc *txBds:\t\t\t%08x\n", (int)pDevCtrl->txBds);
        len += sprintf(&buf[len], "DmaDesc *txHeadIndex:\t\t%d\n", pDevCtrl->txHeadIndex);
        len += sprintf(&buf[len], "DmaDesc *txTailIndex:\t\t%d\n", pDevCtrl->txTailIndex);
        len += sprintf(&buf[len], "txFreeBds (TxBDs %d):\t\t%d\n", NR_TX_BDS, pDevCtrl->txFreeBds);

        len += sprintf(&buf[len], "\ntx DMA Channel Config\t\t%08lx\n", pDevCtrl->txDma->cfg);
        len += sprintf(&buf[len], "tx DMA Intr Control/Status\t%08lx\n", pDevCtrl->txDma->intStat);
        len += sprintf(&buf[len], "tx DMA Intr Mask\t\t%08lx\n", pDevCtrl->txDma->intMask);

        len += sprintf(&buf[len], "\nrx DMA Channel Config\t\t%08lx\n", pDevCtrl->rxDma->cfg);
        len += sprintf(&buf[len], "rx DMA Intr Control/Status\t%08lx\n", pDevCtrl->rxDma->intStat);
        len += sprintf(&buf[len], "rx DMA Intr Mask\t\t%08lx\n", pDevCtrl->rxDma->intMask);
        if (pDevCtrl->unit == 0) {
            len += sprintf(&buf[len], "rx DMA Controller Configuration\t%08lx\n", pDevCtrl->dmaCtrl->controller_cfg);
            len += sprintf(&buf[len], "rx DMA Low Threshold\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch1_thresh_lo);
            len += sprintf(&buf[len], "rx DMA High Threshold\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch1_thresh_hi);
            len += sprintf(&buf[len], "rx DMA Buffer Alloc\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch1_alloc);
        } else {
            len += sprintf(&buf[len], "rx DMA Controller Configuration\t%08lx\n", pDevCtrl->dmaCtrl->controller_cfg);
            len += sprintf(&buf[len], "rx DMA Low Threshold\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch3_thresh_lo);
            len += sprintf(&buf[len], "rx DMA High Threshold\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch3_thresh_hi);
            len += sprintf(&buf[len], "rx DMA Buffer Alloc\t\t%ld\n", pDevCtrl->dmaCtrl->flowctl_ch3_alloc);
        }
        break;

    case BUF_POOL: /* buffer usage */
        bufcnt = 0;
        len += sprintf(&buf[len], "\nbuf\taddress\n");
        for (i = 0; i < NR_RX_BDS; i++) {
            len += sprintf(&buf[len], "%d\t%08x\n", i, (int)pDevCtrl->buf_pool[i]);
        }
        break;

    case EMAC_REGS: /* EMAC registers */
        emac = pDevCtrl->emac;
        len += sprintf(&buf[len], "\nEMAC registers\n");
        len += sprintf(&buf[len], "rx config register\t0x%08lx\n", emac->rxControl);
        len += sprintf(&buf[len], "rx max length register\t0x%08lx\n", emac->rxMaxLength);
        len += sprintf(&buf[len], "tx max length register\t0x%08lx\n", emac->txMaxLength);
        len += sprintf(&buf[len], "interrupt mask register\t0x%08lx\n", emac->intMask);
        len += sprintf(&buf[len], "interrupt register\t0x%08lx\n", emac->intStatus);
        len += sprintf(&buf[len], "control register\t0x%08lx\n", emac->config);
        len += sprintf(&buf[len], "tx control register\t0x%08lx\n", emac->txControl);
        len += sprintf(&buf[len], "tx watermark register\t0x%08lx\n", emac->txThreshold);
        break;

    case MISC_REGS: /* misc registers */
        if (pDevCtrl->unit == 0)
            StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[EMAC1_RX_CHAN];
        else
            StateRam = (DmaStateRam *)&pDevCtrl->dmaCtrl->stram.s[EMAC2_RX_CHAN];
        len += sprintf(&buf[len], "ring start address\t0x%08lx\n", StateRam->baseDescPtr);
        len += sprintf(&buf[len], "state/bytes offset\t0x%08lx\n", StateRam->state_data);
        len += sprintf(&buf[len], "BD status and len\t0x%08lx\n", StateRam->desc_len_status);
        len += sprintf(&buf[len], "BD current processing\t0x%08lx\n", StateRam->desc_base_bufptr);
        val = (read_c0_cause() & CAUSEF_IP);
        len += sprintf(&buf[len], "CP0 cause\t\t0x%08x\n", val);
        val = read_c0_status();
        len += sprintf(&buf[len], "CP0 status\t\t0x%08x\n", val);
        len += sprintf(&buf[len], "PERF->IrqMask\t\t0x%08lx\n", PERF->IrqMask);
        len += sprintf(&buf[len], "PERF->IrqStatus\t\t0x%08lx\n", PERF->IrqStatus);
        break;

    case MIB_REGS: /* MIB regsisters */
        if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH)
        {
            len += mii_dump_page(pDevCtrl->dev, 0x00, &buf[len]);
            len += mii_dump_page(pDevCtrl->dev, 0x34, &buf[len]);
        }
        break;

    default:
        break;
    }

    return len;
}

static int dev_proc_engine(void *data,loff_t pos, char *buf)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)data;
    int len = 0;

    if (pDevCtrl == NULL)
        return len;

    switch((int)pos) {
    case TX_BD_RING1: /* tx DMA BD descriptor ring */
        len += bcm63xx_enet_dump(pDevCtrl, buf, TX_BD_RING1);
        break;
    case TX_BD_RING2: /* tx DMA BD descriptor ring */
        len += bcm63xx_enet_dump(pDevCtrl, buf, TX_BD_RING2);
        break;
    case RX_BD_RING: /* rx DMA BD descriptor ring */
        len += bcm63xx_enet_dump(pDevCtrl, buf, RX_BD_RING);
        break;
    case DMA_DESC_STATUS: /* DMA descriptors pointer and status */
        len += bcm63xx_enet_dump(pDevCtrl, buf, DMA_DESC_STATUS);
        break;
    case BUF_POOL: /* buffer usage */
        len += bcm63xx_enet_dump(pDevCtrl, buf, BUF_POOL);
        break;
    case EMAC_REGS: /* EMAC registers */
        len += bcm63xx_enet_dump(pDevCtrl, buf, EMAC_REGS);
        break;
    case MISC_REGS: /* misc registers */
        len += bcm63xx_enet_dump(pDevCtrl, buf, MISC_REGS);
        break;
    case MIB_REGS: /* MIB registers */
        len += bcm63xx_enet_dump(pDevCtrl, buf, MIB_REGS);
        break;
    default:
        break;
    }
    return len;
}

/*
 *  read proc interface
 */
static ssize_t eth_proc_read(struct file *file, char *buf, size_t count,
        loff_t *pos)
{
    const struct inode *inode = file->f_dentry->d_inode;
    const struct proc_dir_entry *dp = PDE(inode);
    char *page;
    int len = 0, x, left;

    page = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!page)
        return -ENOMEM;
    left = PAGE_SIZE - 256;
    if (count < left)
        left = count;

    for (;;) {
        x = dev_proc_engine(dp->data, *pos, &page[len]);
        if (x == 0)
            break;
        if ((x + len) > left)
            x = -EINVAL;
        if (x < 0) {
            break;
        }
        len += x;
        left -= x;
        (*pos)++;
        if (left < 256)
            break;
    }
    if (len > 0 && copy_to_user(buf, page, len))
        len = -EFAULT;
    kfree(page);
    return len;
}

/*
 * /proc/driver/eth0info
 */
static struct file_operations eth_proc_operations = {
        read: eth_proc_read, /* read_proc */
};

#endif

/*
 *      bcm63xx_enet_probe: - Probe Ethernet switch and allocate device
 */
int __init bcm63xx_enet_probe(void)
{
    static int probed = 0;
    struct net_device *dev = NULL;
    BcmEnet_devctrl *pDevCtrl = NULL;
    unsigned int chipid;
    unsigned int chiprev;
    unsigned char macAddr[ETH_ALEN];
    ETHERNET_MAC_INFO EnetInfo[MAX_EMACS];
    int status;
    int unit;
    int i;
#ifdef USE_PROC
    struct proc_dir_entry *p;
    char proc_name[32];
#endif

    TRACE(("bcm63xxenet: bcm63xx_enet_probe\n"));

    /* make sure emac clock is on */
    PERF->blkEnables |= EMAC_CLK_EN;
    /* Reset Block */
    PERF->BlockSoftReset &= ~BSR_EMAC;
    mdelay(5);
    PERF->BlockSoftReset |= BSR_EMAC;

    if (probed == 0) {
        chipid  = (PERF->RevID & 0xFFFF0000) >> 16;
        chiprev = (PERF->RevID & 0xFF);
        if( BpGetEthernetMacInfo( &EnetInfo[0], MAX_EMACS ) != BP_SUCCESS ) {
            printk(KERN_DEBUG CARDNAME" board id not set\n");
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
            kerSysLedCtrl(kLedInternet,kLedStateFail);    
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
            return -ENODEV;
        }
        probed++;
    } else {
        /* device has already been initialized */
        return -ENXIO;
    }

    for ( unit = 0; unit < MAX_EMACS; unit++ ) {
        switch(EnetInfo[unit].ucPhyType) {
            case BP_ENET_INTERNAL_PHY:
            case BP_ENET_EXTERNAL_PHY:
            case BP_ENET_EXTERNAL_SWITCH:
                break;

            case BP_ENET_NO_PHY:
            default:
                continue;
        }
        dev = alloc_etherdev(sizeof(*pDevCtrl));
        if (dev == NULL) {
            printk(KERN_ERR CARDNAME": Unable to allocate "
                    "net_device structure!\n");
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
            kerSysLedCtrl(kLedInternet,kLedStateFail);    
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
            continue;
        }
        pDevCtrl = netdev_priv(dev);
        pDevCtrl->dev = dev;
        pDevCtrl->unit = unit;
        pDevCtrl->chipId  = chipid;
        pDevCtrl->chipRev = chiprev;
        pDevCtrl->linkstatus_phyport = -1;
        memcpy(&(pDevCtrl->EnetInfo), &EnetInfo[unit], sizeof(ETHERNET_MAC_INFO));
        /* print the ChipID and module version info */
        printk("Broadcom BCM%X%X Ethernet Network Device ", chipid, chiprev);
        printk(VER_STR);
        printk("\n");

        if ((status = bcm63xx_init_dev(pDevCtrl))) {
            printk((KERN_ERR CARDNAME ": device initialization error!\n"));
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
            kerSysLedCtrl(kLedInternet,kLedStateFail);    
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
            bcm63xx_uninit_dev(pDevCtrl);
            continue;
        }
#ifdef USE_PROC
        /* create a /proc entry for display driver runtime information */
        sprintf(proc_name, PROC_ENTRY_NAME, pDevCtrl->unit);
        if ((p = create_proc_entry(proc_name, 0, NULL)) == NULL) {
            printk((KERN_ERR CARDNAME ": unable to create proc entry!\n"));
            bcm63xx_uninit_dev(pDevCtrl);
            continue;
        }
        p->proc_fops = &eth_proc_operations;
        p->data = pDevCtrl;
#endif

        dev_alloc_name(dev, dev->name);
        SET_MODULE_OWNER(dev);

        pDevCtrl->next_dev = eth_root_dev;
        eth_root_dev = dev;
        /* Setup the timer */
        init_timer(&pDevCtrl->timer);
        pDevCtrl->timer.data = (unsigned long)pDevCtrl;
        pDevCtrl->timer.function = bcm63xx_enet_poll_timer;

        if (pDevCtrl->unit == 0)
            dev->base_addr      = EMAC1_BASE;
#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96358) 
        else
            dev->base_addr      = EMAC2_BASE;
#endif
        dev->irq                = pDevCtrl->rxIrq;
        dev->open               = bcm63xx_enet_open;
        dev->stop               = bcm63xx_enet_close;
        dev->hard_start_xmit    = bcm63xx_enet_xmit;
        dev->tx_timeout         = bcm63xx_enet_timeout;
        dev->watchdog_timeo     = 2*HZ;
        dev->get_stats          = bcm63xx_enet_query;
        dev->set_mac_address    = bcm_set_mac_addr;
        dev->set_multicast_list = bcm_set_multicast_list;
        dev->do_ioctl           = &bcm63xx_enet_ioctl;
        dev->poll               = bcm63xx_enet_poll;
#if defined(CONFIG_BCM_ENDPOINT) || defined(CONFIG_BCM_ENDPOINT_MODULE)
        dev->weight             = 16; // lower weight for less voice latency
#else
        dev->weight             = 64;
#endif
#ifdef VPORTS
        if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH) {
            dev->header_cache_update    = NULL;
            dev->hard_header_cache      = NULL;
            dev->hard_header_parse      = NULL;
            dev->hard_header            = bcm63xx_header;
            dev->hard_header_len        = ETH_HLEN + BRCM_TAG_LEN;
        }
#endif
        status = register_netdev(dev);

        if (status == 0) {
            probed++;

            if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH) {
#ifdef DYING_GASP_API
                kerSysRegisterDyingGaspHandler(pDevCtrl->dev->name, &bcm63xx_dying_gasp_handler, dev);
#endif
#ifdef VPORTS
                set_vnet_dev(0, dev);
                mii_switch_unmanage_mode(vnet_dev[0]);
                /*start add of lsw seperate by port_base vlan for kpn by l39225 2006-8-15*/
                 mii_switch_unPort_vlan_base_mode(vnet_dev[0]); 
                /*end add of lsw seperate by port_base vlan for kpn by l39225 2006-8-15*/
                
                /*this is no problem for ADM6996M LSW */
                ethsw_config_vlan(vnet_dev[0], VLAN_DISABLE, 0);
#endif
            }
            /* Read MAC address */
            macAddr[0] = 0xff;

            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC modify begin:
            kerSysGetMacAddress( macAddr, dev->ifindex );
            */
            kerSysGetMacAddress( macAddr, RESERVE_MAC_MASK );
            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC modify end. */

            if( (macAddr[0] & ETH_MULTICAST_BIT) == ETH_MULTICAST_BIT ) {
                memcpy( macAddr, "\x00\x10\x18\x63\x00\x00", ETH_ALEN );
                printk((KERN_CRIT "%s: MAC address has not been initialized in NVRAM.\n"),dev->name);
            }

            /* fill in the MAC address */
            for (i = 0; i < ETH_ALEN; i++) {
                dev->dev_addr[i] = macAddr[i];
            }
            write_mac_address(dev);

            /* print the Ethenet address */
            printk("%s: MAC Address: ", dev->name);
            for (i = 0; i < ETH_ALEN-1; i++) {
                printk("%2.2X:", dev->dev_addr[i]);
            }
            printk("%2.2X\n", dev->dev_addr[i]);

            /* Start the poll timer. */
            pDevCtrl->timer.expires = jiffies + POLLTIME_100MS;
            add_timer(&pDevCtrl->timer);

            /*add of support ETHWAN by l39225 20061218*/
            /*This source code  must be writed  afer the  eth0/eth1 get mac*/
#ifdef CONFIG_ETHWAN
        #ifdef CONFIG_LSWWAN
               if(pDevCtrl->EnetInfo.ucPhyType ==  BP_ENET_EXTERNAL_SWITCH)
               {        wan_dev = dev;
	 	           for(i= 0;i < MAX_NUM_WAN; i++)
	            	   {
	            	   	  bcm63xx_create_enetWan();
	            	   }
	          }
        #else
	         if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY)
	         {
		           wan_dev = dev;
	 	           for(i= 0;i < MAX_NUM_WAN; i++)
	            	   {
	            	   	  bcm63xx_create_enetWan();
	            	   }
	          }
        #endif
 #endif
            /*end of support ETHWAN by l39225 20061218*/
       

            /* Enable the internal BCM63xx PHY interrupt. */
            if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY) {
                mii_enablephyinterrupt(dev, pDevCtrl->EnetInfo.ucPhyAddress);
                BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
            }
            continue;
        } else {
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
            kerSysLedCtrl(kLedInternet,kLedStateFail);    
            /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
            bcm63xx_uninit_dev(pDevCtrl);
            printk(KERN_ERR CARDNAME "bcm63xx_enet_probe failed, returns %d\n", status);
            return status;
        }
    }
    return 0;
}

#ifdef DYING_GASP_API
void bcm63xx_dying_gasp_handler(void *context)
{
    mii_switch_power_off(context);
}
#endif

/* get ethernet port's status; return nonzero for Link up, 0 for Link down */
static int bcmIsEnetUp(struct net_device *dev)
{
    static int sem = 0;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int linkState = pDevCtrl->linkState;
    uint16 val16;
    unsigned long val32;

    if( sem == 0 )
    {
        sem = 1;

        if( (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH ) || (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_PHY))
        {
              /*start of 以太网 3.1.1 porting by l39225 20060504*/
              /*如果为SPI方式采用SPI的查寻方式*/
             if ( (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_0)
             	 ||(pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_1)
             	 || (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_2) 
             	 || (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SPI_SSB_3))
             	{
             	    ethsw_rreg(dev, PAGE_STATUS, REG_LS_SUM, (char*)&val16, sizeof(val16));    
                  linkState = ((val16>>8) & 0x000f) ;/* only take the high byte */;
             	}
             /*end of 以太网 3.1.1 porting by l39225 20060504*/
             /*start of support 	ADM9669M LSW by 139225  20061218*/
             else if (pDevCtrl->EnetInfo.usConfigType == BP_ENET_CONFIG_SMI)       //FOR ADM9669M LSW
             {
             	       unsigned long value = mii_read(dev,0,SWI_PORT_STATUS0);
             	       linkState = value & 0x01;
             	       val32 = (value >> 8) & 0x01;
             	       linkState |= (val32 << 1);
             	       value = mii_read(dev,0,SWI_PORT_STATUS1 );
             	       linkState |= ((value & 0x01) << 2);
             	       val32  = (value >> 8) & 0x01;
             	       linkState |= (val32 << 3);
             }
             /*end of support 	ADM9669M LSW by 139225  20061218*/
             else   //如果为MDIO或PSEUDO_PHY方式
             {
			/* Call non-blocking functions to read the link state. */
			if( pDevCtrl->linkstatus_phyport == -1 )
			{
				/* Start reading the link state of each switch port. */
				pDevCtrl->linkstatus_phyport = 0;
				mii_linkstatus_start(dev, pDevCtrl->EnetInfo.ucPhyAddress |
				pDevCtrl->linkstatus_phyport);
			}
		        else
		        {
		            int ls = 0;

		            /* Check if the link state is done being read for a particular
		             * switch port.
		             */
		            if( mii_linkstatus_check(dev, &ls) )
		            {
		                /* The link state for one switch port has been read.  Save
		                 * it in a temporary holder.
		                 */
		                pDevCtrl->linkstatus_holder |=
		                    ls << pDevCtrl->linkstatus_phyport++;

		                if( pDevCtrl->linkstatus_phyport ==
		                    pDevCtrl->EnetInfo.numSwitchPorts )
		                {
		                    /* The link state for all switch ports has been read.
		                     * Return the current link state.
		                     */
		                    linkState = pDevCtrl->linkstatus_holder;
		                    pDevCtrl->linkstatus_holder = 0;
		                    pDevCtrl->linkstatus_polltimer = 0;
		                    pDevCtrl->linkstatus_phyport = -1;
		                 }
		                 else
		                 {
		                    /* Start to read the link state for the next switch
		                     * port.
		                     */
		                    mii_linkstatus_start(dev,
		                        pDevCtrl->EnetInfo.ucPhyAddress |
		                        pDevCtrl->linkstatus_phyport);
		                 }
		            }
		            else if( pDevCtrl->linkstatus_polltimer > (3 * POLLCNT_1SEC) )
		            {
		                    /* Timeout reading MII status. Reset link state check. */
		                    pDevCtrl->linkstatus_holder = 0;
		                    pDevCtrl->linkstatus_polltimer = 0;
		                    pDevCtrl->linkstatus_phyport = -1;
		             }
	              }
              }
       }
        else
        {
            /* The link state for the internal PHY is only read after an
             * interrupt indicates that it has changed.  Therefore, it is OK
             * to call the blocking link state read function.
             */
            mii_clearphyinterrupt(dev, pDevCtrl->EnetInfo.ucPhyAddress);
            BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
            linkState = mii_linkstatus(dev, pDevCtrl->EnetInfo.ucPhyAddress);
            pDevCtrl->linkstatus_polltimer = POLLCNT_FOREVER;
        }

        sem = 0;
    }

    return linkState;
}

static void __exit bcmenet_module_cleanup(void)
{
    BcmEnet_devctrl *pDevCtrl;
    struct net_device *next_dev;
    int i = 0;

    TRACE(("bcm63xxenet: bcmenet_module_cleanup\n"));

    /* No need to check MOD_IN_USE, as sys_delete_module() checks. */
    while (eth_root_dev) {
        next_dev = NULL;
        pDevCtrl = (BcmEnet_devctrl *)netdev_priv(eth_root_dev);
        if (pDevCtrl) {
#ifdef DYING_GASP_API
            if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH)
                kerSysDeregisterDyingGaspHandler(pDevCtrl->dev->name);
#endif
            next_dev = pDevCtrl->next_dev;
            bcm63xx_uninit_dev(pDevCtrl);
        }
        eth_root_dev = next_dev;
    }

/*add of supprot ETHWAN by l39225 20061218*/
  #ifdef CONFIG_ETHWAN
     for (i = 0; i < MAX_NUM_WAN; i++)
    {
        if (wan_net_dev[i] != NULL)
        {
            unregister_netdev(wan_net_dev[i]);
            free_netdev(wan_net_dev[i]);
        }
    }
  #endif
/*end of support ETHWAN by l39225 20061218*/
    
}

static int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    u32 ethcmd;
    
#ifdef VPORTS
    if (is_vport(dev)) {
        pDevCtrl = netdev_priv(vnet_dev[0]);
    }
#endif

    ASSERT(pDevCtrl != NULL);   
        
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
    default:
        break;
    }
    
    return -EOPNOTSUPP;    
}


/*add of support ETHWAN by l39225 20061218*/
#ifdef CONFIG_ETHWAN

static int  bcm63xx_set_enetWan(unsigned short wanId)
{
    struct net_device *dev;
    int i = 0;
    unsigned char macAddr[ETH_ALEN];

    static int probed = 0;

 
    probed++;
    
    if(NULL == wan_dev)
    {
          printk("=== oh oh!! my god,wan is NULL\n");
          return -1;
    }

   if(probed > MAX_NUM_WAN)
   {
   	   printk("=== oh oh!! my god,more enet wan interface\n");
   	   return -1;
   }


   dev = wan_net_dev[wanId];

   if(NULL == dev)
   {
   	 printk("===oh oh !! my god ,not find enet wan dev\n");
   	 return -1;
   }
   dev->base_addr |= WAN_UP;

   /*In order to reduce the mumber of mac ,the wan0 and br0 share the same mac.
      Of course ,this is error,but no problem to use*/
   if(probed == 1)      
   {
   	  for (i = 0; i < ETH_ALEN; i++) 
        {
	       dev->dev_addr[i] = wan_dev->dev_addr[i];
         }
   }
   else
   {
	   macAddr[0] = 0xff;
	   kerSysGetMacAddress( macAddr, dev->ifindex );
	   if( (macAddr[0] & ETH_MULTICAST_BIT) == ETH_MULTICAST_BIT ) 
	   {
		   memcpy( macAddr, "\x00\x10\x18\x63\x00\x00", ETH_ALEN );
		   printk((KERN_CRIT "%s: MAC address has not been initialized in NVRAM.\n"),dev->name);
	   }
   
	   /* fill in the MAC address */
	   for (i = 0; i < ETH_ALEN; i++) 
	   {
		   dev->dev_addr[i] = macAddr[i];
	   }
   }

    printk("%s: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            dev->name,
            dev->dev_addr[0],
            dev->dev_addr[1],
            dev->dev_addr[2],
            dev->dev_addr[3],
            dev->dev_addr[4],
            dev->dev_addr[5]
     );
    return 0;
 }


static struct net_device_stats *ewan_query(struct net_device * dev)
{
         return (struct net_device_stats *)netdev_priv(dev);
}

static int  bcm63xx_create_enetWan(void)
{
    struct net_device *dev;

    static int probed = 0;
    int status;
 
    probed++;

    if(NULL == wan_dev)
    {
          printk("=== oh oh!! my god,wan is NULL\n");
          return -1;
    }

   if(probed > MAX_NUM_WAN)
   {
   	   printk("=== oh oh!! my god,more enet wan interface\n");
   	   return -1;
   }
   
    dev = alloc_etherdev(sizeof(struct net_device_stats));
    memset(netdev_priv(dev), 0, sizeof(struct net_device_stats));

    if (dev == NULL)
    {
	    printk("=== oh oh!! my god,net wan alloc fail\n");
    }

    dev_alloc_name(dev, dev->name);
    SET_MODULE_OWNER(dev);


    sprintf(dev->name, "%s.%d", wan_dev->name, probed-1);

    printk("======oh oh dev name is %s\n",dev->name);
  

    dev->open                   = wan_dev->open;
    dev->stop                   = wan_dev->stop;
    dev->hard_start_xmit   = wan_dev->hard_start_xmit;
    
    /* need to hook the set_mac_address and set_multicast_list. but need not to do_ioctl*/
    dev->set_mac_address     = wan_dev->set_mac_address;
    dev->set_multicast_list     = wan_dev->set_multicast_list;
    //dev->do_ioctl               = wan_dev->do_ioctl;
    dev->get_stats              =  ewan_query;
    dev->base_addr              = DEV_BASE |probed;

   
    /*this is must be called in module_init*/
     status = register_netdev(dev);

    if (status != 0)
    {
        printk("===oh oh !! my god ,register enet wan fail\n");
        unregister_netdev(dev);
        free_netdev(dev);
        return -1;
    }

    wan_net_dev[probed -1 ] = dev;

    return 0;
}
#endif
/*end of support ETHWAN by l39225 20061218*/

static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int *data=(int*)rq->ifr_data;
    struct mii_ioctl_data *mii;
    int val = 0;
    int mask = 0;
    unsigned short iVlanData = 0;
    unsigned char ucPhyType;
    

#ifdef VPORTS
    if (is_vport(dev)) {
        pDevCtrl = netdev_priv(vnet_dev[0]);
    }
#endif

    /* we can add sub-command in ifr_data if we need to in the future */
    switch (cmd)
    {
    case SIOCGMIIPHY:       /* Get address of MII PHY in use. */
        mii = (struct mii_ioctl_data *)&rq->ifr_data;
        mii->phy_id = pDevCtrl->EnetInfo.ucPhyAddress;

    case SIOCGMIIREG:       /* Read MII PHY register. */
        local_bh_disable();
        mii = (struct mii_ioctl_data *)&rq->ifr_data;
        mii->val_out = mii_read(dev, mii->phy_id & 0x1f, mii->reg_num & 0x1f);
        __local_bh_enable();
        break;

    case SIOCSMIIREG:       /* Write MII PHY register. */
        local_bh_disable();
        mii = (struct mii_ioctl_data *)&rq->ifr_data;
        mii_write(dev, mii->phy_id & 0x1f, mii->reg_num & 0x1f, mii->val_in);
        __local_bh_enable();
        break;

    case SIOCGLINKSTATE:
#ifdef VPORTS
        if (vnet_dev[0] == dev)
            mask = 0xffffffff;
        else if (is_vport(dev))
            mask = 0x00000001 << egress_vport_id_from_dev(dev);
        else
            mask = 0x00000001;
#else
        mask = 0xffffffff;
#endif

        val = (pDevCtrl->linkState & mask)? 1: 0;

        if (copy_to_user((void*)data, (void*)&val, sizeof(int))) {
            return -EFAULT;
        }
        val = 0;
        break;

    case SIOCGLSWLINKSTATE:
        val = (pDevCtrl->linkState);
        if (copy_to_user((void*)data, (void*)&val, sizeof(int))) 
        {
            return -EFAULT;
        }
        break;

    case SIOCSCLEARMIBCNTR:
        ASSERT(pDevCtrl != NULL);
        memset(&pDevCtrl->stats, 0, sizeof(struct net_device_stats));
        break;

    case SIOCMIBINFO:
        if (copy_to_user((void*)data, (void*)&pDevCtrl->MibInfo,
            sizeof(pDevCtrl->MibInfo))) {
            return -EFAULT;
        }
        break;

    case SIOCETHTOOL:
        return netdev_ethtool_ioctl(dev, (void *) rq->ifr_data);

    case SIOCGENABLEVLAN:
        /*
        pDevCtrl->vid = 2;
        val = ethsw_config_vlan(dev, (int)VLAN_ENABLE, pDevCtrl->vid);
        break;
        */
        /*start of add lsw port separate by port_base vlan by 139225 2006-8-15*/
        copy_from_user((void*)&iVlanData,(void*)data,sizeof(unsigned short));
        if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH)
        {
            if( vnet_dev[0] != NULL)
            {
                 mii_switch_port_vlan_base_mode(vnet_dev[0], iVlanData); 
            }
        }
        val = 0;
        break;
       /*end of add lsw port separate by port_base vlan by 139225 2006-8-15*/

    case SIOCGDISABLEVLAN:
        /*
        pDevCtrl->vid = 0;
        val = ethsw_config_vlan(dev, VLAN_DISABLE, 0);
        */
        /*start of add lsw port separate by port_base vlan by 139225 2006-8-15*/
        if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH)
        {
            if( vnet_dev[0] != NULL)
            {
                 mii_switch_unPort_vlan_base_mode(vnet_dev[0]); 
            }
        }
        val = 0;
        /*end of add lsw port separate by port_base vlan by 139225 2006-8-15*/
        break;

  /*add of  support ETHWAN by l39225 20061218*/
    case SIOCSETMODE:
    	    val = 0;
    	    copy_from_user((void*)&iVlanData,(void*)data,sizeof(unsigned short));
      #ifdef  CONFIG_ETHWAN
            s_iWanMode = iVlanData;
           printk("===oh oh wanMode is:%d\n",s_iWanMode);
      #endif
           break;
           
    case  SIOCREATWAN:
    	
    	   val = 0;
    #ifdef CONFIG_ETHWAN
         copy_from_user((void*)&iVlanData,(void*)data,sizeof(unsigned short));
      #ifdef CONFIG_LSWWAN
	        if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH)
	     	{
	     		if( wan_dev !=NULL)
			{
				bcm63xx_set_enetWan(iVlanData);
			}
	     		
	     	}
      #else
	       if(pDevCtrl->EnetInfo.ucPhyType == BP_ENET_INTERNAL_PHY)
	       {
	            if( wan_dev != NULL)
	            {
	                  bcm63xx_set_enetWan(iVlanData); 
	            }
	        }
       #endif
    #endif

    	   break;
    /*end of supprot ETHWAN by l39225 20061218*/

    case SIOCGQUERYNUMVLANPORTS:
        switch(pDevCtrl->ethSwitch.type) {
        case ESW_TYPE_BCM5325M:
            val = 1;
            break;
        case ESW_TYPE_BCM5325E:
        case ESW_TYPE_BCM5325F:
        case ESW_TYPE_ADM6996M:
        default:
            val = pDevCtrl->EnetInfo.numSwitchPorts;
            break;
        }
        if (copy_to_user((void*)data, (void*)&val, sizeof(int))) {
            return -EFAULT;
        }
        val = 0;
        break;

    case SIOCGSWITCHTYPE:
        if (pDevCtrl->EnetInfo.ucPhyType == BP_ENET_EXTERNAL_SWITCH) 
        {
            /*start add of support ADM6996M LSW by l39225 20061218*/
             ucPhyType = pDevCtrl->ethSwitch.type;
             if(ESW_TYPE_ADM6996M ==  pDevCtrl->ethSwitch.type)
             {
             	      ucPhyType = ESW_TYPE_BCM5325E;
             }
             /*end  of support ADM6996M LSW by l39225 20061218*/
            if (copy_to_user((void*)data, (void*)&ucPhyType, sizeof(int)))
            {
                return -EFAULT;
            }
        } else
            return -EFAULT;
        break;

    case SIOCGQUERYNUMPORTS:
        val = pDevCtrl->EnetInfo.numSwitchPorts;
        if (copy_to_user((void*)data, (void*)&val, sizeof(int))) {
            return -EFAULT;
        }
        val = 0;
        break;

    /* start 支持以太网口配置 by l68693 2009-02-17*/
    case SIOCGPORTOFF:        
        mii_port_power_off(dev,(int) *data);
        val = 0;        
        break;
        
    case SIOCGPORTON:
        mii_port_power_on(dev,(int) *data);
        val = 0;        
        break;
    default:
        printk("===IOTCL UnKnown.\n");
        val = -EOPNOTSUPP;
        break;
    /* end 支持以太网口配置 by l68693 2009-02-17*/
        
    }

    return val;
}

static int __init bcmenet_module_init(void)
{
    int status;

    TRACE(("bcm63xxenet: bcmenet_module_init\n"));

    status = bcm63xx_enet_probe();

    return status;
}

module_init(bcmenet_module_init);
module_exit(bcmenet_module_cleanup);
MODULE_LICENSE("Proprietary");

