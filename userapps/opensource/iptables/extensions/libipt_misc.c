#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_misc.h>
/*add by z67625 增加支持5种攻击方式 start*/
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
/*add by z67625 增加支持5种攻击方式 end*/

#define MISC_TYPE_STR_IPLAND "ip-land"
#define MISC_TYPE_STR_TCPPTSCAN "tcp-pt-scan"
#define MISC_TYPE_STR_DNSURL "dns-url"
#define MISC_TYPE_STR_RTP     "rtp"
#define MISC_TYPE_STR_RTCP     "rtcp"
#define MISC_TYPE_STR_TCPLEN  "tcp-len"
#define MISC_TYPE_STR_UDPLEN  "udp-len"
#define MISC_TYPE_STR_UDPBOMB  "udpbomb"
/*add by z67625 增加支持5种攻击方式 start*/
#define MISC_TYPE_STR_SAMESRCDST "samesrcdst"
#define MISC_TYPE_STR_BRDCSTSRC "brdcstsrc"
#define MISC_TYPE_STR_LANSRCIP "lansrcip"
#define MISC_TYPE_STR_FRGMTHDR "frgmthdr"
/*add by z67625 增加支持5种攻击方式 end*/

static void
help(void)
{
	printf(
"iprange match v%s options:\n"
"  --type                    Match Dos type.'%s','%s','%s','%s',"
"                            '%s','%s','%s','%s' \n"
"                            are  supported, split by ','(comma)\n"
"  --scan-ports              Match number of ports which are requested \n"
"                            together in a special interval, there may \n"
"                            be a port-scan attack\n"
"  --scan-port-expire        Match live time of the scanned ports monitored\n"
"  --dns-url                 Match URL string against DNS request\n"
"  --rtp-factor              Match RTP or RTCP packets, the factor is the number"
"                            of UDP packet be checked\n"
"[!] --tcp-len               Match payload length of TCP packets\n"
"[!] --udp-len               Match payload length of UDP packets\n"
"\n",
IPTABLES_VERSION,
MISC_TYPE_STR_IPLAND, MISC_TYPE_STR_TCPPTSCAN, MISC_TYPE_STR_DNSURL, 
MISC_TYPE_STR_RTP, MISC_TYPE_STR_RTCP, MISC_TYPE_STR_TCPLEN, MISC_TYPE_STR_UDPLEN,
/*add by z67625 增加支持5种攻击方式 start*/
MISC_TYPE_STR_UDPBOMB, MISC_TYPE_STR_SAMESRCDST, MISC_TYPE_STR_BRDCSTSRC,
MISC_TYPE_STR_LANSRCIP, MISC_TYPE_STR_FRGMTHDR);
/*add by z67625 增加支持5种攻击方式 end*/
}

static struct option opts[] = {
	{ "type", 1, 0, '1' },
	{ "scan-ports", 1, 0, '2' },
	{ "scan-port-expire", 1, 0, '3' },
	{ "dns-url", 1, 0, '4' },
    { "rtp-factor", 1, 0, '5'},
    { "tcp-len", 1, 0, '6'},
    { "udp-len", 1, 0, '7'},
	{0}
};

static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
    struct ipt_misc_info *info = (struct ipt_misc_info *)m->data;

	/* Can't cache this. */
	*nfcache |= NFC_UNKNOWN;

	memset(info->dns_url, 0 , MISC_DNS_URL_MAX);
}

/*add by z67625 根据接口名获取ip和子网掩码 start*/
static int getipandmaskbyifc(unsigned int *mask, unsigned int *ipaddr, char *name)
{
   struct ifreq ifr;
   struct in_addr wan_ip;  
   int s = 0;

   if ( name == NULL || mask == NULL||ipaddr == NULL ) 
   {
       printf("%s %d: input pointer is NULL\n", __FUNCTION__, __LINE__);
       return -1;
   }

   if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
   {
       printf("%s %d: get socket failed\n", __FUNCTION__, __LINE__);
       return -1;
   }
  
   /*获取ip地址*/
   strcpy(ifr.ifr_name, name);
   if ( ioctl(s, SIOCGIFADDR, &ifr) < 0 ) 
   {
      printf("%s %d: get ipaddress failed\n", __FUNCTION__, __LINE__);
      close(s);
      return -1;
   }

   if( ifr.ifr_addr.sa_family == AF_INET )
   {  
      wan_ip = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr;
      *ipaddr = wan_ip.s_addr;
   }

   /*获取子网掩码*/
   strcpy(ifr.ifr_name, name);
   if ( ioctl(s, SIOCGIFNETMASK, &ifr) < 0 ) 
   {
      printf("%s %d: get netmask failed\n", __FUNCTION__, __LINE__);
      close(s);
      return -1;
   }

   if( ifr.ifr_addr.sa_family == AF_INET )
   {  
      wan_ip = ((struct sockaddr_in *)&(ifr.ifr_netmask))->sin_addr;
      *mask = wan_ip.s_addr;
   }
   
   close(s);   
   return 0;
}
/*add by z67625 根据接口名获取ip和子网掩码 end*/

static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_misc_info *info = (struct ipt_misc_info *)(*match)->data;
    

	switch (c) {
	case '1':
		if (*flags & MISC_TYPE)
			exit_error(PARAMETER_PROBLEM,
				   "misc match: Only use --type ONCE!");
		*flags |= MISC_TYPE;

		if(strstr(optarg, MISC_TYPE_STR_IPLAND))	
		    info->flag_ipland = 1;

        /*add by z67625 增加5种攻击方式处理 start*/
        if(strstr(optarg, MISC_TYPE_STR_SAMESRCDST))
            info->flag_samesrcdst = 1;
        
        if(strstr(optarg, MISC_TYPE_STR_BRDCSTSRC))
            info->flag_brdcstsrc = 1;
        
        if(strstr(optarg, MISC_TYPE_STR_LANSRCIP))
        {
            info->flag_lansrcip = 1;
            memset(&(info->mask), 0, sizeof(info->mask));
            memset(&(info->lanip), 0, sizeof(info->lanip));
            getipandmaskbyifc(&(info->mask), &(info->lanip),"br0"); 
        }

        if(strstr(optarg, MISC_TYPE_STR_FRGMTHDR))
        {
            info->flag_frgmthdr = 1;
        }
        /*add by z67625 增加5种攻击方式处理 end*/

		if(strstr(optarg, MISC_TYPE_STR_TCPPTSCAN))
		    info->flag_tcpptscan = 1;

		if(strstr(optarg, MISC_TYPE_STR_DNSURL))
		    info->flag_dnsurl= 1;
		
		if(strstr(optarg, MISC_TYPE_STR_RTP))
		    info->flag_rtp= 1;
		
		if(strstr(optarg, MISC_TYPE_STR_RTCP))
		    info->flag_rtcp= 1;

        if(strstr(optarg, MISC_TYPE_STR_TCPLEN))
            info->flag_tcplen= 1;

        if(strstr(optarg, MISC_TYPE_STR_UDPLEN))
            info->flag_udplen= 1;
        
        if(strstr(optarg, MISC_TYPE_STR_UDPBOMB))
            info->flag_udpbomb = 1;
        
		break;

	case '2':
		if (*flags & MISC_PORTS)
			exit_error(PARAMETER_PROBLEM,
				   "misc match: Only use --scan-ports ONCE!");
		*flags |= MISC_PORTS;

        info->ports_ptscan = atol(optarg);
		break;
		
    case '3':
        if (*flags & MISC_PTEXPIRE)
        exit_error(PARAMETER_PROBLEM,
        "misc match: Only use --scan-port-expire ONCE!");
        *flags |= MISC_PTEXPIRE;

        info->expire_ptscan = atol(optarg);
        break;
        
    case '4':
        if (*flags & MISC_DNSURL)
            exit_error(PARAMETER_PROBLEM,
                        "misc match: Only use --dns-url ONCE!");
        *flags |= MISC_DNSURL;

        memset(info->dns_url, 0, MISC_DNS_URL_MAX);
        if(strlen(optarg) < MISC_DNS_URL_MAX)
            strcpy(info->dns_url, optarg);
        else
            strncpy(info->dns_url, optarg, MISC_DNS_URL_MAX - 1);
        break;
        
    case '5':
        if (*flags & MISC_RTPFCTR)
            exit_error(PARAMETER_PROBLEM,
                        "misc match: Only use --rtp-factor ONCE!");
        *flags |= MISC_RTPFCTR;

        info->rtp_diff_factor = atoi(optarg);
        break;
    case '6':
        if(invert)
            info->invert.flag_tcplen = 1;
        
        if (*flags & MISC_TCPLEN)
            exit_error(PARAMETER_PROBLEM,
                        "misc match: Only use --tcp-len ONCE!");
        *flags |= MISC_TCPLEN;

        info->payload = atoi(optarg);
        break;
    case '7':
        if(invert)
            info->invert.flag_udplen = 1;
        
        if (*flags & MISC_UDPLEN)
        exit_error(PARAMETER_PROBLEM,
                    "misc match: Only use --udp-len ONCE!");
        *flags |= MISC_UDPLEN;

        info->payload = atoi(optarg);
        break;
	default:
		return 0;
	}
	return 1;
}

static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "iprange match: You must specify `--type' ");
}

static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_misc_info *info = (struct ipt_misc_info *)match->data;

	printf("misc match type:");
	if (info->flag_ipland) 
	    printf("%s" ,MISC_TYPE_STR_IPLAND);

    /*add by z67625 增加支持5种攻击方式处理 start*/
    if(info->flag_samesrcdst)
        printf("%s", MISC_TYPE_STR_SAMESRCDST);
    
    if(info->flag_brdcstsrc)
        printf("%s", MISC_TYPE_STR_BRDCSTSRC);
    
    if(info->flag_lansrcip)
        printf("%s", MISC_TYPE_STR_LANSRCIP);

    if(info->flag_frgmthdr)
        printf("%s", MISC_TYPE_STR_FRGMTHDR);
    /*add by z67625 增加支持5种攻击方式处理 end*/
	
	if (info->flag_tcpptscan) 
	    printf("%s", MISC_TYPE_STR_TCPPTSCAN);	

	if (info->flag_dnsurl) 
	    printf("%s", MISC_TYPE_STR_DNSURL);	

	if (info->flag_rtp) 
	    printf("%s", MISC_TYPE_STR_RTP);	

	if (info->flag_rtcp) 
	    printf("%s", MISC_TYPE_STR_RTCP);	

	if(info->flag_tcplen)
    {
        if(info->invert.flag_tcplen)
            printf(" ! ");
	            
	    printf("%s", MISC_TYPE_STR_TCPLEN);	
    }
	
	if(info->flag_udplen)
    {
        if(info->invert.flag_udplen)
            printf(" ! ");
        
	    printf("%s", MISC_TYPE_STR_UDPLEN);	
    }
	
	if(info->flag_udpbomb)
	    printf("%s", MISC_TYPE_STR_UDPBOMB);	
	
    if (info->flag_tcpptscan) {
		printf(" scan-ports:%u ", info->ports_ptscan);
		printf(" scan-port-expire:%u ", info->expire_ptscan);
	}

	if(info->flag_dnsurl)
	    printf(" dns-url:%s ", info->dns_url);

	if(info->flag_rtp || info->flag_rtcp)
	    printf(" rtp-factor:%d ", info->rtp_diff_factor);

	if(info->flag_tcplen || info->flag_udplen)
	    printf(" payload-len:%d ", info->payload);
	
}

static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_misc_info *info = (struct ipt_misc_info *)match->data;

	if (info->flag_ipland) {
		printf("--type %s", MISC_TYPE_STR_IPLAND);
	}
    /*add by z67625 增加支持5种攻击方式处理 start*/
    if(info->flag_samesrcdst){
        printf("--type %s", MISC_TYPE_STR_SAMESRCDST);
    }
    if(info->flag_brdcstsrc){
        printf("--type %s", MISC_TYPE_STR_BRDCSTSRC);
    }
    if(info->flag_lansrcip){
        printf("--type %s", MISC_TYPE_STR_LANSRCIP);
    }
    if(info->flag_frgmthdr){
        printf("--type %s", MISC_TYPE_STR_FRGMTHDR);
    }
    /*add by z67625 增加支持5种攻击方式处理 end*/
	else if (info->flag_tcpptscan) {
		printf("--type %s --scan-ports %u --scan-port-expire %u", 
		    MISC_TYPE_STR_TCPPTSCAN, info->ports_ptscan, info->expire_ptscan);
	}
}

static struct iptables_match misc
= { NULL,
    "misc",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_misc_info)),
    IPT_ALIGN(sizeof(struct ipt_misc_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};

void _init(void)
{
	register_match(&misc);
}


