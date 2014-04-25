/*
 *	Userspace interface
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_if.c,v 1.2.10.1 2009/11/26 03:43:06 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <net/sock.h>

#include "br_private.h"

/*start of IGMP Snooping 功能支持 by l129990,2008,9,8*/	
#include "br_igmp_snooping.h"
/*end of IGMP Snooping 功能支持 by l129990,2008,9,8*/

/*start of 增加组播mac控制功能 by s53329  at  20070802*/
struct mc_check_list g_mcChecklist = {.inited = 0};
/*end  of 增加组播mac控制功能 by s53329  at  20070802*/

/*start of删除原有的snooping全局变量初始化 by l129990,2008,9,9*/	
//#if defined(CONFIG_MIPS_BRCM)
//int snooping = 0;
//#endif
/*end of删除原有的snooping全局变量初始化 by l129990,2008,9,9*/	
/*
 * Determine initial path cost based on speed.
 * using recommendations from 802.1d standard
 *
 * Need to simulate user ioctl because not all device's that support
 * ethtool, use ethtool_ops.  Also, since driver might sleep need to
 * not be holding any locks.
 */
 
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
int dhcprelayCtl = 0;
/*end of support to the dhcp relay  function by l129990,2009,11,12*/

static int br_initial_port_cost(struct net_device *dev)
{

	struct ethtool_cmd ecmd = { ETHTOOL_GSET };
	struct ifreq ifr;
	mm_segment_t old_fs;
	int err;

	strncpy(ifr.ifr_name, dev->name, IFNAMSIZ);
	ifr.ifr_data = (void __user *) &ecmd;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	err = dev_ethtool(&ifr);
	set_fs(old_fs);
	
	if (!err) {
		switch(ecmd.speed) {
		case SPEED_100:
			return 19;
		case SPEED_1000:
			return 4;
		case SPEED_10000:
			return 2;
		case SPEED_10:
			return 100;
		default:
			pr_info("bridge: can't decode speed from %s: %d\n",
				dev->name, ecmd.speed);
			return 100;
		}
	}

	/* Old silly heuristics based on name */
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;	/* assume old 10Mbps */
}

static void destroy_nbp(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;

	dev->br_port = NULL;
	p->br = NULL;
	p->dev = NULL;
	dev_put(dev);

	br_sysfs_freeif(p);
}

static void destroy_nbp_rcu(struct rcu_head *head)
{
	struct net_bridge_port *p =
			container_of(head, struct net_bridge_port, rcu);
	destroy_nbp(p);
}

/* called with RTNL */
static void del_nbp(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;
	struct net_device *dev = p->dev;

	dev_set_promiscuity(dev, -1);

	spin_lock_bh(&br->lock);
	br_stp_disable_port(p);
	spin_unlock_bh(&br->lock);

	br_fdb_delete_by_port(br, p);

	list_del_rcu(&p->list);

	del_timer_sync(&p->message_age_timer);
	del_timer_sync(&p->forward_delay_timer);
	del_timer_sync(&p->hold_timer);
	
	call_rcu(&p->rcu, destroy_nbp_rcu);
}

/* called with RTNL */
static void del_br(struct net_bridge *br)
{
	struct net_bridge_port *p, *n;
    /*start vlan bridge , s60000658, 20060627*/
    struct vlan_8021q *vp, *vn;
	int i;

	for (i = 0; i < BR_VLAN_8021Q_MAX; i++)
    {
        list_for_each_entry_safe(vp, vn, &br->vlan_table[i], list) {
    		if(atomic_read(&(vp->__refcnt)))
            {
                atomic_set(&(vp->dirty), 1);
            }
            else
            {
                spin_lock_bh(&br->vlock);
                list_del_rcu(&vp->list);
                kfree((void*)vp);
                spin_unlock_bh(&br->vlock);
            }
    	}
    }
    /*end vlan bridge , s60000658, 20060627*/
    
  /*start of IGMP Snooping 功能支持 by l129990,2008,9,6*/	
	br_igmp_snooping_clear();
  /*end of IGMP Snooping 功能支持 by l129990,2008,9,6*/
    
	list_for_each_entry_safe(p, n, &br->port_list, list) {
		br_sysfs_removeif(p);
		del_nbp(p);
	}

/*start of 删除清空桥snooping的mc_list列表函数 by l129990 2008,9,6*/
    //br_mc_fdb_cleanup(br);
/*end of 删除清空桥snooping的mc_list列表函数 by l129990 2008,9,6*/
	del_timer_sync(&br->gc_timer);

	br_sysfs_delbr(br->dev);
 	unregister_netdevice(br->dev);
}

static struct net_device *new_bridge_dev(const char *name)
{
	struct net_bridge *br;
	struct net_device *dev;
    int i; /*vlan bridge , s60000658, 20060627*/

	dev = alloc_netdev(sizeof(struct net_bridge), name,
			   br_dev_setup);
	
	if (!dev)
		return NULL;

	br = netdev_priv(dev);
	br->dev = dev;

	br->lock = SPIN_LOCK_UNLOCKED;
	INIT_LIST_HEAD(&br->port_list);
	br->hash_lock = SPIN_LOCK_UNLOCKED;

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;
	memset(br->bridge_id.addr, 0, ETH_ALEN);

#if defined(CONFIG_MIPS_BRCM)
	br->lock = SPIN_LOCK_UNLOCKED;
 /*start of IGMP Snooping 功能支持 by l129990,2008,9,6*/	
	//INIT_LIST_HEAD(&br->mc_list);
 /*end of IGMP Snooping 功能支持 by l129990,2008,9,6*/	
	br->hash_lock = SPIN_LOCK_UNLOCKED;
#endif
	br->stp_enabled = 0;
	br->designated_root = br->bridge_id;
	br->root_path_cost = 0;
	br->root_port = 0;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
	br->topology_change = 0;
	br->topology_change_detected = 0;
	br->ageing_time = 300 * HZ;
	INIT_LIST_HEAD(&br->age_list);

	br_stp_timer_init(br);

    /*start vlan bridge , s60000658, 20060627*/
	for(i = 0; i < BR_VLAN_8021Q_MAX; i++)
    {
        INIT_LIST_HEAD(&br->vlan_table[i]);
    }
	br->vlan_enable  = 0;
	br->mngr_vid = 0;
    /*end vlan bridge , s60000658, 20060627*/
	

	return dev;
}

/* find an available port number */
static int find_portno(struct net_bridge *br)
{
	int index;
	struct net_bridge_port *p;
	unsigned long *inuse;

	inuse = kmalloc(BITS_TO_LONGS(BR_MAX_PORTS)*sizeof(unsigned long),
			GFP_KERNEL);
	if (!inuse)
		return -ENOMEM;

	memset(inuse, 0, BITS_TO_LONGS(BR_MAX_PORTS)*sizeof(unsigned long));
	set_bit(0, inuse);	/* zero is reserved */
	list_for_each_entry(p, &br->port_list, list) {
		set_bit(p->port_no, inuse);
	}
	index = find_first_zero_bit(inuse, BR_MAX_PORTS);
	kfree(inuse);

	return (index >= BR_MAX_PORTS) ? -EXFULL : index;
}

/* called with RTNL */
static struct net_bridge_port *new_nbp(struct net_bridge *br, 
				       struct net_device *dev,
				       unsigned long cost)
{
	int index;
	struct net_bridge_port *p;
	
	index = find_portno(br);
	if (index < 0)
		return ERR_PTR(index);

	p = kmalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL)
		return ERR_PTR(-ENOMEM);

	memset(p, 0, sizeof(*p));
	p->br = br;
	dev_hold(dev);
	p->dev = dev;
	p->path_cost = cost;
 	p->priority = 0x8000 >> BR_PORT_BITS;
	dev->br_port = p;
	p->port_no = index;
	br_init_port(p);
	p->state = BR_STATE_DISABLED;
	kobject_init(&p->kobj);
	/*start vlan bridge, s60000658, 20060627*/
	VLAN_SET_VID(p->tci,1);
	p->mode = BR_PORT_MODE_NORMAL;
	/*end vlan bridge, s60000658, 20060627*/

	return p;
}

int br_add_bridge(const char *name)
{
	struct net_device *dev;
	int ret;

	dev = new_bridge_dev(name);
	if (!dev) 
		return -ENOMEM;

	rtnl_lock();
	if (strchr(dev->name, '%')) {
		ret = dev_alloc_name(dev, dev->name);
		if (ret < 0)
			goto err1;
	}

	ret = register_netdevice(dev);
	if (ret)
		goto err2;

	/* network device kobject is not setup until
	 * after rtnl_unlock does it's hotplug magic.
	 * so hold reference to avoid race.
	 */
	dev_hold(dev);
	rtnl_unlock();

	ret = br_sysfs_addbr(dev);
	dev_put(dev);

	if (ret) 
		unregister_netdev(dev);
 out:
	return ret;

 err2:
	free_netdev(dev);
 err1:
	rtnl_unlock();
	goto out;
}

int br_del_bridge(const char *name)
{
	struct net_device *dev;
	int ret = 0;

	rtnl_lock();
	dev = __dev_get_by_name(name);
	if (dev == NULL) 
		ret =  -ENXIO; 	/* Could not find device */

	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		/* Attempt to delete non bridge device! */
		ret = -EPERM;
	}

	else if (dev->flags & IFF_UP) {
		/* Not shutdown yet. */
		ret = -EBUSY;
	} 

	else 
		del_br(netdev_priv(dev));

	rtnl_unlock();
	return ret;
}

/*start of support to the dhcp relay  function by l129990,2009,11,12*/
void br_set_dhcprelay(int relayFlag)
{
     dhcprelayCtl = relayFlag;
}
/*end of support to the dhcp relay  function by l129990,2009,11,12*/

/* Mtu of the bridge pseudo-device 1500 or the minimum of the ports */
int br_min_mtu(const struct net_bridge *br)
{
	const struct net_bridge_port *p;
	int mtu = 0;

	ASSERT_RTNL();

	if (list_empty(&br->port_list))
		mtu = 1500;
	else {
		list_for_each_entry(p, &br->port_list, list) {
			if (!mtu  || p->dev->mtu < mtu)
				mtu = p->dev->mtu;
		}
	}
	return mtu;
}

/* called with RTNL */
int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	int err = 0;

	if (dev->flags & IFF_LOOPBACK || dev->type != ARPHRD_ETHER)
		return -EINVAL;

	if (dev->hard_start_xmit == br_dev_xmit)
		return -ELOOP;

	if (dev->br_port != NULL)
		return -EBUSY;

	if (IS_ERR(p = new_nbp(br, dev, br_initial_port_cost(dev))))
		return PTR_ERR(p);

 	if ((err = br_fdb_insert(br, p, dev->dev_addr, 1)))
		destroy_nbp(p);
 
	else if ((err = br_sysfs_addif(p)))
		del_nbp(p);
	else {
		dev_set_promiscuity(dev, 1);

		list_add_rcu(&p->list, &br->port_list);

		spin_lock_bh(&br->lock);
		br_stp_recalculate_bridge_id(br);
		if ((br->dev->flags & IFF_UP) 
		    && (dev->flags & IFF_UP) && netif_carrier_ok(dev))
			br_stp_enable_port(p);
		spin_unlock_bh(&br->lock);

		dev_set_mtu(br->dev, br_min_mtu(br));
	}

	return err;
}

/* called with RTNL */
int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p = dev->br_port;
	
	if (!p || p->br != br) 
		return -EINVAL;

	br_sysfs_removeif(p);
	del_nbp(p);

	spin_lock_bh(&br->lock);
	br_stp_recalculate_bridge_id(br);
	spin_unlock_bh(&br->lock);

	return 0;
}

/*start vlan bridge, s60000658, 20060627*/
/*如果get成功, 使用完毕后一定要调用br_release_vlan*/
__inline__ struct vlan_8021q* br_get_vlan(struct net_bridge *br, unsigned short vid)
{
    struct vlan_8021q *pvlan;
    
    list_for_each_entry_rcu(pvlan, &br->vlan_table[VLAN_HASH_KEY(vid)], list)
    {
        if(pvlan->vid == vid)
        {
            atomic_inc(&(pvlan->__refcnt));
            return pvlan;
        }
    }

    return NULL;
}

__inline__ void br_release_vlan(struct vlan_8021q *pvlan)
{    
    atomic_dec(&(pvlan->__refcnt));
    if(atomic_read( &(pvlan->__refcnt) ) <= 0 && atomic_read( &(pvlan->dirty) ))
    {
        spin_lock_bh(&pvlan->lock);
        kfree((void*)pvlan);
        spin_unlock_bh(&pvlan->lock);
    }
    
}


int br_add_vlan(struct net_bridge *br, unsigned short vid)
{
    struct vlan_8021q *pvlan;
    
    if(NULL != (pvlan = br_get_vlan(br, vid)))
    {
        br_release_vlan(pvlan);
        return -EINVAL;
    }

    pvlan = (struct vlan_8021q *)kmalloc(sizeof(*pvlan), GFP_KERNEL);
    
    if(NULL != pvlan)
    {
        if(DEBUG_VBR)
            printk("BRVLAN(kenl): add vlan %d, br %s, file %s, line %d\n",
               vid, br->dev->name, __FILE__, __LINE__);
        
        memset(pvlan, 0, sizeof(struct vlan_8021q));
        pvlan->vid = vid;
        spin_lock_bh(&br->vlock);
        list_add_rcu(&pvlan->list, &br->vlan_table[VLAN_HASH_KEY(vid)]);
        spin_unlock_bh(&br->vlock);
        return 0;
    }
    
    return -EINVAL;
}
int br_del_vlan(struct net_bridge *br, unsigned short vid)
{
    struct vlan_8021q *pvlan = NULL;
    
    list_for_each_entry_rcu(pvlan, &br->vlan_table[VLAN_HASH_KEY(vid)], list)
    {
        if(pvlan->vid == vid)
        {   
            spin_lock_bh(&br->vlock);
            list_del_rcu(&pvlan->list);
            INIT_LIST_HEAD(&pvlan->list);
            spin_unlock_bh(&br->vlock);
            if(atomic_read(&(pvlan->__refcnt)) > 0)
            {
                if(DEBUG_VBR)
                    printk("BRVLAN(kenl): del vlan %d to dirty, br %s, file %s, line %d\n",
                           vid, br->dev->name, __FILE__, __LINE__);
                atomic_set(&(pvlan->dirty), 1);
            }
            else
            {
                if(DEBUG_VBR)
                    printk("BRVLAN(kenl): del vlan %d drastically, br %s, file %s, line %d\n",
                           vid, br->dev->name, __FILE__, __LINE__);
                spin_lock_bh(&br->vlock);                
                kfree((void*)pvlan);
                spin_unlock_bh(&br->vlock);
            }

            break;
        }
    }
    return 0;
}
int br_add_vlan_ports(struct net_bridge *br, unsigned short vid, unsigned short pts_mask)
{
    struct vlan_8021q *pvlan = NULL;

    if(NULL == (pvlan = br_get_vlan(br, vid)) )
    {
        return -EINVAL;
    }
    
    if(DEBUG_VBR)
        printk("BRVLAN(kenl): add vlan %d ports 0x%x, br %s, file %s, line %d\n",
               vid, pts_mask, br->dev->name, __FILE__, __LINE__);
    
    pvlan->ports |= pts_mask;
    br_release_vlan(pvlan);   

    return 0;
}
int br_del_vlan_ports(struct net_bridge *br, unsigned short vid, unsigned short pts_mask)
{
    struct vlan_8021q *pvlan = NULL;

    if(NULL == (pvlan = br_get_vlan(br, vid)) )
    {
        return -EINVAL;
    }

    if(DEBUG_VBR)
        printk("BRVLAN(kenl): del vlan %d ports 0x%x, br %s, file %s, line %d\n",
               vid, pts_mask, br->dev->name, __FILE__, __LINE__);
    
    pvlan->ports &= ~pts_mask;
    br_release_vlan(pvlan);

    return 0;
}
int br_set_pmd(int ifindex, unsigned short mode)
{
    struct net_device *dev;
	struct net_bridge_port *p;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if (mode >= BR_PORT_MODE_UNKNOWN)
        return -EINVAL;	

    dev = dev_get_by_index(ifindex);
	if (dev == NULL)
		return -EINVAL;
	
	p = dev->br_port;
	
	if (!p) 
    {
        dev_put(dev);
		return -EINVAL;
    }
	
    if(DEBUG_VBR)
        printk("BRVLAN(kenl): set port %s mode %d,  file %s, line %d\n",
               p->dev->name, mode,  __FILE__, __LINE__);
    
    p->mode = mode;
    dev_put(dev);
    return 0;
}
int br_set_pvid(int ifindex, unsigned short pvid)
{
    struct net_device *dev;
	struct net_bridge_port *p;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if(!ISVALID_PVID(pvid))
        return -EINVAL;

    dev = dev_get_by_index(ifindex);
	if (dev == NULL)
		return -EINVAL;

    p = dev->br_port;
	
	if (!p ) 
    {
        dev_put(dev);
		return -EINVAL;
    }
	
	VLAN_SET_VID(p->tci, pvid);
	
    if(DEBUG_VBR)
        printk("BRVLAN(kenl): set port %s pvid %d,p->tci 0x%x,  file %s, line %d\n",
               p->dev->name, pvid, p->tci,  __FILE__, __LINE__);
    
    dev_put(dev);
    return 0;
}
int br_set_prio(int ifindex, unsigned short prio, unsigned char enable)
{
    struct net_device *dev;
	struct net_bridge_port *p;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if(!ISVALID_PRIO(prio))
        return -EINVAL;

    dev = dev_get_by_index(ifindex);
	if (dev == NULL)
		return -EINVAL;

    p = dev->br_port;
	
	if (!p ) 
    {
        dev_put(dev);
		return -EINVAL;
    }

	VLAN_SET_PRIO(p->tci, prio);
	p->prio_enable = enable;
	
    if(DEBUG_VBR)
        printk("BRVLAN(kenl): set port %s prio %d, p->tci 0x%x,  file %s, line %d\n",
               p->dev->name, prio, p->tci, __FILE__, __LINE__);   
	
    dev_put(dev);
    return 0;
}
int br_set_vlanflag(struct net_bridge *br, unsigned char vlanflag)
{
    if (!br) 
		return -EINVAL;
    
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if(DEBUG_VBR)
            printk("BRVLAN(kenl): set vlan %s  in %s,  file %s, line %d\n",
                   (vlanflag==1)?"enable":"disable", br->dev->name, __FILE__, __LINE__);
    
    br->vlan_enable = vlanflag;
    
	return 0;
}
int br_set_mngr_vid(struct net_bridge *br, unsigned short vid)
{
    if (!br) 
		return -EINVAL;
    
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    if(DEBUG_VBR)
            printk("BRVLAN(kenl): set management vlan id %d in %s,  file %s, line %d\n",
                   vid, br->dev->name, __FILE__, __LINE__);
    
    br->mngr_vid = vid;
    
	return 0;
}
int br_set_macln(struct net_bridge *br, unsigned short vid, unsigned char enable)
{
    struct vlan_8021q *pvlan = NULL;
    
    if (!br) 
		return -EINVAL;
    
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

    pvlan = br_get_vlan(br, vid);

    if(NULL == pvlan)
        return -EINVAL;

    if(DEBUG_VBR)
            printk("BRVLAN(kenl): set vlan %d macln %d, in %s,  file %s, line %d\n",
                   vid, enable, br->dev->name, __FILE__, __LINE__);  
    
    pvlan->macln_enable = enable;
    br_release_vlan(pvlan);
    
	return 0;
}
/*end vlan bridge, s60000658, 20060627*/
void __exit br_cleanup_bridges(void)
{
	struct net_device *dev, *nxt;

	rtnl_lock();
	for (dev = dev_base; dev; dev = nxt) {
		nxt = dev->next;
		if (dev->priv_flags & IFF_EBRIDGE)
			del_br(dev->priv);
	}
	rtnl_unlock();

}
/*start of 增加组播mac控制功能 by s53329  at  20070802*/
void br_init_mccheck(void)
{
	if(g_mcChecklist.inited)
		return;

	INIT_LIST_HEAD(&g_mcChecklist.acc_list);
	spin_lock_init(&g_mcChecklist.lock);
	g_mcChecklist.inited = 1;
}

int br_add_mccheck(unsigned char* src_mac)
{
    struct mc_check_node *pnode = NULL;
    br_init_mccheck();
    spin_lock_bh(&g_mcChecklist.lock);
    list_for_each_entry_rcu(pnode, &g_mcChecklist.acc_list, list)
    {
        if(pnode!=NULL)
        {
            if(memcmp(src_mac, pnode->src_mac, 6) == 0)
            {
                spin_unlock_bh(&g_mcChecklist.lock);
                return 1;
            }
        }
    }

    pnode = NULL;
    if(NULL == (pnode = (struct mc_check_node*)kmalloc(sizeof(struct mc_check_node), GFP_KERNEL)))
    {
        spin_unlock_bh(&g_mcChecklist.lock);
        return 0;
    }
    else
    {
        memset(pnode, 0, sizeof(struct mc_check_node));
        memcpy(pnode->src_mac, src_mac, 6);
    }
    list_add_rcu(&pnode->list, &g_mcChecklist.acc_list);
    spin_unlock_bh(&g_mcChecklist.lock);
    return 1;
}

int br_del_mccheck(unsigned char *src_mac)
{
    struct mc_check_node *pnode = NULL;
    int node_existed = 0;
    int ret = 0;
    br_init_mccheck();
    spin_lock_bh(&g_mcChecklist.lock);
    list_for_each_entry_rcu(pnode, &g_mcChecklist.acc_list, list)
    {
        if(pnode!=NULL)
        {
            if(memcmp(src_mac, pnode->src_mac, 6) == 0)
            {
                node_existed = 1;
                break;
            }
        }
    }
    if(node_existed)
    {
        list_del_rcu(&pnode->list);
        kfree(pnode);
        ret = 1;
    }
    spin_unlock_bh(&g_mcChecklist.lock);
    return ret;
}
/*end  of 增加组播mac控制功能 by s53329  at  20070802*/

