/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_private.h,v 1.2.10.1 2009/11/26 03:43:06 l43571 Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>
#include <linux/if_vlan.h>/*vlan bridge, s60000658, 20060627*/

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)
/*start of 增加组播mac控制功能 by s53329  at  20070802*/
struct mc_check_list
{
	unsigned int inited;
	spinlock_t lock;	
	struct list_head acc_list;
};
struct mc_check_node
{
	struct list_head list;
	unsigned char src_mac[6];
};
/*end of 增加组播mac控制功能 by s53329  at  20070802*/

/*start vlan bridge, s60000658, 20060627*/
#ifndef DEBUG_VBR
#define  DEBUG_VBR 0
#define  DEBUG_DUMPSKB 0
    #if(DEBUG_DUMPSKB)
    #define  VLAN_DUMP_SKB(skb, info) ({                               \
        printk("BRVLAN: dump skb len = %d, info %s\n", skb->len, info); \
        int i;                                                            \
        for(i = 0; i < skb->len; i++)                                    \
        {printk("0x%x ", skb->data[i]);}                                 \
         printk("\n");                                                    \
    })
    #else
    #define  VLAN_DUMP_SKB(skb, info) ({;})
    #endif
#undef DEBUG_DUMPSKB
#endif
#define BR_VLAN_8021Q_MAX 16 
#define VLAN_HASH_KEY(vid) ((vid)%BR_VLAN_8021Q_MAX)
#define ISVALID_VID(vid)   (((vid) < 0xFFF) && ((vid) >= 0))
#define ISVALID_PVID(pvid) (((pvid) < 0xFFF) && ((pvid) >= 0))
#define ISVALID_PRIO(prio) (((prio) <= 7) && ((prio) >= 0))
#define ISIN_VLAN(vlan_ports, port) ((vlan_ports) & ((unsigned short)1 << (port)))
#define VLAN_SET_VID(tag, vid) ({ \
    (tag) &= (~((unsigned short)VLAN_VID_MASK)); \
    (tag) |= (unsigned short)((vid) & VLAN_VID_MASK); \
    })
#define VLAN_SET_PRIO(tag, prio)({ \
    (tag) &= (unsigned short)0x1FFF; \
    (tag) |= (unsigned short)((prio) << 13); \
    })
#define VLAN_GET_VID(tag) (((unsigned short)(tag))&VLAN_VID_MASK)
#define VLAN_GET_PRIO(tag) (((unsigned short)(tag)) >> 13)
/*end vlan bridge, s60000658, 20060627*/

#if defined(CONFIG_MIPS_BRCM)
#define TIMER_CHECK_TIMEOUT 10
#define QUERY_TIMEOUT 130
//#define QUERY_TIMEOUT 60
#endif

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

/*start vlan bridge, s60000658, 20060627*/
enum 
{
    BR_PORT_MODE_NORMAL = 0,
    BR_PORT_MODE_ACCESS,
    BR_PORT_MODE_TRUNK,
    BR_PORT_MODE_HYBRID,
    /*more add here*/
    BR_PORT_MODE_UNKNOWN
};

struct vlan_8021q
{
	struct list_head	list;
    spinlock_t			lock;
	atomic_t		    __refcnt;
	atomic_t            dirty;
	unsigned short      vid;
	unsigned short      ports; /*端口索引port_no(低位起)对应的
	                            位是1表示端口在vlan_table中*/ 
	unsigned char       macln_enable;/*mac learning*/
	unsigned char       reserved1;
	unsigned short      reserved2;
};
/*end vlan bridge, s60000658, 20060627*/

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

#if defined(CONFIG_MIPS_BRCM)
struct net_bridge_mc_fdb_entry
{
	struct net_bridge_port		*dst;
	mac_addr			addr;
	mac_addr			host;
	unsigned char			is_local;
	unsigned char			is_static;
	unsigned long			tstamp;
	struct list_head 		list;
};

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;
	union {
		struct list_head	age_list;
		struct rcu_head		rcu;
	} u;
	atomic_t			use_count;
	unsigned long			ageing_timer;
	mac_addr			addr;
	unsigned char			is_local;
	unsigned char			is_static;
};
#endif

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;
	/*start vlan bridge, s60000658, 20060627*/
	unsigned short          mode;
	unsigned short          tci;           /* Encapsulates priority and VLAN ID */
	unsigned char           prio_enable;
	unsigned char           reserved1;
	unsigned short          reserved2;
	/*end vlan bridge, s60000658, 20060627*/
    
	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;
#if defined(CONFIG_MIPS_BRCM)
	int		 		dirty;
#endif

	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;
};

struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;
	struct net_device_stats		statistics;
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
	struct list_head		age_list;

/*start of IGMP Snooping 功能支持,除去桥结构里的组播内容 by l129990,2008,9,8*/	
#if 0
#if defined(CONFIG_MIPS_BRCM)
	struct list_head		mc_list;
	struct timer_list 		igmp_timer;
	int		 		proxy;
	spinlock_t			mcl_lock;
	int		 		start_timer;
#endif
#endif
/*end of IGMP Snooping 功能支持，除去桥结构里的组播内容 by l129990,2008,9,8*/	

    /*vlan bridge , s60000658, 20060627*/
    spinlock_t        vlock;
    struct list_head vlan_table[BR_VLAN_8021Q_MAX];
    unsigned char     vlan_enable;
    unsigned char     reserved1;
    u16               mngr_vid;  /*management vlan id*/

	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u16				root_port;
	unsigned char			stp_enabled;
	unsigned char			topology_change;
	unsigned char			topology_change_detected;

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			ifobj;
};

extern struct notifier_block br_device_notifier;
extern const unsigned char bridge_ula[6];

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}


/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern void br_fdb_init(void);
extern void br_fdb_fini(void);
extern void br_fdb_changeaddr(struct net_bridge_port *p,
			      const unsigned char *newaddr);
extern void br_fdb_cleanup(unsigned long arg);
extern void br_fdb_delete_by_port(struct net_bridge *br,
			   struct net_bridge_port *p);
extern struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
						 const unsigned char *addr);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					       unsigned char *addr);
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int br_fdb_fillbuf(struct net_bridge *br, void *buf, 
			  unsigned long count, unsigned long off);
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr,
			 int is_local);

/* br_forward.c */
extern void br_deliver(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_dev_queue_push_xmit(struct sk_buff *skb);
extern void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_forward_finish(struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
extern void br_flood_forward(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);

/* br_if.c */
extern int br_add_bridge(const char *name);
extern int br_del_bridge(const char *name);
extern void br_cleanup_bridges(void);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
extern void br_set_dhcprelay(int relayFlag);
/*end of support to the dhcp relay  function by l129990,2009,11,12*/
extern int br_min_mtu(const struct net_bridge *br);
/*start vlan bridge, s60000658, 20060627*/
extern struct vlan_8021q* br_get_vlan(struct net_bridge *br, unsigned short vid);
extern void br_release_vlan(struct vlan_8021q *pvlan);
extern int br_add_vlan(struct net_bridge * br, unsigned short vid);
extern int br_del_vlan (struct net_bridge * br, unsigned short vid);
extern int br_add_vlan_ports(struct net_bridge * br, unsigned short vid, unsigned short pts_mask);
extern int br_del_vlan_ports (struct net_bridge * br, unsigned short vid, unsigned short pts_mask);
extern int br_set_pmd(int ifindex, unsigned short mode);
extern int br_set_pvid(int ifindex, unsigned short pvid);
extern int br_set_prio(int ifindex, unsigned short prio, unsigned char enable);
extern int br_set_vlanflag(struct net_bridge *br, unsigned char vlanflag);
extern int br_set_mngr_vid(struct net_bridge *br, unsigned short vid);
extern int br_set_macln(struct net_bridge *br, unsigned short vid, unsigned char enable);
/*end vlan bridge, s60000658, 20060627*/

/* br_input.c */
extern int br_handle_frame_finish(struct sk_buff *skb);
extern int br_handle_frame(struct sk_buff *skb);
/*start of删除Brcm桥中涉及组播snooping的原有的变量和函数声明 by l129990 2008,9,9*/
#if 0
#if defined(CONFIG_MIPS_BRCM)
extern int snooping;
extern void query_timeout(unsigned long ptr);
extern int mc_forward(struct net_bridge *br, struct sk_buff *skb, unsigned char *dest,int forward, int clone);
extern void dolist(struct net_bridge *br);
extern int br_mc_fdb_update(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host);
extern struct net_bridge_mc_fdb_entry *br_mc_fdb_get(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host);
extern int br_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host);
extern void br_mc_fdb_cleanup(struct net_bridge *br);
extern void br_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest);
extern int br_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host);
extern void addr_debug(unsigned char *dest);
#endif
#endif
/*end of删除Brcm桥中涉及组播snooping的原有的变量和函数声明 by l129990 2008,9,9*/

/*start of support to the dhcp relay  function by l129990,2009,11,12*/
extern int dhcprelayCtl;
/*end of support to the dhcp relay  function by l129990,2009,11,12*/

/* br_ioctl.c */
extern int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
extern int br_ioctl_deviceless_stub(unsigned int cmd, void __user *arg);

/* br_netfilter.c */
extern int br_netfilter_init(void);
extern void br_netfilter_fini(void);

/* br_stp.c */
extern void br_log_state(const struct net_bridge_port *p);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
				    	   u16 port_no);
extern void br_init_port(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				       u16 newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
				     u8 newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
				 u32 path_cost);
extern ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
extern int br_stp_handle_bpdu(struct sk_buff *skb);

/* br_stp_timer.c */
extern void br_stp_timer_init(struct net_bridge *br);
extern void br_stp_port_timer_init(struct net_bridge_port *p);
extern unsigned long br_timer_value(const struct timer_list *timer);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern int br_sysfs_addif(struct net_bridge_port *p);
extern void br_sysfs_removeif(struct net_bridge_port *p);
extern void br_sysfs_freeif(struct net_bridge_port *p);

/* br_sysfs_br.c */
extern int br_sysfs_addbr(struct net_device *dev);
extern void br_sysfs_delbr(struct net_device *dev);

#else

#define br_sysfs_addif(p)	(0)
#define br_sysfs_removeif(p)	do { } while(0)
#define br_sysfs_freeif(p)	kfree(p)
#define br_sysfs_addbr(dev)	(0)
#define br_sysfs_delbr(dev)	do { } while(0)
#endif /* CONFIG_SYSFS */

#endif
