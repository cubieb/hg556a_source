/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/in.h>
#if defined(CONFIG_MIPS_BRCM)
#include <linux/ip.h>
#endif
#include <linux/udp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>

unsigned long ip_ct_udp_timeout = 30*HZ;
unsigned long ip_ct_udp_timeout_stream = 180*HZ;

#if defined(CONFIG_MIPS_BRCM)
unsigned long ip_ct_udp_unreplieddns_timeout = 1*HZ;
#endif

static int udp_pkt_to_tuple(const struct sk_buff *skb,
			     unsigned int dataoff,
			     struct ip_conntrack_tuple *tuple)
{
	struct udphdr hdr;

	/* Actually only need first 8 bytes. */
	if (skb_copy_bits(skb, dataoff, &hdr, 8) != 0)
		return 0;

	tuple->src.u.udp.port = hdr.source;
	tuple->dst.u.udp.port = hdr.dest;

	return 1;
}

static int udp_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
	tuple->src.u.udp.port = orig->dst.u.udp.port;
	tuple->dst.u.udp.port = orig->src.u.udp.port;
	return 1;
}

/* Print out the per-protocol part of the tuple. */
static unsigned int udp_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "sport=%hu dport=%hu ",
		       ntohs(tuple->src.u.udp.port),
		       ntohs(tuple->dst.u.udp.port));
}

/* Print out the private part of the conntrack. */
static unsigned int udp_print_conntrack(char *buffer,
					const struct ip_conntrack *conntrack)
{
	return 0;
}

/* Returns verdict for packet, and may modify conntracktype */
static int udp_packet(struct ip_conntrack *conntrack,
		      const struct sk_buff *skb,
		      enum ip_conntrack_info conntrackinfo)
{
	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */

    struct iphdr *iph = skb->nh.iph;
		struct udphdr *udph = (void *)iph + iph->ihl * 4;
		__u16 dport = ntohs(udph->dest);
                __u16 sport = ntohs(udph->source);
        
	if (test_bit(IPS_SEEN_REPLY_BIT, &conntrack->status)) {
                if (dport == 53 || sport == 53)
                {
                       ip_ct_refresh(conntrack, ip_ct_udp_unreplieddns_timeout);
                }
                else
                {
        		ip_ct_refresh(conntrack, ip_ct_udp_timeout_stream);
                }
		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &conntrack->status);
	} else
#if defined(CONFIG_MIPS_BRCM) 
	{
		/* Special handling of UNRPLIED DNS query packet: Song Wang
		*  Before NAT and WAN interface are UP, during that time window,
		* if a DNS query is sent out, there will be an UNRPLIED DNS connection track entry
		* in which expected src/dst are private IP addresses in the tuple.
		* After  NAT and WAN interface are UP, the UNRPLIED DNS connection track
		* entry should go away ASAP to enable the establishment  of the tuple with
		* the expected src/dst that are public IP addresses. 
		*/
		
		if (dport == 53 || sport == 53)
			ip_ct_refresh(conntrack, ip_ct_udp_unreplieddns_timeout);
		else   
			ip_ct_refresh(conntrack, ip_ct_udp_timeout);
	}
#else	
		ip_ct_refresh(conntrack, ip_ct_udp_timeout);
#endif		

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int udp_new(struct ip_conntrack *conntrack, const struct sk_buff *skb)
{
	return 1;
}

struct ip_conntrack_protocol ip_conntrack_protocol_udp
= { { NULL, NULL }, IPPROTO_UDP, "udp",
    udp_pkt_to_tuple, udp_invert_tuple, udp_print_tuple, udp_print_conntrack,
    udp_packet, udp_new, NULL, NULL, NULL };
