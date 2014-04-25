/*****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       ip_conntrack_ipsec.c
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the IPSec ALG connectiontracking.
//
*****************************************************************************/
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_ipsec.h>

MODULE_AUTHOR("Pavan Kumar <pavank@broadcom.com>");
MODULE_DESCRIPTION("Netfilter connection tracking module for ipsec");
MODULE_LICENSE("Proprietary");

#define MAX_PORTS          253

static int ports[MAX_PORTS];
static int ports_c = 0;
#ifdef MODULE_PARM
MODULE_PARM(ports, "1-" __MODULE_STRING(MAX_PORTS) "i");
MODULE_PARM_DESC(ports, "port numbers of IPSEC");
#endif

#define IPSEC_CONNTRACK_DEBUG 0
#if IPSEC_CONNTRACK_DEBUG
#define IPSEC_DUMP_TUPLE(tp)                                               \
printk(KERN_DEBUG "tuple %p: %u %u.%u.%u.%u:%hu <-> %u.%u.%u.%u:%hu\n",    \
       (tp), (tp)->dst.protonum,                                           \
       NIPQUAD((tp)->src.ip), ntohs((tp)->src.u.all),                      \
       NIPQUAD((tp)->dst.ip), ntohs((tp)->dst.u.all))
#endif

/*
 * Handle an incoming packet.
 */
static int
ipsec_help(struct sk_buff *skb,
        struct ip_conntrack *ct,
        enum ip_conntrack_info ctinfo)
{
    struct ip_conntrack_expect *exp;

#if IPSEC_CONNTRACK_DEBUG
    IPSEC_DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
    IPSEC_DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
    printk (KERN_DEBUG "Received packet Direction %s\n",
            ( ctinfo >= IP_CT_IS_REPLY ) ? "REPLY" : "ORIG");
#endif
    /*
     * We got to trust the netfilter to call this routine only on
     * receiving the first packet without an expectation.
     * There will be at least one more packet for this tuple
     * so set our expectation for it here.
     */
     /* Allocate expectation which will be inserted */
     exp = ip_conntrack_expect_alloc();
     if (exp == NULL) {
             goto out;
     }
     memset(exp, 0, sizeof(exp));

     exp->tuple                       = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
     exp->mask.src.ip                 = 0xFFFFFFFF;
     exp->mask.dst.ip                 = 0xFFFFFFFF;
     exp->mask.dst.u.udp.port         = ntohs(IPSEC_UDP_PORT);
     exp->mask.dst.protonum           = 0xFFFF;
     exp->expectfn                    = NULL;
#if IPSEC_CONNTRACK_DEBUG
     printk( KERN_DEBUG "%s:%s expect: 0x%x 0x%x"
             " %u.%u.%u.%u:%u\n <--> %u.%u.%u.%u:%u\n", __FILE__, __FUNCTION__,
             ntohl(isakmph->initcookie[0]), ntohl(isakmph->initcookie[1]),
             NIPQUAD(iph->saddr), udph->source, NIPQUAD(iph->daddr), udph->dest);
#endif
     ip_conntrack_expect_related(exp, ct);
out:
    return NF_ACCEPT;
}

static struct ip_conntrack_helper ipsec[MAX_PORTS];
static char ipsec_names[MAX_PORTS][10];

static void
fini(void)
{
    int i;

    for (i = 0 ; i < ports_c; i++) {
        //printk( KERN_DEBUG "%s:%s unregistering helper for port %d\n",
        //      __FILE__, __FUNCTION__, ports[i]);
        ip_conntrack_helper_unregister(&ipsec[i]);
    } 
}

static int __init
init(void)
{
    int i, ret;
    char *tmpname;

    if (!ports[0])
        ports[0] = IPSEC_UDP_PORT;

    for (i = 0 ; (i < MAX_PORTS) && ports[i] ; i++) {
        /* Create helper structure */
        ipsec[i].tuple.dst.protonum   = IPPROTO_UDP;
        ipsec[i].tuple.src.u.udp.port = htons(ports[i]);
        ipsec[i].mask.dst.protonum    = 0xFFFF;
        ipsec[i].mask.src.u.udp.port  = 0xFFFF;
        ipsec[i].max_expected         = 1;
        ipsec[i].timeout              = 0;
        ipsec[i].flags                = IP_CT_HELPER_F_REUSE_EXPECT;
        ipsec[i].me                   = THIS_MODULE;
        ipsec[i].help                 = ipsec_help;
        tmpname                     = &ipsec_names[i][0];
        if (ports[i] == IPSEC_UDP_PORT)
            sprintf(tmpname, "ipsec");
        else
            sprintf(tmpname, "ipsec-%d", i);
        ipsec[i].name   = tmpname;

        //printk( KERN_DEBUG "%s:%sport #%d: %d\n", __FILE__,
        //      __FUNCTION__, i, ports[i]);

        ret=ip_conntrack_helper_register(&ipsec[i]);
        if (ret) {
            //printk("ERROR registering helper for port %d\n", ports[i]);
            fini();
            return(ret);
        }
        ports_c++;
    }
    return(0);
}
PROVIDES_CONNTRACK(ipsec);
module_init(init);
module_exit(fini);
