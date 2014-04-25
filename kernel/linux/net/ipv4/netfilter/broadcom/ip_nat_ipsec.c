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
//  Filename:       ip_nat_ipsec.c
//  Author:         Pavan Kumar
//  Creation Date:  05/27/04
//
//  Description:
//      Implements the IPSec ALG connectiontracking.
//
*****************************************************************************/
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <net/tcp.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_ipsec.h>
#include <linux/netfilter_ipv4/ip_conntrack_ipsec.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

MODULE_AUTHOR("Pavan Kumar <pavank@broadcom.com>");
MODULE_DESCRIPTION("Netfilter NAT helper for ipsec");
MODULE_LICENSE("Proprietary");

#define MAX_PORTS          253
#define UDP_HLEN           8
#define IPSEC_FREE         0
#define IPSEC_INUSE        1
#define REFRESH_TIMEOUT    (10 * HZ)
#define CT_REFRESH_TIMEOUT (13 * HZ)
#define CT_15MIN_TIME      (60 * HZ * 15)
#define MAX_TIMEOUT_COUNT  ((CT_15MIN_TIME)/REFRESH_TIMEOUT)

static int ports[MAX_PORTS];
static int ports_c = 0;
#ifdef MODULE_PARM
MODULE_PARM(ports,"1-" __MODULE_STRING(MAX_PORTS) "i");
MODULE_PARM_DESC(ports, "port numbers of IPSEC");
#endif

struct _ipsec_table {
    ulong               initcookie[2];
    u_int32_t           l_ip;
    struct ip_conntrack *ct;
    struct timer_list   refresh_timer;
    int                 timer_active;
    int                 pkt_rcvd;
    int                 ntimeouts;
    int                 port_masq_flag;
    int                 inuse;
} ipsec_table[MAX_PORTS];

#undef IPSEC_NAT_DEBUG
#if IPSEC_NAT_DEBUG
#define IPSEC_DUMP_TUPLE(tp)                                    \
printk(KERN_DEBUG "tuple %p: %u %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu\n",     \
       (tp), (tp)->dst.protonum,                                \
       NIPQUAD((tp)->src.ip), ntohs((tp)->src.u.all),           \
       NIPQUAD((tp)->dst.ip), ntohs((tp)->dst.u.all))
#endif

static struct _ipsec_table *
ipsec_alloc_entry(int *index)
{
    int idx = 0;

    for (; idx < MAX_PORTS; idx++) {
        if (ipsec_table[idx].inuse) {
            continue;
        }
        *index = idx;
        memset(&ipsec_table[idx], 0, sizeof(struct _ipsec_table));
        return (&ipsec_table[idx]);
    }
    return NULL;
}

static void
ipsec_free_entry(int index)
{
    if (ipsec_table[index].inuse) {
        if (ipsec_table[index].timer_active) {
            del_timer(&ipsec_table[index].refresh_timer);
        }
        memset(&ipsec_table[index], 0, sizeof(struct _ipsec_table));
    }
}

static void
ipsec_refresh_ct(unsigned long data)
{
    struct _ipsec_table *ipsec_entry = NULL;

    if (data > MAX_PORTS) {
        return;
    }

    ipsec_entry = &ipsec_table[data];
#if IPSEC_NAT_DEBUG
    printk(KERN_DEBUG "ntimeouts %d pkt_rcvd %d entry %p data %lu ct %p\n",
           ipsec_entry->ntimeouts, ipsec_entry->pkt_rcvd, ipsec_entry, data,
           ipsec_entry->ct);
#endif
    if (ipsec_entry->pkt_rcvd) {
        ipsec_entry->pkt_rcvd  = 0;
        ipsec_entry->ntimeouts = 0;
    } else {
        ipsec_entry->ntimeouts++;
        if (ipsec_entry->ntimeouts >= MAX_TIMEOUT_COUNT) {
            ipsec_free_entry(data);
            return;
        }
    }
    ipsec_entry->refresh_timer.expires = jiffies + REFRESH_TIMEOUT;
    ipsec_entry->refresh_timer.function = ipsec_refresh_ct;
    ipsec_entry->refresh_timer.data = data;
    ip_ct_refresh(ipsec_entry->ct, CT_REFRESH_TIMEOUT);
    add_timer(&ipsec_entry->refresh_timer);
    ipsec_entry->timer_active = 1;
}

/*
 * Search an IPSEC table entry by the initiator cookie.
 */
struct _ipsec_table *
search_ipsec_entry_by_cookie ( struct isakmphdr *isakmph )
{
    int idx = 0;
    struct _ipsec_table *ipsec_entry = ipsec_table;

    for ( ; idx < MAX_PORTS; idx++ ) {
#if IPSEC_NAT_DEBUG
        printk(KERN_DEBUG "Searching Cookie 0x%x:0x%x <-> 0x%x:0x%x\n",
                          ntohl(isakmph->initcookie[0]),
                          ntohl(isakmph->initcookie[1]),
                          ntohl(ipsec_entry->initcookie[0]),
                          ntohl(ipsec_entry->initcookie[1]));
#endif
        if ( (ntohl(isakmph->initcookie[0]) == ntohl(ipsec_entry->initcookie[0])) &&
             (ntohl(isakmph->initcookie[1]) == ntohl(ipsec_entry->initcookie[1]))) {
            return ipsec_entry;
        }
        ipsec_entry++;
    }
    return NULL;
}

/*
 * Search an IPSEC table entry by the source IP address.
 */
struct _ipsec_table *
search_ipsec_entry_by_addr ( struct isakmphdr *isakmph,
                                              const struct iphdr *iph, int *index )
{
    int idx = 0;
    struct _ipsec_table *ipsec_entry = ipsec_table;

    for ( ; idx < MAX_PORTS; idx++ ) {

#if IPSEC_NAT_DEBUG
        printk(KERN_DEBUG "Looking up %u.%u.%u.%u table entry %u.%u.%u.%u\n",
               NIPQUAD(ntohl(iph->saddr)), NIPQUAD(ntohl(ipsec_entry->l_ip)));
#endif
        if ( ntohl(ipsec_entry->l_ip) == ntohl(iph->saddr) ) {
#if IPSEC_NAT_DEBUG
            printk(KERN_DEBUG "Search by addr returning entry %p\n", ipsec_entry);
#endif
            *index = idx;
            return ipsec_entry;
        }
        ipsec_entry++;
    }
    return NULL;
}

static unsigned int 
ipsec_nat_help(struct ip_conntrack *ct,
    struct ip_conntrack_expect *exp,
    struct ip_nat_info *info,
    enum ip_conntrack_info ctinfo,
    unsigned int hooknum,
    struct sk_buff **pskb)
{
    int dir = CTINFO2DIR(ctinfo);
    struct iphdr              *iph = (*pskb)->nh.iph;
    struct udphdr             *udph = (void *)iph + iph->ihl * 4;
    struct isakmphdr *isakmph = (void *)udph + 8;
    struct _ipsec_table       *ipsec_entry = NULL;
    int                       index = 0;

    /* The initiator cookie is not supposed to be 0 but the AT&T VPN client
     * sends packets this way.  Just accept the packet.
     */
    if( isakmph->initcookie[0] == 0 && isakmph->initcookie[1] == 0 )
        return NF_ACCEPT;

#if IPSEC_NAT_DEBUG
    printk("\n%s ", (hooknum == NF_IP_POST_ROUTING) ? "NF_IP_POST_ROUTING" : "NF_IP_PRE_ROUTING");
    printk("%s ", (dir == IP_CT_DIR_ORIGINAL) ? "IP_CT_DIR_ORIGINAL" : "IP_CT_DIR_REPLY");
    IPSEC_DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
    IPSEC_DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
#endif
    LOCK_BH(&ip_ipsec_lock);
    if (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_ORIGINAL)
    {
#if IPSEC_NAT_DEBUG
        printk( KERN_DEBUG "%s:%s PRE_ROUTING UDP Header (%hu:%hu)\n", 
                __FILE__, __FUNCTION__, udph->source, udph->dest);
#endif
        if ( (ipsec_entry = search_ipsec_entry_by_cookie ( isakmph )) == NULL ) {
            if ( (ipsec_entry = search_ipsec_entry_by_addr ( isakmph, iph, &index )) == NULL ) {
                ipsec_entry = ipsec_alloc_entry (&index);
                if ( ipsec_entry == NULL ) {
                    /* All entries are currently in use */
                    printk (KERN_DEBUG "%s:%s Out of table entries\n",
                            __FILE__, __FUNCTION__);
                    UNLOCK_BH(&ip_ipsec_lock);
                    return NF_DROP;
                }
            } else {
                /* Update our cookie information */
                ipsec_entry->initcookie[0] = isakmph->initcookie[0];
                ipsec_entry->initcookie[1] = isakmph->initcookie[1];
                ipsec_entry->ct            = ct;
                if ( udph->source == IPSEC_UDP_PORT && udph->dest == IPSEC_UDP_PORT ) {
                    ipsec_entry->port_masq_flag = 1;
                } else {
                    ipsec_entry->port_masq_flag = 0;
                }
                /* We have seen traffic from this client before */
                UNLOCK_BH(&ip_ipsec_lock);
                return NF_ACCEPT;
            }
#if IPSEC_NAT_DEBUG
            printk("created ipsec entry %p ct %p\n", ipsec_entry, ct);
#endif
            /*
             * One more check before we fill it with the new information.
             * Kill the timer if there was a connection with this table
             * entry.
             */
            if (ipsec_entry->inuse) {
                if (ipsec_entry->timer_active) {
                    del_timer(&ipsec_entry->refresh_timer);
                    memset(ipsec_entry, 0, sizeof(struct _ipsec_table));
                }
            }
            ipsec_entry->ct                     = ct;
            ipsec_entry->initcookie[0]          = isakmph->initcookie[0];
            ipsec_entry->initcookie[1]          = isakmph->initcookie[1];
            ipsec_entry->l_ip                   = iph->saddr;
            init_timer(&ipsec_entry->refresh_timer);
            ipsec_entry->refresh_timer.expires  = jiffies + REFRESH_TIMEOUT;
            ipsec_entry->refresh_timer.function = ipsec_refresh_ct;
            ipsec_entry->refresh_timer.data     = index;
            ipsec_entry->inuse                  = IPSEC_INUSE;
            add_timer(&ipsec_entry->refresh_timer);
            ipsec_entry->timer_active           = 1;
            if ( udph->source == IPSEC_UDP_PORT && udph->dest == IPSEC_UDP_PORT ) {
                ipsec_entry->port_masq_flag           = 1;
            }
        } else {
            /*
             * Some clients are changing there source port after a
             * couple of transactions of ISAKMP so make another check
             * here to see if the client has changed the source port
             * if it has changed the source port, then turn off the
             * masquerade flag so that on the way out we do not
             * mangle it.
             */
             if ( udph->source == IPSEC_UDP_PORT && udph->dest == IPSEC_UDP_PORT ) {
                 ipsec_entry->port_masq_flag = 1;
             } else {
                 ipsec_entry->port_masq_flag = 0;
             }
        }
    }

    /*
     *      for original direction (outgoing), masquerade the UDP
     *      port and IP address.
     */
    if(hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
    {
        if ( (ipsec_entry = search_ipsec_entry_by_cookie ( isakmph )) == NULL ) {
            if ( (ipsec_entry = search_ipsec_entry_by_addr ( isakmph, iph, &index )) == NULL ) {
                /* if detected in post-routing but not pre-routing, it should be locally generated => add to table */
                ipsec_entry = ipsec_alloc_entry (&index);
                if ( ipsec_entry == NULL ) {
                    /* All entries are currently in use */
                    printk (KERN_DEBUG "%s:%s Out of table entries\n",
                            __FILE__, __FUNCTION__);
                    UNLOCK_BH(&ip_ipsec_lock);
                    return NF_DROP;
                }
                if (ipsec_entry->inuse) {
                    if (ipsec_entry->timer_active) {
                        del_timer(&ipsec_entry->refresh_timer);
                        memset(ipsec_entry, 0, sizeof(struct _ipsec_table));
                    }
                }
                ipsec_entry->ct                     = ct;
                ipsec_entry->initcookie[0]          = isakmph->initcookie[0];
                ipsec_entry->initcookie[1]          = isakmph->initcookie[1];
                ipsec_entry->l_ip                   = iph->saddr;
                init_timer(&ipsec_entry->refresh_timer);
                ipsec_entry->refresh_timer.expires  = jiffies + REFRESH_TIMEOUT;
                ipsec_entry->refresh_timer.function = ipsec_refresh_ct;
                ipsec_entry->refresh_timer.data     = index;
                ipsec_entry->inuse                  = IPSEC_INUSE;
                add_timer(&ipsec_entry->refresh_timer);
                ipsec_entry->timer_active           = 1;
                if ( udph->source == IPSEC_UDP_PORT && udph->dest == IPSEC_UDP_PORT ) {
                    ipsec_entry->port_masq_flag           = 1;
                }
            }
            /* Update our cookie information */
            ipsec_entry->initcookie[0] = isakmph->initcookie[0];
            ipsec_entry->initcookie[1] = isakmph->initcookie[1];
        }
        if ( ipsec_entry->port_masq_flag == 1 ) {
#if IPSEC_NAT_DEBUG
            printk (KERN_DEBUG "%s:%s Masquerading Source Port\n", __FILE__, __FUNCTION__);
#endif
            udph->source = htons(IPSEC_UDP_PORT);
            udph->check = 0;
        }
    }

    if(hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY)
    {
        if ( (ipsec_entry = search_ipsec_entry_by_cookie ( isakmph )) == NULL ) {
            if ( (ipsec_entry = search_ipsec_entry_by_addr ( isakmph, iph, &index )) == NULL ) {
                printk (KERN_DEBUG "%s:%s Out of table entries\n",
                        __FILE__, __FUNCTION__);
                UNLOCK_BH(&ip_ipsec_lock);
                return NF_DROP;
            }
            /* Update our cookie information */
            ipsec_entry->initcookie[0] = isakmph->initcookie[0];
            ipsec_entry->initcookie[1] = isakmph->initcookie[1];
        }
#if IPSEC_NAT_DEBUG
        printk("retrieved entry %p ipsec ct %p\n", ipsec_entry, ct);
#endif
        ipsec_entry->pkt_rcvd++;
        iph->daddr = ipsec_entry->l_ip;
        udph->check = 0;
        iph->check = 0;
        iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
    }
    UNLOCK_BH(&ip_ipsec_lock);

    return NF_ACCEPT;
}

static unsigned int 
ipsec_nat_expected(struct sk_buff **pskb,
    unsigned int hooknum,
    struct ip_conntrack *ct, 
    struct ip_nat_info *info) 
{
    const struct ip_conntrack *master = ct->master->expectant;
    const struct ip_conntrack_tuple *orig = 
            &master->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    struct ip_nat_multi_range mr;
#if IPSEC_NAT_DEBUG
    struct iphdr  *iph  = (*pskb)->nh.iph;
    struct udphdr *udph = (void *)iph + iph->ihl * 4;
#endif

    IP_NF_ASSERT(info);
    IP_NF_ASSERT(master);
    IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

    mr.rangesize = 1;
    mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;

    if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC) {
        mr.range[0].min_ip = mr.range[0].max_ip = orig->dst.ip; 
#if IPSEC_NAT_DEBUG
        printk( KERN_DEBUG "%s:%s MANIP_SRC: %u.%u.%u.%u:%u <-> %u.%u.%u.%u:%u "
                "newsrc: %u.%u.%u.%u\n", __FILE__, __FUNCTION__,
                NIPQUAD((*pskb)->nh.iph->saddr), ntohs(udph->source),
                NIPQUAD((*pskb)->nh.iph->daddr), ntohs(udph->dest),
                NIPQUAD(orig->dst.ip));
#endif
    }
    if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST) {
        mr.range[0].min_ip = mr.range[0].max_ip = orig->src.ip;
        mr.range[0].min.udp.port = mr.range[0].max.udp.port = 
                            orig->src.u.udp.port;
        mr.range[0].flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
#if IPSEC_NAT_DEBUG
        printk( KERN_DEBUG "%s:%s MANIP_DEST: %u.%u.%u.%u:%u <-> %u.%u.%u.%u:%u "
                "newdst: %u.%u.%u.%u:%u\n", __FILE__, __FUNCTION__,
                NIPQUAD((*pskb)->nh.iph->saddr), ntohs(udph->source),
                NIPQUAD((*pskb)->nh.iph->daddr), ntohs(udph->dest),
                NIPQUAD(orig->src.ip), ntohs(orig->src.u.udp.port));
#endif
    }

    return ip_nat_setup_info(ct,&mr,hooknum);
}

static struct ip_nat_helper ipsec[MAX_PORTS];
static char ipsec_names[MAX_PORTS][10];

static void 
fini(void)
{
    int i;

    for (i = 0 ; i < ports_c; i++) {
#if IPSEC_NAT_DEBUG
        printk( KERN_DEBUG "%s:%sunregistering helper for port %d\n",
                __FILE__, __FUNCTION__, ports[i]);
#endif
        ip_nat_helper_unregister(&ipsec[i]);
    }
    /* Delete the timers if any */
    for (i = 0; i < MAX_PORTS; i++) {
        if (ipsec_table[i].inuse) {
            if (ipsec_table[i].timer_active) {
                del_timer(&ipsec_table[i].refresh_timer);
            }
        }
    }
}

static int __init 
init(void)
{
    int i, ret = 0;
    char *tmpname;

    if (!ports[0])
        ports[0] = IPSEC_UDP_PORT;

    for (i = 0 ; (i < MAX_PORTS) && ports[i] ; i++) {
        ipsec[i].tuple.dst.protonum = IPPROTO_UDP;
        ipsec[i].tuple.src.u.udp.port = htons(ports[i]);
        ipsec[i].mask.dst.protonum = 0xFFFF;
        ipsec[i].mask.src.u.udp.port = 0xFFFF;
        ipsec[i].help = ipsec_nat_help;
        ipsec[i].flags = 0;
        ipsec[i].me = THIS_MODULE;
        ipsec[i].expect = ipsec_nat_expected;

        tmpname = &ipsec_names[i][0];
        if (ports[i] == IPSEC_UDP_PORT)
            sprintf(tmpname, "ipsec");
        else
            sprintf(tmpname, "ipsec-%d", i);
        ipsec[i].name = tmpname;
#if IPSEC_NAT_DEBUG        
        printk( KERN_DEBUG "%s:%s registering for port %d: name %s\n",
              __FILE__, __FUNCTION__, ports[i], ipsec[i].name);
#endif
        ret = ip_nat_helper_register(&ipsec[i]);

        if (ret) {
#if IPSEC_NAT_DEBUG
            printk(KERN_DEBUG "ip_nat_ipsec: unable to register"
                   " for port %d\n", ports[i]);
#endif
            fini();
            return ret;
        }
        ports_c++;
    }
    for (i = 0; i < MAX_PORTS; i++) {
        memset(&ipsec_table[i], 0, sizeof(struct _ipsec_table));
        ipsec_table[i].initcookie[0] = ipsec_table[i].initcookie[1] = 0xdeadbeef;
    }
    return ret;
}

module_init(init);
module_exit(fini);
