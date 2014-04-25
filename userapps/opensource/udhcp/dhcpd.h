/* dhcpd.h */
#ifndef _DHCPD_H
#define _DHCPD_H

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "leases.h"

#ifdef VDF_OPTION
	#ifndef SUPPORT_OPTION61
	#define SUPPORT_OPTION61
	#endif	
#endif

/************************************/
/* Defaults _you_ may want to tweak */
/************************************/

/* the period of time the client is allowed to use that address */
#define LEASE_TIME              (60*60*24*10) /* 10 days of seconds */

/* where to find the DHCP server configuration file */
#define DHCPD_CONF_FILE         "/etc/udhcpd.conf"

// BRCM
/* where to find the DHCP vendor IDs configuration file */
#define DHCPD_VENDORID_CONF_FILE         "/var/udhcpd_vendorid.conf"

#ifdef	VDF_RESERVED
#define MAXRESERVEDIP 8
#endif


/*****************************************************************/
/* Do not modify below here unless you know what you are doing!! */
/*****************************************************************/

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT		67
#define CLIENT_PORT		68

#define DHCP_MAGIC		0x63825363

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_TR069_ACS        0x2b//w44771 add for option43
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
/*w44771 add for more dhcp optioins, begin, 2006-7-7*/
#define DHCP_TFTP_SERVER_NAME		0x42
#define DHCP_BOOTFILE_NAME		0x43
/*w44771 add for more dhcp optioins, end, 2006-7-7*/
/*START:MODIFY BY XKF19988 2009.12.21*/
#define DHCP_OPTION160	0xA0
/*end: modify by xkf19988 2009.12.21*/
/* j00100803 Add Begin 2008-03-10 */
#ifdef SUPPORT_VDF_DHCP
#define DHCP_USER_CLASS         0x4D    // option77
#endif
/* j00100803 Add End 2008-03-10 */
#ifdef VDF_OPTION
#define DHCP_STATIC_ROUTE	0x79		//option121
#define DHCP_VENDOR_IDENTIFYING	0x7d     //add for option125
#endif


#define DHCP_END		0xFF

#ifdef VDF_OPTION
/*start of VDF 2008.4.17 V100R001C02B013 j00100803 AU8D00535 */
#define VDF_VERNDOR	"dslforum.org"
/*end of VDF 2008.4.17 V100R001C02B013 j00100803 AU8D00535 */
#endif

#define BOOTREQUEST		1
#define BOOTREPLY		2

#define ETH_10MB		1
#define ETH_10MB_LEN		6

#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8
#ifdef VDF_FORCERENEW
//Begin of add by y67514 to support forcerenew
#define DHCPFORCERENEW	9
//End of add by y67514 to support forcerenew
#endif

#define BROADCAST_FLAG		0x8000

#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

/* miscellaneous defines */
#define TRUE			1
#define FALSE			0
#define MAC_BCAST_ADDR		"\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_VALUE 2

#ifdef SUPPORT_OPTION61
#define OPT61_HWTYPE 2
#define OPT61_MACADDR 3
#define OPT61_DATA_LENGTH 7
#endif

// BRCM
#define MAX_VENDOR_IDS		80

struct option_set {
	unsigned char *data;
	struct option_set *next;
};

#ifdef SUPPORT_MACMATCHIP
typedef struct macip_entry
{
    u_int8_t    mac[16];
    u_int32_t   ip;    
}MACIP_ENTRY,*PMACIP_ENTRY;

typedef struct macip_list
{
    MACIP_ENTRY  macip;
    PMACIP_ENTRY next;
}MACIP_LIST,*PMACIP_LIST;
#endif

struct server_config_t {
	u_int32_t server;		/* Our IP, in network order */
	u_int32_t start;		/* Start address of leases, network order */
	u_int32_t end;			/* End of leases, network order */
	#ifdef	VDF_RESERVED
	u_int32_t reservedAddr[MAXRESERVEDIP];
	#endif
    #ifdef SUPPORT_MACMATCHIP
    PMACIP_ENTRY  macip;
    #endif
	struct option_set *options;	/* List of DHCP options loaded from the config file */
	char *interface;		/* The name of the interface to use */
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */
	unsigned long lease;		/* lease time in seconds (host order) */
	unsigned long max_leases; 	/* maximum number of leases (including reserved address) */
	char remaining; 		/* should the lease file be interpreted as lease time remaining, or
			 		 * as the time the lease expires */
	unsigned long auto_time; 	/* how long should udhcpd wait before writing a config file.
					 * if this is zero, it will only write one on SIGUSR1 */
	unsigned long decline_time; 	/* how long an address is reserved if a client returns a
				    	 * decline message */
	unsigned long conflict_time; 	/* how long an arp conflict offender is leased for */
	unsigned long offer_time; 	/* how long an offered address is reserved */
	unsigned long min_lease; 	/* minimum lease a client can request*/
	char *lease_file;
	char *pidfile;
	char *notify_file;		/* What to run whenever leases are written */
	u_int32_t siaddr;		/* next server bootp option */
	char *sname;			/* bootp server name */
	char *boot_file;		/* bootp boot file option */
	// BRCM vendirid and decline_file
        char *vendorid;
        char *decline_file;
	 /*start: modify by xkf19988 2009.12.22*/
	char * tftpserver;
	char * bootfilename;
	char * option160;
	/*end:modify by xkf19988 2009.12.22*/
    #ifdef SUPPORT_PORTMAPING
        unsigned char *option60;
    #endif
};	

// BRCM
struct vendor_id_config_t {
    char vendorid[64];
};

/* BEGIN: Added by y67514, 2008/5/22 vista无法获取地址*/
extern int flag_option121;
/* END:   Added by y67514, 2008/5/22 */

/* BEGIN: Added by y67514, 2008/9/27   PN:GLB:支持Option15*/
extern char option_15[];
/* END:   Added by y67514, 2008/9/27 */

/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
struct ipPoolData
{
	u_int32_t start;		/* Start address of leases, network order */
	u_int32_t end;			/* End of leases, network order */
	unsigned long lease;		/* lease time in seconds (host order) */
	char option60[64];
      //struct dhcpOfferedAddr leases[254];
};

extern struct ipPoolData ipPool[5];
extern int ipPoolIndex;
extern u_int32_t master_start;		/* Start address of leases, network order */
extern u_int32_t master_end;			/* End of leases, network order */
unsigned long master_lease;
extern int inmaster;/*== 1 表示是在主地址池中分配*/

#ifdef SUPPORT_CHINATELECOM_DHCP
struct cntelOption60
{
    char vendor[33];
    char category[33];
    char model[33];
    char version[33];
    unsigned int protocol;//电信规定只用2个字节，这里统一按4个字节处理
    unsigned int port;
};

struct cntelOption43
{
   char configVersion[5];
};

extern struct cntelOption60 cOption60;
extern struct cntelOption43 cOption43;
extern char cOption60Cont[256];//存放整个option60

extern char c60common[256];//存放整理后的option60
extern char ccategory[32];
extern char cmodel[32];
#endif

#endif
#ifdef VDF_OPTION
//add for option125
/* vendor identifying option */
typedef struct vi_option_info {
  u_int32_t enterprise;
  char *oui;
  char *serialNumber;
  char *productClass;
  u_int32_t ipAddr;
  struct vi_option_info *next;
} VI_OPTION_INFO, *pVI_OPTION_INFO;

typedef struct viInfoList 
{
  int count;
  pVI_OPTION_INFO pHead;
  pVI_OPTION_INFO pTail;
} VI_INFO_LIST, *pVI_INFO_LIST;
extern pVI_INFO_LIST viList;
#endif
/*w44771 add for 第一IP支持5段地址池，end*/

/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
extern struct domainTab domains[32];
extern int g_enblDomainTab;        /*域名IP对应的规则数量*/
extern int g_needToChang;          /*是否需要通知dns*/
extern int g_dnsPid;                      /*dnsmasq的pid*/
/* END:   Added by y67514, 2008/9/18 */
extern struct server_config_t server_config;
extern struct dhcpOfferedAddr *leases;
/*start DHCP Server支持第二地址池, s60000658, 20060616*/
extern struct server_config_t server2_config;
extern struct dhcpOfferedAddr *leases2;
extern int g_enblSrv1;
extern int g_enblSrv2;
/*end DHCP Server支持第二地址池, s60000658, 20060616*/
// BRCM
extern struct dhcpOfferedAddr *declines;
extern struct vendor_id_config_t vendor_id_config[MAX_VENDOR_IDS];
extern g_option66;
extern g_option67;
extern g_option160;
int glb_isDhcpc;            /*1表示dhcpc，0表示dhcpd*/
		

#endif
