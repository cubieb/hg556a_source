/*
 *	Device event handling
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_notify.c,v 1.1.2.1 2009/03/17 11:43:24 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>

#include "br_private.h"

/* Notify DHCPD to update its lease table. <tanyin 2009.3.13> */
#define UDHCPD_PID_FILE "/var/run/udhcpd.pid"

/*
 * Notify module that care about this with specified signal.
 * It is easy but remember that reading file in kernel is not recommended.
 * <tanyin 2009.3.13>
 */
void notify_on_enet_down(char *pidfile, int sig)
{
    int fd = -1;
    int pid = 0;
    mm_segment_t old_fs;
    char data[4];

    old_fs = get_fs();
    set_fs(KERNEL_DS); /* tell it is in kernel data segment */
   	fd = sys_open(pidfile, O_RDONLY, 0);
	if (fd > 0)	{
		sys_lseek(fd, 0, 0);
		sys_read(fd, data, sizeof(data));
        pid = simple_strtol(data, NULL, 10);
		pr_info("Send SIG(%d) to pid %d on eth down\n",sig,pid);
		sys_kill(pid, sig);
		sys_close(fd);
	} else {
	    pr_info("open %s failed, result %d\n",pidfile,fd);
	}
    set_fs(old_fs);
}

static int br_device_event(struct notifier_block *unused, unsigned long event, void *ptr);

struct notifier_block br_device_notifier = {
	.notifier_call = br_device_event
};

/*
 * Handle changes in state of network devices enslaved to a bridge.
 * 
 * Note: don't care about up/down if bridge itself is down, because
 *     port state is checked when bridge is brought up.
 */
static int br_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;
	struct net_bridge_port *p = dev->br_port;
	struct net_bridge *br;

	/* not a port of a bridge */
	if (p == NULL)
		return NOTIFY_DONE;

	br = p->br;

	spin_lock_bh(&br->lock);
	switch (event) {
	case NETDEV_CHANGEMTU:
		dev_set_mtu(br->dev, br_min_mtu(br));
		break;

	case NETDEV_CHANGEADDR:
		br_fdb_changeaddr(p, dev->dev_addr);
		br_stp_recalculate_bridge_id(br);
		break;

	case NETDEV_CHANGE:	/* device is up but carrier changed */
		if (!(br->dev->flags & IFF_UP))
			break;

		if (netif_carrier_ok(dev)) {
			if (p->state == BR_STATE_DISABLED)
				br_stp_enable_port(p);
		} else {
			if (p->state != BR_STATE_DISABLED)
				br_stp_disable_port(p);
		}
		break;

	case NETDEV_DOWN:
		if (br->dev->flags & IFF_UP) {
			br_stp_disable_port(p);

    	    /* Notify DHCPD to update its lease table. <tanyin 2009.3.13> */
            notify_on_enet_down(UDHCPD_PID_FILE, SIGHUP);
    	}
		break;

	case NETDEV_UP:
		if (netif_carrier_ok(dev) && (br->dev->flags & IFF_UP)) 
			br_stp_enable_port(p);
		break;

	case NETDEV_UNREGISTER:
		br_del_if(br, dev);
		break;
	} 
	spin_unlock_bh(&br->lock);

	return NOTIFY_DONE;
}
