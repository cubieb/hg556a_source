/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		The Internet Protocol (IP) module.
 *
 * Version:	$Id: ip_input.c,v 1.1.2.10.6.2 2010/01/07 03:15:41 l43571 Exp $
 *
 * Authors:	Ross Biro, <bir7@leland.Stanford.Edu>
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Donald Becker, <becker@super.org>
 *		Alan Cox, <Alan.Cox@linux.org>
 *		Richard Underwood
 *		Stefan Becker, <stefanb@yello.ping.de>
 *		Jorge Cwik, <jorge@laser.satlink.net>
 *		Arnt Gulbrandsen, <agulbra@nvg.unit.no>
 *		
 *
 * Fixes:
 *		Alan Cox	:	Commented a couple of minor bits of surplus code
 *		Alan Cox	:	Undefining IP_FORWARD doesn't include the code
 *					(just stops a compiler warning).
 *		Alan Cox	:	Frames with >=MAX_ROUTE record routes, strict routes or loose routes
 *					are junked rather than corrupting things.
 *		Alan Cox	:	Frames to bad broadcast subnets are dumped
 *					We used to process them non broadcast and
 *					boy could that cause havoc.
 *		Alan Cox	:	ip_forward sets the free flag on the
 *					new frame it queues. Still crap because
 *					it copies the frame but at least it
 *					doesn't eat memory too.
 *		Alan Cox	:	Generic queue code and memory fixes.
 *		Fred Van Kempen :	IP fragment support (borrowed from NET2E)
 *		Gerhard Koerting:	Forward fragmented frames correctly.
 *		Gerhard Koerting: 	Fixes to my fix of the above 8-).
 *		Gerhard Koerting:	IP interface addressing fix.
 *		Linus Torvalds	:	More robustness checks
 *		Alan Cox	:	Even more checks: Still not as robust as it ought to be
 *		Alan Cox	:	Save IP header pointer for later
 *		Alan Cox	:	ip option setting
 *		Alan Cox	:	Use ip_tos/ip_ttl settings
 *		Alan Cox	:	Fragmentation bogosity removed
 *					(Thanks to Mark.Bush@prg.ox.ac.uk)
 *		Dmitry Gorodchanin :	Send of a raw packet crash fix.
 *		Alan Cox	:	Silly ip bug when an overlength
 *					fragment turns up. Now frees the
 *					queue.
 *		Linus Torvalds/ :	Memory leakage on fragmentation
 *		Alan Cox	:	handling.
 *		Gerhard Koerting:	Forwarding uses IP priority hints
 *		Teemu Rantanen	:	Fragment problems.
 *		Alan Cox	:	General cleanup, comments and reformat
 *		Alan Cox	:	SNMP statistics
 *		Alan Cox	:	BSD address rule semantics. Also see
 *					UDP as there is a nasty checksum issue
 *					if you do things the wrong way.
 *		Alan Cox	:	Always defrag, moved IP_FORWARD to the config.in file
 *		Alan Cox	: 	IP options adjust sk->priority.
 *		Pedro Roque	:	Fix mtu/length error in ip_forward.
 *		Alan Cox	:	Avoid ip_chk_addr when possible.
 *	Richard Underwood	:	IP multicasting.
 *		Alan Cox	:	Cleaned up multicast handlers.
 *		Alan Cox	:	RAW sockets demultiplex in the BSD style.
 *		Gunther Mayer	:	Fix the SNMP reporting typo
 *		Alan Cox	:	Always in group 224.0.0.1
 *	Pauline Middelink	:	Fast ip_checksum update when forwarding
 *					Masquerading support.
 *		Alan Cox	:	Multicast loopback error for 224.0.0.1
 *		Alan Cox	:	IP_MULTICAST_LOOP option.
 *		Alan Cox	:	Use notifiers.
 *		Bjorn Ekwall	:	Removed ip_csum (from slhc.c too)
 *		Bjorn Ekwall	:	Moved ip_fast_csum to ip.h (inline!)
 *		Stefan Becker   :       Send out ICMP HOST REDIRECT
 *	Arnt Gulbrandsen	:	ip_build_xmit
 *		Alan Cox	:	Per socket routing cache
 *		Alan Cox	:	Fixed routing cache, added header cache.
 *		Alan Cox	:	Loopback didn't work right in original ip_build_xmit - fixed it.
 *		Alan Cox	:	Only send ICMP_REDIRECT if src/dest are the same net.
 *		Alan Cox	:	Incoming IP option handling.
 *		Alan Cox	:	Set saddr on raw output frames as per BSD.
 *		Alan Cox	:	Stopped broadcast source route explosions.
 *		Alan Cox	:	Can disable source routing
 *		Takeshi Sone    :	Masquerading didn't work.
 *	Dave Bonn,Alan Cox	:	Faster IP forwarding whenever possible.
 *		Alan Cox	:	Memory leaks, tramples, misc debugging.
 *		Alan Cox	:	Fixed multicast (by popular demand 8))
 *		Alan Cox	:	Fixed forwarding (by even more popular demand 8))
 *		Alan Cox	:	Fixed SNMP statistics [I think]
 *	Gerhard Koerting	:	IP fragmentation forwarding fix
 *		Alan Cox	:	Device lock against page fault.
 *		Alan Cox	:	IP_HDRINCL facility.
 *	Werner Almesberger	:	Zero fragment bug
 *		Alan Cox	:	RAW IP frame length bug
 *		Alan Cox	:	Outgoing firewall on build_xmit
 *		A.N.Kuznetsov	:	IP_OPTIONS support throughout the kernel
 *		Alan Cox	:	Multicast routing hooks
 *		Jos Vos		:	Do accounting *before* call_in_firewall
 *	Willy Konynenberg	:	Transparent proxying support
 *
 *  
 *
 * To Fix:
 *		IP fragmentation wants rewriting cleanly. The RFC815 algorithm is much more efficient
 *		and could be made very efficient with the addition of some virtual memory hacks to permit
 *		the allocation of a buffer that can then be 'grown' by twiddling page tables.
 *		Output fragmentation wants updating along with the buffer management to use a single 
 *		interleaved copy algorithm so that fragmenting has a one copy overhead. Actual packet
 *		output should probably do its own fragmentation at the UDP/RAW layer. TCP shouldn't cause
 *		fragmentation anyway.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <asm/system.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/config.h>

#include <linux/net.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <net/snmp.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <net/xfrm.h>
#include <linux/mroute.h>
#include <linux/netlink.h>

#include <linux/netfilter_ipv4/ipt_misc.h>
extern int sysctl_rtp_port_max;
extern int sysctl_rtp_port_min;
static struct list_head g_hash[TCPPTSCAN_LISTHASH_SIZE];
static unsigned int g_expire_ptscan = 0;
/*
 *	SNMP management statistics
 */

DEFINE_SNMP_STAT(struct ipstats_mib, ip_statistics);

/*
 *	Process Router Attention IP option
 */ 
int ip_call_ra_chain(struct sk_buff *skb)
{
	struct ip_ra_chain *ra;
	u8 protocol = skb->nh.iph->protocol;
	struct sock *last = NULL;

	read_lock(&ip_ra_lock);
	for (ra = ip_ra_chain; ra; ra = ra->next) {
		struct sock *sk = ra->sk;

		/* If socket is bound to an interface, only report
		 * the packet if it came  from that interface.
		 */
		if (sk && inet_sk(sk)->num == protocol &&
		    (!sk->sk_bound_dev_if ||
		     sk->sk_bound_dev_if == skb->dev->ifindex)) {
			if (skb->nh.iph->frag_off & htons(IP_MF|IP_OFFSET)) {
				skb = ip_defrag(skb);
				if (skb == NULL) {
					read_unlock(&ip_ra_lock);
					return 1;
				}
			}
			if (last) {
				struct sk_buff *skb2 = skb_clone(skb, GFP_ATOMIC);
				if (skb2)
					raw_rcv(last, skb2);
			}
			last = sk;
		}
	}

	if (last) {
		raw_rcv(last, skb);
		read_unlock(&ip_ra_lock);
		return 1;
	}
	read_unlock(&ip_ra_lock);
	return 0;
}

static inline int ip_local_deliver_finish(struct sk_buff *skb)
{
	int ihl = skb->nh.iph->ihl*4;

#ifdef CONFIG_NETFILTER_DEBUG
	nf_debug_ip_local_deliver(skb);
#endif /*CONFIG_NETFILTER_DEBUG*/

	__skb_pull(skb, ihl);

	/* Free reference early: we don't need it any more, and it may
           hold ip_conntrack module loaded indefinitely. */
	nf_reset(skb);

        /* Point into the IP datagram, just past the header. */
        skb->h.raw = skb->data;

	rcu_read_lock();
	{
		/* Note: See raw.c and net/raw.h, RAWV4_HTABLE_SIZE==MAX_INET_PROTOS */
		int protocol = skb->nh.iph->protocol;
		int hash;
		struct sock *raw_sk;
		struct net_protocol *ipprot;

	resubmit:
		hash = protocol & (MAX_INET_PROTOS - 1);
		raw_sk = sk_head(&raw_v4_htable[hash]);

		/* If there maybe a raw socket we must check - if not we
		 * don't care less
		 */
		if (raw_sk)
			raw_v4_input(skb, skb->nh.iph, hash);

		if ((ipprot = inet_protos[hash]) != NULL) {
			int ret;

			smp_read_barrier_depends();
			if (!ipprot->no_policy &&
			    !xfrm4_policy_check(NULL, XFRM_POLICY_IN, skb)) {
				kfree_skb(skb);
				goto out;
			}
			ret = ipprot->handler(skb);
			if (ret < 0) {
				protocol = -ret;
				goto resubmit;
			}
			IP_INC_STATS_BH(IPSTATS_MIB_INDELIVERS);
		} else {
			if (!raw_sk) {
				if (xfrm4_policy_check(NULL, XFRM_POLICY_IN, skb)) {
					IP_INC_STATS_BH(IPSTATS_MIB_INUNKNOWNPROTOS);
					icmp_send(skb, ICMP_DEST_UNREACH,
						  ICMP_PROT_UNREACH, 0);
				}
			} else
				IP_INC_STATS_BH(IPSTATS_MIB_INDELIVERS);
			kfree_skb(skb);
		}
	}
 out:
	rcu_read_unlock();

	return 0;
}

/*
 * 	Deliver IP Packets to the higher protocol layers.
 */ 
int ip_local_deliver(struct sk_buff *skb)
{
    /* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
    /* 赶在报文重组之前打印报文内容，这样报文长度就有限制了 */
    //dg_recordInputPackage(skb);
    /* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */

	/*
	 *	Reassemble IP fragments.
	 */

	if (skb->nh.iph->frag_off & htons(IP_MF|IP_OFFSET)) {
		skb = ip_defrag(skb);
		if (!skb)
			return 0;
	}

	return NF_HOOK(PF_INET, NF_IP_LOCAL_IN, skb, skb->dev, NULL,
		       ip_local_deliver_finish);
}

static inline int ip_rcv_finish(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	struct iphdr *iph = skb->nh.iph;

	/*
	 *	Initialise the virtual path cache for the packet. It describes
	 *	how the packet travels inside Linux networking.
	 */ 
	if (skb->dst == NULL) {
		if (ip_route_input(skb, iph->daddr, iph->saddr, iph->tos, dev))
			goto drop; 
	}

#ifdef CONFIG_NET_CLS_ROUTE
	if (skb->dst->tclassid) {
		struct ip_rt_acct *st = ip_rt_acct + 256*smp_processor_id();
		u32 idx = skb->dst->tclassid;
		st[idx&0xFF].o_packets++;
		st[idx&0xFF].o_bytes+=skb->len;
		st[(idx>>16)&0xFF].i_packets++;
		st[(idx>>16)&0xFF].i_bytes+=skb->len;
	}
#endif

	if (iph->ihl > 5) {
		struct ip_options *opt;

		/* It looks as overkill, because not all
		   IP options require packet mangling.
		   But it is the easiest for now, especially taking
		   into account that combination of IP options
		   and running sniffer is extremely rare condition.
		                                      --ANK (980813)
		*/

		if (skb_cow(skb, skb_headroom(skb))) {
			IP_INC_STATS_BH(IPSTATS_MIB_INDISCARDS);
			goto drop;
		}
		iph = skb->nh.iph;

		if (ip_options_compile(NULL, skb))
			goto inhdr_error;

		opt = &(IPCB(skb)->opt);
		if (opt->srr) {
			struct in_device *in_dev = in_dev_get(dev);
			if (in_dev) {
				if (!IN_DEV_SOURCE_ROUTE(in_dev)) {
					if (IN_DEV_LOG_MARTIANS(in_dev) && net_ratelimit())
						printk(KERN_INFO "source route option %u.%u.%u.%u -> %u.%u.%u.%u\n",
						       NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
					in_dev_put(in_dev);
					goto drop;
				}
				in_dev_put(in_dev);
			}
			if (ip_options_rcv_srr(skb))
				goto drop;
		}
	}

	return dst_input(skb);

inhdr_error:
	IP_INC_STATS_BH(IPSTATS_MIB_INHDRERRORS);
drop:
        kfree_skb(skb);
        return NET_RX_DROP;
}

/* 获取 br0 的ip, add by j00100803 */
int get_lan_ip(u32 * lanip, u32 * mask)
{
	struct net_device * net_dev = NULL;
	struct in_device * in_dev = NULL;
	struct in_ifaddr ** ifap = NULL;
	struct in_ifaddr * ifa = NULL;
	
	if ((net_dev = __dev_get_by_name("br0")) == NULL)
		return -1;
	if ((in_dev = __in_dev_get(net_dev)) != NULL)
	{
		for (ifap = &in_dev->ifa_list; 
			(ifa = *ifap) != NULL;
			ifap = &ifa->ifa_next)
		{
			if (!strcmp(ifa->ifa_label, "br0"))
		    {
				/* found */
				*lanip = ifa->ifa_local;
				*mask = ifa->ifa_mask;
				return 0;
			}
		}
	}

	return -1;
}
/* HG556A 新增 dos 过滤, 在进防火墙之前 
 返回0 pass, 返回1 drop, add by j00100803 */
static int is_mac_broadcast(struct sk_buff *skb)
{
	char szBroadcastMac[ETH_ALEN];
	memset(szBroadcastMac, 1, ETH_ALEN);
	if(!(skb->mac.raw >= skb->head && (skb->mac.raw + ETH_HLEN) <= skb->data))
	{
        return 0;
	}
	
	return !(memcmp(skb->mac.ethernet->h_source, szBroadcastMac, ETH_ALEN));
}

#define SCAN_PORT_EXPIRE 100
#define SCAN_PORT_NUM 5
static __inline__ int iphash_key(const unsigned char *srcip, const unsigned char *dstip)
{
    unsigned long x;
    unsigned long y;

	x = srcip[0];
	x = (x << 2) ^ srcip[1];
	x = (x << 2) ^ srcip[2];
	x = (x << 2) ^ srcip[3];
	x ^= x >> 8;

	y = dstip[0];
	y = (y << 2) ^ dstip[1];
	y = (y << 2) ^ dstip[2];
	y = (y << 2) ^ dstip[3];
	y ^= y >> 8;

	return y & x & (TCPPTSCAN_LISTHASH_SIZE - 1);
}

static void ptscan_node_free(struct iptmisc_tcpptscan_node *pnode)
{
    if(!pnode)
        return ;
    spin_lock_bh(&pnode->lock);
    del_timer(&pnode->timer_gc);
    del_timer(&pnode->timer_refresh);
    list_del_rcu(&pnode->list);
    spin_unlock_bh(&pnode->lock);
    kfree(pnode);
}

void inline init_ptscan(void)
{
    static int inited = 0;
    int i;
    
    if(inited)
        return;
    
    for(i = 0 ; i < TCPPTSCAN_LISTHASH_SIZE; i++)
    {
        INIT_LIST_HEAD(&g_hash[i]);
    }

    inited = 1;
}

static void refresh_timeout(unsigned long ptr)
{
    struct iptmisc_tcpptscan_node *pnode = (struct iptmisc_tcpptscan_node*)ptr;

    if((!pnode->silent) || (pnode->silent_start + TCPPTSCAN_SLIENT_MAX < jiffies))    
    {
        memset(pnode->dst_ports, 0, sizeof(pnode->dst_ports));
        pnode->recorded_ports = 0;
        pnode->silent = 0;
    }

    mod_timer(&pnode->timer_refresh, jiffies+g_expire_ptscan*TCPPTSCAN_REFRESH_INTERVAL);
}

static void gc_timeout(unsigned long ptr)
{
     struct iptmisc_tcpptscan_node *pnode = (struct iptmisc_tcpptscan_node*)ptr;
     int just_visited = 0;
     int i = 0 ;

     if(pnode->silent)
     {
         mod_timer(&pnode->timer_gc, jiffies+TCPPTSCAN_GC_INTERVAL);    
     }
     else
    {
        for(i = 0; i < TCPPTSCAN_PTHASH_SIZE2; i++)
        {
            if(pnode->dst_ports[i])
            {
                just_visited = 1;
            }
        }

        if(just_visited)
        {
             mod_timer(&pnode->timer_gc, jiffies+TCPPTSCAN_GC_INTERVAL);              
        }
        else
        {
            del_timer(&pnode->timer_gc);
            del_timer(&pnode->timer_refresh);
            list_del_rcu(&pnode->list);
            kfree(pnode);
        }
    }
     
}

static inline int is_tcp_port_scan(struct sk_buff* skb)
{
	const struct iphdr *iph = (void *)skb->nh.iph;
	const struct tcphdr *tcph = (void *)skb->nh.iph + skb->nh.iph->ihl*4;
    struct iptmisc_tcpptscan_node *pnode = NULL;
    unsigned int saddr, daddr;
    int node_existed = 0;
    int seg, pos, mode;
    int ret = 0;

    /* The first pkt of one tcp connection is a syn pkt */
    if(!tcph->syn)
        return 0;

    init_ptscan();
    
    daddr = iph->daddr;
    saddr = iph->saddr;
    list_for_each_entry_rcu(pnode, &g_hash[iphash_key((unsigned char*)&saddr, (unsigned char*)&daddr)], list)
    {
        spin_lock_bh(&pnode->lock);
        if(pnode->srcip == iph->saddr && pnode->dstip == iph->daddr)
        {
            node_existed = 1;
            break;
        }
        spin_unlock_bh(&pnode->lock);
    }

    if(!node_existed)
    {
        if(NULL == (pnode = (struct iptmisc_tcpptscan_node*)kmalloc(sizeof(struct iptmisc_tcpptscan_node), GFP_KERNEL)))
            return 0;
        else
        {
            memset(pnode, 0, sizeof(struct iptmisc_tcpptscan_node));
            daddr = pnode->dstip = iph->daddr;
            saddr = pnode->srcip = iph->saddr;
            init_timer(&pnode->timer_refresh);
            init_timer(&pnode->timer_gc);
            pnode->timer_refresh.expires = jiffies + SCAN_PORT_EXPIRE * TCPPTSCAN_REFRESH_INTERVAL;
            pnode->timer_refresh.data = pnode;
            pnode->timer_refresh.function = refresh_timeout;
            pnode->timer_gc.expires = jiffies + TCPPTSCAN_GC_INTERVAL;
            pnode->timer_gc.data = pnode;
            pnode->timer_gc.function = gc_timeout;
            add_timer(&pnode->timer_refresh);
            add_timer(&pnode->timer_gc);
            spin_lock_init(&pnode->lock);
            list_add_rcu(&pnode->list, &g_hash[iphash_key((unsigned char*)&saddr, (unsigned char*)&daddr)]); 
            spin_lock_bh(&pnode->lock);
        }
    }

    if(pnode->silent)
    {    
        ret = 1; 
        goto local_exit;
    }
    
    /* the space of index is [0,1023], each bit of the dst_port member is a index of the dest port */
	mode = tcph->dest%TCPPTSCAN_PTHASH_SIZE1;
	for(seg = 0, pos = mode; pos > TCPPTSCAN_PTHASH_WDWIDTH; pos-=TCPPTSCAN_PTHASH_WDWIDTH, seg++)
    {;}

	if(!(pnode->dst_ports[seg] & (0x0001 << pos)))
    {
    	pnode->dst_ports[seg] |= (0x0001 << pos);
    	pnode->recorded_ports++;

    	/*if a special numbers of port are quested in a special interval ,there might be a tcp port scanning*/
        if(pnode->recorded_ports > SCAN_PORT_NUM)
        {
            pnode->silent = 1;
            pnode->silent_start = jiffies;
            ret = 1;        
        }
    }

local_exit:
    spin_unlock_bh(&pnode->lock);
    return ret;
}

int ip_rcv_secdos_filter(struct sk_buff *skb, struct net_device *dev)
{
    const struct iphdr *iph = (void *)skb->nh.iph;
	struct udphdr * udph = (void *)iph + iph->ihl*4;
	struct icmphdr * icmph = NULL;
    u32 lanip = 0;
	u32 mask = 0;
	u_int16_t pktlen = 0;
	u_int16_t udp_len = 0;
	char * pszPromptLog;

	pktlen = ntohs(iph->tot_len);
    /* 源地址的广播风暴, 源地址为广播地址进行丢弃*/
    if(BADCLASS(iph->saddr))
    {	
    	pszPromptLog = "Broadcast attack";
        goto PKT_DROP;
    }
	/* 非来自br0, lo,且源地址为LAN地址的报文丢弃 */
    if ((NULL == strstr(skb->dev->name,"br")) && (NULL == strstr(skb->dev->name,"lo")))
    {	
    	/* 首先获取br0的ip地址 */
		if(0 != get_lan_ip(&lanip, &mask))
		{
			pszPromptLog = "br0 no ip";
        	goto PKT_DROP;
		}
		if((lanip & mask) == (iph->saddr & mask))
		{
			pszPromptLog = "Bad source(not br0) packet attack";
			goto PKT_DROP;
		}
    }
	/* ip land or same src between dst */
	do
	{
	    if (iph->saddr == iph->daddr)
	    {
			if(iph->protocol == IPPROTO_UDP)
			{
				if(sysctl_rtp_port_max 
					&& sysctl_rtp_port_min
					&& (udph->dest <= sysctl_rtp_port_max) 
					&& (udph->dest >= sysctl_rtp_port_min))
				{
					break;			
				}
			}
			
	        if (0x7f000001 != iph->saddr)
	        {
	        	/* to exclude the lo dev */
	        	if(NULL == strstr(skb->dev->name,"lo"))
	        	{
	        		pszPromptLog = "Same src and dst, ipland attack";
					if (net_ratelimit())
					{
					    printk("<8>" "[fwlog] %s, SRC=%u.%u.%u.%u DST=%u.%u.%u.%u.\n",
				       	    pszPromptLog, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
					}
	            	//goto PKT_DROP;
	        	}
	        }
	    }
	}while(0);

    if(iph->protocol == IPPROTO_UDP) 
    {
    	/* fragment attack */
    	if((pktlen - iph->ihl *4) < 8)
    	{
    		pszPromptLog = "Fragment attack";
			if (net_ratelimit())
			{
				printk("<8>" "[fwlog] %s, SRC=%u.%u.%u.%u DST=%u.%u.%u.%u.\n",
       				pszPromptLog, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
			}
    		//goto PKT_DROP;
    	}
		/* udp bomb */
		udp_len = ntohs(udph->len);
		if(!(pktlen == iph->ihl*4 + udp_len))
    	{
    		pszPromptLog = "Udp bomb attack";
    		if (net_ratelimit())
			{
				printk("<8>" "[fwlog] %s, SRC=%u.%u.%u.%u DST=%u.%u.%u.%u.\n",
       				pszPromptLog, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
			}
    		//goto PKT_DROP;
    	}
    }

	if(iph->protocol == IPPROTO_TCP)
	{
		/* fragment attack */
		if((pktlen - iph->ihl *4) < 20)
		{
			pszPromptLog = "Fragment attack";
			goto PKT_DROP;
		}
	//add by y67514 解决load page slow问题
#if 0
		/* tcp port scan */
		if(is_tcp_port_scan(skb))
		{
			pszPromptLog = "Tcp port scan";
			goto PKT_DROP;
		}
#endif
	}
	if(iph->protocol == IPPROTO_ICMP)
	{
		/* fragment attack */
		if((pktlen - iph->ihl *4) < 8)
		{
			pszPromptLog = "Fragment attack";
			goto PKT_DROP;
		}
		/* ping of death */
		icmph = (void *)iph + iph->ihl*4;
		if((icmph != NULL) && ((icmph->type == 0) || (icmph->type == 8)))
		{
			if((iph->daddr == 0xffffffff) || is_mac_broadcast(skb))
			{
				pszPromptLog = "Icmp smurf";
				if (net_ratelimit())
				{
    				printk("<8>" "[fwlog] %s, SRC=%u.%u.%u.%u DST=%u.%u.%u.%u.\n",
	       				pszPromptLog, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
				}
			}
			/*Start: w00135358 del for HG556a 20090427 PN:AU8D02098*/
			#if 0
			/* the length of the echo request and reply can't
			be bigger than 84, 84 is a experimental value */
	 		if(pktlen > 84)
	 		{
	 			pszPromptLog = "Ping of death attack";
				goto PKT_DROP;
	 		}
			#endif
			/*End: w00135358 del for HG556a 20090427 PN:AU8D02098*/
		}
	}
	
PKT_PASS:
    return 0;
PKT_DROP:
	if (net_ratelimit())
	{
    	printk("<8>" "[fwlog] %s, SRC=%u.%u.%u.%u DST=%u.%u.%u.%u.\n",
	       	pszPromptLog, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
	}
	return 1;
}
/*
 * 	Main IP Receive routine.
 */ 
int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt)
{
	struct iphdr *iph;

	/* When the interface is in promisc. mode, drop all the crap
	 * that it receives, do not try to analyse it.
	 */
	if (skb->pkt_type == PACKET_OTHERHOST)
		goto drop;

	IP_INC_STATS_BH(IPSTATS_MIB_INRECEIVES);

	if ((skb = skb_share_check(skb, GFP_ATOMIC)) == NULL) {
		IP_INC_STATS_BH(IPSTATS_MIB_INDISCARDS);
		goto out;
	}

	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
		goto inhdr_error;

	iph = skb->nh.iph;

	/*
	 *	RFC1122: 3.1.2.2 MUST silently discard any IP frame that fails the checksum.
	 *
	 *	Is the datagram acceptable?
	 *
	 *	1.	Length at least the size of an ip header
	 *	2.	Version of 4
	 *	3.	Checksums correctly. [Speed optimisation for later, skip loopback checksums]
	 *	4.	Doesn't have a bogus length
	 */

	if (iph->ihl < 5 || iph->version != 4)
		goto inhdr_error; 

	if (!pskb_may_pull(skb, iph->ihl*4))
		goto inhdr_error;

	iph = skb->nh.iph;

	if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		goto inhdr_error; 

	{
		__u32 len = ntohs(iph->tot_len); 
		if (skb->len < len || len < (iph->ihl<<2))
			goto inhdr_error;

		/* Our transport medium may have padded the buffer out. Now we know it
		 * is IP we can trim to the true length of the frame.
		 * Note this now means skb->len holds ntohs(iph->tot_len).
		 */
		if (skb->len > len) {
			__pskb_trim(skb, len);
			if (skb->ip_summed == CHECKSUM_HW)
				skb->ip_summed = CHECKSUM_NONE;
		}
	}
	if(1 == ip_rcv_secdos_filter(skb, dev))
	{
		goto drop;
	}
	
	return NF_HOOK(PF_INET, NF_IP_PRE_ROUTING, skb, dev, NULL,
		       ip_rcv_finish);

inhdr_error:
	IP_INC_STATS_BH(IPSTATS_MIB_INHDRERRORS);
drop:
        kfree_skb(skb);
out:
        return NET_RX_DROP;
}

EXPORT_SYMBOL(ip_rcv);
EXPORT_SYMBOL(ip_statistics);
