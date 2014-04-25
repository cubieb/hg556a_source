/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.2.10.2 2009/11/26 03:43:06 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#if defined(CONFIG_MIPS_BRCM)
#include <linux/if_vlan.h>
#include <linux/timer.h>
#include <linux/igmp.h>
#endif
#include "br_private.h"
/*start of IGMP Snooping 功能支持 by l129990,2008,9,9*/	
#include "br_igmp_snooping.h"
#include <linux/seq_file.h>
/*end of IGMP Snooping 功能支持 by l129990,2008,9,9*/

/*start of 删除snooping中的block模式功能by l129990,2008,9,9*/	
//#if defined(CONFIG_MIPS_BRCM)
//#define SNOOPING_BLOCKING_MODE 2
//#endif
/*end of 删除snooping中的block模式功能by l129990,2008,9,9*/

/*start  of 增加br中对dhcp广播报文的处理，避免广播到公网上去*/
#ifdef CONFIG_PORTMAPING
#define DHCP_OPTION           264
#define DHCP_OPTION_LEN   268
extern char dhcpoption[DHCP_OPTION]; 
extern struct mc_check_list g_mcChecklist;
#endif
 /*end   of 增加br中对dhcp广播报文的处理，避免广播到公网上去*/

const unsigned char bridge_ula[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
/*start  of 增加br中组播报文的处理，来控制组播报文by s53329  at  20070802*/
#ifdef CONFIG_PORTMAPING
#define SKB_MAC_SOURCE(skb) (skb->mac.ethernet->h_source)
static int mc_access_check(struct sk_buff *skb)
{
    struct mc_check_node *pnode = NULL;
    
    if(!g_mcChecklist.inited )
    {
        return 0;
    }
    if (strncmp(skb->dev->name, "nas", 3) == 0)
    {
        return 1;
    }

    spin_lock_bh(&g_mcChecklist.lock);
    list_for_each_entry_rcu(pnode, &g_mcChecklist.acc_list, list)
    {
        if(memcmp(SKB_MAC_SOURCE(skb), pnode->src_mac, 6) == 0)
        {
            spin_unlock_bh(&g_mcChecklist.lock);
            return 1;
        }
    }
    spin_unlock_bh(&g_mcChecklist.lock);
    return 0;
}
#endif
/*end   of 增加br中组播报文的处理，来控制组播报文by s53329  at  20070802*/


static int br_pass_frame_up_finish(struct sk_buff *skb)
{
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
#if defined(CONFIG_MIPS_BRCM)
	/* If pass up to IP, remove VLAN header */
	if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
		unsigned short proto;
		struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);

		skb = skb_share_check(skb, GFP_ATOMIC);
		if (skb) {
			memmove(skb->data - ETH_HLEN + VLAN_HLEN,
				skb->data - ETH_HLEN, 12);
			skb_pull(skb, VLAN_HLEN);
			skb->mac.raw += VLAN_HLEN;
			skb->nh.raw += VLAN_HLEN;
			skb->h.raw += VLAN_HLEN;
		}
		/* make sure protocol is correct before passing up */
		proto = vhdr->h_vlan_encapsulated_proto;
		skb->protocol = proto;
		/* TODO: do we need to assign skb->priority? */
	}
#endif
	netif_rx(skb);

	return 0;
}

static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;
    /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN 
	struct vlan_hdr *vhdr = NULL;
    if(br->vlan_enable && (skb->protocol == __constant_htons(ETH_P_8021Q)))
    {
        vhdr = (struct vlan_hdr *)(skb->data);
        if(ISVALID_VID(br->mngr_vid)
         &&(br->mngr_vid != VLAN_GET_VID(vhdr->h_vlan_TCI)))
        {
            kfree_skb(skb);
            return;
        }
    }
#endif //CONFIG_KPN_VLAN
    /*end vlan bridge, s60000658, 20060627*/
    
	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = br->dev;

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
			br_pass_frame_up_finish);
}

/*start of 删除brcm原有的组播snooping功能函数 by l129990 2008,9,8*/
#if 0
void query_timeout(unsigned long ptr)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *tmp;
	struct list_head *lh;
	struct net_bridge *br;
    
	br = (struct net_bridge *) ptr;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (jiffies > dst->tstamp) {
		list_del_rcu(&dst->list);
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
		
	mod_timer(&br->igmp_timer, jiffies + TIMER_CHECK_TIMEOUT*HZ);		
}

void addr_debug(unsigned char *dest)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < 6 && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & dest[i]);
	}
	printk("%s ", buf);
}

#if defined(CONFIG_MIPS_BRCM)
void addr_conv(unsigned char *in, char * out)
{
    sprintf(out, "%02x%02x%02x%02x%02x%02x", in[0], in[1], in[2], in[3], in[4], in[5]);
}

mac_addr upnp_addr = {{0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa}};
mac_addr sys1_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x01}};
mac_addr sys2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x02}};
mac_addr ospf1_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x05}};
mac_addr ospf2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x06}};
mac_addr ripv2_addr = {{0x01, 0x00, 0x5e, 0x00, 0x00, 0x09}};
mac_addr sys_addr = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

int control_filter(unsigned char *dest)
{
    if ((!memcmp(dest, &upnp_addr, ETH_ALEN)) ||
	(!memcmp(dest, &sys1_addr, ETH_ALEN)) ||
	(!memcmp(dest, &sys2_addr, ETH_ALEN)) ||
	(!memcmp(dest, &ospf1_addr, ETH_ALEN)) ||
	(!memcmp(dest, &ospf2_addr, ETH_ALEN)) ||
	(!memcmp(dest, &sys_addr, ETH_ALEN)) ||
	(!memcmp(dest, &ripv2_addr, ETH_ALEN)))
	    return 0;
    else
	return 1;
}
/*start of BRIDGE <3.3.2组播成员离开导致其他成员暂停> porting by s60000658 20060506*/
int br_mc_fdb_empty(struct net_bridge *br, unsigned char *dest)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
    
	spin_lock_bh(&br->mcl_lock);
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *)list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN))) {
			spin_unlock_bh(&br->mcl_lock);
	    	return 0;
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
	return 1;
}
/*end of BRIDGE <3.3.2组播成员离开导致其他成员暂停> porting by s60000658 20060506*/

int mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	int status = 0;
	struct sk_buff *skb2;
	struct net_bridge_port *p;
	unsigned char tmp[6];

    /*start vlan bridge, s60000658, 20060627*/
    unsigned char *data = skb->data;
    unsigned short protocol ;
#ifdef CONFIG_KPN_VLAN
    struct vlan_hdr   *vlhdr = NULL;
    struct vlan_8021q *pvlan;

    if(br->vlan_enable)
    {
        int ntag = 0;
        while(__constant_htons(ETH_P_8021Q) == ((u16 *) data)[-1] && (ntag < 3))
        {
             data += VLAN_HLEN;
             ntag++;
        }
    }
#endif //CONFIG_KPN_VLAN
       protocol = ((u16 *) data)[-1];
/*start  of 增加br中组播报文的处理，来控制组播报文*/
#ifdef CONFIG_PORTMAPING
        if (data[9] == IPPROTO_IGMP)
        {
            if(!mc_access_check(skb))
            {
                if(!forward)
                {
                     kfree_skb(skb);
                }
                return 1;
            }
        }
#endif
/*end   of 增加br中组播报文的处理，来控制组播报文*/
        
	if (!snooping || protocol != __constant_htons(ETH_P_IP))//if (!snooping)/*BRIDGE <3.3.3未判断协议类型导致内核崩溃> porting by s60000658 20060506*/
		return 0;

	if ((snooping == SNOOPING_BLOCKING_MODE) && control_filter(dest))
	    status = 1;

	if (data[9] == IPPROTO_IGMP) {
	    // For proxy; need to add some intelligence here 
	    if (!br->proxy) {
		if (data[24] == IGMPV2_HOST_MEMBERSHIP_REPORT &&
		    protocol == __constant_htons(ETH_P_IP))
		    br_mc_fdb_add(br, skb->dev->br_port, dest, skb->mac.ethernet->h_source);
		else if (data[24] == IGMP_HOST_LEAVE_MESSAGE) {
		    tmp[0] = 0x01;
		    tmp[1] = 0x00;
		    tmp[2] = 0x5e;
		    tmp[3] = 0x7F & data[29];
		    tmp[4] = data[30];
		    tmp[5] = data[31];
		    br_mc_fdb_remove(br, skb->dev->br_port, tmp, skb->mac.ethernet->h_source);
            /*start of BRIDGE <3.3.2组播成员离开导致其他成员暂停(snooping)> porting by s60000658 20060506*/
		    if (br_mc_fdb_empty(br, tmp))
	        {
                skb2 = skb_clone(skb, GFP_ATOMIC);
	          /*add by z67625 for dst cache overflow*/      
			  if(forward)
				 br_flood_forward(br, skb2, 0);
			  else
				 br_flood_deliver(br, skb2, 0);
			/*add by z67625 for dst cache overflow*/
	        }		    
	        status = 1;
            /*end of BRIDGE <3.3.2组播成员离开导致其他成员暂停> porting by s60000658 20060506*/
		}
		else
		    ;
	    }
	     /*start of BRIDGE <3.3.2组播成员离开导致其他成员暂停(proxy)> porting by s60000658 20060525*/
	    else if (IGMP_HOST_LEAVE_MESSAGE == data[24])
        {
            status = 1;
        }
	     /*end of BRIDGE <3.3.2组播成员离开导致其他成员暂停(proxy)> porting by s60000658 20060525*/
	      /*add by z67625 for dst cache overflow*/      
	     if((!forward)&& status)
		     kfree_skb(skb);
	      /*add by z67625 for dst cache overflow*/      
	    return status;
	}
    /*end vlan bridge, s60000658, 20060627*/
    
	/*
	if (clone) {
		struct sk_buff *skb3;

		if ((skb3 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb3;
	}
	*/
	
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (!memcmp(&dst->addr, dest, ETH_ALEN)) {
	    /* Start of igmp snooping by f60014464 20060628 */
	    if (NULL == dst->dst)
	    {
	        continue;
	    }
	    /* End of igmp snooping by f60014464 20060628 */
        /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
        if(br->vlan_enable && (__constant_htons(ETH_P_8021Q) == skb->protocol))
        {
            vlhdr = (struct vlan_hdr*)skb->data;
            pvlan = br_get_vlan(br, VLAN_GET_VID(vlhdr->h_vlan_TCI));

            if((NULL != pvlan)
             && (ISIN_VLAN(pvlan->ports, dst->dst->port_no))
             && (!dst->dst->dirty))
            {
                skb2 = skb_clone(skb, GFP_ATOMIC);
    		    if (forward)
        			br_forward(dst->dst, skb2);
    		    else
        			br_deliver(dst->dst, skb2);
            }
            
            if( NULL != pvlan ) 
            {
                br_release_vlan(pvlan);
            }
        }
        else 
#endif //CONFIG_KPN_VLAN
        if (!dst->dst->dirty) {
		    skb2 = skb_clone(skb, GFP_ATOMIC);
		    if (forward)
			br_forward(dst->dst, skb2);
		    else
			br_deliver(dst->dst, skb2);
		}
		dst->dst->dirty = 1;
		status = 1;
		/*end vlan bridge, s60000658, 20060627*/
	    }	    
	}
	if (status) {
	    list_for_each_entry_rcu(p, &br->port_list, list) {
		p->dirty = 0;
	  }
	}

	if ((!forward) && (status))
	kfree_skb(skb);

	return status;
}
#endif
#endif
/*end of 删除brcm原有的组播snooping功能函数 by l129990 2008,9,8*/
/*start of 删除br中对dhcp广播报文的处理，避免广播到公网上去 by l129990 2008,9,8*/
#if 0
/*start  of 增加br中对dhcp广播报文的处理，避免广播到公网上去*/
#ifdef  CONFIG_PORTMAPING
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
#define DHCP_VENDOR 0x3c

#define DHCP_PADDING	0x00
#define DHCP_OPTION_OVER 0x34
#define DHCP_END		0xFF
static unsigned char *get_option60(unsigned char* data, int code)
{
	int i, length;
	static char err[] = "bogus packet, option fields too long."; /* save a few bytes */
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	optionptr = data;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			return optionptr + i ;
		}			
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				//optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				//optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
    int check_option60(unsigned char *optionstr, unsigned char *vendorstr)
    {
        char r1, r2;
        char *l;
        if (optionstr == NULL || vendorstr == NULL)
        {
            return 0;
        }
         if (!strlen(vendorstr))
    	  return 0;
        if(!strlen(optionstr))
    	  return 0;
        char *end = optionstr + strlen(optionstr)-1;
        l = strstr(optionstr, vendorstr);
        for (;;) {
            r1 =0; r2=0;
            if (l != NULL) {
                if (l == optionstr) {
                    r1 = 1;
                }
                else { // need to see | if not the first one
                    if (*(l-1) == '|') {
                        r1 = 1;
                    }
                }
                
                if (*(l + strlen(vendorstr)) == 0 || *(l + strlen(vendorstr)) == '|' ) { //need to see |if not last one
                    r2 = 1;
                }

                if (r1 && r2) {
                    /*if (loc != NULL) {
                        *loc = l;
                    }*/
                    return 1;
                }
                else {
                    if (l < end) {
                        l = strstr(l+1, vendorstr);
                    }
                }
            }
            else {
                return 0;
            }
        }
    }
#endif
/*end   of 增加br中对dhcp广播报文的处理，避免广播到公网上去*/
#endif
/*end of 删除br中对dhcp广播报文的处理，避免广播到公网上去 by l129990 2008,9,8*/

/*start of 移植ATP的建立IP和MAC影射表功能 by l129990,2008,9,9/
/*start of ATP 2008.05.05 获取lan侧发送组播报文其IP和MAC地址对应关系以便协议栈记录日志使用 add by h00101845*/
struct mc_ip_mac
{
    struct list_head     ipmac_list;
    struct timer_list    ipmac_timer;
    spinlock_t           ipmac_lock;
    int                  ipmac_timer_start;
    int                  count;
};

struct ip_mac_entry
{
    struct list_head    list;
    char ip[16];
    unsigned char mac[ETH_ALEN];
    long   time;
};

struct mc_ip_mac  mim_list;

#define IP_MAC_TIMEOUT        135
#define IP_MAC_CHECK_TIMEOUT  2

static void ip_mac_timeout(unsigned long data)
{
    struct ip_mac_entry  *mim_entry;
    struct mc_ip_mac     *im_list;

    im_list = (struct mc_ip_mac*)data;

    spin_lock_bh(&im_list->ipmac_lock);
	list_for_each_entry_rcu(mim_entry, &im_list->ipmac_list, list) 
    {
	    if ((jiffies > mim_entry->time) && ((jiffies - mim_entry->time) < (IP_MAC_TIMEOUT*HZ)))
	    {
    		list_del_rcu(&mim_entry->list);
            mim_list.count--;
    		kfree(mim_entry);
	    }
	}
	spin_unlock_bh(&im_list->ipmac_lock);
		
	mod_timer(&im_list->ipmac_timer, jiffies + IP_MAC_CHECK_TIMEOUT*HZ);	
}

static int ip_mac_add(unsigned char *host_mac, unsigned char *host_ip)
{
    struct ip_mac_entry *im_entry;

    list_for_each_entry_rcu(im_entry, &mim_list.ipmac_list, list)
    {
        if (!memcmp(im_entry->mac, host_mac, ETH_ALEN))
        {
            if (!memcmp(im_entry->ip, host_ip, 16))
            {
                im_entry->time = jiffies + IP_MAC_TIMEOUT*HZ;
                return 0;
            }
            else
            {
                spin_lock_bh(&mim_list.ipmac_lock);
                list_del_rcu(&im_entry->list);
                mim_list.count--;
        		kfree(im_entry);
                spin_unlock_bh(&mim_list.ipmac_lock);
            }
        }
    }

    im_entry = kmalloc(sizeof(struct ip_mac_entry), GFP_KERNEL);
    if (!im_entry)
	{
	    return -ENOMEM;
	}

    memcpy(im_entry->mac, host_mac, ETH_ALEN);	
	memcpy(im_entry->ip, host_ip, 16);
	im_entry->time = jiffies + IP_MAC_TIMEOUT*HZ;

    spin_lock_bh(&mim_list.ipmac_lock);
	list_add_rcu(&im_entry->list, &mim_list.ipmac_list);
	spin_unlock_bh(&mim_list.ipmac_lock);

	if (!mim_list.ipmac_timer_start) 
	{
    	init_timer(&mim_list.ipmac_timer);
	    mim_list.ipmac_timer.expires = jiffies + IP_MAC_CHECK_TIMEOUT*HZ;
	    mim_list.ipmac_timer.function = ip_mac_timeout;
	    mim_list.ipmac_timer.data = (unsigned long)(&mim_list);
	    add_timer(&mim_list.ipmac_timer);
	    mim_list.ipmac_timer_start = 1;
        mim_list.count++;
	}

    return 0;
}

static void ip_match_mac(struct sk_buff *skb)
{
    unsigned char *data = skb->data;
    unsigned short protocol ;
    unsigned char ipaddr[16] = {0};
    protocol = ((u16 *) data)[-1];
    sprintf(ipaddr, "%u.%u.%u.%u", data[12], data[13], data[14], data[15]);

    if (data[9] == IPPROTO_IGMP && protocol == __constant_htons(ETH_P_IP))
    {
#if 0        
        printk("************%s, ipaddr: [%s], macaddr: [%02x:%02x:%02x:%02x:%02x:%02x]**********\r\n", __FUNCTION__, ipaddr, 
            skb->mac.ethernet->h_source[0],
            skb->mac.ethernet->h_source[1],
            skb->mac.ethernet->h_source[2],
            skb->mac.ethernet->h_source[3],
            skb->mac.ethernet->h_source[4],
            skb->mac.ethernet->h_source[5]);
#endif
        ip_mac_add(skb->mac.ethernet->h_source, ipaddr);              
    }

    return ;
}

#ifdef CONFIG_PROC_FS

static rwlock_t ip_mac_lock = RW_LOCK_UNLOCKED;

struct ip_mac_iter
{
    int ct;
};

static struct ip_mac_entry *ip_mac_seq_idx(struct ip_mac_iter *iter,
					   loff_t pos)
{
    struct ip_mac_entry *im_entry = NULL;
    struct list_head *lh;

    list_for_each_entry_rcu(im_entry, &mim_list.ipmac_list, list)
    {
        if (pos-- == 0)
        {
            return im_entry;
        }
    }

	return NULL;   
}

static int ip_mac_flg = 0;

static void *mac_mc_seq_start(struct seq_file *seq, loff_t *pos)
{
    read_lock(&ip_mac_lock);    
    return *pos ? ip_mac_seq_idx(seq->private, *pos - 1) 
		: SEQ_START_TOKEN;
}

static void *mac_mc_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
    struct ip_mac_entry *im_entry;
    struct ip_mac_iter *iter = seq->private;
    
    ++*pos;
	if (v == SEQ_START_TOKEN)
	{
		return ip_mac_seq_idx(iter, 0);
	}
    
    if (ip_mac_flg < (mim_list.count - 1))
    {
        ip_mac_flg++;
        im_entry = (struct ip_mac_entry*)(((struct ip_mac_entry*)v)->list.next);
        return im_entry;
    }
    
	return NULL;
}

static void mac_mc_seq_stop(struct seq_file *seq, void *v)
{
    ip_mac_flg = 0;
	read_unlock(&ip_mac_lock);
}


static int mac_mc_seq_show(struct seq_file *seq, void *v)
{
    struct mc_ip_mac    *mim;

	spin_lock_bh(&mim->ipmac_lock);
    if (v == SEQ_START_TOKEN) 
    {
		seq_puts(seq, 
			 "IP Address            HW Address\n");
	}
    else if (v != NULL)
    {
        struct ip_mac_entry *im_entry = v;
        seq_printf(seq, "%-16s      %02x:%02x:%02x:%02x:%02x:%02x", 
               im_entry->ip,
        	   im_entry->mac[0],
        	   im_entry->mac[1],
        	   im_entry->mac[2],
        	   im_entry->mac[3],
        	   im_entry->mac[4],
        	   im_entry->mac[5]);
        seq_putc(seq, '\n');
    }
        
	spin_unlock_bh(&mim->ipmac_lock);
	return 0;
}

static struct seq_operations mac_mc_seq_ops = {
	.start = mac_mc_seq_start,
	.next  = mac_mc_seq_next,
	.stop  = mac_mc_seq_stop,
	.show  = mac_mc_seq_show,
};

static int mac_mc_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &mac_mc_seq_ops);
}

static struct file_operations mac_mc_seq_fops = {
	.owner	 = THIS_MODULE,
	.open    = mac_mc_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

#endif
void __init br_ip_mac_mc_init(void)
{
    INIT_LIST_HEAD(&mim_list.ipmac_list);
	mim_list.ipmac_lock = SPIN_LOCK_UNLOCKED;
    mim_list.ipmac_timer_start = 0;
    mim_list.count = 0;

    proc_net_fops_create("mac_mcast", 0, &mac_mc_seq_fops);
}
/*start of ATP 2008.05.05 获取lan侧发送组播报文其IP和MAC地址对应关系以便协议栈记录日志使用 add by h00101845*/
/*end of 移植ATP的建立IP和MAC影射表功能 by l129990,2008,9,9 */

int br_handle_frame_finish(struct sk_buff *skb)
{
	struct net_bridge *br;
	unsigned char *dest;
#if defined(CONFIG_MIPS_BRCM)
	unsigned char *src;
#endif
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_port *p;
	int passedup;

	dest = skb->mac.ethernet->h_dest;
#if defined(CONFIG_MIPS_BRCM)
	src = skb->mac.ethernet->h_source;
#endif
	
	rcu_read_lock();
	p = skb->dev->br_port;
	smp_read_barrier_depends();

	if (p == NULL || p->state == BR_STATE_DISABLED) {
		kfree_skb(skb);
		goto out;
	}

	br = p->br;
	passedup = 0;
	if (br->dev->flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			passedup = 1;
			br_pass_frame_up(br, skb2);
		}
	}
/*start of 删除brcm原有的组播snooping功能 by l129990 2008,9,8*/  
#if 0
	if (dest[0] & 1) {
#if defined(CONFIG_MIPS_BRCM)
		if (snooping && br->proxy) {
           /*start vlan bridge, s60000658, 20060627*/
	       unsigned char *data = skb->data;
#ifdef CONFIG_KPN_VLAN
           if(br->vlan_enable)
           {
               int ntag = 0;
               while(__constant_htons(ETH_P_8021Q) == ((u16 *) data)[-1] && (ntag < 3))
               {
                    data += VLAN_HLEN;
                    ntag++;
               }
           }
           
	       if(DEBUG_VBR){
    	        VLAN_DUMP_SKB(skb, "igmp proxy*snooping");
    	            printk("data begin at: delta %d, data %x\n", data - skb->data, data[0]);
	        }
#endif// CONFIG_KPN_VLAN
               
               
                if (data[9] == IPPROTO_IGMP) {
		    char destS[16];
		    char srcS[16];
		    if (data[24] == IGMP_HOST_LEAVE_MESSAGE) {
			unsigned char tmp[6];
			
			tmp[0] = 0x01;
			tmp[1] = 0x00;
			tmp[2] = 0x5e;
			tmp[3] = 0x7F & data[29];
			tmp[4] = data[30];
			tmp[5] = data[31];
			addr_conv(tmp, destS);
		    }
		    else
			addr_conv(dest, destS);
		    addr_conv(src, srcS);
		    sprintf(skb->extif, "%s %s %s/%s", br->dev->name, p->dev->name, destS, srcS);
		  }
		  /*end vlan bridge, s60000658, 20060627*/
		}
		/*start  of 增加br中对dhcp广播报文的处理，避免广播到公网上去by s53329*/
	  #ifdef CONFIG_PORTMAPING
                if (ntohs(skb->protocol) == 0x0800 )
                {
		        //if((dest[0]== 0xff) && (dest[1]== 0xff) &&(dest[2]== 0xff)  && (dest[3]== 0xff))
		         if((dest[0]== 0xff))
        		  {
        			  if(skb->data[9] == IPPROTO_UDP)
        			  {
                                    //判断是否溢出，或者用宏来表示
        			        if(skb->data[20] == 0x00 && skb->data[21] == 0x44 
        			        	&& skb->data[22] == 0x00 && skb->data[23] == 0x43)
        		        	 {
                                           if (skb->len <= DHCP_OPTION_LEN)
                                           {
                                                br_flood_forward(br, skb, !passedup);
                                                goto out;
                                           }
                                            unsigned char * option = get_option60(&skb->data[DHCP_OPTION_LEN], DHCP_VENDOR);
                                            unsigned char tmpbuff[32];
                                            memset(tmpbuff, 0, 32);

                                            if(NULL != option)
                                            {
                                                char *tmppoint = option;
                                                memcpy(tmpbuff,tmppoint + 2,*(tmppoint + 1));
                                            }
                                            if(check_option60(dhcpoption, tmpbuff) == 0)
                                            {
                                                br_pass_frame_up(br,skb);
                                                goto out;
                                            }
                                            else
                                            {
                                                br_flood_forward(br, skb, !passedup);
                                                if (!passedup)
                                                {
                                                    br_pass_frame_up(br, skb);
                                                }
                                                goto out;
                                            }
        			        }
        			    }
        		    }
                    }
                
		   #endif
		  /*end   of 增加br中对dhcp广播报文的处理，避免广播到公网上去by s53329*/
		if (!mc_forward(br, skb, dest, 1, !passedup))		
#endif
		br_flood_forward(br, skb, !passedup);
		if (!passedup)
			br_pass_frame_up(br, skb);
		goto out;
	}
#endif
/*end of 删除brcm原有的组播snooping功能 by l129990 2008,9,8*/ 

/*start of support to the dhcp relay  function by l129990,2009,11,12*/	
    if (ntohs(skb->protocol) == 0x0800 )
    {
        if((dest[0]== 0xff))
        {
            if(skb->data[9] == IPPROTO_UDP)
            {        
                if(skb->data[20] == 0x00 && skb->data[21] == 0x44 
                	&& skb->data[22] == 0x00 && skb->data[23] == 0x43)
            	 {
                     if( dhcprelayCtl == 0)
                     {
                         if (!passedup)
                          {
                              br_pass_frame_up(br, skb);
                          }
                          goto out;
                     }
                     else
                     {
                          br_flood_forward(br, skb, !passedup);
                          goto out;
                     }  
                 }  
             }
         }
    }   
/*end of support to the dhcp relay  function by l129990,2009,11,12*/	

/*start of IGMP Snooping 功能支持 by l129990,2008,9,8*/	
    if (dest[0] & 1) {
/*add by h00101845 for igmp log 2008-05-05*/
        ip_match_mac(skb);
/*add by h00101845 for igmp log 2008-05-05*/

    if (!br_igmp_snooping_forward(skb,br,dest, 1))  
          br_flood_forward(br, skb, !passedup);
          if (!passedup)
               br_pass_frame_up(br, skb);
          goto out;
     }
/*end of IGMP Snooping 功能支持 by l129990,2008,9,8*/	

	dst = __br_fdb_get(br, dest);
	if (dst != NULL && dst->is_local) {
		if (!passedup)
			br_pass_frame_up(br, skb);
		else
			kfree_skb(skb);
		goto out;
	}

	if (dst != NULL) {
	    /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
    	if (br->vlan_enable && (skb->protocol == __constant_htons(ETH_P_8021Q))) {
    		struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);
    		struct vlan_8021q *pvlan = br_get_vlan(br, vhdr->h_vlan_TCI & VLAN_VID_MASK);
    		    
    		if( NULL != pvlan )
		    {
		        if(pvlan->macln_enable && ISIN_VLAN(pvlan->ports , dst->dst->port_no))
	            {
    	    		br_forward(dst->dst, skb);
			        br_release_vlan(pvlan);    
    		        goto out;
	            }		        
                br_release_vlan(pvlan);    
		    }
    		else
		    {
    		    kfree_skb(skb);
		        goto out;
		    }
    		
	    }
	    else
#endif //CONFIG_KPN_VLAN
        {
    		br_forward(dst->dst, skb);
    		goto out;
        }
		/*end vlan bridge, s60000658, 20060627*/
	}

	br_flood_forward(br, skb, 0);

out:
	rcu_read_unlock();
	return 0;
}

int br_handle_frame(struct sk_buff *skb)
{
	unsigned char *dest;
	struct net_bridge_port *p;
    /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
    struct sk_buff *skb2;
    unsigned short tag = 0;
    struct vlan_8021q* pvlan = NULL;
    int hastag = 0;
#endif// CONFIG_KPN_VLAN
    /*end vlan bridge, s60000658, 20060627*/
	dest = skb->mac.ethernet->h_dest;

	rcu_read_lock();
	p = skb->dev->br_port;
	if (p == NULL || p->state == BR_STATE_DISABLED)
		goto err;

	if (skb->mac.ethernet->h_source[0] & 1)
		goto err;

	/*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
    if(p->br->vlan_enable)
    {
    	skb->data -= ETH_HLEN;
    	skb->len += ETH_HLEN;
    	if(DEBUG_VBR)VLAN_DUMP_SKB(skb, "bridge rcv skb");
        switch( p->mode )
        {
        case BR_PORT_MODE_NORMAL:
            /*do nothing*/
            break;
        case BR_PORT_MODE_ACCESS:
            if(p->prio_enable)
            {
                tag = p->tci;
            }
            else 
            {
                (void)vlan_get_tag(skb, &tag);
                VLAN_SET_VID(tag, VLAN_GET_VID(p->tci));
            }
                            
            skb2 = vlan_put_tag(skb, tag);
            
            if(NULL != skb2)
            {
                skb = skb2;
                hastag = 1;
            } 
            else
            {
                goto err;
            }
            
            break;
        case BR_PORT_MODE_TRUNK:
            if(0 != vlan_get_tag(skb, &tag))
            {
                if(p->prio_enable)
                {
                    tag = p->tci;
                }
                else
                {
                    VLAN_SET_VID(tag, VLAN_GET_VID(p->tci));
                }

                skb2 = vlan_put_tag(skb, tag);
                
                if(NULL != skb2)
                {
                    skb = skb2;
                }
                else
                {
                    goto err;                
                }
            }
            hastag = 1;
                       
            break;
        case BR_PORT_MODE_HYBRID:
            /*not surported*/
            break;
        case  BR_PORT_MODE_UNKNOWN:
        default:
            printk("%s: vlan port(%s) mode %d is unknown\n",
                __FUNCTION__, skb->dev->name, skb->dev->br_port->mode);
            goto err;
        }
    	if(DEBUG_VBR)VLAN_DUMP_SKB(skb, "bridge rcv skb(after do vlan)");
    	skb->data += ETH_HLEN;
    	skb->len -= ETH_HLEN; 
    }
    dest = skb->mac.ethernet->h_dest;
#endif //CONFIG_KPN_VLAN
    /*end vlan bridge, s60000658, 20060627*/

	if (p->state == BR_STATE_LEARNING ||
	    p->state == BR_STATE_FORWARDING)
    /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
        if(p->br->vlan_enable)
        {
           do{
                if(0 == hastag)
                    break;
                /*start A36D02417管理vlan mac学习, s60000658, 20060803*/
                if(VLAN_GET_VID(tag) == p->br->mngr_vid)
                {
                    br_fdb_insert(p->br, p, skb->mac.ethernet->h_source, 0);
                    break;
                }
                /*end A36D02417管理vlan mac学习, s60000658, 20060803*/
                pvlan = br_get_vlan(p->br, VLAN_GET_VID(tag));
                
                if(NULL == pvlan)
                    break;

                if(pvlan->macln_enable && ISIN_VLAN(pvlan->ports, p->port_no))
                {
                    br_fdb_insert(p->br, p, skb->mac.ethernet->h_source, 0);                        
                }
                br_release_vlan(pvlan);
               
            }while(0);
        }
        else
#endif //CONFIG_KPN_VLAN
            br_fdb_insert(p->br, p, skb->mac.ethernet->h_source, 0);
    
    /*end vlan bridge, s60000658, 20060627*/
    
	if (p->br->stp_enabled &&
	    !memcmp(dest, bridge_ula, 5) &&
	    !(dest[5] & 0xF0)) {
		if (!dest[5]) {
			NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev, 
				NULL, br_stp_handle_bpdu);
			rcu_read_unlock();
			return 0;
		}
	}

	else if (p->state == BR_STATE_FORWARDING) {
		if (br_should_route_hook && br_should_route_hook(&skb)) {
			rcu_read_unlock();
			return -1;
		}

		if (!memcmp(p->br->dev->dev_addr, dest, ETH_ALEN))
			skb->pkt_type = PACKET_HOST;
#if 0
		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL, br_handle_frame_finish);
#else
		br_handle_frame_finish(skb);
#endif
		rcu_read_unlock();
		return 0;
	}

err:
	rcu_read_unlock();
	kfree_skb(skb);
	return 0;
}
