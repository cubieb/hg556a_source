/* dhcpd.h */
#ifndef _DHCPC_H
#define _DHCPC_H


#define INIT_SELECTING	0
#define REQUESTING	1
#define BOUND		2
#define RENEWING	3
#define REBINDING	4
#define INIT_REBOOT	5
#define RENEW_REQUESTED 6
#define RELEASED	7


/* Paramaters the client should request from the server */
#ifdef VDF_OPTION
//add by y67514 to support  option121,option121必须在option3的前面
#define PARM_REQUESTS \
	DHCP_STATIC_ROUTE,\			
	DHCP_SUBNET, \
	DHCP_ROUTER, \
	DHCP_DNS_SERVER, \
	DHCP_DOMAIN_NAME, \
	DHCP_ROOT_PATH, \
	DHCP_BROADCAST,\
        DHCP_TIME_OFFSET,\
        DHCP_TIME_SERVER,\
        DHCP_LOG_SERVER,\
        DHCP_NTP_SERVER,\
        DHCP_TFTP_SERVER_NAME,\
        DHCP_BOOTFILE_NAME,\
        DHCP_TR069_ACS
        //w44771 add for option43
	/*w44771 add for more dhcp optioins, 2006-7-7*/
	/*WAN <3.4.5桥使能dhcp, 自动升级, 添加了宏DHCP_ROOT_PATH> porting by s60000658 20060505*/
#else
#define PARM_REQUESTS \
	DHCP_SUBNET, \
	DHCP_ROUTER, \
	DHCP_DNS_SERVER, \
	DHCP_DOMAIN_NAME, \
	DHCP_ROOT_PATH, \
	DHCP_BROADCAST,\
        DHCP_TIME_OFFSET,\
        DHCP_TIME_SERVER,\
        DHCP_LOG_SERVER,\
        DHCP_NTP_SERVER,\
        DHCP_TFTP_SERVER_NAME,\
        DHCP_BOOTFILE_NAME,\
        DHCP_TR069_ACS
        //w44771 add for option43
	/*w44771 add for more dhcp optioins, 2006-7-7*/
	/*WAN <3.4.5桥使能dhcp, 自动升级, 添加了宏DHCP_ROOT_PATH> porting by s60000658 20060505*/

#endif

struct client_config_t {
	char foreground;		/* Do not fork */
	char quit_after_lease;		/* Quit after obtaining lease */
	char abort_if_no_lease;		/* Abort if no lease */
	char *interface;		/* The name of the interface to use (listen)*/
	/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
	char *Interface;		/* The name of the Interface to use (get ip)*/
	/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
	char *pidfile;			/* Optionally store the process ID */
	char *script;			/* User script to run at dhcp events */
	char *clientid;			/* Optional client id to use */
	char *hostname;			/* Optional hostname to use */
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */
};

extern struct client_config_t client_config;

// brcm
extern char vendor_class_id[];
/*start of 增加dhcp主机标识功能by l129990 */

#ifdef SUPPORT_DHCPHOSTID
#define DHCP_HOST_SIZE 65
#endif

/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
#define LOG_DISPLAY 1
/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

/*end of 增加dhcp主机标识功能by l129990 */

/*start of 支持option61上报内网设备标识 by l129990,2008,12,31*/
#ifdef SUPPORT_DHCPCLIENTID
#define OPTION61_INDEX 2
#define OPTION61_DATA_LEN 9
#define OPTION61_IAID_LEN 4
#endif
/*end of 支持option61上报内网设备标识 by l129990,2008,12,31*/
#endif
