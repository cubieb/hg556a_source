/* clientpacket.c
 *
 * Packet generation and dispatching functions for the DHCP client.
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
#include <string.h>
#include <sys/socket.h>
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "dhcpd.h"
#include "packet.h"
#include "options.h"
#include "dhcpc.h"
#include "debug.h"

#ifdef VDF_OPTION
	#include "board_api.h"
	
	#define MAX_SERIAL_NUM_LEN	32
#endif

/* Create a random xid */
unsigned long random_xid(void)
{
	static int initialized;
	if (!initialized) {
		srand(time(0));
		initialized++;
	}
	return rand();
}


/* initialize a packet with the proper defaults */
static void init_packet(struct dhcpMessage *packet, char type)
{
#ifdef VDF_OPTION
	char serial[MAX_SERIAL_NUM_LEN+1];
	//char macaddr[7];
	//char clientid[64]; 			//终端标识的格式为："OUI-SN"
	int i = 0;
	struct vendor  
	{
		char vendor, length;
		char str[64];
	} vendor_id = { DHCP_VENDOR,  sizeof(VDF_VERNDOR) - 1, VDF_VERNDOR};
#else
	struct vendor  
	{
		char vendor, length;
		/*w44771 mofify for A36D02098, OPTION60最大长度改为64，begin, 2006-6-20*/
		//char str[sizeof("uDHCP "VERSION)];
		char str[64];
		/*w44771 mofify for A36D02098, OPTION60最大长度改为64，end, 2006-6-20*/
	} vendor_id = { DHCP_VENDOR,  sizeof("uDHCP "VERSION) - 1, "uDHCP "VERSION};
#endif
	memset(packet, 0, sizeof(struct dhcpMessage));
	
	packet->op = BOOTREQUEST;
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	memcpy(packet->chaddr, client_config.arp, 6);
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);

#if 0
    #ifdef SUPPORT_OPTION61
	#ifndef VDF_OPTION
	if (client_config.clientid)
		free(client_config.clientid);

	client_config.clientid = malloc(2 + OPT61_DATA_LENGTH);
	if (client_config.clientid)
	{
       	client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
       	client_config.clientid[OPT_LEN] = OPT61_DATA_LENGTH;
       	client_config.clientid[OPT61_HWTYPE]=0x01;
       	memcpy(client_config.clientid+OPT61_MACADDR, client_config.arp, 6);
       	add_option_string(packet->options, client_config.clientid);
	}
	#else
        /* BEGIN: Modified by y67514, 2008/7/17   问题单号:将获取serial num的地方放在进程初始的时候，不用每次读*/
        if (client_config.clientid) 
        {
            add_option_string(packet->options, client_config.clientid);
        }
        else
        {
            memset(serial,0,sizeof(serial));
            while(!strlen(serial))
            {
                boardIoctl(BOARD_IOCTL_EQUIPMENT_TEST, GET_SERIAL_NUMBER, serial, MAX_SERIAL_NUM_LEN+1, 0, "");
                if(i++ > 2)
                {
                    printf("ERRO:DHCPC %d can't get the SN!\n",getpid());
                    break;
                }
                sleep(1);
            }

            if(strlen(serial))
            {
                	client_config.clientid = malloc(OPT61_MACADDR+ strlen(serial));
                	if (client_config.clientid)
                	{
                       	client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
                       	client_config.clientid[OPT_LEN] = strlen(serial)+1;
                       	client_config.clientid[OPT61_HWTYPE]=0x00;		//当option61内容为终端标识时，硬件type必须是0
                       	memcpy(client_config.clientid+OPT61_MACADDR, serial, strlen(serial));
                             add_option_string(packet->options, client_config.clientid);
                	}
            }
        }
        /* END:   Modified by y67514, 2008/7/17 */
	#endif
   	#else
	if (client_config.clientid) add_option_string(packet->options, client_config.clientid);
       #endif
 #endif

#ifdef SUPPORT_DHCPCLIENTID
    if (client_config.clientid) add_option_string(packet->options, client_config.clientid);
#else
    if (client_config.clientid)
		free(client_config.clientid);

	client_config.clientid = malloc(2 + OPT61_DATA_LENGTH);
	if (client_config.clientid)
	{
       	client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
       	client_config.clientid[OPT_LEN] = OPT61_DATA_LENGTH;
       	client_config.clientid[OPT61_HWTYPE]=0x01;
       	memcpy(client_config.clientid+OPT61_MACADDR, client_config.arp, 6);
       	add_option_string(packet->options, client_config.clientid);
	}
#endif

	if (client_config.hostname) add_option_string(packet->options, client_config.hostname);

        /* BEGIN: Added by y67514, 2008/9/27   PN:GLB:支持Option15*/
        if ( DHCP_DOMAIN_NAME == option_15[OPT_CODE])
        {
            add_option_string(packet->options, option_15);
        }
        /* END:   Added by y67514, 2008/9/27 */
        
// brcm
	if (strlen(vendor_class_id)) {
	    vendor_id.length = strlen(vendor_class_id);
	    sprintf(vendor_id.str, "%s", vendor_class_id);
	}
	add_option_string(packet->options, (char *) &vendor_id);
}


/* Add a paramater request list for stubborn DHCP servers */
static void add_requests(struct dhcpMessage *packet)
{
	char request_list[] = {DHCP_PARAM_REQ, 0, PARM_REQUESTS};
	
	request_list[OPT_LEN] = sizeof(request_list) - 2;
	add_option_string(packet->options, request_list);
}


/* Broadcast a DHCP discover packet to the network, with an optionally requested IP */
int send_discover(unsigned long xid, unsigned long requested)
{
	struct dhcpMessage packet;

	init_packet(&packet, DHCPDISCOVER);
	packet.xid = xid;
	if (requested)
		add_simple_option(packet.options, DHCP_REQUESTED_IP, ntohl(requested));

	add_requests(&packet);
/* j00100803 Add Begin 2008-03-08 */
#ifdef SUPPORT_VDF_DHCP
    char szOption125[VENDOR_IDENTIFYING_INFO_LEN];
    memset(szOption125, 0, VENDOR_IDENTIFYING_INFO_LEN);
	if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY, szOption125) != -1)
	{
		add_option_string(packet.options, szOption125);
	}

	char szOption77[128];
    szOption77[0] = DHCP_USER_CLASS;
    szOption77[1] = 4;
    szOption77[2] = 3;
    szOption77[3] = 'H';
    szOption77[4] = 'W';
    szOption77[5] = 'G';
    add_option_string(packet.options, szOption77);
#endif
/* j00100803 Add End 2008-03 -08 */
	// brcm
	// LOG(LOG_DEBUG, "Sending discover...");
	
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
     #if LOG_DISPLAY
	 syslog(LOG_INFO, "Sending discover on interface %s", session_path); 
     #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	return raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST, 
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
}


/* Broadcasts a DHCP request message */
int send_selecting(unsigned long xid, unsigned long server, unsigned long requested)
{
	struct dhcpMessage packet;
	struct in_addr addr;

	init_packet(&packet, DHCPREQUEST);
	packet.xid = xid;

	/* expects host order */
	add_simple_option(packet.options, DHCP_REQUESTED_IP, ntohl(requested));

	/* expects host order */
	add_simple_option(packet.options, DHCP_SERVER_ID, ntohl(server));
	
	add_requests(&packet);
	
/* j00100803 Add Begin 2008-03-08 */
#ifdef SUPPORT_VDF_DHCP
    char szOption125[VENDOR_IDENTIFYING_INFO_LEN];
    memset(szOption125, 0, VENDOR_IDENTIFYING_INFO_LEN);
	if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY,szOption125) != -1)
	{
		add_option_string(packet.options, szOption125);
	}
#endif
/* j00100803 Add End 2008-03-08*/	
	addr.s_addr = requested;
	// brcm
	//LOG(LOG_DEBUG, "Sending select for %s...", inet_ntoa(addr));
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
     #if LOG_DISPLAY
	 syslog(LOG_INFO, "Sending select for %s on interface %s ", inet_ntoa(addr), session_path);  
     #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	return raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST, 
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
}


/* Unicasts or broadcasts a DHCP renew message */
int send_renew(unsigned long xid, unsigned long server, unsigned long ciaddr)
{
	struct dhcpMessage packet;
	int ret = 0;
    /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	struct in_addr addr;
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	init_packet(&packet, DHCPREQUEST);
	packet.xid = xid;
	packet.ciaddr = ciaddr;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    addr.s_addr = server;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

	LOG(LOG_DEBUG, "Sending renew...");
	if (server) 
		{ret = kernel_packet(&packet, ciaddr, CLIENT_PORT, server, SERVER_PORT);
	    /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
        #if LOG_DISPLAY
		syslog(LOG_INFO, "Sending a dhcp renew message to server %s on interface %s ",
			inet_ntoa(addr), session_path);  
		#endif
		/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
		}
        else {ret = raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
	    /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
        #if LOG_DISPLAY
		syslog(LOG_INFO, "Broadcasting a dhcp renew message on interface %s on rebinding state", session_path);  
		#endif
        /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	}
        return ret;
}	
/* BEGIN: Added by y67514, 2008/2/20   PN:AU8D00203*/
int send_renew_with_request(unsigned long xid, unsigned long server, unsigned long ciaddr)
{
	struct dhcpMessage packet;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	struct in_addr addr;
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	int ret = 0;

	init_packet(&packet, DHCPREQUEST);
	packet.xid = xid;
	packet.ciaddr = ciaddr;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    addr.s_addr = server;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

	add_requests(&packet);
	
	LOG(LOG_DEBUG, "Sending renew...");
	if (server) 
		{ret = kernel_packet(&packet, ciaddr, CLIENT_PORT, server, SERVER_PORT);
	    /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
        #if LOG_DISPLAY
		syslog(LOG_INFO, "Sending a dhcp renew_with_request to server %s on interface %s", inet_ntoa(addr), session_path);  
        #endif
        /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	}
        
	else {ret = raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
	     /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
        #if LOG_DISPLAY
		syslog(LOG_INFO, "broadcasts a dhcp renew message on interface %s", session_path);  
        #endif
        /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	}
        return ret;
}	

/* END:   Added by y67514, 2008/2/20 */




/* Unicasts a DHCP release message */
int send_release(unsigned long server, unsigned long ciaddr)
{
	struct dhcpMessage packet;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	struct in_addr addr;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

	init_packet(&packet, DHCPRELEASE);
	packet.xid = random_xid();
	packet.ciaddr = ciaddr;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	addr.s_addr = server;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	
	/* expects host order */
	add_simple_option(packet.options, DHCP_REQUESTED_IP, ntohl(ciaddr));
	add_simple_option(packet.options, DHCP_SERVER_ID, ntohl(server));

	LOG(LOG_DEBUG, "Sending release...");
	    /*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
        #if LOG_DISPLAY
		syslog(LOG_INFO, "Sending release to server %s on interface %s", inet_ntoa(addr), session_path);  
        #endif
        /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	return kernel_packet(&packet, ciaddr, CLIENT_PORT, server, SERVER_PORT);
}


int get_raw_packet(struct dhcpMessage *payload, int fd)
{
	int bytes;
	struct udp_dhcp_packet packet;
	u_int32_t source, dest;
	u_int16_t check;

	memset(&packet, 0, sizeof(struct udp_dhcp_packet));
	bytes = read(fd, &packet, sizeof(struct udp_dhcp_packet));
	if (bytes < 0) {
		DEBUG(LOG_INFO, "couldn't read on raw listening socket -- ignoring");
		usleep(500000); /* possible down interface, looping condition */
		return -1;
	}
	
	if (bytes < (int) (sizeof(struct iphdr) + sizeof(struct udphdr))) {
		DEBUG(LOG_INFO, "message too short, ignoring");
		return -1;
	}
	
    /* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
    if (packet.ip.protocol != IPPROTO_UDP || packet.ip.version != IPVERSION ||
	    packet.ip.ihl != sizeof(packet.ip) >> 2 || packet.udp.dest != htons(CLIENT_PORT) ||
	    bytes > (int) sizeof(struct udp_dhcp_packet) 
	    ) {
	    	DEBUG(LOG_INFO, "unrelated/bogus packet");
	    	return -1;
	}
    /* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

	/* check IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;
	if (check != checksum(&(packet.ip), sizeof(packet.ip))) {
		DEBUG(LOG_INFO, "bad IP header checksum, ignoring");
		return -1;
	}
	
	/* verify the UDP checksum by replacing the header with a psuedo header */
	source = packet.ip.saddr;
	dest = packet.ip.daddr;
	check = packet.udp.check;
	packet.udp.check = 0;
	memset(&packet.ip, 0, sizeof(packet.ip));

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source;
	packet.ip.daddr = dest;
	packet.ip.tot_len = packet.udp.len; /* cheat on the psuedo-header */
	// brcm
	/*
	if (check != checksum(&packet, bytes)) {
		DEBUG(LOG_ERR, "packet with bad UDP checksum received, ignoring");
		// cwu
		//return -1;
	}
	*/
	
	memcpy(payload, &(packet.data), bytes - (sizeof(packet.ip) + sizeof(packet.udp)));
	
	if (ntohl(payload->cookie) != DHCP_MAGIC) {
		LOG(LOG_ERR, "received bogus message (bad magic) -- ignoring");
		return -1;
	}
	DEBUG(LOG_INFO, "oooooh!!! got some!");
	return bytes - (sizeof(packet.ip) + sizeof(packet.udp));
}

