/*
 *	Spanning tree protocol; interface code
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_stp_if.c,v 1.2 2008/10/29 03:32:38 l65130 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/smp_lock.h>

#include "br_private.h"
#include "br_private_stp.h"

/*start of Global 解决一线br0mac跳变问题 by c00131380 at 2008.10.29*/
int g_iInitbr0Flag = 0;  //初始化br0基MAC完成标志，1:已初始过 0:未初始化
/*end of Global 解决一线br0mac跳变问题 by c00131380 at 2008.10.29*/

/* Port id is composed of priority and port number.
 * NB: least significant bits of priority are dropped to
 *     make room for more ports.
 */
static inline port_id br_make_port_id(__u8 priority, __u16 port_no)
{
	return ((u16)priority << BR_PORT_BITS) 
		| (port_no & ((1<<BR_PORT_BITS)-1));
}

/* called under bridge lock */
void br_init_port(struct net_bridge_port *p)
{
	p->port_id = br_make_port_id(p->priority, p->port_no);
	br_become_designated_port(p);
	p->state = BR_STATE_BLOCKING;
	p->topology_change_ack = 0;
	p->config_pending = 0;

	br_stp_port_timer_init(p);
}

/* called under bridge lock */
void br_stp_enable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	mod_timer(&br->hello_timer, jiffies + br->hello_time);
	br_config_bpdu_generation(br);

	// brcm
	br->dev->trans_start = jiffies * 100 / HZ;

	list_for_each_entry(p, &br->port_list, list) {
		if ((p->dev->flags & IFF_UP) && netif_carrier_ok(p->dev))
			br_stp_enable_port(p);

	}
	spin_unlock_bh(&br->lock);
}

/* NO locks held */
void br_stp_disable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock(&br->lock);
	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED)
			br_stp_disable_port(p);

	}

	br->topology_change = 0;
	br->topology_change_detected = 0;
	spin_unlock(&br->lock);

	// brcm
	br->dev->trans_start = jiffies * 100 / HZ;

	del_timer_sync(&br->hello_timer);
	del_timer_sync(&br->topology_change_timer);
	del_timer_sync(&br->tcn_timer);
}

/* called under bridge lock */
void br_stp_enable_port(struct net_bridge_port *p)
{
	br_init_port(p);
	br_port_state_selection(p->br);
}

/* called under bridge lock */
void br_stp_disable_port(struct net_bridge_port *p)
{
	struct net_bridge *br;
	int wasroot;

	br = p->br;
	printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
	       br->dev->name, p->port_no, p->dev->name, "disabled");

	wasroot = br_is_root_bridge(br);
	br_become_designated_port(p);
	p->state = BR_STATE_DISABLED;
	p->topology_change_ack = 0;
	p->config_pending = 0;

	del_timer(&p->message_age_timer);
	del_timer(&p->forward_delay_timer);
	del_timer(&p->hold_timer);

	br_configuration_update(br);

	br_port_state_selection(br);

	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

/* called under bridge lock */
static void br_stp_change_bridge_id(struct net_bridge *br, 
				    const unsigned char *addr)
{
	unsigned char oldaddr[6];
	struct net_bridge_port *p;
	int wasroot;

	wasroot = br_is_root_bridge(br);

	memcpy(oldaddr, br->bridge_id.addr, ETH_ALEN);
	memcpy(br->bridge_id.addr, addr, ETH_ALEN);
	memcpy(br->dev->dev_addr, addr, ETH_ALEN);

	list_for_each_entry(p, &br->port_list, list) {
		if (!memcmp(p->designated_bridge.addr, oldaddr, ETH_ALEN))
			memcpy(p->designated_bridge.addr, addr, ETH_ALEN);

		if (!memcmp(p->designated_root.addr, oldaddr, ETH_ALEN))
			memcpy(p->designated_root.addr, addr, ETH_ALEN);

	}

	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

static const unsigned char br_mac_zero[6];

/* called under bridge lock */
void br_stp_recalculate_bridge_id(struct net_bridge *br)
{
	const unsigned char *addr = br_mac_zero;
	struct net_bridge_port *p;

    /*start of Global 解决一线br0mac跳变问题 by c00131380 at 2008.10.28*/
    /*WIFI使能去使能，会导致br0 mac地址从eth0 和 wlo之间跳来跳去*/
    /*原因说明*/
    /*broadcom 解决方案中在每次添加桥信息后，会从桥MAC链表中挑选最小的MAC地址*/
    /*作为br0的基mac地址，如果br0的原mac地址大于wl0的mac地址，则在wifi激活后*/
    /*wl0的mac添加到桥的mac链表中，br_stp_change_bridge_id()将wl0的mac作为br0的*/
    /*基mac进行发布，去激活时，wl0的mac从桥的mac链表中删除，eth0的mac又作为最*/
    /*小的mac成为了br0的基mac，所以导致br0 mac跳变的问题*/
    /*解决措施*/
    /*第一次初始化桥的基地址后，以后添加完桥，不再从桥的mac链表中选取最小的mac作为桥的基地址*/
    if (!g_iInitbr0Flag)
    {
    	list_for_each_entry(p, &br->port_list, list) 
        {
    		if (addr == br_mac_zero ||
    		    memcmp(p->dev->dev_addr, addr, ETH_ALEN) < 0)
    		{
    			addr = p->dev->dev_addr;
            }
    	}
    
    	if (memcmp(br->bridge_id.addr, addr, ETH_ALEN))
        {
    		br_stp_change_bridge_id(br, addr);
        }
        g_iInitbr0Flag = 1;//br0基地址已经初始化完成
    }
    /*end of Global 解决一线br0mac跳变问题 by c00131380 at 2008.10.28*/
}

/* called under bridge lock */
void br_stp_set_bridge_priority(struct net_bridge *br, u16 newprio)
{
	struct net_bridge_port *p;
	int wasroot;

	wasroot = br_is_root_bridge(br);

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p)) {
			p->designated_bridge.prio[0] = (newprio >> 8) & 0xFF;
			p->designated_bridge.prio[1] = newprio & 0xFF;
		}

	}

	br->bridge_id.prio[0] = (newprio >> 8) & 0xFF;
	br->bridge_id.prio[1] = newprio & 0xFF;
	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

/* called under bridge lock */
void br_stp_set_port_priority(struct net_bridge_port *p, u8 newprio)
{
	port_id new_port_id = br_make_port_id(newprio, p->port_no);

	if (br_is_designated_port(p))
		p->designated_port = new_port_id;

	p->port_id = new_port_id;
	p->priority = newprio;
	if (!memcmp(&p->br->bridge_id, &p->designated_bridge, 8) &&
	    p->port_id < p->designated_port) {
		br_become_designated_port(p);
		br_port_state_selection(p->br);
	}
}

/* called under bridge lock */
void br_stp_set_path_cost(struct net_bridge_port *p, u32 path_cost)
{
	p->path_cost = path_cost;
	br_configuration_update(p->br);
	br_port_state_selection(p->br);
}

ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id)
{
	return sprintf(buf, "%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x\n",
	       id->prio[0], id->prio[1],
	       id->addr[0], id->addr[1], id->addr[2],
	       id->addr[3], id->addr[4], id->addr[5]);
}
