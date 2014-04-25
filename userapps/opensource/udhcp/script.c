/* script.c
 *
 * Functions to call the DHCP client notification scripts 
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "packet.h"
#include "options.h"
#include "debug.h"

/* start of maintain dhcp server、log server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ifcdefs.h"
/* end of maintain dhcp server、log server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
/*start of WAN <3.4.5桥使能dhcp, option17> porting by s60000658 20060505*/
#include "syscall.h"
#define DHCP_ROOT_PATH_MAX 256 
/*end of WAN <3.4.5桥使能dhcp, option17> porting by s60000658 20060505*/

/*w44771 add for option43, begin, 2006-9-28*/
#ifdef SUPPORT_TR69C
#include "../../broadcom/tr69c/inc/tr69cdefs.h"
#endif
/*w44771 add for option43, end, 2006-9-28*/


// brcm
static char local_ip[32]="";
static char router_ip[32]="";
static char dns_ip[128]="";
static char subnet_ip[32]="";
static char root_path[DHCP_ROOT_PATH_MAX] = {0};/*WAN <3.4.5桥使能dhcp, option17> porting by s60000658 20060505*/

/*w44771 add for more dhcp option, begin, 2006-6-29*/
#define DHCP_OPTION_128 128
#define DHCP_OPTION_32 32
#define DHCP_OPTION_8 8

static char option_timezone[DHCP_OPTION_8] = {0};
static char option_logsvr1[DHCP_OPTION_32] = {0};
static char option_logsvr2[DHCP_OPTION_32] = {0};
static char option_logsvr_string[DHCP_OPTION_128] = {0};
static char option_hostname_string[DHCP_OPTION_128] = {0};
static char option_domain_string[DHCP_OPTION_128] = {0};
static char option_ntpsvr1[DHCP_OPTION_32] = {0};
static char option_ntpsvr2[DHCP_OPTION_32] = {0};
static char option_ntpsvr_string[DHCP_OPTION_128] = {0};
static char option_tftpsvr1[DHCP_OPTION_32] = {0};
static char option_tftpsvr2[DHCP_OPTION_32] = {0};
static char option_tftp_string[DHCP_OPTION_128] = {0};
static char option_bootfile_string[DHCP_OPTION_128] = {0};
/*w44771 add for option43, begin, 2006-9-28*/
static char option_43[258] = {0};
/*w44771 add for option43, end, 2006-9-28*/
#ifdef VDF_OPTION
#define MAX_DHCP_OPTION_SIZE 256
static unsigned char option_121[MAX_DHCP_OPTION_SIZE] = {0};
static int option121_lenth = 0;
#endif


static int opt_seqnum = 0;

int option_is_ip(unsigned char *option, int len)
{
    if ((4 == len) || (8 == len))//ip mode must be 4 or 8-bit long
    {
        return 1;
    }
	
    return 0;
}
/*w44771 add for more dhcp option, end, 2006-6-29*/

/* get a rough idea of how long an option will be (rounding up...) */
/*modify 默认强制类型转换导致option长度计算错误, 参数option由char*修改成unsigned char*, s60000658, 20060526*/
static int max_option_length(unsigned char *option, struct dhcp_option *type)

{
	unsigned int size = 0;/*modify 默认强制类型转换导致option长度计算错误(有符号改为无符号), s60000658, 20060526*/
	
	switch (type->flags & TYPE_MASK) {
	case OPTION_IP:
	case OPTION_IP_PAIR:
		size = (option[OPT_LEN - 2] / 4) * sizeof("255.255.255.255 ");
		break;
	case OPTION_STRING:
		size = option[OPT_LEN - 2] + 1;
		break;
	case OPTION_BOOLEAN:
		size = option[OPT_LEN - 2] * sizeof("yes ");
		break;
	case OPTION_U8:
		size = option[OPT_LEN - 2] * sizeof("255 ");
		break;
	case OPTION_U16:
		size = (option[OPT_LEN - 2] / 2) * sizeof("65535 ");
		break;
	case OPTION_S16:
		size = (option[OPT_LEN - 2] / 2) * sizeof("-32768 ");
		break;
	case OPTION_U32:
		size = (option[OPT_LEN - 2] / 4) * sizeof("4294967295 ");
		break;
	case OPTION_S32:
		size = (option[OPT_LEN - 2] / 4) * sizeof("-2147483684 ");
		break;
	}
	
	return size;
}

// brcm
static int file_save(char * path, char * content, int check)
{
    FILE *f;

    // brcm
    if (check) {
        if ((f = fopen(path, "r")) > 0) {
	    // the file already exists.
	    fclose(f);
	    return 0;
	}
    }

    f = fopen(path, "w");
    if (f == NULL) {
	error("Failed to create %s: %m", path);
	return -1;
    }

    fprintf(f, "%s\n", content);

    fclose(f);
    return TRUE;
}

#ifdef VDF_OPTION
void file_save_route()
{
	FILE* fp;
	char dfpath[128] = "";
	char stpath[128] = "";
	char destIp[DHCP_OPTION_32];
	char subMask[DHCP_OPTION_32];
	char gwIp[DHCP_OPTION_32];
	char stRtEntry[DHCP_OPTION_128];
	int len = option121_lenth;
	int i = 0;
	int maskwidth = 0;
	int mask[8]={128,192,224,240,248,252,254,255};
	if (strlen(session_path) > 0)
	{
		sprintf(stpath, "%s/%s/%s", _PATH_WAN_ROOT, session_path, _PATH_STATIC_ROUTE);
		sprintf(dfpath, "%s/%s/%s", _PATH_WAN_ROOT, session_path, _PATH_DEFAULT_ROUTE);
	}
	else
	{
		return;
	}

	fp = fopen(stpath, "w");
	if(NULL != fp)
	{
		while(i < len)
		{
			maskwidth = option_121[i];
			/*
			掩码位数与后面表示ip的字节数的关系
			掩码位数	     字节数
		                     0                   	  0
		                  1- 8              	    	  1
		                  9-16                    	  2
		                 17-24                  	  3
		                 25-32                 	  4
			*/
			if((0 == maskwidth)&&((i +4) < len))
			{
				//默认路由
				sprintf(router_ip,"%d.%d.%d.%d",option_121[i+1],option_121[i+2],option_121[i+3],option_121[i+4]);
				//file_save(dfpath, router_ip, FALSE);
				i += 5;
			}
			else if((1 <= maskwidth)&&(maskwidth <= 8)&&((i +5) < len))
			{
				//掩码长度1~8位时
				sprintf(destIp,"%d.%d.%d.%d",option_121[i+1]&mask[maskwidth - 1],0,0,0);
				sprintf(subMask,"%d.%d.%d.%d",mask[maskwidth - 1],0,0,0);
				sprintf(gwIp,"%d.%d.%d.%d",option_121[i+2],option_121[i+3],option_121[i+4],option_121[i+5]);
				sprintf(stRtEntry,"%s %s %s\n",destIp,subMask,gwIp);
				fputs(stRtEntry,fp);
				i += 6;
			}
			else if((9 <=maskwidth )&&(maskwidth <= 16)&&((i +6) < len))
			{
				//掩码长度9~16位时
				sprintf(destIp,"%d.%d.%d.%d",option_121[i+1],option_121[i+2]&mask[maskwidth - 9],0,0);
				sprintf(subMask,"%d.%d.%d.%d",255,mask[maskwidth - 9],0,0);
				sprintf(gwIp,"%d.%d.%d.%d",option_121[i+3],option_121[i+4],option_121[i+5],option_121[i+6]);
				sprintf(stRtEntry,"%s %s %s\n",destIp,subMask,gwIp);
				fputs(stRtEntry,fp);
				i += 7;
			}
			else if((17 <=maskwidth)&&(maskwidth <= 24)&&((i +7) < len))
			{
				//掩码长度17~24位时
				sprintf(destIp,"%d.%d.%d.%d",option_121[i+1],option_121[i+2],option_121[i+3]&mask[maskwidth - 17],0);
				sprintf(subMask,"%d.%d.%d.%d",255,255,mask[maskwidth - 17],0);
				sprintf(gwIp,"%d.%d.%d.%d",option_121[i+4],option_121[i+5],option_121[i+6],option_121[i+7]);
				sprintf(stRtEntry,"%s %s %s\n",destIp,subMask,gwIp);
				fputs(stRtEntry,fp);
				i += 8;
			}
			else if((25 <=maskwidth)&&(maskwidth <= 32)&&((i +8) < len))
			{
				//掩码长度25~32位时
				sprintf(destIp,"%d.%d.%d.%d",option_121[i+1],option_121[i+2],option_121[i+3],option_121[i+4]&mask[maskwidth - 25]);
				sprintf(subMask,"%d.%d.%d.%d",255,255,255,mask[maskwidth - 25]);
				sprintf(gwIp,"%d.%d.%d.%d",option_121[i+5],option_121[i+6],option_121[i+7],option_121[i+8]);
				sprintf(stRtEntry,"%s %s %s\n",destIp,subMask,gwIp);
				fputs(stRtEntry,fp);
				i += 9;
			}
			else
			{
				//erro
				printf("VDF:%s:%s:%d:option121 has the wrong mask width:maskwidth=%d,i=%d,len=%d***\n",__FILE__,__FUNCTION__,__LINE__,maskwidth,i,len);
				fclose(fp);
				return;
			}
		}
		fclose(fp);
		return;
	}
	printf("VDF:%s:%s:%d:can't open file %s***\n",__FILE__,__FUNCTION__,__LINE__,stpath);
	return;
}
#endif

// brcm
static void config_save()
{
/*start of HG553 2008.03.29 V100R001C02B010 AU8D00340 by yepeng */
    FILE *fp;
    char path[128]="";
    char cmd[128]="";
    char fileName[128]="";
	#ifdef VDF_OPTION
	char dfpath[128] = "";
	#endif
    char temps[32];
    int DataFlag = 0;
    int iRetryTimes =0;
    if (strlen(session_path) > 0)
	sprintf(path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_IP);
    file_save(path, local_ip, FALSE);
    if (strlen(session_path) > 0)
	sprintf(path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_MASK);

    /*start of HG553V100R001C02B019 2008.07.05  AU8D00754 */
    if(strlen(subnet_ip))
    {
        file_save(path, subnet_ip, FALSE);
    }
    /*end of HG553V100R001C02B019 2008.07.05  AU8D00754 */
    sprintf(cmd, "mkdir -p %s", _PATH_SYS_DIR);
    system(cmd);
    /*start of 根据vdf 要求业务自动切换需要切换dns by s53329 at  20080115
    file_save(_PATH_RESOLV, dns_ip, TRUE);
    end  of 根据vdf 要求业务自动切换需要切换dns by s53329 at  20080115*/
    if (strlen(dns_ip))
    {
        fp = fopen("/var/HspaDataStatus","r");
        if(fp)
        {
            fgets(temps,32,fp);
            if(strchr(temps,'1'))
            {
                DataFlag = 1;
            }
            else
            {
                DataFlag = 0;
            }
            fclose(fp);
        }
        if (0 == DataFlag)
        {
                //ADSL模式时如果已有则不写入
               file_save("/var/fyi/sys/dynamicdns", dns_ip, TRUE);
        }
        else
        {
                //HSPA模式时则会覆盖
                file_save("/var/fyi/sys/dynamicdns", dns_ip, FALSE);
                sprintf(cmd,"echo 0 >/var/HspaDataStatus");
                system(cmd);
        }
        /*start of  清空dns 配置信息，避免dns接口与路由接口不同 by s53329 at  20080507
        file_save(_PATH_RESOLV, dns_ip, FALSE);
        end  of  清空dns 配置信息，避免dns接口与路由接口不同 by s53329 at  20080507*/
        
    }
    if (strlen(router_ip))
    {
          if (0 == DataFlag)
          {
                //ADSL模式时如果已有则不写入
                file_save(_PATH_GW, router_ip, TRUE);
          }
          else
          {
                //HSPA模式时则会覆盖
                file_save(_PATH_GW, router_ip, FALSE);
          }
    }
/*end of HG553 2008.03.29 V100R001C02B010 AU8D00340 by yepeng */
    /*start of 增加保存各接口默认网关功能by s53329  at  20071220*/
#ifdef VDF_OPTION
     if (strlen(session_path) > 0)
     {
         if(strlen(dns_ip)> 0) 
         {
             sprintf(path, "%s/%s", _PATH_WAN_ROOT, session_path);
             sprintf(cmd, "mkdir -p %s", path);
             system(cmd);
	      sprintf(path, "%s/%s/dns", _PATH_WAN_ROOT, session_path);
             file_save(path, dns_ip, FALSE);
         }
    }
	if (strlen(session_path) > 0)
	{
	     if(strlen(option_121) > 0)
	     {
	          //处理option121, 如果option121中带有静态路由，该函数会修改router_ip的值
	          file_save_route();
	     }
	     
             if(strlen(router_ip)> 0) 
	      {
                  sprintf(cmd, "mkdir -p %s/%s", _PATH_WAN_ROOT, session_path);
                  system(cmd);
                  sprintf(dfpath, "%s/%s/%s", _PATH_WAN_ROOT, session_path, _PATH_DEFAULT_ROUTE);
                  file_save(dfpath, router_ip, FALSE);
	      }
	}

	#else
	  if (strlen(session_path) > 0)
     {
         sprintf(path, "%s/%s", _PATH_INTERFACE_GW, session_path);
         sprintf(cmd, "mkdir -p %s", path);
         system(cmd);
	  sprintf(path, "%s/%s/%s", _PATH_INTERFACE_GW, session_path, _PATH_GATAWAY);
         file_save(path, router_ip, FALSE);
     }
     if (strlen(session_path) > 0)
     {
         sprintf(path, "%s/%s", _PATH_INTERFACE_GW, session_path);
         sprintf(cmd, "mkdir -p %s", path);
         system(cmd);
	  sprintf(path, "%s/%s/dns", _PATH_INTERFACE_GW, session_path);
         file_save(path, dns_ip, FALSE);
    }
	#endif
    /*end of 增加保存各接口默认网关功能by s53329  at  20071220*/

    /*START ADD:Jaffen for pvc dns setting A36D03768*/
  /*start of 增加wan接口对应的dns功能 by s53329  at  20080217*/
//#ifdef SINGAPORE_LOGIN
    /*start of 解决只能发主dns 地址解析，从地址解析失败问题 by s53329 at  20080220 
    if(strlen(dns_ip) > 0)
    {
        char dns[256];
    	sprintf(path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_DNS);
        dns[0] = 0xa;
        dns[1] = '\0';
        file_save(path,dns,0);
        if( strstr(dns_ip,"\n") != NULL )
        {
            memset(dns,0,sizeof(dns));
            if((strstr(dns_ip,"\n") - dns_ip-1) < sizeof(dns))
            {
                strncpy(dns, dns_ip, strstr(dns_ip,"\n") - dns_ip-1);
                dns[sizeof(dns)-1] = '\0';
            }
            if(strlen(strstr(dns_ip,"\n")+1) < sizeof(dns))
            {
                strcat(dns,strstr(dns_ip,"\n")+1);
            }
            file_save(path, dns, 0);

        }
        else
        {
            if(strlen(dns_ip) < sizeof(dns))
            {
                strcpy(dns,dns_ip);
                file_save(path, dns, 0);
            }
        }
    }
    */
    if(strlen(dns_ip) > 0)
    {
        char dns[256];
        sprintf(path, "%s/%s/%s", _PATH_WAN_DIR, session_path, _PATH_DNS);
        if(strlen(dns_ip) < sizeof(dns))
        {
            strcpy(dns,dns_ip);
            file_save(path, dns, 0);
        }
    }
    /*end  of 解决只能发主dns 地址解析，从地址解析失败问题 by s53329 at  20080220 */
//#endif
    /*END ADD:Jaffen for pvc dns setting A36D03768*/
     /*end of 增加wan接口对应的dns功能 by s53329  at  20080217*/
    //printf("router_ip:%s\r\n",router_ip);

    /*START MODIFY: liujianfeng 37298 for [A36D03489] at 2007-03-16*/
    /*start of 修改 策略路由丢失问题 by s53329  at  20080218*/
    if(strlen(router_ip))
    {
    /*end  of 修改 策略路由丢失问题 by s53329  at  20080218*/

        sprintf(fileName,"gw_%s",client_config.Interface);
        printf("\nclient_config.Interface=%s\n",fileName);
        (void)tmpfile_writevalue(fileName, router_ip);
    /*start of 修改 策略路由丢失问题 by s53329  at  20080218*/
    }
    /*end  of 修改 策略路由丢失问题 by s53329  at  20080218*/

    /*END MODIFY: liujianfeng 37298 for [A36D03489] at 2007-03-16*/
    
    /*start of WAN <3.4.5桥使能dhcp, option17> porting by s60000658 20060505*/
    (void)tmpfile_writevalue("RootPath", root_path);
    /*end of WAN <3.4.5桥使能dhcp, option17> porting by s60000658 20060505*/
    /* start of maintain dhcp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
    FILE *fs;
    int pid;
    char line[16];
    key_t key;
    int msgid;
    struct 
    {
        long mtype;
        char mtext[128];
    }msg;
    /* end of maintain dhcp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */

    /*w44771 add for more dhcp option, begin, 2006-6-29*/
	if (strlen (option_timezone) > 0 )
	{
           (void)tmpfile_writevalue("timezone", option_timezone);
	}
	

	if (strlen (option_logsvr1) > 0 )
	{
	    (void)tmpfile_writevalue("logsvr1", option_logsvr1);
	    
	    if (strlen (option_logsvr2) > 0 )
	    {
	       (void)tmpfile_writevalue("logsvr2", option_logsvr2);	
	    }

        /* start of maintain syslog server地址通过dhcp获取 by liuzhijie 00028714 2006年7月10日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
        key = ftok(CFM_MSG_PATH, CFM_MSG_SEED);

        msgid = msgget(key, IPC_CREAT | 0666);
        if (-1 == msgid)
        {
            printf("Error: get message queue in dhcp process failed.");
        }
        else
        {
            msg.mtype = LOGSRV_DHCP_TYPE;
            iRetryTimes =0;
            while (-1 == msgsnd(msgid, &msg, 0, IPC_NOWAIT))
            {
                iRetryTimes++;
                if (10 <= iRetryTimes)
                {
                    perror("ntpsvr");
                    break;
                }
                usleep(100000);
            }
        }
#endif
        /* end of maintain syslog server地址通过dhcp获取 by liuzhijie 00028714 2006年7月10日 */
	}

	if (strlen (option_logsvr_string) > 0 )
	{
	    (void)tmpfile_writevalue("logsvr_string", option_logsvr_string);

        /* start of maintain syslog server地址通过dhcp获取 by liuzhijie 00028714 2006年7月10日 */
#ifdef SUPPORT_SYSLOG_AUTOCONF
        key = ftok(CFM_MSG_PATH, CFM_MSG_SEED);

        msgid = msgget(key, IPC_CREAT | 0666);
        if (-1 == msgid)
        {
            printf("Error: get message queue in dhcp process failed.");
        }
        else
        {
            msg.mtype = LOGSRV_DHCP_TYPE;
            iRetryTimes = 0;
            while (-1 == msgsnd(msgid, &msg, 0, IPC_NOWAIT))
            {
                iRetryTimes++;
                if (10 <= iRetryTimes)
                {
                    perror("ntpsvr");
                    break;
                }
                usleep(100000);
            }
        }
#endif
        /* end of maintain syslog server地址通过dhcp获取 by liuzhijie 00028714 2006年7月10日 */	}

	if (strlen (option_tftpsvr1) > 0 )
	{
	    (void)tmpfile_writevalue("tftpsvr1", option_tftpsvr1);
	    
	    if (strlen (option_tftpsvr2) > 0 )
	    {
	       (void)tmpfile_writevalue("tftpsvr2", option_tftpsvr2);	
	    }
	}

	if (strlen (option_tftp_string) > 0 )
	{
	    (void)tmpfile_writevalue("tftp_string", option_tftp_string);
	}
	
	if (strlen (option_hostname_string) > 0 )
	{
	    (void)tmpfile_writevalue("hostname", option_hostname_string);
	}

	if (strlen (option_domain_string) > 0 )
	{
	    (void)tmpfile_writevalue("domain", option_domain_string);
	}

	if (strlen (option_ntpsvr1) > 0 )
	{
	    (void)tmpfile_writevalue("ntpsvr1", option_ntpsvr1);		

	    if (strlen (option_ntpsvr2) > 0 )
	    {
		  (void)tmpfile_writevalue("ntpsvr2", option_ntpsvr2);
			
	    }
        /* start of maintain sntp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
#ifdef SUPPORT_SNTP_AUTOCONF
        key = ftok(CFM_MSG_PATH, CFM_MSG_SEED);

        msgid = msgget(key, IPC_CREAT | 0666);
        if (-1 == msgid)
        {
            printf("Error: get message queue in dhcp process failed.");
        }
        else
        {
            msg.mtype = SNTPSRV_DHCP_TYPE;
            iRetryTimes = 0;
            while (-1 == msgsnd(msgid, &msg, 0, IPC_NOWAIT))
            {
                iRetryTimes++;
                if (10 <= iRetryTimes)
                {
                    perror("ntpsvr");
                    break;
                }
                usleep(100000);
            }
        }
#endif
        /* end of maintain sntp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */

	}

	if (strlen (option_ntpsvr_string) > 0 )
	{
	    (void)tmpfile_writevalue("ntpsvr_string", option_ntpsvr_string);

        /* start of maintain sntp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
#ifdef SUPPORT_SNTP_AUTOCONF
        key = ftok(CFM_MSG_PATH, CFM_MSG_SEED);

        msgid = msgget(key, IPC_CREAT | 0666);
        if (-1 == msgid)
        {
            printf("Error: get message queue in dhcp process failed.");
        }
        else
        {
            msg.mtype = SNTPSRV_DHCP_TYPE;
            iRetryTimes = 0;
            while (-1 == msgsnd(msgid, &msg, 0, IPC_NOWAIT))
            {
                iRetryTimes++;
                if (10 <= iRetryTimes)
                {
                    perror("ntpsvr");
                    break;
                }
                usleep(100000);
            }
        }
#endif
        /* end of maintain sntp server地址通过dhcp获取 by liuzhijie 00028714 2006年7月7日" */
	}

	if (strlen (option_tftp_string) > 0 )
	{
	    (void)tmpfile_writevalue("tftp_string", option_tftp_string);
	}

	if (strlen (option_bootfile_string) > 0 )
	{
	    (void)tmpfile_writevalue("bootfile", option_bootfile_string);	
	}
    /*w44771 add for more dhcp option, begin, 2006-6-29*/

    /*w44771 add for dhcp option 43, begin, 2006-10-10*/
    	if (strlen (option_43) > 0 )
	{
	    (void)tmpfile_writevalue("option_43", option_43);	
	}
    /*w44771 add for dhcp option 43, end, 2006-10-10*/

}
    
/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, unsigned char *option, struct dhcp_option *type_p)
{
	int type, optlen;
	u_int16_t val_u16;
	int16_t val_s16;
	u_int32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];
        // brcm
	char tmp[128]="";
	//w44771 add for more dhcp options
	int isip = 0;

	dest += sprintf(dest, "%s=", type_p->name);

	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		/*w44771 add for more dhcp option, begin, 2006-6-29*/
		case OPTION_IP_STRING:
			isip = option_is_ip(option, len);
			if (1 == isip) //svr addr in ip mode
			{
			    optlen = option_lengths[OPTION_IP];
				
			    if ( 0 == strcmp(type_p->name, "logsvr") ) //logsvr in ip mode begin
			    {
 			      if( 0 == opt_seqnum )
 			      	{
 			      	    sprintf(option_logsvr1, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);
						
				    opt_seqnum++; 
					
				    //printf("===>option_logsvr1 is : %s\n", option_logsvr1);
 			      	}
				  else
				  {
				  	 sprintf(option_logsvr2, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);

					 opt_seqnum = 0;
					 
					 //printf("===>option_logsvr2 is : %s\n", option_logsvr2);
				  }
			    }//logsvr in ip mode end

			    if ( 0 == strcmp(type_p->name, "tftp") ) //tftpsvr in ip mode begin
			    {
 			      if( 0 == opt_seqnum )
 			      	{
 			      	    sprintf(option_tftpsvr1, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);
						
				    opt_seqnum++; 
					
				    //printf("===>option_tftpsvr1 is : %s\n", option_tftpsvr1);
 			      	}
				  else
				  {
				  	 sprintf(option_tftpsvr2, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);

					 opt_seqnum = 0;
					 
					 //printf("===>option_tftpsvr2 is : %s\n", option_tftpsvr2);
				  }
			    }//tftpsvr in ip mode end

			    if ( 0 == strcmp(type_p->name, "ntpsvr") ) //ntpsvr in ip mode begin
			    {
 			      if( 0 == opt_seqnum )
 			      	{
 			      	    sprintf(option_ntpsvr1, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);
						
				    opt_seqnum++; 
					
				    //printf("===>option_ntpsvr1 is : %s\n", option_ntpsvr1);
 			      	}
				  else
				  {
				  	 sprintf(option_ntpsvr2, "%d.%d.%d.%d",
 					    option[0], option[1],
 					    option[2], option[3]);

					 opt_seqnum = 0;
					 
					 //printf("===>option_ntpsvr2 is : %s\n", option_ntpsvr2);
				  }
			    }//ntpsvr in ip mode begin

			}
			else //svr addr in string mode
			{
			    if (( 0 == strcmp(type_p->name, "logsvr") ) && ( len < DHCP_OPTION_128 ) )//logsvr in sting mode begin
			    {
			        memcpy(option_logsvr_string, option, len);
	                     option_logsvr_string[len] = '\0';
				  //printf("=====>option_logsvr_string is: %s\n", option_logsvr_string);
			    }  //logsvr in sting mode end

			    if (( 0 == strcmp(type_p->name, "ntpsvr") ) && ( len < DHCP_OPTION_128 ) ) //ntpsvr in sting mode begin
			    {
			        memcpy(option_ntpsvr_string, option, len);
	                     option_ntpsvr_string[len] = '\0';
				  //printf("=====>option_ntpsvr_string is: %s\n", option_ntpsvr_string);
			    }  //ntpsvr in sting mode end	

			    if (( 0 == strcmp(type_p->name, "tftp") ) && ( len < DHCP_OPTION_128 ) ) //tftpsvr in sting mode begin
			    {
			        memcpy(option_tftp_string, option, len);
	                     option_tftp_string[len] = '\0';
				  //printf("=====>option_ftp_string is: %s\n", option_tftp_string);
			    }  //tftpsvr in sting mode end			    
			}
		/*w44771 add for more dhcp option, end, 2006-6-29*/
		case OPTION_IP:	/* Works regardless of host byte order. */
			dest += sprintf(dest, "%d.%d.%d.%d",
					option[0], option[1],
					option[2], option[3]);
		        // brcm
		        sprintf(tmp, "%d.%d.%d.%d",
				option[0], option[1],
				option[2], option[3]);
			if (!strcmp(type_p->name, "dns")) {
			    // cwu
			    strcat(dns_ip, "nameserver ");
			    strcat(dns_ip, tmp);
			    strcat(dns_ip, "\n");
			}
			if (!strcmp(type_p->name, "router"))
			    strcpy(router_ip, tmp);
			if (!strcmp(type_p->name, "subnet"))
			    strcpy(subnet_ip, tmp);
			/*start of VDF 2008.5.9 V100R001C02B013 j00100803 AU8D00620 */
			if ( 0 == strcmp(type_p->name, "ntpsvr") )
			{
                if( 0 == opt_seqnum )
                {
              	    sprintf(option_ntpsvr1, "%d.%d.%d.%d",
            	        option[0], option[1], option[2], option[3]);
                	
                    opt_seqnum++; 
                    printf("----->option_ntpsvr1 is : %s\n", option_ntpsvr1);
                }
                else
                {
                    sprintf(option_ntpsvr2, "%d.%d.%d.%d",
                        option[0], option[1], option[2], option[3]);

                    opt_seqnum = 0;  
                    printf("----->option_ntpsvr2 is : %s\n", option_ntpsvr2);
                }
			}
			/*end of VDF 2008.5.9 V100R001C02B013 j00100803 AU8D00620 */
 			break;
		case OPTION_IP_PAIR:
			dest += sprintf(dest, "%d.%d.%d.%d, %d.%d.%d.%d",
					option[0], option[1],
					option[2], option[3],
					option[4], option[5],
					option[6], option[7]);
			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes" : "no");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld", (long) ntohl(val_s32));
			/*w44771 add for more dhcp option, begin, 2006-6-29*/
			if ( 0 == strcmp(type_p->name, "timezone") ) //timezone begin
			{
			     sprintf(option_timezone, "%ld", (long) ntohl(val_s32));
			     //printf("=====>option_timezone is: %s\n", option_timezone);
			}  //timezone end		    
			/*w44771 add for more dhcp option, end, 2006-6-29*/
			break;
		case OPTION_STRING:
			#ifdef VDF_OPTION
			 if (( 0 == strcmp(type_p->name, "option121") ) && ( len < DHCP_OPTION_128 ) ) //classless static route in sting mode begin
			{
		          memcpy(option_121, option, len);
                       option_121[len] = '\0';
			    option121_lenth = len;
			    //printf("VDF:option_121 is: %s\n", option_121);
		       }  //classless static route in sting mode end	
			#endif
			/*w44771 add for more dhcp option, begin, 2006-6-29*/
		      if (( 0 == strcmp(type_p->name, "hostname") ) && ( len < DHCP_OPTION_128 ) ) //hostname in sting mode begin
		      {
		          memcpy(option_hostname_string, option, len);
                       option_hostname_string[len] = '\0';
			    //printf("=====>option_hostname_string is: %s\n", option_hostname_string);
		       }  //hostname in sting mode end	

			 if (( 0 == strcmp(type_p->name, "domain") ) && ( len < DHCP_OPTION_128 ) ) //domain in sting mode begin
			 {
			      memcpy(option_domain_string, option, len);
	                   option_domain_string[len] = '\0';
				//printf("=====>option_domain_string is: %s\n", option_domain_string);
			  }  //domain in sting mode end	 

			 if (( 0 == strcmp(type_p->name, "tftp") ) && ( len < DHCP_OPTION_128 ) ) //tftp in sting mode begin
			 {
			      memcpy(option_tftp_string, option, len);
	                   option_tftp_string[len] = '\0';
				//printf("=====>option_tftp_string is: %s\n", option_tftp_string);
			  }  //tftp in sting mode end

			 if (( 0 == strcmp(type_p->name, "bootfile") ) && ( len < DHCP_OPTION_128 ) ) //bootfile in sting mode begin
			 {
			      memcpy(option_bootfile_string, option, len);
	                   option_bootfile_string[len] = '\0';
				//printf("=====>option_bootfile_string is: %s\n", option_bootfile_string);
			  }  //bootfile in sting mode end	  
			/*w44771 add for more dhcp option, end, 2006-6-29*/

			/*w44771 add for option43, begin, 2006-9-28*/
			 if (( 0 == strcmp(type_p->name, "tr069acs") ) && ( len <= 256 ) ) //bootfile in sting mode begin
			 {
			      memcpy(option_43, option, len);
	                   option_43[len] = '\0';
				//printf("=====>dhcp, option_43 is: %s\n", option_43);
	                   #ifdef SUPPORT_TR69C
                          TR069_SendMessage(GET_DHCP_OPTION43, option_43, len);
	                   #endif
			 }
			/*w44771 add for option43, end, 2006-9-28*/

			/*start of WAN <3.4.5桥使能dhcp, 自动升级> porting by s60000658 20060505*/
		    if(!strcmp(type_p->name, "rootpath") && (len < DHCP_ROOT_PATH_MAX))  
	        {
	            memcpy(root_path, option, len);
	            root_path[len] = '\0';
	        }
	    	/*end of WAN <3.4.5桥使能dhcp, 自动升级> porting by s60000658 20060505*/
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
		*(dest++) = ' ';
	}
}


static char *find_env(const char *prefix, char *defaultstr)
{
	extern char **environ;
	char **ptr;
	const int len = strlen(prefix);

	for (ptr = environ; *ptr != NULL; ptr++) {
		if (strncmp(prefix, *ptr, len) == 0)
		return *ptr;
	}
	return defaultstr;
}


/* put all the paramaters into an environment */
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	unsigned char *addr;
	/*start 默认强制类型转换导致option长度计算错误, s60000658, 20060526
	char **envp, *temp;*/
	char **envp = NULL;
	unsigned char *temp = NULL;
	/*end 默认强制类型转换导致option长度计算错误, s60000658, 20060526*/
	char over = 0;

	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; options[i].code; i++)
			if (get_option(packet, options[i].code))
				num_options++;
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;		
	}
	
	envp = malloc((num_options + 5) * sizeof(char *));
	/*start of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
	//envp[0] = malloc(strlen("interface=") + strlen(client_config.interface) + 1);
	//sprintf(envp[0], "interface=%s", client_config.interface);
	envp[0] = malloc(strlen("interface=") + strlen(client_config.Interface) + 1);
	sprintf(envp[0], "interface=%s", client_config.Interface);
	/*end of WAN <3.4.5桥使能dhcp> porting by s60000658 20060505*/
	envp[1] = malloc(sizeof("ip=255.255.255.255"));
	
	/*start of DHCP 检查envp是否申请内存成功，并初始化环境变量中的IP porting by w44771 20060505*/
	if (NULL != envp[1])
	{
	    sprintf(envp[1], "ip=0.0.0.0");
	}
	/*end of DHCP 检查envp是否申请内存成功，并初始化环境变量中的IP porting by w44771 20060505*/
	
	envp[2] = find_env("PATH", "PATH=/bin:/usr/bin:/sbin:/usr/sbin");
	envp[3] = find_env("HOME", "HOME=/");

	if (packet == NULL) {
		envp[4] = NULL;
		return envp;
	}

	addr = (unsigned char *) &packet->yiaddr;
	sprintf(envp[1], "ip=%d.%d.%d.%d",
		addr[0], addr[1], addr[2], addr[3]);
	// brcm
	sprintf(local_ip, "%d.%d.%d.%d",
		addr[0], addr[1], addr[2], addr[3]);
	strcpy(dns_ip, "");

	for (i = 0, j = 4; options[i].code; i++) {
		if ((temp = get_option(packet, options[i].code))) {
			envp[j] = malloc(max_option_length(temp, &options[i]) + 
					strlen(options[i].name) + 2);
            /*start 默认强制类型转换导致option长度计算错误, s60000658, 20060526
			fill_options(envp[j], temp, &options[i]);
			j++;*/
			if (NULL != envp[j])
		    {
    			fill_options(envp[j], temp, &options[i]);
    			j++;
		    }
			else
		    {
		        printf("DHCP: memery malloc fail, option name = %s, len = 0x%x\n", 
		                options[i].name, max_option_length(temp, &options[i]) + strlen(options[i].name) + 2);
		    }
            /*end 默认强制类型转换导致option长度计算错误, s60000658, 20060526*/
		}
	}
	if (packet->siaddr) {
		envp[j] = malloc(sizeof("siaddr=255.255.255.255"));
		addr = (unsigned char *) &packet->yiaddr;
		sprintf(envp[j++], "siaddr=%d.%d.%d.%d",
			addr[0], addr[1], addr[2], addr[3]);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		envp[j] = malloc(sizeof("boot_file=") + strlen(packet->file));
		sprintf(envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		envp[j] = malloc(sizeof("sname=") + strlen(packet->sname));
		sprintf(envp[j++], "sname=%s", packet->sname);
	}	
	envp[j] = NULL;
	// brcm
	config_save();
	return envp;
}


/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	int pid;
	char **envp;

	if (client_config.script == NULL)
		return;

	/* call script */
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
		return;
	} else if (pid == 0) {
		envp = fill_envp(packet);
		
		/* close fd's? */
		
		/* exec script */
		DEBUG(LOG_INFO, "execle'ing %s", client_config.script);
		execle(client_config.script, client_config.script,
		       name, NULL, envp);
		LOG(LOG_ERR, "script %s failed: %s",
		    client_config.script, strerror(errno));
		exit(1);
	}			
}

/*start of  2008.05.04 HG553V100R001C02B013 AU8D00566 */
//for static route of option121
void run_staticRoute()
{
    char fDirectory[IFC_LARGE_LEN];
   char oldDirectory[IFC_LARGE_LEN];
   char line[IFC_LARGE_LEN];
   char addr[16];
   char mask[16];
   char gtwy[16];
   char cmd[IFC_LARGE_LEN];

   if (NULL == session_path) 
   {
      return;
   }

   sprintf(fDirectory, "/var/fyi/sys/%s/staticroute", session_path);
   
   
   sprintf(oldDirectory, "/var/fyi/sys/%s/oldstaticroute", session_path);

   FILE* fsRoute = fopen(fDirectory, "r");
   
   if(fsRoute != NULL) 
   {
      FILE* oldRoute = fopen(oldDirectory, "a+");
      
      //先删除旧的路由
      if(oldRoute == NULL)
      {
          fclose(fsRoute);
          printf("open oldstaticroute file failed !");
          return;
      }

      while(fgets(line, sizeof(line), oldRoute)) 
      {
	    sscanf(line, "%s %s %s",addr, mask, gtwy);
	    sprintf(cmd, "route del -net %s netmask %s metric 1", addr, mask);

	   if (strcmp(gtwy, "0.0.0.0") != 0) 
	   {
            strcat(cmd, " gw ");
            strcat(cmd, gtwy);
         }
    
         strcat(cmd, " dev ");
         strcat(cmd, session_path);  
         strcat(cmd, " 2>/dev/null");	
         bcmSystem(cmd);
      }/* end while */
      fclose(oldRoute);

      //创建新的路由，同时将新的路由记录在oldstaticroute文件中
      oldRoute = fopen(oldDirectory, "w");
      while(fgets(line, sizeof(line), fsRoute)) 
      {
         //printf("ADD ROUTE INFO:%s\n",line);
		 sscanf(line, "%s %s %s",addr, mask, gtwy);
         //printf("GET INFO:addr=%s,mask=%s,gtwy=%s\n",addr,mask,gtwy);
		 
		 sprintf(cmd, "route add -net %s netmask %s metric 1", addr, mask);

		 if (strcmp(gtwy, "0.0.0.0") != 0) 
		{
	            strcat(cmd, " gw ");
	            strcat(cmd, gtwy);
             }
    
         strcat(cmd, " dev ");
         strcat(cmd, session_path);    
         strcat(cmd, " 2>/dev/null");	
         bcmSystem(cmd);

	  if(oldRoute != NULL)
	  {
	      fputs(line, oldRoute);
	  }
      }/* end while */

	//关闭文件
      fclose(fsRoute);
      if(oldRoute != NULL)
      	{
      	    fclose(oldRoute);
      	}
	
	sprintf(cmd,"rm -rf %s",fDirectory);
	bcmSystem(cmd);
   }/* end if( fsRoute != NULL ) */
}
/*end of  2008.05.04 HG553V100R001C02B013 AU8D00566 */ 

