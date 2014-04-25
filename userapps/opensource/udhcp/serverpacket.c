/* serverpacket.c
 *
 * Constuct and send DHCP server packets
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	u_int32_t ciaddr;
	char chaddr[6];
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		memcpy(chaddr, MAC_BCAST_ADDR, 6);		
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		memcpy(chaddr, payload->chaddr, 6);
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		memcpy(chaddr, MAC_BCAST_ADDR, 6);		
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
        /*w44771 modify for A36D02260, 应该广播ack报文，begin, 2006-7-7*/
        //ciaddr = payload->yiaddr;
		ciaddr = INADDR_BROADCAST;        
	    /*w44771 modify for A36D02260, 应该广播ack报文，end, 2006-7-7*/

		memcpy(chaddr, payload->chaddr, 6);
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	
	packet->op = BOOTREPLY;
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
	add_simple_option(packet->options, DHCP_SERVER_ID, ntohl(server_config.server)); /* expects host order */
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}
static void add_option66_67_160( char *old_option,char * build_option,char option_id )
{
	memset(build_option,0,OPTION_LEN);
	int l_ilen = strlen(old_option);
	if( l_ilen > OPTION_LEN - 2 )
	{
		l_ilen = OPTION_LEN - 2;
	}
	build_option[OPTION_ID] = option_id;
	build_option[OPTION_ILEN] = l_ilen;
	memcpy(build_option + OPTION_VALUE,old_option,l_ilen);
}

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	char *req, *lease_time;
	struct option_set *curr;
	char * l_vdfoption[OPTION_LEN];
	int l_ilen = 0;
	struct in_addr addr;
#ifdef VDF_OPTION
//add for option125
        char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
        unsigned char StrOption121[OPTION121_LEN];
#endif
	/*start A36D02806, s60000658, 20060906*/
	struct option_set *opt = NULL;
	u_int32_t router_ip = 0;

    if(NULL != (opt = find_option(server_config.options, DHCP_ROUTER)))
    {
        router_ip = *(u_int32_t*)(opt->data + 2);
    }
    /*end A36D02806, s60000658, 20060906*/
	init_packet(&packet, oldpacket, DHCPOFFER);
	
#ifdef SUPPORT_MACMATCHIP
        if ( !ismacmatch(oldpacket->chaddr) )
        {  
			packet.yiaddr = find_matchip(oldpacket->chaddr);           
		} 
        else
#endif	
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		if (!lease_expired(lease)) 
			lease_time_align = lease->expires - getSysUpTime();
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		packet.yiaddr = lease->yiaddr;
		
	/* Or the client has a requested ip */
	} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

		   /* Don't look here (ugly hackish thing to do) */
		   memcpy(&req_align, req, 4) &&

		   /* and the ip is in the lease range */
		   ntohl(req_align) >= ntohl(server_config.start) &&
		   ntohl(req_align) <= ntohl(server_config.end) &&
		   #ifdef	VDF_RESERVED
		   //and the ip is not the reserved ip
		   reservedIp(req_align)&&
		   #endif
           #ifdef SUPPORT_MACMATCHIP
           isipmatch(req_align) &&
           #endif
		   /* and its not already taken/offered */
		   ((!(lease = find_lease_by_yiaddr(req_align)) ||
		   
		   /* or its taken, but expired */
		   lease_expired(lease)))) {

#ifdef SUPPORT_DHCP_FRAG
                         int tmpNum = 1, i =0;
			      if(1 == inmaster)
				{
				   for(i = 0; i < 5; i++)
				   {
				       if(ntohl(req_align) >= ntohl(ipPool[i].start) &&
		                       ntohl(req_align) <= ntohl(ipPool[i].end))
				       {
				           tmpNum = 0;
				           break;
				       }
				   }
				}
#endif
     				/*start of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/
				/*packet.yiaddr = req_align;			*/
				if((req_align != server_config.server)
#ifdef SUPPORT_DHCP_FRAG
                          && (req_align != router_ip) && (1 == tmpNum))/*A36D02806, s60000658, 20060906*/
#else
                          && (req_align != router_ip))/*A36D02806, s60000658, 20060906*/
#endif
                          {				    
				    packet.yiaddr = req_align; 
				}
			    else
				{					
#ifdef SUPPORT_DHCP_FRAG
				   if(1 == inmaster)
				   {
				    packet.yiaddr = find_address2(0);
				   }
				   else
#endif
				    packet.yiaddr = find_address(0);
				}
     				/*end of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/


	/* otherwise, find a free IP */
	} else {
#ifdef SUPPORT_DHCP_FRAG
		 if(1 == inmaster)
		{
		    packet.yiaddr = find_address2(0);
		}
		else
#endif
		packet.yiaddr = find_address(0);
		
		/* try for an expired lease */
		if (!packet.yiaddr) 
		{
#ifdef SUPPORT_DHCP_FRAG
			 if(1 == inmaster)
			{
			    packet.yiaddr = find_address2(1);
			}
			else
#endif
			packet.yiaddr = find_address(1);
		}
	}
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}

	#ifdef SUPPORT_CHINATELECOM_DHCP
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time, 0)) {
	#else
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
	#endif
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		

	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config.min_lease) 
		lease_time_align = server_config.lease;

	//w44771 add for 全新分配每次下发的租期,begin
#ifdef SUPPORT_DHCP_FRAG
      char *vendorid_tmp = NULL;
	int tmpIndex = 0;
	
	vendorid_tmp = get_option(oldpacket, DHCP_VENDOR);

        /*w44771 add for test CNTEL DHCP, begin*/
        #if 0
        char tmp60[64];
        memset(tmp60, 0, sizeof(tmp60));
        
        if(vendorid_tmp != NULL)
        {
            memcpy(tmp60, vendorid_tmp, *(vendorid_tmp - 1));            
            if(0 == strcmp(tmp60, "MSFT 5.0"))
            {
                memset(tmp60, 0, sizeof(tmp60));
                
                tmp60[0] = 60;
                tmp60[1] = 12;
                tmp60[2] = 'C';
                tmp60[3] = 'T';
                tmp60[4] = 1;
                tmp60[5] = 8;
                strcat(tmp60, "MSFT 5.0");

                vendorid_tmp = tmp60 + 2;                
            }
        }        
        #endif
        /*w44771 add for test CNTEL DHCP, end*/
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	cOption60Cont[0] = '\0';
	memset(cOption60Cont, 0, sizeof(cOption60Cont));
	if(NULL != vendorid_tmp)
	{
	    memcpy(cOption60Cont, vendorid_tmp - 2, (*(vendorid_tmp-1)) + 2);
	}	
	#endif
	
	if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid_tmp 
            && strlen(server2_config.vendorid) == *(vendorid_tmp - 1)  && !memcmp(vendorid_tmp, server2_config.vendorid, *(vendorid_tmp - 1)))
	{
		    lease_time_align = server2_config.lease;
		    //printf("===>send offer, use server2_config.lease= 0x%08x\n", server2_config.lease);
	}
	else
	{
		for(tmpIndex = 0; tmpIndex < 5; tmpIndex++)
		{
		  if(ntohl(packet.yiaddr) >= ntohl(ipPool[tmpIndex].start) &&
		     ntohl(packet.yiaddr) <= ntohl(ipPool[tmpIndex].end) )
		  	{
		  	    lease_time_align = ipPool[tmpIndex].lease;
		  	    //printf("===>send offer, use ipPool[%d].lease= 0x%08x\n", tmpIndex, ipPool[tmpIndex].lease);
		  	    break;
		  	}
		}
		if(5 == tmpIndex)
		{
		    lease_time_align = master_lease;
		    //printf("===>send offer, use master lease, master_lease=0x%08x\n",master_lease);
		}
	}
#endif
	//w44771 add for 全新分配每次下发的租期,end
		
	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
/*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
    add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
    add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
/*j00100803 Add End 2008-04-15 for dhcp option58,59  */
	curr = server_config.options;
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	if(cOption60Cont[0] != '\0')
	{
	    add_option_string(packet.options, cOption60Cont);
	}
	#endif


	add_bootp_options(&packet);
#ifdef VDF_OPTION
//add for option125
        /* if DHCPDISCOVER from client has device identity, send back gateway identity */
	if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) 
	{
		if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY,VIinfo) != -1)
		{
			add_option_string(packet.options,VIinfo);
		}
	}
#endif
	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	/*start: modify by xkf19988 2009.12.22*/
	
	 if( g_option66== 1 )
	{
		if(server_config.tftpserver!= NULL )
		{
			if( strlen(server_config.tftpserver) != 0 )
			{
       			add_option66_67_160(server_config.tftpserver,l_vdfoption,DHCP_TFTP_SERVER_NAME);
				add_option_string(packet.options, l_vdfoption);
			}
    	}
	}
	 if( g_option67== 1 )
	 {
		if(server_config.bootfilename!= NULL )
		{
			if( strlen(server_config.bootfilename) != 0 )
			{
				add_option66_67_160(server_config.bootfilename,l_vdfoption,DHCP_BOOTFILE_NAME);
				add_option_string(packet.options,l_vdfoption);
			}
		}
	}
	 if( g_option160== 1 )
	{
		if(server_config.option160!= NULL )
		{
			if(strlen(server_config.option160) != 0 )
			{
				add_option66_67_160(server_config.option160,l_vdfoption,DHCP_OPTION160);
				add_option_string(packet.options,l_vdfoption);
			}
		}
	}
	/*end :modify by xkf19988 2009.12.22*/
	
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
#ifdef SUPPORT_VDF_DHCP
    char szNotifyMsgOption56[128];
    szNotifyMsgOption56[0] = DHCP_MESSAGE;
    szNotifyMsgOption56[1] = 4;
    szNotifyMsgOption56[2] = '0';
    szNotifyMsgOption56[3] = '0';
    szNotifyMsgOption56[4] = '0';
    szNotifyMsgOption56[5] = '\0';
    add_option_string(packet.options, szNotifyMsgOption56);
#endif
/* j00100803 Add End 2008-03-10 */ 
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}


int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	char * l_vdfoption[OPTION_LEN];
	int l_ilen = 0;
	struct dhcpMessage packet;
	struct option_set *curr;
	char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;
#ifdef VDF_OPTION
//add for option125
        char VIinfo[VENDOR_IDENTIFYING_INFO_LEN];
        char *req;
        unsigned char StrOption121[OPTION121_LEN];
#endif

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;

	//w44771 add for 全新分配每次下发的租期,begin
#ifdef SUPPORT_DHCP_FRAG
      char *vendorid_tmp = NULL;
	int tmpIndex = 0;
	
	vendorid_tmp = get_option(oldpacket, DHCP_VENDOR);

        /*w44771 add for test CNTEL DHCP, begin*/
        #if 0
        char tmp60[64];
        memset(tmp60, 0, sizeof(tmp60));
        
        if(vendorid_tmp != NULL)
        {
            memcpy(tmp60, vendorid_tmp, *(vendorid_tmp - 1));            
            if(0 == strcmp(tmp60, "MSFT 5.0"))
            {
                memset(tmp60, 0, sizeof(tmp60));
                
                tmp60[0] = 60;
                tmp60[1] = 18;
                tmp60[2] = 'C';
                tmp60[3] = 'T';
                tmp60[4] = 1;
                tmp60[5] = 8;
                strcat(tmp60, "MSFT 5.0");

                *(tmp60 + 6 + strlen("MSFT 5.0")) = 5; //Field Type 5
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1) = 4;//Field Len
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1) = 0;//top protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1 + 1) = 11;//bottom protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1) = 20;//top port half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1 + 1) = 6;//top port half

                vendorid_tmp = tmp60 + 2;                
            }
        }        
        #endif
        /*w44771 add for test CNTEL DHCP, end*/
	
	#ifdef SUPPORT_CHINATELECOM_DHCP
	cOption60Cont[0] = '\0';
	memset(cOption60Cont, 0, sizeof(cOption60Cont));
	if(NULL != vendorid_tmp)
	{
	    memcpy(cOption60Cont, vendorid_tmp - 2, (*(vendorid_tmp-1)) + 2);
	}	
	#endif
	
	if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid_tmp 
            && strlen(server2_config.vendorid) == *(vendorid_tmp - 1)  && !memcmp(vendorid_tmp, server2_config.vendorid, *(vendorid_tmp - 1)))
	{
		    lease_time_align = server2_config.lease;
		    //printf("===>send ack, use server2_config.lease= 0x%08x\n", server2_config.lease);
	}
	else
	{
		for(tmpIndex = 0; tmpIndex < 5; tmpIndex++)
		{
		  if(ntohl(packet.yiaddr) >= ntohl(ipPool[tmpIndex].start) &&
		     ntohl(packet.yiaddr) <= ntohl(ipPool[tmpIndex].end) )
		  	{
		  	    lease_time_align = ipPool[tmpIndex].lease;
		  	    //printf("===>send ack, use ipPool[%d].lease= 0x%08x\n", tmpIndex, ipPool[tmpIndex].lease);
		  	    break;
		  	}
		}
		if(5 == tmpIndex)
		{
		    lease_time_align = master_lease;
		    //printf("===>send ack, use master lease, master_lease=0x%08x\n",master_lease);
		}
	}
#endif
	//w44771 add for 全新分配每次下发的租期,end
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
/*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
    add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
    add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
/*j00100803 Add End 2008-04-15 for dhcp option58,59  */	
	curr = server_config.options;
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */

	#ifdef SUPPORT_CHINATELECOM_DHCP
	if(cOption60Cont[0] != '\0')
	{
	    add_option_string(packet.options, cOption60Cont);
	}

	char tmp43[8];
	
	memset(tmp43, 0, sizeof(tmp43));
	tmp43[0] = 43;
	tmp43[1] = strlen(cOption43.configVersion);
	strcat(tmp43, cOption43.configVersion);

	if(tmp43[2] != '\0')
	{
	    add_option_string(packet.options, tmp43);
	}
	#endif

	add_bootp_options(&packet);
	/*start: modify by xkf19988 2009.12.22*/
	 if( g_option66== 1 )
	{
		if(server_config.tftpserver!= NULL )
       	 {
       	 	if( strlen( server_config.tftpserver) != 0 )
       	 	{
       			add_option66_67_160(server_config.tftpserver,l_vdfoption,DHCP_TFTP_SERVER_NAME);
				add_option_string(packet.options, l_vdfoption);
       	 	}
    	}
	}
	 if( g_option67== 1 )
	{
		if(server_config.bootfilename!= NULL )
		{
			if( strlen( server_config.bootfilename) != 0 )
			{
				add_option66_67_160(server_config.bootfilename,l_vdfoption,DHCP_BOOTFILE_NAME);
				add_option_string(packet.options,l_vdfoption);
			}
		}
	}
	if( g_option160== 1 )
	{
		if(server_config.option160!= NULL )
		{
			if( strlen( server_config.option160) != 0 )
			{
				add_option66_67_160(server_config.option160,l_vdfoption,DHCP_OPTION160);
				add_option_string(packet.options,l_vdfoption);
			}
		}
	}
#ifdef VDF_OPTION
//add for option125
        /* if DHCPRequest from client has device identity, send back gateway identity,
           and save the device identify */
	if ((req = get_option(oldpacket, DHCP_VENDOR_IDENTIFYING))) 
	{
		if (createVIoption(VENDOR_IDENTIFYING_FOR_GATEWAY,VIinfo) != -1)
			add_option_string(packet.options,VIinfo);
		saveVIoption(req,packet.yiaddr);
	}
#endif

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0) < 0) 
		return -1;

       #ifdef SUPPORT_CHINATELECOM_DHCP
       char *vendorid = NULL;
       int foundport = 0;
       unsigned int portvalue = 0;
       int i = 0;
       char portstr[3];

       memset(portstr, 0, sizeof(portstr));
	vendorid = get_option(oldpacket, DHCP_VENDOR);

	/*w44771 for debug test, begin*/
	//vendorid = cOption60Cont + 2;
	/*w44771 for debug test, end*/
	
       if(*(vendorid-1) > 4)//最少有一个Field Type
       {                          
           vendorid += 2;//指向第一个Field Type

           for(i = 0; i < 5; i++)//最多有5个Field
           {
               if(*vendorid == 5)//Type 5, protocol&port
               {
                  vendorid += 4;
                  foundport = 1;
                  //printf("===>CHINA TELECOM Port found!\n");
                  break;
               }
               else
               {
                    vendorid =vendorid + *(vendorid +1) + 2;//指向下一个Field Type 
               }
       	  }
       }

       if(1 == foundport)
       {
           memcpy(portstr, vendorid, 2);
           portvalue = ntohs(*(unsigned short *)(portstr));
           //printf("===>portvalue is %d\n", portvalue);
       }
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align, portvalue);
       #else
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
       #endif
       
	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;
        unsigned char StrOption121[OPTION121_LEN];
        u_int32_t lease_time_align = server_config.lease;

    	init_packet(&packet, oldpacket, DHCPACK);

        add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);
        /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
        add_simple_option(packet.options, DHCP_T1, lease_time_align / 2);
        add_simple_option(packet.options, DHCP_T2, (lease_time_align * 0x07) >> 3);
#endif
        /*j00100803 Add End 2008-04-15 for dhcp option58,59  */ 
	
	curr = server_config.options;        
        /* BEGIN: Modified by y67514, 2008/5/22 Vista无法获取地址*/
        if ( flag_option121)
        {
            //option3和邋Option121不能同时下发
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE)&&
                    (curr->data[OPT_CODE] != DHCP_ROUTER))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
            StrOption121[OPT_CODE] = DHCP_STATIC_ROUTE;
            StrOption121[OPT_LEN] = 5;
            StrOption121[OPT_VALUE] = 0;
            memcpy(&StrOption121[OPT_VALUE + 1], &server_config.server, 4);
            add_option_string(packet.options, StrOption121);
        }
        else
        {
            while (curr) 
            {
                /*j00100803 Add Begin 2008-04-15 for dhcp option58,59 */
#ifdef SUPPORT_VDF_DHCP
                if ((curr->data[OPT_CODE] != DHCP_LEASE_TIME) &&
                    (curr->data[OPT_CODE] != DHCP_T1) &&
                    (curr->data[OPT_CODE] != DHCP_T2)&&
                    (curr->data[OPT_CODE] != DHCP_STATIC_ROUTE))
#else
                if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
#endif
                /*j00100803 Add End 2008-04-15 for dhcp option58,59  */
                {
                    add_option_string(packet.options, curr->data);
                }
                curr = curr->next;
            }
        }
        /* END:   Modified by y67514, 2008/5/22 */
        
	add_bootp_options(&packet);

	return send_packet(&packet, 0);
}



