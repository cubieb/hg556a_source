/*
 *	Ioctl handler
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_ioctl.c,v 1.2.10.1 2009/11/26 03:43:06 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <linux/times.h>
#include <asm/uaccess.h>
#include "br_private.h"
/*start of IGMP Snooping 功能支持 by l129990,2008,9,9*/	
#include "br_igmp_snooping.h"
/*end of IGMP Snooping 功能支持 by l129990,2008,9,9*/

#if defined(CONFIG_MIPS_BRCM)
#define BRCTL_SET_PORT_SNOOPING 21
#define BRCTL_CLEAR_PORT_SNOOPING 22
#define BRCTL_ENABLE_SNOOPING 23
#define BRCTL_SHOW_SNOOPING 24
#define SIOCBRSHOWSNOOPING 24
#endif
/*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
#define    DHCP_OPTION    264
char dhcpoption[DHCP_OPTION];   
/*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/
/* called with RTNL */
static int get_bridge_ifindices(int *indices, int num)
{
	struct net_device *dev;
	int i = 0;

	for (dev = dev_base; dev && i < num; dev = dev->next) {
		if (dev->priv_flags & IFF_EBRIDGE) 
			indices[i++] = dev->ifindex;
	}

	return i;
}

/* called with RTNL */
static void get_port_ifindices(struct net_bridge *br, int *ifindices, int num)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->port_no < num)
			ifindices[p->port_no] = p->dev->ifindex;
	}
}

#if defined(CONFIG_MIPS_BRCM)
int br_set_port_snooping(struct net_bridge_port *p,  void __user * userbuf)
{
    unsigned char tmp[26];
    
    if (copy_from_user(tmp, userbuf, sizeof(tmp)))
		return -EFAULT;
  /*start of 移植ATP的设置端口snooping函数by l129990 2008,9,8*/  
  //br_mc_fdb_add(p->br, p, tmp, tmp+6);
   br_igmp_snooping_add(p,tmp,tmp+6);
   /*end of 移植ATP的设置端口snooping函数by l129990 2008,9,8*/  
    return 0;
}

int br_clear_port_snooping(struct net_bridge_port *p,  void __user * userbuf)
{
    unsigned char tmp[26];
    unsigned char all[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    if (copy_from_user(tmp, userbuf, sizeof(tmp)))
		return -EFAULT;
/*start of 删除Brcm原有的路由和桥模式的snooping控制开关by l129990 2008,9,9*/
    //p->br->proxy = 1;
/*end of 删除Brcm原有的路由和桥模式的snooping控制开关by l129990 2008,9,9*/
    if (!memcmp(tmp+6, all, 6))
/*start of 新增设置端口snooping函数by l129990 2008,9,9*/     
	//br_mc_fdb_remove_grp(p->br, p, tmp);
	br_igmp_snooping_del_grp(p, tmp);
/*end of 新增设置端口snooping函数by l129990 2008,9,9*/ 
    else
/*start of 移植ATP的设置端口snooping函数by l129990 2008,9,9*/  
    //br_mc_fdb_remove(p->br, p, tmp, tmp+6);
    br_igmp_snooping_del(p, tmp, tmp+6);
/*end of 移植ATP的设置端口snooping函数by l129990 2008,9,9*/ 
    return 1;
}
#endif

/*
 * Format up to a page worth of forwarding table entries
 * userbuf -- where to copy result
 * maxnum  -- maximum number of entries desired
 *            (limited to a page for sanity)
 * offset  -- number of records to skip
 */
static int get_fdb_entries(struct net_bridge *br, void __user *userbuf, 
			   unsigned long maxnum, unsigned long offset)
{
	int num;
	void *buf;
	size_t size = maxnum * sizeof(struct __fdb_entry);

	if (size > PAGE_SIZE) {
		size = PAGE_SIZE;
		maxnum = PAGE_SIZE/sizeof(struct __fdb_entry);
	}

	buf = kmalloc(size, GFP_USER);
	if (!buf)
		return -ENOMEM;
	
	num = br_fdb_fillbuf(br, buf, maxnum, offset);
	if (num > 0) {
		if (copy_to_user(userbuf, buf, num*sizeof(struct __fdb_entry)))
			num = -EFAULT;
	}
	kfree(buf);

	return num;
}

static int add_del_if(struct net_bridge *br, int ifindex, int isadd)
{
	struct net_device *dev;
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	dev = dev_get_by_index(ifindex);
	if (dev == NULL)
		return -EINVAL;
	
	if (isadd)
		ret = br_add_if(br, dev);
	else
		ret = br_del_if(br, dev);

	dev_put(dev);
	return ret;
}

/*start vlan bridge, s60000658, 20060627*/
static int add_del_vlan(struct net_bridge *br, unsigned short vid, int isadd)
{
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	
	if (isadd)
		ret = br_add_vlan(br, vid);
	else
		ret = br_del_vlan(br, vid);

	return ret;
}
static int add_del_vlan_ports(struct net_bridge *br,  unsigned short vid,
                                      unsigned short pts_mask, int isadd)
{
	int ret;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	
	if (isadd)
		ret = br_add_vlan_ports(br, vid, pts_mask);
	else
		ret = br_del_vlan_ports(br, vid, pts_mask);

	return ret;
}
/*end vlan bridge, s60000658, 20060627*/

/*
 * Legacy ioctl's through SIOCDEVPRIVATE
 * This interface is deprecated because it was too difficult to
 * to do the translation for 32/64bit ioctl compatability.
 */
static int old_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_bridge *br = netdev_priv(dev);
	unsigned long args[4];
	
	if (copy_from_user(args, rq->ifr_data, sizeof(args)))
		return -EFAULT;

	switch (args[0]) {
	case BRCTL_ADD_IF:
	case BRCTL_DEL_IF:
		return add_del_if(br, args[1], args[0] == BRCTL_ADD_IF);
	/*start vlan bridge, s60000658, 20060627*/
	case BRCTL_ADD_VLAN :
    case BRCTL_DEL_VLAN :
        return add_del_vlan(br, (unsigned short)args[1], args[0] == BRCTL_ADD_VLAN);
    case BRCTL_ADD_VLAN_PORTS :
    case BRCTL_DEL_VLAN_PORTS :
        return add_del_vlan_ports(br, (unsigned short)args[1], (unsigned short)args[2], 
                               args[0] == BRCTL_ADD_VLAN_PORTS);
    case BRCTL_SET_PORT_MODE:
        return br_set_pmd(args[1], (unsigned short)args[2]);
    case BRCTL_SET_PVID:
        return br_set_pvid(args[1], (unsigned short)args[2]);
    case BRCTL_SET_PRIO:
        return br_set_prio(args[1], (unsigned short)args[2], (unsigned char)args[3]);
    case BRCTL_SET_VLANFLAG:
        return br_set_vlanflag(br, (unsigned char)args[1]);
    case BRCTL_SET_MNGRVID:        
        return br_set_mngr_vid(br, (unsigned short)args[1]);
    case BRCTL_SET_MACLN:        
        return br_set_macln(br, (unsigned short)args[1], (unsigned char)args[2]);
    case BRCTL_GET_VLAN:
    {
        struct vlan_8021q *pvlan;
        struct net_bridge_port *p;
        
        pvlan = br_get_vlan(br, (unsigned short)args[1]);
        
        if(NULL == pvlan)
        {
            int i, isnull = 1;
            
            /*如果找不到指定的vlan,就将所有的vlan显示出来*/
            printk("\n=========bridge %s===========\n", br->dev->name);
            printk("vlan_enable      : %d\n", br->vlan_enable);
            printk("management vlanid: %d\n", br->mngr_vid);
            printk("=========\n");
            for (i = 0; i < BR_VLAN_8021Q_MAX; i++)
            {
                list_for_each_entry_rcu(pvlan, &br->vlan_table[i], list)
                {
                    printk("vlan %d, mac_learn %d: ", pvlan->vid, pvlan->macln_enable);
                    list_for_each_entry_rcu(p, &br->port_list, list)
                    {
                        if(pvlan->ports & ((unsigned short)1 << p->port_no))
                        {
                            isnull = 0;
                            printk("%s ", p->dev->name);                       
                        }                        
                    }
                    if(isnull)
                    {
                        printk("(null)\n");
                    }
                    else
                    {
                        printk("\n");
                    }
                    list_for_each_entry_rcu(p, &br->port_list, list)
                    {
                        if(pvlan->ports & ((unsigned short)1 << p->port_no))
                        {
                            printk("   %s :\n", p->dev->name); 
                            printk("   port_no    : %d\n", p->port_no);                            
                            printk("   mode       : %d\n", p->mode);
                            printk("   pvid       : %d\n", VLAN_GET_VID(p->tci));
                            printk("   prio       : %d\n", VLAN_GET_PRIO(p->tci));
                            printk("   prio_eanble: %d\n", p->prio_enable);
                            printk("   *********\n");
                        }                        
                    }
                }
            }
            return -ENXIO;
        }
        
		if (copy_to_user((void __user *)args[2], &(pvlan->ports), sizeof(pvlan->ports)))
	    {
	        br_release_vlan( pvlan );
			return -EFAULT;
	    }
        
        br_release_vlan(pvlan);
        
        return 0;
    }        
	/*end vlan bridge, s60000658, 20060627*/
      /*start of 增加组播mac控制功能 by s53329  at  20070802*/
      case BRCTL_ADD_MCCHECK:
	{
		unsigned char src_mac[6];
		
		if(!copy_from_user(src_mac, (void __user *)args[1], 6))
		{
			br_add_mccheck(src_mac);
		}
		
		return 0;		
	}
	case BRCTL_DEL_MCCHECK:
	{
		unsigned char src_mac[6];
	
		if(!copy_from_user(src_mac, (void __user *)args[1], 6))
		{
			br_del_mccheck(src_mac);
		}
		
		return 0;
	}
        /*end  of 增加组播mac控制功能 by s53329  at  20070802*/

	case BRCTL_GET_BRIDGE_INFO:
	{
		struct __bridge_info b;

		memset(&b, 0, sizeof(struct __bridge_info));
		rcu_read_lock();
		memcpy(&b.designated_root, &br->designated_root, 8);
		memcpy(&b.bridge_id, &br->bridge_id, 8);
		b.root_path_cost = br->root_path_cost;
		b.max_age = jiffies_to_clock_t(br->max_age);
		b.hello_time = jiffies_to_clock_t(br->hello_time);
		b.forward_delay = br->forward_delay;
		b.bridge_max_age = br->bridge_max_age;
		b.bridge_hello_time = br->bridge_hello_time;
		b.bridge_forward_delay = jiffies_to_clock_t(br->bridge_forward_delay);
		b.topology_change = br->topology_change;
		b.topology_change_detected = br->topology_change_detected;
		b.root_port = br->root_port;
		b.stp_enabled = br->stp_enabled;
		b.ageing_time = jiffies_to_clock_t(br->ageing_time);
		b.hello_timer_value = br_timer_value(&br->hello_timer);
		b.tcn_timer_value = br_timer_value(&br->tcn_timer);
		b.topology_change_timer_value = br_timer_value(&br->topology_change_timer);
		b.gc_timer_value = br_timer_value(&br->gc_timer);
	        rcu_read_unlock();

		if (copy_to_user((void __user *)args[1], &b, sizeof(b)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_GET_PORT_LIST:
	{
		int num, *indices;

		num = args[2];
		if (num < 0)
			return -EINVAL;
		if (num == 0)
			num = 256;
		if (num > BR_MAX_PORTS)
			num = BR_MAX_PORTS;

		indices = kmalloc(num*sizeof(int), GFP_KERNEL);
		if (indices == NULL)
			return -ENOMEM;

		memset(indices, 0, num*sizeof(int));

		get_port_ifindices(br, indices, num);
		if (copy_to_user((void __user *)args[1], indices, num*sizeof(int)))
			num =  -EFAULT;
		kfree(indices);
		return num;
	}

	case BRCTL_SET_BRIDGE_FORWARD_DELAY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_forward_delay = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->forward_delay = br->bridge_forward_delay;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_BRIDGE_HELLO_TIME:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_hello_time = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->hello_time = br->bridge_hello_time;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_BRIDGE_MAX_AGE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br->bridge_max_age = clock_t_to_jiffies(args[1]);
		if (br_is_root_bridge(br))
			br->max_age = br->bridge_max_age;
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_AGEING_TIME:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		br->ageing_time = clock_t_to_jiffies(args[1]);
		return 0;

	case BRCTL_GET_PORT_INFO:
	{
		struct __port_info p;
		struct net_bridge_port *pt;

		rcu_read_lock();
		if ((pt = br_get_port(br, args[2])) == NULL) {
			rcu_read_unlock();
			return -EINVAL;
		}

		memset(&p, 0, sizeof(struct __port_info));
		memcpy(&p.designated_root, &pt->designated_root, 8);
		memcpy(&p.designated_bridge, &pt->designated_bridge, 8);
		p.port_id = pt->port_id;
		p.designated_port = pt->designated_port;
		p.path_cost = pt->path_cost;
		p.designated_cost = pt->designated_cost;
		p.state = pt->state;
		p.top_change_ack = pt->topology_change_ack;
		p.config_pending = pt->config_pending;
		p.message_age_timer_value = br_timer_value(&pt->message_age_timer);
		p.forward_delay_timer_value = br_timer_value(&pt->forward_delay_timer);
		p.hold_timer_value = br_timer_value(&pt->hold_timer);
        /*start vlan bridge, s60000658, 20060627*/
        p.tci = pt->tci;
        p.mode = pt->mode;
		/*end vlan bridge, s60000658, 20060627*/
		rcu_read_unlock();

		if (copy_to_user((void __user *)args[1], &p, sizeof(p)))
			return -EFAULT;

		return 0;
	}

	case BRCTL_SET_BRIDGE_STP_STATE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		br->stp_enabled = args[1]?1:0;
		return 0;

	case BRCTL_SET_BRIDGE_PRIORITY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		br_stp_set_bridge_priority(br, args[1]);
		spin_unlock_bh(&br->lock);
		return 0;

	case BRCTL_SET_PORT_PRIORITY:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (args[2] >= (1<<(16-BR_PORT_BITS)))
			return -ERANGE;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL) 
			ret = -EINVAL;
		else
			br_stp_set_port_priority(p, args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}

#if defined(CONFIG_MIPS_BRCM)
	case BRCTL_SET_PORT_SNOOPING:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL) 
			ret = -EINVAL;
		else
			br_set_port_snooping(p, (void __user *) args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}

	case BRCTL_CLEAR_PORT_SNOOPING:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL) 
			ret = -EINVAL;
		else
			br_clear_port_snooping(p, (void __user *) args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}
#endif

	case BRCTL_SET_PATH_COST:
	{
		struct net_bridge_port *p;
		int ret = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		spin_lock_bh(&br->lock);
		if ((p = br_get_port(br, args[1])) == NULL)
			ret = -EINVAL;
		else
			br_stp_set_path_cost(p, args[2]);
		spin_unlock_bh(&br->lock);
		return ret;
	}

	case BRCTL_GET_FDB_ENTRIES:
		return get_fdb_entries(br, (void __user *)args[1], 
				       args[2], args[3]);
	}

	return -EOPNOTSUPP;
}

static int old_deviceless(void __user *uarg)
{
	unsigned long args[3];

	if (copy_from_user(args, uarg, sizeof(args)))
		return -EFAULT;

	switch (args[0]) {
	case BRCTL_GET_VERSION:
		return BRCTL_VERSION;

	case BRCTL_GET_BRIDGES:
	{
		int *indices;
		int ret = 0;

		indices = kmalloc(args[2]*sizeof(int), GFP_KERNEL);
		if (indices == NULL)
			return -ENOMEM;

		memset(indices, 0, args[2]*sizeof(int));
		args[2] = get_bridge_ifindices(indices, args[2]);

		ret = copy_to_user((void __user *)args[1], indices, args[2]*sizeof(int))
			? -EFAULT : args[2];

		kfree(indices);
		return ret;
	}

	case BRCTL_ADD_BRIDGE:
	case BRCTL_DEL_BRIDGE:
	{
		char buf[IFNAMSIZ];

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, (void __user *)args[1], IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;

		if (args[0] == BRCTL_ADD_BRIDGE)
			return br_add_bridge(buf);

		return br_del_bridge(buf);
	}
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
    case BRCTL_SET_DHCPRELAY:
	{
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
        br_set_dhcprelay((int)args[1]);
		return 0;
	}
/*end of support to the dhcp relay  function by l129990,2009,11,12*/
    
#if defined(CONFIG_MIPS_BRCM)
	case BRCTL_SHOW_SNOOPING:
/*start of修改查看snooping MAC转发表功能 by l129990,2008,9,18*/
#if 0
    {
		char buf[IFNAMSIZ];
		struct net_device *dev;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, (void __user *)args[1], IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;

		dev = __dev_get_by_name(buf);
		if (dev == NULL) 
		    return  -ENXIO; 	/* Could not find device */
        /*start of BRIDGE <3.3.6showsnooping不存在的桥导致系统挂起> porting by s60000658 20060506*/
		if (!(dev->priv_flags & IFF_EBRIDGE)) {
               /* Attempt to delete non bridge device! */
               return -EPERM;
        }
        /*end of BRIDGE <3.3.6showsnooping不存在的桥导致系统挂起> porting by s60000658 20060506*/

        dolist(netdev_priv(dev));
		return 0;
	}
#endif

{
    br_igmp_snooping_show();
	return 0;
}
/*end of修改查看snooping MAC转发表功能 by l129990,2008,9,18*/
	case BRCTL_ENABLE_SNOOPING:
	{
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
/*start of删除原有的通过从用户态读取数值来设置snooping值 by l129990 2008,9,8*/
		//snooping = args[1];
/*end of删除原有的通过从用户态读取数值来设置snooping值 by l129990 2008,9,8*/
/*start of IGMP Snooping 功能支持 by l129990,2008,9,8*/	
        br_igmp_snooping_set_enable((int)args[1]);
/*end of IGMP Snooping 功能支持 by l129990,2008,9,8*/	
		return 0;
	}
#endif
	}

	return -EOPNOTSUPP;
}

int br_ioctl_deviceless_stub(unsigned int cmd, void __user *uarg)
{
	switch (cmd) {
	case SIOCGIFBR:
	case SIOCSIFBR:
		return old_deviceless(uarg);
		
	case SIOCBRADDBR:
	case SIOCBRDELBR:
	{
		char buf[IFNAMSIZ];

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, uarg, IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;
		if (cmd == SIOCBRADDBR)
			return br_add_bridge(buf);

		return br_del_bridge(buf);
	}
       /*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
	case SIOCBRADDDHCPOPTION:
	 {
		if (copy_from_user(dhcpoption, uarg, DHCP_OPTION))
			return -EFAULT;
		  return 0;
	  }
        /*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/

	    
#if defined(CONFIG_MIPS_BRCM)
	case SIOCBRSHOWSNOOPING:
/*start of修改查看snooping MAC转发表功能 by l129990,2008,9,18*/
#if 0
    {
		char buf[IFNAMSIZ];
		struct net_device *dev;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;

		if (copy_from_user(buf, uarg, IFNAMSIZ))
			return -EFAULT;

		buf[IFNAMSIZ-1] = 0;
		
		dev = __dev_get_by_name(buf);
		if (dev == NULL) 
		    return -ENXIO; 	/* Could not find device */

		/*start of BRIDGE <3.3.6showsnooping不存在的桥导致系统挂起> porting by s60000658 20060506*/
		if (!(dev->priv_flags & IFF_EBRIDGE)) {
               /* Attempt to delete non bridge device! */
               return -EPERM;
        }
        /*end of BRIDGE <3.3.6showsnooping不存在的桥导致系统挂起> porting by s60000658 20060506*/

        dolist(netdev_priv(dev));

		return 0;
	}
#endif
    {
        br_igmp_snooping_show();
	    return 0;
    }
/*end of修改查看snooping MAC转发表功能 by l129990,2008,9,18*/
#endif
	}
	return -EOPNOTSUPP;
}

int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_bridge *br = netdev_priv(dev);

	switch(cmd) {
	case SIOCDEVPRIVATE:
		return old_dev_ioctl(dev, rq, cmd);

	case SIOCBRADDIF:
	case SIOCBRDELIF:
		return add_del_if(br, rq->ifr_ifindex, cmd == SIOCBRADDIF);

	}

	pr_debug("Bridge does not support ioctl 0x%x\n", cmd);
	return -EOPNOTSUPP;
}

