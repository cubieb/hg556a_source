/*
Experimental ethernet netdevice using ATM AAL5 as underlying carrier
(RFC1483 obsoleted by RFC2684) for Linux 2.4
Author: Marcell GAL, 2000, XDSL Ltd, Hungary
*/

#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <asm/uaccess.h>
#include <net/arp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/seq_file.h>

#include <linux/atmbr2684.h>

#include "common.h"
#include "ipcommon.h"
#include "../../../bcmdrivers/opensource/include/bcm963xx/board.h"

extern int kerSysGetLedState(BOARD_LED_NAME ledName);

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
#define PPPOE_PROTOCOL   0
#define IPOE_PROTOCOL    2
/* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
#define BRIDGE_PROTOCOL  3
/* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/*
 * Define this to use a version of the code which interacts with the higher
 * layers in a more intellegent way, by always reserving enough space for
 * our header at the begining of the packet.  However, there may still be
 * some problems with programs like tcpdump.  In 2.5 we'll sort out what
 * we need to do to get this perfect.  For now we just will copy the packet
 * if we need space for the header
 */
/* #define FASTER_VERSION */
//#define VLAN_DEBUG
//#define SKB_DEBUG

#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "br2684: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
extern int g_nPtmTraffic;
/* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
extern int g_nPtmConfig;
/* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

#ifdef SKB_DEBUG
static void skb_debug(const struct sk_buff *skb)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < skb->len && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & skb->data[i]);
	}
	printk(KERN_DEBUG "br2684: skb: %s\n", buf);
}
#else
#define skb_debug(skb)	do {} while (0)
#endif

static unsigned char llc_oui_pid_pad[] =
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 };
#define PADLEN	(2)

enum br2684_encaps {
	e_vc  = BR2684_ENCAPS_VC,
	e_llc = BR2684_ENCAPS_LLC,
};

struct br2684_vcc {
	struct atm_vcc  *atmvcc;
	struct net_device *device;
	/* keep old push,pop functions for chaining */
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
	/* void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb); */
	enum br2684_encaps encaps;
	struct list_head brvccs;
#ifdef CONFIG_ATM_BR2684_IPFILTER
	struct br2684_filter filter;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
#ifndef FASTER_VERSION
	unsigned copies_needed, copies_failed;
#endif /* FASTER_VERSION */
#if defined(CONFIG_MIPS_BRCM)
	/* Protocol filter flag, currently only PPPoE.
	   When turned on, all non-PPPoE traffic will be dropped
	   on this PVC */  
        int proto_filter;
#ifdef SUPPORT_VLAN
        unsigned short vlan_id;        /* vlan id (0-4096) */
#endif  // SUPPORT_VLAN


#endif
};

struct br2684_dev {
	struct net_device *net_dev;
	struct list_head br2684_devs;
	int number;
	struct list_head brvccs; /* one device <=> one vcc (before xmas) */
	struct net_device_stats stats;
	int mac_was_set;
};

#if defined(CONFIG_MIPS_BRCM)
#define MIN_PKT_SIZE 70
#endif

/*start ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
#ifndef PROTO_PPPOE 
#define PROTO_PPPOE		0
#endif
#ifndef PROTO_MER 
#define PROTO_MER			 2
#endif
#ifndef PTM_TRAFFIC
#define PTM_TRAFFIC         1
#endif
#ifndef  PTM_TRAFFIC_PRTCL
#define PTM_TRAFFIC_PRTCL  2
#endif
#ifndef PTM_TRAFFIC_BR     
#define PTM_TRAFFIC_BR     4
#endif

#define PTM_IFC_HASH_MAX  16
#define PTM_HASH_KEY(vid) ((((vid)>>8)^((vid)>>4)^(vid))&0xF)
struct ptmdev{
    struct net_device *net_dev;
	struct list_head list;
	unsigned short id;
	unsigned short reserved;
	
};
spinlock_t        g_ptmlock;
static struct list_head g_ptmhash[PTM_IFC_HASH_MAX];

static void ptmhash_init(void)
{
    int i;
    for(i = 0; i < PTM_IFC_HASH_MAX; i++)
        INIT_LIST_HEAD(&g_ptmhash[i]);
}

static struct ptmdev* alloc_ptmdev(void)
{
    struct ptmdev* p;

    p = (struct ptmdev*)kmalloc(sizeof(struct ptmdev), GFP_KERNEL);

    if(p)
        memset(p, 0, sizeof(struct ptmdev));
    
    return p;
}
static int register_ptmdev(struct ptmdev *pdev)
{
    if(!pdev)
        return -1;

    spin_lock_bh(&g_ptmlock);
    list_add_rcu(&pdev->list, &g_ptmhash[PTM_HASH_KEY(pdev->id)]);
    spin_unlock_bh(&g_ptmlock);

    return 0;
}
static void unregister_ptmdev(struct ptmdev *pdev)
{
    if(!pdev)
        return;
    
    spin_lock_bh(&g_ptmlock);
    list_del_rcu(&pdev->list);
    kfree((void*)pdev);
    spin_unlock_bh(&g_ptmlock);
}
static __inline__ struct ptmdev* find_ptmdev(unsigned short id) 
{
    struct ptmdev *pdev;
    
    list_for_each_entry_rcu(pdev, &g_ptmhash[PTM_HASH_KEY(id)], list)
    {
        if(pdev->id == id)
        {
            return pdev;
        }
    }

    return NULL;
}
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
unsigned long crc32_table[256];

// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
unsigned long Reflect(unsigned long  ref, char ch)
{// Used only by Init_CRC32_Table()

	unsigned long value = 0;

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	int i;
	for( i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}
void Init_CRC32_Table()
{// Called by OnInitDialog()

	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	unsigned long  ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	int i,j;
	for( i = 0; i <= 0xFF; i++)
	{
		crc32_table[i]=Reflect(i, 8) << 24;
		for ( j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		crc32_table[i] = Reflect(crc32_table[i], 32);
	}
}



int Get_CRC(unsigned char * csData, int dwSize)
{
	// Be sure to use unsigned variables,
	// because negative values introduce high bits
	// where zero bits are required.
	unsigned  long crc = 0xffffffff;
	int len;
	unsigned char* buffer;

	len = dwSize;
	// Save the text in the buffer.
	buffer = csData;
	// Perform the algorithm on each character
	// in the string, using the lookup table values.
	while(len--)
		crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *buffer++];
	// Exclusive OR the result with the beginning value.
	return crc^0xffffffff;
}

#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/*
 * This lock should be held for writing any time the list of devices or
 * their attached vcc's could be altered.  It should be held for reading
 * any time these are being queried.  Note that we sometimes need to
 * do read-locking under interrupt context, so write locking must block
 * the current CPU's interrupts
 */
static rwlock_t devs_lock = RW_LOCK_UNLOCKED;

static LIST_HEAD(br2684_devs);

static inline struct br2684_dev *BRPRIV(const struct net_device *net_dev)
{
	return (struct br2684_dev *) net_dev->priv;
}

static inline struct net_device *list_entry_brdev(const struct list_head *le)
{
	return list_entry(le, struct br2684_dev, br2684_devs)->net_dev;
}

static inline struct br2684_vcc *BR2684_VCC(const struct atm_vcc *atmvcc)
{
	return (struct br2684_vcc *) (atmvcc->user_back);
}

static inline struct br2684_vcc *list_entry_brvcc(const struct list_head *le)
{
	return list_entry(le, struct br2684_vcc, brvccs);
}

/* Caller should hold read_lock(&devs_lock) */
static struct net_device *br2684_find_dev(const struct br2684_if_spec *s)
{
	struct list_head *lh;
	struct net_device *net_dev;
	switch (s->method) {
	case BR2684_FIND_BYNUM:
		list_for_each(lh, &br2684_devs) {
			net_dev = list_entry_brdev(lh);
			if (BRPRIV(net_dev)->number == s->spec.devnum)
				return net_dev;
		}
		break;
	case BR2684_FIND_BYIFNAME:
		list_for_each(lh, &br2684_devs) {
			net_dev = list_entry_brdev(lh);
			if (!strncmp(net_dev->name, s->spec.ifname, IFNAMSIZ))
				return net_dev;
		}
		break;
	}
	return NULL;
}

#ifdef SUPPORT_VLAN
#include <linux/if_vlan.h>
/**
 * vlan_tag_insert - regular VLAN tag inserting
 * @skb: skbuff to tag
 * @tag: VLAN tag to insert
 *
 * Inserts the VLAN tag into @skb as part of the payload
 * Returns a VLAN tagged skb. If a new skb is created, @skb is freed.
 * 
 * Following the skb_unshare() example, in case of error, the calling function
 * doesn't have to worry about freeing the original skb.
 */
#define ATM_HEADER_LEN     10
#define ATM_AND_MAC_LEN    ((ATM_HEADER_LEN) + (2 * VLAN_ETH_ALEN))

struct atm_vlan_ethhdr {
   unsigned char        h_atm_stuff[ATM_HEADER_LEN]; /* atm stuff 10 bytes */
   unsigned char	h_dest[ETH_ALEN];	   /* destination eth addr	*/
   unsigned char	h_source[ETH_ALEN];	   /* source ether addr	*/
   unsigned short       h_vlan_proto;              /* Should always be 0x8100 */
   unsigned short       h_vlan_TCI;                /* Encapsulates priority and VLAN ID */
   unsigned short	h_vlan_encapsulated_proto; /* packet type ID field (or len) */
};

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
struct ptm_vlan_ethhdr {
   unsigned char	h_dest[ETH_ALEN];	   /* destination eth addr	*/
   unsigned char	h_source[ETH_ALEN];	   /* source ether addr	*/
   unsigned short       h_vlan_proto;              /* Should always be 0x8100 */
   unsigned short       h_vlan_TCI;                /* Encapsulates priority and VLAN ID */
   unsigned short	h_vlan_encapsulated_proto; /* packet type ID field (or len) */
};
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

static inline struct sk_buff *vlan_tag_insert(struct sk_buff *skb, unsigned short tag)
{
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
    if (g_nPtmTraffic)
    {
	    struct ptm_vlan_ethhdr *veth;

	    if (skb_headroom(skb) < VLAN_HLEN) {
		struct sk_buff *sk_tmp = skb;
		skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
		kfree_skb(sk_tmp);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to realloc headroom\n");
			return NULL;
		  }
    	} else {
    		skb = skb_unshare(skb, GFP_ATOMIC);
    		if (!skb) {
    			printk(KERN_ERR "vlan: failed to unshare skbuff\n");
    			return NULL;
    		}
    	}

    	veth = (struct ptm_vlan_ethhdr *)skb_push(skb, VLAN_HLEN);

    	/* Move the mac addresses to the beginning of the new header. */
    	memmove(skb->data, skb->data + VLAN_HLEN, (2 * VLAN_ETH_ALEN));

    	/* first, the ethernet type */
    	veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);

    	/* now, the tag */
    	veth->h_vlan_TCI = htons(tag);

    	skb->protocol = __constant_htons(ETH_P_8021Q);
    	skb->mac.raw -= VLAN_HLEN;
    	skb->nh.raw -= VLAN_HLEN;

    	return skb;
    }
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

	struct atm_vlan_ethhdr *veth;

	if (skb_headroom(skb) < VLAN_HLEN) {
		struct sk_buff *sk_tmp = skb;
		skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
		kfree_skb(sk_tmp);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to realloc headroom\n");
			return NULL;
		}
	} else {
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to unshare skbuff\n");
			return NULL;
		}
	}

	veth = (struct atm_vlan_ethhdr *)skb_push(skb, VLAN_HLEN);

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data, skb->data + VLAN_HLEN, (2 * VLAN_ETH_ALEN) + ATM_HEADER_LEN);

	/* first, the ethernet type */
	veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);

	/* now, the tag */
	veth->h_vlan_TCI = htons(tag);

	skb->protocol = __constant_htons(ETH_P_8021Q);
	skb->mac.raw -= VLAN_HLEN;
	skb->nh.raw -= VLAN_HLEN;

	return skb;
}
#endif  // SUPPORT_VLAN

/*
 * Send a packet out a particular vcc.  Not to useful right now, but paves
 * the way for multiple vcc's per itf.  Returns true if we can send,
 * otherwise false
 */
static int br2684_xmit_vcc(struct sk_buff *skb, struct br2684_dev *brdev,
	struct br2684_vcc *brvcc)
{
	struct atm_vcc *atmvcc;
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
  if( !g_nPtmTraffic ) {
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef FASTER_VERSION
	if (brvcc->encaps == e_llc)
		memcpy(skb_push(skb, 8), llc_oui_pid_pad, 8);
	/* last 2 bytes of llc_oui_pid_pad are managed by header routines;
	   yes, you got it: 8 + 2 = sizeof(llc_oui_pid_pad)
	 */
#else
	int minheadroom = (brvcc->encaps == e_llc) ? 10 : 2;
	if (skb_headroom(skb) < minheadroom) {
		struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);
		brvcc->copies_needed++;
		dev_kfree_skb(skb);
		if (skb2 == NULL) {
			brvcc->copies_failed++;
			return 0;
		}
		skb = skb2;
	}
	skb_push(skb, minheadroom);
	if (brvcc->encaps == e_llc)
		memcpy(skb->data, llc_oui_pid_pad, 10);
	else
		memset(skb->data, 0, 2);
#endif /* FASTER_VERSION */

#if defined(CONFIG_MIPS_BRCM)
	if (skb->len < MIN_PKT_SIZE)
	{
		struct sk_buff *skb2=skb_copy_expand(skb, 0, MIN_PKT_SIZE - skb->len, GFP_ATOMIC);
		dev_kfree_skb(skb);
		if (skb2 == NULL) {
			brvcc->copies_failed++;
			return 0;
		}
		skb = skb2;		
		memset(skb->tail, 0, MIN_PKT_SIZE - skb->len);		
		skb_put(skb, MIN_PKT_SIZE - skb->len);
	}
#endif

#ifdef SUPPORT_VLAN
#ifdef VLAN_DEBUG
        printk("=====> br2684_xmit_vcc  bef add vlan tag, skb->len=0x%04x\n", skb->len);
        skb_debug(skb);
#endif // VLAN_DEBUG

	/* Construct the second two bytes. This field looks something
	 * like:
	 * usr_priority: 3 bits	 (high bits)
	 * CFI		 1 bit
	 * VLAN ID	 12 bits (low bits)
	 */
	//brvcc->vlan_id |= vlan_dev_get_egress_qos_mask(dev, skb);
        /* bit 3-0 of the 32-bit nfmark is the atm priority, set by iptables
         * bit 7-4 is the Ethernet switch physical port number, set by lan port drivers.
         * bit 8-11 is the wanVlan priority bits
         */
	if (brvcc->vlan_id != 0xffff) {
		brvcc->vlan_id &= 0xffff0fff;   // clear the priority bits first
		// if bit 8-11 is set (none zeros), add in the priority bits
		if (skb->nfmark & 0x0000f000) {
			brvcc->vlan_id |= (skb->nfmark & 0x0000f000);
		}
		skb = vlan_tag_insert(skb, brvcc->vlan_id);
		if (!skb) {
			brdev->stats.tx_dropped++;
			return 1;
		}
#ifdef VLAN_DEBUG
		printk("=====> br2684_xmit_vcc  aft add vlan tag, skb->len=%d\n", skb->len);
		skb_debug(skb);
#endif // VLAN_DEBUG
	}
#endif  // SUPPORT_VLAN

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
  } // g_nPtmTrafffic
  else
  {

	#if 1
       #ifdef SUPPORT_VLAN
       #ifdef VLAN_DEBUG
        printk("=====> br2684_xmit_vcc  bef add vlan tag, skb->len=0x%04x\n", skb->len);
        skb_debug(skb);
       #endif // VLAN_DEBUG

	/* Construct the second two bytes. This field looks something
	 * like:
	 * usr_priority: 3 bits	 (high bits)
	 * CFI		 1 bit
	 * VLAN ID	 12 bits (low bits)
	 */
	//brvcc->vlan_id |= vlan_dev_get_egress_qos_mask(dev, skb);
        /* bit 3-0 of the 32-bit nfmark is the atm priority, set by iptables
         * bit 7-4 is the Ethernet switch physical port number, set by lan port drivers.
         * bit 8-11 is the wanVlan priority bits
         */
	if (brvcc->vlan_id != 0xffff) {

	     //printk("=================the vlan id:%d\n",brvcc->vlan_id);
		brvcc->vlan_id &= 0xffff0fff;   // clear the priority bits first
		// if bit 8-11 is set (none zeros), add in the priority bits
		if (skb->nfmark & 0x0000f000) {
			brvcc->vlan_id |= (skb->nfmark & 0x0000f000);
		}

		skb = vlan_tag_insert(skb, brvcc->vlan_id);

		if (!skb) {
			brdev->stats.tx_dropped++;
			return 1;
		}
#ifdef VLAN_DEBUG
		printk("=====> br2684_xmit_vcc  aft add vlan tag, skb->len=%d\n", skb->len);
		skb_debug(skb);
#endif // VLAN_DEBUG
	}
#endif  // SUPPORT_VLAN
    
    #endif

    #if 1
       #define MIN_PKT_SIZE  60
       #if defined(CONFIG_MIPS_BRCM)
	if (skb->len < MIN_PKT_SIZE)
	{
		struct sk_buff *skb2=skb_copy_expand(skb, 0, MIN_PKT_SIZE - skb->len, GFP_ATOMIC);
		dev_kfree_skb(skb);
		if (skb2 == NULL) {
			brvcc->copies_failed++;
			return 0;
		}
		skb = skb2;		
		memset(skb->tail, 0, MIN_PKT_SIZE - skb->len);		
		skb_put(skb, MIN_PKT_SIZE - skb->len);
	}
	#endif
    #endif
    
    /* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete begin:
    
        unsigned long crc = Get_CRC(skb->data,skb->len);
        unsigned char *pcrc = (unsigned char*)(&crc);
        //printk("len %d send cacu crc is : 0x%08x\n", skb->len,crc);

	skb->tail[0] = pcrc[3];
	skb->tail[1] = pcrc[2];
	skb->tail[2] = pcrc[1];
	skb->tail[3] = pcrc[0];
  	  skb->len += 4;	// extra four bytes for ethernet checksum
     HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete end.*/
  
  }
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
	ATM_SKB(skb)->vcc = atmvcc = brvcc->atmvcc;
	DPRINTK("atm_skb(%p)->vcc(%p)->dev(%p)\n", skb, atmvcc, atmvcc->dev);
	if (!atm_may_send(atmvcc, skb->truesize)) {
		/* we free this here for now, because we cannot know in a higher 
			layer whether the skb point it supplied wasn't freed yet.
			now, it always is.
		*/
		dev_kfree_skb(skb);
		return 0;
		}
	atomic_add(skb->truesize, &atmvcc->sk->sk_wmem_alloc);
	ATM_SKB(skb)->atm_options = atmvcc->atm_options;
	brdev->stats.tx_packets++;
	brdev->stats.tx_bytes += skb->len;
#if defined(CONFIG_MIPS_BRCM)
	if (atmvcc->send(atmvcc, skb) != 0)
	    brdev->stats.tx_dropped++;
#else
	atmvcc->send(atmvcc, skb);
#endif	

	return 1;
}

static inline struct br2684_vcc *pick_outgoing_vcc(struct sk_buff *skb,
	struct br2684_dev *brdev)
{
	return list_empty(&brdev->brvccs) ? NULL :
	    list_entry_brvcc(brdev->brvccs.next); /* 1 vcc/dev right now */
}

static int br2684_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct br2684_dev *brdev = BRPRIV(dev);
	struct br2684_vcc *brvcc;

	/*l65130 2008-09-20 start*/
	int iLedState = kerSysGetLedState(kLedAdsl);
      
     /* BEGIN: Modified by c106292, 2009/4/15   PN: */
#if 0	 
    if (kLedStateOff !=  iLedState && kLedStateSlowBlinkContinues != iLedState &&kLedStateVerySlowBlinkContiunes!=iLedState)
	/* END:   Modified by c106292, 2009/4/15 */
    {
        if ((NULL !=dev)&&( NULL == dev->br_port ))
	    {
        #if 0	    	
	        static unsigned long last_time = 0;
	        if (jiffies - last_time > 20)
	        {
	            last_time = jiffies;
	            kerSysLedCtrl(kLedWanData, kLedStateBlinkOnce);
	        }
        #endif
            kerSysLedCtrl(kLedAdsl, kLedStateOn); /*c106292*/
      //      kerSysLedCtrl(kLedAdsl, kLedStateSlowFlickerOnce); 
	    }
    }
#endif
    /*l65130 2008-09-20 end*/
  
    
	DPRINTK("br2684_start_xmit, skb->dst=%p\n", skb->dst);
	read_lock(&devs_lock);
	brvcc = pick_outgoing_vcc(skb, brdev);
	if (brvcc == NULL) {
		DPRINTK("no vcc attached to dev %s\n", dev->name);
		brdev->stats.tx_errors++;
		brdev->stats.tx_carrier_errors++;
		/* netif_stop_queue(dev); */
		dev_kfree_skb(skb);
		read_unlock(&devs_lock);
		return -EUNATCH;
	}
	
#if defined(CONFIG_MIPS_BRCM)
#if 0
	if (brvcc->proto_filter & FILTER_PPPOE) {
		if ((skb->protocol != htons(ETH_P_PPP_DISC)) && (skb->protocol != htons(ETH_P_PPP_SES))) {
			DPRINTK("non-PPPOE packet dropped on TX dev %s\n", dev->name);
			dev_kfree_skb(skb);
			read_unlock(&devs_lock);
			return 0;
		}
	}
#endif
#endif	

#ifdef VLAN_DEBUG
        if (brvcc->vlan_id != 0xffff)
                printk("=====> br2684_start_xmit  vlan_id=0x%04x\n", brvcc->vlan_id);
#endif // VLAN_DEBUG

	if (!br2684_xmit_vcc(skb, brdev, brvcc)) {
		/*
		 * We should probably use netif_*_queue() here, but that
		 * involves added complication.  We need to walk before
		 * we can run
		 */
		/* don't free here! this pointer might be no longer valid!
		dev_kfree_skb(skb);
		*/
		brdev->stats.tx_errors++;
		brdev->stats.tx_fifo_errors++;
	}
	read_unlock(&devs_lock);
	return 0;
}

static struct net_device_stats *br2684_get_stats(struct net_device *dev)
{
	DPRINTK("br2684_get_stats\n");
	return &BRPRIV(dev)->stats;
}

#ifdef FASTER_VERSION
/*
 * These mirror eth_header and eth_header_cache.  They are not usually
 * exported for use in modules, so we grab them from net_device
 * after ether_setup() is done with it.  Bit of a hack.
 */
static int (*my_eth_header)(struct sk_buff *, struct net_device *,
	unsigned short, void *, void *, unsigned);
static int (*my_eth_header_cache)(struct neighbour *, struct hh_cache *);

static int
br2684_header(struct sk_buff *skb, struct net_device *dev,
	      unsigned short type, void *daddr, void *saddr, unsigned len)
{
	u16 *pad_before_eth;
	int t = my_eth_header(skb, dev, type, daddr, saddr, len);
	if (t > 0) {
		pad_before_eth = (u16 *) skb_push(skb, 2);
		*pad_before_eth = 0;
		return dev->hard_header_len;	/* or return 16; ? */
	} else
		return t;
}

static int
br2684_header_cache(struct neighbour *neigh, struct hh_cache *hh)
{
/* hh_data is 16 bytes long. if encaps is ether-llc we need 24, so
xmit will add the additional header part in that case */
	u16 *pad_before_eth = (u16 *)(hh->hh_data);
	int t = my_eth_header_cache(neigh, hh);
	DPRINTK("br2684_header_cache, neigh=%p, hh_cache=%p\n", neigh, hh);
	if (t < 0)
		return t;
	else {
		*pad_before_eth = 0;
		hh->hh_len = PADLEN + ETH_HLEN;
	}
	return 0;
}

/*
 * This is similar to eth_type_trans, which cannot be used because of
 * our dev->hard_header_len
 */
static inline unsigned short br_type_trans(struct sk_buff *skb,
					       struct net_device *dev)
{
	struct ethhdr *eth;
	unsigned char *rawp;
	eth = skb->mac.ethernet;

	if (*eth->h_dest & 1) {
		if (memcmp(eth->h_dest, dev->broadcast, ETH_ALEN) == 0)
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	}

	else if (memcmp(eth->h_dest, dev->dev_addr, ETH_ALEN))
		skb->pkt_type = PACKET_OTHERHOST;

	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;

	rawp = skb->data;

	/*
	 * This is a magic hack to spot IPX packets. Older Novell breaks
	 * the protocol design and runs IPX over 802.3 without an 802.2 LLC
	 * layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
	 * won't work for fault tolerant netware but does for the rest.
	 */
	if (*(unsigned short *) rawp == 0xFFFF)
		return htons(ETH_P_802_3);

	/*
	 * Real 802.2 LLC
	 */
	return htons(ETH_P_802_2);
}
#endif /* FASTER_VERSION */

/*
 * We remember when the MAC gets set, so we don't override it later with
 * the ESI of the ATM card of the first VC
 */
static int (*my_eth_mac_addr)(struct net_device *, void *);
static int br2684_mac_addr(struct net_device *dev, void *p)
{
	int err = my_eth_mac_addr(dev, p);
	if (!err)
		BRPRIV(dev)->mac_was_set = 1;
	return err;
}

#ifdef CONFIG_ATM_BR2684_IPFILTER
/* this IOCTL is experimental. */
static int br2684_setfilt(struct atm_vcc *atmvcc, void __user *arg)
{
	struct br2684_vcc *brvcc;
	struct br2684_filter_set fs;

	if (copy_from_user(&fs, arg, sizeof fs))
		return -EFAULT;
	if (fs.ifspec.method != BR2684_FIND_BYNOTHING) {
		/*
		 * This is really a per-vcc thing, but we can also search
		 * by device
		 */
		struct br2684_dev *brdev;
		read_lock(&devs_lock);
		brdev = BRPRIV(br2684_find_dev(&fs.ifspec));
		if (brdev == NULL || list_empty(&brdev->brvccs) ||
		    brdev->brvccs.next != brdev->brvccs.prev)  /* >1 VCC */
			brvcc = NULL;
		else
			brvcc = list_entry_brvcc(brdev->brvccs.next);
		read_unlock(&devs_lock);
		if (brvcc == NULL)
			return -ESRCH;
	} else
		brvcc = BR2684_VCC(atmvcc);
	memcpy(&brvcc->filter, &fs.filter, sizeof(brvcc->filter));
	return 0;
}

/* Returns 1 if packet should be dropped */
static inline int
packet_fails_filter(u16 type, struct br2684_vcc *brvcc, struct sk_buff *skb)
{
	if (brvcc->filter.netmask == 0)
		return 0;			/* no filter in place */
	if (type == __constant_htons(ETH_P_IP) &&
	    (((struct iphdr *) (skb->data))->daddr & brvcc->filter.
	     netmask) == brvcc->filter.prefix)
		return 0;
	if (type == __constant_htons(ETH_P_ARP))
		return 0;
	/* TODO: we should probably filter ARPs too.. don't want to have
	 *   them returning values that don't make sense, or is that ok?
	 */
	return 1;		/* drop */
}
#endif /* CONFIG_ATM_BR2684_IPFILTER */

static void br2684_close_vcc(struct br2684_vcc *brvcc)
{
	DPRINTK("removing VCC %p from dev %p\n", brvcc, brvcc->device);
	write_lock_irq(&devs_lock);
	list_del(&brvcc->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->atmvcc->user_back = NULL;	/* what about vcc->recvq ??? */
	brvcc->old_push(brvcc->atmvcc, NULL);	/* pass on the bad news */
	kfree(brvcc);
	module_put(THIS_MODULE);
}

/* when AAL5 PDU comes in: */
static void br2684_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	struct br2684_vcc *brvcc = BR2684_VCC(atmvcc);
	struct net_device *net_dev = brvcc->device;
    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    struct atm_backend_br2684 be;
    unsigned short tag;
    char ifname[IFNAMSIZ];

	/*l65130 2008-09-20 start*/	
	int iLedState = kerSysGetLedState(kLedAdsl);
	/*l65130 2008-09-20 end*/

#ifdef CONFIG_BCM_VDSL
    if (g_nPtmTraffic)
    {
    /*start ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
        struct ptmdev *pdev = NULL;
        switch(g_nPtmConfig)
        {
        case  PTM_TRAFFIC ://ptm vlan
            tag  = *((unsigned short*)&(skb->data[14])) & 0x0fff;
            if(NULL == (pdev = find_ptmdev(tag)))
                net_dev = brvcc->device;
            else
                net_dev = pdev->net_dev;
            
            break;
        case  PTM_TRAFFIC_PRTCL://ptm protocol
            /*pppoe*/
            if(*((unsigned short*)&(skb->data[12]))== 0x8863 
             || *((unsigned short*)&(skb->data[12])) == 0x8864)
            {
                if(NULL == (pdev = find_ptmdev(PROTO_PPPOE)))
                    net_dev = brvcc->device;
                else
                    net_dev = pdev->net_dev;
            }
            else/*ipoe*/
            {
                if(NULL == (pdev = find_ptmdev(PROTO_MER)))
                    net_dev = brvcc->device;
                else
                    net_dev = pdev->net_dev;
            }
            break;
        case  PTM_TRAFFIC_BR: //ptm simple bridge
            net_dev = brvcc->device;
            break;
        default: //mode not be defined
             net_dev = brvcc->device;            
        }
    /*end ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
    #if 0
        be.ifspec.method =  BR2684_FIND_BYIFNAME;

        if ( skb->data[12] == 0x81 && skb->data[13] == 0x0) //differ vlan
        {
               tag  = *((unsigned short*)&(skb->data[14])) & 0x0fff;
               sprintf(ifname,"nas_%d_%d_%d",atmvcc->vpi,atmvcc->vci,tag);
               memcpy(be.ifspec.spec.ifname,ifname,IFNAMSIZ);
               net_dev = br2684_find_dev(&be.ifspec);
               if(NULL == net_dev)
                {
                       net_dev = brvcc->device;
                }
        }
        else //differ protocol
        {
                be.ifspec.method =  BR2684_FIND_BYIFNAME;
               if(*((unsigned short*)&(skb->data[12]))	 == 0x8863 || *((unsigned short*)&(skb->data[12])) == 0x8864)
                {
                	 sprintf(ifname,"nas_%d_%d_%d",atmvcc->vpi,atmvcc->vci,PPPOE_PROTOCOL );
                }
               else
               {
                 /* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
                 if (g_nPtmConfig == 2) //protocol ipoe
                 {
                     sprintf(ifname,"nas_%d_%d_%d",atmvcc->vpi,atmvcc->vci,IPOE_PROTOCOL);
                 }
                 else //protocol bridge
                 {
               	     sprintf(ifname,"nas_%d_%d_%d",atmvcc->vpi,atmvcc->vci,BRIDGE_PROTOCOL);
                 }
                 /* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
               }
               memcpy(be.ifspec.spec.ifname,ifname,IFNAMSIZ);
               net_dev = br2684_find_dev(&be.ifspec);
                if(NULL == net_dev)
                {
                       net_dev = brvcc->device;
                }
        }
        #endif
    }
#endif      
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

	struct br2684_dev *brdev = BRPRIV(net_dev);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifndef CONFIG_BCM_VDSL
	int plen = sizeof(llc_oui_pid_pad) + ETH_HLEN;
#else
  /* Ethernet packets from PTM traffic will not have an RFC header in place */
	int plen = ( !g_nPtmTraffic ) ? sizeof(llc_oui_pid_pad) + ETH_HLEN : ETH_HLEN; 
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

	DPRINTK("br2684_push\n");

	if (unlikely(skb == NULL)) {
		/* skb==NULL means VCC is being destroyed */
		br2684_close_vcc(brvcc);
		if (list_empty(&brdev->brvccs)) {
			read_lock(&devs_lock);
			list_del(&brdev->br2684_devs);
			read_unlock(&devs_lock);
			unregister_netdev(net_dev);
			free_netdev(net_dev);
		}
		return;
	}
#if defined(CONFIG_MIPS_BRCM)
	//skb->__unused=FROM_WAN;
#endif	

	atm_return(atmvcc, skb->truesize);
	DPRINTK("skb from brdev %p\n", brdev);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
    if ( !g_nPtmTraffic ) {
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
	if (brvcc->encaps == e_llc) {
		/* let us waste some time for checking the encapsulation.
		   Note, that only 7 char is checked so frames with a valid FCS
		   are also accepted (but FCS is not checked of course) */
		if (memcmp(skb->data, llc_oui_pid_pad, 7)) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);
			return;
		}

		/* Strip FCS if present */
		if (skb->len > 7 && skb->data[7] == 0x01)
			__skb_trim(skb, skb->len - 4);
	} else {
		plen = PADLEN + ETH_HLEN;	/* pad, dstmac,srcmac, ethtype */
		/* first 2 chars should be 0 */
		if (*((u16 *) (skb->data)) != 0) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);
			return;
		}
	}
	if (skb->len < plen) {
		brdev->stats.rx_errors++;
		dev_kfree_skb(skb);	/* dev_ not needed? */
		return;
	}
#ifdef SUPPORT_VLAN
   if (brvcc->vlan_id != 0xffff) { /*  Vcc was configured to be vlan tagged*/
		unsigned tmp[ATM_AND_MAC_LEN];
#ifdef VLAN_DEBUG
		printk("=====> before removing vlan id\n");
		skb_debug(skb);
#endif
/*
** There exist a situation where we tag vlan id upstream. But DSLAM sends untagged frame downstream.  So we need to check this situation before we move data around
*/
//eddie added if {}
	    if ( skb->data[22] == 0x81 && skb->data[23] == 0x0) {
		memcpy(tmp, skb->data, ATM_AND_MAC_LEN);
		skb_pull(skb, VLAN_HLEN);
		memcpy(skb->data, tmp, ATM_AND_MAC_LEN);
	    }
#ifdef VLAN_DEBUG
		printk("=====> after removing vlan id\n");
		skb_debug(skb);
#endif
	}
#endif // SUPPORT_VLAN
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
  } // !g_nPtmTraffic
  else 
  {
     #if 1
  	#ifdef SUPPORT_VLAN
       if (brvcc->vlan_id != 0xffff) { /*  Vcc was configured to be vlan tagged*/
		unsigned tmp[2 * VLAN_ETH_ALEN];
           #ifdef VLAN_DEBUG
		printk("=====> before removing vlan id\n");
		skb_debug(skb);
           #endif
/*
** There exist a situation where we tag vlan id upstream. But DSLAM sends untagged frame downstream.  So we need to check this situation before we move data around
*/
//eddie added if {}
	    if ( skb->data[12] == 0x81 && skb->data[13] == 0x0) {
		memcpy(tmp, skb->data, 2 * VLAN_ETH_ALEN);
		skb_pull(skb, VLAN_HLEN);
		memcpy(skb->data, tmp, 2 * VLAN_ETH_ALEN);
	    }
#ifdef VLAN_DEBUG
		printk("=====> after removing vlan id\n");
		skb_debug(skb);
#endif
	}
#endif // SUPPORT_VLAN
    #endif
  }
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

#ifdef FASTER_VERSION
	/* FIXME: tcpdump shows that pointer to mac header is 2 bytes earlier,
	   than should be. What else should I set? */
	skb_pull(skb, plen);
	skb->mac.raw = ((char *) (skb->data)) - ETH_HLEN;
	skb->pkt_type = PACKET_HOST;
#ifdef CONFIG_BR2684_FAST_TRANS
	skb->protocol = ((u16 *) skb->data)[-1];
#else				/* some protocols might require this: */
	skb->protocol = br_type_trans(skb, net_dev);
#endif /* CONFIG_BR2684_FAST_TRANS */
#else
	skb_pull(skb, plen - ETH_HLEN);
	skb->protocol = eth_type_trans(skb, net_dev);
#endif /* FASTER_VERSION */
#ifdef CONFIG_ATM_BR2684_IPFILTER
	if (unlikely(packet_fails_filter(skb->protocol, brvcc, skb))) {
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
#endif /* CONFIG_ATM_BR2684_IPFILTER */
#if defined(CONFIG_MIPS_BRCM)
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#if 0
	if (brvcc->proto_filter & FILTER_PPPOE) {
		if ((skb->protocol != htons(ETH_P_PPP_DISC)) && (skb->protocol != htons(ETH_P_PPP_SES))) {
			DPRINTK("non-PPPOE packet dropped on RX dev %s\n", net_dev->name);
			dev_kfree_skb(skb);
			return;
		}
	}
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#endif

        skb->dev = net_dev;
	ATM_SKB(skb)->vcc = atmvcc;	/* needed ? */
	DPRINTK("received packet's protocol: %x\n", ntohs(skb->protocol));

	if (unlikely(!(net_dev->flags & IFF_UP))) {
		/* sigh, interface is down */
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
/* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete begin:*/
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
/*#ifdef CONFIG_BCM_VDSL
	skb->len = (g_nPtmTraffic) ? (skb->len - 4) : skb->len ;
#endif*/
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
/* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete end.*/

	brdev->stats.rx_packets++;
	brdev->stats.rx_bytes += skb->len;
	memset(ATM_SKB(skb), 0, sizeof(struct atm_skb_data));

	netif_rx(skb);

    /*l65130 2008-09-20 start*/      
     /* BEGIN: Modified by c106292, 2009/4/15   PN: */
#if 0	 
    if (kLedStateOff !=  iLedState && kLedStateSlowBlinkContinues != iLedState &&kLedStateVerySlowBlinkContiunes!=iLedState)
	/* END:   Modified by c106292, 2009/4/15 */
    {
    	if ((NULL != skb->dev)&&( NULL == skb->dev->br_port ))
	    {
	    #if 0 	
	        static unsigned long last_time = 0;
	        if (jiffies - last_time > 20)
	        {
	            last_time = jiffies;
	            kerSysLedCtrl(kLedWanData, kLedStateBlinkOnce);
	        }
        #endif	    
             kerSysLedCtrl(kLedAdsl, kLedStateOn); /*c106292*/
      //      kerSysLedCtrl(kLedAdsl, kLedStateSlowFlickerOnce); 
	    }
    }
	#endif

    /*l65130 2008-09-20 end*/ 
}

static int br2684_regvcc(struct atm_vcc *atmvcc, void __user *arg)
{
/* assign a vcc to a dev
Note: we do not have explicit unassign, but look at _push()
*/
	int err;
	struct br2684_vcc *brvcc;
	struct sk_buff_head copy;
	struct sk_buff *skb;
	struct br2684_dev *brdev;
	struct net_device *net_dev;
	struct atm_backend_br2684 be;

	if (copy_from_user(&be, arg, sizeof be))
		return -EFAULT;
	brvcc = kmalloc(sizeof(struct br2684_vcc), GFP_KERNEL);
	if (!brvcc)
		return -ENOMEM;
	memset(brvcc, 0, sizeof(struct br2684_vcc));
	write_lock_irq(&devs_lock);
	net_dev = br2684_find_dev(&be.ifspec);
	if (net_dev == NULL) {
		printk(KERN_ERR
		    "br2684: tried to attach to non-existant device\n");
		err = -ENXIO;
		goto error;
	}
	brdev = BRPRIV(net_dev);
	if (atmvcc->push == NULL) {
		err = -EBADFD;
		goto error;
	}
	if (!list_empty(&brdev->brvccs)) {
		/* Only 1 VCC/dev right now */
		err = -EEXIST;
		goto error;
	}
	if (be.fcs_in != BR2684_FCSIN_NO || be.fcs_out != BR2684_FCSOUT_NO ||
	    be.fcs_auto || be.has_vpiid || be.send_padding || (be.encaps !=
	    BR2684_ENCAPS_VC && be.encaps != BR2684_ENCAPS_LLC) ||
	    be.min_size != 0) {
		err = -EINVAL;
		goto error;
	}
	DPRINTK("br2684_regvcc vcc=%p, encaps=%d, brvcc=%p\n", atmvcc, be.encaps,
		brvcc);
	if (list_empty(&brdev->brvccs) && !brdev->mac_was_set) {
		unsigned char *esi = atmvcc->dev->esi;
		if (esi[0] | esi[1] | esi[2] | esi[3] | esi[4] | esi[5])
			memcpy(net_dev->dev_addr, esi, net_dev->addr_len);
		else
			net_dev->dev_addr[2] = 1;
	}
	list_add(&brvcc->brvccs, &brdev->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->device = net_dev;
	brvcc->atmvcc = atmvcc;
	atmvcc->user_back = brvcc;
	brvcc->encaps = (enum br2684_encaps) be.encaps;
	brvcc->old_push = atmvcc->push;
#if defined(CONFIG_MIPS_BRCM)
	brvcc->proto_filter |= be.proto_filter;
#ifdef SUPPORT_VLAN
        brvcc->vlan_id = be.vlan_id;
#endif // SUPPORT_VLAN
#endif	
	barrier();
	atmvcc->push = br2684_push;
	skb_queue_head_init(&copy);
	skb_migrate(&atmvcc->sk->sk_receive_queue, &copy);
	while ((skb = skb_dequeue(&copy)) != NULL) {
		BRPRIV(skb->dev)->stats.rx_bytes -= skb->len;
		BRPRIV(skb->dev)->stats.rx_packets--;
		br2684_push(atmvcc, skb);
	}
	__module_get(THIS_MODULE);
	return 0;
    error:
	write_unlock_irq(&devs_lock);
	kfree(brvcc);
	return err;
}

static void br2684_setup(struct net_device *netdev)
{
	struct br2684_dev *brdev = BRPRIV(netdev);

	ether_setup(netdev);
	brdev->net_dev = netdev;

#ifdef FASTER_VERSION
	my_eth_header = netdev->hard_header;
	netdev->hard_header = br2684_header;
	my_eth_header_cache = netdev->hard_header_cache;
	netdev->hard_header_cache = br2684_header_cache;
	netdev->hard_header_len = sizeof(llc_oui_pid_pad) + ETH_HLEN;	/* 10 + 14 */
#endif
	my_eth_mac_addr = netdev->set_mac_address;
	netdev->set_mac_address = br2684_mac_addr;
	netdev->hard_start_xmit = br2684_start_xmit;
	netdev->get_stats = br2684_get_stats;

	INIT_LIST_HEAD(&brdev->brvccs);
}

static int br2684_create(void __user *arg)
{
	int err;
	struct net_device *netdev;
	struct br2684_dev *brdev;
	struct atm_newif_br2684 ni;

	DPRINTK("br2684_create\n");

	if (copy_from_user(&ni, arg, sizeof ni)) {
		return -EFAULT;
	}
    /* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 modify begin:
    /*start TDE MTU不能支持最大1500规避, s60000658, 20060624*/
	/*if (ni.media != BR2684_MEDIA_ETHERNET || ni.mtu > 1500 || ni.mtu < 46) {
		return -EINVAL;
	}*/
    /*end TDE MTU不能支持最大1500规避, s60000658, 20060624*/
    
	if (ni.media != BR2684_MEDIA_ETHERNET || ni.mtu != 1500) {
		return -EINVAL;
	}
	
    /* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 modify end. */

	netdev = alloc_netdev(sizeof(struct br2684_dev),
			      ni.ifname[0] ? ni.ifname : "nas%d",
			      br2684_setup);
	if (!netdev)
		return -ENOMEM;

    /* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete begin:
    netdev->mtu = ni.mtu;/*TDE MTU不能支持最大1500规避, s60000658, 20060624*/
    /* HUAWEI HGW s48571 2006年8月22日 FW9.1.13适配 delete end.*/
    
	brdev = BRPRIV(netdev);

	DPRINTK("registered netdev %s\n", netdev->name);
	/* open, stop, do_ioctl ? */
	err = register_netdev(netdev);
	if (err < 0) {
		printk(KERN_ERR "br2684_create: register_netdev failed\n");
		free_netdev(netdev);
		return err;
	}
    /*start ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
    do{
        struct ptmdev *pdev;

        
        if(ni.id < 0)
            break;   

        if(NULL != (pdev = find_ptmdev((unsigned short)ni.id)))
            break;

        if(NULL == (pdev = alloc_ptmdev()))
            break;

        pdev->id = (unsigned short)ni.id;
        pdev->net_dev = netdev;   

        register_ptmdev(pdev);
            
    }while(0);

	write_lock_irq(&devs_lock);
	brdev->number = list_empty(&br2684_devs) ? 1 :
	    BRPRIV(list_entry_brdev(br2684_devs.prev))->number + 1;
	list_add_tail(&brdev->br2684_devs, &br2684_devs);
	write_unlock_irq(&devs_lock);
	return 0;
}

/*
 * This handles ioctls actually performed on our vcc - we must return
 * -ENOIOCTLCMD for any unrecognized ioctl
 */
static int br2684_ioctl(struct socket *sock, unsigned int cmd,
	unsigned long arg)
{
	struct atm_vcc *atmvcc = ATM_SD(sock);
	void __user *argp = (void __user *)arg;

	int err;
	switch(cmd) {
	case ATM_SETBACKEND:
	case ATM_NEWBACKENDIF: {
		atm_backend_t b;
		err = get_user(b, (atm_backend_t __user *) argp);
		if (err)
			return -EFAULT;
		if (b != ATM_BACKEND_BR2684)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (cmd == ATM_SETBACKEND)
			return br2684_regvcc(atmvcc, argp);
		else
			return br2684_create(argp);
		}
#ifdef CONFIG_ATM_BR2684_IPFILTER
	case BR2684_SETFILT:
		if (atmvcc->push != br2684_push)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		err = br2684_setfilt(atmvcc, argp);
		return err;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	}
	return -ENOIOCTLCMD;
}

static struct atm_ioctl br2684_ioctl_ops = {
	.owner	= THIS_MODULE,
	.ioctl	= br2684_ioctl,
};


#ifdef CONFIG_PROC_FS
static void *br2684_seq_start(struct seq_file *seq, loff_t *pos)
{
	loff_t offs = 0;
	struct br2684_dev *brd;

	read_lock(&devs_lock);

	list_for_each_entry(brd, &br2684_devs, br2684_devs) {
		if (offs == *pos)
			return brd;
		++offs;
	}
	return NULL;
}

static void *br2684_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct br2684_dev *brd = v;

	++*pos;

	brd = list_entry(brd->br2684_devs.next, 
			 struct br2684_dev, br2684_devs);
	return (&brd->br2684_devs != &br2684_devs) ? brd : NULL;
}

static void br2684_seq_stop(struct seq_file *seq, void *v)
{
	read_unlock(&devs_lock);
}

static int br2684_seq_show(struct seq_file *seq, void *v)
{
	const struct br2684_dev *brdev = v;
	const struct net_device *net_dev = brdev->net_dev;
	const struct br2684_vcc *brvcc;

	seq_printf(seq, "dev %.16s: num=%d, mac=%02X:%02X:"
		       "%02X:%02X:%02X:%02X (%s)\n", net_dev->name,
		       brdev->number,
		       net_dev->dev_addr[0],
		       net_dev->dev_addr[1],
		       net_dev->dev_addr[2],
		       net_dev->dev_addr[3],
		       net_dev->dev_addr[4],
		       net_dev->dev_addr[5],
		       brdev->mac_was_set ? "set" : "auto");

	list_for_each_entry(brvcc, &brdev->brvccs, brvccs) {
		seq_printf(seq, "  vcc %d.%d.%d: encaps=%s"
#ifndef FASTER_VERSION
				    ", failed copies %u/%u"
#endif /* FASTER_VERSION */
				    "\n", brvcc->atmvcc->dev->number,
				    brvcc->atmvcc->vpi, brvcc->atmvcc->vci,
				    (brvcc->encaps == e_llc) ? "LLC" : "VC"
#ifndef FASTER_VERSION
				    , brvcc->copies_failed
				    , brvcc->copies_needed
#endif /* FASTER_VERSION */
				    );
#ifdef CONFIG_ATM_BR2684_IPFILTER
#define b1(var, byte)	((u8 *) &brvcc->filter.var)[byte]
#define bs(var)		b1(var, 0), b1(var, 1), b1(var, 2), b1(var, 3)
			if (brvcc->filter.netmask != 0)
				seq_printf(seq, "    filter=%d.%d.%d.%d/"
						"%d.%d.%d.%d\n",
						bs(prefix), bs(netmask));
#undef bs
#undef b1
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	}
	return 0;
}

static struct seq_operations br2684_seq_ops = {
	.start = br2684_seq_start,
	.next  = br2684_seq_next,
	.stop  = br2684_seq_stop,
	.show  = br2684_seq_show,
};

static int br2684_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &br2684_seq_ops);
}

static struct file_operations br2684_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = br2684_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

extern struct proc_dir_entry *atm_proc_root;	/* from proc.c */
#endif

static int __init br2684_init(void)
{
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_BCM_VDSL
     Init_CRC32_Table();
#endif
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *p;
	if ((p = create_proc_entry("br2684", 0, atm_proc_root)) == NULL)
		return -ENOMEM;
	p->proc_fops = &br2684_proc_ops;
#endif
	register_atm_ioctl(&br2684_ioctl_ops);
    /*start ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
    ptmhash_init();
    /*end ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
	return 0;
}

static void __exit br2684_exit(void)
{
	struct net_device *net_dev;
	struct br2684_dev *brdev;
	struct br2684_vcc *brvcc;
	deregister_atm_ioctl(&br2684_ioctl_ops);

#ifdef CONFIG_PROC_FS
	remove_proc_entry("br2684", atm_proc_root);
#endif

	while (!list_empty(&br2684_devs)) {
		net_dev = list_entry_brdev(br2684_devs.next);
		brdev = BRPRIV(net_dev);
		while (!list_empty(&brdev->brvccs)) {
			brvcc = list_entry_brvcc(brdev->brvccs.next);
			br2684_close_vcc(brvcc);
		}

		list_del(&brdev->br2684_devs);
		unregister_netdev(net_dev);
		free_netdev(net_dev);
	}
	/*start ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
    {
    int i;
    struct ptmdev *pdev = NULL;
    for (i = 0; i < PTM_IFC_HASH_MAX; i++)
    {
        if(list_empty(&g_ptmhash[i]))
            continue;

        list_for_each_entry_rcu(pdev, &g_ptmhash[i], list)
        {
            unregister_ptmdev(pdev);
        }
    }
    }
	/*end ptm模式下按接口分发报文性能优化, s60000658, 20060829*/
}

module_init(br2684_init);
module_exit(br2684_exit);

MODULE_AUTHOR("Marcell GAL");
MODULE_DESCRIPTION("RFC2684 bridged protocols over ATM/AAL5");
MODULE_LICENSE("GPL");
