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
//  Filename:       ip_conntrack_pt.c
//  Author:         Eddie Shi
//  Creation Date:  12/26/01
//
//******************************************************************************
/* PT for IP connection tracking. */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/ctype.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

struct module *ip_conntrack_pt = THIS_MODULE;

/*
** Parameters passed from insmod.
** outport[25]={proto,a1,a2,proto,b1,b2,proto,c1,c2.............}
** inport[25]={proton,a1,a2,proto,b1,b2,proto,c1,c2.............}
** insmod pt.o outport=0,100,200,1,6000,7000 
**             inport=0,300,400,2,800,900
**	       entries=N
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

module_param_array(outport, int, outcount, 0);
module_param_array(inport, int, incount, 0);
module_param(entries, int, 0);

#define MAXPT_PORTS	1000
#define PTMAXEXPECTED	1000
#define MAXENTRIES	200

static int registered_port=0;

struct pt_record {
	u_int16_t outport[2];		/* the outgoing port range*/
	u_int16_t inport[2];		/* the incoming port range*/
	u_int16_t outproto;		/* the outgoing protocol udp,tcp or both */
	u_int16_t inproto;		/* the incoming protocol udp,tcp or both */
};
static struct pt_record pt_record[MAXENTRIES];

static int  normalize(void);

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif


/* FIXME: This should be in userspace.  Later. */
static int help(struct sk_buff *skb,
		struct ip_conntrack *ct,
		enum ip_conntrack_info ctinfo)
{
	/* tcplen not negative guaranteed by ip_conntrack_tcp.c */
	struct iphdr *iph = skb->nh.iph;
        struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
	struct udphdr *udph = (void *)iph + iph->ihl * 4;
  	unsigned int tcplen = skb->len - iph->ihl*4;
  	unsigned int udplen = skb->len - iph->ihl*4;

	int dir = CTINFO2DIR(ctinfo);
        struct ip_conntrack_expect *exp;
	int i;
    /*start of HG553 2008.05.07 V100R001C02B013 AU8D00612 by c65985 */
	u_int32_t port;
    /*end of HG553 2008.05.07 V100R001C02B013 AU8D00612 by c65985 */
	u_int16_t outproto;
	u_int16_t outport;
/* start of AU8D01163 2008-11-19 by z67625 Port Trigger对UDP端口一对多功能失效 */
  	/* Until there's been traffic both ways, don't look in packets. */
        /*if (ctinfo != IP_CT_ESTABLISHED
            && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
                DEBUGP("pt: Conntrackinfo = %u\n", ctinfo);
                return NF_ACCEPT;
        }*/
	if ( iph->protocol == IPPROTO_TCP ) {

        if (ctinfo != IP_CT_ESTABLISHED
            && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
                DEBUGP("pt: Conntrackinfo = %u\n", ctinfo);
                return NF_ACCEPT;
        }
/* end of AU8D01163 2008-11-19 by z67625 Port Trigger对UDP端口一对多功能失效 */
		/* Not whole TCP header? */
        	if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff*4) {
                  DEBUGP("pt: tcplen = %u\n", (unsigned)tcplen);
//		  printk("ip_conntrack_pt: Not whole TCP header \n");
                  return NF_ACCEPT;
        	}

        	/* Checksum invalid?  Ignore. */
        	/* FIXME: Source route IP option packets --RR */
        	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
                         csum_partial((char *)tcph, tcplen, 0))) {
//                      printk("ip_conntrack_pt: bad TCP checksum \n");
                	return NF_ACCEPT;
        	}
        }
	else if ( iph->protocol == IPPROTO_UDP ) {
		/* Not whole UDP header? */
        	if (udplen < sizeof(struct udphdr)) {
//                	printk("ip_conntrack_pt: too short for udph, udplen = %u\n", (unsigned)udplen);
                	return NF_ACCEPT;
        	}

	}
	else {
//             	printk("ip_conntrack_pt: unknow protocol (ACCEPT)\n");
                return NF_ACCEPT;
	}

	DEBUGP("pt_help: Conntrackinfo = %u dir=%d\n", ctinfo,dir);
        DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
        DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
/*
** We need to search the pt_record[] to see what inports need to be triggered
** based on the tuple's destination protonum and port number 
** we know from the ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple the protonum and port number
** ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum
** ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all
*/
	//dump_pt_record();
	outproto = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
	outport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all >> 48;
	//printk(" all=0x%08Lx \n",ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all);
	//printk("pt_help: outport=%d \n",outport);
	/*convert proto */
	if (outproto == IPPROTO_TCP)
		outproto=1;
	else if (outproto == IPPROTO_UDP)
		outproto=2;
	else
		outproto=0;

	//dump_pt_record();
	for(i = 0; i < entries; i++) {
	  /* we only call expect_related for entries match our outgoing port(outport) */
	  if ((outproto == pt_record[i].outproto || pt_record[i].outproto== 0) && (ntohs(outport) >= pt_record[i].outport[0] && ntohs(outport) <= pt_record[i].outport[1])) { 
	    for (port = pt_record[i].inport[0]; port <= pt_record[i].inport[1];port++) { 
	exp = ip_conntrack_expect_alloc();
	if ( exp == NULL ) {
		printk("**** ip_conntrack_expect_alloc() failed ***\n");
		goto out;
	}
        memset(exp, 0, sizeof(exp));
        exp->tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;

		if ( pt_record[i].inproto == 1 ) {
            	   exp->tuple.dst.protonum = IPPROTO_TCP;
            	   exp->tuple.dst.u.tcp.port = htons(port);
//printk("exp->port=%d \n",exp->tuple.dst.u.tcp.port);
            	   exp->mask.src.ip = 0;
            	   exp->mask.src.u.all = 0;
            	   exp->mask.dst.ip = 0xffffffff;
            	   exp->mask.dst.u.tcp.port = 0xffff;
            	   exp->mask.dst.protonum = 0xffff;
		}
		else if ( pt_record[i].inproto == 2 ) {
            	   exp->tuple.dst.u.udp.port = htons(port);
//printk("exp->port=%d \n",exp->tuple.dst.u.udp.port);
            	   exp->tuple.dst.protonum = IPPROTO_UDP;
            	   //exp->mask.src.ip = 0xffffffff;
            	   exp->mask.src.ip = 0;
            	   exp->mask.src.u.all = 0;
            	   exp->mask.dst.ip = 0xffffffff;
            	   exp->mask.dst.u.udp.port = 0xffff;
            	   exp->mask.dst.protonum = 0xffff;
		}
		else {
            	   exp->tuple.dst.u.all =  htons(port);
            	   exp->tuple.dst.u.all <<= 48;
            	   //exp->mask.src.ip = 0xffffffff;
            	   exp->mask.src.ip = 0;
            	   exp->mask.src.u.all = 0;
            	   exp->mask.dst.ip = 0xffffffff;
            	   exp->mask.dst.u.all = 0xffffffffffffffff;
            	   exp->mask.dst.protonum = 0;
		}
                exp->expectfn = NULL;

                //DEBUGP("expect: ");
                //DUMP_TUPLE(exp->tuple);
                //DUMP_TUPLE(exp->mask);
                ip_conntrack_expect_related(exp, ct);
	    } /* end of for */
	  } /* end of if */
	}
out:
        return NF_ACCEPT;
}

static struct ip_conntrack_helper pt[MAXPT_PORTS];

/* Not __exit: called from init() */
static void fini(void)
{
	int i;
	if (failed_port == 0)
	  for (i = 0; i < registered_port ; i++) {
		DEBUGP("ip_conntrack_pt: unregistering helper for port %d\n",ntohs(pt[i].tuple.src.u.all));
		ip_conntrack_helper_unregister(&pt[i]);
	  }
	 else
	    for (i = 0; i < registered_port-1 ; i++) {
		DEBUGP("ip_conntrack_pt: unregistering helper for port %d\n",ntohs(pt[i].tuple.src.u.all));
		ip_conntrack_helper_unregister(&pt[i]);
	     }
}
static int dump_pt_record(void)
{
	int i;

	for (i=0;i<entries;i++) {
		printk("entry %d: \n",i);
		printk("proto %d:  outgoing: %d - %d\n",pt_record[i].outproto,pt_record[i].outport[0],pt_record[i].outport[1]);
		printk("proto %d:  incoming: %d - %d\n",pt_record[i].inproto,pt_record[i].inport[0],pt_record[i].inport[1]);
		printk("\n");
	}
	return 0;
}
static int  normalize(void)
{
	int i;

	DEBUGP("normalize: entries=%d \n",entries);
	if (entries > MAXENTRIES) {
		entries = MAXENTRIES;
		printk("ip_conntrack_pt: Only support  maximum number of entries %d \n",MAXENTRIES); 
	}
	for ( i=0; i< entries ; i++ ) {

	  pt_record[i].outproto=outport[3*i]; 	/* 1=TCP,2=UDP,0=both */
	  pt_record[i].outport[0]=outport[3*i + 1];
	  pt_record[i].outport[1]=outport[3*i + 2];

	  pt_record[i].inproto=inport[3*i]; 	/* 1=TCP,2=UDP,0=both */
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

	/* nornamlize the input */
	normalize();

	for (i=0;i<MAXPT_PORTS;i++)
	     memset(&pt[i], 0, sizeof(struct ip_conntrack_helper));

	for (i = 0; i < entries && registered_port < MAXPT_PORTS; i++) {
	    for (port = pt_record[i].outport[0]; port <= pt_record[i].outport[1]; port++ ) {

	        pt[registered_port].max_expected = PTMAXEXPECTED;
                pt[registered_port].timeout = 0;
                pt[registered_port].flags = IP_CT_HELPER_F_REUSE_EXPECT;
                pt[registered_port].me = ip_conntrack_pt;
	        pt[registered_port].help = help;

		if ( pt_record[i].outproto == 1 ) {
		  pt[registered_port].tuple.src.u.tcp.port = htons(port);
		  pt[registered_port].tuple.dst.protonum = IPPROTO_TCP;
		  pt[registered_port].mask.src.u.tcp.port = 0xFFFF;
		  pt[registered_port].mask.dst.protonum = 0xFFFF;
		}
		else if ( pt_record[i].outproto == 2 ) {
		  pt[registered_port].tuple.src.u.udp.port = htons(port);
		  pt[registered_port].tuple.dst.protonum = IPPROTO_UDP;
		  pt[registered_port].mask.src.u.udp.port = 0xFFFF;
		  pt[registered_port].mask.dst.protonum = 0xFFFF;
		}
		else {
//printk("registe: htons(port)=%x port=%x \n",htons(port),port);
		  pt[registered_port].tuple.src.u.all = htons(port)<< 48;
//printk("registe: TCP/UCP all=0x%8Lx \n",pt[registered_port].tuple.src.u.all);
		  pt[registered_port].mask.src.u.all = 0xFFFFFFFFFFFFFFFF;
		  pt[registered_port].mask.dst.protonum = 0;
		}
	    	DEBUGP("ip_conntrack_pt: registering helper for port %d\n",port);
		if ( check_port(port) )
	    	    printk("ip_conntrack_pt: cannot register port %d (already registered by other module)\n",port);
		else
	    	    ret = ip_conntrack_helper_register(&pt[registered_port++]);

		if( registered_port >= MAXPT_PORTS ) {
			printk( "Conntrack port forwarding table is full.  Remaining entries are not processed.\n" );
			pt_record[i].outport[1] = port;
			entries = i + 1;
			break;
		}
        /*modified by z67625 AU8D01022 配置某些Port trigger规则时，出现Port trigger功能失效 start*/
        if(port == 0xFFFF)
        {
            printk("The port reach the max,will process the next\n");
            break;
        }
        /*modified by z67625 AU8D01022 配置某些Port trigger规则时，出现Port trigger功能失效 end*/

	    	if (ret) {
		   failed_port = port;
	           printk("ip_conntrack_pt: port %d register FAILED,port already registered  \n",port); 
	           fini();
	           return ret;
	    	}
	    }
	}

	return 0;
}


MODULE_AUTHOR("Eddie Shi <eddieshi@broadcom.com>");
MODULE_DESCRIPTION("Netfilter Conntrack helper for PT ");
MODULE_LICENSE("Proprietary");

PROVIDES_CONNTRACK(pt);
module_init(init);
module_exit(fini);
