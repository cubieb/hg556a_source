/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.2.10.1 2009/10/28 14:26:33 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_bridge.h>
/*start of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
#include <linux/ip.h>
/*end of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
#include "br_private.h"
/*start of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
#include "br_igmp_snooping.h"

#define IGMPV2_HOST_MEMBERSHIP_REPORT	0x16	/* V2 version of 0x11 */
#define IGMP_HOST_MEMBERSHIP_REPORT	 0x12
#define IGMPV3_HOST_MEMBERSHIP_REPORT	0x22
#define IPPROTO_IGMP  2
#define INADDR_UPNP  0xEFFFFFFAU

extern struct net_bridge_igmp_snooping igmp_snooping_list;
/*end of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */

 /*start of LAN 广播包不能回流 by l39225 2006-7-15*/
static inline int should_eth_deliver(const struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
     unsigned char *dest;
     dest = NULL;

     /*start of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
     unsigned char *data = skb->data;
     unsigned short usIpHeadLen = 0;
     /*end of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
	
    dest = skb->mac.ethernet->h_dest;
    /*start of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
    usIpHeadLen = (skb->data[0] & 0x0f) * 4;
    /*end of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
  
    if (dest != NULL)
    {        
        if ( (dest[0] == 0xff) && (dest[1] ==0xff) && (dest[2] == 0xff)
              &&(dest[3] == 0xff) && (dest[4] ==0xff) && (dest[5] == 0xff)
            )
       {
            return  1;
       }

    }
    
    /* BEGIN: Modified by aizhanlong 00169427, 2011/11/18   PN:如果关闭LAN switch的软件转发功能，LAN到LAN的报文会上升到CPU来处理*/
    /*if (  (*(p->dev->name) == 'e')  &&  (*(p->dev->name+1) == 't')
           && (*(p->dev->name+2) == 'h')  && (*(skb->dev->name) == 'e')
           && (*(skb->dev->name+1) == 't') &&  (*(skb->dev->name+2) == 'h')
           && (*(skb->dev->name +3) == *(p->dev->name+3)) )
      {
            return 0;
      }*/
      if (  (*(p->dev->name) == 'e')  &&  (*(p->dev->name+1) == 't')
           && (*(p->dev->name+2) == 'h')  && (*(p->dev->name+3) == '0')
           && (*(p->dev->name+4) == '.')  && (*(skb->dev->name) == 'e')
           && (*(skb->dev->name+1) == 't') &&  (*(skb->dev->name+2) == 'h')
           && (*(skb->dev->name+3) == '0') &&  (*(skb->dev->name+4) == '.')           
           && (*(skb->dev->name +5) == *(p->dev->name+5)) )
      {
            return 0;
      }
    /* END:   Modified by aizhanlong 00169427, 2011/11/18 */

    /*start of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
    if(  (1 == igmp_snooping_list.igmp_snooping_enable) && (data[9] == IPPROTO_IGMP) && (data[usIpHeadLen] == IGMP_HOST_MEMBERSHIP_REPORT ||
        data[usIpHeadLen] == IGMPV2_HOST_MEMBERSHIP_REPORT || data[usIpHeadLen] == IGMPV3_HOST_MEMBERSHIP_REPORT) && (INADDR_UPNP != ntohl(skb->nh.iph->daddr)) )
    {    if (  (*(p->dev->name) == 'e')  &&  (*(p->dev->name+1) == 't')
           && (*(p->dev->name+2) == 'h')  && (*(skb->dev->name) == 'a')
           && (*(skb->dev->name+1) == 't') &&  (*(skb->dev->name+2) == 'h') )
         {     
             //printk("++++++ func= %s line= %d p->dev->name=%c skb->dev->name=%c addr=%x \n" , __FUNCTION__,__LINE__,*(skb->dev->name +3),*(p->dev->name+3),ntohl(skb->nh.iph->daddr));
             return 0;
         }
    
         if (  (*(p->dev->name) == 'a')  &&  (*(p->dev->name+1) == 't')
           && (*(p->dev->name+2) == 'h')  && (*(skb->dev->name) == 'e')
           && (*(skb->dev->name+1) == 't') &&  (*(skb->dev->name+2) == 'h') )
         {     
            // printk("++++++ func= %s line= %d p->dev->name=%c skb->dev->name=%c addr=%x\n" , __FUNCTION__,__LINE__,*(skb->dev->name +3),*(p->dev->name+3),ntohl(skb->nh.iph->daddr));
             return 0;
         }
    }
    /*end of 解决AU8D01204问题单: Wlan主机与LAN侧主机组播不能组播同一节目 by l129990 ,2008,12,29 */
    
     return 1;
}
    /*end of LAN 广播包不能回流 by l39225 2006-7-15*/
    
static inline int should_deliver(const struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
    /*start of WLAN 屏蔽无线组播 porting by w44771 20060506*/
    #if 0//w44771 delete, 12-01-2007
	unsigned char *dest;
	dest = NULL;
	
	dest = skb->mac.ethernet->h_dest;
    #endif
    /*end of WLAN 屏蔽无线组播 porting by w44771 20060506*/

 	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING)
		return 0;

    /*start of WLAN 屏蔽无线组播 porting by w44771 20060506*/
    #if 0//w44771 delete, 12-01-2007
    if (dest != NULL)
    {        
        if ( (*(p->dev->name) == 'w') 
            &&(*(p->dev->name + 1) == 'l')
            &&(*(p->dev->name + 2) == '0') 
             && (dest[0] == 0x01) && (dest[1] == 0) && (dest[2] == 0x5e)
           )
        {
           return 0;
        }
    }
    #endif
    /*end of WLAN 屏蔽无线组播 porting by w44771 20060506*/

#if defined(CONFIG_MIPS_BRCM)
	/*
	 * Do not forward  any packets received from
	 *  one WAN interface to other WAN interfaces in multiple PVC case
	 * In the following condition, if a new interface is added, please
	 * add the check to compare the first 3 chars from the name as
	 * shown below. Using strcmp will be expensive in this path.
	*/
	if ((skb->nfmark & FROM_WAN)  &&
	    ((*(p->dev->name)    == 'n') &&
	    (*(p->dev->name + 1) == 'a') &&
	    (*(p->dev->name + 2) == 's')))
		return 0;
#endif

	return 1;
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
/*start of WAN <3.5.2skb长度合法性，导致usb下载崩溃> porting by s60000658 20060506*/
#if 0
//#if defined(CONFIG_MIPS_BRCM)
	// Just to make it consistent with 2.4 so it will not surprise the customers.(Should be more intelligent.)
#ifdef CONFIG_BRIDGE_NETFILTER
	/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
	nf_bridge_maybe_copy_header(skb);
#endif
	skb_push(skb, ETH_HLEN);

	dev_queue_xmit(skb);
//#else
#endif
	
#ifdef CONFIG_BRIDGE_NETFILTER
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		nf_bridge_maybe_copy_header(skb);
#endif
		skb_push(skb, ETH_HLEN);
        /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
    	if(DEBUG_VBR)VLAN_DUMP_SKB(skb, "bridge send skb");
        if(skb->dev->br_port && skb->dev->br_port->br->vlan_enable)
        {
            switch(skb->dev->br_port->mode)
            {
            case BR_PORT_MODE_NORMAL:
                /*do nothing, xmit directly*/
                break;
            case BR_PORT_MODE_ACCESS:
                if(skb->protocol == __constant_htons(ETH_P_8021Q))
                {
                    unsigned tmp[VLAN_ETH_ALEN * 2];
                    
                    skb = skb_unshare(skb, GFP_ATOMIC);
                    if(!skb)
                    {
                        printk("BRVLAN: unshare skb fail ,maybe memery not enough or other reasons, %s, %d\n", __FUNCTION__, __LINE__);
                        return 0;
                    }
                    
                    /*remove vlan tag*/
            		memcpy(tmp, skb->data, VLAN_ETH_ALEN * 2);
            		skb_pull(skb, VLAN_HLEN);
            		memcpy(skb->data, tmp, VLAN_ETH_ALEN * 2);
            		skb->protocol = *((unsigned short*)&(skb->data[VLAN_ETH_ALEN * 2]));
                }
                break;
            case BR_PORT_MODE_TRUNK:
                {
                    unsigned short tag;
                    unsigned char tmp[VLAN_ETH_ALEN * 2];
                    unsigned char *src;
                    struct net_bridge_port *p;
                    struct net_bridge_fdb_entry *dst;
                    
                    if(0 != vlan_get_tag(skb, &tag))
                    {
                        src = skb->mac.ethernet->h_source;
                        p = skb->dev->br_port;
                        dst = __br_fdb_get(p->br, src);
                
                        /*报文来自网关自己, 并且管理vlan id和端口pvid相同则不用打tag*/
                        if(dst != NULL && dst->is_local )
                        {
                            if(p->br->mngr_vid != VLAN_GET_VID(p->tci))
                        {
                                skb = vlan_put_tag(skb, p->br->mngr_vid);                            
                            }
                        } 
                        else
                            skb = vlan_put_tag(skb, p->tci);
              
                        if(!skb)
                            return 0;
                    }
                    else if(VLAN_GET_VID(skb->dev->br_port->tci) == VLAN_GET_VID(tag))
                    {               
                        skb = skb_unshare(skb, GFP_ATOMIC);
                        if(!skb)
                        {
                            printk("BRVLAN: unshare skb fail ,maybe memery not enough or other reasons, %s, %d\n", __FUNCTION__, __LINE__);
                            return 0;
                        }
                        
                        /*remove vlan tag*/
                		memcpy(tmp, skb->data, VLAN_ETH_ALEN * 2);
                		skb_pull(skb, VLAN_HLEN);
                		memcpy(skb->data, tmp, VLAN_ETH_ALEN * 2);
                		skb->protocol = *((unsigned short*)&(skb->data[VLAN_ETH_ALEN * 2]));
                    }                    
                }
                break;
            case BR_PORT_MODE_HYBRID:
                /*not surported*/
                break;
            case  BR_PORT_MODE_UNKNOWN:
            default:
                printk("%s: vlan port(%s) mode %d is unknown\n",
                    __FUNCTION__, skb->dev->name, skb->dev->br_port->mode);
                kfree_skb(skb);
                return 0;
            }
        }
    	if(DEBUG_VBR)VLAN_DUMP_SKB(skb, "bridge send skb(after do vlan)");
#endif //CONFIG_KPN_VLAN
        /*end vlan bridge, s60000658, 20060627*/

        /* start of PROTOCOL bridge long packet by zhouyi+00037589 2006年8月29日 */
//        if ((skb->len - ETH_HLEN) > skb->dev->mtu) 
        if ((skb->len - ETH_HLEN) > 1512) 
        {
		    kfree_skb(skb);
        }
	    else 
	    {
		    dev_queue_xmit(skb);
	    }
        /* end of PROTOCOL bridge long packet by zhouyi+00037589 2006年8月29日 */
//#endif
/*end of WAN <3.5.2skb长度合法性，导致usb下载崩溃> porting by s60000658 20060506*/
	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
#if 0
	NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
			br_dev_queue_push_xmit);
#else
	br_dev_queue_push_xmit(skb);

#endif

	return 0;
}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;
#if 0
	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
#else
	br_forward_finish(skb);
#endif
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(const struct net_bridge_port *p, 
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;
    /*start vlan bridge, s60000658, 20060627*/
#ifdef CONFIG_KPN_VLAN
    if(br->vlan_enable && (skb->protocol == __constant_htons(ETH_P_8021Q)))
    {
		struct vlan_hdr *vhdr = (struct vlan_hdr *)(skb->data);
		unsigned short vid = (unsigned short)(vhdr->h_vlan_TCI & VLAN_VID_MASK);
		struct vlan_8021q *pvlan = br_get_vlan(br, vid);

		if(NULL != pvlan)
	    {
    		list_for_each_entry_rcu(p, &br->port_list, list) {
    		if (should_deliver(p, skb) && should_eth_deliver(p,skb) && (ISIN_VLAN(pvlan->ports , p->port_no))) {
    			if (prev != NULL)
                {
    				struct sk_buff *skb2;

                   /*
                    * start of y42304 added 20060831: brideg禁用mac地址情况下, 将nfmark最高4位
                    * 置1给以太网驱动用.
                    */
                    if ((0 == pvlan->macln_enable)
                         && (*(prev->dev->name) != 'n')
                         && (*(prev->dev->name+1) != 'a')
                         && (*(prev->dev->name+2) != 's')
                        )
                    {
                        skb->nfmark |= BCM_TAG_NFMARK;                                                                       
                    }
                    /* end of y42304 added 20060831 */
                    

    				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
    					br->statistics.tx_dropped++;
    					kfree_skb(skb);
    					br_release_vlan(pvlan);
    					return;
    				}

    				__packet_hook(prev, skb2);
    			}

    			prev = p;
    		}
        	}
    		br_release_vlan(pvlan);
        }
    }
    else
#endif// CONFIG_KPN_VLAN
    /*end vlan bridge, s60000658, 20060627*/
	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (should_deliver(p, skb) &&  should_eth_deliver(p,skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
