/* dhcpd.c
 *
 * Moreton Bay DHCP Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"


/* globals */
/* BEGIN: Added by y67514, 2008/5/22 */
int flag_option121 = 0;
/* END:   Added by y67514, 2008/5/22 */
/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
struct domainTab domains[32];
int g_enblDomainTab = 0;        /*域名IP对应的规则数量*/
int g_needToChang = 0;          /*是否需要通知dns*/
int g_dnsPid = 0;                      /*dnsmasq的pid*/
/* END:   Added by y67514, 2008/9/18 */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
/*begin DHCP Server支持第二地址池, s60000658, 20060616*/
struct dhcpOfferedAddr *leases2;
struct server_config_t server2_config;
int g_enblSrv1 = 0;
int g_enblSrv2 = 0;
/*end DHCP Server支持第二地址池, s60000658, 20060616*/
// BRCM_begin
struct dhcpOfferedAddr *declines;
struct vendor_id_config_t vendor_id_config[MAX_VENDOR_IDS];
#ifdef VDF_OPTION
//add for option125
pVI_INFO_LIST viList;
#endif

 /*start DHCP Server支持第二地址池, s60000658, 20060616*/
    unsigned long max_lease_in_both;
    struct dhcpOfferedAddr *tmpls;
    struct server_config_t  tmpSrvCfg; 
    /*end DHCP Server支持第二地址池, s60000658, 20060616*/
static int vendor_id_cfg = 0;
// BRCM_end
/*start: modify by xkf19988 2009.12.21*/
int g_option66= 0;
int g_option67 = 0;
int g_option160 = 0;
/*end: modify by xkf19988 2009.12.21*/

/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
struct ipPoolData ipPool[5];
int ipPoolIndex;
u_int32_t Xid[256];
u_int32_t master_start;		/* Start address of leases, network order */
u_int32_t master_end;			/* End of leases, network order */
unsigned long master_lease;
int inmaster;/*== 1 表示是在主地址池中分配*/
int ismatch;/*== 1 表示有匹配的option*/

#ifdef SUPPORT_CHINATELECOM_DHCP
struct cntelOption60 cOption60;
struct cntelOption43 cOption43;
char cOption60Cont[256];//存放整个option60

char c60common[256];//存放整理后的option60
char ccategory[32];
char cmodel[32];
#endif
#endif
/*w44771 add for 第一IP支持5段地址池，end*/

#ifdef SUPPORT_MACMATCHIP
PMACIP_LIST _macip_list = NULL;
#endif

/* Start to update lease when this flag is open. <tanyin 2009.3.13> */
int gUpdateLease = FALSE;

/* Exit and cleanup */
static void exit_server(int retval)
{
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void udhcpd_killed(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
	exit_server(0);
}


// BRCM_BEGIN
static void test_vendorid(struct dhcpMessage *packet, char *vendorid, int *declined)
{
	int i = 0;
	char length = 0;
        char opt_vend[64];
	for (i = 0; i < MAX_VENDOR_IDS; i++) {
		if (strlen(vendor_id_config[i].vendorid) == 0) {
			continue;
		}
		memset(opt_vend, 0, 64);
		length = *(vendorid - 1);
		memcpy(opt_vend, vendorid, (size_t)length);
		if (strlen(opt_vend) != strlen(vendor_id_config[i].vendorid)) {
			continue;
		}
		if (strncmp(opt_vend, vendor_id_config[i].vendorid,
			strlen(vendor_id_config[i].vendorid)) == 0) {
			memset(declines, 0, sizeof(struct dhcpOfferedAddr));
			memcpy(declines->chaddr, packet->chaddr, 16);
			memset(declines->vendorid, 0, 64);
			memcpy(declines->vendorid, opt_vend, (size_t)length);
			/* Write this to the decline file */
			write_decline(0);
			/* remain silent */
			sendNAK(packet);
			*declined = 1;
			break;
		}
	}
}
// BRCM_END

/*
 * signal handler. <tanyin 2009.3.13>
 */
void start_update_lease(int dumy)
{
    gUpdateLease = TRUE;
}

#ifdef COMBINED_BINARY	
int udhcpd(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
        //BRCM --initialize server_socket to -1
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
       /* BEGIN: Added by y67514, 2008/5/22 Vista无法获取地址*/
       unsigned char *RequestList = NULL;
       int req_num = 0;
       int k = 0;
       /* END:   Added by y67514, 2008/5/22 */
       glb_isDhcpc = 0;
        // BRCM added vendorid
	char *server_id, *requested, *hostname, *vendorid = NULL;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	struct sockaddr_in *sin;
	int pid_fd;
	/* j00100803 Add Begin 2008-04-15 dhcp option77*/
	#ifdef SUPPORT_VDF_DHCP
/*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
    struct in_addr addrOpt50;
/*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
	char * pUserClassInfo = NULL;
	char szUserClassInfo[128];
	char szUserMacAddr[18];
	FILE * pUserClassFile = NULL;
	int iLen = 0;
	#endif
	/* j00100803 Add End 2008-04-15 dhcp option77*/		

	/* server ip addr */
	int fd = -1;
	struct ifreq ifr;
	// BRCM_begin
        int i = 0;
        char opt_vend[64];
        // BRCM_end
#ifdef  SUPPORT_PORTMAPING
        unsigned char  vendorstr[48];
         unsigned int   option_len;
#endif

	argc = argv[0][0]; /* get rid of some warnings */
	
	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);
	
	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	memset(&server_config, 0, sizeof(struct server_config_t));
	memset(&server2_config, 0, sizeof(struct server_config_t));	/*DHCP Server支持第二地址池, s60000658, 20060616*/
	memset(&tmpSrvCfg, 0, sizeof(struct server_config_t));
	
	read_config(DHCPD_CONF_FILE);
        /* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
        read_domain();
        /* END:   Added by y67514, 2008/9/18 */
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
      for (i = 0; i < 5; i++)
      	{
      	    ipPool[i].lease *= 3600; 
      	}
      master_start = server_config.start;
      master_end = server_config.end;
      
#ifdef SUPPORT_CHINATELECOM_DHCP
      if(strlen(cOption60.vendor) > 0)
      	{
      	     strcpy(ipPool[0].option60, cOption60.vendor);
      	}
#endif

#endif
/*w44771 add for 第一IP支持5段地址池，end*/	
	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;

#ifdef SUPPORT_DHCP_FRAG
      master_lease = server_config.lease;
#endif
        // 061208 add by c60023298
	if(server_config.max_leases > server2_config.max_leases)
		{
		max_lease_in_both=server_config.max_leases;
		}
	else
		{
		max_lease_in_both=server2_config.max_leases;
		}
         tmpls = malloc(sizeof(struct dhcpOfferedAddr) * max_lease_in_both);
	memset(tmpls, 0, sizeof(struct dhcpOfferedAddr) * max_lease_in_both);	
	// end by c60023298
	
	leases = malloc(sizeof(struct dhcpOfferedAddr) * max_lease_in_both);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * max_lease_in_both);
	read_leases(server_config.lease_file);

	/*start DHCP Server支持第二地址池, s60000658, 20060616*/
	if ((option = find_option(server2_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server2_config.lease, option->data + 2, 4);
		server2_config.lease = ntohl(server2_config.lease);
	}
	else server2_config.lease = LEASE_TIME;

	leases2 = malloc(sizeof(struct dhcpOfferedAddr) * max_lease_in_both);
	memset(leases2, 0, sizeof(struct dhcpOfferedAddr) * max_lease_in_both);
	read_leases2(server2_config.lease_file);
	/*end DHCP Server支持第二地址池, s60000658, 20060616*/	
        // BRCM
	declines = malloc(sizeof(struct dhcpOfferedAddr));
#ifdef VDF_OPTION
//add for option125
	/* vendor identifying info list */
	viList = malloc(sizeof(VI_INFO_LIST));
	memset(viList, 0, sizeof(VI_INFO_LIST));
#endif

	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, server_config.interface);
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
			sin = (struct sockaddr_in *) &ifr.ifr_addr;
			server_config.server = sin->sin_addr.s_addr;
			server2_config.server = sin->sin_addr.s_addr;/*DHCP Server支持第二地址池, s60000658, 20060616*/
			DEBUG(LOG_INFO, "%s (server_ip) = %s", ifr.ifr_name, inet_ntoa(sin->sin_addr));
		} else {
			LOG(LOG_ERR, "SIOCGIFADDR failed!");
			exit_server(1);
		}
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			server_config.ifindex = ifr.ifr_ifindex;
			server2_config.ifindex = ifr.ifr_ifindex;/*DHCP Server支持第二地址池, s60000658, 20060616*/
		} else {
			LOG(LOG_ERR, "SIOCGIFINDEX failed!");
			exit_server(1);
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(server_config.arp, ifr.ifr_hwaddr.sa_data, 6);
			memcpy(server2_config.arp, ifr.ifr_hwaddr.sa_data, 6);/*DHCP Server支持第二地址池, s60000658, 20060616*/
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				server_config.arp[0], server_config.arp[1], server_config.arp[2], 
				server_config.arp[3], server_config.arp[4], server_config.arp[5]);
		} else {
			LOG(LOG_ERR, "SIOCGIFHWADDR failed!");
			exit_server(1);
		}
	} else {
		LOG(LOG_ERR, "socket failed!");
		exit_server(1);
	}
	close(fd);

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	switch(fork()) {
	case -1:
		perror("fork");
		exit_server(1);
		/*NOTREACHED*/
	case 0:
		break; /* child continues */
	default:
		exit(0); /* parent exits */
		/*NOTREACHED*/
		}
	close(0);
	setsid();
	pidfile_write_release(pid_fd);
#endif


	signal(SIGUSR1, write_leases);
    signal(SIGUSR2, write_leases2);/*DHCP Server支持第二地址池, s60000658, 20060616*/
	signal(SIGTERM, udhcpd_killed);
/*GLB:y67514,for DNS*/
    signal(SIGALRM, write_dnsconf);

    /* Update lease table when one of the LAN host leases.
     * Ethernet driver is supposed to send this signal.(see bcmenet.c)
     * <tanyin 2009.3.13>
     */
    signal(SIGHUP, start_update_lease);


        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	timeout_end = getSysUpTime() + server_config.auto_time;
       /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/

	while(1) { /* loop until universe collapses */
                //BRCM_begin
                int declined = 0;
                if (server_socket < 0) {
                     server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface);
                     if(server_socket < 0) {
                           LOG(LOG_ERR, "couldn't create server socket -- au revoir");
                           exit_server(0);
                     }			
                }  //BRCM_end

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		if (server_config.auto_time) {
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			tv.tv_sec = timeout_end - getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			if (tv.tv_sec <= 0) {
				tv.tv_sec = server_config.auto_time;
                            /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
				timeout_end = getSysUpTime() + server_config.auto_time;
                            /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
				write_leases(0);
				write_leases2(0);/*DHCP Server支持第二地址池, s60000658, 20060616*/
			}
			tv.tv_usec = 0;
		}
		retval = select(server_socket + 1, &rfds, NULL, NULL, server_config.auto_time ? &tv : NULL);
		if (retval == 0) {
			write_leases(0);
			write_leases2(0);/*DHCP Server支持第二地址池, s60000658, 20060616*/
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			timeout_end = getSysUpTime() + server_config.auto_time;
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
                        close(server_socket);
                        //BRCM
                        server_socket = -1;

			continue;
		} else if (retval < 0) {
			DEBUG(LOG_INFO, "error on select");
            /* select() is inturrupted by a signal. <tanyin 2009.3.13>*/
            if (errno == EINTR) {
                /* Start to update lease when ethx is disabled. <tanyin 2009.3.13> */
                if (gUpdateLease) {
                    update_lease(0);
                    gUpdateLease = FALSE;
                }
            } else {
    			close(server_socket);
                //BRCM
                server_socket = -1;
            }
			continue;
		}
		
		bytes = get_packet(&packet, server_socket); /* this waits for a packet - idle */
                //BRCM_BEGIN
                //close(server_socket);
		if(bytes < 0)
			continue;

		if((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
			continue;
		}
#ifdef SUPPORT_PORTMAPING
        vendorid = get_option(&packet, DHCP_VENDOR);
        if (NULL != vendorid)
        {
        	char cmd[128];
        	
            option_len = (unsigned int)*(vendorid - 1);
            //printf("====option_len=%d\n",option_len);
            //printf("====dhcp=====option60=%s, \n",server_config.option60);
	        memset(vendorstr, 0, sizeof(vendorstr));
			strncpy(vendorstr,vendorid,option_len);
			//printf("====vendorid=%s\n",vendorstr);
			
			if(check_option60(server_config.option60, vendorstr))
			{
				sprintf(cmd, "brctl addmc br0 %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", 
					packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
					packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);

				system(cmd);
				continue;
			}
			
			sprintf(cmd, "brctl delmc br0 %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", 
					packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
					packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
					
			system(cmd);
            /*if (!strncmp(vendorid, "Aminoaminet110fisys",option_len))
            {
                continue;
            }*/
        }
#endif
        /*start DHCP Server支持第二地址池, s60000658, 20060616*/
        vendorid = get_option(&packet, DHCP_VENDOR);
        /*w44771 add for test CNTEL DHCP, begin*/
        #if 0
        char tmp60[64];
        memset(tmp60, 0, sizeof(tmp60));
        
        if(vendorid != NULL)
        {
            memcpy(tmp60, vendorid, *(vendorid - 1));            
            if(0 == strcmp(tmp60, "MSFT 5.0"))
            {
                memset(tmp60, 0, sizeof(tmp60));
                
                tmp60[0] = 60;
                tmp60[1] = 12;
                tmp60[2] = 'C';
                tmp60[3] = 'T';
                tmp60[4] = 1;
                tmp60[5] = 8;
                strcat(tmp60, "MSFT 5.0");//vendor

                /*
                *(tmp60 + 6 + strlen("MSFT 5.0")) = 2; //Field Type
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1) = 3;//Field Len
                strcat((tmp60 + 6 + strlen("MSFT 5.0") + 1 +1), "cat" );

                *(tmp60 + 6 + strlen("MSFT 5.0") + 2 + strlen("cat")) = 3; //Field Type
                *(tmp60 + 6 + strlen("MSFT 5.0") + 2 + strlen("cat") + 1) = 5;//Field Len
                strcat((tmp60 + 6 + strlen("MSFT 5.0") + 2 + strlen("cat") + 1 +1), "model" );
                */
                

                /*
                *(tmp60 + 6 + strlen("MSFT 5.0")) = 5; //Field Type 5
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1) = 4;//Field Len
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1) = 0;//top protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 +1 + 1) = 11;//bottom protocol half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1) = 20;//top port half
                *(tmp60 + 6 + strlen("MSFT 5.0") + 1 + 1 + 1 + 1 + 1) = 6;//top port half
                */

                vendorid = tmp60 + 2;                
            }
        }        
        #endif
        /*w44771 add for test CNTEL DHCP, end*/
        if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid 
            && strlen(server2_config.vendorid) == *(vendorid - 1)  && !memcmp(vendorid, server2_config.vendorid, *(vendorid - 1)))//w44771 modify for A36D02366
        {
            //缺省地址池和租用列表备份, 使用第二地址池
            memcpy(&tmpSrvCfg, &server_config, sizeof(struct server_config_t));
            memcpy(&server_config, &server2_config, sizeof(struct server_config_t));
            for (i = 0; i < max_lease_in_both ; i++)
          	{          
          	memcpy(&tmpls[i], &leases[i], sizeof(struct dhcpOfferedAddr));
          	memcpy(&leases[i], &leases2[i], sizeof(struct dhcpOfferedAddr)); 
          	}
#ifdef SUPPORT_DHCP_FRAG
            inmaster = 2;
#endif
        }
        else if(0 == g_enblSrv1)
        {
            continue;
        }
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
        else if(1 == g_enblSrv1)
        {
            //取出xid，若Xid[256]中没有此xid，则加入；如有此xid，则不在匹配此xid.
            //printf("===>dhcpd, xid is %lu\n", packet.xid);
            for (i = 0; i < 256; i++)
            {
                if((0 != Xid[i]) && (Xid[i] == packet.xid))
                {
                   //printf("===>find xid!\n");
                   break;
                }
            }

            if(256 == i) //没有找到相应的xid
            	{
            	      //将xid加入Xid[256]中
            	      for(i = 0; i < 256; i++)
            	      {
            	          if(0 == Xid[i])
            	          {
            	             break;
            	          }
            	      }
            	      if(i < 256)
            	      {
            	         Xid[i] = packet.xid;
            	      }
            	      
        
	             inmaster = 0;
	             ismatch = 0;
	             //若client带的option60为空，在主地址池中分配
	             if (NULL == vendorid)
	             {
	                 inmaster = 1;
		          server_config.start = master_start;
		          server_config.end = master_end;
		          server_config.lease = master_lease;
	             }
	             else //客户端option60不为空
	             {

                       #ifdef SUPPORT_CHINATELECOM_DHCP
                       char ccategoryvar[32];
                       char cmodelvar[32];
                       int catandmod = 0;
                       int cat = 0;
                       int mod = 0;
                       
                       memset(ccategoryvar, 0, sizeof(ccategoryvar));
                       memset(cmodelvar, 0, sizeof(cmodelvar));
                       
                       if(*(vendorid-1) > 4)//最少有一个Field Type
                       {                          
                           vendorid += 2;//指向第一个Field Type

                           char *tmppoint = vendorid;//提取category
                           for(i = 0; i < 5; i++)//最多有5个Field
                           {
	                           if(*tmppoint == 2)//Type 2, category
	                           {
	                              tmppoint += 2;
	                              printf("===>CHINA TELECOM Category found!\n");
	                              break;
	                           }
	                           else
	                           {
	                            tmppoint =tmppoint + *(tmppoint +1) + 2;//指向下一个Field Type 
	                           }
                       	  }
                           if( i < 5 )//找到category
                           {
                               memcpy(ccategoryvar, tmppoint, *(tmppoint - 1));
                           }

                           tmppoint = vendorid;//提取model
                           for(i = 0; i < 5; i++)//最多有5个Field
                           {
	                           if(*tmppoint == 3)//Type 3, model
	                           {
	                              tmppoint += 2;
	                              printf("===>CHINA TELECOM Model found!\n");
	                              break;
	                           }
	                           else
	                           {
	                            tmppoint =tmppoint + *(tmppoint +1) + 2;//指向下一个Field Type 
	                           }
                       	  }
                           if( i < 5 )//找到model
                           {
                               memcpy(cmodelvar, tmppoint, *(tmppoint - 1));
                           }

                           if((*ccategory == '\0') && (*ccategoryvar == '\0'))
                           {
                               cat = 1;
                           }
                           else if((*ccategory != '\0') && (*ccategoryvar != '\0'))
                           {
                               if(0 == strcmp(ccategory, ccategoryvar))
                               {
                                   cat = 1;
                               }
                           }

                           if((*cmodel == '\0') && (*cmodelvar == '\0'))
                           {
                               mod = 1;
                           }
                           else if((*cmodel != '\0') && (*cmodelvar != '\0'))
                           {
                               if(0 == strcmp(cmodel, cmodelvar))
                               {
                                   mod = 1;
                               }
                           }

                           for(i = 0; i < 5; i++)//最多有5个Field
                           {
	                           if(*vendorid == 1)//Type 1, vendor
	                           {
	                              vendorid += 2;
	                              printf("===>CHINA TELECOM Vendor found!\n");
	                              break;
	                           }
	                           else
	                           {
	                            vendorid =vendorid + *(vendorid +1) + 2;//指向下一个Field Type 
	                           }
                       	  }
                       }
                       #endif
	             
		          //这里替换server_config的start,end,lease字段,替换leases
		          for (i = 0; i < 5; i++)
		          {
		             //printf("===>dhcpd, before replace:ipPool[%d].option60=%s;vendorid len=%d; strlen(ipPool[%d].option60)=%d\n", 
		             //	 i, ipPool[i].option60, *(vendorid - 1), i, strlen(ipPool[i].option60));

		              if(NULL != vendorid && strlen(ipPool[i].option60) == *(vendorid - 1)  
		              	&& !memcmp(vendorid, ipPool[i].option60, *(vendorid - 1))
		              	&& (ntohl(ipPool[i].start) >= ntohl(master_start))
		              	#ifdef SUPPORT_CHINATELECOM_DHCP
		              	&& (ntohl(ipPool[i].end) <= ntohl(master_end))&&(1 == cat)&&(1 == mod))
		              	#else
		              	&& (ntohl(ipPool[i].end) <= ntohl(master_end)))
		              	#endif
		              {
				     //printf("===>dhcpd, use ip pool %d\n", i);

				     ismatch = 1;				     

		                  server_config.start = ipPool[i].start;
		                  server_config.end = ipPool[i].end;
		                  server_config.lease = ipPool[i].lease;
		                  //leases = ipPool[i].leases;
		                  if(0 != find_address(1))
		                  {
		                      //printf("===>dhcpd, vendorid is not null, found ip in pool %d!\n", i);
		                      ipPoolIndex = i;
		                      break;
		                  }
		              }
		          }

	                /*如果option60内容为其他值,就从缺省主地址池的其它地址分配*/
		          if(0 == ismatch) 
		          {
	                     //printf("===>dhcpd, vendorid is not match, use master ip!\n");
	                     inmaster = 1;
		               server_config.start = master_start;
		               server_config.end = master_end;
		               server_config.lease = master_lease;
		          }
	             }
                 }
        }
#endif
/*w44771 add for 第一IP支持5段地址池，end*/

        /*end DHCP Server支持第二地址池, s60000658, 20060616*/
		
		lease = find_lease_by_chaddr(packet.chaddr);
	// 在第二地址池所对应的租期域中查找。061208 add by c60023298
	if (!lease)
		{
		lease = find_lease_two_by_chaddr(packet.chaddr);
		}
	// c60023298 end
	    //暂不支持121
	        #if 0
        	/* BEGIN: Added by y67514, 2008/5/22 vista无法获取地址*/
        	RequestList = get_option(&packet, DHCP_PARAM_REQ);
              if(RequestList)
              {
                    req_num = *(RequestList -1);
                    for ( k = 0 ; k < req_num; k++ )
                    {
                        if(DHCP_STATIC_ROUTE == RequestList[k])
                        {
                            flag_option121 = 1;
                            break;
                        }
                    }
              }
        	/* END:   Added by y67514, 2008/5/22 */
             #else
             flag_option121 = 0;
             #endif
		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			/* j00100803 Add Begin 2008-07-21 for logging dhcp client report option */
			iLen = 0;
			pUserClassInfo = NULL;
			memset(szUserMacAddr, 0, sizeof(szUserMacAddr));
			memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			if(packet.htype == 1)
			{
			    bcmMacNumToStr(packet.chaddr, szUserMacAddr);
			}
			else
			{
			    memcpy(szUserMacAddr, packet.chaddr, 16);
			}
			/* j00100803 Add End 2008-07-21 for logging dhcp client report option */

			/* j00100803 Add Begin 2008-07-21 for dhcp option12 */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_HOST_NAME)))
			{
				/* receive a report from DHCP Client, save a log */
			    iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "D,Mac: %s, option12 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option12 */
			/* j00100803 Add Begin 2008-07-21 for dhcp option15 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_DOMAIN_NAME)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "D,Mac: %s, option15 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option15 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option50 for log*/
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_REQUESTED_IP)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
/*start of 修改AU8D00960问题单:对网关进行discover攻击，网关无法再分配ip地址 by l129990,2008,10,23*/
                //if((iLen > 0) && (iLen < 128))
                if(iLen != 4)
			    {
                    continue;
                }
                else
/*end of 修改AU8D00960问题单:对网关进行discover攻击，网关无法再分配ip地址 by l129990,2008,10,23*/
                {
                    /*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			    	//memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        //memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        memset(&addrOpt50, 0, sizeof(addrOpt50));
			        memcpy(&addrOpt50.s_addr, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "D,Mac: %s, option50 %s", szUserMacAddr, inet_ntoa(addrOpt50));
                    /*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option50 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option60 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_VENDOR)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "D,Mac: %s, option60 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option60 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option61 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_CLIENT_ID)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
                    /*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			        //memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        if ( 7 == iLen && 0x01 == *pUserClassInfo )
			        {
			            bcmOption61EthToStr(pUserClassInfo, szUserClassInfo);
			        }
                    else
                    {
                        memcpy(szUserClassInfo, pUserClassInfo, iLen);
                    }
                    /*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			        syslog(LOG_CRIT, "D,Mac: %s, option61 %s", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option61 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option77 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_USER_CLASS)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "D,Mac: %s, option77 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option77 for log */

			/*start: modify by xkf19988 2009.12.21*/
                     /*0～255，所以必须是unsigned型的*/
			unsigned char * temp = NULL;
			g_option66 = 0;
			g_option67 = 0;
			g_option160 = 0;
			if( NULL != ( temp = get_option(&packet,DHCP_PARAM_REQ)))
			{
				int temp_len = *(temp - 1);
				for( i=0;i<temp_len;i++ )
				{
					if( temp[i] ==  DHCP_TFTP_SERVER_NAME )
					{
						g_option66 = 1;
					}
					if( temp[i] == DHCP_BOOTFILE_NAME )
					{
						g_option67 = 1;
					}
					if( temp[i] == DHCP_OPTION160 )
					{
						g_option160 = 1;
					}
				}
			}
			/*end: modify by xkf19988 2009.12.21*/
			//BRCM_begin
			     /*start of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/			     
			     #ifdef SUPPORT_DHCP_VENDOR
                        vendorid = get_option(&packet, DHCP_VENDOR);

                        /* Check the vendor ID with the configured vendor ID */
                        if (read_vendor_id_config(DHCPD_VENDORID_CONF_FILE) == 1) {
                                vendor_id_cfg = 1;
                        }
                        if (vendor_id_cfg) {
				test_vendorid(&packet, vendorid, &declined);
                        }
			// BRCM_end

                        if (!declined) {
                        #endif
			    /*end of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/

				if (sendOffer(&packet) < 0) {
					LOG(LOG_ERR, "send OFFER failed -- ignoring");
				}
		     /*start of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/	
                   #ifdef SUPPORT_DHCP_VENDOR 
			}
			#endif
		     /*start of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/
		
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO,"received REQUEST");
			/* j00100803 Add Begin 2008-07-21 for logging dhcp client report option */
			iLen = 0;
			pUserClassInfo = NULL;
			memset(szUserMacAddr, 0, sizeof(szUserMacAddr));
			memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			if(packet.htype == 1)
			{
			    bcmMacNumToStr(packet.chaddr, szUserMacAddr);
			}
			else
			{
			    memcpy(szUserMacAddr, packet.chaddr, 16);
			}
			/* j00100803 Add End 2008-07-21 for logging dhcp client report option */
			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);
			hostname = get_option(&packet, DHCP_HOST_NAME);
			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
			/*start: modify by xkf19988 2009.12.21*/
			temp = NULL;
			g_option66= 0;
			g_option67 = 0;
			g_option160= 0;
			if( NULL != ( temp = get_option(&packet,DHCP_PARAM_REQ)))
			{
				int temp_len = *( temp - 1 );
				for( i=0;i<temp_len;i++)
				{
					if( temp[i] == DHCP_TFTP_SERVER_NAME )
						g_option66= 1;
					if( temp[i] == DHCP_BOOTFILE_NAME )
						g_option67= 1;
					if( temp[i] == DHCP_OPTION160 )
						g_option160= 1;
				}
			}
			/*end: modify by xkf19988 2009.12.21*/
			/* j00100803 Add Begin 2008-07-21 for dhcp option12 */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_HOST_NAME)))
			{
				/* receive a report from DHCP Client, save a log */
			    iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "R,Mac: %s, option12 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option12 */
			/* j00100803 Add Begin 2008-07-21 for dhcp option15 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_DOMAIN_NAME)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "R,Mac: %s, option15 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option15 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option50 for log*/
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_REQUESTED_IP)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
 /*start of 修改AU8D00960问题单:对网关进行discover攻击，网关无法再分配ip地址 by l129990,2008,10,23*/
			    //if((iLen > 0) && (iLen < 128))
                if(iLen != 4)
			    {
                    continue;
                }
                else
 /*end of 修改AU8D00960问题单:对网关进行discover攻击，网关无法再分配ip地址 by l129990,2008,10,23*/
                {
                    /*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			    	//memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        //memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        memset(&addrOpt50, 0, sizeof(addrOpt50));
			        memcpy(&addrOpt50.s_addr, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "R,Mac: %s, option50 %s", szUserMacAddr, inet_ntoa(addrOpt50));
                   /*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option50 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option60 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_VENDOR)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "R,Mac: %s, option60 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option60 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option61 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_CLIENT_ID)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
                    /*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			        //memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        if ( 7 == iLen && 0x01 == *pUserClassInfo )
			        {
			            bcmOption61EthToStr(pUserClassInfo, szUserClassInfo);
			        }
                    else
                    {
                        memcpy(szUserClassInfo, pUserClassInfo, iLen);
                    }
                    /*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
			        syslog(LOG_CRIT, "R,Mac: %s, option61 %s", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option61 for log */
			/* j00100803 Add Begin 2008-07-21 for dhcp option77 for log */
			if(NULL != (pUserClassInfo = get_option(&packet, DHCP_USER_CLASS)))
			{
				/* receive a report from DHCP Client, save a log */
				iLen = (int)(*(pUserClassInfo - 1));
			    if((iLen > 0) && (iLen < 128))
			    {
			    	memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "R,Mac: %s, option77 %s\n", szUserMacAddr, szUserClassInfo);
			    }
			}
			/* j00100803 Add End 2008-07-21 for dhcp option77 for log */
			//BRCM_begin
			     /*start of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/			     
			     #ifdef SUPPORT_DHCP_VENDOR
                        vendorid = get_option(&packet, DHCP_VENDOR);

                        /* Check the vendor ID with the configured vendor ID */
                        if (read_vendor_id_config(DHCPD_VENDORID_CONF_FILE) == 1) {
                                vendor_id_cfg = 1;
                        }
                        if (vendor_id_cfg) {
				test_vendorid(&packet, vendorid, &declined);
                        }
			// BRCM_end
			if (!declined) {
				#endif
				/*end of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/			     
				if (lease) {
					if (server_id) {
						/* SELECTING State */
						DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
						if (server_id_align == server_config.server && requested && 
						    requested_align == lease->yiaddr) {
/*start, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
						    #ifdef SUPPORT_MACMATCHIP 
						    if ( !ismacmatch(packet.chaddr) && (requested_align != find_matchip(packet.chaddr)) )
						    {
                                sendNAK(&packet);
						    }
                            else
                            {
							    sendACK(&packet, lease->yiaddr);
                            }
                            #else
							sendACK(&packet, lease->yiaddr);
						    #endif                           
/*end, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
						} 
/*start of 解决问题单AU8D01113:收到pc请求的IP与Offer报文中分配的Your IP不一致时不能回复Nak by l129990,2008,11,10*/
                        else 
                        {
                            sendNAK(&packet);
                        }
/*end of 解决问题单AU8D01113:收到pc请求的IP与Offer报文中分配的Your IP不一致时不能回复Nak by l129990,2008,11,10*/
					} else {
						if (requested) {
							/* INIT-REBOOT State */
/*start, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
                            #ifdef SUPPORT_MACMATCHIP
							if (lease->yiaddr == requested_align && 
                                !(!ismacmatch(packet.chaddr) && (requested_align != find_matchip(packet.chaddr))))
                            #else
							if (lease->yiaddr == requested_align)
                            #endif
/*end, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
								sendACK(&packet, lease->yiaddr);
							else sendNAK(&packet);
						} else {
							/* RENEWING or REBINDING State */
/*start, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
                            #ifdef SUPPORT_MACMATCHIP
							if (lease->yiaddr == packet.ciaddr && 
                                !(!ismacmatch(packet.chaddr) && (requested_align != find_matchip(packet.chaddr))))
                            #else
							if (lease->yiaddr == packet.ciaddr)
                            #endif
/*start, 修改特定ip分配给特定mac需要renew才能使配置生效的问题， chenweiqing 65985 08-05-07*/
								sendACK(&packet, lease->yiaddr);
							else {
								/* don't know what to do!!!! */
								sendNAK(&packet);
							}
						}						
					}
					if (hostname) {
						bytes = hostname[-1];
						if (bytes >= (int) sizeof(lease->hostname))
							bytes = sizeof(lease->hostname) - 1;
						strncpy(lease->hostname, hostname, bytes);
						lease->hostname[bytes] = '\0';
					} else
						lease->hostname[0] = '\0';
				} else { /* else remain silent */				
    					sendNAK(&packet);
            			}
	            /*start of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/			     
			#ifdef SUPPORT_DHCP_VENDOR
			}
			#endif
			/*end of DHCP 去掉dhcp vendor 功能 porting by w44771 20060505*/			     
			/* j00100803 Add Begin 2008-04-15 dhcp option77*/
			#ifdef SUPPORT_VDF_DHCP
			memset(szUserClassInfo, 0, sizeof(szUserClassInfo));
			memset(szUserMacAddr, 0, sizeof(szUserMacAddr));

			if(packet.htype == 1)
			{
			    bcmMacNumToStr(packet.chaddr, szUserMacAddr);
			}
			else
			{
			    memcpy(szUserMacAddr, packet.chaddr, 16);
			}
			pUserClassInfo = get_option(&packet, DHCP_USER_CLASS);

			if(pUserClassInfo)
			{
			    iLen = (int)(*(pUserClassInfo - 1));
			    if(iLen > 0)
			    {
			        memcpy(szUserClassInfo, pUserClassInfo, iLen);
			        syslog(LOG_CRIT, "Mac: %s, option77 %s\n", szUserMacAddr, szUserClassInfo);
                }
			}
			#endif
			/* j00100803 Add End 2008-04-15 dhcp option77*/
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
                            /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
				lease->expires = getSysUpTime() + server_config.decline_time;
                            /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
                     /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			if (lease) lease->expires = getSysUpTime();
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
        /* BEGIN: Added by y67514, 2008/5/22 */
        flag_option121 = 0;
        /* END:   Added by y67514, 2008/5/22 */
        /*start DHCP Server支持第二地址池, s60000658, 20060616*/
        vendorid = get_option(&packet, DHCP_VENDOR);
        if(g_enblSrv2 && NULL != server2_config.vendorid && NULL != vendorid
            && strlen(server2_config.vendorid) == *(vendorid - 1) && !memcmp(vendorid, server2_config.vendorid, *(vendorid - 1)))//w44771 modify for A36D02366
        {
            //恢复到缺省地址池和租用列表
            memcpy(&server2_config, &server_config, sizeof(struct server_config_t));
            memcpy(&server_config, &tmpSrvCfg, sizeof(struct server_config_t));
            for (i = 0; i < max_lease_in_both ; i++)
          	{
          	memcpy(&leases2[i], &leases[i], sizeof(struct dhcpOfferedAddr));
          	memcpy(&leases[i], &tmpls[i], sizeof(struct dhcpOfferedAddr));  
          	}
        }
        /*end DHCP Server支持第二地址池, s60000658, 20060616*/
	}
        //BRCM_BEGIN 
        if (server_socket > 0)
          close(server_socket);
	return 0;
}

