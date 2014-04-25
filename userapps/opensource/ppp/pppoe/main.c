/*
 * main.c - Point-to-Point Protocol main module
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define RCSID	"$Id: main.c,v 1.11.2.1.6.2 2010/01/19 17:03:03 l43571 Exp $"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <netdb.h>
#include <utmp.h>
#include <pwd.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// brcm
#include <syscall.h>

#include "pppd.h"
#include "magic.h"
#include "fsm.h"
#include "lcp.h"
#include "ipcp.h"
#ifdef INET6
#include "ipv6cp.h"
#endif
#include "upap.h"
#include "chap.h"
#include "ccp.h"
#include "pathnames.h"
#include "tdb.h"

#ifdef CBCP_SUPPORT
#include "cbcp.h"
#endif

#ifdef IPX_CHANGE
#include "ipxcp.h"
#endif /* IPX_CHANGE */
#ifdef AT_CHANGE
#include "atcp.h"
#endif
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

static const char rcsid[] = RCSID;
#define FILE_PATH_LEN		128   /*begin  modify by xia 133940 2008年9月18日*/ 

/* interface vars */
char ifname[32];		/* Interface name */
int ifunit;			/* Interface unit number */

struct channel *the_channel;

char *progname;			/* Name of this program */
char hostname[MAXNAMELEN];	/* Our hostname */
static char pidfilename[MAXPATHLEN];	/* name of pid file */
static char linkpidfile[MAXPATHLEN];	/* name of linkname pid file */
char ppp_devnam[MAXPATHLEN];	/* name of PPP tty (maybe ttypx) */
uid_t uid;			/* Our real user-id */
struct notifier *pidchange = NULL;
struct notifier *phasechange = NULL;
struct notifier *exitnotify = NULL;
struct notifier *sigreceived = NULL;

int hungup;			/* terminal has been hung up */
int privileged;			/* we're running as real uid root */
int need_holdoff;		/* need holdoff period before restarting */
int detached;			/* have detached from terminal */
volatile int status;		/* exit status for pppd */
/* BEGIN: Added by hKF20513, 2009/11/12   PN:记录PPP连接失败的原因*/
char LastErr[FILE_PATH_LEN];
/* END:   Added by hKF20513, 2009/11/12 */
int unsuccess;			/* # unsuccessful connection attempts */
int do_callback;		/* != 0 if we should do callback next */
int doing_callback;		/* != 0 if we are doing callback */
TDB_CONTEXT *pppdb;		/* database for storing status etc. */
char db_key[32];

int (*holdoff_hook) __P((void)) = NULL;
int (*new_phase_hook) __P((int)) = NULL;

static int conn_running;	/* we have a [dis]connector running */
static int devfd;		/* fd of underlying device */
static int fd_ppp = -1;		/* fd for talking PPP */
static int fd_loop;		/* fd for getting demand-dial packets */

int phase;			/* where the link is at */
int kill_link;
int open_ccp_flag;
int listen_time;
int got_sigusr2;
int got_sigterm;
int got_sighup;
/* BEGIN: Added by y67514, 2008/11/6   PN:AU8D01121:HSPA E180进行PPPoE拨号失效*/
extern int authmode_failed;
/* END:   Added by y67514, 2008/11/6 */
/* BEGIN: Added by y67514, 2008/11/1   PN:GLB:KeepAlive需求*/
int	lcp_echo_interval = 30; 	/* Interval between LCP echo-requests */
/* END:   Added by y67514, 2008/11/1 */
//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
int lcp_echo_fails = 3;
//END:add by zhourongfei to config number of PPP keepalive attempts

static int waiting;
static sigjmp_buf sigjmp;

char **script_env;		/* Env. variable values for scripts */
int s_env_nalloc;		/* # words avail at script_env */

u_char outpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for outgoing packet */
u_char inpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for incoming packet */

static int n_children;		/* # child processes still running */
static int got_sigchld;		/* set if we have received a SIGCHLD */

int privopen;			/* don't lock, open device as root */

char *no_ppp_msg = "Sorry - this system lacks PPP kernel support\n";

GIDSET_TYPE groups[NGROUPS_MAX];/* groups the user is in */
int ngroups;			/* How many groups valid in groups */

static struct timeval start_time;	/* Time when link was started. */

struct pppd_stats link_stats;
int link_connect_time;
int link_stats_valid;
#ifdef VDF_PPPOU
/*start , ppp over usb*/
struct ppp_his_stat  g_pppd_history_demand_stats = {0, 0}; 
struct ppp_his_stat  g_pppd_overflow_status = {0, 0};
static int in_stats_overflowing = 0, out_stats_overflowing = 0;
/*end, ppp over usb*/
int g_hspavoice_flg = 0;
/* BEGIN: Added by y67514, 2008/7/19   PN:只拨3次，失败则不拨*/
int DialCount = 0;                      /*拨号计数*/
/* END:   Added by y67514, 2008/7/19 */
#endif

int g_bCurrWlanChip = E_RALINK_WLAN;

/*
 * We maintain a list of child process pids and
 * functions to call when they exit.
 */
struct subprocess {
    pid_t	pid;
    char	*prog;
    void	(*done) __P((void *));
    void	*arg;
    struct subprocess *next;
};

static struct subprocess *children;

/* Prototypes for procedures local to this file. */

static void setup_signals __P((void));
static void create_pidfile __P((void));
static void create_linkpidfile __P((void));
static void cleanup __P((void));
static void get_input __P((void));
static void calltimeout __P((void));
static struct timeval *timeleft __P((struct timeval *));
static void kill_my_pg __P((int));
static void hup __P((int));
static void term __P((int));
static void chld __P((int));
static void toggle_debug __P((int));
static void open_ccp __P((int));
static void toggle_hspavoice __P((int));
static void bad_signal __P((int));
static void alarm_signal __P((int));
static void holdoff_end __P((void *));
static int reap_kids __P((int waitfor));
static void update_db_entry __P((void));
static void add_db_key __P((const char *));
static void delete_db_key __P((const char *));
static void cleanup_db __P((void));
static void handle_events __P((void));
static void setPid __P((void));

extern	char	*ttyname __P((int));
extern	char	*getlogin __P((void));
int main __P((int, char *[]));

#ifdef ultrix
#undef	O_NONBLOCK
#define	O_NONBLOCK	O_NDELAY
#endif

#ifdef ULTRIX
#define setlogmask(x)
#endif

/*
 * PPP Data Link Layer "protocol" table.
 * One entry per supported protocol.
 * The last entry must be NULL.
 */
struct protent *protocols[] = {
    &lcp_protent,
    &pap_protent,
    &chap_protent,
#ifdef CBCP_SUPPORT
    &cbcp_protent,
#endif
    &ipcp_protent,
#ifdef INET6
    &ipv6cp_protent,
#endif
// brcm
//    &ccp_protent,
#ifdef IPX_CHANGE
    &ipxcp_protent,
#endif
#ifdef AT_CHANGE
    &atcp_protent,
#endif
    NULL
};

/*
 * If PPP_DRV_NAME is not defined, use the default "ppp" as the device name.
 */
#if !defined(PPP_DRV_NAME)
#define PPP_DRV_NAME	"ppp"
#endif /* !defined(PPP_DRV_NAME) */

#ifdef VDF_PPPOU

#define PPPOU_PID   "/var/pppoupid"

/* 写pppou的pid*/
void write_pppou_pid(   )
{
    FILE *fp = NULL;

    fp = fopen(PPPOU_PID,"w");
    if(fp)
    {
        fprintf(fp,"%d",getpid());
        fclose(fp);
    }
    return;
}



//start modify by xia 133940 2008年9月2日
//函数功能:记录下每一条PPP的连接时间，
//以计算出它的在线时间，
//属于命令行ppp uptime的程序段
/*
void thisCurrTime()
{    	
	char getp[12]="";	
	char filename[FILE_PATH_LEN]="";
	
	long up_time = 0;
	char acLine[200];
	char *pcLine = NULL;
	char cmd[200];	

	FILE *fp = NULL;

	sprintf(filename, "%s%s%s", "/var/", session_path, "_uptime");

	
	if(NULL != (fp = fopen("/proc/uptime","r")))
	{
		memset(acLine, 0, sizeof(acLine));
		fgets(acLine, 200,fp);
		fclose(fp);
		if(NULL != (pcLine = strstr(acLine," ")))
		{
		   *pcLine ='\0';
		}
		//舍掉小数
		if(NULL != (pcLine = strstr(acLine,".")))
		{
		   *pcLine ='\0';
		}
		up_time = atol(acLine);
		//printf("\nprint in line %d , now time is __________  %d  _____________ \n" , __LINE__ , up_time );
	}
	else
	{
		printf("\nprint in main.c %d line, open /proc/uptime default!\n" , __LINE__);
		return;
	}



	sprintf(cmd, "echo %ld > %s",  up_time , filename);
	system(cmd); 


	sleep(1);
		  

}
//end modify by xia 133940 2008年9月2日

*/




//start modify by xia 133940 2008年9月2日
int readfiles()
{
	FILE *fp=NULL;
	char getp[12]="";
	int npid=0;
	char filename[FILE_PATH_LEN]="";
	sprintf(filename, "%s%s%s", "/var/", session_path, "_status");

	if (( fp=fopen(filename ,"r")) !=NULL )
	{
		fgets( getp, 10, fp);
		npid=atoi( getp );
		fclose(fp);				
	}

	return npid;



}
//end modify by xia 133940 2008年9月2日

int get_wlan_chip_type(void)
{
#define KRNL_DECTECTED_DEVICES  "/proc/bus/pci/devices"
#define BCM_VERNDOR_ID          0x14e4
#define ATHEROS_VERNDOR_ID      0x168c
#define RALINK_VERNDOR_ID       0x1814

/* seq_printf(m, "%02x%02x\t%04x%04x\t%x", dev->bus->number, dev->devfn, */ 
#define SKIP_TAG                 "\t"     //skip bus number and devfn

    char  acLineBbuf[SYS_CMD_LEN];
    char  acVendorStr[][12] = {"RaLink", "Atheros", "Broadcom"};
    char  *pStr = NULL;
    int iVendorProduct = 0;
    int iVendorID = 0;
    int iVendorType = E_RALINK_WLAN;
    FILE *fp = NULL;
    
    if (NULL == (fp = fopen(KRNL_DECTECTED_DEVICES, "r")))
    {
        printf("Open Device Failed\n");
        return E_RALINK_WLAN;
    }

    if (fgets(acLineBbuf, SYS_CMD_LEN, fp) != NULL)
    {    
        pStr = strstr(acLineBbuf, SKIP_TAG);
        if (NULL == pStr)
        {        
            return E_RALINK_WLAN;
        }
        
        sscanf(pStr+1, "%x", &iVendorProduct);
        
        iVendorID = iVendorProduct >> 16;
        switch (iVendorID)
        {
            case BCM_VERNDOR_ID:
                iVendorType = E_BCM_WLAN;
                break;
            case ATHEROS_VERNDOR_ID:
                iVendorType = E_ATHEROS_WLAN;
                break;
            case RALINK_VERNDOR_ID:
                iVendorType = E_RALINK_WLAN;
                break;
            default:
                printf("getWlanChipType Unkown Vendor Type = <0x%x>.\n", iVendorType);
                break;
        }

        printf("PPP@WLAN CHIP Vendor=<%s>, ProductId=<0x%x>\n", acVendorStr[iVendorType], iVendorProduct & 0xFFFF);
        
    }

    fclose(fp);

    return iVendorType;
}


int pppd_main(argc, argv)
//#else
//main(argc,argv)
//#endif
#else
int
#ifdef BUILD_STATIC
pppd_main(argc, argv)
#else
main(argc,argv)
#endif
#endif
    int argc;
    char *argv[];
{
    int i, t;
    char *p;
    struct passwd *pw;
    struct protent *protp;
    char numbuf[16];
    // brcm
    int demandBegin=0;

    g_bCurrWlanChip = get_wlan_chip_type();

    new_phase(PHASE_INITIALIZE);

    /*
     * Ensure that fds 0, 1, 2 are open, to /dev/null if nowhere else.
     * This way we can close 0, 1, 2 in detach() without clobbering
     * a fd that we are using.
     */
    if ((i = open("/dev/null", O_RDWR)) >= 0) {
	while (0 <= i && i <= 2)
	    i = dup(i);
	if (i >= 0)
	    close(i);
    }

    script_env = NULL;

    /* Initialize syslog facilities */
    reopen_log();

    if (gethostname(hostname, MAXNAMELEN) < 0 ) {
	option_error("Couldn't get hostname: %m");
	exit(1);
    }
    hostname[MAXNAMELEN-1] = 0;

    /* make sure we don't create world or group writable files. */
    umask(umask(0777) | 022);

    uid = getuid();
    privileged = uid == 0;
    slprintf(numbuf, sizeof(numbuf), "%d", uid);
    script_setenv("ORIG_UID", numbuf, 0);

    ngroups = getgroups(NGROUPS_MAX, groups);

    /*
     * Initialize magic number generator now so that protocols may
     * use magic numbers in initialization.
     */
    magic_init();


    myparse_args(argc, argv);


/*start modify by xia 2008-9-22*/	
	if ( pppoeWan )
	{
	FILE *fp12=NULL;
	int ntimeout;
	char tfilepath[MAXNAMELEN]="";	 
	char atemp[MAXNAMELEN]="";
	char *whichline=NULL;
	 
	sprintf(tfilepath, "%s%s", "/var/ppprun/", mysession_path );
	//printf("\nprint in %s at %d line by xia ,filepath is %s: \n", __FILE__,__LINE__ , filepath);

	if (( fp12=fopen(tfilepath ,"r")) !=NULL )
	{
		memset(atemp, 0, sizeof(atemp));
		fgets(atemp, sizeof(atemp) , fp12);

		fclose(fp12);

		
		if(NULL != (whichline = strstr(atemp,"timeout=")))
		{
			  whichline += strlen("timeout=");
			  ntimeout= atol( whichline ) ;
			  printf("\ntimeout  in file %s  at  line %d is:%d\n",__FILE__, __LINE__ ,ntimeout );
		}
		

	}

	used_in_fsm_timeout=ntimeout;
	}
/*end modify by xia 2008-9-22*/		


//BEGIN:modify by zhourongfei 修改时序，移动至parse_args(argc, argv);之后
    /*
     * Initialize each protocol.
     
     *	for (i = 0; (protp = protocols[i]) != NULL; ++i)
     *   (*protp->init)(0);
	*/

//END:end by zhourongfei
    /*
     * Initialize the default channel.
     */
    // brcm
    //tty_init();
#ifdef VDF_PPPOU
    tty_init();//pppousb
    DialCount = 0;
#endif

    progname = *argv;

    /*
     * Parse, in order, the system options file, the user's options file,
     * and the command line arguments.
     */

// brcm
#if 0
    if (!options_from_file(_PATH_SYSOPTIONS, !privileged, 0, 1)
	|| !options_from_user()
// brcm
	|| !parse_args(argc, argv))
//	|| !parse_args(argc-1, argv+1))
	exit(EXIT_OPTION_ERROR);
#endif
    parse_args(argc, argv);
//BEGIN:modify by zhourongfei 修改时序
    for (i = 0; (protp = protocols[i]) != NULL; ++i)
        (*protp->init)(0);
//END:end by zhourongfei
    devnam_fixed = 1;		/* can no longer change device name */
    if ( autoscan)
    {
        pppoeWan = 0;
    }

    setPid();

    // brcm
    //setdevname_pppoe("eth0");
#ifdef VDF_PPPOU
    /*start, add by sxg, pppousb*/
    if(usbWan){
//        __p(&g_pppd_start_mutex);
 //       __v(&g_pppd_start_mutex);
	   	setdevname(USB_DEVNAM_MODEM, NULL, 1);
                write_pppou_pid();
    	VDF_DBG("VDF TEST: %s %d, dev=%s", __FILE__, __LINE__, devnam);
	}
    /*end, add by sxg, pppousb*/
#endif

    /*
     * Work out the device name, if it hasn't already been specified,
     * and parse the tty's options file.
     */
    if (the_channel->process_extra_options)
	(*the_channel->process_extra_options)();

    if (debug)
	setlogmask(LOG_UPTO(LOG_DEBUG));

    /*
     * Check that we are running as root.
     */
    if (geteuid() != 0) {
	option_error("must be root to run %s, since it is not setuid-root",
		     argv[0]);
	exit(EXIT_NOT_ROOT);
    }

    if (!ppp_available()) {
	option_error("%s", no_ppp_msg);
	exit(EXIT_NO_KERNEL_SUPPORT);
    }

    /*
     * Check that the options given are valid and consistent.
     */
// brcm
#if 0
    check_options();
    if (!sys_check_options())
	exit(EXIT_OPTION_ERROR);
    auth_check_options();
#ifdef HAVE_MULTILINK
    mp_check_options();
#endif
    for (i = 0; (protp = protocols[i]) != NULL; ++i)
	if (protp->check_options != NULL)
	    (*protp->check_options)();
    if (the_channel->check_options)
	(*the_channel->check_options)();


    if (dump_options || dryrun) {
	init_pr_log(NULL, LOG_INFO);
	print_options(pr_log, NULL);
	end_pr_log();
	if (dryrun)
	    die(0);
    }
#endif

    /*
     * Initialize system-dependent stuff.
     */
    sys_init();

    pppdb = tdb_open(_PATH_PPPDB, 0, 0, O_RDWR|O_CREAT, 0644);
    if (pppdb != NULL) {
	slprintf(db_key, sizeof(db_key), "pppd%d", getpid());
	update_db_entry();
    } else {
	warn("Warning: couldn't open ppp database %s", _PATH_PPPDB);
	if (multilink) {
	    warn("Warning: disabling multilink");
	    multilink = 0;
	}
    }

    /*
     * Detach ourselves from the terminal, if required,
     * and identify who is running us.
     */
    if (!nodetach && !updetach)
	detach();
    p = getlogin();
    if (p == NULL) {
	pw = getpwuid(uid);
	if (pw != NULL && pw->pw_name != NULL)
	    p = pw->pw_name;
	else
	    p = "(unknown)";
    }
    syslog(LOG_NOTICE, "pppd %s started by %s, uid %d", VERSION, p, uid);
    script_setenv("PPPLOGNAME", p, 0);

    if (devnam[0])
	script_setenv("DEVICE", devnam, 1);
    slprintf(numbuf, sizeof(numbuf), "%d", getpid());
    script_setenv("PPPD_PID", numbuf, 1);

    setup_signals();

    waiting = 0;

//    create_linkpidfile();

	if (autoscan)
		demandBegin=1;

    /*
     * If we're doing dial-on-demand, set up the interface now.
     */
    if (demand) {
	/*
	 * Open the loopback channel and set it up to be the ppp interface.
	 */
	tdb_writelock(pppdb);
	fd_loop = open_ppp_loopback();
	set_ifunit(1);
	tdb_writeunlock(pppdb);

	/*
	 * Configure the interface and mark it up, etc.
	 */
	demand_conf();
    }




/*begin  modify by xia 133940 2008年10月15日V100R001C01B020  AU8D00972*/ 

	FILE *fp1;
	char acLine[200];
	char *pcLine = NULL;
	int automanualConnect;
	int ndelayTime;  //延迟多少秒后再拨号(自动有效)
	int delayRedial = 0;
	int ntimeout;
	int nrand_delaytime;
	char filepath[128]="";
	char filename[FILE_PATH_LEN]="";
	char cmd[200]="";	


	if ( pppoeWan )
	{
	//printf("\nprint in %s at %d line,  xxxxxxxxxx   \n", __FILE__, __LINE__);
	
		sprintf(filepath, "%s%s", "/var/ppprun/", session_path );
	//	printf("\nprint in %s at %d line by xia ,filepath is %s: \n", __FILE__,__LINE__ , filepath);
		
		if (( fp1=fopen(filepath ,"r")) !=NULL )
		{
			memset(acLine, 0, sizeof(acLine));
			fgets(acLine, sizeof(acLine) , fp1);
		
			fclose(fp1);
		
			if(NULL != (pcLine = strstr(acLine,"automanualConnect=")))
			{
				  pcLine += strlen("automanualConnect=");
				  automanualConnect= atol( pcLine ) ;
				
			}
			if(NULL != (pcLine = strstr(acLine,"delayRedial=")))
			{
				 pcLine += strlen("delayRedial=");
				 delayRedial= atol( pcLine ) ;
				 
		/*begin  modify by xia 133940 2008年10月25日AU8D00972*/ 		  
		/*delayRedial是重拨时间,减3的原因是当delayRedial为0，即没有设置
		重拨时间时，系统本身还会有3秒钟的运行时间，这段时间
		系统在关闭PPP连接，进行下一次拨号，所以为了让用户直观的
		感觉到等待时间就是他自己设置的时间，所以必需减去这个4秒*/
				 if ( delayRedial<=3 )
					delayRedial = 0;
				 else
					delayRedial = delayRedial -3;
	       /*end  modify by xia 133940 2008年10月25日AU8D00972*/ 		
		   
				
			}
			if(NULL != (pcLine = strstr(acLine,"rand_delaytime=")))
			{
				  pcLine += strlen("rand_delaytime=");
				  nrand_delaytime= atol( pcLine ) ;
				
			}
			//BEGIN:add by zhourongfei to config number of PPP keepalive attempts
			if(NULL != (pcLine = strstr(acLine,"pppNumber=")))
			{
				  pcLine += strlen("pppNumber=");
				  lcp_echo_fails= atol( pcLine ) ;
				  if(lcp_echo_fails <= 0)
				  	lcp_echo_fails = 3;
				
			}
			//END:add by zhourongfei to config number of PPP keepalive attempts
			/* BEGIN: Added by y67514, 2008/11/3   PN:KeepAlive需求*/
			if(NULL != (pcLine = strstr(acLine,"lcpEchoInterval=")))
			{
                pcLine += strlen("lcpEchoInterval=");
                lcp_echo_interval= atol( pcLine ) ;
                /*Start of 问题单AU8D01488:ppp keepalive报文发送间隔的配置范围为10~60s by l129990,2008,12,15*/
                if ( lcp_echo_interval < 10 || lcp_echo_interval > 60 )
                /*end of 问题单AU8D01488:ppp keepalive报文发送间隔的配置范围为10~60s by l129990,2008,12,15*/
                {
                    lcp_echo_interval = 30;
                }
			}
			/* END:   Added by y67514, 2008/11/3 */
		
		}
		else 
              {      
                  //return;
              }

		
	}

/*end  modify by xia 133940 2008年10月15日V100R001C01B020  AU8D00972 */ 



    do_callback = 0;
    for (;;) {

	listen_time = 0;
	need_holdoff = 1;
	devfd = -1;
	status = EXIT_OK;
	
       /* BEGIN: Added by hKF20513, 2009/11/12   PN:记录PPP连接失败的原因*/
	sprintf(LastErr,"%s/%s/%s","echo ERROR_NONE > /var/fyi/sys",session_path,"LastConnectionError");
	system(LastErr);      
       /* END:   Added by hKF20513, 2009/11/12 */

	++unsuccess;
	doing_callback = do_callback;
	do_callback = 0;
       /* BEGIN: Added by y67514, 2008/11/6   PN:AU8D01121:HSPA E180进行PPPoE拨号失效*/
       authmode_failed = 0;
       if ( usbWan )
       {
           persist = 1;
       }
       /* END:   Added by y67514, 2008/11/6 */
    //s48571 modified begin for vdf-sp (back to origin version to delete the feature of forbid hspa led blinking when no traffic )
	if (!autoscan)
	    while(!link_up())      //如果掉线了，就设ppp为第一次拨号
	    {
    		sleep(1);
		bIsPppFirstDial =0;           /*begin  modify by xia 133940 2008年10月17日V100R001C01B020  AU8D00972*/ 	
	    }
	if(usbWan)
	{
	    /* BEGIN: Added by y67514, 2008/7/19   PN:只拨3次，失败则不拨*/
	    DialCount++;
            if ( DialCount > MAXCOUNT)
            {
                //0-hapa不拨号
                system("echo 0 >/var/HspaDialFlag");
                sleep(1);
                DialCount = 0;
            }
	    /* END:   Added by y67514, 2008/7/19 */
	    /* BEGIN: Added by y67514, 2008/4/17  Dial on demand */
    	    //检测是否可以进行拨号
    	    /*start of 问题单AU8D03724：关闭切换确认机制后，网关在LAN侧未接PC的情况下仍然反复拨号断开 by l129990,2010-1-14*/
    	    while ( !HspaDialFlag()||!link_up()||!lan_intf_up() )
            /*end of 问题单AU8D03724：关闭切换确认机制后，网关在LAN侧未接PC的情况下仍然反复拨号断开 by l129990,2010-1-14*/
    	    {
    	        sleep(1);
    	    }
            /* END:   Added by y67514, 2008/4/17 */
		//数据卡的查询与初始设置
		while(at_init())
		{
			sleep(1);
		}
		VDF_DBG("VDF:%s:%s:%d:pppou start\n",__FILE__,__FUNCTION__,__LINE__);

		//BEGIN:add by zhourongfei to enable demand
		if(demand)
		{
			system("route del default");
			demand_conf();
		}
		//END:add by zhourongfei to enable demand
	}


	/*begin  modify by xia 133940 2008年10月17日V100R001C01B020  AU8D00972*/ 


	if ( pppoeWan )
	{
	
	printf("\nfilepath in main.c line %s is: %d\n",filepath , __LINE__);//line 724
	if (0==automanualConnect)     //0表示手动连接
	{
			while (1)   //add by xia 133940 2008年8月20日
			{
				//读temp，直到读出1才break，0表示断开
				if ( 1==readfiles() && link_up() )
					break;
				//printf("\nprint in %s at %d line  \n",__FILE__ , __LINE__ );
				sleep(1);
			}
	}
	else
	{
			if ( 0==bIsPppFirstDial )  
			{
				
				int itemp=0;
					
				for (  itemp=1 ; itemp<=nrand_delaytime ; itemp++ )		
				{
					//printf("\nprint in %s at %d line, rand delaytime is :...........................%d ......................\n",__FILE__ , __LINE__, itemp );
					sleep(1);
				}

				bIsPppFirstDial =1;
			}
		}
		
	
	}
	/*end  modify by xia 133940 2008年9月11日*/ 

	

	
        syslog(LOG_NOTICE, "PPP: Start to connect ...\n");
    //s48571 modified end for vdf-sp (back to origin version to delete the feature of forbid hspa led blinking when no traffic )

	if (ipext && !demandBegin)
	    while (!lan_link_up())
		sleep(1);

	if (autoscan) {
	    holdoff=0;
	    ses_retries = 3;

	    if (!demandBegin)
		exit(0);
	}    


	// brcm
	//BEGIN:modify by zhourongfei
	if(0 == access("/var/HspaDialFlag_temp",F_OK))
	{
		system("rm /var/HspaDialFlag_temp -f");
	}
	else
	{
		if (demand && !doing_callback && !demandBegin) {
		//if (demand && !doing_callback) {
		    /*
		     * Don't do anything until we see some activity.
		     */
		    new_phase(PHASE_DORMANT);
		    demand_unblock();
		    add_fd(fd_loop);
		    for (;;) {
			handle_events();
			if (kill_link && !persist)
			    break;
			if (get_loop_output())
			    break;
		    }

		    remove_fd(fd_loop);
		    if (kill_link && !persist)
			break;

		    /*
		     * Now we want to bring up the link.
		     */
		    demand_block();
		    info("Starting link");
		}
	}
	//END:modify by zhourongfei
	// brcm
	demandBegin=0;

	// brcm
	printf("PPP: PPP%s Start to connect ...\n", req_name);

	new_phase(PHASE_SERIALCONN);

	devfd = the_channel->connect();
	if (devfd < 0)
	    goto fail;

        if ( autoscan)
        {
            goto disconnect;
        }

	/* set up the serial device as a ppp interface */
	tdb_writelock(pppdb);
	fd_ppp = the_channel->establish_ppp(devfd);
	if (fd_ppp < 0) {
	    tdb_writeunlock(pppdb);
	    status = EXIT_FATAL_ERROR;
	    goto disconnect;
	}

	if (!demand && ifunit >= 0)
	    set_ifunit(1);
	tdb_writeunlock(pppdb);

	/*
	 * Start opening the connection and wait for
	 * incoming events (reply, timeout, etc.).
	 */
	notice("Connect: %s <--> %s", ifname, ppp_devnam);
	gettimeofday(&start_time, NULL);
	link_stats_valid = 0;
	script_unsetenv("CONNECT_TIME");
	script_unsetenv("BYTES_SENT");
	script_unsetenv("BYTES_RCVD");
	lcp_lowerup(0);

	add_fd(fd_ppp);
	lcp_open(0);		/* Start protocol */
	status = EXIT_NEGOTIATION_FAILED;
	new_phase(PHASE_ESTABLISH);
	
	if ( pppoeWan &&  nPppAuthPass )     /*begin  modify by xia 133940 2008年10月20日*/ 
	{
		//thisCurrTime();    
		
	}
	
	while (phase != PHASE_DEAD) {
	    handle_events();
	    get_input();
	    if (kill_link  ) 
	    	{	    	   
			   lcp_close(0, "User request");   		    
	    	}
// brcm
#if 0
	    if (open_ccp_flag) {
		if (phase == PHASE_NETWORK || phase == PHASE_RUNNING) {
		    ccp_fsm[0].flags = OPT_RESTART; /* clears OPT_SILENT */
		    (*ccp_protent.open)(0);
		}
	    }
#endif
	}

	/*
	 * Print connect time and statistics.
	 */
	if (link_stats_valid) {
	    int t = (link_connect_time + 5) / 6;    /* 1/10ths of minutes */
	    info("Connect time %d.%d minutes.", t/10, t%10);
	    info("Sent %u bytes, received %u bytes.",
		 link_stats.bytes_out, link_stats.bytes_in);
	}

	/*
	 * Delete pid file before disestablishing ppp.  Otherwise it
	 * can happen that another pppd gets the same unit and then
	 * we delete its pid file.
	 */
	if (!demand) {
	    if (pidfilename[0] != 0
		&& unlink(pidfilename) < 0 && errno != ENOENT)
		warn("unable to delete pid file %s: %m", pidfilename);
	    pidfilename[0] = 0;
	}

	/*
	 * If we may want to bring the link up again, transfer
	 * the ppp unit back to the loopback.  Set the
	 * real serial device back to its normal mode of operation.
	 */
	remove_fd(fd_ppp);
	clean_check();
	the_channel->disestablish_ppp(devfd);
	fd_ppp = -1;
	if (!hungup)
	    lcp_lowerdown(0);
	if (!demand)
	    script_unsetenv("IFNAME");

	/*
	 * Run disconnector script, if requested.
	 * XXX we may not be able to do this if the line has hung up!
	 */
    disconnect:
        VDF_DBG("PPP:%s:%s:%d:disconnect\n",__FILE__,__FUNCTION__,__LINE__);
	new_phase(PHASE_DISCONNECT);
	the_channel->disconnect();

    fail:
        VDF_DBG("PPP:%s:%s:%d:fail\n",__FILE__,__FUNCTION__,__LINE__);
	if (the_channel->cleanup)
	    (*the_channel->cleanup)();

	if (!demand) {
	    if (pidfilename[0] != 0
		&& unlink(pidfilename) < 0 && errno != ENOENT)
		warn("unable to delete pid file %s: %m", pidfilename);
	    pidfilename[0] = 0;
	}

	if (!persist || (maxfail > 0 && unsuccess >= maxfail))
	// brcm
	    ;
	//    printf("PPP: fail test\n");
	//    break;
	if ( autoscan)
	{
	    break;
	}

	if (demand)
	    demand_discard();
	t = need_holdoff? holdoff: 0;
	if (holdoff_hook)
	    t = (*holdoff_hook)();
	if (t > 0) {
	    new_phase(PHASE_HOLDOFF);
	    TIMEOUT(holdoff_end, NULL, t);
	    do {
		handle_events();
		if (kill_link)
		    new_phase(PHASE_DORMANT); /* allow signal to end holdoff */
	    } while (phase == PHASE_HOLDOFF);
	  //  if (!persist)       /*delete by xia 133940 2008年10月21日*/ 
		  //  sleep(2);
	}	


	/*start add by xia 133940 2008年9月3日*/
	//作用:当点击断开按钮后删掉计时文件，以免还会有在线时间显示


/*begin  modify by xia 133940 2008年10月17日V100R001C01B020  AU8D00972*/ 
/*	if ( delayRedial<=4 )
		delayRedial = 0;
	else
		delayRedial = delayRedial -4;*/
  if ( pppoeWan )
  { 
      if (0 != delayRedial)
	  {
	      int itemp=0;
					
		  for (  itemp=1 ; itemp<=delayRedial ; itemp++ )		
		  {
			//printf("\nprint in %s at %d line,  delayRedial  is : ##########  %d  ##########\n",__FILE__ , __LINE__, itemp );
			sleep(1);
		  }
	  }
  }
/*end  modify by xia 133940 2008年10月17日V100R001C01B020  AU8D00972*/ 

	   
	
    }

    /* Wait for scripts to finish */
    /* XXX should have a timeout here */
    while (n_children > 0) {
	if (debug) {
	    struct subprocess *chp;
	    dbglog("Waiting for %d child processes...", n_children);
	    for (chp = children; chp != NULL; chp = chp->next)
		dbglog("  script %s, pid %d", chp->prog, chp->pid);
	}
	if (reap_kids(1) < 0)
	    break;
    }

    die(status);
    return 0;
}

/*
 * handle_events - wait for something to happen and respond to it.
 */
static void
handle_events()
{
    struct timeval timo;
    sigset_t mask;

    kill_link = open_ccp_flag = 0;
    if (sigsetjmp(sigjmp, 1) == 0) {
	sigprocmask(SIG_BLOCK, &mask, NULL);
	if (got_sighup || got_sigterm || got_sigusr2 || got_sigchld) {
	    sigprocmask(SIG_UNBLOCK, &mask, NULL);
	} else {
	    waiting = 1;
	    sigprocmask(SIG_UNBLOCK, &mask, NULL);
	    wait_input(timeleft(&timo));
	}
    }
    waiting = 0;
    calltimeout();
    if (got_sighup) 
    	{
		kill_link = 1;
		got_sighup = 0;
		if (status != EXIT_HANGUP)
		{
			status = EXIT_USER_REQUEST;
			/* BEGIN: Added by hKF20513, 2009/11/12   PN:记录PPP连接失败的原因*/
			sprintf(LastErr,"%s/%s/%s","echo ERROR_USER_DISCONNECT > /var/fyi/sys",session_path,"LastConnectionError");
			system(LastErr);      
			/* END:   Added by hKF20513, 2009/11/12 */
		}
    }
    if (got_sigterm) 
    {
		kill_link = 1;
		persist = 0;
		status = EXIT_USER_REQUEST;
		/* BEGIN: Added by hKF20513, 2009/11/12   PN:记录PPP连接失败的原因*/
		sprintf(LastErr,"%s/%s/%s","echo ERROR_USER_DISCONNECT > /var/fyi/sys",session_path,"LastConnectionError");
		system(LastErr);      
		/* END:   Added by hKF20513, 2009/11/12 */
		got_sigterm = 0;
    }
    if (got_sigchld) {
	reap_kids(0);	/* Don't leave dead kids lying around */
	got_sigchld = 0;
    }
    if (got_sigusr2) {
	open_ccp_flag = 1;
	got_sigusr2 = 0;
    }
}

/*
 * setup_signals - initialize signal handling.
 */
static void
setup_signals()
{
    struct sigaction sa;
    sigset_t mask;

    /*
     * Compute mask of all interesting signals and install signal handlers
     * for each.  Only one signal handler may be active at a time.  Therefore,
     * all other signals should be masked when any handler is executing.
     */
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGUSR2);

#define SIGNAL(s, handler)	do { \
	sa.sa_handler = handler; \
	if (sigaction(s, &sa, NULL) < 0) \
	    fatal("Couldn't establish signal handler (%d): %m", s); \
    } while (0)

    sa.sa_mask = mask;
    sa.sa_flags = 0;
    SIGNAL(SIGHUP, hup);		/* Hangup */
    SIGNAL(SIGINT, SIG_IGN);		/* Interrupt */
    SIGNAL(SIGTERM, term);		/* Terminate */
    SIGNAL(SIGCHLD, chld);

    SIGNAL(SIGUSR1, toggle_debug);	/* Toggle debug flag */
    SIGNAL(SIGUSR2, open_ccp);		/* Reopen CCP */

    /*
     * Install a handler for other signals which would otherwise
     * cause pppd to exit without cleaning up.
     */
    SIGNAL(SIGABRT, bad_signal);
    SIGNAL(SIGALRM, alarm_signal);
    SIGNAL(SIGFPE, bad_signal);
    SIGNAL(SIGILL, toggle_hspavoice);
    SIGNAL(SIGPIPE, bad_signal);
    SIGNAL(SIGQUIT, bad_signal);
    SIGNAL(SIGSEGV, bad_signal);
#ifdef SIGBUS
    SIGNAL(SIGBUS, bad_signal);
#endif
#ifdef SIGEMT
    SIGNAL(SIGEMT, bad_signal);
#endif
#ifdef SIGPOLL
    SIGNAL(SIGPOLL, bad_signal);
#endif
#ifdef SIGPROF
    SIGNAL(SIGPROF, bad_signal);
#endif
#ifdef SIGSYS
    SIGNAL(SIGSYS, bad_signal);
#endif
#ifdef SIGTRAP
    SIGNAL(SIGTRAP, bad_signal);
#endif
#ifdef SIGVTALRM
    SIGNAL(SIGVTALRM, bad_signal);
#endif
#ifdef SIGXCPU
    SIGNAL(SIGXCPU, bad_signal);
#endif
#ifdef SIGXFSZ
    SIGNAL(SIGXFSZ, bad_signal);
#endif

    /*
     * Apparently we can get a SIGPIPE when we call syslog, if
     * syslogd has died and been restarted.  Ignoring it seems
     * be sufficient.
     */
    signal(SIGPIPE, SIG_IGN);
}

/*
 * set_ifunit - do things we need to do once we know which ppp
 * unit we are using.
 */
void
set_ifunit(iskey)
    int iskey;
{
// brcm
    info("Using interface %s%s", PPP_DRV_NAME, req_name);
    slprintf(ifname, sizeof(ifname), "%s_%s", PPP_DRV_NAME, req_name);
//    info("Using interface %s%d", PPP_DRV_NAME, ifunit);
//    slprintf(ifname, sizeof(ifname), "%s%d", PPP_DRV_NAME, ifunit);
    script_setenv("IFNAME", ifname, iskey);
//    if (iskey) {
//	create_pidfile();	/* write pid to file */
//	create_linkpidfile();
//    }
}

/*
 * detach - detach us from the controlling terminal.
 */
void
detach()
{
    int pid;
    char numbuf[16];

    if (detached)
	return;
    if ((pid = fork()) < 0) {
	error("Couldn't detach (fork failed: %m)");
	die(1);			/* or just return? */
    }
    if (pid != 0) {
	/* parent */
	notify(pidchange, pid);
	exit(0);		/* parent dies */
    }
    setsid();
    chdir("/");
    close(0);
    close(1);
    close(2);
    detached = 1;
    if (log_default)
	log_to_fd = -1;
    /* update pid files if they have been written already */
//    if (pidfilename[0])
//	create_pidfile();
//    if (linkpidfile[0])
//	create_linkpidfile();
    slprintf(numbuf, sizeof(numbuf), "%d", getpid());
    script_setenv("PPPD_PID", numbuf, 1);
}

/*
 * reopen_log - (re)open our connection to syslog.
 */
void
reopen_log()
{
#ifdef ULTRIX
    openlog("pppd", LOG_PID);
#else
    openlog("pppd", LOG_PID | LOG_NDELAY, LOG_PPP);
    setlogmask(LOG_UPTO(LOG_INFO));
#endif
}

void setPid() {
    char path[128]="";
    char cmd[128] = "";
    
    sprintf(path, "%s/%s/%s", "/proc/var/fyi/wan", session_path, "pid");
    sprintf(cmd, "echo %d > %s", getpid(), path);
    system(cmd); 
}

#if 0
/*
 * Create a file containing our process ID.
 */
static void
create_pidfile()
{
    FILE *pidfile;

    slprintf(pidfilename, sizeof(pidfilename), "%s%s.pid",
	     _PATH_VARRUN, ifname);
    if ((pidfile = fopen(pidfilename, "w")) != NULL) {
	fprintf(pidfile, "%d\n", getpid());
	(void) fclose(pidfile);
    } else {
	error("Failed to create pid file %s: %m", pidfilename);
	pidfilename[0] = 0;
    }
}

static void
create_linkpidfile()
{
    FILE *pidfile;

    if (linkname[0] == 0)
	return;
    script_setenv("LINKNAME", linkname, 1);
    slprintf(linkpidfile, sizeof(linkpidfile), "%sppp-%s.pid",
	     _PATH_VARRUN, linkname);
    if ((pidfile = fopen(linkpidfile, "w")) != NULL) {
	fprintf(pidfile, "%d\n", getpid());
	if (ifname[0])
	    fprintf(pidfile, "%s\n", ifname);
	(void) fclose(pidfile);
    } else {
	error("Failed to create pid file %s: %m", linkpidfile);
	linkpidfile[0] = 0;
    }
}
#endif


/*
 * holdoff_end - called via a timeout when the holdoff period ends.
 */
static void
holdoff_end(arg)
    void *arg;
{
    new_phase(PHASE_DORMANT);
}

/* List of protocol names, to make our messages a little more informative. */
struct protocol_list {
    u_short	proto;
    const char	*name;
} protocol_list[] = {
    { 0x21,	"IP" },
    { 0x23,	"OSI Network Layer" },
    { 0x25,	"Xerox NS IDP" },
    { 0x27,	"DECnet Phase IV" },
    { 0x29,	"Appletalk" },
    { 0x2b,	"Novell IPX" },
    { 0x2d,	"VJ compressed TCP/IP" },
    { 0x2f,	"VJ uncompressed TCP/IP" },
    { 0x31,	"Bridging PDU" },
    { 0x33,	"Stream Protocol ST-II" },
    { 0x35,	"Banyan Vines" },
    { 0x39,	"AppleTalk EDDP" },
    { 0x3b,	"AppleTalk SmartBuffered" },
    { 0x3d,	"Multi-Link" },
    { 0x3f,	"NETBIOS Framing" },
    { 0x41,	"Cisco Systems" },
    { 0x43,	"Ascom Timeplex" },
    { 0x45,	"Fujitsu Link Backup and Load Balancing (LBLB)" },
    { 0x47,	"DCA Remote Lan" },
    { 0x49,	"Serial Data Transport Protocol (PPP-SDTP)" },
    { 0x4b,	"SNA over 802.2" },
    { 0x4d,	"SNA" },
    { 0x4f,	"IP6 Header Compression" },
    { 0x6f,	"Stampede Bridging" },
    { 0xfb,	"single-link compression" },
    { 0xfd,	"1st choice compression" },
    { 0x0201,	"802.1d Hello Packets" },
    { 0x0203,	"IBM Source Routing BPDU" },
    { 0x0205,	"DEC LANBridge100 Spanning Tree" },
    { 0x0231,	"Luxcom" },
    { 0x0233,	"Sigma Network Systems" },
    { 0x8021,	"Internet Protocol Control Protocol" },
    { 0x8023,	"OSI Network Layer Control Protocol" },
    { 0x8025,	"Xerox NS IDP Control Protocol" },
    { 0x8027,	"DECnet Phase IV Control Protocol" },
    { 0x8029,	"Appletalk Control Protocol" },
    { 0x802b,	"Novell IPX Control Protocol" },
    { 0x8031,	"Bridging NCP" },
    { 0x8033,	"Stream Protocol Control Protocol" },
    { 0x8035,	"Banyan Vines Control Protocol" },
    { 0x803d,	"Multi-Link Control Protocol" },
    { 0x803f,	"NETBIOS Framing Control Protocol" },
    { 0x8041,	"Cisco Systems Control Protocol" },
    { 0x8043,	"Ascom Timeplex" },
    { 0x8045,	"Fujitsu LBLB Control Protocol" },
    { 0x8047,	"DCA Remote Lan Network Control Protocol (RLNCP)" },
    { 0x8049,	"Serial Data Control Protocol (PPP-SDCP)" },
    { 0x804b,	"SNA over 802.2 Control Protocol" },
    { 0x804d,	"SNA Control Protocol" },
    { 0x804f,	"IP6 Header Compression Control Protocol" },
    { 0x006f,	"Stampede Bridging Control Protocol" },
    { 0x80fb,	"Single Link Compression Control Protocol" },
    { 0x80fd,	"Compression Control Protocol" },
    { 0xc021,	"Link Control Protocol" },
    { 0xc023,	"Password Authentication Protocol" },
    { 0xc025,	"Link Quality Report" },
    { 0xc027,	"Shiva Password Authentication Protocol" },
    { 0xc029,	"CallBack Control Protocol (CBCP)" },
    { 0xc081,	"Container Control Protocol" },
    { 0xc223,	"Challenge Handshake Authentication Protocol" },
    { 0xc281,	"Proprietary Authentication Protocol" },
    { 0,	NULL },
};

/*
 * protocol_name - find a name for a PPP protocol.
 */
const char *
protocol_name(proto)
    int proto;
{
    struct protocol_list *lp;

    for (lp = protocol_list; lp->proto != 0; ++lp)
	if (proto == lp->proto)
	    return lp->name;
    return NULL;
}

/*
 * get_input - called when incoming data is available.
 */
static void
get_input()
{
    int len, i;
    u_char *p;
    u_short protocol;
    struct protent *protp;

    p = inpacket_buf;	/* point to beginning of packet buffer */

    len = read_packet(inpacket_buf);
    if (len < 0)
	return;

    if (len == 0) {
	notice("Modem hangup");
	hungup = 1;
	status = EXIT_HANGUP;
	lcp_lowerdown(0);	/* serial link is no longer available */
	link_terminated(0);
	return;
    }

    if (debug /*&& (debugflags & DBG_INPACKET)*/)
	dbglog("rcvd %P", p, len);

    if (len < PPP_HDRLEN) {
	MAINDEBUG(("io(): Received short packet."));
	return;
    }

    p += 2;				/* Skip address and control */
    GETSHORT(protocol, p);
    len -= PPP_HDRLEN;

    /*
     * Toss all non-LCP packets unless LCP is OPEN.
     */
    if (protocol != PPP_LCP && lcp_fsm[0].state != OPENED) {
	MAINDEBUG(("get_input: Received non-LCP packet when LCP not open."));
	return;
    }

    /*
     * Until we get past the authentication phase, toss all packets
     * except LCP, LQR and authentication packets.
     */
    if (phase <= PHASE_AUTHENTICATE
	&& !(protocol == PPP_LCP || protocol == PPP_LQR
	     || protocol == PPP_PAP || protocol == PPP_CHAP)) {
	MAINDEBUG(("get_input: discarding proto 0x%x in phase %d",
		   protocol, phase));
	return;
    }

    /*
     * Upcall the proper protocol input routine.
     */
    for (i = 0; (protp = protocols[i]) != NULL; ++i) {
	if (protp->protocol == protocol && protp->enabled_flag) {
	    (*protp->input)(0, p, len);
	    return;
	}
        if (protocol == (protp->protocol & ~0x8000) && protp->enabled_flag
	    && protp->datainput != NULL) {
	    (*protp->datainput)(0, p, len);
	    return;
	}
    }

    if (debug) {
	const char *pname = protocol_name(protocol);
	if (pname != NULL)
	    warn("Unsupported protocol '%s' (0x%x) received", pname, protocol);
	else
	    warn("Unsupported protocol 0x%x received", protocol);
    }
    lcp_sprotrej(0, p - PPP_HDRLEN, len + PPP_HDRLEN);
}

/*
 * new_phase - signal the start of a new phase of pppd's operation.
 */
void
new_phase(p)
    int p;
{
    phase = p;
    if (new_phase_hook)
	(*new_phase_hook)(p);
    notify(phasechange, p);
}

/*
 * die - clean up state and exit with the specified status.
 */
void
die(status)
    int status;
{
    cleanup();
    notify(exitnotify, status);
    syslog(LOG_INFO, "Exit.");
    exit(status);
}

/*
 * cleanup - restore anything which needs to be restored before we exit
 */
/* ARGSUSED */
static void
cleanup()
{
    sys_cleanup();

    if (fd_ppp >= 0)
	the_channel->disestablish_ppp(devfd);
    if (the_channel->cleanup)
	(*the_channel->cleanup)();

    if (pidfilename[0] != 0 && unlink(pidfilename) < 0 && errno != ENOENT)
	warn("unable to delete pid file %s: %m", pidfilename);
    pidfilename[0] = 0;
    if (linkpidfile[0] != 0 && unlink(linkpidfile) < 0 && errno != ENOENT)
	warn("unable to delete pid file %s: %m", linkpidfile);
    linkpidfile[0] = 0;

    if (pppdb != NULL)
	cleanup_db();
}

/*
 * update_link_stats - get stats at link termination.
 */
void
update_link_stats(u)
    int u;
{
    struct timeval now;
    char numbuf[32];

    if (!get_ppp_stats(u, &link_stats)
	|| gettimeofday(&now, NULL) < 0)
	return;
    link_connect_time = now.tv_sec - start_time.tv_sec;
    link_stats_valid = 1;

    slprintf(numbuf, sizeof(numbuf), "%d", link_connect_time);
    script_setenv("CONNECT_TIME", numbuf, 0);
    slprintf(numbuf, sizeof(numbuf), "%d", link_stats.bytes_out);
    script_setenv("BYTES_SENT", numbuf, 0);
    slprintf(numbuf, sizeof(numbuf), "%d", link_stats.bytes_in);
    script_setenv("BYTES_RCVD", numbuf, 0);
}


struct	callout {
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct		callout *c_next;
};

static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of milliseconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, secs, usecs)
    void (*func) __P((void *));
    void *arg;
    int secs, usecs;
{
    struct callout *newp, *p, **pp;

    MAINDEBUG(("Timeout %p:%p in %d.%03d seconds.", func, arg,
	       secs, usecs));

    /*
     * Allocate timeout.
     */
    if ((newp = (struct callout *) malloc(sizeof(struct callout))) == NULL)
	fatal("Out of memory in timeout()!");
    newp->c_arg = arg;
    newp->c_func = func;
    gettimeofday(&timenow, NULL);
    newp->c_time.tv_sec = timenow.tv_sec + secs;
    newp->c_time.tv_usec = timenow.tv_usec + usecs;
    if (newp->c_time.tv_usec >= 1000000) {
	newp->c_time.tv_sec += newp->c_time.tv_usec / 1000000;
	newp->c_time.tv_usec %= 1000000;
    }

    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (newp->c_time.tv_sec < p->c_time.tv_sec
	    || (newp->c_time.tv_sec == p->c_time.tv_sec
		&& newp->c_time.tv_usec < p->c_time.tv_usec))
	    break;
    newp->c_next = p;
    *pp = newp;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(func, arg)
    void (*func) __P((void *));
    void *arg;
{
    struct callout **copp, *freep;

    MAINDEBUG(("Untimeout %p:%p.", func, arg));

    /*
     * Find first matching timeout and remove it from the list.
     */
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep->c_func == func && freep->c_arg == arg) {
	    *copp = freep->c_next;
	    free((char *) freep);
	    break;
	}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
static void
calltimeout()
{
    struct callout *p;

    while (callout != NULL) {
	p = callout;

	if (gettimeofday(&timenow, NULL) < 0)
	    fatal("Failed to get time of day: %m");
	if (!(p->c_time.tv_sec < timenow.tv_sec
	      || (p->c_time.tv_sec == timenow.tv_sec
		  && p->c_time.tv_usec <= timenow.tv_usec)))
	    break;		/* no, it's not time yet */

	callout = p->c_next;
	(*p->c_func)(p->c_arg);

	free((char *) p);
    }
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}


/*
 * kill_my_pg - send a signal to our process group, and ignore it ourselves.
 */
static void
kill_my_pg(sig)
    int sig;
{
    struct sigaction act, oldact;

    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    kill(0, sig);
    sigaction(sig, &act, &oldact);
    sigaction(sig, &oldact, NULL);
}


/*
 * hup - Catch SIGHUP signal.
 *
 * Indicates that the physical layer has been disconnected.
 * We don't rely on this indication; if the user has sent this
 * signal, we just take the link down.
 */
static void
hup(sig)
    int sig;
{
	/*begin  modify by xia 133940 2008年9月18日*/ 
	char cmd[FILE_PATH_LEN] = "";
	char path[FILE_PATH_LEN] = "";
	/*end  modify by xia 133940 2008年9月18日*/ 
	
   // info("Hangup (SIGHUP)");
   // printf("\n\n PPP:%s:%s:%d:   SIGHUP!!!\n",__FILE__,__FUNCTION__,__LINE__);
    
    if(usbWan)
    {
        //0-hapa不拨号
        system("echo 0 >/var/HspaDialFlag");
    }


	//add by xia 133940 2008年8月20日，将状态文件置0，表示按下了断开按钮
	if ( pppoeWan )
	{
            sprintf(path, "%s%s%s", "/var/", session_path, "_status");    	
            sprintf(cmd, "echo 0 > %s",  path);
            system(cmd); 
            sprintf(oldsession, "%s", "");
	}
	//end
    
    
    got_sighup = 1;
    if (conn_running)
	/* Send the signal to the [dis]connector process(es) also */
	kill_my_pg(sig);
    notify(sigreceived, sig);
    if (waiting)
	siglongjmp(sigjmp, 1);
}


/*
 * term - Catch SIGTERM signal and SIGINT signal (^C/del).
 *
 * Indicates that we should initiate a graceful disconnect and exit.
 */
/*ARGSUSED*/
static void
term(sig)
    int sig;
{
    info("Terminating on signal %d.", sig);
    got_sigterm = 1;
    if (conn_running)
	/* Send the signal to the [dis]connector process(es) also */
	kill_my_pg(sig);
    notify(sigreceived, sig);
    if (waiting)
	siglongjmp(sigjmp, 1);
}


/*
 * chld - Catch SIGCHLD signal.
 * Sets a flag so we will call reap_kids in the mainline.
 */
static void
chld(sig)
    int sig;
{
    got_sigchld = 1;
    if (waiting)
	siglongjmp(sigjmp, 1);
}


/*
 * toggle_debug - Catch SIGUSR1 signal.
 *
 * Toggle debug flag.
 */
/*ARGSUSED*/
static void
toggle_debug(sig)
    int sig;
{
    debug = !debug;
    printf("PPP:recieved SIGUSR1.   debug=%d\n",debug);
    if (debug) {
	setlogmask(LOG_UPTO(LOG_DEBUG));
    } else {
	setlogmask(LOG_UPTO(LOG_WARNING));
    }
}


/*
 * open_ccp - Catch SIGUSR2 signal.
 *
 * Try to (re)negotiate compression.
 */
/*ARGSUSED*/
static void
open_ccp(sig)
    int sig;
{
    got_sigusr2 = 1;
    if (waiting)
	siglongjmp(sigjmp, 1);
}

static void
toggle_hspavoice(sig)
    int sig;
{
    g_hspavoice_flg = 1;
}

/*
 * bad_signal - We've caught a fatal signal.  Clean up state and exit.
 */
static void
bad_signal(sig)
    int sig;
{
    static int crashed = 0;

    if (crashed)
	_exit(127);
    crashed = 1;
    error("Fatal signal %d", sig);
    if (conn_running)
	kill_my_pg(SIGTERM);
    notify(sigreceived, sig);
    die(127);
}

static void
alarm_signal(sig)
    int sig;
{
	printf("\n!!!AT TIME OUT !!!\n");
}


/*
 * device_script - run a program to talk to the specified fds
 * (e.g. to run the connector or disconnector script).
 * stderr gets connected to the log fd or to the _PATH_CONNERRS file.
 */
int
device_script(program, in, out, dont_wait)
    char *program;
    int in, out;
    int dont_wait;
{
    int pid, fd;
    int status = -1;
    int errfd;

    ++conn_running;
    pid = fork();

    if (pid < 0) {
	--conn_running;
	error("Failed to create child process: %m");
	return -1;
    }

    if (pid != 0) {
	if (dont_wait) {
	    record_child(pid, program, NULL, NULL);
	    status = 0;
	} else {
	    while (waitpid(pid, &status, 0) < 0) {
		if (errno == EINTR)
		    continue;
		fatal("error waiting for (dis)connection process: %m");
	    }
	    --conn_running;
	}
	return (status == 0 ? 0 : -1);
    }

    /* here we are executing in the child */
    /* make sure fds 0, 1, 2 are occupied */
    while ((fd = dup(in)) >= 0) {
	if (fd > 2) {
	    close(fd);
	    break;
	}
    }

    /* dup in and out to fds > 2 */
    in = dup(in);
    out = dup(out);
    if (log_to_fd >= 0) {
	errfd = dup(log_to_fd);
    } else {
	errfd = open(_PATH_CONNERRS, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }

    /* close fds 0 - 2 and any others we can think of */
    close(0);
    close(1);
    close(2);
    sys_close();
    if (the_channel->close)
	(*the_channel->close)();
    closelog();

    /* dup the in, out, err fds to 0, 1, 2 */
    dup2(in, 0);
    close(in);
    dup2(out, 1);
    close(out);
    if (errfd >= 0) {
	dup2(errfd, 2);
	close(errfd);
    }

    setuid(uid);
    if (getuid() != uid) {
	error("setuid failed");
	exit(1);
    }
    setgid(getgid());
    execl("/bin/sh", "sh", "-c", program, (char *)0);
    error("could not exec /bin/sh: %m");
    exit(99);
    /* NOTREACHED */
}


/*
 * run-program - execute a program with given arguments,
 * but don't wait for it.
 * If the program can't be executed, logs an error unless
 * must_exist is 0 and the program file doesn't exist.
 * Returns -1 if it couldn't fork, 0 if the file doesn't exist
 * or isn't an executable plain file, or the process ID of the child.
 * If done != NULL, (*done)(arg) will be called later (within
 * reap_kids) iff the return value is > 0.
 */
pid_t
run_program(prog, args, must_exist, done, arg)
    char *prog;
    char **args;
    int must_exist;
    void (*done) __P((void *));
    void *arg;
{
    int pid;
    struct stat sbuf;

    /*
     * First check if the file exists and is executable.
     * We don't use access() because that would use the
     * real user-id, which might not be root, and the script
     * might be accessible only to root.
     */
    errno = EINVAL;
    if (stat(prog, &sbuf) < 0 || !S_ISREG(sbuf.st_mode)
	|| (sbuf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) == 0) {
	if (must_exist || errno != ENOENT)
	    warn("Can't execute %s: %m", prog);
	return 0;
    }

    pid = fork();
    if (pid == -1) {
	error("Failed to create child process for %s: %m", prog);
	return -1;
    }
    if (pid == 0) {
	int new_fd;

	/* Leave the current location */
	(void) setsid();	/* No controlling tty. */
	(void) umask (S_IRWXG|S_IRWXO);
	(void) chdir ("/");	/* no current directory. */
	setuid(0);		/* set real UID = root */
	setgid(getegid());

	/* Ensure that nothing of our device environment is inherited. */
	sys_close();
	closelog();
	close (0);
	close (1);
	close (2);
	if (the_channel->close)
	    (*the_channel->close)();

        /* Don't pass handles to the PPP device, even by accident. */
	new_fd = open (_PATH_DEVNULL, O_RDWR);
	if (new_fd >= 0) {
	    if (new_fd != 0) {
	        dup2  (new_fd, 0); /* stdin <- /dev/null */
		close (new_fd);
	    }
	    dup2 (0, 1); /* stdout -> /dev/null */
	    dup2 (0, 2); /* stderr -> /dev/null */
	}

#ifdef BSD
	/* Force the priority back to zero if pppd is running higher. */
	if (setpriority (PRIO_PROCESS, 0, 0) < 0)
	    warn("can't reset priority to 0: %m");
#endif

	/* SysV recommends a second fork at this point. */

	/* run the program */
	execve(prog, args, script_env);
	if (must_exist || errno != ENOENT) {
	    /* have to reopen the log, there's nowhere else
	       for the message to go. */
	    reopen_log();
	    syslog(LOG_ERR, "Can't execute %s: %m", prog);
	    closelog();
	}
	_exit(-1);
    }

    if (debug)
	dbglog("Script %s started (pid %d)", prog, pid);
    record_child(pid, prog, done, arg);

    return pid;
}


/*
 * record_child - add a child process to the list for reap_kids
 * to use.
 */
void
record_child(pid, prog, done, arg)
    int pid;
    char *prog;
    void (*done) __P((void *));
    void *arg;
{
    struct subprocess *chp;

    ++n_children;

    chp = (struct subprocess *) malloc(sizeof(struct subprocess));
    if (chp == NULL) {
	warn("losing track of %s process", prog);
    } else {
	chp->pid = pid;
	chp->prog = prog;
	chp->done = done;
	chp->arg = arg;
	chp->next = children;
	children = chp;
    }
}


/*
 * reap_kids - get status from any dead child processes,
 * and log a message for abnormal terminations.
 */
static int
reap_kids(waitfor)
    int waitfor;
{
    int pid, status;
    struct subprocess *chp, **prevp;

    if (n_children == 0)
	return 0;
    while ((pid = waitpid(-1, &status, (waitfor? 0: WNOHANG))) != -1
	   && pid != 0) {
	for (prevp = &children; (chp = *prevp) != NULL; prevp = &chp->next) {
	    if (chp->pid == pid) {
		--n_children;
		*prevp = chp->next;
		break;
	    }
	}
	if (WIFSIGNALED(status)) {
	    warn("Child process %s (pid %d) terminated with signal %d",
		 (chp? chp->prog: "??"), pid, WTERMSIG(status));
	} else if (debug)
	    dbglog("Script %s finished (pid %d), status = 0x%x",
		   (chp? chp->prog: "??"), pid, status);
	if (chp && chp->done)
	    (*chp->done)(chp->arg);
	if (chp)
	    free(chp);
    }
    if (pid == -1) {
	if (errno == ECHILD)
	    return -1;
	if (errno != EINTR)
	    error("Error waiting for child process: %m");
    }
    return 0;
}

/*
 * add_notifier - add a new function to be called when something happens.
 */
void
add_notifier(notif, func, arg)
    struct notifier **notif;
    notify_func func;
    void *arg;
{
    struct notifier *np;

    np = malloc(sizeof(struct notifier));
    if (np == 0)
	novm("notifier struct");
    np->next = *notif;
    np->func = func;
    np->arg = arg;
    *notif = np;
}

/*
 * remove_notifier - remove a function from the list of things to
 * be called when something happens.
 */
void
remove_notifier(notif, func, arg)
    struct notifier **notif;
    notify_func func;
    void *arg;
{
    struct notifier *np;

    for (; (np = *notif) != 0; notif = &np->next) {
	if (np->func == func && np->arg == arg) {
	    *notif = np->next;
	    free(np);
	    break;
	}
    }
}

/*
 * notify - call a set of functions registered with add_notify.
 */
void
notify(notif, val)
    struct notifier *notif;
    int val;
{
    struct notifier *np;

    while ((np = notif) != 0) {
	notif = np->next;
	(*np->func)(np->arg, val);
    }
}

/*
 * novm - log an error message saying we ran out of memory, and die.
 */
void
novm(msg)
    char *msg;
{
    fatal("Virtual memory exhausted allocating %s\n", msg);
}

/*
 * script_setenv - set an environment variable value to be used
 * for scripts that we run (e.g. ip-up, auth-up, etc.)
 */
void
script_setenv(var, value, iskey)
    char *var, *value;
    int iskey;
{
// brcm
#if 0
    size_t varl = strlen(var);
    size_t vl = varl + strlen(value) + 2;
    int i;
    char *p, *newstring;

    newstring = (char *) malloc(vl+1);
    if (newstring == 0)
	return;
    *newstring++ = iskey;
    slprintf(newstring, vl, "%s=%s", var, value);

    /* check if this variable is already set */
    if (script_env != 0) {
	for (i = 0; (p = script_env[i]) != 0; ++i) {
	    if (strncmp(p, var, varl) == 0 && p[varl] == '=') {
		if (p[-1] && pppdb != NULL)
		    delete_db_key(p);
		free(p-1);
		script_env[i] = newstring;
		if (iskey && pppdb != NULL)
		    add_db_key(newstring);
		update_db_entry();
		return;
	    }
	}
    } else {
	/* no space allocated for script env. ptrs. yet */
	i = 0;
	script_env = (char **) malloc(16 * sizeof(char *));
	if (script_env == 0)
	    return;
	s_env_nalloc = 16;
    }

    /* reallocate script_env with more space if needed */
    if (i + 1 >= s_env_nalloc) {
	int new_n = i + 17;
	char **newenv = (char **) realloc((void *)script_env,
					  new_n * sizeof(char *));
	if (newenv == 0)
	    return;
	script_env = newenv;
	s_env_nalloc = new_n;
    }

    script_env[i] = newstring;
    script_env[i+1] = 0;

    if (pppdb != NULL) {
	if (iskey)
	    add_db_key(newstring);
	update_db_entry();
    }
// brcm
#endif
}

/*
 * script_unsetenv - remove a variable from the environment
 * for scripts.
 */
void
script_unsetenv(var)
    char *var;
{
// brcm
#if 0
    int vl = strlen(var);
    int i;
    char *p;

    if (script_env == 0)
	return;
    for (i = 0; (p = script_env[i]) != 0; ++i) {
	if (strncmp(p, var, vl) == 0 && p[vl] == '=') {
	    if (p[-1] && pppdb != NULL)
		delete_db_key(p);
	    free(p-1);
	    while ((script_env[i] = script_env[i+1]) != 0)
		++i;
	    break;
	}
    }
    if (pppdb != NULL)
	update_db_entry();
// brcm
#endif
}

/*
 * update_db_entry - update our entry in the database.
 */
static void
update_db_entry()
{
// brcm
#if 0
    TDB_DATA key, dbuf;
    int vlen, i;
    char *p, *q, *vbuf;

    if (script_env == NULL)
	return;
    vlen = 0;
    for (i = 0; (p = script_env[i]) != 0; ++i)
	vlen += strlen(p) + 1;
    vbuf = malloc(vlen);
    if (vbuf == 0)
	novm("database entry");
    q = vbuf;
    for (i = 0; (p = script_env[i]) != 0; ++i)
	q += slprintf(q, vbuf + vlen - q, "%s;", p);

    key.dptr = db_key;
    key.dsize = strlen(db_key);
    dbuf.dptr = vbuf;
    dbuf.dsize = vlen;
    if (tdb_store(pppdb, key, dbuf, TDB_REPLACE))
	error("tdb_store failed: %s", tdb_error(pppdb));
// brcm
#endif
}

/*
 * add_db_key - add a key that we can use to look up our database entry.
 */
static void
add_db_key(str)
    const char *str;
{
// brcm
#if 0
    TDB_DATA key, dbuf;

    key.dptr = (char *) str;
    key.dsize = strlen(str);
    dbuf.dptr = db_key;
    dbuf.dsize = strlen(db_key);
    if (tdb_store(pppdb, key, dbuf, TDB_REPLACE))
	error("tdb_store key failed: %s", tdb_error(pppdb));
// brcm
#endif
}

/*
 * delete_db_key - delete a key for looking up our database entry.
 */
static void
delete_db_key(str)
    const char *str;
{
// brcm
#if 0
    TDB_DATA key;

    key.dptr = (char *) str;
    key.dsize = strlen(str);
    tdb_delete(pppdb, key);
// brcm
#endif
}

/*
 * cleanup_db - delete all the entries we put in the database.
 */
static void
cleanup_db()
{
// brcm
#if 0
    TDB_DATA key;
    int i;
    char *p;

    key.dptr = db_key;
    key.dsize = strlen(db_key);
    tdb_delete(pppdb, key);
    for (i = 0; (p = script_env[i]) != 0; ++i)
	if (p[-1])
	    delete_db_key(p);
// brcm
#endif
}
#ifdef VDF_PPPOU

/* HUAWEI HGW lichangqing 45517 2005年8月8日"
  */

/*------------------------------------------------------------
  函数原型: void get_cur_link_time(struct timeval* cur_time)
  描述: 获取当前连接时长
  输入: cur_time(用来返回当前连接时长)
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void get_cur_link_time(struct timeval* cur_time)
{
    struct timeval now;

    if ((gettimeofday(&now, NULL)) < 0) {
        ERROR("gettimeofday error");        
        return;
    }

    if (PHASE_RUNNING == phase) {
        cur_time->tv_sec = now.tv_sec - start_time.tv_sec;
    } else {
        cur_time->tv_sec = 0;
    }

	return;
}

// A064D00429 EC506 ADD (by L45517 2005年12月1日 ) BEGIN
// 此问题单是在问题单A064D00313基础上进行的修改
/*------------------------------------------------------------
  函数原型: int get_cur_link_stats(struct ppp_his_stat* cur_stats)
  描述: 获取当前流量
  输入: cur_stats(用来保存连接流量)
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
int get_cur_link_stats(struct ppp_his_stat* got_stats)
{
    int ret;
    // A64D00601 EC506 DEL (by l45517 2006年1月9日" ) BEGIN
    // static int in_stats_overflowing = 0, out_stats_overflowing = 0;
    // A64D00601 EC506 DEL (by l45517 2006年1月9日" ) END
    struct pppd_stats cur_stats;
        
    ret = get_ppp_stats(0, &cur_stats);
    if (ret) {
        
        if (cur_stats.bytes_in >= G1) {
            in_stats_overflowing = 1;
        } else {
            if (in_stats_overflowing) {
                g_pppd_overflow_status.bytes_in += G4;
            }
            in_stats_overflowing = 0;
        }
        
        if (cur_stats.bytes_out >= G1) {
            out_stats_overflowing = 1;
        } else {
            if (out_stats_overflowing) {
                g_pppd_overflow_status.bytes_out += G4;
            }
            out_stats_overflowing = 0;
        }
        
    }

    got_stats->bytes_in  = g_pppd_overflow_status.bytes_in  + cur_stats.bytes_in;
    got_stats->bytes_out = g_pppd_overflow_status.bytes_out + cur_stats.bytes_out;

    if (demand)
    {
        got_stats->bytes_in  -= g_pppd_history_demand_stats.bytes_in;
        got_stats->bytes_out -= g_pppd_history_demand_stats.bytes_out;
    }

	return ret;
}
// A064D00429 EC506 ADD (by L45517 2005年12月1日 ) END


/*------------------------------------------------------------
  函数原型: void ppp_thread_destroy(void)
  描述: pppd线程注销
  输入: 无
  输出: 无
  返回值:无 
-------------------------------------------------------------*/
void ppp_thread_destroy(void)
{
	return;
}
#endif
