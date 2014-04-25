/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#ifdef VDF_OPTION
//add for option125
#include <sys/file.h>
#include "board_api.h"
#endif
/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
#include <signal.h>
/* END:   Added by y67514, 2008/9/18 */
#ifdef SUPPORT_MACMATCHIP
extern _macip_list;
#endif

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
	struct in_addr *addr = arg;
	inet_aton(line, addr);
	return 1;
}


static int read_str(char *line, void *arg)
{
	char **dest = arg;
	int i;
	
	if (*dest) free(*dest);

	*dest = strdup(line);

	/*w44771 delete for allowing server2's option60 ends with space characer, begin, 2006-07-27*/
	/* elimate trailing whitespace */	
	//for (i = strlen(*dest) - 1; i > 0 && isspace((*dest)[i]); i--);
	//(*dest)[i > 0 ? i + 1 : 0] = '\0';
	/*w44771 delete for allowing server2's option60 ends with space characer, end, 2006-07-27*/
	
	return 1;
}

/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
static int read_str2(char *line, void *arg)
{
	strcpy(arg, line);	
	return 1;
}
#endif
/*w44771 add for 第一IP支持5段地址池，end*/


static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	*dest = strtoul(line, NULL, 0);
	return 1;
}


static int read_yn(char *line, void *arg)
{
	char *dest = arg;
	if (!strcasecmp("yes", line) || !strcmp("1", line) || !strcasecmp("true", line))
		*dest = 1;
	else if (!strcasecmp("no", line) || !strcmp("0", line) || !strcasecmp("false", line))
		*dest = 0;
	else return 0;
	
	return 1;
}


/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val;
	char fail;
	struct dhcp_option *option = NULL;
	int length = 0;
	char buffer[255];
	u_int16_t result_u16;
	int16_t result_s16;
	u_int32_t result_u32;
	int32_t result_s32;
	
	int i;
	
	if (!(opt = strtok(line, " \t="))) return 0;
	
	for (i = 0; options[i].code; i++)
		if (!strcmp(options[i].name, opt)) {
			option = &(options[i]);
			break;
		}
		
	if (!option) return 0;
	
	do {
		val = strtok(NULL, ",\t");//w44771，支持option中包含空格
		if (val) {
			fail = 0;
			length = 0;
			switch (option->flags & TYPE_MASK) {
			case OPTION_IP:
				read_ip(val, buffer);
				break;
			case OPTION_IP_PAIR:
				read_ip(val, buffer);
				if ((val = strtok(NULL, ", \t/-")))
					read_ip(val, buffer + 4);
				else fail = 1;
				break;
			case OPTION_STRING:
				length = strlen(val);
				if (length > 254) length = 254;
				memcpy(buffer, val, length);
				break;
			case OPTION_BOOLEAN:
				if (!read_yn(val, buffer)) fail = 1;
				break;
			case OPTION_U8:
				buffer[0] = strtoul(val, NULL, 0);
				break;
			case OPTION_U16:
				result_u16 = htons(strtoul(val, NULL, 0));
				memcpy(buffer, &result_u16, 2);
				break;
			case OPTION_S16:
				result_s16 = htons(strtol(val, NULL, 0));
				memcpy(buffer, &result_s16, 2);
				break;
			case OPTION_U32:
				result_u32 = htonl(strtoul(val, NULL, 0));
				memcpy(buffer, &result_u32, 4);
				break;
			case OPTION_S32:
				result_s32 = htonl(strtol(val, NULL, 0));	
				memcpy(buffer, &result_s32, 4);
				break;
			default:
				break;
			}
			length += option_lengths[option->flags & TYPE_MASK];
			if (!fail)
				attach_option(opt_list, option, buffer, length);
		} else fail = 1;
	} while (!fail && ((option->flags & OPTION_LIST)||(((unsigned char)(option->code) >= 240) && ((unsigned char)(option->code) <= 245)))); //w44771 modify for option 24x's length
	return 1;
}

#ifdef	VDF_RESERVED
int reserved_aton(const char* reservedIp,u_int32_t* reservedaddr)
{
	int i;
	char* pstart;
	char* pend;
	char ip[16];
	char buffer[128];
	if(NULL == reservedIp)
	{
		return 1;
	}
	memset(buffer,0,sizeof(buffer));
	strcpy(buffer,reservedIp);
	pstart = buffer;
	for(i = 0;(i < MAXRESERVEDIP)&&(NULL != (reservedaddr + i));i++)
	{
		pend = strchr(pstart,',');
		if(NULL == pend)
		{
			*(reservedaddr + i) = inet_addr(pstart);
			return 0;
		}
		memset(ip,0,sizeof(ip));
		memcpy(ip,pstart,pend - pstart);
		*(reservedaddr + i) = inet_addr(ip);
		pstart = pend +1;
		if(NULL == pstart)
		{
			return 0;
		}
	}
	return 0;
}

//read the reserved ip
static int read_rsvip(char *line, void *arg)
{
	u_int32_t* rservip = arg;
	reserved_aton(line,rservip);
	return 1;
}
#endif

#ifdef SUPPORT_MACMATCHIP
int macip_add(u_int8_t *mac, u_int32_t *address)
{
    PMACIP_LIST new_service, current, prev = NULL;

  if ( NULL == mac || NULL == address )
  {
    return -1;
  }

  // First we find out if it already exists
  if ( ( new_service = (PMACIP_LIST) malloc(sizeof(MACIP_LIST)) ) == NULL ) {
    return -1;
  }

  memcpy( new_service->macip.mac, mac, 16 );
  new_service->macip.ip = address;
  new_service->next = NULL;

  if (_macip_list == NULL ) {
    _macip_list = new_service;
  } else {
    for( current = _macip_list; current != NULL; prev = current, current = current->next ) {
      if (memcmp( mac, &current->macip.mac, 16 ) == 0 ) {
        free( new_service );
        
        return -1;
      }     
      else if ( address == current->macip.ip ) {
        free( new_service );

        return -1;
      }

    }
    prev->next = new_service;
  }
  
  return 1;
}

int macip_aton(const char* macipstr, PMACIP_ENTRY macip)
{
	char     mac[18];
    char     ip[16];
    u_int8_t mac_net[16];
    
    memset(ip, 0, sizeof(ip));
    memset(mac, 0, sizeof(mac));
    memset(mac_net, 0, sizeof(mac_net));
    
	if (NULL == macipstr)
	{
		return 1;
	}
	
    sscanf(macipstr,"%s / %s", mac, ip);   
    bcmMacStrToNum(mac_net, mac);
    macip_add(mac_net, inet_addr(ip));
   
	return 0;   
}

static int read_macip(char *line, void *arg)
{
	PMACIP_ENTRY macip = arg;
	macip_aton(line, macip);
    
	return 1;
}
#endif

static struct config_keyword keywords[] = {
	/* keyword	handler   variable address		default */
	{"start",	read_ip,  &(server_config.start),	"192.168.0.20"},
	{"end",		read_ip,  &(server_config.end),		"192.168.0.254"},
	#ifdef	VDF_RESERVED
	{"ReservedIpstr",read_rsvip,server_config.reservedAddr,""},
	#endif
    #ifdef SUPPORT_MACMATCHIP
    {"macip",	read_macip, &(server_config.macip),	""},
    #endif
    /*start: modify by xkf19988 2009.12.22*/
    {"option66",read_str,&(server_config.tftpserver),""},
    {"option67",read_str,&(server_config.bootfilename),""},
    {"option160",read_str,&(server_config.option160),""},
    /*end: modify by xkf19988 2009.12.22*/
	{"interface",	read_str, &(server_config.interface),	"eth0"},
	{"option",	read_opt, &(server_config.options),	""},
	{"opt",		read_opt, &(server_config.options),	""},
	{"max_leases",	read_u32, &(server_config.max_leases),	"254"},
	{"remaining",	read_yn,  &(server_config.remaining),	"yes"},
	{"auto_time",	read_u32, &(server_config.auto_time),	"7200"},
	{"decline_time",read_u32, &(server_config.decline_time),"3600"},
	{"conflict_time",read_u32,&(server_config.conflict_time),"3600"},
	{"offer_time",	read_u32, &(server_config.offer_time),	"60"},
	{"min_lease",	read_u32, &(server_config.min_lease),	"60"},
	{"lease_file",	read_str, &(server_config.lease_file),	"/etc/udhcpd.leases"},
	{"pidfile",	read_str, &(server_config.pidfile),	"/var/run/udhcpd.pid"},
	{"notify_file", read_str, &(server_config.notify_file),	""},
	{"siaddr",	read_ip,  &(server_config.siaddr),	"0.0.0.0"},
	{"sname",	read_str, &(server_config.sname),	""},
	{"boot_file",	read_str, &(server_config.boot_file),	""},
	// BRCM
	{"vendorid",read_str, &(server_config.vendorid),	""},
	{"decline_file",read_str, &(server_config.decline_file),""},
	/*start DHCP Server支持第二地址池, s60000658, 20060616*/
	{"enblsrv1",	read_u32,  &(g_enblSrv1), "0"},
	{"enblsrv2",	read_u32,  &(g_enblSrv2), "0"},
	{"start2",	read_ip,  &(server2_config.start),	"0.0.0.0"},
	{"end2",		read_ip,  &(server2_config.end),		"0.0.0.0"},
	{"interface2",	read_str, &(server2_config.interface),	"eth0"},
	{"option2",	read_opt, &(server2_config.options),	""},
	{"max_leases2",	read_u32, &(server2_config.max_leases),	"254"},
	{"offer_time2",	read_u32, &(server2_config.offer_time),	"60"},
	{"min_lease2",	read_u32, &(server2_config.min_lease),	"60"},
	{"opt2",		read_opt, &(server2_config.options),	""},
    {"remaining2",	read_yn,  &(server2_config.remaining),	"yes"},
	{"auto_time2",	read_u32, &(server2_config.auto_time),	"7200"},
	{"decline_time2",read_u32, &(server2_config.decline_time),"3600"},
	{"conflict_time2",read_u32,&(server2_config.conflict_time),"3600"},
	{"notify_file2", read_str, &(server2_config.notify_file),	""},
	{"siaddr2",	read_ip,  &(server2_config.siaddr),	"0.0.0.0"},
	{"sname2",	read_str, &(server2_config.sname),	""},
	{"boot_file2",	read_str, &(server2_config.boot_file),	""},
	{"classid2",read_str, &(server2_config.vendorid),	""},
	{"lease_file2",	read_str, &(server2_config.lease_file),	"/etc/udhcpd2.leases"},
	/*end DHCP Server支持第二地址池, s60000658, 20060616*/ 	
/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
	{"poolIndex", read_u32, &ipPoolIndex, "1"},
	{"dhcpStart1_1", read_ip, &(ipPool[0].start), "192.168.1.33"},
	{"dhcpEnd1_1", read_ip, &(ipPool[0].end), "192.168.1.254"},
	{"dhcpLease1_1",read_u32, &(ipPool[0].lease), "72"},
#ifdef SUPPORT_CHINATELECOM_DHCP
	{"cameraport", read_str2, cOption43.configVersion, "0"},
	{"ccategory", read_str2, ccategory, ""},
	{"cmodel", read_str2, cmodel, ""},
	//{"dhcpSrv1Option60_1", read_str2, cOption60.vendor, ""},	
#endif
	{"dhcpSrv1Option60_1", read_str2, ipPool[0].option60, ""},
	{"dhcpStart1_2", read_ip, &(ipPool[1].start), "0.0.0.0"},
	{"dhcpEnd1_2", read_ip, &(ipPool[1].end), "0.0.0.0"},
	{"dhcpLease1_2",read_u32, &(ipPool[1].lease), "72"},
	{"dhcpSrv1Option60_2", read_str2, ipPool[1].option60, ""},
	{"dhcpStart1_3", read_ip, &(ipPool[2].start), "0.0.0.0"},
	{"dhcpEnd1_3", read_ip, &(ipPool[2].end), "0.0.0.0"},
	{"dhcpLease1_3",read_u32, &(ipPool[2].lease), "72"},
	{"dhcpSrv1Option60_3", read_str2, ipPool[2].option60, ""},
	{"dhcpStart1_4", read_ip, &(ipPool[3].start), "0.0.0.0"},
	{"dhcpEnd1_4", read_ip, &(ipPool[3].end), "0.0.0.0"},
	{"dhcpLease1_4",read_u32, &(ipPool[3].lease), "72"},
	{"dhcpSrv1Option60_4", read_str2, ipPool[3].option60, ""},
	{"dhcpStart1_5", read_ip, &(ipPool[4].start), "0.0.0.0"},
	{"dhcpEnd1_5", read_ip, &(ipPool[4].end), "0.0.0.0"},
	{"dhcpLease1_5",read_u32, &(ipPool[4].lease), "72"},
	{"dhcpSrv1Option60_5", read_str2, ipPool[4].option60, ""},		
#endif
/*w44771 add for 第一IP支持5段地址池，end*/	
#ifdef SUPPORT_PORTMAPING
      	{"option60str", read_str, &(server_config.option60), ""},
#endif
	{"",		NULL, 	  NULL,				""}
};


int read_config(char *file)
{
	FILE *in;
	/*start of  修改bufer 的长度，避免option60过大 by s53329 at  20070806
	char buffer[80], *token, *line;
	end  of  修改bufer 的长度，避免option60过大 by s53329 at  20070806*/
       char buffer[300], *token, *line;

	int i;

	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
		/*start of  修改bufer 的长度，避免option60过大 by s53329 at  20070806
	        while (fgets(buffer, 80, in)) {
		end  of  修改bufer 的长度，避免option60过大 by s53329 at  20070806*/
             while (fgets(buffer, 300, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		line = line + strspn(line, " \t=");
		if (*line == '\0') continue;
		
		
		
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				keywords[i].handler(line, keywords[i].var);
	}
	fclose(in);
	return 1;
}

/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
#define DNS_CFG_FILE    "/var/dnscfg"
#define DNS_TEMP_FILE    "/var/dnsTemp.conf"
#define DNSMASQ_CFG_FILE    "/var/dnsmasq.conf"
#define DNS_PID_FILE "/var/run/dnsmasq.pid"
#define LINE_SIZE                       128
/*****************************************************************************
 函 数 名  : read_domain
 功能描述  : 从临时文件中读取MAC和域名的对应关系
 输入参数  : void
 输出参数  : 无
 返 回 值  : int 有几条域名信息
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月18日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int read_domain( void )
{
    FILE *fp = NULL;
    char line[LINE_SIZE];
    char mac[18];
    char domain[40];
    char * temp = NULL;
    int i = 0;
    
    memset(domains,0,sizeof(domains));
    memset(line,0,sizeof(line));

    if ( fp = fopen(DNS_CFG_FILE,"r") )
    {
        while ( fgets(line,LINE_SIZE,fp) )
        {
            memset(mac,0,sizeof(mac));
            memset(domain,0,sizeof(domain));
            
            sscanf(line,"%s %s",mac,domain);
            bcmMacStrToNum(domains[i].chaddr,mac);
            strcpy(domains[i].domain,domain);
            domains[i].yiaddr = 0; /*赋初值0*/
            i++;
        }
        fclose(fp);
    }
    g_enblDomainTab = i;
    
    return g_enblDomainTab;
}

/*****************************************************************************
 函 数 名  : write_dnsconf
 功能描述  : 将IP、域名的关系写入dns的控制文件
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月18日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
void write_dnsconf( int sig )
{
    FILE *fd = NULL;
    FILE *ptempFile = NULL;
    FILE *pdnsConfFile = NULL;
    char buf[LINE_SIZE];
    struct in_addr addr;
    int i = 0;
    printf("==DHCPD:%s:%s:%d:==\n",__FILE__,__FUNCTION__,__LINE__);

    /*临时控制文件的内容是internet.page这些网关域名*/
    ptempFile = fopen(DNS_TEMP_FILE, "r");
    if ( ptempFile)
    {
        pdnsConfFile = fopen(DNSMASQ_CFG_FILE, "w");
    }
    else
    {
        printf("==DHCPD:%s:%s:%d:can't open the file %s==\n",__FILE__,__FUNCTION__,__LINE__,DNS_TEMP_FILE);
        return;
    }
    if ( !pdnsConfFile)
    {
        /*没申请到则关闭前一个文件*/
        printf("==DHCPD:%s:%s:%d:can't open the file %s==\n",__FILE__,__FUNCTION__,__LINE__,DNSMASQ_CFG_FILE);
        fclose(ptempFile);
        return;
    }

    memset(buf,0,LINE_SIZE);
    /*将dns临时控制文件的内容复制到dns控制文件中*/
    while ( fgets(buf,LINE_SIZE,ptempFile) )
    {
        printf("==DHCPD:%s:%s:%d:BUF[%s]==\n",__FILE__,__FUNCTION__,__LINE__,buf);
        fputs(buf,pdnsConfFile);
    }
    fclose(ptempFile);

    if ( g_enblDomainTab)
    {
        for (  i = 0 ; i < g_enblDomainTab; i++ )
        {
            if ( domains[i].yiaddr )
            {
                addr.s_addr = domains[i].yiaddr;
                fprintf(pdnsConfFile,"address=/%s/%s\n",domains[i].domain,inet_ntoa(addr));
                printf("==DHCPD:%s:%s:%d:==\n",__FILE__,__FUNCTION__,__LINE__);
            }
        }
    }
    fclose(pdnsConfFile);
    /*start of解决问题单AU8D02061，ADSL Down时，网关Lan DNS功能设置后失效by l129990,2009,4,2*/
     if ( !g_dnsPid)
    {
        /*有则不获取，现在dns不需重起，所有PID不会变化*/
        fd = fopen(DNS_PID_FILE, "r");
        if (fd == NULL) 
        {
            printf("Can't get dns's pid\n");
            return;
        }
        fscanf(fd, "%d", &g_dnsPid);
        fclose(fd);  
    }
    /*end of解决问题单AU8D02061，ADSL Down时，网关Lan DNS功能设置后失效by l129990,2009,4,2*/
    kill(g_dnsPid,SIGHUP);
    if ( g_needToChang )
    {
        g_needToChang = 0;
    }
    
}

/* END:   Added by y67514, 2008/9/18 */

/* the dummy var is here so this can be a signal handler */
void write_leases(int dummy)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
       /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	time_t curr = getSysUpTime();
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	unsigned long lease_time;
	
	dummy = 0;
	
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
	
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
			if (server_config.remaining) {
				if (lease_expired(&(leases[i])))
					lease_time = 0;
				else lease_time = leases[i].expires - curr;
			} else lease_time = leases[i].expires;
			lease_time = htonl(lease_time);
			fwrite(leases[i].chaddr, 16, 1, fp);
			fwrite(&(leases[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
			fwrite(leases[i].hostname, 64, 1, fp);
			//w44771 add for test
			#ifdef SUPPORT_CHINATELECOM_DHCP
			fwrite(&leases[i].port, 4, 1, fp);
			#endif
		}
	}
	fclose(fp);
	
	if (server_config.notify_file) {
		sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
		system(buf);
	}
}


void read_leases(char *file)
{
	FILE *fp;
	unsigned int i = 0;
        /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	time_t curr = getSysUpTime();
        /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	struct dhcpOfferedAddr lease;
	
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
	
	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		if (lease.yiaddr >= server_config.start && lease.yiaddr <= server_config.end) {
			leases[i].yiaddr = lease.yiaddr;
			leases[i].expires = ntohl(lease.expires);	
			if (server_config.remaining) leases[i].expires += curr;
			memcpy(leases[i].chaddr, lease.chaddr, sizeof(lease.chaddr));
			i++;
		}
	}
	
	DEBUG(LOG_INFO, "Read %d leases", i);
	
	if (i == server_config.max_leases) {
		if (fgetc(fp) == EOF)
			/* might be helpfull to drop expired leases */
			LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
	}
	fclose(fp);
}
		
// BRCM_begin
/* the dummy var is here so this can be a signal handler */
void write_decline(int dummy)
{
	FILE *fp;
	
	dummy = 0;
	if (!(fp = fopen(server_config.decline_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}

	fwrite(declines->chaddr, 16, 1, fp);
	fwrite(declines->vendorid, 64, 1, fp);
	fclose(fp);
}

int read_vendor_id_config(char *file)
{
	FILE *in;
	char buffer[64];
	int i = 0;

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
	
	memset(buffer, 0, 64);
        for (i = 0; i < MAX_VENDOR_IDS; i++) {
		memset(vendor_id_config[i].vendorid, 0, 64);
        }
        i = 0;
	while (fgets(buffer, 64, in)) {
		if (i < MAX_VENDOR_IDS) {
			memset(vendor_id_config[i].vendorid, 0, 64);
			if (buffer[strlen(buffer)-1] == '\n') {
				buffer[strlen(buffer)-1] = '\0';
			}
			memcpy(vendor_id_config[i].vendorid, buffer, strlen(buffer));
			i++;
			memset(buffer, 0, 64);
		} else {
			break;
		}
	}
	fclose(in);
        //unlink(file);
	return 1;
}
// BRCM_end
/*start DHCP Server支持第二地址池, s60000658, 20060616*/
/* the dummy var is here so this can be a signal handler */
void write_leases2(int dummy)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	time_t curr = getSysUpTime();
    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	unsigned long lease_time;

	dummy = 0;

		
	if (!(fp = fopen(server2_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server2_config.lease_file);
		return;
	}

	for (i = 0; i < server2_config.max_leases; i++) {
		if (leases2[i].yiaddr != 0) {
			if (server2_config.remaining) {
				if (lease_expired(&(leases2[i])))
					lease_time = 0;
				else lease_time = leases2[i].expires - curr;
			} else lease_time = leases2[i].expires;
			lease_time = htonl(lease_time);
			fwrite(leases2[i].chaddr, 16, 1, fp);
			fwrite(&(leases2[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
			fwrite(leases2[i].hostname, 64, 1, fp);
		}
	}
	fclose(fp);
	
	if (server2_config.notify_file) {
		sprintf(buf, "%s %s", server2_config.notify_file, server2_config.lease_file);
		system(buf);
	}
}


void read_leases2(char *file)
{
	FILE *fp;
	unsigned int i = 0;
    /*Start of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	time_t curr = getSysUpTime();
    /*End of Mod by y67514:time（0）取的系统时间会受SNTP影响，导致定时器混乱*/
	struct dhcpOfferedAddr lease;
	
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
	
	while (i < server2_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		if (lease.yiaddr >= server2_config.start && lease.yiaddr <= server2_config.end) {
			leases2[i].yiaddr = lease.yiaddr;
			leases2[i].expires = ntohl(lease.expires);	
			if (server2_config.remaining) leases2[i].expires += curr;
			memcpy(leases2[i].chaddr, lease.chaddr, sizeof(lease.chaddr));
			i++;
		}
	}
	
	DEBUG(LOG_INFO, "Read %d leases", i);
	
	if (i == server2_config.max_leases) {
		if (fgetc(fp) == EOF)
			/* might be helpfull to drop expired leases2 */
			LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
	}
	fclose(fp);
}
/*end DHCP Server支持第二地址池, s60000658, 20060616*/

#ifdef VDF_OPTION
//add for option125
/* get signal to write viTable to file */
void write_viTable()
{
	FILE *fp;
	int count;
	pVI_OPTION_INFO pPtr=NULL;
	struct dhcpOfferedAddr *lease = NULL;

	if (!(fp = fopen("/var/udhcpd/managable.device", "w+"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", "/var/udhcpd/managable.device");
		return;
	}
	count = viList->count;
	fprintf(fp,"NumberOfDevices %d\n",count);
	if (count > 0) {
	  pPtr = viList->pHead;
	  while (pPtr) {
	    if ((lease = find_lease_by_yiaddr(pPtr->ipAddr)) &&
		lease_expired(lease)) {
	      strcpy(pPtr->oui,"");
	      strcpy(pPtr->serialNumber,"");
	      strcpy(pPtr->productClass,"");
	    }
	    fprintf(fp,"IPaddr %x Enterprise %d OUI %s SerialNumber %s ProductClass %s\n",
		    pPtr->ipAddr,pPtr->enterprise,pPtr->oui,pPtr->serialNumber,
		    pPtr->productClass);
	    pPtr = pPtr->next;
	  }
	}
	fclose(fp);
}
#endif

