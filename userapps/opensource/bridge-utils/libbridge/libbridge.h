/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _LIBBRIDGE_H
#define _LIBBRIDGE_H

#include <net/if.h>
#include <linux/if_bridge.h>

/*start vlan bridge, s60000658, 20060627*/
#ifndef DEBUG_VBR
#define  DEBUG_VBR 0
#endif
/*start of 增加brtcl 命令 by s53329 at  20070801*/
#define DHCP_OPTION60          264
#define MAC_LEN                       6
/*end  of 增加brtcl 命令 by s53329 at  20070801*/
enum 
{
    BR_PORT_MODE_NORMAL = 0,
    BR_PORT_MODE_ACCESS,
    BR_PORT_MODE_TRUNK,
    BR_PORT_MODE_HYBRID,
    /*more add here*/
    BR_PORT_MODE_UNKNOWN
};//refer to br_private.h
/*end vlan bridge, s60000658, 20060627*/

struct bridge_id
{
	unsigned char prio[2];
	unsigned char addr[6];
};

struct bridge_info
{
	struct bridge_id designated_root;
	struct bridge_id bridge_id;
	unsigned root_path_cost;
	struct timeval max_age;
	struct timeval hello_time;
	struct timeval forward_delay;
	struct timeval bridge_max_age;
	struct timeval bridge_hello_time;
	struct timeval bridge_forward_delay;
	u_int16_t root_port;
	unsigned char stp_enabled;
	unsigned char topology_change;
	unsigned char topology_change_detected;
	struct timeval ageing_time;
	struct timeval hello_timer_value;
	struct timeval tcn_timer_value;
	struct timeval topology_change_timer_value;
	struct timeval gc_timer_value;
};

struct fdb_entry
{
	u_int8_t mac_addr[6];
	u_int16_t port_no;
	unsigned char is_local;
	struct timeval ageing_timer_value;
};

struct port_info
{
	unsigned port_no;
	struct bridge_id designated_root;
	struct bridge_id designated_bridge;
	u_int16_t port_id;
	u_int16_t designated_port;
	u_int8_t priority;
	unsigned char top_change_ack;
	unsigned char config_pending;
	unsigned char state;
	unsigned path_cost;
	unsigned designated_cost;
	struct timeval message_age_timer_value;
	struct timeval forward_delay_timer_value;
	struct timeval hold_timer_value;
};

extern int br_init(void);
extern int br_refresh(void);
extern void br_shutdown(void);

extern int br_foreach_bridge(int (*iterator)(const char *brname, void *),
			     void *arg);
extern int br_foreach_port(const char *brname,
			   int (*iterator)(const char *brname, const char *port,
					   void *arg ),
			   void *arg);
extern const char *br_get_state_name(int state);

extern int br_get_bridge_info(const char *br, struct bridge_info *info);
extern int br_get_port_info(const char *brname, const char *port, 
			    struct port_info *info);
extern int br_add_bridge(const char *brname);
extern int br_del_bridge(const char *brname);
extern int br_add_interface(const char *br, const char *dev);
extern int br_del_interface(const char *br, const char *dev);
extern int br_set_bridge_forward_delay(const char *br, struct timeval *tv);
extern int br_set_bridge_hello_time(const char *br, struct timeval *tv);
extern int br_set_bridge_max_age(const char *br, struct timeval *tv);
extern int br_set_ageing_time(const char *br, struct timeval *tv);
extern int br_set_stp_state(const char *br, int stp_state);
extern int br_set_bridge_priority(const char *br, int bridge_priority);
extern int br_set_port_priority(const char *br, const char *p, 
				int port_priority);
// brcm begin
extern int br_set_port_snooping(const char *br, const char *p, 
				const char *addr);
extern int br_clear_port_snooping(const char *br, const char *p, 
				const char *addr);
extern int br_show_port_snooping(const char *br) ;
extern int br_enable_port_snooping(int enable) ;
// brcm end
/*start of support to the dhcp relay  function by l129990,2009,11,12*/
extern int br_enable_dhcp_relay(int dhcprelayFlag);
/*end of support to the dhcp relay  function by l129990,2009,11,12*/
extern int br_set_path_cost(const char *br, const char *p, 
			    int path_cost);
extern int br_read_fdb(const char *br, struct fdb_entry *fdbs, 
		       unsigned long skip, int num);
/*start vlan bridge, s60000658, 20060627*/
extern int br_add_vlan(const char *bridge, const char *vid);
extern int br_del_vlan (const char *bridge, const char *vid);
extern int br_add_vlanports(const char *bridge, unsigned short vid, unsigned short pts_mask);
extern int br_del_vlanports (const char *bridge, unsigned short vid, unsigned short pts_mask);
extern int br_get_vlan (const char *bridge, unsigned short vid, unsigned short* pports);
extern int br_get_portno(const char *brname, const char *ifname);
extern int br_set_pvid(const char *port, const char *pvid);
extern int br_set_pmd(const char *port, const char *mode);
extern int br_set_prio(const char *port, const char *prio, const char *enable);
extern int br_set_vlanflag(const char *bridge, const char *vlanflag);
extern int br_set_mngr_vid(const char *bridge, const char *vid);
extern int br_set_macln(const char *bridge, const char *vid, const char *enable);
/*end vlan bridge, s60000658, 20060627*/
/*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
extern  int br_cmd_add_mccheck(char** argv);
extern int br_del_mccheck(const char *bridge, const char *src_mac);
/*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/

#endif
