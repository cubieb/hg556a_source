/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"

#ifdef SUPPORT_MACMATCHIP
extern _macip_list;
#endif

/*
 * Remove entry from ARP.
 * <tanyin 2009.3.19>
 */
int arp_remove(int ip_addr)
{
    int sockfd = 0;
    int sts = 0;
    struct arpreq req;
    struct sockaddr sa;
    struct sockaddr_in *addr = (struct sockaddr_in*)&sa;

    if (ip_addr != 0 ) {
        addr->sin_family = AF_INET;
        addr->sin_port = 0;
        addr->sin_addr.s_addr = htonl(ip_addr);
        printf("remove %s from ARP\n",inet_ntoa(addr->sin_addr));
        memcpy((char *)&req.arp_pa, (char *)&sa, sizeof(struct sockaddr));

        bzero((char*)&sa, sizeof(struct sockaddr));
        sa.sa_family = ARPHRD_ETHER;
        memcpy((char *)&req.arp_ha, (char *)&sa, sizeof(struct sockaddr));
        req.arp_flags = ATF_COM;
        req.arp_dev[0] = '\0';

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("DHCPD:Error in create socket\n");
            sts = -1;
        } else {
            if (ioctl(sockfd, SIOCDARP, &req) < 0) {
                printf("DHCPD:Error in SIOCDARP\n");
                sts = -1;
            }
        }
        if ( sockfd > 0 )
            close(sockfd);
    } else {
       printf("DHCPD:Invalid IP address\n");
       sts = -1;
    }

    return sts;

}

/*
 * Update lease table once one of the LAN host leaves.
 * <tanyin 2009.3.13>
 */
void update_lease(int dumy)
{
    u_int32_t i = 0;

    (void)dumy; /* reserved */
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
                if (!check_ip_and_add(leases[i].chaddr,leases[i].yiaddr)) {
                    printf("DHCPD: %s not assigned to host, reclaim it\n",
                        inet_ntoa(*((struct in_addr*)(&leases[i].yiaddr))));

                    /* Remove entry from ARP.*/
                    arp_remove(leases[i].yiaddr);
                    /* Remove entry from lease. <tanyin 2009.5.22>*/
                    //clear_lease(leases[i].chaddr, leases[i].yiaddr);
                    /* Just mark this lease expired so that it can be
                       offered to the same PC if it request again.
                       This always happen when PC shutdown and restart.
                       <tanyin 2009.5.22>
                    */
                    expire_lease(leases[i].chaddr, leases[i].yiaddr);

                }
		}
	}
}

/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, blank_chaddr = 0, blank_yiaddr = 0;
    int j = 0;
	
	for (i = 0; i < 16 && !chaddr[i]; i++);
	if (i == 16) blank_chaddr = 1;
	blank_yiaddr = (yiaddr == 0);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((!blank_chaddr && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (!blank_yiaddr && leases[i].yiaddr == yiaddr)) 
        {		
            /* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
            for (  j = 0 ;  j < g_enblDomainTab;  j++ )
            {
                if ( !memcmp(leases[i].chaddr,domains[j].chaddr,16) )
                {
                    /*mac匹配*/
                    domains[j].yiaddr = 0;
                }
            }
            /* END:   Added by y67514, 2008/9/18 */
            memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}

/*
 * mark a lease expired. <tanyin 2009.5.22>
 */
void expire_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, blank_chaddr = 0, blank_yiaddr = 0;
    int j = 0;
	
	for (i = 0; i < 16 && !chaddr[i]; i++);
	if (i == 16) blank_chaddr = 1;
	blank_yiaddr = (yiaddr == 0);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((!blank_chaddr && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (!blank_yiaddr && leases[i].yiaddr == yiaddr)) 
        {
            leases[i].expires = getSysUpTime();
		}
}


/* add a lease into the table, clearing out any old ones */
#ifdef SUPPORT_CHINATELECOM_DHCP
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, unsigned int port)
#else
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
#endif
{
	struct dhcpOfferedAddr *oldest;
    /* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
    int i = 0;
    /* END:   Added by y67514, 2008/9/18 */
	
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);
		
	oldest = oldest_expired_lease();
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		oldest->expires = getSysUpTime() + lease;
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
             #ifdef SUPPORT_CHINATELECOM_DHCP
             oldest->port = port;
             #endif
             /* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
             for (  i = 0 ;  i < g_enblDomainTab;  i++ )
             {
                 if ( !memcmp(chaddr,domains[i].chaddr,16) )
                 {
                    /*mac匹配*/
                     if ( yiaddr != domains[i].yiaddr)
                     {
                        /*IP变化时才通知dnsmasq*/
                         domains[i].yiaddr = yiaddr;
                        if ( 0 == g_needToChang)
                        {
                            /*15秒后才通知dns，避免一段时间内获取IP的主机数
                              太多造成通知频率太高*/
                            g_needToChang = 1;
                            alarm(15);
                        }
                     }
                 }
             }
             /* END:   Added by y67514, 2008/9/18 */
	}
	
	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	return (lease->expires < (unsigned long) getSysUpTime());
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
}	


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	unsigned long oldest_lease = getSysUpTime();
    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}
//061208 add by c60023298
struct dhcpOfferedAddr *find_lease_two_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server2_config.max_leases; i++)
		if (!memcmp(leases2[i].chaddr, chaddr, 16)) return &(leases2[i]);
	
	return NULL;
}
//end


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}
#ifdef	VDF_RESERVED
//判断是否为保留IP
u_int32_t reservedIp(uint32_t ip)
{
	int i;
	for(i = 0;(i < MAXRESERVEDIP)&&(0 != server_config.reservedAddr[i]);i++)
	{
		if(server_config.reservedAddr[i]== ip)
		{
			return 0;	//IP是保留IP
		}
	}
	return 1;	//不是保留IP
}

#endif

#ifdef SUPPORT_MACMATCHIP
int ismacmatch(u_int8_t *mac)
{   
    PMACIP_LIST current = NULL;

    if ( NULL == mac )
    {
        return 1;
    }
 
    for ( current = _macip_list; current != NULL; current = current->next ) 
    {
        if ( memcmp(mac, &current->macip.mac, 16) == 0 ) 
        {
            return 0;
        }      
    }
    
    return 1;
}

int isipmatch(u_int32_t *ip)
{
    PMACIP_LIST current = NULL;

    if ( NULL == ip )
    {
        return 1;
    }
 
    for ( current = _macip_list; current != NULL; current = current->next ) 
    {
        if ( ip == current->macip.ip ) 
        {
            return 0;
        }
    }
  
    return 1;
}

int find_matchip(u_int8_t *mac)
{
    PMACIP_LIST current = NULL;
    u_int32_t   ipaddr;
        
    if ( NULL == mac )
    {       
        return 1;
    }
 
    for( current = _macip_list; current != NULL; current = current->next ) 
    {
        if ( memcmp(mac, &current->macip.mac, 16) == 0 ) 
        {         
            ipaddr = current->macip.ip;
            return ipaddr;
        }
    }

    return 1;
}
#endif



/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
u_int32_t find_address(int check_expired) 
{
	u_int32_t addr, ret = 0;
	struct dhcpOfferedAddr *lease = NULL;	
	/*start A36D02806, s60000658, 20060906*/
	struct option_set *opt = NULL;
	u_int32_t router_ip = 0;

    if(NULL != (opt = find_option(server_config.options, DHCP_ROUTER)))
    {
        router_ip = *(u_int32_t*)(opt->data + 2);
    }
	/*end A36D02806, s60000658, 20060906*/
	addr = server_config.start;
	// brcm
	for (;ntohl(addr) <= ntohl(server_config.end) ;addr = htonl(ntohl(addr) + 1)) {

		/* ie, 192.168.55.0 */
		if (!(ntohl(addr) & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((ntohl(addr) & 0xFF) == 0xFF) continue;
		#ifdef	VDF_RESERVED
		//如果是保留IP就继续看下一个
		if(!reservedIp(addr))
		{
			continue;
		}
		#endif

        #ifdef SUPPORT_MACMATCHIP
        if ( !isipmatch(addr) )
        {
			continue;
		}
        #endif

    		 /*start of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/
	 	if ((addr == server_config.server) 
	 	 || (addr == router_ip)) /*A36D02806, s60000658, 20060906*/
		{
		    continue;
		}
		/*end of DHCP 网关会分配自己的维护IP porting by w44771 20060505*/

		/* lease is not taken */
		if ((!(lease = find_lease_by_yiaddr(addr)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(addr)) {
			ret = addr;
			break;
		}
	}
	return ret;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)
{
	char blank_chaddr[] = {[0 ... 15] = 0};
	struct in_addr temp;

    if(0 != read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp))
			   printf("DHCPD: cannot get mac of interface %s\n", server_config.interface);

	if (!arpping(addr, server_config.server, server_config.arp, server_config.interface)) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);
		/*start of DHCP 不必添加到lease列表中 porting by w44771 20060505*/
		/*add_lease(blank_chaddr, addr, server_config.conflict_time);*/
		/*end of DHCP  不必添加到lease列表中 porting by w44771 20060505*/
		return 1;
	} else return 0;
}

/* check is an IP is taken, if it is, add it to the lease table */
int check_ip_and_add(u_int8_t *chaddr, u_int32_t addr)
{
	struct in_addr temp;

    if(0 != read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp))
			   printf("DHCPD: cannot get mac of interface %s\n", server_config.interface);

	if (!arpping(addr, server_config.server, server_config.arp, server_config.interface)) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);

        /* Add this one */
        if (find_lease_by_yiaddr(addr) == NULL) {
    		add_lease(chaddr, addr, server_config.conflict_time);
        }

		return 1;
	} else return 0;
}

/*w44771 add:此函数用来查找在主地址池且不在所有子地址池的ip*/
#ifdef SUPPORT_DHCP_FRAG
u_int32_t find_address2(int check_expired) 
{
	u_int32_t addr, ret = 0;
	struct dhcpOfferedAddr *lease = NULL;	
	addr = master_start;
	int i = 0, insubnet = 0;
	
	for (;ntohl(addr) <= ntohl(master_end) ;addr = htonl(ntohl(addr) + 1)) {
	 	if (addr == server_config.server)
		{
		    continue;
		}
		#ifdef	VDF_RESERVED
		//如果是保留IP就继续看下一个
		if(!reservedIp(addr))
		{
			continue;
		}
		#endif
        
        #ifdef SUPPORT_MACMATCHIP
        if ( !isipmatch(addr) )
        {
			continue;
		}
        #endif
        
	 	for(i = 0; i < 5; i++)
	 	{
	 	    if((ntohl(addr) >= ntohl(ipPool[i].start))&&(ntohl(addr) <= ntohl(ipPool[i].end)))
	 	    {
	 	       insubnet = 1;
	 	       break;
	 	    }
	 	}

	 	if(1 == insubnet)
	 	{
	 	    insubnet = 0;
	 	    continue;
	 	}

		/* lease is not taken */
		if ((!(lease = find_lease_by_yiaddr(addr)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(addr)) {
			ret = addr;
			break;
		}
	}
	return ret;
}
#endif

