//*****************************************************************************
//
//  Copyright (c) 2000-2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//******************************************************************************
//
//  Filename:       ip_nat_pt.c
//  Author:         Eddie Shi
//  Creation Date:  12/26/01
//
//******************************************************************************
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/udp.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>

MODULE_AUTHOR("Eddie Shi <eddieshi@broadcom.com>");
MODULE_DESCRIPTION("Netfilter NAT helper for PT ");
MODULE_LICENSE("Proprietary");


/*
** Parameters passed from insmod.
** outport[25]={proto,a1,a2,proto,b1,b2,proto,c1,c2.............}
** inport[25]={proton,a1,a2,proto,b1,b2,proto,c1,c2.............}
** insmod pt.o outport=0,100,200,1,6000,7000
**             inport=0,300,400,2,800,900
**             entries=N
** where number tells us how many entries user entered
** where 1 means tcp
** where 2 means udp
** where 0 means both
*/
static int outport[300];
static int inport[300];
static int entries;
static int outcount;
static int incount;
static u_int16_t failed_port=0;

#if 0
MODULE_PARM(outport,"0-" "i");
MODULE_PARM(inport,"0-" "i");
MODULE_PARM(entries,"i");
#endif

module_param_array(outport,int,outcount,0);
module_param_array(inport,int,incount,0);
module_param(entries,int,0);

#define MAXPT_PORTS     1000
#define PTMAXEXPECTED   1000
#define MAXENTRIES     200

static int registered_port=0;

struct pt_record {
        int outport[2];         /* the outgoing port range*/
        int inport[2];          /* the incoming port range*/
        short outproto;         /* the outgoing protocol udp,tcp or both */
        short inproto;          /* the incoming protocol udp,tcp or both */
};

static struct pt_record pt_record[MAXENTRIES];

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif
static unsigned int 
help(struct ip_conntrack *ct,
	      struct ip_conntrack_expect *exp,
	      struct ip_nat_info *info,
	      enum ip_conntrack_info ctinfo,
	      unsigned int hooknum,
	      struct sk_buff **pskb)
{
	int dir = CTINFO2DIR(ctinfo);
	//struct iphdr *iph = (*pskb)->nh.iph;
//	struct udphdr *udph = (void *)iph + iph->ihl * 4;
	struct ip_conntrack_tuple repl;

	DEBUGP("............... pt_nat_help hooknum=%d\n",hooknum);
	if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
	      || (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY))) {
	DEBUGP(" pt_nat_help hooknum=%d return NF_ACCEPT\n",hooknum);
		return NF_ACCEPT;
	}

	if (!exp) {
		DEBUGP("no conntrack expectation to modify\n");
		return NF_ACCEPT;
	}

	DEBUGP(" pt_nat_help .....................\n");
	repl = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
	repl.dst.u.all = exp->tuple.dst.u.all;
	/* Start of pt A36D02078 by c47036 20060616 */
	repl.dst.protonum = exp->tuple.dst.protonum;
	/* End of pt A36D02078 by c47036 20060616 */
	DEBUGP(" pt_nat_help repl.dstport=%d\n",htons(repl.dst.u.all));
	DEBUGP("\n");
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	DEBUGP("expecting:\n ");
	DUMP_TUPLE(&repl);
	DUMP_TUPLE(&exp->mask);
DEBUGP("pt_nat_help:calling ip_conntrack_change_expect \n"); 
	ip_conntrack_change_expect(exp, &repl);

	return NF_ACCEPT;
}

static unsigned int 
pt_nat_expected(struct sk_buff **pskb,
		  unsigned int hooknum,
		  struct ip_conntrack *ct, 
		  struct ip_nat_info *info) 
{
	const struct ip_conntrack *master = ct->master->expectant;
	const struct ip_conntrack_tuple *orig = 
			&master->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	struct ip_nat_multi_range mr;

	DEBUGP("pt_nat_expected hooknum=%d\n",hooknum);
	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);
	IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

	mr.rangesize = 1;
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;

#if 1
	const struct ip_conntrack_tuple *repl =
			&master->tuplehash[IP_CT_DIR_REPLY].tuple;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (void *)iph + iph->ihl*4;
#endif
	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC) {
		//mr.range[0].min_ip = mr.range[0].max_ip = orig->dst.ip; 
		mr.range[0].min_ip = mr.range[0].max_ip = (*pskb)->nh.iph->saddr; 
		DEBUGP("MANIPSRC:orig: %u.%u.%u.%u:%u <-> %u.%u.%u.%u:%u "
			"newsrc: %u.%u.%u.%u\n",
                        NIPQUAD((*pskb)->nh.iph->saddr), ntohs(udph->source),
			NIPQUAD((*pskb)->nh.iph->daddr), ntohs(udph->dest),
			NIPQUAD(orig->dst.ip));
	} else {
		mr.range[0].min_ip = mr.range[0].max_ip = orig->src.ip;
		mr.range[0].min.all = mr.range[0].max.all = 
							orig->src.u.all;
		//eshi mr.range[0].flags |= IP_NAT_RANGE_PROTO_SPECIFIED;

		DEBUGP("MANIPDST:orig: %u.%u.%u.%u:%u <-> %u.%u.%u.%u:%u "
			"newdst: %u.%u.%u.%u:%u\n",
                        NIPQUAD((*pskb)->nh.iph->saddr), ntohs(udph->source),
                        NIPQUAD((*pskb)->nh.iph->daddr), ntohs(udph->dest),
                        NIPQUAD(orig->src.ip), ntohs(orig->src.u.udp.port));
	}

	DEBUGP("pt_nat_expected:calling ip_nat_setup_info\n"); 
	return ip_nat_setup_info(ct,&mr,hooknum);
}

static struct ip_nat_helper pt[MAXPT_PORTS];

static void fini(void)
{
	int i;

	if (failed_port == 0)
	for (i = 0 ; i < registered_port; i++) {
		DEBUGP("unregistering helper for port %d\n", ntohs(pt[i].tuple.src.u.all));
		ip_nat_helper_unregister(&pt[i]);
	}
	else
	  for (i = 0; i < registered_port-1; i++) {
		DEBUGP("unregistering helper for port %d\n", ntohs(pt[i].tuple.src.u.all));
		ip_nat_helper_unregister(&pt[i]);
	  }
}
static int dump_pt_record(void)
{
        int i;

        for (i=0; i < entries; i++) {
                printk("entry %d: \n",i);
                printk("proto %d:  outgoing: %d - %d\n",pt_record[i].outproto,pt_record[i].outport[0],pt_record[i].outport[1]);
                printk("proto %d:  incoming: %d - %d\n",pt_record[i].inproto,pt_record[i].inport[0],pt_record[i].inport[1]);
                printk("\n");
        }
	return(0);
}

static int normalize(void)
{
        int i;

	DEBUGP("normalize: entries=%d \n",entries);
        if (entries > MAXENTRIES) {
                entries = MAXENTRIES;
                printk("ip_conntrack_pt: Only support  maximum number of entries %d \n",MAXENTRIES);
        }
        for ( i=0; i< entries ; i++ ) {

          pt_record[i].outproto=outport[3*i];   /* 1=TCP,2=UDP,0=both */
          pt_record[i].outport[0]=outport[3*i + 1];
          pt_record[i].outport[1]=outport[3*i + 2];

          pt_record[i].inproto=inport[3*i];     /* 1=TCP,2=UDP,0=both */
          pt_record[i].inport[0]=inport[3*i + 1];
          pt_record[i].inport[1]=inport[3*i + 2];
        }
        dump_pt_record();
        return 0;
}

/*
** Becareful with ports that are registered already.
** ftp:21
** irc:6667
** tftp:69
** snmp: 161,162
** talk: 517,518
** h323: 1720
** pptp: 1723
** http: 80
*/
static int check_port(int port)
{
        if (port == 21 ||
            port == 6667 ||
            port == 69 ||
            port == 161 ||
            port == 162 ||
            port == 517 ||
            port == 518 ||
            port == 1720 ||
            port == 1723 ||
            port == 80)
        return 1;
        else
        return 0;
}

static int __init init(void)
{
	int i, ret=0;
	u_int16_t port;

	for (i = 0; i < MAXPT_PORTS; i++) {
	   memset(&pt[i], 0, sizeof(struct ip_nat_helper));
	}
	normalize();
	for (i = 0; i < entries && registered_port < MAXPT_PORTS; i++) {
	   for (port = pt_record[i].outport[0]; port <= pt_record[i].outport[1]; port++ ) {
	    	pt[registered_port].help = help;
		pt[registered_port].flags = 0;
		pt[registered_port].me = THIS_MODULE;
	    	pt[registered_port].expect = pt_nat_expected;

                if ( pt_record[i].outproto == 1 ) {
//printk("nat.proto=TCP port=%d\n",port);
                  pt[registered_port].tuple.src.u.tcp.port = htons(port);
                  pt[registered_port].tuple.dst.protonum = IPPROTO_TCP;
                  pt[registered_port].mask.src.u.tcp.port = 0xFFFF;
                  pt[registered_port].mask.dst.protonum = 0xFFFF;
                }
                else if ( pt_record[i].outproto == 2 ) {
//printk("nat.proto=UDP port=%d\n",port);
                  pt[registered_port].tuple.src.u.udp.port = htons(port);
                  pt[registered_port].tuple.dst.protonum = IPPROTO_UDP;
                  pt[registered_port].mask.src.u.udp.port = 0xFFFF;
                  pt[registered_port].mask.dst.protonum = 0xFFFF;
                }
                else {
//printk("nat.proto=UDP/TCP port=%d\n",port);
                  pt[registered_port].tuple.src.u.all = htons(port)<<48;
                  pt[registered_port].mask.src.u.all = 0xFFFFFFFFFFFFFFFF;
                  pt[registered_port].mask.dst.protonum = 0;
                }
	        DEBUGP("ip_nat_pt: registering for pt:%d \n",port);
		if ( check_port(port))
		  printk("ip_nat_pt: cannot register port %d (already registered by other modules)\n",port);
		else
	          ret = ip_nat_helper_register(&pt[registered_port++]);

		if ( ret < 0) registered_port--;
         /*modified by z67625 AU8D01022 配置某些Port trigger规则时，出现Port trigger功能失效 start*/
        if(port == 0xFFFF)
        {
            printk("The port reach the max,will process the next\n");
            break;
        }
         /*modified by z67625 AU8D01022 配置某些Port trigger规则时，出现Port trigger功能失效 end*/
		if( registered_port >= MAXPT_PORTS ) {
			printk( "NAT port forwarding table is full.  Remaining entries are not processed.\n" );
			pt_record[i].outport[1] = port;
			entries = i + 1;
			break;
		}

//The only time ip_nat_helper_register will fail is due to multiple entires
#if 0
	        if (ret) {
		    failed_port=port;
		    printk("ip_nat_pt: port %d  register FAILED,port already registered \n",port);
		    fini();
		    return ret;
	        }
#endif
            }

	}
	//return ret;
	return 0;
}
NEEDS_CONNTRACK(pt);

module_init(init);
module_exit(fini);
