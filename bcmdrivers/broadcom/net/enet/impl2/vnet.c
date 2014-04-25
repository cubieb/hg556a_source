/*
<:copyright-broadcom 
 
 Copyright (c) 2005 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include "bcmenet.h"
#include <linux/stddef.h>
#include <asm/r4kcache.h>

static struct net_device_stats * vnet_query(struct net_device * dev);
static void __exit vnet_module_cleanup(void);
static int __init vnet_module_init(void);
int __init vnet_probe(void);

extern struct net_device* get_vnet_dev(int port);
extern struct net_device* set_vnet_dev(int port, struct net_device *dev);
static struct net_device* vnet_dev[NUM_OF_VPORTS] = {NULL, NULL, NULL, NULL};

static struct net_device_stats *vnet_query(struct net_device * dev)
{
    return (struct net_device_stats *)netdev_priv(dev);
}

int __init vnet_probe(void)
{
    struct net_device *dev;
    struct sockaddr sockaddr;
    int status;
    int port;

    static int probed = 0;

    if (probed ++ != 0)
        return -ENXIO;

    vnet_dev[0] = get_vnet_dev(0);

    if (vnet_dev[0] == NULL)
        return -ENXIO;

    for (port = 1; port < NUM_OF_VPORTS; port++)
    {
        dev = alloc_etherdev(sizeof(struct net_device_stats));
        memset(netdev_priv(dev), 0, sizeof(struct net_device_stats));

        if (dev == NULL)
            continue;

        dev_alloc_name(dev, dev->name);
        SET_MODULE_OWNER(dev);
        sprintf(dev->name, "%s.%d", vnet_dev[0]->name, port + 1);

        dev->open                   = vnet_dev[0]->open;
        dev->stop                   = vnet_dev[0]->stop;
        dev->hard_start_xmit        = vnet_dev[0]->hard_start_xmit;
        dev->set_mac_address        = vnet_dev[0]->set_mac_address;
        dev->set_multicast_list     = vnet_dev[0]->set_multicast_list;
        dev->hard_header            = vnet_dev[0]->hard_header;
        dev->hard_header_len        = vnet_dev[0]->hard_header_len;
        dev->do_ioctl               = vnet_dev[0]->do_ioctl;
        dev->get_stats              = vnet_query;
        dev->base_addr              = port;
        dev->header_cache_update    = NULL;
        dev->hard_header_cache      = NULL;
        dev->hard_header_parse      = NULL;

        status = register_netdev(dev);

        if (status != 0)
        {
            unregister_netdev(dev);
            free_netdev(dev);
            return status;
        }

        vnet_dev[port] = dev;
        set_vnet_dev(port, dev);

        memmove(dev->dev_addr, vnet_dev[0]->dev_addr, ETH_ALEN);
        memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);
        dev->set_mac_address(dev, &sockaddr);

        printk("%s: MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            dev->name,
            dev->dev_addr[0],
            dev->dev_addr[1],
            dev->dev_addr[2],
            dev->dev_addr[3],
            dev->dev_addr[4],
            dev->dev_addr[5]
            );
    }

    return 0;
}

static int __init vnet_module_init(void)
{
    return vnet_probe();
}

static void __exit vnet_module_cleanup(void)
{
    int port;

    synchronize_net();

    for (port = 1; port < NUM_OF_VPORTS; port++)
    {
        set_vnet_dev(port, NULL);

        if (vnet_dev[port] != NULL)
        {
            unregister_netdev(vnet_dev[port]);
            free_netdev(vnet_dev[port]);
        }
    }
}

module_init(vnet_module_init);
module_exit(vnet_module_cleanup);
MODULE_LICENSE("Proprietary");

/* End of file */
