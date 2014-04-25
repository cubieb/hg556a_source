
#ifndef __BR_IGMP_SNOOPING_H
#define __BR_IGMP_SNOOPING_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>

#define IGMP_TIMER_CHECK_TIMEOUT 10
#define IGMP_QUERY_TIMEOUT 150

#ifndef ETH_ALEN
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#endif

#define BR_IGMP_SNOOPING_DISABLE 0
#define BR_IGMP_SNOOPING_ENABLE 1

typedef union ipv4
{
    unsigned int ulIpAddr;
    unsigned char acIpAddr[4];
}igmp_ipaddr;

struct net_bridge_igmp_snooping_entry
{
    struct list_head 		  list;
	struct net_bridge_port	  *dev_dst;             /*目的桥端口对应的地址，如eth0.**/
	unsigned char			  grp_mac[ETH_ALEN];  /*组播地址对应的MAC地址，如01:00:5e:xx:xx:xx*/
	unsigned char			  host_mac[ETH_ALEN];   /*PC机的MAC地址*/
	unsigned long			  time;                  /*定时器*/
};

struct net_bridge_igmp_snooping
{
	struct list_head		igmp_list;         /*链表头*/
	struct timer_list 		igmp_timer;        /*定时器*/
	int		                igmp_snooping_enable;         /*0:表示IGMP Snooping开启*/
	spinlock_t		        igmp_lock;         /*链表锁*/
	int		 	            igmp_start_timer; /*定时器是否开启的标志*/
};

extern  void  br_igmp_snooping_clear(void);
extern  int   br_igmp_snooping_forward(struct sk_buff *skb, struct net_bridge *br,unsigned char *dest,int forward);
extern  void  br_igmp_snooping_set_enable(int enable);
extern  void  br_igmp_snooping_show(void);
extern  void  br_igmp_snooping_init(void);
/*start of 增加对组播MAC端口转发表增加、删除函数的声明by l129990 2008,9,9*/
extern   int br_igmp_snooping_add(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac);
extern   int br_igmp_snooping_del(struct net_bridge_port *dev_port, unsigned char *grp_mac, unsigned char *host_mac);
extern   int br_igmp_snooping_del_grp(struct net_bridge_port *dev_port, unsigned char *grp_mac);
/*end of 增加对组播MAC端口转发表增加、删除函数的声明by l129990 2008,9,9*/
#endif
