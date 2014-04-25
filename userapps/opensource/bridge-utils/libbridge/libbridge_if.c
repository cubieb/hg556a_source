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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include "libbridge.h"
#include "libbridge_private.h"


int br_add_bridge(const char *brname)
{
	int ret;

#ifdef SIOCBRADDBR
	ret = ioctl(br_socket_fd, SIOCBRADDBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] 
			= { BRCTL_ADD_BRIDGE, (unsigned long) _br };

		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_socket_fd, SIOCSIFBR, arg);
	} 

	return ret < 0 ? errno : 0;
}

int br_del_bridge(const char *brname)
{
	int ret;

#ifdef SIOCBRDELBR	
	ret = ioctl(br_socket_fd, SIOCBRDELBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] 
			= { BRCTL_DEL_BRIDGE, (unsigned long) _br };

		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_socket_fd, SIOCSIFBR, arg);
	} 
	return  ret < 0 ? errno : 0;
}

int br_add_interface(const char *bridge, const char *dev)
{
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);

	if (ifindex == 0) 
		return ENODEV;
	
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_socket_fd, SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };
					  
		ifr.ifr_data = (char *) args;
		err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}

	return err < 0 ? errno : 0;
}

int br_del_interface(const char *bridge, const char *dev)
{
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);

	if (ifindex == 0) 
		return ENODEV;
	
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRDELIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_socket_fd, SIOCBRDELIF, &ifr);
	if (err < 0)
#endif		
	{
		unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };
					  
		ifr.ifr_data = (char *) args;
		err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}

	return err < 0 ? errno : 0;
}
/*start vlan bridge, s60000658, 20060627*/
int br_add_vlan(const char *bridge, const char *vid)
{
	struct ifreq ifr;
	int err;
	char *p = vid;
	
	while('\0' != *p)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    	p++;
    }
		
	unsigned long args[4] = { BRCTL_ADD_VLAN, atol(vid), 0, 0 };
	
    if(DEBUG_VBR)
        printf("BRVLAN: add vlan %s, br %s, file %s, line %d\n", 
                 vid, bridge, __FILE__, __LINE__);
        
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_del_vlan(const char *bridge, const char *vid)
{
	struct ifreq ifr;
	int err;
	char *p = vid;
	
	while('\0' != *p)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    	p++;
    }
	
	unsigned long args[4] = { BRCTL_DEL_VLAN, atol(vid), 0, 0 };

    if(DEBUG_VBR)
        printf("BRVLAN: del vlan %d, br %s, file %s, line %d\n",
               vid, bridge, __FILE__, __LINE__);

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_add_vlanports(const char *bridge, unsigned short vid, unsigned short pts_mask)
{
	struct ifreq ifr;
	int err;
	unsigned long args[4] = { BRCTL_ADD_VLAN_PORTS, (unsigned long)vid, (unsigned short)pts_mask, 0 };

    if(DEBUG_VBR)
        printf("BRVLAN: add vlan ports, vlan %d, br %s, ports mask 0x%x, file %s, line %d\n", 
                vid, bridge, pts_mask, __FILE__, __LINE__);

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_del_vlanports(const char *bridge, unsigned short vid, unsigned short pts_mask)
{
	struct ifreq ifr;
	int err;
	unsigned long args[4] = { BRCTL_DEL_VLAN_PORTS, (unsigned long)vid, (unsigned short)pts_mask, 0 };
	
	if(DEBUG_VBR)
        printf("BRVLAN: del vlan ports, vlan %d, br %s, ports mask 0x%x, file %s, line %d\n", 
                 vid, bridge, pts_mask, __FILE__, __LINE__);

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_set_pmd(const char *port, const char *mode)
{
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(port);
	char *p = NULL;
	
	if (ifindex == 0) 
		return ENODEV;

    for(p = mode; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }
    
	if(atol(mode) < BR_PORT_MODE_NORMAL || atol(mode) >= BR_PORT_MODE_UNKNOWN)
	    return EINVAL;
	
	unsigned long args[4] = { BRCTL_SET_PORT_MODE, ifindex, atol(mode), 0 };

    if(DEBUG_VBR)
        printf("BRVLAN: set port %s mode %s,  file %s, line %d\n",
               port, mode,  __FILE__, __LINE__);

	strncpy(ifr.ifr_name, "br0", IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}

int br_set_pvid(const char *port, const char *pvid)
{
	struct ifreq ifr;
	int err;
	char *p = pvid;
	int  ifindex = if_nametoindex(port);

	if (ifindex == 0) 
		return ENODEV;

	while('\0' != *p)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    	p++;
    }
	
	unsigned long args[4] = { BRCTL_SET_PVID, ifindex, atol(pvid), 0 };

    if(DEBUG_VBR)
        printf("BRVLAN: set port %s pvid %s,  file %s, line %d\n",
               port, pvid,  __FILE__, __LINE__);

	strncpy(ifr.ifr_name, "br0", IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_set_prio(const char *port, const char *prio, const char *enable)
{
	struct ifreq ifr;
	int err;
	char *p = prio;
	int  ifindex = if_nametoindex(port);

	if (ifindex == 0) 
		return ENODEV;

	while('\0' != *p)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    	p++;
    }

    for(p = enable; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }
	
	unsigned long args[4] = { BRCTL_SET_PRIO, ifindex, atol(prio), atol(enable) };

    if(DEBUG_VBR)
        printf("BRVLAN: set port %s prio %s, file %s, line %d\n",
               port, prio,  __FILE__, __LINE__);

	strncpy(ifr.ifr_name, "br0", IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}

int br_set_vlanflag(const char *bridge, const char *vlanflag)
{
    struct ifreq ifr;
	int err;
	char *p ;
	
	for(p = vlanflag; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }
	
	unsigned long args[4] = { BRCTL_SET_VLANFLAG,  atol(vlanflag), 0, 0 };
	
    if(DEBUG_VBR)
        printf("BRVLAN: set bridge vlanflag %s, br %s, file %s, line %d\n", 
                 vlanflag, bridge, __FILE__, __LINE__);
        
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
int br_set_mngr_vid(const char *bridge, const char *vid)
{
    struct ifreq ifr;
	int err;
	char *p ;
	
	for(p = vid; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }
	
	unsigned long args[4] = { BRCTL_SET_MNGRVID,  atol(vid), 0, 0 };
	
    if(DEBUG_VBR)
        printf("BRVLAN: set bridge management vlan id %s, br %s, file %s, line %d\n", 
                 vid, bridge, __FILE__, __LINE__);
        
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
/*start of 增加添加内核brctl option60的值 by s53329 at  20070731*/
int mac_strtou8(const char *macstr, unsigned char *mac)
{	
    int i;
    char buf[8];
    for(i = 0; i < MAC_LEN; i++)
    {
        memset(buf, 0, sizeof(buf));
        strncpy(buf, macstr + i*2, 2);
        mac[i] = (unsigned char)strtoul(buf, NULL, 16);
    }
    return 1;
}
int br_add_dhcpoption(const char *dhcpoption)
{
       printf("enter br_add_dhcpoption \n");
       char option[DHCP_OPTION60];
       strcpy(option, dhcpoption);
       int err = 0;
	err = ioctl(br_socket_fd, SIOCBRADDDHCPOPTION, option);
	return err < 0 ? errno : 0;
}
int br_add_mccheck(const char *bridge, const char *src_mac)
{
	struct ifreq ifr;
	int err;
	unsigned char mac[MAC_LEN];
	mac_strtou8(src_mac, mac);
	unsigned long args[4] = { BRCTL_ADD_MCCHECK, mac, 0, 0 };
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	
	return err < 0 ? errno : 0;
}

int br_del_mccheck(const char *bridge, const char *src_mac)
{
	struct ifreq ifr;
	int err;
	unsigned char mac[MAC_LEN];
	mac_strtou8(src_mac, mac);
	unsigned long args[4] = { BRCTL_DEL_MCCHECK, mac, 0, 0 };
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
       ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
/*end  of 增加添加内核brctl option60的值 by s53329 at  20070731*/

/*end vlan bridge, s60000658, 20060627*/

int br_set_macln(const char *bridge, const char *vid, const char *enable)
{
    struct ifreq ifr;
	int err;
	char *p ;
	
	for(p = vid; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }

	for(p = enable; '\0' != *p; p++)
    {
        if( !isdigit(*p) )
        {
            return EINVAL;
	    }
    }
	
	unsigned long args[4] = { BRCTL_SET_MACLN,  atol(vid), atol(enable), 0 };
	
    if(DEBUG_VBR)
        printf("BRVLAN: set vlan %s mac learning %s, br %s, file %s, line %d\n", 
                 vid, enable, bridge, __FILE__, __LINE__);
        
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) args;
	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	return err < 0 ? errno : 0;
}
/*end vlan bridge, s60000658, 20060627*/
