/* dhcpd.c
 *
 * udhcp DHCP client
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
 
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"

// brcm
#include "board_api.h"

#ifdef VDF_OPTION
	#define MAX_SERIAL_NUM_LEN	32
#endif


static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
static unsigned long subnet_addr;
char server_ip[16];
char req_ip[16];
/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */

static long timeout;//add by yp 2008-06-12
static int packet_num; /* = 0 */
//w44771 add for A36D02465, begin, 2006-8-14
static int discover_times; /* = 0 */
//w44771 add for A36D02465, end, 2006-8-14

// brcm
char session_path[64];
static char status_path[128]="";
static char pid_path[128]="";
char vendor_class_id[128]="";
/* BEGIN: Added by y67514, 2008/9/27   PN:GLB:支持Option15*/
#define DOMAIN_LEN      40
#define DOMAIN_FILE     "/var/domain_name"
char option_15[64] = "";        /*domainName*/
/* END:   Added by y67514, 2008/9/27 */

#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode = LISTEN_RAW;
// brcm
static int old_mode = LISTEN_RAW;
#define INIT_TIMEOUT 5
#define REQ_TIMEOUT 4

#define DEFAULT_SCRIPT	"/etc/dhcp/dhcp_getdata"

/*w44771 add for 防止DHCP浪涌,begin, 2006-7-7*/
#define SUPPORT_DHCP_SURGE
/*w44771 add for 防止DHCP浪涌,end, 2006-7-7*/

struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
	foreground: 0,
	quit_after_lease: 0,
	interface: "eth0",
	Interface: "eth0",  	/*WAN <3.4.5桥使能dhcp> add by s60000658 20060505*/
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
	hostname: NULL,
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
};

// brcm
void setStatus(int status) {
    char cmd[128] = "";
    int f;
    static int oldStatus  = -1;
    sprintf(cmd, "echo %d > %s", status, status_path);
    system(cmd); 

    /*start of  2008.05.10 HG553V100R001C02B016 AU8D00566 */
    if(oldStatus == status)
    {
        return;
    }
    oldStatus = status;
    /*end of  2008.05.10 HG553V100R001C02B016 AU8D00566 */
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    #if LOG_DISPLAY
	if (1 == status)
	{
		syslog(LOG_INFO, "udhcp client up on interface %s", session_path);  
	}
    #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
    
    if( (f = open( "/dev/brcmboard", O_RDWR )) != -1 )
    {

		syslog(LOG_INFO, "udhcp trigger monitor up");  

	
        ioctl( f, BOARD_IOCTL_WAKEUP_MONITOR_TASK, NULL);
        close(f);
    }
}

void setPid() {
    char cmd[128] = "";
    
    sprintf(cmd, "echo %d > %s", getpid(), pid_path);
    system(cmd); 
}

static void print_usage(void)
{
	printf(
"Usage: udhcpcd [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
"  -m,--mode                atm simple bridge or ptm simple bridge or others\n" //w44771 add for simple bridge
	);
}


/* SIGUSR1 handler (renew) */
static void renew_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR1");

	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    #if LOG_DISPLAY
    syslog(LOG_INFO, "Received SIGUSR1 on interface %s", session_path);  
    #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	
	/*
	if (state == BOUND || state == RENEWING || state == REBINDING ||
	    state == RELEASED) {
	    */
	if (state == BOUND || state == RENEWING || state == REBINDING)
	{
            listen_mode = LISTEN_KERNEL;
            //SERVER_addr不需要清空，serveraddr的加入或更新只在offer报文的处理中进行
            //server_addr = 0;
            packet_num = 0;
            state = RENEW_REQUESTED;
	}

	if (state == RELEASED) {
		listen_mode = LISTEN_RAW;
		state = INIT_SELECTING;
		packet_num = 0;
	}

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* SIGUSR2 handler (release) */
static void release_requested(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGUSR2");
	
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    #if LOG_DISPLAY
    syslog(LOG_INFO, "Received SIGUSR2 on interface %s", session_path);  
    #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	
	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");
	}

	listen_mode = 0;
	state = RELEASED;
	timeout = 0x7fffffff;
}

/*start of 增加dhcp主机标识即时生效功能by l129990 */
 #ifdef SUPPORT_DHCPHOSTID
static void alert_hostname(int sig)
{
    char hostName[DHCP_HOST_SIZE];
	FILE *fp = NULL;
	sig = 0;
	int len=0;
	
	LOG(LOG_INFO, "Received SIGILL");

	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
     #if LOG_DISPLAY
     syslog(LOG_INFO, "Received SIGILL on interface %s", session_path);  
     #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	 
	fp=fopen("/var/dhcphostid","r");
	if (fp) 
	{
		if (( NULL != fgets(hostName, DHCP_HOST_SIZE, fp)))
        {
			hostName[DHCP_HOST_SIZE - 1] = '\0';
			len = strlen(hostName);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = malloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, hostName, len);
        }
	}
	else
	{
        return;
	}

	fclose(fp);
}
#endif

/*end of 增加dhcp主机标识即时生效功能by l129990 */

/*start of 解决问题单AU8D01168:分配IP地址后受到nak轰击导致不能续租问题by l129990 2008,11,8*/
void alarm_handle(int sig)   
{   
    ;
}   
/*end of 解决问题单AU8D01168:分配IP地址后受到nak轰击导致不能续租问题by l129990 2008,11,8*/

/* Exit and cleanup */
static void exit_client(int retval)
{
	pidfile_delete(client_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void terminate(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
    #if LOG_DISPLAY
    syslog(LOG_INFO, "Received SIGTERM on interface %s", session_path);  
    #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
	exit_client(0);
}


static void background(void)
{
	int pid_fd;
	if (client_config.quit_after_lease) {
		exit_client(0);
	} else if (!client_config.foreground) {
		pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
		switch(fork()) {
		case -1:
			perror("fork");
			exit_client(1);
			/*NOTREACHED*/
		case 0:
			// brcm
			setPid();
			break; /* child continues */
		default:
			exit(0); /* parent exits */
			/*NOTREACHED*/
		}
		close(0);
		close(1);
		close(2);
		setsid();
		client_config.foreground = 1; /* Do not fork again. */
		pidfile_write_release(pid_fd);
	}
}

/* j00100803 Add Begin 2008-03-07 */
#if  0 // 暂时屏蔽此功能
typedef enum
{
    TIMER_ENUM_NORMAL,
    TIMER_ENUM_RENEWING,
    TIMER_ENUM_REBINDING
}TimerEnumType;

typedef enum
{
    TIMER_ENUM_RUN,
    TIMER_ENUM_STOP
}TimerEnumState;

int ExecCommandToConfigSNTP(char * pSrvAddr1, char * pSrvAddr2)
{
    char cmd[128];
    memset(cmd, 0, 128);
    if(NULL == pSrvAddr1)
    {
        return -1;
    }
    
    if(NULL == pSrvAddr2)
    {
        sprintf(cmd, "sntp -s %s\n", pSrvAddr1);
    }
    else
    {
        sprintf(cmd, "sntp -s %s -s %s\n", pSrvAddr1, pSrvAddr2);
    }
    system(cmd);
    return 0;
}

#endif
/* j00100803 Add End 2008-03-07 */

/* BEGIN: Added by y67514, 2008/9/27   PN:GLB:支持Option15*/
/*****************************************************************************
 函 数 名  : getDomainName
 功能描述  : 获取网关域名
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月27日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
void getDomainName( void )
{
    FILE *fp = NULL;
    char domainName[DOMAIN_LEN];
    char * tmp = NULL;

    memset(option_15,0,sizeof(option_15));
    memset(domainName,0,sizeof(domainName));
    if ( fp = fopen(DOMAIN_FILE,"r") )
    {
        if ( fgets(domainName,DOMAIN_LEN,fp) )
        {
            if ( tmp = strchr(domainName,'\n') )
            {
                *tmp = '\0';
            }
            option_15[OPT_CODE] = DHCP_DOMAIN_NAME;
            option_15[OPT_LEN] = strlen(domainName);
            strncpy(option_15 + OPT_VALUE,domainName,strlen(domainName));
        }
        fclose(fp);
    }
}

/* END:   Added by y67514, 2008/9/27 */

#ifdef COMBINED_BINARY
int udhcpc(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	char *temp, *message ;
       /* BEGIN: Added by y67514, 2008/9/28   PN:GLB:支持Option15*/
       char *option56 = NULL;
       /* END:   Added by y67514, 2008/9/28 */
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	char *log_subnet;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
       glb_isDhcpc = 1;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int fd, retval;
	struct timeval tv;
	int c, len;
	struct ifreq ifr;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
	struct in_addr tempsec_addr;
	struct in_addr tempthird_addr;
	struct in_addr tempfour_addr;
	/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
    /*start of 支持option61上报内网设备标识 by l129990,2008,12,31*/
    #ifdef SUPPORT_DHCPCLIENTID
    int serialNumber;
    #endif
    /*end of 支持option61上报内网设备标识 by l129990,2008,12,31*/
	int pid_fd;
   struct sigaction sa;
	/*w44771 add for 防止DHCP浪涌和169网段ip,begin, 2006-7-7*/
	char cLastMac[24];
	int iTimeRemain, iSeed;
	int dhcp_surge = 0;//w44771 add for simple bridge
	/*w44771 add for 防止DHCP浪涌和169网段ip,end, 2006-7-7*/
	/* BEGIN: Added by y67514, 2008/2/18   PN:AU8D00223*/
	FILE *classidfp;
	/* END:   Added by y67514, 2008/2/18 */
	/* j00100803 Add Begin 2008-03-07 */
	#if 0 //暂时屏蔽此特性
	static int iRenewingTimerOption58 = 0;
	static int iRebindingTimerOption59 = 0;
	TimerEnumType enTimerType = TIMER_ENUM_NORMAL;
	TimerEnumState enTimerOption58 = TIMER_ENUM_STOP;
	TimerEnumState enTimerOption59 = TIMER_ENUM_STOP;
	#endif
	/* j00100803 Add End 2008-03-07 */
	static struct option options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"foreground",	no_argument,		0, 'f'},
		{"hostname",	required_argument,	0, 'H'},
		{"help",	no_argument,		0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		{"Interface",	required_argument,	0, 'I'},
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
		{"version",	no_argument,		0, 'v'},
		{0, 0, 0, 0}
	};

	/* get options */
	while (1) {
		int option_index = 0;
// brcm
		/* BEGIN: Modified by y67514, 2008/2/18   PN:AU8D00223*/
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		//c = getopt_long(argc, argv, "c:fH:hi:np:qr:s:d:v", options, &option_index);
		//c = getopt_long(argc, argv, "c:fH:hi:I:np:qr:s:d:vm", options, &option_index);//w44771 add for simple bridge
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		c = getopt_long(argc, argv, "c:fH:hi:I:np:qr:s:dvm", options, &option_index);
		/* END:   Modified by y67514, 2008/2/18 */
		
		
		if (c == -1) break;
		
		switch (c) {
		case 'c':
            #if 0
            len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = malloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			strncpy(client_config.clientid + 2, optarg, len);
			break;
            #endif
            /*start of 支持option61上报内网设备标识 by l129990,2008,12,31*/
            #ifdef SUPPORT_DHCPCLIENTID
            serialNumber = atoi(optarg);
			break;
            #endif
            /*end of 支持option61上报内网设备标识 by l129990,2008,12,31*/
		case 'f':
			client_config.foreground = 1;
			break;
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = malloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
			break;
		case 'h':
			print_usage();
			return 0;
		case 'i':
			client_config.interface =  optarg;
			client_config.Interface =  optarg; /*WAN <3.4.5桥使能dhcp> add by s60000658 20060505*/
// brcm
			strcpy(session_path, optarg);
			break;
		/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		case 'I':
			client_config.Interface =  optarg;
			strcpy(session_path, optarg);
			break;
		/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
// brcm
		case 'd':
			/* BEGIN: Modified by y67514, 2008/2/18   PN:AU8D00223*/
			/*strcpy(vendor_class_id, optarg);  */
			
			classidfp = fopen("/var/dhcpClassIdentifier","r");
			if ( NULL !=  classidfp)
			{
			    fgets(vendor_class_id,128,classidfp);
			    printf("VDF:%s:%s:%d:vendor_class_id=%s***\n",__FILE__,__FUNCTION__,__LINE__,vendor_class_id);
			    fclose(classidfp);
			}
			/* END:   Modified by y67514, 2008/2/18 */
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			break;
		case 'm'://w44771 add for simple bridge
		         dhcp_surge = 1;
		         break;
		}
	}
        /* BEGIN: Added by y67514, 2008/9/27   PN:GLB:支持Option15*/
        getDomainName();
        /* END:   Added by y67514, 2008/9/27 */
#if 0
#ifdef VDF_OPTION
        /* BEGIN: Added by y67514, 2008/7/17   问题单号:将获取serial num的地方放在进程初始的时候，不用每次读*/
        int i = 0;
        char serial[MAX_SERIAL_NUM_LEN+1];
        if (client_config.clientid) 
        {
            free(client_config.clientid);
        }

        memset(serial,0,sizeof(serial));
        while(!strlen(serial))
        {
            boardIoctl(BOARD_IOCTL_EQUIPMENT_TEST, GET_SERIAL_NUMBER, serial, MAX_SERIAL_NUM_LEN+1, 0, "");
            if(i++ > 2)
            {
                printf("ERRO:DHCPC %d can't get the SN!\n",getpid());
                break;
            }
            sleep(3);
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
            }
        }
        /* END:   Added by y67514, 2008/7/17 */
#endif
#endif
	// brcm
        if (strlen(session_path) > 0) {
	    sprintf(status_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_MSG);
	    sprintf(pid_path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_PID);
	}

	OPEN_LOG("udhcpc");
	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);
	
	/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
     #if LOG_DISPLAY
     syslog(LOG_INFO, "udhcp client (v%s) started on interface %s", VERSION , session_path);  
     #endif
    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		strcpy(ifr.ifr_name, client_config.interface);
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			client_config.ifindex = ifr.ifr_ifindex;
		} else {
			LOG(LOG_ERR, "SIOCGIFINDEX failed! %s", strerror(errno));
			exit_client(1);
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(client_config.arp, ifr.ifr_hwaddr.sa_data, 6);
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				client_config.arp[0], client_config.arp[1], client_config.arp[2], 
				client_config.arp[3], client_config.arp[4], client_config.arp[5]);
		} else {
			LOG(LOG_ERR, "SIOCGIFHWADDR failed! %s", strerror(errno));
			exit_client(1);
		}
	} else {
		LOG(LOG_ERR, "socket failed! %s", strerror(errno));
		exit_client(1);
	}
	close(fd);
	fd = -1;

	/* setup signal handlers */
	signal(SIGUSR1, renew_requested);
	signal(SIGUSR2, release_requested);
    signal(SIGTERM, terminate);
    /*start of 增加dhcp主机标识即时生效功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
    signal(SIGILL, alert_hostname);
#endif
    /*end of 增加dhcp主机标识即时生效功能by l129990 */

/*start of 解决问题单AU8D01168:分配IP地址后受到nak轰击导致不能续租问题by l129990 2008,11,8*/
		sa.sa_handler = alarm_handle;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGALRM, &sa, NULL);
 /*end of 解决问题单AU8D01168:分配IP地址后受到nak轰击导致不能续租问题by l129990 2008,11,8*/

 /*start of 支持option61上报内网设备标识 by l129990,2008,12,31*/
    #ifdef SUPPORT_DHCPCLIENTID
    if (client_config.clientid) free(client_config.clientid);
    client_config.clientid = malloc(OPTION61_INDEX + OPTION61_DATA_LEN);
	client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
	client_config.clientid[OPT_LEN] = OPTION61_DATA_LEN;
    client_config.clientid[OPTION61_INDEX] = 0xff;
    *( (int *)(client_config.clientid + OPTION61_INDEX + 1) ) = serialNumber;
    client_config.clientid[OPTION61_INDEX + OPTION61_IAID_LEN + 1] = client_config.arp[2];
    client_config.clientid[OPTION61_INDEX + OPTION61_IAID_LEN + 2] = client_config.arp[3];
    client_config.clientid[OPTION61_INDEX + OPTION61_IAID_LEN + 3] = client_config.arp[4];
    client_config.clientid[OPTION61_INDEX + OPTION61_IAID_LEN + 4] = client_config.arp[5];
    #endif
 /*end of 支持option61上报内网设备标识 by l129990,2008,12,31*/
    
	/*w44771 add for 防止DHCP浪涌,begin, 2006-7-7*/
	#ifdef SUPPORT_DHCP_SURGE
	if( 1 == dhcp_surge ) 
	{
	        sprintf(cLastMac, "%02d%02d%02d", 
	        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
	        
	        iSeed = atoi(cLastMac);
	        srand(iSeed);
	        iTimeRemain = rand()%7200;
            /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	        timeout = iTimeRemain + getSysUpTime();
            /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	}

        #endif
	/*w44771 add for 防止DHCP浪涌,end, 2006-7-7*/

	state = INIT_SELECTING;
	// brcm
	// run_script(NULL, "deconfig");

	// brcm
	setStatus(0);

	for (;;) {

		// brcm
		if ((old_mode != listen_mode) || (fd == -1)) {
		    old_mode = listen_mode;

            /*START MODIFY:jaffen the fd 0/1/2 will be close when dhcp get ip ADDRESS
            so later file open/socket create may get fd 0,so it need to close fd 0*/
		    if (fd >= 0) {
			    close(fd);
			    fd = -1;
		    }
            /*END MODIFY:jaffen*/
		
		    if (listen_mode == LISTEN_RAW) {
			    if ((fd = raw_socket(client_config.ifindex)) < 0) {
				    LOG(LOG_ERR, "couldn't create raw socket -- au revoir");
				    exit_client(0);
			    }
		    }
		    else if (listen_mode == LISTEN_KERNEL) {
			    if ((fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface)) < 0) {
				    LOG(LOG_ERR, "couldn't create server socket -- au revoir");
				    exit_client(0);
			    }			
		    } else 
			fd = -1;
		}
/* j00100803 Add Begin 2008-03-07 */
#if 0 //暂时屏蔽此特性
        unsigned long lNow = time(0);
        if(timeout > lNow)
        {
            tv.tv_sec = timeout - lNow;
        }
        else
        {
            tv.tv_sec = 0;
        }
#else
                /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
		tv.tv_sec = timeout - getSysUpTime();
                /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/

#endif
/* j00100803 Add End 2008-03-07 */
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		if (listen_mode) FD_SET(fd, &rfds);
		
		if (tv.tv_sec > 0) {
            /*START MODIFY:jaffen If fd is no file attach to it ,just use select to wait time out
            According to Singtel test ,if use select to wait event of a null fd set ,it may cause 
            network block,may be it is a bug of kernel or poxis ,i don't konw*/
            if(fd == -1)
            {
                retval = select(0, NULL, NULL, NULL, &tv);
            }
            else
            {
    			retval = select(fd + 1, &rfds, NULL, NULL, &tv);
            }
            /*END MOFIFY:jaffen*/
		} else retval = 0; /* If we already timed out, fall through */
		
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
				// brcm

				syslog(LOG_INFO, "IN INIT_SELECTING state ");  

				
				setStatus(0);
				if (packet_num < 3) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
                                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + ((packet_num == 2) ? REQ_TIMEOUT : 2);
                                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num++;
				} else {
					if (client_config.abort_if_no_lease) {
						LOG(LOG_INFO,
						    "No lease, failing.");
						exit_client(1);
				  	}
					/* wait to try again */
                                        //w44771 add for A36D02465, begin, 2006-8-14
                                        discover_times ++;
					if(3 == discover_times)
					{
					    discover_times = 0;
					    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定,范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定，范围1~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4);
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface,ip3, ip4);
					}
					system(cmd);
					}
                                        //w44771 add for A36D02465, end, 2006-8-14
					packet_num = 0;
                                   /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + INIT_TIMEOUT;
                                   /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
						/* BEGIN: Modified by y67514, 2008/2/20   PN:AU8D00203*/
						send_renew_with_request(xid, server_addr, requested_ip); /* unicast */
						/* END:   Modified by y67514, 2008/2/20 */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
					/*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + ((packet_num == 2) ? REQ_TIMEOUT : 2);
                                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num++;
				} else {
					/* timed out, go back to init state */
                                        //w44771 add for A36D02465, begin, 2006-8-14				    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定，范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定范围，0~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4 );
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface, ip3, ip4);
					}
					system(cmd);
					
                                        //w44771 add for A36D02465, end, 2006-8-14
					state = INIT_SELECTING;
										
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					/*Start -- w00135358 add for Default Gateway Lost issue: QC issue453*/
					//timeout = getSysUpTime();
					timeout = getSysUpTime()+6;
					setStatus(0);
					/*End -- w00135358 add for Default Gateway Lost issue: QC issue453*/
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/

					packet_num = 0;
					listen_mode = LISTEN_RAW;
					
				}
				break;
			case BOUND:
			    /* j00100803 Add Begin 2008-03-07 */
			    #if 0  //暂时屏蔽此特性
                if(TIMER_ENUM_NORMAL == enTimerType)
                {
                    state = RENEWING;
				    DEBUG(LOG_INFO, "Entering renewing state");
                }
                /* 当前的定时器是renewing定时器 */
                else if(TIMER_ENUM_RENEWING == enTimerType)
                {
                    state = RENEWING;
                    /* renewing定时器已走完 */
                    enTimerOption58 = TIMER_ENUM_STOP;
                    /* 如果还设置了rebinding定时器 */
                    if(TIMER_ENUM_RUN == enTimerOption59)
                    {
                        enTimerType = TIMER_ENUM_REBINDING;
                    }
                    /* 未设置其它定时器了 */
                    else
                    {
                        enTimerType = TIMER_ENUM_NORMAL;
                    }
                    DEBUG(LOG_INFO, "Entering renewing state");
                }
                /* 当前的定时器是rebinding定时器 */
                else if(TIMER_ENUM_REBINDING == enTimerType)
                {
                    state = REBINDING;
                    /* rebinding 定时器已走完 */
                    enTimerOption59 = TIMER_ENUM_STOP;
                    enTimerType = TIMER_ENUM_NORMAL;
                    
				    DEBUG(LOG_INFO, "Entering rebinding state");
				    /* 跳入rebinding状态，直接返回最上面不等待的select */
				    break;
                }
                
			    #else
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
			    #endif
				/* j00100803 Add End 2008-03-07 */
				listen_mode = LISTEN_KERNEL;
				DEBUG(LOG_INFO, "Entering renew state");
				/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
                #if LOG_DISPLAY
                syslog(LOG_INFO, "Entering renew state on interface %s", session_path); 
				#endif
			    /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
				/* fall right through */
			case RENEWING:
			    /* j00100803 Add Begin 2008-03-07 */
			    #if 0  //暂时屏蔽此特性
			    if(TIMER_ENUM_NORMAL == enTimerType)
			    {
    			    if ((t2 - t1) <= (lease / 14400 + 1))
    			    {
    					/* timed out, enter rebinding state */
    					state = REBINDING;
    					timeout = time(0) + (t2 - t1);
    					DEBUG(LOG_INFO, "Entering rebinding state");
    				}
    				else
    				{
    					/* send a request packet */
    					send_renew(xid, server_addr, requested_ip); /* unicast */
    					
    					t1 = (t2 - t1) / 2 + t1;
    					timeout = t1 + start;
    				}
				}
				else if(TIMER_ENUM_REBINDING == enTimerType)
				{
                    /* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				
			    #else
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
                                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime() + (t2 - t1);
                                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				#endif
				/* j00100803 Add End 2008-03--07 */
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
                                        //w44771 add for A36D02465, begin, 2006-8-14				    
					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定，范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定范围，0~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4 );
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface, ip3, ip4);
					}
					system(cmd);
					
                                        //w44771 add for A36D02465, end, 2006-8-14
					
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
					#if LOG_DISPLAY
					syslog(LOG_INFO, "Lease lost, entering init state on interface %s", session_path);  
                    #endif
					/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
					run_script(NULL, "deconfig");
                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/

					/*Start -- w00135358 add for Default Gateway Lost issue: QC issue453*/
					//timeout = getSysUpTime();
					timeout = getSysUpTime()+6;
					setStatus(0);
					/*End -- w00135358 add for Default Gateway Lost issue: QC issue453*/

					
                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					packet_num = 0;
					listen_mode = LISTEN_RAW;
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* broadcast */

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0x7fffffff;
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */
			
			if (listen_mode == LISTEN_KERNEL) {
				if (get_packet(&packet, fd) < 0) continue;
			} else {
				if (get_raw_packet(&packet, fd) < 0) continue;
			} 
			
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
			
			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;
						/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
						#if LOG_DISPLAY
                        temp_addr.s_addr = server_addr;
						tempfour_addr.s_addr = requested_ip;
 						strcpy(server_ip,inet_ntoa(temp_addr));
						strcpy(req_ip,inet_ntoa(tempfour_addr));
						syslog(LOG_INFO, "Received DHCPOFFER on interface %s ,Server IP: %s , Offer IP: %s", 
						 	session_path, server_ip,req_ip);
						#endif
						/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */

						 /* enter requesting state */
						state = REQUESTING;
                                            /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
						timeout = getSysUpTime();
                                            /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
						packet_num = 0;
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					
					/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
                    #if LOG_DISPLAY
					syslog(LOG_INFO, "Received DHCPACK on on interface %s ", session_path);  
					#endif
					/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60*60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}
					/* j00100803 Add Begin 2008-03-07 */
				#if 0  //暂时屏蔽此特性
					char * pDataBegin = NULL;
					if(NULL != (pDataBegin = get_option(&packet, DHCP_T1)))
					{
					    int iOptionLen = (unsigned int)*(pDataBegin - 1);
					    if(iOptionLen == 4)
					    {
    					    int * pTmp = &iRenewingTimerOption58;
    					    memcpy(pTmp, pDataBegin, 4);
					    }
					}
                    pDataBegin = NULL;
					if(NULL != (pDataBegin = get_option(&packet, DHCP_T2)))
					{
					    int iOptionLen = (unsigned int)*(pDataBegin - 1);
					    if(iOptionLen == 4)
					    {   
    					    int * pTmp = &iRebindingTimerOption59;
    					    memcpy(pTmp, pDataBegin, 4);
					    }
					}
					if(iRenewingTimerOption58 > 0)
					{
					    t1 = iRenewingTimerOption58;
					    enTimerOption58 = TIMER_ENUM_RUN;
					    enTimerType = TIMER_ENUM_RENEWING;
					    iRenewingTimerOption58 = 0;
					}
                    else
                    {
                        /* 默认配置 */
                        enTimerOption58 = TIMER_ENUM_STOP;
                        enTimerType = TIMER_ENUM_NORMAL;
                        t1 = lease / 2;
                    }

                    if(iRebindingTimerOption59 > 0)
                    {
                        t2 = iRebindingTimerOption59;
                        enTimerOption59 = TIMER_ENUM_RUN;
                        if(enTimerType != TIMER_ENUM_RENEWING)
                        {
                            enTimerType = TIMER_ENUM_REBINDING;
                        }
                        iRebindingTimerOption59 = 0;
                    }
                    else 
                    {
                        /* 默认配置 */
                        enTimerOption59 = TIMER_ENUM_STOP;
                        t2 = (lease * 0x7) >> 3;
                    }
                    
                    temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					/* 用start记录当前计时器开始的时刻 */
					start = time(0);
                    /* 非正常情况 */
                    if(t1 > t2)
                    {
                        /* 恢复缺省值 */
                        t1 = lease / 2;
                        t2 = (lease * 0x7) >> 3;
                        timeout = t1 + start;
                        enTimerOption58 = TIMER_ENUM_STOP;
                        enTimerOption59 = TIMER_ENUM_STOP;
                        enTimerType = TIMER_ENUM_NORMAL;                        
                    }
                    /* 正常情况 */
                    else
                    {
                        timeout = t1 + start; 
                    }
					#else
					/* enter bound state */
					t1 = lease / 2;
					
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					
					/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
					#if LOG_DISPLAY
				    temp = get_option(&packet, DHCP_SERVER_ID);
				    if (temp)
                    {    
					    memcpy(&server_addr, temp, 4);
					    tempsec_addr.s_addr = server_addr; 
				    }
					else 
					{
                        tempsec_addr.s_addr = 0; 
					}
					if (!(log_subnet =get_option(&packet, DHCP_SUBNET)))
                    {    
                       tempthird_addr.s_addr = subnet_addr;
					}
					else
					{
                        memcpy(&subnet_addr, log_subnet, 4);
					    tempthird_addr.s_addr = subnet_addr;   
					}
                    syslog(LOG_INFO, "DHCP lease obtained: IP address: %s on interface %s" , 
						inet_ntoa(temp_addr),session_path);
					syslog(LOG_INFO, "Option 1(subnet): %s  on interface %s" ,inet_ntoa(tempthird_addr) ,session_path);
					syslog(LOG_INFO, "Option 51(lease): %ld  on interface %s" ,lease ,session_path);
					syslog(LOG_INFO, "Option 53(dhcptype): %d  on interface %s" ,*message,session_path);
					syslog(LOG_INFO, "Option 54(serverid): %s  on interface %s" ,inet_ntoa(tempsec_addr) ,session_path);
                    #endif
				   /*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
                                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					start = getSysUpTime();
                                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = t1 + start;
					#endif
					/* j00100803 Add Begin 2008-03-07 */
					requested_ip = packet.yiaddr;
					/* 这里解析注册的所有option选项 */
					run_script(&packet,
						   ((state == RENEWING || state == REBINDING) ? "renew" : "bound"));

                               /*start of  2008.05.04 HG553V100R001C02B013 AU8D00566 */
					//添加option121的路由
					run_staticRoute();
                               /*end of  2008.05.04 HG553V100R001C02B013 AU8D00566 */

					state = BOUND;
					//listen_mode = LISTEN_NONE;
					listen_mode = LISTEN_KERNEL;
					
					// brcm
					setStatus(1);
					background();
					
				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");

					/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
                    #if LOG_DISPLAY
                     syslog(LOG_INFO, "Received DHCPNAK on interface %s ", session_path);  
					#endif
					/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
                                   /* BEGIN: Added by y67514, 2008/9/28   PN:GLB:支持Option15*/
                                   if ( option56 = get_option(&packet, DHCP_MESSAGE) )
                                   {
                                       syslog(LOG_INFO, "Received Option 56 %s",option56);
                                   }
                                   /* END:   Added by y67514, 2008/9/28 */
					
					if (state != REQUESTING)
						run_script(NULL, "deconfig");




					/*Start -- w00135358 add for Default Gateway Lost issue: QC issue453*/

					char cmdfile[128];
   					char cmd[128];
   					
   					int ip3 = 0;
   					int ip4 = 0;
   					char rand_ip[16];

   					memset(rand_ip, 0, sizeof(rand_ip));
   					
				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[0], client_config.arp[1], client_config.arp[2]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip3 = rand()%256;//IP3 根据前3个MAC地址确定，范围0~255

				        sprintf(cLastMac, "%02d%02d%02d", 
				        client_config.arp[3], client_config.arp[4], client_config.arp[5]);
				        
				        iSeed = atoi(cLastMac);
				        srand(iSeed);
				        ip4 = rand()%255;//IP4 根据后3个MAC地址确定范围，0~254
				        if(0 == ip4)
				        {
				            ip4 = 1;
				        }   					
   					
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/status", client_config.Interface);
					sprintf(cmd, "echo 4 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmdfile);
					system(cmd);
					sprintf(cmdfile, "/proc/var/fyi/wan/%s/ipaddress", client_config.Interface);
					sprintf(cmd, "echo 169.254.%d.%d > %s", ip3, ip4, cmdfile);
					system(cmd);
					
					if ( strcmp(client_config.interface,"br0") == 0 )
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.Interface,ip3, ip4 );
					}
					else
					{
					    sprintf(cmd, "ifconfig %s 169.254.%d.%d", client_config.interface, ip3, ip4);
					}
					system(cmd);



					state = INIT_SELECTING;
                                    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					timeout = getSysUpTime()+6;
                                    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
					requested_ip = 0;
					packet_num = 0;
					listen_mode = LISTEN_RAW;
					syslog(LOG_INFO, "Received DHCPNAK on interface %s, going to INIT_SELECTING state ", session_path);  
					setStatus(0);

					#if 0
					// brcm
					//setStatus(0);
					 char cmd[128] = "";
					  sprintf(cmd, "echo 4 > /proc/var/fyi/wan/%s/status", session_path);
					system(cmd);

					sprintf(cmd, "/proc/var/fyi/wan/%s/daemonstatus", client_config.Interface);
					sprintf(cmd, "echo 0 > %s", cmd);
					//setStatus(0);
					#endif					
				    /*End -- w00135358 add for Default Gateway Lost issue: QC issue453*/

				}
//#ifdef VDF_FORCERENEW
#if 0
				else if(DHCPFORCERENEW == *message)		//支持RFC3203:forcerenew
				{
					printf("VDF:%s:%s:%d:Received forcerenew***\n",__FILE__,__FUNCTION__,__LINE__);
					renew_requested(0);
				}
#endif
				break;
			case BOUND:
#ifdef VDF_FORCERENEW
				if(DHCPFORCERENEW == *message)		//支持RFC3203:forcerenew
				{
					printf("VDF:%s:%s:%d:Received forcerenew***\n",__FILE__,__FUNCTION__,__LINE__);
					renew_requested(0);
					/*l129990 add for 增加dhcp日志记录功能,begin,2008-7-7 */
                    #if LOG_DISPLAY
                    syslog(LOG_INFO, "Received forcerenew on interface %s ", session_path);  
                    #endif
					/*l129990 add for 增加dhcp日志记录功能,end,2008-7-7 */
				}
#endif
				break;
			case RELEASED:

				/* ignore all packets */
				break;
			}					
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */
			
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}
		
	}
	return 0;
}

