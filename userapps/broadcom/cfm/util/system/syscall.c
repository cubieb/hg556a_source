/*****************************************************************************
//
//  Copyright (c) 2000-2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       syscall.c
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      It provides system call functions for Linux.
//
//  History:
//    <Author>      <Date>      <Desc>
//    caiyanfeng    2006-05-05  Add some system funcs called by cfm.
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <crypt.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
/* start of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bcmprocfs.h"
/* end of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */
#include <sys/utsname.h>
#include <dirent.h>
#include <ctype.h>
#include <net/if.h>
#include <net/route.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>

#include "bcmtypes.h"
#ifdef USE_ALL
#include "bcmadsl.h"
#include "ifcdefs.h"
#include "psiapi.h"
#include "sysdiag.h"
#include "clidefs.h"
#include "cliapi.h"
#include "adslctlapi.h"
#if defined(SUPPORT_VDSL)
#include "vdslctlapi.h"
#endif
#include "secapi.h"
#include "dbapi.h"
#include "dbobject.h"
#include "dbvalid.h"
#include "version.h"
#ifdef PORT_MIRRORING
#include <atm.h>
#include "portMirror.h"
#include "atmapidrv.h"
#endif
#endif

#include "bcmxdsl.h"
#include "syscall.h"
#include "board_api.h"
#include "ttyUSB.h"
#include "pthread.h"
#ifndef VDF_RESERVED
#define VDF_RESERVED
#endif
#ifdef VOXXXLOAD
#include <vodslapi.h>
#endif

/* BEGIN: Added by y67514, 2009/12/11   PN:APNSecurity Enhancement*/
#include "ifcdefs.h"
/* END:   Added by y67514, 2009/12/11 */
/*start of waninfo by c65985 for vdf 080314*/
#ifdef SUPPORT_VDF_WANINFO

#include "bcmcfm.h"

#include "webstr_lang.h"
#include "cgimain.h"
#define NUM_SHOWPVC  2
#define DATA_VPI   10 
#define DATA_VCI   35
#define VOICE_VPI  10
#define VOICE_VCI  36
#define HSPA_VPI  255
#define HSPA_VCI  65535
#endif
/*end of waninfo by c65985 for vdf 080314*/

#define PRIORITY_HIGH    3
#define PRIORITY_MEDIUM  2
#define PRIORITY_LOW     1

/* start of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日*/
#ifdef SUPPORT_TR69C
#include "../../../tr69c/inc/appdefs.h"
#endif
/* end of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日*/
    
#ifdef BUILD_STATIC
#include "busybox.h"
#endif

#ifndef USE_ALL
#define IFC_LARGE_LEN        264
#endif
/*start  of 增加Radius 认证功能by s53329   at   20060714*/
#ifdef  SUPPORT_RADIUS
#define  CFG_FILE_LEN              197
#endif
/*end  of 增加Radius 认证功能by s53329   at   20060714*/

/* flag file that notice HSPA to start/stop dialing */
#define HSPA_DIAL_FLAG_FILE "/var/HspaDialFlag"

extern char **environ;
extern char *getIfName(void);

extern char glbErrMsg[IFC_LARGE_LEN];
#define READ_BUF_SIZE        128

/* If you change this definition, make sure you change it everywhere */
#define UDHCPD_DECLINE "/var/udhcpd.decline"

/* start of maintain tmpfile by liuzhijie 00028714 2006年5月23日 */
#define TMPFILE_DEBUG 0
/* end of maintain tmpfile by liuzhijie 00028714 2006年5月23日 */

/* start of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */
//for IPV4, IPV6 must add "DAC"
static const char flagchars[] = 		/* Must agree with flagvals[]. */
	"GHRDM"
;

//for IPV4, IPV6 must add RTF_DEFAULT, RTF_ADDRCONF, RTF_CACHE
static const unsigned int flagvals[] = { /* Must agree with flagchars[]. */
	RTF_GATEWAY,
	RTF_HOST,
	RTF_REINSTATE,
	RTF_DYNAMIC,
	RTF_MODIFIED,
};

#define IPV4_MASK (RTF_GATEWAY|RTF_HOST|RTF_REINSTATE|RTF_DYNAMIC|RTF_MODIFIED)

#define	INADDR_ANY		((unsigned long int) 0x00000000)

/* end of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */

#ifdef USE_ALL

/***************************************************************************
// Function Name: bcmSystemNoHang().
// Description  : launch shell command in the child process without hang.
// Parameters   : command - shell command to launch.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSystemNoHang (char *command) {
   int pid = 0, status = 0, counter = 0, ret = 0;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      execve("/bin/sh", argv, environ);
      exit(127);
   }

   do {
      // check the child is exited or not without hang
      ret = waitpid(pid, &status, WNOHANG | WUNTRACED);
      switch ( ret ) {
         case -1:   // error occurs then return -1
            return -1;
         case 0:    // child does not exited yet
         /*BEGIN: modify for 700 times setting the wpa key results in the httpd process dead by DHS00169988; 2010-12-26 */
            //if ( ++counter > 20 ) {
            if ( ++counter > 250 ) {
         /*END: modify for 700 times  setting the wpa key results in the httpd process dead by DHS00169988; 2010-12-26 */
               printf("app: child process cannot exits while executing command - %s\n", command);
               kill(pid, SIGTERM);
               return -1;
            }
            usleep(20000);
            break;
         default:   // child does exit successfully
            return status;
      }
   } while ( 1 );

   return status;
}

int bcmSystemNoHangForIptables (char *command) {
   int pid = 0, status = 0, counter = 0, ret = 0;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      execve("/bin/sh", argv, environ);
      exit(127);
   }

   do {
      // check the child is exited or not without hang
      ret = waitpid(pid, &status, WNOHANG | WUNTRACED);
      switch ( ret ) {
         case -1:   // error occurs then return -1
            return -1;
         case 0:    // child does not exited yet
         /*BEGIN: modify for 700 times setting the wpa key results in the httpd process dead by DHS00169988; 2010-12-26 */
            //if ( ++counter > 20 ) {
            if ( ++counter > 250 ) {
         /*END: modify for 700 times  setting the wpa key results in the httpd process dead by DHS00169988; 2010-12-26 */
               printf("app: child process cannot exits while executing command - %s\n", command);
               kill(pid, SIGTERM);
               return -1;
            }
            usleep(20000);
            break;
         default:   // child does exit successfully
            return status;
      }
   } while ( 1 );

   return status;
}

/*j00100803 Add Begin 2008-04-15*/
#ifdef SUPPORT_VDF_DHCP
int bcmGetSubMaskWidth(int ipAddr)
{
    int m, iMaskWidth = 0;
	int * pTmp = &ipAddr;
	unsigned char szTmp[5];
	memset(szTmp, 0, 5);
	memcpy(szTmp, pTmp, 4);

    if(0 == ipAddr)
    {
        return 0;
    }
	for(m = 0; m < 4; m++)
	{
		if(szTmp[m] == 255)
		{
			iMaskWidth += 8;
        }
		else
		{
			if(szTmp[m] == 128)
			{
				iMaskWidth += 1;
			}
			else if(szTmp[m] == 192)
			{
				iMaskWidth += 2;
			}
			else if(szTmp[m] == 224)
			{
				iMaskWidth += 3;
			}
			else if(szTmp[m] == 240)
			{
				iMaskWidth += 4;
			}
			else if(szTmp[m] == 248)
			{
				iMaskWidth += 5;
			}
			else if(szTmp[m] == 252)
			{
				iMaskWidth += 6;
			}
			else if(szTmp[m] == 254)
			{
				iMaskWidth += 7;
			}
			break;
		}
	}	

	return iMaskWidth;
}
#endif
/*j00100803 Add End 2008-04-15*/
/*start DHCP Server支持第二地址池, s60000658, 20060616*/
/***************************************************************************
// Function Name: bcmCreateDhcpCfg().
// Description  : create DHCP server configuration file.
// Parameters   : ipAddr - IP address of target.
//                mask - subnet mask of target.
//                addrStart - start IP address of DHCP server pool.
//                addrEnd - end IP address of DHCP server pool.
//                leasedTime - leased time.
//                ipAddr2 - IP address of target of the second DHCP server pool.
//                mask2 - subnet mask of target of the second DHCP server pool.
//                addrStart2 - start IP address of the second DHCP server pool.
//                addrEnd2 - end IP address of the second DHCP server pool.
//                leasedTime2 - leased time of the second DHCP server pool.
//                classid2 - dhcp class identifier(option60) of the second DHCP server pool.
//                           only client with the id can get ip for the second pool
//                dns1 - primary dns.
//                dns2 - secondary dns.
//                leasedTime - leased time.
//                protoInfo - current network protocol.
//                enblNat -- is nat enabled?
//                enblFirewall -- is firewall enabled?
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmCreateDhcpCfg(
             char *ipAddr,  char *mask, char *addrStart,  char *addrEnd, int leasedTime,
             char *ipAddr2, char *mask2,char *addrStart2, char *addrEnd2,int leasedTime2,
             char * classid2, char *dns1, char *dns2, char * pOption66,char * pOption67, char * pOption160,
#ifdef SUPPORT_PORTMAPING
            char *option60,
#endif 
#ifdef	VDF_RESERVED
		char *reservedIp,
#endif
#ifdef SUPPORT_MACMATCHIP
            PMACMATCH_ENTRY macmatch_entry,
            int   num_entries,
#endif
#ifdef SUPPORT_VDF_DHCP
            char * ntp1,
            char * ntp2,
            unsigned char * pOption121,
            char * pOption43,
#endif
#ifdef SUPPORT_DHCP_FRAG             
             int protocol, int enblNat, int enblFirewall, int enblSer1, int enblSer2, PIFC_LAN_INFO lanInfo) {/*w44771 add for 第一IP支持5段地址池，modify*/
#else
             int protocol, int enblNat, int enblFirewall, int enblSer1, int enblSer2) {
 #endif

   FILE* fs = fopen("/etc/udhcpd.conf", "w");
   /*start of 修改option 的大小校验by s53329 at  20070616
   char cmd[128]="";
   end of 修改option 的大小校验by s53329 at  20070616*/
   char cmd[1280]="";
   /* Start of dns&dhcp by c47036 20060821 */
   char primarydns[IFC_TINY_LEN];
   char secondarydns[IFC_TINY_LEN];
   int dnsmode = 1;
#ifdef SUPPORT_MACMATCHIP
   int i;
   char macstring[20];
#endif

   primarydns[0] = '\0';
   secondarydns[0] = '\0';
   bcmGetDnsSettings(&dnsmode, primarydns, secondarydns);
   /* End of dns&dhcp by c47036 20060821 */

   if ( fs != NULL ) 
   {
          /*dhcp server第一地址池*/
          if(enblSer1)
          {
              sprintf(cmd, "enblsrv1 %s\n", "1");
          }
          else
          {
              sprintf(cmd, "enblsrv1 %s\n", "0");
          }
          fputs(cmd, fs);
          
          // cwu
          sprintf(cmd, "echo %s > /var/run/ip", addrStart);
          system(cmd);

          // start IP address
          sprintf(cmd, "start %s\n", addrStart);
          fputs(cmd, fs);

          // end IP address
          sprintf(cmd, "end %s\n", addrEnd);
          fputs(cmd, fs);
		  
#ifdef	VDF_RESERVED
		//reservedip
		sprintf(cmd, "ReservedIpstr %s\n", reservedIp);
		fputs(cmd, fs);
#endif

#ifdef SUPPORT_MACMATCHIP
        for( i = 0; i < num_entries; i++ ) 
        {
           bcmMacNumToStr(macmatch_entry[i].mac, macstring);
           
           sprintf(cmd, "macip %s / %s\n", macstring, macmatch_entry[i].ipaddress);
		   fputs(cmd, fs);        
        }
#endif

/*w44771 add for 第一IP支持5段地址池，begin*/
#ifdef SUPPORT_DHCP_FRAG
          sprintf(cmd, "poolIndex %d\n", lanInfo->dhcpSrv.poolIndex);
          fputs(cmd, fs);
          
          sprintf(cmd, "dhcpStart1_1 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_1));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpEnd1_1 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_1));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpLease1_1 %d\n", lanInfo->dhcpSrv.dhcpLease1_1);
          fputs(cmd, fs);
          sprintf(cmd, "dhcpSrv1Option60_1 %s\n", lanInfo->dhcpSrv.dhcpSrv1Option60_1);
          fputs(cmd, fs);
          
          sprintf(cmd, "dhcpStart1_2 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_2));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpEnd1_2 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_2));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpLease1_2 %d\n", lanInfo->dhcpSrv.dhcpLease1_2);
          fputs(cmd, fs);
          sprintf(cmd, "dhcpSrv1Option60_2 %s\n", lanInfo->dhcpSrv.dhcpSrv1Option60_2);
          fputs(cmd, fs);
          
          sprintf(cmd, "dhcpStart1_3 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_3));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpEnd1_3 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_3));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpLease1_3 %d\n", lanInfo->dhcpSrv.dhcpLease1_3);
          fputs(cmd, fs);
          sprintf(cmd, "dhcpSrv1Option60_3 %s\n", lanInfo->dhcpSrv.dhcpSrv1Option60_3);
          fputs(cmd, fs);
          
          sprintf(cmd, "dhcpStart1_4 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_4));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpEnd1_4 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_4));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpLease1_4 %d\n", lanInfo->dhcpSrv.dhcpLease1_4);
          fputs(cmd, fs);
          sprintf(cmd, "dhcpSrv1Option60_4 %s\n", lanInfo->dhcpSrv.dhcpSrv1Option60_4);
          fputs(cmd, fs);
          
          sprintf(cmd, "dhcpStart1_5 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpStart1_5));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpEnd1_5 %s\n", inet_ntoa(lanInfo->dhcpSrv.dhcpEnd1_5));
          fputs(cmd, fs);
          sprintf(cmd, "dhcpLease1_5 %d\n", lanInfo->dhcpSrv.dhcpLease1_5);
          fputs(cmd, fs);
          sprintf(cmd, "dhcpSrv1Option60_5 %s\n", lanInfo->dhcpSrv.dhcpSrv1Option60_5);
          fputs(cmd, fs);

#ifdef SUPPORT_CHINATELECOM_DHCP
          sprintf(cmd, "cameraport %s\n", lanInfo->dhcpSrv.cameraport);
          fputs(cmd, fs);
          sprintf(cmd, "ccategory %s\n", lanInfo->dhcpSrv.ccategory);
          fputs(cmd, fs);
          sprintf(cmd, "cmodel %s\n", lanInfo->dhcpSrv.cmodel);
          fputs(cmd, fs);
#endif
#endif
/*w44771 add for 第一IP支持5段地址池，end*/

          // interface
          switch ( protocol ) {
          case PROTO_PPPOA:
          case PROTO_PPPOE:
          case PROTO_PPPOU://add by sxg
          case PROTO_MER:
          case PROTO_IPOA:
          case PROTO_NONE:
    #if SUPPORT_ETHWAN
          case PROTO_IPOWAN:
    #endif
             fputs("interface br0\n", fs);
             break;
          }

          // If you change the name of this file, make sure you change it
          // everywhere by searching for UDHCPD_DECLINE macro
          sprintf(cmd, "decline_file %s\n", UDHCPD_DECLINE);
          fputs(cmd, fs);

    	  // lease
          sprintf(cmd, "option lease %d\n", leasedTime);
          fputs(cmd, fs);
          sprintf(cmd, "option min_lease 30\n");
          fputs(cmd, fs);

          // subnet mask
          sprintf(cmd, "option subnet %s\n", mask);
          fputs(cmd, fs);

          // router
          sprintf(cmd, "option router %s\n", ipAddr);
          fputs(cmd, fs);

          // use DNS relay only when NAT is enabled (and dnsmode is dynamic --c47036)
          if (( enblNat == TRUE ) && (dnsmode == 1)) {
             // always use DSL router IP address as DNS
             // for DHCP server since we want local PCs
             // use DHCP server relay. The real DNS is
             // stored in /etc/resolv.conf
             /*start of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
             // primary DNS
             /* start of VDF 2008.6.19V100R001C02B018 j00100803AU8D00742 */
             sprintf(cmd, "option dns %s\n", dns1);
             fputs(cmd, fs);
             // secondary DNS
             sprintf(cmd, "option dns %s\n", dns2);
             fputs(cmd, fs);
             /* end of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
             /*end of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
          } else { // use real DNS when NAT is disabled
             // primary DNS
             /* start of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
             sprintf(cmd, "option dns %s\n", dns1);
             fputs(cmd, fs);
             // secondary DNS
             sprintf(cmd, "option dns %s\n", dns2);
             fputs(cmd, fs);
             /* end of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
          }
          
            #ifdef SUPPORT_VDF_DHCP
            // ntp server
            if(strcmp(ntp1, "0.0.0.0") != 0 )
            {
                sprintf(cmd, "option ntpsvr %s\n", ntp1);
                fputs(cmd, fs);
            }
            if(strcmp(ntp2, "0.0.0.0") != 0)
            {
                sprintf(cmd, "option ntpsvr %s\n", ntp2);
                fputs(cmd, fs);
            }
            if(strcmp(pOption121, "0.0.0.0") != 0)
            {
                sprintf(cmd, "option option121 %s\n", pOption121);
                fputs(cmd, fs);
            }
            /* j00100803 Add Begin 2008-05-20 for option43 */
            // option43
            if('\0' != *pOption43)
            {
                sprintf(cmd, "option option43 %s\n", pOption43);
                fputs(cmd, fs);
            }
            /* j00100803 Add End 2008-05-20 for option43 */
            #endif
            /* j00100803 Add Begin 2008-06-28 for option15 */
            fputs("option domain localdomain\n", fs);
            /* j00100803 Add Begin 2008-06-28 for option15 */
          /*start of support to configure the option66,67,160 by l129990,2009,12,22*/
	     printf("*******go there ,the pOption66 is %s*******:\n",pOption66);
	     printf("*******go there the pOption67 is %s********:\n",pOption67);
	     printf("*******go there the pOption160 is %s*******:\n",pOption160);
            if(NULL != pOption66)
            {
                if('\0' != *pOption66)
                {
                    sprintf(cmd, "option66 %s\n", pOption66);
                    fputs(cmd, fs);
                }
            }
            
            if(NULL != pOption67)
            {
                if('\0' != *pOption67)
                {
                   sprintf(cmd, "option67 %s\n", pOption67);
                   fputs(cmd, fs);
                }
            }

            if(NULL != pOption160)
            {
                if('\0' != *pOption160)
                {
                    sprintf(cmd, "option160 %s\n", pOption160);
                    fputs(cmd, fs);
                }
            }
          /*end of support to configure the option66,67,160 by l129990,2009,12,22*/

        /*dhcp server第二地址池*/
   
          // second dhcp server enable flag
          if(enblSer2)
          {
              sprintf(cmd, "enblsrv2 %s\n", "1");
          }
          else
          {
              sprintf(cmd, "enblsrv2 %s\n", "0");
          }
          fputs(cmd, fs);
          
          // start IP address
          sprintf(cmd, "start2 %s\n", addrStart2);
          fputs(cmd, fs);

          // end IP address
          sprintf(cmd, "end2 %s\n", addrEnd2);
          fputs(cmd, fs);

          // interface
          switch ( protocol ) {
          case PROTO_PPPOA:
          case PROTO_PPPOE:
          case PROTO_PPPOU://add by sxg
          case PROTO_MER:
          case PROTO_IPOA:
          case PROTO_NONE:
    #if SUPPORT_ETHWAN
          case PROTO_IPOWAN:
    #endif
             fputs("interface2 br0\n", fs);
             break;
          }

    	  // lease
          sprintf(cmd, "option2 lease %d\n", leasedTime2);
          fputs(cmd, fs);
          sprintf(cmd, "option2 min_lease 30\n");
          fputs(cmd, fs);

          // subnet mask
          sprintf(cmd, "option2 subnet %s\n", mask2);
          fputs(cmd, fs);

          // router
          sprintf(cmd, "option2 router %s\n", ipAddr2);
          fputs(cmd, fs);

          // dhcp class identifier, option60
          if(NULL != classid2 && strlen(classid2) > 0)
          {
              sprintf(cmd, "classid2 %s\n", classid2);
              fputs(cmd, fs);
          }
          
          // use DNS relay only when NAT is enabled (and dnsmode is dynamic --c47036)
          if (( enblNat == TRUE ) && (dnsmode == 1)) {
             // always use DSL router IP address as DNS
             // for DHCP server since we want local PCs
             // use DHCP server relay. The real DNS is
             // stored in /etc/resolv.conf
             /*start of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
             // primary DNS
             /* start of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
             sprintf(cmd, "option2 dns %s\n", dns1);
             fputs(cmd, fs);
             // secondary DNS
             sprintf(cmd, "option2 dns %s\n", dns2);
             fputs(cmd, fs);
             /* end of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
             /*end of VDF 2008.4.28 V100R001C02B013 j00100803 AU8D00423 */
          } else { // use real DNS when NAT is disabled
             // primary DNS
             /* start of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
             sprintf(cmd, "option2 dns %s\n", dns1);
             fputs(cmd, fs);
             // secondary DNS
             sprintf(cmd, "option2 dns %s\n", dns2);
             fputs(cmd, fs);
             /* end of VDF 2008.6.19 V100R001C02B018 j00100803AU8D00742 */
          }

            #ifdef SUPPORT_VDF_DHCP
            // ntp server
            if ( strcmp(ntp1, "0.0.0.0") != 0 )
            {
                sprintf(cmd, "option2 ntpsvr %s\n", ntp1);
                fputs(cmd, fs);
            }
            if(strcmp(ntp2, "0.0.0.0") != 0)
            {
                sprintf(cmd, "option2 ntpsvr %s\n", ntp2);
                fputs(cmd, fs);
            }
            if(strcmp(pOption121, "0.0.0.0") != 0)
            {
                sprintf(cmd, "option2 option121 %s\n", pOption121);
                fputs(cmd, fs);
            }
            // option43
            if('\0' != *pOption43)
            {
                sprintf(cmd, "option2 option43 %s\n", pOption43);
                fputs(cmd, fs);
            }
            /* j00100803 Add Begin 2008-06-28 for option15 */
            fputs("option2 domain localdomain\n", fs);
            /* j00100803 Add Begin 2008-06-28 for option15 */
            #endif
            
          //w44771 modify for option 241 and 242 and length
#ifdef SUPPORT_DHCPOPTIONS
        {
              DHCPOptions dhcpSrvOpts;
              
              char tmpOption[64];

              int OpFragNum = 0, i = 0;
              
              memset(tmpOption, 0, 64);
              
              memset(&dhcpSrvOpts, 0, sizeof(DHCPOptions));
              if(DB_GET_OK == BcmDb_getDhcpOptionInfo(&dhcpSrvOpts))
              {
                    OpFragNum = strlen(dhcpSrvOpts.option240)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option240 + i*61, 61);
	                    sprintf(cmd, "option2 option240 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
                    OpFragNum = strlen(dhcpSrvOpts.option241)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option241 + i*61, 61);
	                    sprintf(cmd, "option2 option241 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
                    OpFragNum = strlen(dhcpSrvOpts.option242)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option242 + i*61, 61);
	                    sprintf(cmd, "option2 option242 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
                    OpFragNum = strlen(dhcpSrvOpts.option243)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option243 + i*61, 61);
	                    sprintf(cmd, "option2 option243 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
                    OpFragNum = strlen(dhcpSrvOpts.option244)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option244 + i*61, 61);
	                    sprintf(cmd, "option2 option244 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
                    OpFragNum = strlen(dhcpSrvOpts.option245)/61;
                    for(i = 0; i <=OpFragNum; i++)
                    {
                            memcpy(tmpOption, dhcpSrvOpts.option245 + i*61, 61);
	                    sprintf(cmd, "option2 option245 %s\n", tmpOption);
	                    fputs(cmd, fs);
                    }
              }          
        }
#endif
#ifdef SUPPORT_PORTMAPING
         sprintf(cmd, "option60str %s\n", option60);
         fputs(cmd, fs);        
#endif
      fclose(fs);
      /* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
      //creatDnsCfg( );
    FILE *fp = NULL;
    int NumOfDns = 0;
    int i = 0;
    PDNS_CFG pdnsInfo = NULL;
     BcmDb_getDnsdomainSize(&NumOfDns);
    if (0 != NumOfDns)
    {
        pdnsInfo = malloc(sizeof(DNS_CFG)*NumOfDns);
        if ( NULL == pdnsInfo )
        {
            return FILE_OPEN_ERR;
        }
        if ( DB_GET_NOT_FOUND == BcmDb_getDnsDomainInfo(pdnsInfo, sizeof(DNS_CFG)*NumOfDns))
        {
            return FILE_OPEN_ERR;
        }
        fp = fopen(DNS_CFG_FILE,"w");
        if ( NULL != fp)
        {
            for ( i = 0 ; i < NumOfDns; i++ )
            {
                fprintf(fp,"%s %s\n",pdnsInfo[i].cMac,pdnsInfo[i].cDnsDomain);
            }
            fclose(fp);
        }
        free(pdnsInfo);
    }

      /* END:   Added by y67514, 2008/9/18 */
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}
/*end DHCP Server支持第二地址池, s60000658, 20060616*/
/***************************************************************************
// Function Name: bcmCreateIpExtDhcpCfg().
// Description  : create DHCP server configuration file for PPP IP Extenstion.
// Parameters   : lanAddr - LAN IP address of target.
//                mask - subnet mask of target.
//                wanAddr - WAN IP address.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
/* BEGIN: Modified by y67514, 2008/9/11   问题单号:GLB:Ip Extension*/
//int bcmCreateIpExtDhcpCfg(char *lanAddr, char *mask, char *wanAddr) {
int bcmCreateIpExtDhcpCfg(char *lanAddr, char *mask, char *wanAddr,char*interface,char *mac) {
/* END:   Modified by y67514, 2008/9/11 */
   char cmd[SYS_CMD_LEN], dns[SYS_CMD_LEN];
   #ifdef SUPPORT_VDF_DHCP
   char dns2[SYS_CMD_LEN];
   char ntp1[SYS_CMD_LEN];
   char ntp2[SYS_CMD_LEN];
   #endif
   /* BEGIN: Added by y67514, 2008/9/11   PN:GLB:Ip Extension*/
   char path[IFC_HOST_LEN];
   char router[IFC_TINY_LEN];
   FILE* fp = NULL;
   char *tempchr = NULL;
   /* END:   Added by y67514, 2008/9/11 */
   FILE* fs = fopen("/etc/udhcpd.conf", "w");

   if ( fs != NULL ) {
       // cwu
      sprintf(cmd, "echo %s > /var/run/ip", wanAddr);
      system(cmd);

        /* BEGIN: Added by y67514, 2008/9/11   问题单号:GLB:Ip Extension*/
        /*enblsrv must be added*/
        sprintf(cmd, "enblsrv1 %s\n", "1");
        fputs(cmd, fs);
        /* END:   Added by y67514, 2008/9/11 */

      // start IP address
      sprintf(cmd, "start %s\n", wanAddr);
      fputs(cmd, fs);

      // end IP address
      sprintf(cmd, "end %s\n", wanAddr);
      fputs(cmd, fs);

      // interface
      fputs("interface br0\n", fs);

      // lease
      fputs("option lease 30\n", fs);
      fputs("option min_lease 30\n", fs);

      // subnet mask
      sprintf(cmd, "option subnet %s\n", mask);
      fputs(cmd, fs);

      // router
      /* BEGIN: Added by y67514, 2008/9/11   PN:GLB:Ip Extension*/
      memset(path,0,sizeof(path));
      memset(router,0,sizeof(router));
      sprintf(path, "/var/fyi/sys/%s/gateway", interface);
      fp = fopen(path,"r");
      if  ( NULL != fp )
      {
            if ( NULL != fgets(router, IFC_TINY_LEN, fp) ) 
            {
                if ( tempchr = strchr(router,'\n'))
                {
                    *tempchr = '\0';
                }
                sprintf(cmd, "option router %s\n", router);
                fputs(cmd, fs);
            }
            fclose(fp); 
      }

      if ( mac )
      {
          sprintf(cmd, "macip %s / %s\n", mac, wanAddr);
          fputs(cmd, fs);
      }

      /* END:   Added by y67514, 2008/9/11 */

      // don't use DNS relay since there is no ip table for
      // PPP IP extension
        /* BEGIN: Modified by y67514, 2008/10/5   问题单号:GLB:Ip Extension*/
/*start of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/
        bcmGetIpExtDns(interface, dns);
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/
        sprintf(cmd, "option dns %s\n", dns);
        fputs(cmd, fs);
        dns2[0] = '\0';
/*start of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/
        bcmGetIpExtDns2(interface, dns2);
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/
        if(dns2[0] != '\0')
        {
            sprintf(cmd, "option dns %s\n", dns2);           
            fputs(cmd, fs);
        }
        /* END:   Modified by y67514, 2008/10/5 */
        
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmSetIpExtInfo().
// Description  : store wan, gateway, and dns for PPP IP extension.
// Parameters   : wan - WAN IP address.
//                gateway - default gateway.
//                dns - dns.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSetIpExtInfo(char *wan, char *gateway, char *dns) {
   char str[256];
   FILE* fs = fopen("/var/ipextinfo", "w");

   if ( fs != NULL ) {
      sprintf(str, "%s %s %s\n", wan, gateway, dns);
      fputs(str, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmGetIpExtInfo().
// Description  : get wan, gateway, or dns for PPP IP extension.
// Parameters   : buf - .
//                type - .
// Returns      : none.
****************************************************************************/
void bcmGetIpExtInfo(char *buf, int type) {
   FILE* fs;
   char wan[64], gateway[64], dns[64], str[256];

   if ( buf == NULL ) return;

   buf[0] = '\0';
   if( bcmGetAdslStatus() == MNTR_STS_OK ) {
      fs = fopen("/var/ipextinfo", "r");
      if ( fs != NULL ) {
         fgets(str, 256, fs);
         fclose(fs);
         sscanf(str, "%s %s %s\n", wan, gateway, dns);
         switch ( type ) {
         case 0:
            if( dns[0] >= '0' && dns[0] <= '9' )
               strcpy(buf, wan);
            break;
         case 1:
            if( dns[0] >= '0' && dns[0] <= '9' )
               strcpy(buf, gateway);
            break;
         case 2:
            if( dns[0] >= '0' && dns[0] <= '9' )
               strcpy(buf, dns);
            break;
         }
      }
   }
}

/***************************************************************************
// Function Name: bcmCreateLocalDhcpCfg().
// Description  : create DHCP server configuration file with default local.
// Parameters   : ipAddr -- default local IP address.
//                mask -- default local subnet mask.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmCreateLocalDhcpCfg(char *ipAddr, char *mask,char *mac) {

   char cmd[SYS_CMD_LEN], nextAddr[IFC_TINY_LEN];
   
   if( (NULL == ipAddr) || (NULL == mask) || (NULL == mac) )
   {
       return FILE_OPEN_ERR;
   }
   
   FILE* fs = fopen("/etc/udhcpd.conf", "w");
   
   if ( fs != NULL ) {
      /*start of 解决问题单AU8D02021:ppp extension当pc不能获取公网地址时为其分配lan ip by l129990,2009,3,25*/ 
      strcpy(nextAddr, ipAddr);
      /*end of 解决问题单AU8D02021:ppp extension当pc不能获取公网地址时为其分配lan ip by l129990,2009,3,25*/ 
      sprintf(cmd, "echo %s > /var/run/ip", nextAddr);
      system(cmd);

      /* BEGIN: Added by y67514, 2008/9/29   PN:GLB:支持Option15*/
      fprintf(fs, "enblsrv1 %s\n", "1");
      /* END:   Added by y67514, 2008/9/29 */
      
      // start IP address
      fprintf(fs, "start %s\n", nextAddr);

      // end IP address
      fprintf(fs, "end %s\n", nextAddr);

      // interface
      fputs("interface br0\n", fs);

      // lease
      fputs("option lease 10\n", fs);
      fputs("option min_lease 10\n", fs);

      // subnet mask
      fprintf(fs, "option subnet %s\n", mask);

      // router
      fprintf(fs, "option router %s\n", ipAddr);

      // dns
      fprintf(fs, "option dns %s\n", ipAddr);

      /* BEGIN: Added by y67514, 2008/9/29   PN:GLB:PPP EXTENSION*/
      if ( mac )
      {
          fprintf(fs, "macip %s / %s\n", mac, nextAddr);
      }
      /* END:   Added by y67514, 2008/9/29 */
      
#ifdef SUPPORT_VDF_DHCP
        //  ntp
        fprintf(fs, "option ntpsvr %s\n", ipAddr);
#endif
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmCreateResolvCfg().
// Description  : create resolv configuration file.
// Parameters   : dns1 - primary dns.
//                dns2 - secondary dns.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmCreateResolvCfg(char *dns1, char *dns2) {
   char cmd[SYS_CMD_LEN];
   FILE* fs = NULL;

   bcmSystemMute("mkdir -p /var/fyi/sys");
   bcmSystemMute("echo > /var/fyi/sys/dns");
   fs = fopen("/var/fyi/sys/dns", "w");

   if ( fs != NULL ) {
      sprintf(cmd, "nameserver %s\n", dns1);
      fputs(cmd, fs);
      sprintf(cmd, "nameserver %s\n", dns2);
      fputs(cmd, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

// global ADSL info variable is declared here (in syscall.c)
// and is used in syscall.c, sysdiag.c, cgimain.c, and cgists.c
ADSL_CONNECTION_INFO glbAdslInfo;
#if defined(SUPPORT_VDSL)
XDSL_CONNECTION_INFO glbVdslInfo;
#endif

/***************************************************************************
// Function Name: bcmGetXdslStatus().
// Description  : get ADSL or VDSL status, depending on which link is up
// Parameters   : none
// Returns      : Link satus as in BCMXDSL_STATUS
****************************************************************************/
int bcmGetXdslStatus() 
{
  BCMXDSL_STATUS  ret = BCM_XDSL_LINK_DOWN;
  BCMADSL_STATUS  sts;
  
/*Start modify:l37298 2006-05-30 for deleting adsl feature*/	
#ifndef SUPPORT_VDSL
    sts = BcmAdslCtl_GetConnectionInfo(&glbAdslInfo);
    /* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日
    if( sts == BCMADSL_STATUS_SUCCESS )
      ret = glbAdslInfo.LinkState;
    */
    if( sts == BCMADSL_STATUS_SUCCESS )
    {
      ret = glbAdslInfo.LinkState;
      sysPutItfState("adsl", ret);
    }
    /* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
#else
    sts = BcmVdslCtl_GetConnectionInfo(&glbVdslInfo, glbErrMsg);
    /* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日
    if( sts == BCMADSL_STATUS_SUCCESS )
      ret = glbVdslInfo.LinkState;
    */
    if( sts == BCMADSL_STATUS_SUCCESS )
    {
      ret = glbVdslInfo.LinkState;
      sysPutItfState("vdsl", ret);
    }
    /* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
#endif
/*End modify:l37298 2006-05-30 for deleting adsl feature*/	

  return((int)ret);
}

/***************************************************************************
// Function Name: bcmGetAdslStatus().
// Description  : get ADSL status.
// Parameters   : none
// Returns      : 0 - ADSL link Up (OK)
//                1 - ADSL link Down
//                2 - Other error
****************************************************************************/
int bcmGetAdslStatus() {
   int ret = 0;

   if (BcmAdslCtl_GetConnectionInfo(&glbAdslInfo) != BCMADSL_STATUS_ERROR) {
      ret = glbAdslInfo.LinkState;
   } else
      ret = BCMADSL_STATUS_ERROR;

   return ret;
}

#if defined(SUPPORT_VDSL)
/***************************************************************************
// Function Name: bcmGetVdslStatus().
// Description  : get VDSL status.
// Parameters   : none
// Returns      : 0 - VDSL link Up (OK)
//                1 - VDSL link Down
//                2 - Other error
****************************************************************************/
int bcmGetVdslStatus() {
   return((BcmVdslCtl_GetConnectionInfo(&glbVdslInfo, glbErrMsg) != BCMADSL_STATUS_ERROR) ?
           glbVdslInfo.LinkState : BCMADSL_STATUS_ERROR);
}
#endif

/***************************************************************************
// Function Name: bcmGetPppStatus().
// Description  : get PPP status.
// Parameters   : str - buffer to retrieve message
//                len - length of buffer
// Returns      : 0 - OK
//                -1 - ERROR
****************************************************************************/
int bcmGetPppStatus(char *str, int len, char *name) {
   FILE* fs;

   char filePath[100];
   sprintf(filePath,"/proc/var/fyi/wan/%s/daemonstatus",name);
   fs = fopen(filePath, "r");
   if ( fs != NULL ) {
      fgets(str, len, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}
void bcmSetPppouDown(char *name) {
    char cmdfile[IFC_DOMAIN_LEN];
    char cmd[IFC_DOMAIN_LEN];
    sprintf(cmdfile, "/proc/var/fyi/wan/%s/daemonstatus", name);
    sprintf(cmd, "echo 0 > %s", cmdfile);
    bcmSystemMute(cmd);
}

/***************************************************************************
// Function Name: bcmGetDhcpcStatus().
// Description  : get DHCPC status.
// Parameters   : str - buffer to retrieve message
//                len - length of buffer
// Returns      : 0 - OK
//                -1 - ERROR
****************************************************************************/
int bcmGetDhcpcStatus(char *str, int len) {
   FILE* fs = fopen("/var/run/dhcpc", "r");

   if ( fs != NULL ) {
      fgets(str, len, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmGetSystemStatus().
// Description  : get system status.
// Parameters   : str - buffer to retrieve message
//                len - length of buffer
// Returns      : 0 - OK
//                -1 - ERROR
****************************************************************************/
int bcmGetSystemStatus(char *str, int len) {
   FILE* fs = fopen("/etc/sysmsg", "r");

   if ( fs != NULL ) {
      fgets(str, len, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmSetSystemStatus().
// Description  : set system status.
// Parameters   : int - system status
// Returns      : 0 - OK
//                -1 - ERROR
****************************************************************************/
int bcmSetSystemStatus(int status) {
   char cmd[SYS_CMD_LEN];
   FILE* fs = fopen("/etc/sysmsg", "w");

   if ( fs != NULL ) {
      sprintf(cmd, "%d\n", status);
      fputs(cmd, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmDisplayLed().
// Description  : display LED corresponding to WAN link status.
// Parameters   : status - WAN link status.
// Returns      : none.
****************************************************************************/
void bcmDisplayLed(int status) {
   switch (status) {
   case MNTR_STS_ADSL_DOWN:
      /* this means ADSL is DOWN */
      sysLedCtrl(kLedPPP, kLedStateOff);
      break;
   case MNTR_STS_ADSL_TRAINING:
      /* this means ADSL is TRAINING */
      sysLedCtrl(kLedPPP, kLedStateOff);
      break;
   case MNTR_STS_PPP_AUTH_ERR:
      sysLedCtrl(kLedPPP, kLedStateFail);
      break;
   case MNTR_STS_PPP_DOWN:
      /* this means ADSL is UP, but not PPP */
      sysLedCtrl(kLedPPP, kLedStateFail);
      break;
   case MNTR_STS_OK:
      /* this means ADSL and PPP are up */
      sysLedCtrl(kLedPPP, kLedStateOn);
      break;
   }
}

void bcmGetDynamicDnsAddr(char *dns, int primary) {
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;

    /* BEGIN: Modified by y67514, 2008/10/21   问题单号:AU8D01013:网关在使用动态IPOE上网时，当本地设置的DNS和局端下发的DNS不同时，不能优先采用本地设置*/
   //fs = fopen("/var/fyi/sys/dynamicdns", "r");
    fs = fopen("/var/fyi/sys/dns", "r");
    /* END:   Modified by y67514, 2008/10/21 */
   if ( fs != NULL ) {
      if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) {
         if (primary)
            sscanf(str, "nameserver %s\n", dns);
         else {
            if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
            sscanf(str, "nameserver %s\n", dns);
         } /* secondary */
      }
      fclose(fs);
   }
   else
      // if cannot find primary dns info then
      // assign default value which is router IP address
      bcmGetIfDestAddr("br0", dns);
}

/***************************************************************************
// Function Name: bcmGetDns().
// Description  : get DSN info.
// Parameters   : dns - buffer to retrieve primary dns.
// Returns      : none.
****************************************************************************/
void bcmGetDns(char *dns) {
   IFC_DNS_INFO dnsInfo;
   int sts = BcmDb_getDnsInfo(&dnsInfo);

   dns[0] = '\0';
   if ( sts == DB_GET_OK ) {
      if ( dnsInfo.dynamic == TRUE )
         bcmGetDynamicDnsAddr(dns,BCM_PRIMARY_DNS);
      else {
         if ( dnsInfo.preferredDns.s_addr != INADDR_NONE )
            strcpy(dns, inet_ntoa(dnsInfo.preferredDns));
      }	  	
   } else
      // in bcmGetDynamicDnsAddr, if cannot find primary dns info then
      // assign default value which is router IP address
      bcmGetDynamicDnsAddr(dns,BCM_PRIMARY_DNS);
}

/***************************************************************************
// Function Name: bcmGetDns2().
// Description  : get DSN info.
// Parameters   : dns - buffer to retrieve primary dns.
// Returns      : none.
****************************************************************************/
void bcmGetDns2(char *dns) {
   IFC_DNS_INFO dnsInfo;
   int sts = BcmDb_getDnsInfo(&dnsInfo);

   dns[0] = '\0';
   if ( sts == DB_GET_OK ) {
      if ( dnsInfo.dynamic == TRUE )
         bcmGetDynamicDnsAddr(dns,BCM_SECONDARY_DNS);
      else {
         if ( dnsInfo.alternateDns.s_addr != INADDR_NONE )
            strcpy(dns, inet_ntoa(dnsInfo.alternateDns));
      }	  	
   } else
      // in bcmGetDynamicDnsAddr, if cannot find primary dns info then
      // assign default value which is router IP address
      bcmGetDynamicDnsAddr(dns,BCM_SECONDARY_DNS);
}

/*start of 解决AU8D01490问题单:增加PPP IP Extension从接口文件中获取dns信息 by l129990,2008,12,16*/
/***************************************************************************
// Function Name: bcmGetIpExtDynamicDnsAddr().
// Description  : get DSN info from temporary file.
// Parameters   : dns - buffer to retrieve primary dns.
// Returns      : none.
****************************************************************************/
void bcmGetIpExtDynamicDnsAddr(char *interface, char *dns, int primary) {
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;
   char path[128] = {0};

    /* BEGIN: Modified by y67514, 2008/10/21   问题单号:AU8D01013:网关在使用动态IPOE上网时，当本地设置的DNS和局端下发的DNS不同时，不能优先采用本地设置*/
   //fs = fopen("/var/fyi/sys/dynamicdns", "r");
    if ( interface != NULL )
    { 
        sprintf(path,"/var/fyi/sys/%s/dns",interface);
        fs = fopen(path, "r");
    /* END:   Modified by y67514, 2008/10/21 */
        if ( fs != NULL ) 
        {
            if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
            {
                if (primary)
                {
                    sscanf(str, "nameserver %s\n", dns);
                }
                else 
                {
                    if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
                    {
                        sscanf(str, "nameserver %s\n", dns);
                    }
                } /* secondary */
           }
           fclose(fs);
       }
       else
       {   // if cannot find primary dns info then
           // assign default value which is router IP address
           bcmGetIfDestAddr("br0", dns);
       }
   }
   else
   {
       // if cannot find primary dns info then
       // assign default value which is router IP address
      
       bcmGetIfDestAddr("br0", dns);
   }
}
/*end of 解决AU8D01490问题单:增加PPP IP Extension从接口文件中获取dns信息 by l129990,2008,12,16*/

/*start of 解决AU8D01490问题单:增加PPP IP Extension获取主dns信息 by l129990,2008,12,16*/
/***************************************************************************
// Function Name: bcmGetIpExtDns().
// Description  : get DSN info.
// Parameters   : dns - buffer to retrieve primary dns.
// Returns      : none.
****************************************************************************/
void bcmGetIpExtDns(char *interface, char *dns) {
   IFC_DNS_INFO dnsInfo;
   
   if ( interface == NULL)
   {
       dns[0] = '\0';
       return;
   }
   
   int sts = BcmDb_getIpExtDnsInfo(interface,&dnsInfo);

   dns[0] = '\0';
   if ( sts == DB_GET_OK ) 
   {
       if ( dnsInfo.dynamic == TRUE )
       {
           bcmGetIpExtDynamicDnsAddr(interface,dns,BCM_PRIMARY_DNS);
       }
       else 
       {
           if ( dnsInfo.preferredDns.s_addr != INADDR_NONE )
           {
               strcpy(dns, inet_ntoa(dnsInfo.preferredDns));
           }
       }	  	
   } 
   else
      // in bcmGetDynamicDnsAddr, if cannot find primary dns info then
      // assign default value which is router IP address
   {
       bcmGetIpExtDynamicDnsAddr(interface,dns,BCM_PRIMARY_DNS);
   }
}
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取主dns信息 by l129990,2008,12,16*/
/*start of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/
/***************************************************************************
// Function Name: bcmGetIpExtDns2().
// Description  : get DSN info.
// Parameters   : dns - buffer to retrieve primary dns.
// Returns      : none.
****************************************************************************/
void bcmGetIpExtDns2(char *interface, char *dns) {
   IFC_DNS_INFO dnsInfo;

   if ( interface == NULL)
   {   
       dns[0] = '\0';
       return;
   }
   
   int sts = BcmDb_getIpExtDnsInfo(interface,&dnsInfo);

   dns[0] = '\0';
   if ( sts == DB_GET_OK ) 
   {
       if ( dnsInfo.dynamic == TRUE )
       {
           bcmGetIpExtDynamicDnsAddr(interface,dns,BCM_SECONDARY_DNS);
       }
       else 
       {
           if ( dnsInfo.alternateDns.s_addr != INADDR_NONE )
           { 
               strcpy(dns, inet_ntoa(dnsInfo.alternateDns));
           }
      }	  	
   } 
   else
      // in bcmGetDynamicDnsAddr, if cannot find primary dns info then
      // assign default value which is router IP address
  {
      bcmGetIpExtDynamicDnsAddr(interface,dns,BCM_SECONDARY_DNS);
  }
}
/*end of 解决AU8D01490问题单:增加PPP IP Extension获取备dns信息 by l129990,2008,12,16*/

void bcmGetDnsSettings(int *mode, char *primary, char *secondary)
{
   IFC_DNS_INFO dnsInfo;
   
   *mode = 1;
   if (BcmDb_getDnsInfo(&dnsInfo) == DB_GET_OK) {
      *mode = dnsInfo.dynamic;
   }
   bcmGetDns(primary);
   bcmGetDns2(secondary);
}

/*start of waninfo by c65985 for vdf 080314*/
#ifdef SUPPORT_VDF_WANINFO
/***************************************************************************
// Function Name: bcmGetDSLInfInfo().
// Description  : get DSLInf info.
// Parameters   : info - buffer to retrieve DSLInfInfo.
// Returns      : none.
****************************************************************************/
void bcmGetDSLInfInfo(char *info) 
{   
    /*start of HG553 2008.03.27 V100R001C02B010 AU8D00366 by c65985 */
    /*start of HG553 2008.03.27 V100R001C02B010 AU8D00335 by c65985 */
    //int             lcount = 0;
    /*end of HG553 2008.03.27 V100R001C02B010 AU8D00335 by c65985 */
    /*end of HG553 2008.03.27 V100R001C02B010 AU8D00366 by c65985 */
    char            interface[IFC_TINY_LEN];
    char            address[IFC_TINY_LEN];
    char            dns[IFC_TINY_LEN];
    char            gateway[IFC_TINY_LEN];
    /*start of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */
    char            protocol[IFC_TINY_LEN];
    /*end of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */     
    char            htmlinfo[1024];
    /*start of HG553 2008.03.27 V100R001C02B010 AU8D00340 by c65985 */
    char            gwWanIf[IFC_TINY_LEN];
    WAN_IP_INFO     ipInfo;
    /*end of HG553 2008.03.27 V100R001C02B010 AU8D00340 by c65985 */
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;
    /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
    unsigned short  status = IFC_STATUS_UNKNOWN;
    /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
    
	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;  
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{ 
        /*start of HG553 2008.03.29 V100R001C02B010 AU8D00366 by c65985 */
        if ( !(HSPA_VPI == wanId.vpi && HSPA_VCI == wanId.vci) )
        /*start of HG553 2008.03.29 V100R001C02B010 AU8D00366 by c65985 */
        {
            interface[0]    = '\0';
            address[0]      = '\0';
            dns[0]          = '\0';
            gateway[0]      = '\0';
            htmlinfo[0]     = '\0';
            /*start of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */
            protocol[0]     = '\0';

            BcmDb_getWanProtocolName(wanInfo.protocol, protocol);
            /*end of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */    
	        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);           
    	    BcmWan_getIpAddress(&wanId, address, IFC_TINY_LEN);
            /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            status = BcmWan_getStatus(&wanId);
            /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */

            /*start of HG553 2008.03.27 V100R001C02B010 AU8D00340 by c65985 */

            /* There is no IP info for bridge mode */
            if ( PROTO_BRIDGE == wanInfo.protocol )
            {
                /* BEGIN: Deleted by y67514, 2008/10/22   问题单号:AU8D00962:WAN侧为桥接方式时，通过web页面查看，没有显示该条PVC的信息。*/
                //continue;
                /* END: Deleted by y67514, 2008/10/22 */
            }

            if ( PROTO_MER == wanInfo.protocol || PROTO_IPOA == wanInfo.protocol )
            {
                BcmDb_getIpSrvInfo(&wanId, &ipInfo);
            }
            /*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
            if ( (PROTO_MER == wanInfo.protocol || PROTO_IPOA == wanInfo.protocol) && (!ipInfo.enblDhcpClnt))
            { 
                /*start of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */ 
                /* dhcp方式的wan不能配置静态缺省网关 */
                //bcmGetDefaultGateway(gateway, gwWanIf); 
                bcmGetStaticDefaultGatewayByWanIfc(gateway, (void *)&wanId, gwWanIf); 
                /*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
            }
            else
            {
                bcmGetGWByInf(gateway, interface);
            }
            /*end of HG553 2008.03.27 V100R001C02B010 AU8D00340 by c65985 */
            bcmGetWanDNSByIdIfc(dns, (void *)&wanId, interface);
            /*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
            if ( wanInfo.conName[0] != '\0' )
            {
                /*start of 修改问题单AU8D00817  by s53329  at   20080715
                sprintf(htmlinfo, "<strong>%s </strong>\n", wanInfo.conName);
                */
                sprintf(htmlinfo, "<table><tr><td>");
                strcat(info, htmlinfo);
                sprintf(htmlinfo, "<strong  style=\"word-break:break-all;\">%s </strong>\n", wanInfo.conName);
                strcat(info, htmlinfo);
                sprintf(htmlinfo, "</td></tr></table>");
                 /*end  of 修改问题单AU8D00817  by s53329  at   20080715 */
                strcat(info, htmlinfo);
            }
            else
            {
                sprintf(htmlinfo, "<strong>VPI/VCI:%d/%d </strong>\n", wanId.vpi, wanId.vci);
                strcat(info, htmlinfo);
            }

            strcat(info, "<table class=\"tableForm2\">\n");
            /*start of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */ 
            strcat(info, "<tr>\n");
            strcat(info, "<td width=\"53%\">&nbsp;&nbsp;&nbsp;&nbsp;");
            strcat(info, WEB_STR_PROTOCOL);
            strcat(info, ":</td>\n");

            if ( protocol[0] == '\0' )
            {
                strcat(info, "<td width=\"47%\">&nbsp;</td>\n");
            }
            if ( protocol[0] != '\0' )
            {
                sprintf(htmlinfo, "<td width=\"47%%\">%s</td>\n", protocol);
                strcat(info, htmlinfo);
            }
            
            strcat(info, "</tr>\n");
            /*end of HG553 2008.03.26 V100R001C02B010 AU8D00284 by c65985 */             
            strcat(info, "<tr>\n");
            strcat(info, "<td width=\"53%\">&nbsp;&nbsp;&nbsp;&nbsp;");
            strcat(info,WEB_STR_IP_ADDR);
            strcat(info,":</td>\n");
            
            /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            if ( strstr(address, "xxx.xxx.xxx.xxx") != NULL || address[0] == '\0' || status != IFC_STATUS_UP )
            /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */

            {
                address[0] = '\0';
                strcat(info, "<td width=\"47%\">0.0.0.0</td>\n");
            }
            if ( address[0] != '\0' )
            {
                sprintf(htmlinfo, "<td width=\"47%%\">%s</td>\n", address);
                strcat(info, htmlinfo);
            }
            
            strcat(info, "</tr>\n");
            strcat(info, "<tr>\n");
            strcat(info, "<td>&nbsp;&nbsp;&nbsp;&nbsp;DNS:</td>\n");

            /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            if ( strstr(dns, "xxx.xxx.xxx.xxx") != NULL || dns[0] == '\0' || status != IFC_STATUS_UP )
            /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            {
                dns[0] = '\0';
                strcat(info, "<td>0.0.0.0</td>\n");
            }
            if ( dns[0] != '\0')
            {
                sprintf(htmlinfo, "<td>%s</td>\n", dns);
                strcat(info, htmlinfo);
            }            

            strcat(info, "</tr>\n");
            strcat(info, "<tr>\n");
            strcat(info, "<td>&nbsp;&nbsp;&nbsp;&nbsp;Gateway:</td>\n");

            /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            if ( strstr(gateway, "xxx.xxx.xxx.xxx") != NULL || gateway[0] == '\0' || status != IFC_STATUS_UP )
            /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
            {
                gateway[0] = '\0';
                strcat(info, "<td>0.0.0.0</td>\n");
            }
            if ( gateway[0] != '\0' )
            {
                sprintf(htmlinfo, "<td>%s</td>\n", gateway);
                strcat(info, htmlinfo);
            }
            
            strcat(info, "</tr>\n");
            strcat(info, "</table>\n"); 

/*start of HG553 2008.03.27 V100R001C02B010 AU8D00366 by c65985 */
/*start of HG553 2008.03.27 V100R001C02B010 AU8D00335 by c65985 */
         /*   lcount++;
            
            if ( NUM_SHOWPVC == lcount )
            {
                break;
            } */
/*end of HG553 2008.03.27 V100R001C02B010 AU8D00335 by c65985 */
/*end of HG553 2008.03.27 V100R001C02B010 AU8D00366 by c65985 */
        }
	}
}

/***************************************************************************
// Function Name: bcmIsOnePVCUp().
// Description  : return TRUE if one PVC is up.
// Parameters   : void.
// Returns      : return TRUE if one PVC is up.
****************************************************************************/
int bcmIsOnePVCUp(void)
{
    char            interface[IFC_TINY_LEN];
    char            address[IFC_TINY_LEN];
    char            protocol[IFC_TINY_LEN];
    char            gwWanIf[IFC_TINY_LEN];
    WAN_IP_INFO     ipInfo;
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;
    unsigned short  status = IFC_STATUS_UNKNOWN;
    int             ret = FALSE;
   
	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;  
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{ 
        interface[0]    = '\0';
        address[0]      = '\0';
        protocol[0]     = '\0';

        BcmDb_getWanProtocolName(wanInfo.protocol, protocol);
        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);           
        BcmWan_getIpAddress(&wanId, address, IFC_TINY_LEN);

        status = BcmWan_getStatus(&wanId);

        if ( wanInfo.protocol != PROTO_BRIDGE )
        {
            if (status == IFC_STATUS_UP)
            {
                BcmWan_getIpAddress(&wanId, address, IFC_TINY_LEN);
                /* IP address */
                if (strstr(address, "xxx.xxx.xxx.xxx") != NULL || address[0] == '\0')
                {
                    address[0] = '\0';
                }
                if ( address[0] != '\0' )
                {
                    /* Valid IP address on one PVC */
                    ret = TRUE;
                    break;
                }
            }
        }
	}
    return ret;
}


/***************************************************************************
// Function Name: bcmGetDataIp().
// Description  : get DataIp info.
// Parameters   : DataIp - buffer to retrieve DataIp.
// Returns      : none.
****************************************************************************/
void bcmGetDataIp(char *dataIp) 
{
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;
	char            name[32];
    unsigned short  datastatus = IFC_DATAPVCSTATUS_UNKNOWN;

	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        /* Hard-coding data pvc to 10/35 seems to be unflexible. */
        #if 0
        if ( DATA_VPI == wanId.vpi && DATA_VCI == wanId.vci )
        {	                 
    	    BcmWan_getIpAddress(&wanId, dataIp, IFC_TINY_LEN);
    		break;
        }
        #endif
        
        BcmWan_getWanName(&wanId, name, sizeof(name));

        if(strcasecmp(name, WAN_DATA_INTERFACE) == 0 ||
           strcasecmp(name, WAN_DATA_VOICE_INTERFACE ) == 0)
	    {
	    /*Begin:Added by luokunling 00192527,2011/12/28*/
            datastatus = BcmWanDataPvc_getStatus(&wanId);
           	BcmWan_getIpAddress(&wanId, dataIp, IFC_TINY_LEN); 

            if (strstr(dataIp, "xxx.xxx.xxx.xxx") != NULL ||
                 dataIp[0] == '\0' ||datastatus == IFC_DATAPVCSTATUS_DOWN)
            {
                dataIp[0] = '\0';
            }
	/*End:Added by luokunling 00192527,2011/12/28*/
            else
            {
                /* found valid data-pvc ip. <tanyin 2009.3.27> */
    		    break;
            }
	    }
	}
}

/***************************************************************************
// Function Name: bcmGetVoiceIp().
// Description  : get Voice ip info.
// Parameters   : voiceIp - buffer to retrieve voiceIp.
// Returns      : none.
****************************************************************************/
void bcmGetVoiceIp(char *voiceIp) 
{
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;
    char            name[32];
    unsigned short  status = IFC_STATUS_UNKNOWN;

	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        /* Hard-coding voice pvc to 10/36 seems to be unflexible. */
        #if 0
        if ( VOICE_VPI == wanId.vpi && VOICE_VCI == wanId.vci )
        {
            BcmWan_getIpAddress(&wanId, voiceIp, IFC_TINY_LEN);
    		break;
        }
        #endif
        BcmWan_getWanName(&wanId, name, sizeof(name));
        if(strcasecmp(name, WAN_VOICE_SERVICENAME) == 0)
	    {
            status = BcmWan_getStatus(&wanId);
           	BcmWan_getIpAddress(&wanId, voiceIp, IFC_TINY_LEN); 
            if ( strstr(voiceIp, "xxx.xxx.xxx.xxx") != NULL ||
                 voiceIp[0] == '\0' ||
                 status != IFC_STATUS_UP )
            {
                voiceIp[0] = '\0';
            }

		    break;
	    }
	}
}

/***************************************************************************
// Function Name: bcmGetHspaIp().
// Description  : get Hspa ip info.
// Parameters   : HspaIp - buffer to retrieve HspaIp.
// Returns      : none.
****************************************************************************/
void bcmGetHspaIp(char *hspaIp) 
{
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;
    /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
    unsigned short  status = IFC_STATUS_UNKNOWN;
    /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */

	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
        if ( HSPA_VPI == wanId.vpi && HSPA_VCI == wanId.vci )
        {
            status = BcmWan_getStatus(&wanId);
            if ( IFC_STATUS_UP != status )
            {
                strcpy(hspaIp, "0.0.0.0");
            }
            else
            {
    	        BcmWan_getIpAddress(&wanId, hspaIp, IFC_TINY_LEN);
            }
    		break;
        }
        /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
	}
}

/***************************************************************************
// Function Name: bcmGetIpByIfc().
// Description  : get interface ip by its name.
// Parameters   : address -- interface ip.
//                ifcName -- interface name.
// Returns      : interface ip
****************************************************************************/
int bcmGetIpByIfc(char *address, char *ifcName) 
{  
   struct ifreq ifr;
   struct in_addr wan_ip;  
   int s = 0;

   if ( ifcName == NULL || address == NULL ) 
   {
       return -1;
   }

   if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
   {
       return -1;
   }

   strcpy(ifr.ifr_name, ifcName);

   if ( ioctl(s, SIOCGIFADDR, &ifr) < 0 ) 
   {
      close(s);
      return -1;
   }

   if( ifr.ifr_addr.sa_family == AF_INET )
   {  
      wan_ip = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr;    
      strcpy(address, inet_ntoa(wan_ip));
   }
   
   close(s);   
   return 0;
}

/***************************************************************************
// Function Name: bcmGetDataGw().
// Description  : get DataGw info.
// Parameters   : DataGw - buffer to retrieve DataGw.
// Returns      : none.
****************************************************************************/
void bcmGetDataGw(char *dataGw) 
{
    char            interface[IFC_TINY_LEN];
    WAN_CON_INFO    wanInfo;
	WAN_CON_ID      wanId;

	wanId.vpi       = 0;
	wanId.vci       = 0;
	wanId.conId     = 0;
    interface[0]    = '\0';
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        if ( DATA_VPI == wanId.vpi && DATA_VCI == wanId.vci )
        {
	        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
    	    bcmGetGWByInf(dataGw, interface);
            break;
        }
	}
}

/***************************************************************************
// Function Name: bcmGetvoiceGw().
// Description  : get voiceGw info.
// Parameters   : voiceGw - buffer to retrieve voiceGw.
// Returns      : none.
****************************************************************************/
void bcmGetVoiceGw(char *voiceGw) 
{
    char interface[IFC_TINY_LEN];
    WAN_CON_INFO wanInfo;
	WAN_CON_ID wanId;

	wanId.vpi = 0;
	wanId.vci = 0;
	wanId.conId = 0;
    interface[0] = '\0';
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        if ( VOICE_VPI == wanId.vpi && VOICE_VCI == wanId.vci )
        {
	        BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
    	    bcmGetGWByInf(voiceGw, interface);
    		break;
    	    
        }
	}
}

/***************************************************************************
// Function Name: bcmGetHspaGw().
// Description  : get hspaGw info.
// Parameters   : hspaGw - buffer to retrieve hspaGw.
// Returns      : none.
****************************************************************************/
void bcmGetHspaGw(char *hspaGw) 
{
    char interface[IFC_TINY_LEN];
    WAN_CON_INFO wanInfo;
	WAN_CON_ID wanId;
    /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
    unsigned short  status = IFC_STATUS_UNKNOWN;
    /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */

	wanId.vpi = 0;
	wanId.vci = 0;
	wanId.conId = 0;
    interface[0] = '\0';
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
        if ( HSPA_VPI == wanId.vpi && HSPA_VCI == wanId.vci )
        {
            status = BcmWan_getStatus(&wanId);
            if ( IFC_STATUS_UP != status )
            {
                strcpy(hspaGw, "0.0.0.0");
            }
            else
            {
	            BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
    	        bcmGetGWByInf(hspaGw, interface);
            }
    		break;    	    
        }
        /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
	}
}

/***************************************************************************
// Function Name: bcmGetGWByInf().
// Description  : get DataGw by interface.
// Parameters   : gtwy - buffer to retrieve Gw.                   
// Returns      : none.
****************************************************************************/
int bcmGetGWByInf(char *gtwy, char *wanIf) {
   char  str[SYS_CMD_LEN];
   FILE* fs = NULL;
   char  addr[SYS_CMD_LEN];
   char  ip[SYS_CMD_LEN];
   char  path[SYS_CMD_LEN];

   memset(str,  0, sizeof(str));
   memset(addr, 0, sizeof(addr));
   memset(ip,   0, sizeof(ip));
   memset(path, 0, sizeof(path));

   if ( NULL == gtwy || NULL == wanIf )
   {
      return -1;
   }

   sprintf(path, "/var/fyi/sys/%s/gateway", wanIf);
  
   fs = fopen(path, "r");
   if ( NULL == fs )
   {
      return -1;
   }   
   
   if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
   {
      sscanf(str, "%s\n", addr);
      
      if ( DB_OBJ_VALID_OK == BcmDb_validateIpAddress(addr) )
      {
         strcpy(gtwy, addr);
         fclose(fs);
         return 0;
      }
      else 
      {  // use ifName
         if ( BCM_DIAG_PASS == bcmGetIfDestAddr(addr, ip) )
         {
            strcpy(gtwy, ip);
            fclose(fs);
            return 0;
         }
      }
   }

   fclose(fs);
   return -1;  
}

/*start of HG553 2008.03.24 V100R001C02B010 AU8D00297 by c65985 */
/***************************************************************************
// Function Name: bcmGetHspaDNS().
// Description  : get hspadns info.
// Parameters   : hspadns - buffer to retrieve hspadns.
// Returns      : none.
****************************************************************************/
void bcmGetHspaDNS(char *hspadns) 
{
    char interface[IFC_TINY_LEN];
    WAN_CON_INFO wanInfo;
	WAN_CON_ID wanId;
    /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
    unsigned short  status = IFC_STATUS_UNKNOWN;
    /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */

	wanId.vpi = 0;
	wanId.vci = 0;
	wanId.conId = 0;
    interface[0] = '\0';
    
    while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK )
	{
        /*start of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
        if ( HSPA_VPI == wanId.vpi && HSPA_VCI == wanId.vci )
        {
            status = BcmWan_getStatus(&wanId);
            if ( IFC_STATUS_UP != status )
            {
                strcpy(hspadns, "0.0.0.0");
            }
            else
            {
	            BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
    	        bcmGetDNSByInf(hspadns, interface); 
            }
    		break;    	    
        }
        /*end of HG553 2008.04.02 V100R001C02B010 AU8D00325 by c65985 */
	}
}

/***************************************************************************
// Function Name: bcmGetDNSByInf().
// Description  : get DataGw by interface.
// Parameters   : dns - buffer to retrieve dns.                   
// Returns      : none.
****************************************************************************/
int bcmGetDNSByInf(char *dns, char *wanIf) {
   char  str[SYS_CMD_LEN];
   FILE* fs = NULL;
   char  path[SYS_CMD_LEN];

   memset(str,  0, sizeof(str));
   memset(path, 0, sizeof(path));

   if ( NULL == dns || NULL == wanIf )
   {
      return -1;
   }

   sprintf(path, "/var/fyi/sys/%s/dns", wanIf);
  
   fs = fopen(path, "r");
   if ( NULL == fs )
   {
      return -1;
   }   
   
   if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) 
   {
      sscanf(str, "nameserver %s\n", dns);
      fclose(fs);
      return 0;
   }

   fclose(fs);
   return -1;  
}
/*end of HG553 2008.03.24 V100R001C02B010 AU8D00297 by c65985 */
#endif
/*end of waninfo by c65985 for vdf 080314*/

/*start of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */
int bcmGetWanDNSByIdIfc(char *pDnsAddr, void * pWanId, char * pWanIfcName) 
{
    char  szStr[SYS_CMD_LEN];
    FILE* fs = NULL;
    char  szPath[SYS_CMD_LEN];
    WAN_CON_INFO stWanInfo;
    
    memset(szStr,  0, sizeof(szStr));
    memset(szPath, 0, sizeof(szPath));
    memset(&stWanInfo, 0, sizeof(WAN_CON_INFO));
    
    if ( (NULL == pDnsAddr) || (NULL == pWanId) || (NULL == pWanIfcName) ) /* Modify for codeview */
    {
        return -1;
    }
    BcmDb_getWanInfo((PWAN_CON_ID)pWanId, &stWanInfo);
    /* dns is static,read psi only  */
    if(0 != stWanInfo.stDns.preferredDns.s_addr)
    {
        strcpy(pDnsAddr, inet_ntoa(stWanInfo.stDns.preferredDns));
    }
    /* dns is dynamic,read config file only */
    else
    {
        sprintf(szPath, "/var/fyi/sys/%s/dns", pWanIfcName);

        fs = fopen(szPath, "r");
        if ( NULL == fs )
        {
            return -1;
        }   

        if ( fgets(szStr, SYS_CMD_LEN, fs) > 0 ) 
        {
            sscanf(szStr, "nameserver %16s\n", pDnsAddr);/* only 16 char, Modify for codeview */
            fclose(fs);
            return 0;
        }

        fclose(fs);
    }
    return -1; 
}
/*end of VDF 2008.6.21 V100R001C02B018 j00100803 AU8D00739 */

/***************************************************************************
// Function Name: bcmRestartDnsProbe().
// Description  : start DNS probe.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmRestartDnsProbe(void) {
   char cmd[CLI_MAX_BUF_SZ];

   // kill the old dnsprobe if it is existed
   int pid = bcmGetPid("/bin/dnsprobe");
   if ( pid > 0 ) {
      sprintf(cmd, "kill -9 %d", pid);
      bcmSystem(cmd);
   }

   // start the new dnsprobe
   bcmSystem("/bin/dnsprobe &");
}

/* Start of dns 3.1.12 porting by c47036 20060508 */
#if 0
/***************************************************************************
// Function Name: bcmConfigDns().
// Description  : add or remove DNS info to PSI.
// Parameters   : primary and secondary DNS.
// Returns      : none.
****************************************************************************/
void bcmConfigDns(char *dns1, char *dns2, int dynamic) {
   //char cmd[CLI_MAX_BUF_SZ], addr[CLI_MAX_BUF_SZ], buf[CLI_MAX_BUF_SZ];
   IFC_DNS_INFO dnsInfo;
    // need to call getDnsInfo before setDnsInfo to retrieve domain name
   int sts = BcmDb_getDnsInfo(&dnsInfo);

   // You must reboot the router to make the new configuration effective. -- c47036
#if 0
   // get local ip address
   bcmGetIfDestAddr("br0", addr);

   // if user changes from static to auto assgined dns
   if (dynamic) {
      if ( sts == DB_GET_OK &&
           bcmIsModuleInserted("iptable_nat") == TRUE ) {
         strcpy(buf, inet_ntoa(dnsInfo.preferredDns));
         // del the old rule
         sprintf(cmd, "iptables -t nat -D PREROUTING_UTILITY -i br0 -d %s -p udp --dport 53 -j DNAT --to %s 2>/dev/null", addr, buf);
         bcmSystemMute(cmd);
         // remove old resolve configuration file
         bcmSystemMute("rm /var/fyi/sys/dns");
      }
   } else {
      // create the new resolv.conf with new dsn info
      bcmCreateResolvCfg(dns1, dns2);
      // get old dns1 info
      bcmGetDns(buf);
      if ( buf[0] != '\0' ) {
         if (strcmp(buf, dns1) != 0) {
            if ( bcmIsModuleInserted("iptable_nat") == TRUE ) {
               // del the old rule
               sprintf(cmd, "iptables -t nat -D PREROUTING_UTILITY -i br0 -d %s -p udp --dport 53 -j DNAT --to %s 2>/dev/null", addr, buf);
               bcmSystemMute(cmd);
               // add new DNS Forwarding rule
               sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s -p udp --dport 53 -j DNAT --to %s", addr, dns1);
               bcmSystemMute(cmd);
               bcmRestartDnsProbe();
            }
         }
      } else {
         printf("No Existing DNS information from DSL router\n");
         sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s -p udp --dport 53 -j DNAT --to %s", addr, dns1);
         bcmSystemMute(cmd);
      }
      // get old dns2 info
      bcmGetDns2(buf);
      // if old dns2 differs with new one, restart dnsprobe
      if (strcmp(buf, dns2) != 0) {
         if ( bcmIsModuleInserted("iptable_nat") == TRUE )
            bcmRestartDnsProbe();
      }
      dnsInfo.preferredDns.s_addr = inet_addr(dns1);
      dnsInfo.alternateDns.s_addr = inet_addr(dns2);
   } /* dynamic to static */
#endif

   if (dynamic)
   {
      bcmSystemMute("rm /var/fyi/sys/dns");
   }
   else
   {
      bcmCreateResolvCfg(dns1, dns2);
      dnsInfo.preferredDns.s_addr = inet_addr(dns1);
      dnsInfo.alternateDns.s_addr = inet_addr(dns2);      
   }
   printf("pri:%d  sec:%d\r\n", dnsInfo.preferredDns.s_addr, dnsInfo.alternateDns.s_addr);
   dnsInfo.dynamic = dynamic;
   BcmDb_setDnsInfo(&dnsInfo);
}
#endif

/*************************************************
  Function:       bcmConfigDns
  Description:    add or remove DNS info to PSI.
  Calls:          bcmRemoveDnsIpTableRule
                  bcmGetDynamicDnsAddr
                  bcmRestartDnsProbe
                  BcmDb_setDnsInfo
  Called By:      do_cgi
                  cigWanAdd
  Input:          dns1: primary dns address
                  dns2: secondary dns address
                  dynamic: dynamic or static
  Output:         none
  Return:         none
*************************************************/
void bcmConfigDns(char *dns1, char *dns2, int dynamic) 
{
    char cmd[CLI_MAX_BUF_SZ] = {0};
    char addr[IFC_TINY_LEN] = {0};
    char dns[IFC_TINY_LEN] = {0};
    /* BEGIN: Modified by y67514, 2008/10/22   PN:AU8D01013:网关在使用动态IPOE上网时，当本地设置的DNS和局端下发的DNS不同时，不能优先采用本地设置*/
    char interface[IFC_TINY_LEN] = {0};
    /* END:   Modified by y67514, 2008/10/22 */
    IFC_DNS_INFO dnsInfo; 
    FILE *fs = NULL;
    int pid = 0;
    char secondaddr[IFC_TINY_LEN] = {0};
    int secondip = 0;

    // need to call getDnsInfo before setDnsInfo to retrieve domain name
    BcmDb_getDnsInfo(&dnsInfo);
    // get local ip address
    bcmGetIfDestAddr("br0", addr);
    
    if(bcmGetIfAddr("br0:0", secondaddr) > 0)
    {
        secondip = 1;
    }

    if (dynamic)
    {
        if (dynamic == (int)dnsInfo.dynamic)
        {
            syslog(LOG_INFO,"Dns configuration is the same as before.\r\n");
            return;
        }
        else
        {
            // del the old rule
            while (bcmRemoveDnsIpTableRule() == TRUE)
            {
                syslog(LOG_INFO,"Remove the old DNS server.\n");
            }
            #if 0
            // kill the dns probe
            pid = bcmGetPid("/bin/dnsprobe");
            if (pid > 0) 
            {
               sprintf(cmd, "kill -9 %d", pid);
               bcmSystem(cmd);
            }
            //del dns relay
            // setup dns relay rule
            dns[0] = '\0';
            bcmGetDynamicDnsAddr(dns,BCM_PRIMARY_DNS);
            if (dns[0] != '\0')
            {
                sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s "
                             "-p udp --dport 53 -j DNAT --to %s", addr, dns);
                bcmSystemMute(cmd);
                if (secondip)
                {
                    sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s "
                                 "-p udp --dport 53 -j DNAT --to %s", secondaddr, dns);
                    bcmSystemMute(cmd);                    
                }
            }
            #endif
            // setup dns probe configuration
            /* BEGIN: Modified by y67514, 2008/10/22   PN:AU8D01013:网关在使用动态IPOE上网时，当本地设置的DNS和局端下发的DNS不同时，不能优先采用本地设置*/
            if ( BcmGetDefaultInterface(interface) )
            {
                sprintf(cmd,"/var/fyi/sys/%s/staticdns",interface);
                if ( (fs = fopen(cmd, "r")) != NULL )
                {
                    fclose(fs);
                    sprintf(cmd,"cat /var/fyi/sys/%s/staticdns > /var/fyi/sys/dns",interface);
                    bcmSystemMute(cmd);
                    //AtpDnsmasq_serverRestart(1);
                }
                else
                {
                    sprintf(cmd,"/var/fyi/sys/%s/dns",interface);
                    if ( (fs = fopen(cmd, "r")) != NULL )
                    {
                        fclose(fs);
                        sprintf(cmd,"cat /var/fyi/sys/%s/dns > /var/fyi/sys/dns",interface);
                        bcmSystemMute(cmd);
                        //AtpDnsmasq_serverRestart(1);
                    }
                    else
                    {
                        // remove old resolve configuration file
                        bcmSystemMute("rm /var/fyi/sys/dns");                
                    }
                }
            }
            else
            {
                // remove old resolve configuration file
                bcmSystemMute("rm /var/fyi/sys/dns");                
            }
            /* END:   Modified by y67514, 2008/10/22 */
        }
    }
    else
    {
        if ((dynamic == dnsInfo.dynamic) 
            && (inet_addr(dns1) == dnsInfo.preferredDns.s_addr)
            && (inet_addr(dns2) == dnsInfo.alternateDns.s_addr))
        {
            syslog(LOG_INFO,"Dns configuration is the same as before.\r\n");
            return;
        }
        else
        {
            // create the new resolv.conf with new dns info
            bcmCreateResolvCfg(dns1, dns2);
            // del the old rule
            while (bcmRemoveDnsIpTableRule() == TRUE)
            {
                syslog(LOG_INFO,"Remove the old DNS server.\n");
            }
             /*start of 去掉dnsprob 功能 by s53329  at  20080508
            // kill the dns probe
            pid = bcmGetPid("/bin/dnsprobe");
            if (pid > 0) 
            {
               sprintf(cmd, "kill -9 %d", pid);
               bcmSystem(cmd);
            }            
            // add new DNS Forwarding rule
            sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s "
                         "-p udp --dport 53 -j DNAT --to %s", addr, dns1);
            bcmSystemMute(cmd);
            if (secondip)
            {
                sprintf(cmd, "iptables -t nat -A PREROUTING_UTILITY -i br0 -d %s "
                             "-p udp --dport 53 -j DNAT --to %s", secondaddr, dns1);
                bcmSystemMute(cmd);                    
            }   
            if ((dns2[0] != '\0') && inet_addr(dns2))
            {
                // restart the dns probe
                if (bcmGetPid("/bin/dnsprobe") <= 0)
                {
                    bcmSystem("/bin/dnsprobe &");
                }
            }    
            end  of 去掉dnsprob 功能 by s53329  at  20080508  */
        }
        dnsInfo.preferredDns.s_addr = inet_addr(dns1);
        dnsInfo.alternateDns.s_addr = inet_addr(dns2); 
    }

    sprintf(cmd, "echo 1 > /proc/sys/net/ipv4/netfilter/ip_conntrack_dns");
    bcmSystem(cmd);

    dnsInfo.dynamic = dynamic;
    BcmDb_setDnsInfo(&dnsInfo);
    //BcmPsi_flush();
}
/* End of dns 3.1.12 porting by c47036 20060508 */

/* Start of sntp by c47036 20061008 */
int bcmGetSntpExisted(void)
{
    WAN_CON_ID wanId;
    WAN_CON_INFO wanInfo;
    WAN_BRDHCPC_INFO brInfo;

    // init wanId to get WAN info from the beginning
    wanId.vpi = wanId.vci = wanId.conId = 0;
   
    while (BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK)
    {
        if (wanInfo.flag.service == FALSE) 
        {
            continue;
        }
        if (wanInfo.protocol == PROTO_BRIDGE)
        {
            if (BcmDb_getBrDhcpcInfo(&wanId, &brInfo) == DB_WAN_GET_NOT_FOUND)
            {
                continue;
            }
            if (brInfo.enableDhcpc == TRUE)
            {
                return 1;
            }
        }
        else if (wanInfo.protocol != PROTO_NONE)
        {            
            return 1;
        }
        
    }
    return 0;
}
/* End of sntp by c47036 20061008 */

uint32 getAvailableRouteIndex(void)
{
    uint32 index = 0;
    uint32 fixedIndex = 0;//记录实际存的个数
	uint32  i = 0;
    /*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
    uint32  entryControl = 0;
    /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
	uint32  routeTable[IFC_HOST_LEN]; //id 号从1开始
    uint32  useId = 0;
	void *node = NULL;
    char addr[IFC_TINY_LEN], mask[IFC_TINY_LEN];
    char gtwy[IFC_TINY_LEN], ifName[IFC_TINY_LEN];
    //PBcmCfm_llist listNode;

	memset(routeTable, 0, sizeof(routeTable));
	/*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
    while ( (node = BcmRcm_getRouteCfg(node, &index, addr, mask, gtwy, ifName, &entryControl)) != NULL ) {
    /*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
        //printf("======index=%d\n",index);
        for(i=1;i<IFC_HOST_LEN;i++)
        {
			if(index == i)
			{
				routeTable[i] = 1;
			}
		}
		fixedIndex++;
    }

	if(fixedIndex < IFC_HOST_LEN/2)  //已有路由表小于32个才可以再添加路由表
	{
       for(i=1; i<IFC_HOST_LEN; i++)
       {
   		  if(routeTable[i]!=1)
   		  {
             useId = i;
			 break;
   		  }
   	   }
	}
	    
    return useId;
}


/***************************************************************************
// Function Name: bcmRemoveDefaultGatewayByWanIf().
// Description  : remove the defaultGateway configuration if it uses the
//                removed wan interface.
// Parameters   : wanIf - the removed wan interface
// Returns      : none
****************************************************************************/
void bcmRemoveDefaultGatewayByWanIf(char *wanIf) {
   char gtwy[IFC_TINY_LEN], ifName[IFC_TINY_LEN];
   
   bcmGetDefaultGateway(gtwy, ifName);
   
   if ( strcmp(ifName, wanIf) == 0 ) {
      // remove static default gateway in PSI
      BcmDb_removeDefaultGatewayInfo();
      BcmPsi_flush();
   }
}

/***************************************************************************
// Function Name: bcmGetDefaultGateway().
// Description  : get default gateway info.
// Parameters   : gtwy - buffer to retrieve default gateway.
// Returns      : none.
****************************************************************************/
void bcmGetDefaultGateway(char *gtwy, char *wanIf) {
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;
   IFC_DEF_GW_INFO defgw;
   char addr[512], ip[512];

   /*start of ROUTE 3.1.6 porting by z37589 20060511*/
   memset(str, 0, sizeof(str));
   memset(addr, 0, sizeof(addr));
   memset(ip, 0, sizeof(ip));
   /*end of ROUTE 3.1.6 porting by z37589 20060511*/
   
   gtwy[0] = wanIf[0] = '\0';
   if ( BcmDb_getDefaultGatewayInfo(&defgw) == DB_GET_OK ){
      if (strcmp(inet_ntoa(defgw.defaultGateway), "0.0.0.0") != 0)
         strcpy(gtwy, inet_ntoa(defgw.defaultGateway));
      if (defgw.ifName != NULL)
         strcpy(wanIf, defgw.ifName);      
   }
   else {
      fs = fopen("/var/fyi/sys/gateway", "r");
      if ( fs != NULL ) {
         if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) {
            sscanf(str, "%s\n", addr);
            if ( BcmDb_validateIpAddress(addr) == DB_OBJ_VALID_OK ) 
               strcpy(gtwy, addr);
            else { // use ifName
               if (bcmGetIfDestAddr(addr, ip) == BCM_DIAG_PASS)
                  strcpy(gtwy, ip); 
            }
         }
         fclose(fs);
      }
   }
}

/*start of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */ 
/***************************************************************************
// Function Name: bcmGetDefaultGatewayByWanIfc.
// Description  : get default gateway info  by wan interface.
// Parameters   : gtwy - buffer to retrieve default gateway.
// Returns      : none.
// 因为配静态pvc时，每个wan接口下面有一个缺省网关
// 所以通过新增此函数
****************************************************************************/
void bcmGetStaticDefaultGatewayByWanIfc(char * pszGateWay, void * pWanId, char * pszGwWanIfc)
{
    char  szStr[SYS_CMD_LEN];
    FILE* fs = NULL;
    char  szPath[SYS_CMD_LEN];
    WAN_CON_INFO stWanInfo;
    
    memset(szStr,  0, sizeof(szStr));
    memset(szPath, 0, sizeof(szPath));
    memset(&stWanInfo, 0, sizeof(WAN_CON_INFO));
    
    if ( (NULL == pszGateWay) || (NULL == pWanId) || (NULL == pszGwWanIfc) )
    {
        return;
    }
    BcmDb_getWanInfo((PWAN_CON_ID)pWanId, &stWanInfo);
    BcmDb_getWanInterfaceName((PWAN_CON_ID)pWanId, stWanInfo.protocol, pszGwWanIfc);
    /* gateway is static,read psi only  */
    if(0 != stWanInfo.ipGateWay.s_addr)
    {
        strcpy(pszGateWay, inet_ntoa(stWanInfo.ipGateWay));
    }
    return;
}
/*end of VDF 2008.7.4 V100R001C02B018 j00100803 AU8D00758 */
/*start of ROUTE 3.1.7 porting by z37589 20060511*/
/***************************************************************************
// Function Name: bcmGetDefaultGatewaySts(char *gtwy, char *wanIf)
// Description  : get default gateway info.
// Parameters   : gtwy - buffer to retrieve default gateway.
// Returns      : none.
****************************************************************************/
void bcmGetDefaultGatewaySts(char *gtwy, char *wanIf) {
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;
   IFC_DEF_GW_INFO defgw;
   char addr[512], ip[512];

   memset(str, 0, sizeof(str));
   memset(addr, 0, sizeof(addr));
   memset(ip, 0, sizeof(ip));

   strcpy(gtwy, "0.0.0.0");
   if (NULL == wanIf){return;}
   if ( BcmDb_getDefaultGatewayInfo(&defgw) == DB_GET_OK ){
      if (strcmp(inet_ntoa(defgw.defaultGateway), "0.0.0.0") != 0){
         strcpy(gtwy, inet_ntoa(defgw.defaultGateway));}
      if (defgw.ifName != NULL){
         strcpy(wanIf, defgw.ifName);}      
   }
   else {
        sprintf(ip ,"/var/fyi/sys/%s/gateway",wanIf );
      fs = fopen(ip, "r");
      if ( fs != NULL ) {
         if ( fgets(str, SYS_CMD_LEN, fs) > 0 ) {
            sscanf(str, "%s\n", addr);
            if ( BcmDb_validateIpAddress(addr) == DB_OBJ_VALID_OK ){ 
               strcpy(gtwy, addr);}
            else { // use ifName
               if (bcmGetIfDestAddr(addr, ip) == BCM_DIAG_PASS){
                  strcpy(gtwy, ip); }
            }
         }
         fclose(fs);
      }
   }
}

/*************************************************
  Function:       bcmGetProcDefaultGateway()
  Description:    get default gateway info.
  Calls:          无
  Called By:      cgiGetDfltGtwyForWeb(varValue)
                  cgiGetDfltGtwyIfcForWeb(varValue)
  Table Accessed: 无
  Table Updated:  无
  Input:          无
  Output:         gtwy - buffer to retrieve default gateway
                  wanIf - buffer to retrieve default dev
  Return:         1 - used. 0 - not used
  Others:         无
*************************************************/
int bcmGetProcDefaultGateway(char *gtwy, char *wanIf) {
    char col[11][32];
    char line[512];
    struct in_addr addr;
    int  count = 0;
    int  flag = 0;

    memset(col, 0, sizeof(col));
    memset(line, 0, sizeof(line));

    gtwy[0] = wanIf[0] = '\0';

    FILE* fsRoute = fopen("/proc/net/route", "r");
    if ( fsRoute != NULL ) 
    {
        while ( fgets(line, sizeof(line), fsRoute) ) 
        {
            // read pass header line
            if ( count++ < 1 ) continue;
            
            sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                col[0], col[1], col[2], col[3], col[4], col[5],
                col[6], col[7], col[8], col[9], col[10]);
            
            flag = strtol(col[3], (char**)NULL, 16);

            if ((flag & RTF_UP) == RTF_UP) 
            {
                /*bcmGetProcDefaultGateway函数只为Summary页面调用
                  因此wanIf返回值只有在gtwy为空时才会去取
                  如果gtwy不为空那么wanif就不会去取值为空*/
                if ((flag & RTF_GATEWAY) == RTF_GATEWAY) 
                {
                    if (strncmp(col[1],"00000000",8) == 0)
                    {
                        /*str to long*/
                        addr.s_addr = strtoul(col[2], (char**)NULL, 16);
                        strcpy(gtwy, inet_ntoa(addr)); //long to network address
                        fclose(fsRoute);
                        return TRUE;        	
                    }
                }
                else
                {
                    if (strncmp(col[1],"00000000",8) == 0)
                    {
                        strcpy(wanIf, col[0]);
                        fclose(fsRoute);
                        return TRUE;        	
                    }
                }
            }            
        }
        fclose(fsRoute);
    }

    return FALSE;
}

/*end of ROUTE 3.1.7 porting by z37589 20060511*/

/*start of ROUTE 3.1.9 porting by z37589 20060531*/

/*************************************************
  Function:       bcmIsdstRouteExisted()
  Description:    judge route for repeat.
  Calls:          无
  Called By:      void cgiRtRouteAdd(char *query, FILE *fs)
  Table Accessed: 无
  Table Updated:  无
  Input:          无
  Output:         dstAddr - route dst Address
                  dstMask - route dst Netmask
  Return:         1 - used. 0 - not used
  Others:         无
*************************************************/
int bcmIsdstRouteExisted(char *dstAddr, char *dstMask) {
    char col[11][32];
    char line[512];
    char procAddr[40];
    char procMask[40];
    struct in_addr addr;
    struct in_addr netmask;
    int  count = 0;
    int  flag = 0;

    memset(col, 0, sizeof(col));
    memset(line, 0, sizeof(line));
    memset(procAddr, 0, sizeof(procAddr));
    memset(procMask, 0, sizeof(procMask));

    FILE* fsRoute = fopen("/proc/net/route", "r");
    if ( fsRoute != NULL ) 
    {
        while ( fgets(line, sizeof(line), fsRoute) ) 
        {
            // read pass header line
            if ( count++ < 1 ) continue;
            
            sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                col[0], col[1], col[2], col[3], col[4], col[5],
                col[6], col[7], col[8], col[9], col[10]);

            /*start of ROUTE 2006.6.15  V200R001B01D011 A36D02084 */
            flag = strtol(col[3], (char**)NULL, 16);
            if (!(flag & RTF_UP)) 
            { 
                /* Skip interfaces that are down. */
			    continue;
		    }
            /*end of ROUTE 2006.6.15  V200R001B01D011 A36D02084 */
            /*str to long*/
            addr.s_addr = strtoul(col[1], (char**)NULL, 16);
            strcpy(procAddr, inet_ntoa(addr)); //long to network address

            netmask.s_addr = strtoul(col[7], (char**)NULL, 16);
            strcpy(procMask, inet_ntoa(netmask)); //long to network address
            
            if ( (strcmp(procAddr, dstAddr) == 0) && (strcmp(procMask, dstMask) == 0) ) 
            {
               fclose(fsRoute);
               return 1;
            }
        }
        fclose(fsRoute);
    }

    return 0;
}

/*end of ROUTE 3.1.9 porting by z37589 20060531*/
/***************************************************************************
// Function Name: bcmIsDefaultGatewayExisted().
// Description  : Check if the defaultGateway is existed in the route table.
// Parameters   : gw and wanIf
// Returns      : 1 - Exist. 0 - not Exist
****************************************************************************/
int bcmIsDefaultGatewayExisted(char *gw, char *wanIf) {
   char col[11][32];
   char line[512];
   struct in_addr addr;
   int count = 0;
   int flag = 0;

   FILE* fsRoute = fopen("/proc/net/route", "r");
   if ( fsRoute != NULL ) {
      while ( fgets(line, sizeof(line), fsRoute) ) {
         // read pass header line
        if ( count++ < 1 ) continue;
        sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
               col[0], col[1], col[2], col[3], col[4], col[5],
               col[6], col[7], col[8], col[9], col[10]);
        flag = strtol(col[3], (char**)NULL, 16);
        if ((flag & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY)) {
           if ( wanIf[0] == '\0' || strcmp(wanIf, col[0]) == 0) {
             addr.s_addr = strtoul(col[2], (char**)NULL, 16);
             if (strcmp(gw, inet_ntoa(addr)) == 0) {
                fclose(fsRoute);
                return TRUE;
             }
           }
        }
      }
      fclose(fsRoute);
   }

   return FALSE;
}

/*start of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/

/*************************************************
  Function:       bcmHasAnyDefaultGateway()
  Description:    Check if the defaultGateway is existed in the route table.
  Calls:          无
  Called By:      bcmSetAutoDefaultGateway(char *errMsg)
                  bcmSetStaticDefaultGateway(char *gw, char *wanIf, char *errMsg)
  Table Accessed: 无
  Table Updated:  无
  Input:          无
  Output:         无
  Return:         1 - Exist. 0 - not Exist
  Others:         无
*************************************************/
int bcmHasAnyDefaultGateway(void) {
    char col[11][32];
    char line[512];
    int  count = 0;
    int  flag = 0;

    FILE* fsRoute = fopen("/proc/net/route", "r");
    if ( fsRoute != NULL ) {
        while ( fgets(line, sizeof(line), fsRoute) ) {
         // read pass header line
        if ( count++ < 1 ) continue;
        sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
               col[0], col[1], col[2], col[3], col[4], col[5],
               col[6], col[7], col[8], col[9], col[10]);
        flag = strtol(col[3], (char**)NULL, 16);
        /*默认路由允许没有目的地址没有子网掩码，即不一定要RTF_GATEWAY
        if ((flag & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY)) {*/
        if ((flag & RTF_UP) == RTF_UP) {
            if (strncmp(col[1],"00000000",8) == 0)
            {
                fclose(fsRoute);
                return TRUE;        	
            }
        }
      }
      fclose(fsRoute);
   }

   return FALSE;
}

/*end of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/


/***************************************************************************
// Function Name: bcmSetAutoDefaultGateway().
// Description  : remove static default gateway in PSI,
//                get the default gateway from var/fyi/sys/gateway, and add
//                it to the route table.
// Parameters   : errMsg -- error message when add default gateway if any.
// Returns      : 1 - Exist. 0 - not Exist
****************************************************************************/
void bcmSetAutoDefaultGateway(char *errMsg) {
   char sysDefaultGateway[IFC_TINY_LEN];
   char cmd[IFC_LARGE_LEN];
   FILE *fs = NULL;
   IFC_DEF_GW_INFO defgw;

   // remove static default gateway
   if ( BcmDb_getDefaultGatewayInfo(&defgw) == DB_GET_OK ) {
      // remove static default gateway in PSI
      BcmDb_removeDefaultGatewayInfo();
      BcmPsi_flush();
      // create command to delete default gateway in route table
      sprintf(cmd, "route del default");
      if (strcmp(inet_ntoa(defgw.defaultGateway), "0.0.0.0") != 0) {
         strcat(cmd, " gw ");
         strcat(cmd, inet_ntoa(defgw.defaultGateway));
      }
      if (strcmp(defgw.ifName, "") != 0) {
         strcat(cmd, " dev ");
         strcat(cmd, defgw.ifName);
      }
      strcat(cmd, " 2> /var/gwerr");
      bcmSystem(cmd);
      fs = fopen("/var/gwerr", "r");
      // read gwerr, if there is err then
      // need to set error message
      if ( fs != NULL ) {
         if ( fgets(cmd, IFC_LARGE_LEN, fs) > 0 ) {
            // remove the last newline character
            cmd[strlen(cmd) - 1] = '\0';
            strcpy(errMsg, cmd);
            bcmSystemMute("cat /var/gwerr");
         } else
            errMsg[0] = '\0';
         fclose(fs);
         bcmSystemMute("rm /var/gwerr");
      }
   }

   /*start of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/
   int i;
   /*默认路由通过SH命令行可以加无限条，所以应该循环删除到没有默认路由再开始添加新默认路由
   for (i=0;bcmHasAnyDefaultGateway() && (i<3);i++)*/
   for (i=0;bcmHasAnyDefaultGateway();i++)
   {
       sprintf(cmd, "route del default");
       bcmSystem(cmd);
   }
   /*end of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/
  
   //get the default gateway from var/fyi/sys/gateway
   bcmGetDefaultGateway(sysDefaultGateway, cmd);
   /*start of ROUTE 3.1.3 porting by z37589 20060506 A36D01077
   if ( sysDefaultGateway[0] != '\0' &&
        bcmIsDefaultGatewayExisted(sysDefaultGateway, "") == FALSE ) { */
   if ( sysDefaultGateway[0] != '\0') {
   /*end of ROUTE 3.1.3 porting by z37589 20060506 A36D01077*/
      sprintf(cmd, "route add default gw %s 2> /var/gwerr", sysDefaultGateway);
      bcmSystem(cmd);
      fs = fopen("/var/gwerr", "r");
      // read gwerr, if there is err then
      // need to set the error message
      if ( fs != NULL ) {
         if ( fgets(cmd, IFC_LARGE_LEN, fs) > 0 ) {
            // remove the last newline character
            cmd[strlen(cmd) - 1] = '\0';
            strcpy(errMsg, cmd);
            bcmSystemMute("cat /var/gwerr");
         } else
            errMsg[0] = '\0';
         fclose(fs);
         bcmSystemMute("rm /var/gwerr");
      }
   }
   /* start Default Gateway syslog by f60014464 20060710 */
   syslog(LOG_NOTICE, "Automatic Assigned Default Gateway.\n");
   /* end Default Gateway syslog by f60014464 20060710 */
}

/* start of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */
void set_flags(char *flagstr, int flags)
{
	int i;

	*flagstr++ = 'U';

	for (i=0 ; (*flagstr = flagchars[i]) != 0 ; i++) {
		if (flags & flagvals[i]) {
			++flagstr;
		}
	}
}

/*************************************************
  Function:       bcmShowDefaultGatewayInfo()
  Description:    Display defaultGateway in cmdline.
  Calls:          无
  Called By:      IfcUiCli::showDefaultGatewayInfo
  Table Accessed: 无
  Table Updated:  无
  Input:          无
  Output:         无
  Return:         无
  Others:         无
*************************************************/
void bcmShowDefaultGatewayInfo(void) {
    char col[11][32];
    char line[512];
    int  count = 0;
    int  flag = 0;

    char devname[64], flags[16], sdest[16], sgw[16];
    unsigned long int d, g, m;
    int flgs, ref, use, metric, mtu, win, ir;
	struct sockaddr_in s_addr;
	struct in_addr mask;

    fprintf(stdout, "\nDefaultGateway of Kernel IP routing table\n"
			  "Destination Gateway         Genmask"
			  "         Flags Metric Ref Use MSS Window irtt Iface\n");
    
    FILE* fsRoute = fopen("/proc/net/route", "r");
    if ( fsRoute != NULL ) {
        while ( fgets(line, sizeof(line), fsRoute) ) {
         // read pass header line
        if ( count++ < 1 ) continue;
        sscanf(line, "%s %lx %lx %X %d %d %d %lx %d %d %d",
               devname, &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win, &ir);
        //flag = strtol(col[3], (char**)NULL, 16);

        //dst addr
        memset(&s_addr, 0, sizeof(struct sockaddr_in));
		s_addr.sin_family = AF_INET;
		s_addr.sin_addr.s_addr = d;
		//这里肯定是默认路由目的地址一定为0，只可能转换为default,不再需要做转换
		//strcpy(sdest, inet_ntoa(s_addr.sin_addr)); 
		
        /*默认路由允许没有目的地址没有子网掩码，即不一定要RTF_GATEWAY
        if ((flag & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY)) {*/
        if ((flgs & RTF_UP) == RTF_UP) {
            if (d == INADDR_ANY) //default route
            {
                set_flags(flags, (flgs & IPV4_MASK));
                if (flgs & RTF_REJECT) {
			        flags[0] = '!';
		        }

                //dst addr
                strcpy(sdest, "default");

                //gw
                if (g == INADDR_ANY)
                {
                    strcpy(sgw, "*");
                }
                else
                {
                    s_addr.sin_addr.s_addr = g;
                    strcpy(sgw, inet_ntoa(s_addr.sin_addr));
                }

                //dst mask
                mask.s_addr = m;

                fprintf(stdout, "%-12s%-16s%-16s%-6s%-6d %-3d %-3d %-4d%-6d %-4d %-16s\n", 
                    sdest, sgw, inet_ntoa(mask), flags, metric, ref, use, mtu, win, ir, devname);    	
            }
        }
      }
      fclose(fsRoute);
   }

   return FALSE;
}
/* end of PROTOCOL DefaultGateway CMD by zhouyi 00037589 2006年8月15日 */

/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
/***************************************************************************
// Function Name: bcmSetAtmConfigMode.
// Description  : Set Atm Module Traffic mode.
// Parameters   : none.
// Returns      : NULL
****************************************************************************/
void bcmSetAtmConfigMode(int trafficmode) {

   char *pchr = NULL;
   
   //printf("bcmSetTrafficMode trafficmode:%d \r\n", trafficmode);
/*Start modify : s48571 2006-10-26 for adsl porting*/     
#ifdef SUPPORT_VDSL
   /* start of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
   if (trafficmode == TM_ATM)
   {
       pchr = "atmctl configmode atm";
   }
   else if (trafficmode == TM_PTM_VLAN)
   {
       pchr = "atmctl configmode ptmvlan";
   }
   else if (trafficmode == TM_PTM_PRTCL)
   {
       pchr = "atmctl configmode ptmprotocol";
   }
   else if (trafficmode == TM_ATM_BR)
   {
       pchr = "atmctl configmode atmbr";
   }
   else if (trafficmode == TM_PTM_BR)
   {
       pchr = "atmctl configmode ptmbr";
   }
   
   printf("bcmSetAtmConfigMode Current mode is:%s\n",pchr);
   /* end of PROTOCOL KPN by zhouyi 00037589 2006年7月21日 */
   #else
   if (trafficmode == TM_ATM)
   {
       pchr = "atmctl configmode atm";
   }
      else if (trafficmode == TM_ATM_BR)
   {
       pchr = "atmctl configmode atmbr";
   }
   #endif
/*End modify : s48571 2006-10-26 for adsl porting*/  
   system(pchr);    
}

/***************************************************************************
// Function Name: bcmSetTrafficMode.
// Description  : Set Traffic mode.
// Parameters   : none.
// Returns      : NULL
****************************************************************************/
void bcmSetTrafficMode(int trafficmode) {

   char *pchr = NULL;
   
   //printf("bcmSetTrafficMode trafficmode:%d \r\n", trafficmode);

   pchr = ( trafficmode == 0 ) ? "atmctl traffic atm" :
                                 "atmctl traffic ptm" ; 
                                 printf("bcmSetTrafficMode Current mode is:%s\n",pchr);
   system(pchr);    
}

/***************************************************************************
// Function Name: bcmGetTrafficMode.
// Description  : Get Traffic mode.
// Parameters   : none.
// Returns      : 0 fail; other trafficmode.
****************************************************************************/
int bcmGetTrafficMode() {
   int trafficmode = 0;
   char line[SYS_CMD_LEN];
   char col[5][IFC_TINY_LEN];
   FILE* fs = NULL;

   bcmSystemMute("atmctl trafficmode > /var/trafficmode");

   fs = fopen("/var/trafficmode", "r");
   if (fs == NULL)
      return 0;

   while ( fgets(line, SYS_CMD_LEN, fs) > 0 ) {
      sscanf(line, "%s", col[0]);
      trafficmode = strtoul(col[0], (char **)NULL, 10);
   }

   fclose(fs);
   bcmSystemMute("rm /var/trafficmode");

   return trafficmode;
}
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

/***************************************************************************
// Function Name: bcmSetStaticDefaultGateway().
// Description  : remove the old, and add the new default gateway.
// Parameters   : gw, wanIf, and error message
// Returns      : none
****************************************************************************/
void bcmSetStaticDefaultGateway(char *gw, char *wanIf, char *errMsg) {
   int runCmd = FALSE;
   char cmd[IFC_LARGE_LEN];
   FILE* fs = NULL;
   IFC_DEF_GW_INFO defgw;

   // intialize gw if it is empty
   if ( gw[0] == '\0' )
      strcpy(gw, "0.0.0.0");

   // if errMsg is NULL then only save configuration
   // but not execute command to avoid error when
   // route destination address cannot be reached since
   // interface device does not created yet
   if ( errMsg != NULL &&
        BcmDb_getDefaultGatewayInfo(&defgw) == DB_GET_OK ) {
      // check if not the same
      if ( strcmp(defgw.ifName, wanIf) != 0 ||
           strcmp(inet_ntoa(defgw.defaultGateway), gw) != 0) {
         // del the previous default route saved in route table
         sprintf(cmd, "route del default");
         if ( strcmp(inet_ntoa(defgw.defaultGateway), "0.0.0.0") != 0 ) {
            strcat(cmd, " gw ");
            strcat(cmd, inet_ntoa(defgw.defaultGateway));
         }
         if ( strcmp(defgw.ifName, "") != 0 ) {
            strcat(cmd, " dev ");
            strcat(cmd, defgw.ifName);
         }
         strcat(cmd, " 2> /var/gwerr");
         bcmSystemMute(cmd);
         fs = fopen("/var/gwerr", "r");
         // read gwerr, if there is err then
         // need to set error message
         if ( fgets(cmd, IFC_LARGE_LEN, fs) > 0 ) {
            // remove the last newline character
            cmd[strlen(cmd) - 1] = '\0';
            strcpy(errMsg, cmd);
            bcmSystemMute("cat /var/gwerr");
         } else
            errMsg[0] = '\0';
         fclose(fs);
         bcmSystemMute("rm /var/gwerr");
      }
      else
         return; // same and do nothing
   }

   /*start of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/
   int i;
   /*默认路由通过SH命令行可以加无限条，所以应该循环删除到没有默认路由再开始添加新默认路由
   for (i=0;bcmHasAnyDefaultGateway() && (i<3);i++)*/
   for (i=0;bcmHasAnyDefaultGateway();i++)
   {
       sprintf(cmd, "route del default");
       bcmSystem(cmd);
   }
   /*end of ROUTE 3.1.2 porting by z37589 20060506 A36D00359*/
   
   // if errMsg is NULL then only save configuration
   // but not execute command to avoid error when
   // route destination address cannot be reached since
   // interface device does not created yet
   // if this gateway and wanIf is already in the route table,
   // do not issue route add default command.
   /*start of ROUTE 3.1.3 porting by z37589 20060506 A36D01077
   if ( errMsg != NULL &&
        bcmIsDefaultGatewayExisted(gw, wanIf) == FALSE ) { */
   if ( errMsg != NULL) {
   /*end of ROUTE 3.1.3 porting by z37589 20060506 A36D01077*/
      strcpy(cmd, "route add default");
      if ( strcmp(gw, "0.0.0.0") != 0 &&
           BcmDb_validateIpAddress(gw) == DB_OBJ_VALID_OK ) {
         strcat(cmd, " gw ");
         strcat(cmd, gw);
         runCmd = TRUE;
      }
      if (wanIf[0] != '\0') {
         strcat(cmd, " dev ");
         strcat(cmd, wanIf);
         runCmd = TRUE;
      }
      if ( runCmd == TRUE ) {
         strcat(cmd, " 2> /var/gwerr");
         bcmSystemMute(cmd);
         fs = fopen("/var/gwerr", "r");
         // read gwerr, if there is no err then
         // need to configure default gateway in PSI
         if ( fgets(cmd, IFC_LARGE_LEN, fs) <= 0 ) {
            // save the new default gateway info to psi
            errMsg[0] = '\0';
         } else {
            // remove the last newline character
            cmd[strlen(cmd) - 1] = '\0';
            strcpy(errMsg, cmd);
            bcmSystemMute("cat /var/gwerr");
         }
         // close gwerr file
         fclose(fs);
         bcmSystemMute("rm /var/gwerr");
      }
   }

   // save to PSI even when error occurs
   defgw.enblGwAutoAssign = 0;
   strcpy(defgw.ifName, wanIf);
   defgw.defaultGateway.s_addr = inet_addr(gw);
   BcmDb_setDefaultGatewayInfo(&defgw);

   /* start new DefaultGateway syslog by f60014464 20060710 */
   //new DefaultGateway syslog
   {
       char logMsg[1024];
       memset(logMsg, 0, sizeof(logMsg));
       sprintf(logMsg, "Default Gateway IP Address: [%s], Interface: [%s]\n", gw, wanIf);
       syslog(LOG_NOTICE, logMsg);
   }
   /* end new DefaultGateway syslog by f60014464 20060710 */
   
   BcmPsi_flush();
}

//**************************************************************************
// Function Name: getPppoeServiceName
// Description  : get pppoe service name of the specific wan interfaces.
// Parameters   : service -- pppoe service name. (output)
//                ifName -- interface name. (input)
// Returns      : none.
//**************************************************************************
void getPppoeServiceName(char *service, char *ifName) {
   char fileName[IFC_LARGE_LEN];
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;

   if (ifName[0] != 0) {
      sprintf(fileName, "/proc/var/fyi/wan/%s/servicename", ifName);
      fs = fopen(fileName, "r");
      if (fs != NULL) {
         if (fgets(str, SYS_CMD_LEN, fs) > 0)
            sscanf(str, "%s\n", service);
	 fclose(fs);
      }
   }
}



/***************************************************************************
// Function Name: setWanLinkStatus().
// Description  : get PPP status.
// Parameters   : up - wan link status
// Returns      : none
//                -1 - ERROR
****************************************************************************/
void setWanLinkStatus(int up) 
{
   FILE * fd = NULL;

   if ( up == 1 ) {
      if ( (fd = fopen("/var/run/wanup", "a+")) != NULL )
         fclose(fd);
   } else
      unlink("/var/run/wanup");
}

/***************************************************************************
// Function Name: disconnectPPP().
// Description  : if PPPD is alive, disconnet it.
// Returns      : none
****************************************************************************/
void disconnectPPP(void)
{
   int pid = 0;

   // If PPP is connected disconnect it first
   if ( (pid = bcmGetPid("pppd")) > 0 ) {
      setWanLinkStatus(0);
      // give PPP some time to disconnect. PPP client checks WAN links status
      // every 3 seconds 
	/*start of cut down the time of restore default 20101119 minpengwei the status is not used*/
      //sleep(6);
	/*end of cut down the time of restore default 20101119 minpengwei*/
   }
}


/***************************************************************************
// Function Name: bcmSocketIfPid().
// Description  : return the socket if pid (== 0 is lan (br0), !=0 is wan)
// Parameters   : none.
// Returns      : pid
****************************************************************************/
int bcmSocketIfPid(void)
{
   char ifName[IFC_SMALL_LEN];
   int pid = 0;
   char cmd[SYS_CMD_LEN];
   FILE *fs = NULL;

   // Check this first
   strncpy(ifName, getIfName(), IFC_SMALL_LEN - 1);
   if (ifName[0] == '\0')
      return pid;

   sprintf(cmd, "/proc/var/fyi/wan/%s/pid", ifName);
   if ((fs = fopen(cmd, "r")) != NULL)
   {
      fgets(cmd, SYS_CMD_LEN, fs);
      pid = atoi(cmd);
      fclose(fs);
   }

   return pid;
}

void killWanPid() {
    char cmd[SYS_CMD_LEN], app[SYS_CMD_LEN];
    char buffer[SYS_CMD_LEN];
    char pidStr[SYS_CMD_LEN];
    int pid = 0;
    int i = 0;
    FILE *fs;
    int curPid = getpid();
    char psList[] = "/var/kwpslist";
    char  *pcPos= NULL;
    char pInterface1[READ_BUF_SIZE];
    char pInterface2[READ_BUF_SIZE];
    int iVpi = 0;
    int iVci =0;
    int iConId = 0;
    printf("killWanPid\n");
    WAN_CON_ID wanId;
    bcmSystem("ps > /var/kwpslist");
    fs = fopen(psList, "r");
    while (fgets(buffer, SYS_CMD_LEN, fs) > 0) 
    {
        //表示为pppoe或者pppoa
        if ((pcPos = strstr(buffer, "pppd")) != NULL && strstr(buffer, "-U")  == NULL) // found command line with match app name
        {  
            sscanf(pcPos, "pppd -c %d.%d.%d", &iVpi, &iVci, &iConId);
            wanId.vpi = iVpi;
            wanId.vci = iVci;
            wanId.conId= iConId;
            BcmDb_getWanInterfaceName(&wanId, PROTO_PPPOA, pInterface1);  
            //表示为数据pvc
            if(bcmIsDataPvc(pInterface1) &&(!(iVpi==WDF_HSPA_DATA_VPI&&iVci == WDF_HSPA_DATA_VCI)))
            {
                sscanf(buffer, "%s\n", pidStr);
                pid = atoi(pidStr);
                if (pid != curPid )
                {
                    printf("kill process [pid: %d] [name: %s]...\n", pid, app);
                    sprintf(cmd, "kill -9 %d", pid);
                    bcmSystem(cmd);
                    /*start of 增加杀死进程，清除对应wan 信息处理函数 by s53329  at   20080618*/
                    sprintf(cmd,"echo 0 > /proc/var/fyi/wan/%s/daemonstatus", pInterface1);
                    bcmSystem(cmd);
                    sprintf(cmd,"echo 1 > /proc/var/fyi/wan/%s/status", pInterface1);
                    bcmSystem(cmd);
                    /*end of 增加杀死进程，清除对应wan 信息处理函数 by s53329  at   20080618*/
                }
            }
        }
        //还应该区分data 的dhcpc进程和voice的dhcpc进程的差异
        else if ((pcPos = strstr(buffer, "dhcpc")) != NULL) // found command line with match app name
        {  
            sscanf(pcPos, "dhcpc -i %s -I %s", pInterface1, pInterface2);
            if(bcmIsDataPvc(pInterface1))
            {
                sscanf(buffer, "%s\n", pidStr);
                pid = atoi(pidStr);
                if (pid != curPid )
                {
                    printf("kill process [pid: %d] [name: %s]...\n", pid, app);
                    sprintf(cmd, "kill -9 %d", pid);
                    bcmSystem(cmd);
                    /*start of 增加杀死进程，清除对应wan 信息处理函数 by s53329  at   20080618*/
                    sprintf(cmd,"echo 0 > /proc/var/fyi/wan/%s/daemonstatus", pInterface1);
                    bcmSystem(cmd);
                    sprintf(cmd,"echo 1 > /proc/var/fyi/wan/%s/status", pInterface1);
                    bcmSystem(cmd);
                    /*end of 增加杀死进程，清除对应wan 信息处理函数 by s53329  at   20080618*/
                }
            }
        }
    }
   fclose(fs);
}
int JudgeDefaultInterface(char *interface)
{
    char psList[] = "/var/iproute";
    char buffer[SYS_CMD_LEN];
    FILE *fs;
    /*start of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/
    int  iDefault = 0;
    /*end  of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/

    if (NULL == interface)
    {
        return 0;
    }
    bcmSystemMute("ip route > /var/iproute");
    fs = fopen(psList, "r");
    if (NULL == fs)
    {
        return 0;
    }
    while (fgets(buffer, SYS_CMD_LEN, fs) > 0) 
    {
        if (NULL != strstr(buffer, "default"))
        {
	     /*start of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/
	     iDefault = 1;
	     /*end of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/
            if (NULL != strstr(buffer, interface))
            {
                fclose(fs);
                return 1;
            }
        }
    }
    fclose(fs);
    /*start of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/
    if (iDefault == 0)
    {
	 return 1;
    }
    /*end  of 增加 因为局端原因导致语音data down，切换功能依然可以成功 by s53329  at  20080228*/
    else
    {
        return 0;
    }
}

int  BcmGetDefaultInterface(char *interface)
{
    char psList[] = "/var/iproute";
    char buffer[SYS_CMD_LEN];
    int iDefault = 0;
    FILE *fs;
    char ipaddr[16];
    char cInterface[32];
    if (NULL == interface)
    {
        return  0 ;
    }

    bcmSystemMute("ip route > /var/iproute");
    fs = fopen(psList, "r");
    if (NULL == fs)
    {
        return 0;
    }
    while (fgets(buffer, SYS_CMD_LEN, fs) > 0) 
    {
        if (NULL != strstr(buffer, "default"))
        {
            sscanf(buffer ,"default via  %s dev  %s", ipaddr, cInterface);
            strcpy(interface, cInterface);
            fclose(fs);
            return 1;
        }
    }
    fclose(fs);
    return 0;
}
/*start of 修改问题单AU8D00125 http 重定向问题by s53329  at   20080126*/
void BcmNtwk_fakeHttpForwarding(void) {
    char cmd[CLI_MAX_BUF_SZ], addr[CLI_MAX_BUF_SZ];

    memset(cmd , 0, CLI_MAX_BUF_SZ);
    memset(addr , 0, CLI_MAX_BUF_SZ);
    bcmGetIfAddr(IFC_BRIDGE_NAME, addr);
    sprintf(cmd, "iptables -t nat -A  PREROUTING_UTILITY -i br0  -p tcp --dport 80  -j DNAT --to  %s 2>/dev/null", addr);
    bcmSystem(cmd);
}	

void BcmNtwk_UnfakeHttpForwarding(void) {
    char cmd[CLI_MAX_BUF_SZ], addr[CLI_MAX_BUF_SZ];

    memset(cmd , 0, CLI_MAX_BUF_SZ);
    memset(addr , 0, CLI_MAX_BUF_SZ);
    bcmGetIfAddr(IFC_BRIDGE_NAME, addr);
    sprintf(cmd, "iptables -t nat -D  PREROUTING_UTILITY -i br0  -p tcp --dport 80  -j DNAT --to  %s 2>/dev/null", addr);
    bcmSystem(cmd);
}
/*end  of 修改问题单AU8D00125 http 重定向问题by s53329  at   20080126*/

/*add for dns proxy by a00169427 at 20111016*/
void BcmNtwk_addLanDnsProxy(void) {
    char cmd[CLI_MAX_BUF_SZ] = {0};
    char addr[CLI_MAX_BUF_SZ] = {0};
    
    bcmGetIfAddr(IFC_BRIDGE_NAME, addr);
    
    sprintf(cmd, "iptables -t nat -D  PREROUTING -i br0  -p udp --dport 53  -j DNAT --to  %s 2>/dev/null", addr);
    bcmSystem(cmd);
    //printf("\n\n[%s:%d]cmd=%s\n\n", __FILE__, __LINE__, cmd);
    sprintf(cmd, "iptables -t nat -A  PREROUTING -i br0  -p udp --dport 53  -j DNAT --to  %s 2>/dev/null", addr);
    bcmSystem(cmd);
    //printf("\n\n[%s:%d]cmd=%s\n\n", __FILE__, __LINE__, cmd);
}	

void BcmNtwk_deleteLanDnsProxy(void) {
    char cmd[CLI_MAX_BUF_SZ] = {0};
    char addr[CLI_MAX_BUF_SZ] = {0};

    bcmGetIfAddr(IFC_BRIDGE_NAME, addr);
    sprintf(cmd, "iptables -t nat -D  PREROUTING -i br0  -p udp --dport 53  -j DNAT --to  %s 2>/dev/null", addr);
    bcmSystem(cmd);
}
/*end add for dns proxy by a00169427 at 20111016*/
//不直接杀进程，而只通知ppp断开连接
void killHspaPid() {
    char cmd[SYS_CMD_LEN], app[SYS_CMD_LEN];
    char buffer[SYS_CMD_LEN];
    char pidStr[SYS_CMD_LEN];
    int pid = 0;
    int i = 0;
    FILE *fs = NULL;
    /* BEGIN: Deleted by y67514, 2008/4/17  Dial on demand  */
    //int curPid = getpid();
    /* END: Deleted by y67514, 2008/4/17 */
    char  *pcPos= NULL;
    char psList[] = "/var/khpslist";
    int iVpi = 0;
    int iVci =0;
    int iConId = 0;
    printf("kill  hspa  pid  \n");
    bcmSystem("ps > /var/khpslist");
    fs = fopen(psList, "r");
    if ( NULL == fs )
    {
        fs = fopen(psList, "r");
        if ( NULL == fs )
        {
            printf("Can't open file khpslist!\n");
            return;
        }
    }

    while (fgets(buffer, SYS_CMD_LEN, fs) > 0) 
    {
        if ((pcPos = strstr(buffer, "pppd")) != NULL) // found command line with match app name
        {  
             sscanf(pcPos, "pppd -c %d.%d.%d", &iVpi, &iVci, &iConId);
             if (iVpi == WDF_HSPA_DATA_VPI && iVci == WDF_HSPA_DATA_VCI)
             {
                 sscanf(buffer, "%s\n", pidStr);
                 pid = atoi(pidStr);
                 /* BEGIN: Modified by y67514, 2008/4/17  Dial on demand  */
                 /*以前hspa的切换是直接杀进程，现在只是让其down掉
                 if (pid != curPid )
                 {
                     printf("kill process [pid: %d] [name: %s]...\n", pid, app);
                     sprintf(cmd, "kill -9 %d", pid);
                     bcmSystem(cmd);
                 }
               */
                    bcmSystem("echo 0 >/var/HspaDialFlag");
                    printf("kill SIGHUP [pid: %d] [name: %s]...\n", pid, app);
                    sprintf(cmd, "kill -SIGHUP %d", pid);
                    bcmSystem(cmd);
                    
                 /* END:   Modified by y67514, 2008/4/17 */
            }
        }
    }

   fclose(fs);
}

/* BEGIN: Added by y67514, 2008/4/16  HSPA Stat */

/*****************************************************************************
 函 数 名  : getHspaStatus
 功能描述  : 获取HSPA的状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int        0-HSPA没拨；1-开始拨；2-已拨上
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年4月17日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int getHspaStatus(void)
{
    FILE * fd = NULL;
    char path[]="/var/HspaDialFlag";
    char data[8];
    int mode = 0;

    if (fd=fopen(path, "r")) 
    {
        fgets(data, 8, fd);
        fclose(fd);
        //printf("data=%x\n", data[0]);
        mode = atoi(data);
        return mode;
    }
    else
    {
        return mode;
    }

}
/* END:   Added by y67514, 2008/4/16 */

/***************************************************************************
// Function Name: bcmKillAllApps().
// Description  : kill all available applications.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmKillAllApps(void) {
   // NOTE: Add the app name before NULL. Order is important
   char *apps[]=
   {
#if 0
#ifdef SUPPORT_TR69C
      "tr69c",
#endif
#endif
      "bftpd",
      "telnetd",   
      "sshd",
      //"snmp",
      "upnp",
      "sntp",
      "ddnsd",
      "reaim",
#if 0      
      "klogd",
      "syslogd",
#endif
      "tftpd",
      "ripd",
      "zebra ",
    #if 0  
      "pppd",
    #endif
      "dnsprobe",
      "dhcpc",
      "igmp",
      //add by z67625
      "motion",     
      "smbd",
      "tr064",
      "twonkymedia",
      "twonkymediaserver",
      "voice",
      NULL,
   };
   char cmd[SYS_CMD_LEN], app[SYS_CMD_LEN], buf[SYS_CMD_LEN];
   char pidStr[SYS_CMD_LEN];
   int pid = 0;
   int i = 0;
   FILE *fs;
   int socketPid = 0;
   int curPid = getpid();
   int httpdPid =0;
   int inWeb = FALSE;

   fs = fopen("/var/run/httpd_pid", "r");
   if (fs != NULL)
   {
      fgets(cmd, SYS_CMD_LEN, fs);
      httpdPid = atoi(cmd);
      fclose(fs);
   }
   else
   {
      printf("Error: httpd pid not found ?\n");
      return;
   }

   bcmSystemMute("ps > /var/kapslist");
   fs = fopen("/var/kapslist", "r");
   if (fs == NULL)
      return;
   bcmSystem("cat /var/kapslist");

   socketPid = bcmSocketIfPid();

   if (socketPid != 0) 
   {
      // add the default route with the ifc the socket is sitting on
      if (strstr(getIfName(), "nas") == NULL &&       // only do it if it is not MER and not ipoa (eth?)
         strstr(getIfName(), "eth") == NULL)
      {
         sprintf(cmd, "route add default dev %s", getIfName());
         bcmSystem(cmd);
      }
      rewind(fs);
   }
      
   if (curPid == httpdPid)
      inWeb = TRUE;

   bcmSystem("sysinfo");

   //printf("curPid=%d, socketPid=%d, htpdPid=%d, inWeb=%d\n", curPid, socketPid, httpdPid, inWeb);
   // get app and read thru pslist according to the order of apps
   i = 0;
   do 
   {
      strcpy(app, apps[i]);
      while (fgets(buf, SYS_CMD_LEN, fs) > 0)  
      {
         if (strstr(buf, app) != NULL) // found command line with match app name
         {  
            // find pid string
            sscanf(buf, "%s\n", pidStr);
            // if not in httpd, must be in telnetd or sshd, just skip the line.
            if (!inWeb && ((strstr(buf, "telnetd")) || (strstr(buf, "sshd"))))
               break;
            pid = atoi(pidStr);
            if (pid != curPid && pid != socketPid)
            {
               printf("kill %s process...\n", app);
               sprintf(cmd, "kill -%d %d", 9, pid);
               bcmSystem(cmd);
            }
         }
      } 
      rewind(fs);    // start pslist over again
   } while (apps[++i] != NULL);

   fclose(fs);
   bcmSystemMute("rm /var/kapslist");
   /*start delete of kill app for A36D02667 by l39225 2006-8-21*/
  // bcmRemoveModules(socketPid);
   /*end delete of kill app for A36D02667 by l39225 2006-8-21*/
}

// function to support encryption password for login
static int i64c(int i) {
    if (i <= 0)
        return ('.');
    if (i == 1)
        return ('/');
    if (i >= 2 && i < 12)
        return ('0' - 2 + i);
    if (i >= 12 && i < 38)
        return ('A' - 12 + i);
    if (i >= 38 && i < 63)
        return ('a' - 38 + i);
    return ('z');
}

// function to support encryption password for login
static char *crypt_make_salt(void) {
    time_t now;
    static unsigned long x;
    static char result[3];

    time(&now);
    x += now + getpid() + clock();
    result[0] = i64c(((x >> 18) ^ (x >> 6)) & 077);
    result[1] = i64c(((x >> 12) ^ x) & 077);
    result[2] = '\0';
    return result;
}

// function to support encryption password for login
static char *pw_encrypt(const char *clear, const char *salt) {
    static char cipher[128];
    char *cp;

#ifdef CONFIG_FEATURE_SHA1_PASSWORDS
    if (strncmp(salt, "$2$", 3) == 0) {
        return sha1_crypt(clear);
    }
#endif
    cp = (char *) crypt(clear, salt);
    /* if crypt (a nonstandard crypt) returns a string too large,
       truncate it so we don't overrun buffers and hope there is
       enough security in what's left */
    if (strlen(cp) > sizeof(cipher)-1) {
        cp[sizeof(cipher)-1] = 0;
    }
    strcpy(cipher, cp);
    return cipher;
}

#ifdef   SUPPORT_RADIUS
/*start  of 增加Radius 认证功能by s53329  at  20060714 */

/***************************************************************************
// Function Name: bcmAddLoginCfg().
// Description  : create password file for login using 'admin' or 'support'.
// Parameters   : cp_admin - clear password of 'admin'.
//                cp_support - clear password of 'support'.
//                cp_user - clear password of 'user'.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmAddLoginCfg(char *cp_admin, char *cp_pass) {
   struct passwd pw;
   FILE *fsPwd = NULL, *fsGrp = NULL;
   int len = CFG_FILE_LEN;
   fsPwd = fopen("/etc/passwd", "r+");
   
   if ( fsPwd != NULL ) {
      // In future, we may change uid of 'admin' and 'support'
      // uclibc may have a bug on putpwent in terms of uid,gid setup
      fseek(fsPwd, len ,SEEK_SET);
      pw.pw_name = strdup(cp_admin);
      pw.pw_passwd = pw_encrypt(cp_pass, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Administrator";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);

      free (pw.pw_name);
      fclose(fsPwd);

      fsGrp = fopen("/etc/group", "w");
      if ( fsGrp != NULL ) {
         fprintf(fsGrp, "root::0:root,admin,support,user\n");
         fclose(fsGrp);
         return FILE_OPEN_OK;
      }
   }

   return FILE_OPEN_ERR;
}
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
#endif


#ifdef SUPPORT_ONE_USER
/***************************************************************************
// Function Name: bcmCreateLoginCfg().
// Description  : create password file for login.
// Parameters   : username
//                password
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmCreateLoginCfg(char *username, char *password) {
   struct passwd pw;
   FILE *fsPwd = NULL, *fsGrp = NULL;
   int len = 0;
   fsPwd = fopen("/etc/passwd", "w");

   if ( fsPwd != NULL ) {
      // In future, we may change uid of 'admin' and 'support'
      // uclibc may have a bug on putpwent in terms of uid,gid setup
      pw.pw_name = username;
      pw.pw_passwd = pw_encrypt(password, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Administrator";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);

      pw.pw_name = "nobody";
      pw.pw_passwd = pw_encrypt(password, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "nobody for ftp";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);
       fseek(fsPwd, 0 ,SEEK_END);
      len = ftell(fsPwd);
      fclose(fsPwd);

      fsGrp = fopen("/etc/group", "w");
      if ( fsGrp != NULL ) {
         fprintf(fsGrp, "root::0:root,%s\n", username);
         fclose(fsGrp);
         return FILE_OPEN_OK;
      }
   }

   return FILE_OPEN_ERR;
}
#else
/***************************************************************************
// Function Name: bcmCreateLoginCfg().
// Description  : create password file for login using 'admin' or 'support'.
// Parameters   : cp_admin - clear password of 'admin'.
//                cp_support - clear password of 'support'.
//                cp_user - clear password of 'user'.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmCreateLoginCfg(char *cp_admin, char *cp_support, char *cp_user) {
   struct passwd pw;
   FILE *fsPwd = NULL, *fsGrp = NULL;
   int len = 0;
   fsPwd = fopen("/etc/passwd", "w");

   if ( fsPwd != NULL ) {
      // In future, we may change uid of 'admin' and 'support'
      // uclibc may have a bug on putpwent in terms of uid,gid setup
      pw.pw_name = "admin";
      pw.pw_passwd = pw_encrypt(cp_admin, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Administrator";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);

      pw.pw_name = "support";
      pw.pw_passwd = pw_encrypt(cp_support, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Technical Support";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);

      pw.pw_name = "user";
      pw.pw_passwd = pw_encrypt(cp_user, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Normal User";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);

      pw.pw_name = "nobody";
      pw.pw_passwd = pw_encrypt(cp_admin, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "nobody for ftp";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);
       fseek(fsPwd, 0 ,SEEK_END);
      len = ftell(fsPwd);
      fclose(fsPwd);

      fsGrp = fopen("/etc/group", "w");
      if ( fsGrp != NULL ) {
         fprintf(fsGrp, "root::0:root,admin,support,user\n");
         fclose(fsGrp);
         return FILE_OPEN_OK;
      }
   }

   return FILE_OPEN_ERR;
}
#endif
int AtpCreateLoginforFtp(char *username, char *password, char* path)
{
   struct passwd pw;
   FILE *fsPwd = NULL, *fsGrp = NULL;
   int len = 0;
   int filepos = 0;
   char entirePath[256]="/var/mnt";
   char line[SYS_CMD_LEN] = "";

   strcat(entirePath, path);
   
   fsPwd = fopen("/etc/passwd", "r+");

   if ( fsPwd != NULL ) {
   	
//      printf("open file successfully!\n");
	
  	
      pw.pw_name = username;      
      pw.pw_passwd = pw_encrypt(password, crypt_make_salt());
      
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "FTP user";  
      pw.pw_dir = entirePath;
     
      pw.pw_shell = "/bin/sh";

      while (fgets(line, SYS_CMD_LEN, fsPwd) > 0)  
     {
   	 filepos = ftell(fsPwd);
         if (strstr(line, "FTP user") != NULL) // found command line with match app name
         {  
            fseek(fsPwd, len, SEEK_SET);
            break;
         }
         len = filepos;
     }
      
      putpwent(&pw, fsPwd);
      len = ftell(fsPwd);
      fgets(line, SYS_CMD_LEN, fsPwd);
      filepos = ftell(fsPwd);
      fseek(fsPwd, len-1 ,SEEK_SET);
      
      memset(line, '\0', filepos);
      fwrite(line, 1, filepos - len, fsPwd);
      fclose(fsPwd);

      fsGrp = fopen("/etc/group", "w");
      if ( fsGrp != NULL ) {
         fprintf(fsGrp, "root::0:root,admin,support,user\n");
         fclose(fsGrp);
         return FILE_OPEN_OK;
      }
      
   }

   return FILE_OPEN_ERR;
}

int bcmAddAccount(char *account, char *passwd) {
   struct passwd pw;
   FILE *fsPwd = NULL, *fsGrp = NULL;
   char line[IFC_GIANT_LEN];
   char *pTail = NULL;

   if(!strcmp(account, "admin") || !strcmp(account, "user") ||
      !strcmp(account, "support") || !strcmp(account, "nobody"))
      return;

   fsPwd = fopen("/etc/passwd", "a");

   if ( fsPwd != NULL ) {
      // In future, we may change uid of 'admin' and 'support'
      // uclibc may have a bug on putpwent in terms of uid,gid setup
      pw.pw_name = account;
      pw.pw_passwd = pw_encrypt(passwd, crypt_make_salt());
      pw.pw_uid = 0;
      pw.pw_gid = 0;
      pw.pw_gecos = "Normal User";
      pw.pw_dir = "/";
      pw.pw_shell = "/bin/sh";
      putpwent(&pw, fsPwd);
      fclose(fsPwd);

      fsGrp = fopen("/etc/group", "r");
      if(fsGrp != NULL) {
         fgets(line, IFC_LARGE_LEN, fsGrp);
         fclose(fsGrp);
         fsGrp = fopen("/etc/group", "w");
         if ( fsGrp != NULL ) {
            pTail = line + strlen(line) - 1;
            strcpy(pTail, ",");
            strcat(pTail, account);
            strcat(pTail, "\n");
            fprintf(fsGrp, line);
            fclose(fsGrp);
            return FILE_OPEN_OK;
         }
      }
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmSetIpExtension().
// Description  : store PPP IP extension info to file.
// Parameters   : ipExt - 1:enable, 0:disable.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSetIpExtension(int ipExt) {
   FILE* fs = fopen("/var/ipextension", "w");

   if ( fs != NULL ) {
      fprintf(fs, "%d\n", ipExt);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmGetIpExtension().
// Description  : retrieve PPP IP extension info from file.
// Parameters   : str -- buffer.
//                len -- size of buffer.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmGetIpExtension(char *str, int len) {
   FILE* fs = fopen("/var/ipextension", "r");

   if ( fs != NULL ) {
      fgets(str, len, fs);
      fclose(fs);
      return FILE_OPEN_OK;
   }

   return FILE_OPEN_ERR;
}

/***************************************************************************
// Function Name: bcmGetIfcIndexByName().
// Description  : get interface index by its name.
// Parameters   : ifcIdx -- interface index.
//                ifcName -- interface name.
// Returns      : interface index
****************************************************************************/
int bcmGetIfcIndexByName(char *ifcName) {
   struct ifreq ifr;
   int s = 0;

   if ( ifcName == NULL ) return -1;

   if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) return -1;

   strcpy(ifr.ifr_name, ifcName);
   if ( ioctl(s, SIOCGIFINDEX, &ifr) < 0 ) {
      close(s);
      return 0;
   }

   close(s);

   return ifr.ifr_ifindex;
}

/***************************************************************************
// Function Name: bcmIsValidIfcName.
// Description  : validate the interface name.
// Parameters   : ifcName -- interface name that need to validate.
// Returns      : TRUE or FALSE
****************************************************************************/
int bcmIsValidIfcName(char *ifcName) {
   int ret = FALSE;

   if (bcmGetIfcIndexByName(ifcName))
      ret = TRUE;
   return ret;
}

/***************************************************************************
// Function Name: bcmGetIfcNameById.
// Description  : get interface name by its id (defined in ifcdefs.h).
// Parameters   : ifcId -- interface Id.
//                ifcName -- interface name.
// Returns      : interface name or NULL
****************************************************************************/
char *bcmGetIfcNameById(int ifcId, char *ifcName) {
   if ( ifcName == NULL ) return NULL;

   ifcName[0] = '\0';

   if ( ifcId >= IFC_ENET_ID && ifcId < IFC_USB_ID )
      sprintf(ifcName, "eth%d", ifcId - IFC_ENET_ID);
   else if ( ifcId >= IFC_USB_ID && ifcId < IFC_HPNA_ID )
      sprintf(ifcName, "usb%d", ifcId - IFC_USB_ID);
   else if ( ifcId >= IFC_HPNA_ID && ifcId < IFC_WIRELESS_ID )
      sprintf(ifcName, "il%d", ifcId - IFC_HPNA_ID);
    /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
   else if ( ifcId >= IFC_WIRELESS_ID && ifcId < IFC_WIRELESS_ID + IFC_LAN_MAX*IFC_WLAN_MAX ) {
      int num = (ifcId - IFC_WIRELESS_ID)%IFC_WLAN_MAX;
/* BEGIN: Modified by c106292, 2008/8/26   PN:1*/
#ifdef SUPPORT_ATHEROSWLAN
	sprintf(ifcName, "ath%d",num); 
#else
      if (num == 0) { // multiple ssid support
         sprintf(ifcName, "wl%d", (ifcId - IFC_WIRELESS_ID)/IFC_LAN_MAX);
      }
      else {
         sprintf(ifcName, "wl%d.%d",(ifcId - IFC_WIRELESS_ID)/IFC_LAN_MAX, num); 
      }
#endif
/* END:   Modified by c106292, 2008/8/26 */
   }
   else if (ifcId >= IFC_ENET0_VNET_ID && ifcId < IFC_ENET0_VNET_ID + IFC_LAN_MAX + 2)
       sprintf(ifcName, "eth0.%d", ifcId - IFC_ENET0_VNET_ID);
   else if (ifcId >= IFC_ENET1_VNET_ID && ifcId < IFC_ENET1_VNET_ID + IFC_LAN_MAX + 2)
       sprintf(ifcName, "eth1.%d", ifcId - IFC_ENET1_VNET_ID);

   return ifcName;
}

/***************************************************************************
// Function Name: bcmGetIfcNameByIpAddr.
// Description  : get interface name by its IP address.
// Parameters   : ifcIpAddr -- interface IP address.
//                ifcName -- interface name.
// Returns      : interface name or NULL
****************************************************************************/
char *bcmGetIfcNameByIpAddr(unsigned long ipAddr, char *ifcName) {
    char str[SYS_CMD_LEN] = "ifconfig -a > /var/ifcs";
    FILE *fs;
    char *ret = NULL;

    bcmSystemMute(str);
    if( (fs = fopen("/var/ifcs", "r")) != NULL ) {
        while( fgets(str, sizeof(str), fs) != NULL ) {
            if( str[0] >= 'A' && str[0] <= 'z' ) {
                int i;
                char *p;
                char name[16];

                // Copy interface name (br0, eth0, pppoe_0_35, etc.) to local
                // variable.
                for(i = 0, p = str; i<sizeof(name)-1 && *p && *p!=' '; i++, p++)
                    name[i] = *p;
                name[i] = '\0';

                // The next line will have the IP address if one is assigned
                // to the current interface.
                if( fgets(str, sizeof(str), fs) != NULL ) {
                    if( (p = strstr(str, "inet addr:")) != NULL ) {
                        unsigned long addr;

                        p += strlen("inet addr:");
                        addr = inet_addr(p);

                        // if the IP address of the current interfaces matches
                        // the supplied IP address, then the interface is found.
                        if( addr == ipAddr ) {
                            strcpy( ifcName, name );
                            ret = ifcName;
                            break;
                        }
                    }
                }
            }
        }
        fclose(fs);
        unlink("/var/ifcs");
    }

    return( ret );
}

/***************************************************************************
// Function Name: bcmGetIpAddrByIfcName.
// Description  : get IP address  by its interface name .
// Parameters   : ifcIpAddr -- interface IP address.
//                ifcName -- interface name.
// Returns      : interface name or NULL
****************************************************************************/
char *bcmGetIpAddrByIfcName(char* ipAddr, char *ifcName) {
    char str[SYS_CMD_LEN] = {0};
    FILE *fs = NULL;
    char *ret = NULL;
    char *pcLocalIfName = NULL;
    
    bcmSystemMute("ifconfig -a > /var/ifcs");
    fs = fopen("/var/ifcs", "r");
    if( fs != NULL ) {
        while( fgets(str, sizeof(str), fs) != NULL ) {
            if( str[0] >= 'A' && str[0] <= 'z' ) {
                int i;
                char *p;
                char name[16];
                // Copy interface name (br0, eth0, pppoe_0_35, etc.) to local
                // variable.
                for(i = 0, p = str; i<sizeof(name)-1 && *p && *p!=' '; i++, p++)
                    name[i] = *p;
                name[i] = '\0';
                if (0 == strncmp(ifcName ,name, sizeof(name)))
                {    
                    // The next line will have the IP address if one is assigned
                    // to the current interface.
                    if( fgets(str, sizeof(str), fs) != NULL ) {
                        if( (p = strstr(str, "inet addr:")) != NULL ) {
                            p += strlen("inet addr:");
                            // if the IP address of the current interfaces matches
                            // the supplied IP address, then the interface is found.
                            if( NULL != p ) {
                                strcpy( ipAddr, p );
                                pcLocalIfName = strstr(ipAddr, " ");
                                *pcLocalIfName = '\0';
                                ret = ipAddr;
                                break;
                            }
                        }
                    }
                }
            }
        }
        fclose(fs);
        unlink("/var/ifcs");
    }

    return( ret );
}
/***************************************************************************
// Function Name: bcmSetConnTrackMax.
// Description  : tune the connection track table size.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmSetConnTrackMax(void) {
   int conntrack_max = 1000;
   char cmd[SYS_CMD_LEN];
   FILE* fs = fopen("/proc/sys/net/ipv4/ip_conntrack_max", "r");

   // init ip_conntrack_max
   if ( fs == NULL ) // ip_conntrack module is not loaded.
      return;	 
   else {
      fgets(cmd, SYS_CMD_LEN, fs);
      conntrack_max = atoi(cmd);
      fclose(fs);
   }
   // Code in ip_conntrack_init function of ip_conntrack_core.c in kernel
   // has been modified to initialize ip_conntrack_max to be always 0
   // when ip_conntrack module is inserted (before NAT or Firewall is enabled).
   // This function setting real conntrack_max will be called after NAT or Firewall is enabled.
   if ( conntrack_max == 0 )
	conntrack_max = 1000;
   else {   
      if ( conntrack_max < 1000 ) {
         conntrack_max *= 2;
         if ( conntrack_max > 1000 )
            conntrack_max = 1000;
      }
   }
   sprintf(cmd, "echo \"%d\" > /proc/sys/net/ipv4/ip_conntrack_max", conntrack_max);
   bcmSystem(cmd);
}

/***************************************************************************
// Function Name: bcmResetConnTrackTable.
// Description  : reset the connection track table.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmResetConnTrackTable(void) {
   if ( bcmIsModuleInserted("ip_conntrack") == TRUE )
      bcmSystem("echo 9 > proc/sys/net/ipv4/netfilter/ip_conntrack_dns");
}

/***************************************************************************
// Function Name: bcmHandleConnTrack.
// Description  : handle the connection track table.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmHandleConnTrack(void) {
   bcmResetConnTrackTable();
   bcmSetConnTrackMax();
}

/***************************************************************************
// Function Name: bcmInsertModules.
// Description  : insert all modules under the given path.
// Parameters   : path -- the given path.
// Returns      : none.
****************************************************************************/
void bcmInsertModules(char *path) {
   struct dirent *entry;
   DIR *dir;
   char *cp = NULL;

   if ( path == NULL ) return;

   dir = opendir(path);
   if ( dir == NULL ) return;

   while ( (entry = readdir(dir)) != NULL )
      if ( (cp = strstr(entry->d_name, ".ko")) != NULL ) {
         *cp = '\0';
         bcmInsertModule(entry->d_name);
      }
   closedir(dir);
}

/***************************************************************************
// Function Name: bcmInsertModule.
// Description  : insert module with the given name.
// Parameters   : modName -- the given module name.
// Returns      : none.
****************************************************************************/
void bcmInsertModule(char *modName) {
   char cmd[SYS_CMD_LEN], modulepath[SYS_CMD_LEN];
   struct utsname kernel;

   if (uname(&kernel) == -1)
      return;

   sprintf(modulepath, "/lib/modules/%s/kernel/net/ipv4/netfilter", kernel.release);

   if ( bcmIsModuleInserted(modName) == FALSE ) {
      sprintf(cmd, "insmod %s/%s.ko", modulepath,modName);
      bcmSystemMute(cmd);
   }
}
 /*start of 增加VDF 数据/语音自动切换功能 by s53329 at  20071230*/
/***************************************************************************
// Function Name: bcmRemoveDnsForwardRule().
// Description  : remove Dns  table rules.
// Parameters   : device -- interface name.
//                table -- IP table name.
//                chain -- IP table chain.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveDnsForwardRule(char *table, char *chain, char  *cDnsPrim, char *cDnsSe) {
   char col[IP_TBL_COL_MAX][IFC_SMALL_LEN];
   char comment[IFC_LARGE_LEN], line[IFC_GIANT_LEN] ,ipaddr[IFC_GIANT_LEN];
   char target[IFC_LARGE_LEN];
   int ret = FALSE, count = 0;
   FILE* fs = NULL;

   if ((access("/bin/iptables",F_OK)) != 0)
      return FALSE;

   // execute iptables command to create iptable file
   sprintf(line, "iptables -t %s -L %s -v --line-numbers > /var/iptable_dnsfwd",
           table, chain);
   bcmSystemNoHang(line);

   fs = fopen("/var/iptable_dnsfwd", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s",
                col[IP_TBL_COL_NUM], col[IP_TBL_COL_PKTS], col[IP_TBL_COL_BYTES],
                col[IP_TBL_COL_TARGET], col[IP_TBL_COL_PROT], col[IP_TBL_COL_OPT],
                col[IP_TBL_COL_IN], col[IP_TBL_COL_OUT], col[IP_TBL_COL_SRC],
                col[IP_TBL_COL_DST], comment, target, ipaddr);
         // if chain rule for the given device is found
         //仅当对应的dns 的wan 接口down 掉，才删除iptables
        if (strstr(target, "dpt:domain") != NULL && ((strstr(ipaddr, cDnsPrim) != NULL&&cDnsPrim[0] != '\0' ) ||(strstr(ipaddr, cDnsSe) != NULL &&cDnsSe[0] != '\0')))
        {
            sprintf(line, "iptables -t %s -D %s %s 2>/dev/null", table, chain, col[IP_TBL_COL_NUM]);
            bcmSystemNoHang(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);
   }
   // codes to remove iptable file is moved to bcmRemoveAllIpTableRules() function
    /*start of 清除连接跟踪dns 缓存 by s53329  at  20080114*/
    sprintf(line, "echo 1 > /proc/sys/net/ipv4/netfilter/ip_conntrack_dns");
    bcmSystem(line);
    /*end  of 清除连接跟踪dns 缓存 by s53329  at  20080114*/
   return ret;
}

 /***************************************************************************
// Function Name: bcmGetDnsByInterface().
// Description  : get dns addr
// Parameters   : device -- interface name.
//                table -- IP table name.
//                chain -- IP table chain.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmGetDnsByInterface(char *pInterface,  char  *cDnsPrim, char *cDnsSe) 
{
    FILE   *fs = NULL;
    char  buf[IFC_LARGE_LEN];
    char  str[IFC_DOMAIN_LEN];
    char  name[IFC_TINY_LEN], cDns[IFC_TINY_LEN];
    int  i= 0;
    
    if (NULL == pInterface || NULL == cDnsPrim ||NULL ==  cDnsSe)
    {
        return  0;
    }
    sprintf(buf, "/var/fyi/sys/%s/dns", pInterface);
    fs = fopen(buf, "r");
    if ( fs != NULL ) {
         while( fgets(str, IFC_DOMAIN_LEN, fs) != NULL ) {
            sscanf(str, "%s %s", name, cDns);
            if (i == 0)
                strcpy(cDnsPrim, cDns);
            else
                strcpy(cDnsSe, cDns);
            i++;
        }
        fclose(fs);
        if (0 != i)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
 }

void  bcmRemoveDnsInfo(char *interface)
{
    char cmd[256];
    if (NULL == interface)
    {
        return ;
    }
    memset(cmd , 0, 256);
    sprintf(cmd, "echo 0 > /var/fyi/sys/%s/dns", interface);
    bcmSystem(cmd);
    return;
}
 /*end  of 增加VDF 数据/语音自动切换功能 by s53329 at  20071230*/ 

/***************************************************************************
// Function Name: bcmRemoveIpTableRule().
// Description  : remove IP table rules.
// Parameters   : device -- interface name.
//                table -- IP table name.
//                chain -- IP table chain.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveIpTableRule(char *device, char *table, char *chain) {
   char col[IP_TBL_COL_MAX][IFC_SMALL_LEN];
   char comment[IFC_LARGE_LEN], line[IFC_GIANT_LEN];
   char fileName[IFC_MEDIUM_LEN];
   int ret = FALSE, count = 0;
   FILE* fs = NULL;

   if ((access("/bin/iptables",F_OK)) != 0)
      return FALSE;
   if ( NULL == device )
   {
       return FALSE;
   }

    memset(col,0,sizeof(col));
    memset(comment,0,sizeof(comment));
    memset(line,0,sizeof(line));
    memset(fileName,0,sizeof(fileName));
    sprintf(fileName,"/var/iptable_%s",device);
   // execute iptables command to create iptable file
   sprintf(line, "iptables -t %s -L %s -v --line-numbers > %s",
           table, chain,fileName);
   bcmSystemNoHang(line);

   fs = fopen(fileName, "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 2 header lines
         if ( count++ < 2 ) continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                col[IP_TBL_COL_NUM], col[IP_TBL_COL_PKTS], col[IP_TBL_COL_BYTES],
                col[IP_TBL_COL_TARGET], col[IP_TBL_COL_PROT], col[IP_TBL_COL_OPT],
                col[IP_TBL_COL_IN], col[IP_TBL_COL_OUT], col[IP_TBL_COL_SRC],
                col[IP_TBL_COL_DST], comment);
         // if chain rule for the given device is found
         if ( strcmp(col[IP_TBL_COL_IN], device) == 0 ||
              strcmp(col[IP_TBL_COL_OUT], device) == 0 ) {
            sprintf(line, "iptables -t %s -D %s %s 2>/dev/null",
                    table, chain, col[IP_TBL_COL_NUM]);
            bcmSystemNoHang(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);
      unlink(fileName);
   }

   // codes to remove iptable file is moved to bcmRemoveAllIpTableRules() function

   return ret;
}

/***************************************************************************
// Function Name: bcmRemoveAllIpTableRules().
// Description  : remove all IP table rules attach with the given device.
// Parameters   : device -- interface name.
// Returns      : none
****************************************************************************/
void bcmRemoveAllIpTableRules(char *device) {
   FILE* fs = NULL;

   if ( bcmIsModuleInserted("iptable_filter") == TRUE ) {
      while ( bcmRemoveIpTableRule(device, "filter", "INPUT") == TRUE )
         ;

      while ( bcmRemoveIpTableRule(device, "filter", "FORWARD") == TRUE )
         ;

      while ( bcmRemoveIpTableRule(device, "filter", "OUTPUT") == TRUE )
         ;
      /* Start of iptables porting by c47036 20060519 */
      while ( bcmRemoveIpTableRule(device, "filter", "FORWARD_IN") == TRUE )
         ;

      while ( bcmRemoveIpTableRule(device, "filter", "FORWARD_UTILITY") == TRUE )
         ;
      /* End of iptables porting by c47036 20060519 */
      
      /* vdf DOS / firewall / urlfilter */
      while ( bcmRemoveIpTableRule(device, "filter", "INPUT_SECDOS") == TRUE )
      { ;}
      while ( bcmRemoveIpTableRule(device, "filter", "FORWARD_SECFW") == TRUE )
      { ;}
      while ( bcmRemoveIpTableRule(device, "filter", "INPUT_SECURLBLK") == TRUE )
      { ;}
      while ( bcmRemoveIpTableRule(device, "filter", "INPUT_SECURLBLK_ADVANCE") == TRUE )
      { ;}
   }

   if ( bcmIsModuleInserted("iptable_nat") == TRUE ) {
      while ( bcmRemoveIpTableRule(device, "nat", "PREROUTING") == TRUE )
         ;
      /* Start of iptables porting by c47036 20060519 */
      while ( bcmRemoveIpTableRule(device, "nat", "PREROUTING_IN") == TRUE )
         ;
      while ( bcmRemoveIpTableRule(device, "nat", "PREROUTING_LOCAL") == TRUE )
         ;
      while ( bcmRemoveIpTableRule(device, "nat", "PREROUTING_UTILITY") == TRUE )
         ;
      /* add by j00100803 to support pc's static dns */
      while ( bcmRemoveIpTableRule(device, "nat", "PREROUTING_DNS") == TRUE )
         ;
      /* End of iptables porting by c47036 20060519 */      
/* We should keep the masquerading rules
      while ( bcmRemoveIpTableRule(device, "nat", "POSTROUTING") == TRUE )
         ;
*/

      while ( bcmRemoveIpTableRule(device, "nat", "OUTPUT") == TRUE )
         ;
   }

}

/* Start of dns 3.1.12 porting by c47036 20060508 */
/***************************************************************************
// Function Name: bcmRemoveDnsIpTableRule().
// Description  : remove DNS IP table rules.
// Parameters   : none.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveDnsIpTableRule(void) {
   char col[IP_TBL_COL_MAX][IFC_SMALL_LEN];
   char comment[IFC_LARGE_LEN], line[IFC_GIANT_LEN];
   char cIpadr[IFC_LARGE_LEN];
   int ret = FALSE, count = 0;
   FILE* fs = NULL;

   if ((access("/bin/iptables",F_OK)) != 0)
      return FALSE;

   // execute iptables command to create iptable file
   sprintf(line, "iptables -t nat -L PREROUTING_UTILITY -v --line-numbers > /var/iptable_domain");
   bcmSystemMute(line);
   
   comment[0] = '\0';
   fs = fopen("/var/iptable_domain", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         /*start of 修改dns删除列数限制 by s53329  at  20080115
         // read pass 2 header lines
         if ( count++ < 2 ) continue;
         end  of 修改dns删除列数限制 by s53329  at  20080115*/
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s",
                col[IP_TBL_COL_NUM], col[IP_TBL_COL_PKTS], col[IP_TBL_COL_BYTES],
                col[IP_TBL_COL_TARGET], col[IP_TBL_COL_PROT], col[IP_TBL_COL_OPT],
                col[IP_TBL_COL_IN], col[IP_TBL_COL_OUT], col[IP_TBL_COL_SRC],
                col[IP_TBL_COL_DST], comment, cIpadr);
         // if protocol column is "udp" and last colum is "udp dpt:domain"
         // then delete rule since it is DNS IP tables rule
         if ( strcmp(col[IP_TBL_COL_PROT], "udp") == 0 &&
              strstr(cIpadr, "dpt:domain") != NULL ) {
            sprintf(line, "iptables -t nat -D PREROUTING_UTILITY %s 2>/dev/null", col[IP_TBL_COL_NUM]);
            bcmSystemMute(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);

      // remove iptable file
      unlink("/var/iptable_domain");
   }

   return ret;
}
/* End of dns 3.1.12 porting by c47036 20060508 */

/***************************************************************************
// Function Name: bcmRemoveRipIpTableRule().
// Description  : remove RIP IP table rules.
// Parameters   : none.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveRipIpTableRule(void) {
   char col[IP_TBL_COL_MAX][IFC_SMALL_LEN];
   char comment[IFC_LARGE_LEN], line[IFC_GIANT_LEN];
   int ret = FALSE, count = 0;
   FILE* fs = NULL;

   if ((access("/bin/iptables",F_OK)) != 0)
      return FALSE;

   // execute iptables command to create iptable file
   sprintf(line, "iptables -L INPUT -v --line-numbers > /var/iptable_rip");
   bcmSystemMute(line);

   comment[0] = '\0';
   fs = fopen("/var/iptable_rip", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 2 header lines
         if ( count++ < 2 ) continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s",
                col[IP_TBL_COL_NUM], col[IP_TBL_COL_PKTS], col[IP_TBL_COL_BYTES],
                col[IP_TBL_COL_TARGET], col[IP_TBL_COL_PROT], col[IP_TBL_COL_OPT],
                col[IP_TBL_COL_IN], col[IP_TBL_COL_OUT], col[IP_TBL_COL_SRC],
                col[IP_TBL_COL_DST], comment, comment);
         // if protocol column is "udp" and last colum is "udp dpt:route"
         // then delete rule since it is RIP IP tables rule
         if ( strcmp(col[IP_TBL_COL_PROT], "udp") == 0 &&
              strcmp(comment, "dpt:route") == 0 ) {
            sprintf(line, "iptables -D INPUT %s 2>/dev/null", col[IP_TBL_COL_NUM]);
            bcmSystemMute(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);

      // remove iptable file
      unlink("/var/iptable_rip");
   }

   return ret;
}

/***************************************************************************
// Function Name: bcmRemoveUpnpIpTableRule().
// Description  : remove UPnP IP table rules.
// Parameters   : none.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveUpnpIpTableRule(void) {
   char col[IP_TBL_COL_MAX][IFC_SMALL_LEN];
   char comment[IFC_LARGE_LEN], line[IFC_GIANT_LEN];
   int ret = FALSE, count = 0;
   FILE* fs = NULL;

   if ((access("/bin/iptables",F_OK)) != 0)
      return FALSE;

   // execute iptables command to create iptable file
   sprintf(line, "iptables -L OUTPUT -v --line-numbers > /var/iptable_upnp");
   bcmSystemMute(line);

   fs = fopen("/var/iptable_upnp", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 2 header lines
         if ( count++ < 2 ) continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                col[IP_TBL_COL_NUM], col[IP_TBL_COL_PKTS], col[IP_TBL_COL_BYTES],
                col[IP_TBL_COL_TARGET], col[IP_TBL_COL_PROT], col[IP_TBL_COL_OPT],
                col[IP_TBL_COL_IN], col[IP_TBL_COL_OUT], col[IP_TBL_COL_SRC],
                col[IP_TBL_COL_DST], comment);
         // if destination column is "239.255.255.250" and target colum is "DROP"
         // then delete rule since it is RIP IP tables rule
         if ( strcmp(col[IP_TBL_COL_TARGET], "DROP") == 0 &&
              strcmp(col[IP_TBL_COL_DST], UPNP_IP_ADDRESS) == 0 ) {
            sprintf(line, "iptables -D OUTPUT %s 2>/dev/null", col[IP_TBL_COL_NUM]);
            bcmSystemMute(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);

      // remove iptable file
      unlink("/var/iptable_upnp");
   }

   return ret;
}

/***************************************************************************
// Function Name: bcmInsertAllUpnpIpTableRules().
// Description  : insert UPnP IP table rules.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void bcmInsertAllUpnpIpTableRules(void) {
   char interface[IFC_TINY_LEN], cmd[IFC_LARGE_LEN];
   WAN_CON_ID wanId;
   WAN_CON_INFO wanInfo;

   // init wanId to get WAN info from the begining
   wanId.vpi = wanId.vci = wanId.conId = 0;
   
   while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
      if ( wanInfo.flag.service == FALSE ) continue;
      if ( wanInfo.flag.nat == TRUE ) {
         // get interface name
         BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
         // If the br0 interface goes down and then comes back up, we do not need to
         // restart UPnP. All UPnP object values are obtained from the actual WAN
         // interface /dev/bcmadsl0 and /dev/bcmatm0.
         if ( bcmGetPid("upnp") <= 0 ) {
            sprintf(cmd, "upnp -L %s -W %s -D", "br0", interface);
            bcmSystem(cmd);
         }
         // Stop multicast reports for UPnP on WAN.
         sprintf(cmd, "iptables -t filter -I OUTPUT -o %s -d %s -j DROP 2>/dev/null",
                 interface, UPNP_IP_ADDRESS);
         bcmSystemMute(cmd);
      }
   }
}

/***************************************************************************
// Function Name: bcmRemoveEbTableRule().
// Description  : remove ebtables rules.
// Parameters   : device -- interface name.
//                table -- ebtables table name.
//                chain -- ebtables table chain.
// Returns      : 1 - Success. 0 - Fail
****************************************************************************/
int bcmRemoveEbTableRule(char *device, char *table, char *chain) {
   char line[IFC_GIANT_LEN];
   int ret = FALSE, count = 0, index = 0;
   FILE* fs = NULL;

   if ((access("/bin/ebtables",F_OK)) != 0)
      return FALSE;

   // execute iptables command to create iptable file
   sprintf(line, "ebtables -t %s -L %s --Ln > /var/ebtable", table, chain);
   bcmSystemMute(line);

   fs = fopen("/var/ebtable", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 3 header lines
         if ( count++ < 3 ) continue;
         // if chain rule for the given device is found
         if ( strstr(line, device) != NULL ) {
	    // get the rule index number
	    sscanf(line, "%d.", &index);
            sprintf(line, "ebtables -t %s -D %s %d 2>/dev/null", table, chain, index);
            bcmSystemMute(line);
            ret = TRUE;
            break;
         }
      }
      fclose(fs);
   }

   // code to remove ebtable file is moved to bcmRemoveAllEbTableRules() function

   return ret;
}

/***************************************************************************
// Function Name: bcmRemoveAllEbTableRules().
// Description  : remove all ebtables rules attach with the given device.
//                Each time, only one rule is removed, hence the rule index changes
//                That's why we need to use while loop to remove all
// Parameters   : device -- interface name.
// Returns      : none
****************************************************************************/
void bcmRemoveAllEbTableRules(char *device) {
// while ( bcmRemoveEbTableRule(device, "filter", "INPUT") == TRUE )
//         ;

   while ( bcmRemoveEbTableRule(device, "filter", "FORWARD") == TRUE )
         ;

// while ( bcmRemoveEbTableRule(device, "filter", "OUTPUT") == TRUE )
//         ;

   // remove ebtable file
   unlink("/var/ebtable");
}


/***************************************************************************
// Function Name: bcmGetDefaultRouteInterfaceName().
// Description  : get interface name that is used for first default route.
// Parameters   : ifcName -- the return interface name, '\0' if not found.
// Returns      : none
****************************************************************************/
void bcmGetDefaultRouteInterfaceName(char *ifcName) {
   char col[11][IFC_SMALL_LEN];
   char line[IFC_GIANT_LEN];
   int count = 0;

   if ( ifcName == NULL ) return;

   ifcName[0] = '\0';

   FILE* fsRoute = fopen("/proc/net/route", "r");
   if ( fsRoute != NULL ) {
      while ( fgets(line, sizeof(line), fsRoute) ) {
         // read pass header line
         if ( count++ < 1 ) continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                col[0], col[1], col[2], col[3], col[4], col[5],
                col[6], col[7], col[8], col[9], col[10]);
         // if destination && mask are 0 then it's default route
         if ( strcmp(col[1], "00000000") == 0 &&
              strcmp(col[7], "00000000") == 0 ) {
            strcpy(ifcName, col[0]);
            break;
         }
      }
      fclose(fsRoute);
   }
}

//***************************************************************************
// Function Name: parseStrInfo
// Description  : parse info to get value of the given variable.
// Parameters   : info -- information string.
//                var -- variable string to be searched in info string.
//                val -- value string after variable string to be returned.
//                len -- size of value string.
// Returns      : none.
//***************************************************************************/
void parseStrInfo(char *info, char *var, char *val, int len) {
   char *pChar = NULL;
   int i = 0;

   if ( info == NULL || var == NULL || val == NULL ) return;

   pChar = strstr(info, var);
   if ( pChar == NULL ) return;

   // move pass the variable string in line
   pChar += strlen(var);

   // Remove spaces from beginning of value string
   while ( *pChar != '\0' && isspace((int)*pChar) != 0 )
      pChar++;

   // get data until end of line, or space char
   for ( i = 0;
         i < len && *pChar != '\0' &&
         isspace((int)*pChar) == 0;
         i++, pChar++ )
      val[i] = *pChar;

   val[i] = '\0';
}

//**************************************************************************
// Function Name: bcmConvertStrToShellStr
// Description  : convert the given string so that each its character is
//                surround by '. If character is ' then it is surround by ".
//                (a#b'c"d => 'a''#''b'"'"'c''"''d')
// Parameters   : str - the given string.
//                buf - the converted string.
// Returns      : none.
//**************************************************************************
void bcmConvertStrToShellStr(char *str, char *buf) {
   if ( buf == NULL ) return;

   int len = strlen(str);
   int i = 0, j = 0;

   for ( i = 0; i < len; i++ ) {
      if ( str[i] != '\'' ) {
         buf[j++] = '\'';
         buf[j++] = str[i];
         buf[j++] = '\'';
      } else {
         buf[j++] = '"';
         buf[j++] = str[i];
         buf[j++] = '"';
      }
   }

   buf[j]  = '\0';
}

//**************************************************************************
// Function Name: bcmProcessMarkStrChars
// Description  : use backslash in front one of the escape codes to process
//                marked string characters.
//                (a'b"c => a\'b\"c)
// Parameters   : str - the string that needs to process its special chars.
// Returns      : none.
//**************************************************************************
void bcmProcessMarkStrChars(char *str) {
   if ( str == NULL ) return;
   if ( str[0] == '\0' ) return;

   char buf[SYS_CMD_LEN];
   int len = strlen(str);
   int i = 0, j = 0;

   for ( i = 0; i < len; i++ ) {
      if ( bcmIsMarkStrChar(str[i]) == TRUE )
         buf[j++] = '\\';
      buf[j++] = str[i];
   }

   buf[j] = '\0';
   strcpy(str, buf);
}

//**************************************************************************
// Function Name: bcmIsMarkStrChar
// Description  : verify the given character is used to mark the begining or
//                ending of string or not.
// Parameters   : c -- the given character.
// Returns      : TRUE or FALSE.
//**************************************************************************
int bcmIsMarkStrChar(char c) {
   // need to add '\0' as termination character to speChars[]
   char specChars[] = { '\'', '"', '\\', '\0' };
   int len = strlen(specChars);
   int i = 0;
   int ret = FALSE;

   for ( i = 0; i < len; i++ )
      if ( c == specChars[i] )
         break;

   if ( i < len )
      ret = TRUE;

   return ret;
}

#if defined(SUPPORT_VDSL)
static char glbVdslSwVer[SYS_CMD_LEN];
//**************************************************************************
//**************************************************************************
// Function Name: bcmSetSwVer
// Description  : store software version to global variable
// Parameters   : swVer - the software version that needs to be set.
// Returns      : none.
//**************************************************************************
void bcmSetVdslSwVer(char *swVer) 
{
   if ( swVer == NULL ) return;

   if ( strlen(swVer) < SYS_CMD_LEN - 1 )
      strcpy(glbVdslSwVer, swVer);
   else {
      strncpy(glbVdslSwVer, swVer, SYS_CMD_LEN - 2);
      glbVdslSwVer[SYS_CMD_LEN - 1] = '\0';
   }
}
#endif

//**************************************************************************
// Function Name: bcmGetSwVer
// Description  : retrieve software version from global variable
// Parameters   : swVer - buffer to get the software version.
//                size - size of buffer.
// Returns      : none.
//**************************************************************************
void bcmGetSwVer(char *swVer, int size) {
   /*start of 修改问题单AU8D01157 by s53329 at  20081104
   char version[SYS_CMD_LEN];
   */
   char version[WEB_MD_BUF_SIZE_MAX];
   /*end of 修改问题单AU8D01157 by s53329 at  20081104*/
/* HUAWEI HGW s48571 2006年11月6日 adsl porting页面修改 add begin:*/
#ifdef SUPPORT_ADSL
    /* start of maintain 去除ADSL 版本号显示 by zhangliang 60003055 2006年5月30? */
   char adslPhyVersion[SYS_CMD_LEN];
    /*end of maintain 去除ADSL 版本号显示 by zhangliang 60003055 2006年5月30日 */
#endif

/* HUAWEI HGW s48571 2006年11月6日 adsl porting页面修改 add end.*/
   if ( swVer == NULL ) return;
/* HUAWEI HGW s48571 2006年11月6日 adsl porting页面修改 add begin:*/
#ifdef SUPPORT_ADSL
   // get adsl physical version
   BcmAdslCtl_GetPhyVersion(adslPhyVersion, SYS_CMD_LEN - 1);
   // create software version
   //sprintf(version, "%s%s", RELEASE_VERSION_VRB, adslPhyVersion);
   sprintf(version, "%s", RELEASE_VERSION_VRB);
   /* HUAWEI HGW s48571 2006年11月6日 adsl porting页面修改 add end.*/
   #else
   sprintf(version, "%s", HGW_SOFTVERSION);
   /* end of maintain 去除ADSL 版本号显示 by zhangliang 60003055 2006年5月27日 */
#endif
   strncpy(swVer, version, size);
}

/* start : s48571 add for VDF project to get firmware version 2007-11-16 */
//**************************************************************************
// Function Name: bcmGetfirmwareVer
// Description  : retrieve firmware version
// Parameters   : swVer - buffer to get the software version.
//                size - size of buffer.
// Returns      : none.
//**************************************************************************
void bcmGetfirmwareVer(char *swVer, int size) {


   char adslPhyVersion[SYS_CMD_LEN];

   if ( swVer == NULL ) return;

   BcmAdslCtl_GetPhyVersion(adslPhyVersion, SYS_CMD_LEN - 1);

   strncpy(swVer, adslPhyVersion, size);
}
/* end  : s48571 add for VDF project to get firmware version 2007-11-16 */   

#if defined(SUPPORT_VDSL)
//**************************************************************************
// Function Name: bcmGetVdslSwVer
// Description  : retrieve software version from global variable
// Parameters   : swVer - buffer to get the software version.
//                size - size of buffer.
// Returns      : none.
//**************************************************************************
void bcmGetVdslSwVer(char *swVer, int size) {
   if ( swVer == NULL ) return;

   if ( strlen(glbVdslSwVer) < size - 1 )
      strcpy(swVer, glbVdslSwVer);
   else {
      strncpy(swVer, glbVdslSwVer, size - 2);
      swVer[size - 1] = '\0';
   }
}
#endif

//**************************************************************************
// Function Name: bcmcheck_enable
// Description  : check the appName with ip address against the psi
//                for access mode
// Parameters   : appName - application name in the acl.conf (telnet, ssh, etc.)
//                clntAddr - incoming ip address
// Returns      : access mode - CLI_ACCESS_LOCAL, CLI_REMOTE_LOCAL, CLI_ACCESS_DISABLED
//**************************************************************************
int bcmCheckEnable(char *appName, struct in_addr clntAddr)
{
   // is client address in Access Control List ?
   /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
   /*if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
      return CLI_ACCESS_DISABLED;
      */
   /*del by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/

   if ( isAccessFromLan(clntAddr) == TRUE ) {
      // is enabled from lan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_LOCAL) == FALSE )
         return CLI_ACCESS_DISABLED;
      else
         return CLI_ACCESS_LOCAL;
   } else {
      // is enabled from wan ?
      if ( BcmScm_isServiceEnabled(appName, CLI_ACCESS_REMOTE) == FALSE )
         return CLI_ACCESS_DISABLED;
      /*start of HG553 2008.08.07 V100R001C02B022 AU8D00877  对FTP不需要使用IP限制*/
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
      else if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
      {
        if (strcmp(appName, "ftp") != 0)
        {
            return CLI_ACCESS_DISABLED;
        }
        else
        {
            return CLI_ACCESS_REMOTE;
        }
      }
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/ 
      /*end of HG553 2008.08.07 V100R001C02B022 AU8D00877 对FTP不需要使用IP限制*/
  #ifdef   SUPPORT_RADIUS
      else
      {
            if (strcmp(appName, "telnet") == 0)
            {
                return  CLI_ACCESS_REMOTE_TELNET;
            }
    #endif
            else
            {
                return CLI_ACCESS_REMOTE;
            }
    #ifdef    SUPPORT_RADIUS
      }
    #endif  
    }
}
int bcmWanEnetQuerySwitch(char *ifName) {
   FILE *errFs = NULL;
   char cmd[IFC_LARGE_LEN];
   char str[IFC_LARGE_LEN];
   int  numIfc = 0;

   sprintf(cmd, "vconfig query %s 2 2>/var/vcfgerr\n", ifName);
   bcmSystem(cmd);
   // Check the status of the previous command
   errFs = fopen("/var/vcfgerr", "r");
   if (errFs != NULL ) {
      fgets(str, IFC_LARGE_LEN, errFs);
      numIfc = atoi(str);
      fclose(errFs);
      bcmSystem("rm /var/vcfgerr");
   }
   return numIfc;
}

#endif // USE_ALL, code below this code can be linked with other apps


//*********** code shared by ftpd and tftpd **********************
//****************************************************************

/***************************************************************************
// Function Name: bcmIsModuleInserted.
// Description  : verify the given module name is already inserted or not.
// Parameters   : modName -- the given module name.
// Returns      : TRUE or FALSE.
****************************************************************************/
int bcmIsModuleInserted(char *modName) {
   int ret = FALSE;
   char buf[SYS_CMD_LEN];
   FILE* fs = fopen("/proc/modules", "r");

   if ( fs != NULL ) {
      while ( fgets(buf, SYS_CMD_LEN, fs) > 0 )
         if ( strstr(buf, modName) != NULL ) {
            ret = TRUE;
            break;
         }
      fclose(fs);
   }

   return ret;
}


/***************************************************************************
// Function Name: bcmCheckInterfaceUp().
// Description  : check status of interface.
// Parameters   : devname - name of device.
// Returns      : 1 - UP.
//                0 - DOWN.
****************************************************************************/
int bcmCheckInterfaceUp(char *devname) {
   int  skfd;
   int  ret;
   struct ifreq intf;

   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      return 0;
   }

   strcpy(intf.ifr_name, devname);

   // if interface is br0:0 and
   // there is no binding IP address then return down
   if ( strchr(devname, ':') != NULL ) {
      if (ioctl(skfd, SIOCGIFADDR, &intf) < 0) {
         close(skfd);
         return 0;
      }
   }

   // if interface flag is down then return down
   if (ioctl(skfd, SIOCGIFFLAGS, &intf) == -1) {
      ret = 0;
   } else {
      if ( (intf.ifr_flags & IFF_UP) != 0)
         ret = 1;
      else
         ret = 0;
   }

   close(skfd);

   return ret;
}

//If we don't link with busybox, we need this function
#ifndef BUILD_STATIC

static void remove_delimitor( char *s)
{
    char *p1, *p2;

    p1 = p2 = s;
    while ( *p1 != '\0' || *(p1+1) != '\0') {
        if (*p1 != '\0') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

}
/* find_pid_by_name()
 *
 *  This finds the pid of the specified process.
 *  Currently, it's implemented by rummaging through
 *  the proc filesystem.
 *
 *  Returns a list of all matching PIDs
 */
static pid_t* find_pid_by_name( char* pidName)
{
        DIR *dir;
        struct dirent *next;
        pid_t* pidList=NULL;
        int i=0;

        /*FILE *status */
        FILE *cmdline;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        /* char name[READ_BUF_SIZE]; */
                
        dir = opendir("/proc");
        if (!dir) {
                printf("cfm:Cannot open /proc");
                return NULL;
        }

        while ((next = readdir(dir)) != NULL) {
                /* re-initialize buffers */
                memset(filename, 0, sizeof(filename));
                memset(buffer, 0, sizeof(buffer));  

                /* Must skip ".." since that is outside /proc */
                if (strcmp(next->d_name, "..") == 0)
                        continue;

                /* If it isn't a number, we don't want it */
                if (!isdigit(*next->d_name))
                        continue;

                /* sprintf(filename, "/proc/%s/status", next->d_name); */
                /* read /porc/<pid>/cmdline instead to get full cmd line */
                sprintf(filename, "/proc/%s/cmdline", next->d_name);
                if (! (cmdline = fopen(filename, "r")) ) {
                        continue;
                }
                if (fgets(buffer, READ_BUF_SIZE-1, cmdline) == NULL) {
                        fclose(cmdline);
                        continue;
                }
                fclose(cmdline);

                /* Buffer should contain a string like "Name:   binary_name" */
                /*sscanf(buffer, "%*s %s", name);*/
                /* buffer contains full commandline params separted by '\0' */
                remove_delimitor(buffer);
                if (strstr(buffer, pidName) != NULL) {
                        pidList=realloc( pidList, sizeof(pid_t) * (i+2));
                        if (!pidList) {
                                printf("cfm: Out of memeory!\n");
				closedir(dir);
                                return NULL;
                        }
                        pidList[i++]=strtol(next->d_name, NULL, 0);
                }
        }
        closedir(dir);

        if (pidList)
                pidList[i]=0;
        else if ( strcmp(pidName, "init")==0) {
                /* If we found nothing and they were trying to kill "init",
                 * guess PID 1 and call it good...  Perhaps we should simply
                 * exit if /proc isn't mounted, but this will do for now. */
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=1;
        } else {
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=-1;
        }
        return pidList;
}

#endif

static pid_t* find_pid_by_name_from_stat( char* pidName)
{
        DIR *dir;
        struct dirent *next;
        pid_t* pidList=NULL;
        int i=0;

        /*FILE *status */
        FILE *cmdline;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        char *p, name[16];
                
        dir = opendir("/proc");
        if (!dir) {
                printf("cfm:Cannot open /proc");
                return NULL;
        }

        while ((next = readdir(dir)) != NULL) {
                /* re-initialize buffers */
                memset(filename, 0, sizeof(filename));
                memset(buffer, 0, sizeof(buffer));  

                /* Must skip ".." since that is outside /proc */
                if (strcmp(next->d_name, "..") == 0)
                        continue;

                /* If it isn't a number, we don't want it */
                if (!isdigit(*next->d_name))
                        continue;

                /* sprintf(filename, "/proc/%s/status", next->d_name); */
                /* read /porc/<pid>/cmdline instead to get full cmd line */
                sprintf(filename, "/proc/%s/stat", next->d_name);
                if (! (cmdline = fopen(filename, "r")) ) {
                        continue;
                }
                if (fgets(buffer, READ_BUF_SIZE-1, cmdline) == NULL) {
                        fclose(cmdline);
                        continue;
                }
                fclose(cmdline);

        		p = strrchr(buffer, ')'); /* split into "PID (cmd" and "<rest>" */
        		if(p == 0 || p[1] != ' ')
        			continue;
        		*p = 0;
                memset(name, 0, sizeof(name));
        		sscanf(buffer, "%*s (%15c", name);

                /* Buffer should contain a string like "Name:   binary_name" */
                /*sscanf(buffer, "%*s %s", name);*/
                /* buffer contains full commandline params separted by '\0' */
                remove_delimitor(name);
                if (strstr(name, pidName) != NULL) {
                        pidList=realloc( pidList, sizeof(pid_t) * (i+2));
                        if (!pidList) {
                                printf("cfm: Out of memeory!\n");
				closedir(dir);
                                return NULL;
                        }
                        pidList[i++]=strtol(next->d_name, NULL, 0);
                }
        }
        closedir(dir);

        if (pidList)
                pidList[i]=0;
        else if ( strcmp(pidName, "init")==0) {
                /* If we found nothing and they were trying to kill "init",
                 * guess PID 1 and call it good...  Perhaps we should simply
                 * exit if /proc isn't mounted, but this will do for now. */
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=1;
        } else {
                pidList=realloc( pidList, sizeof(pid_t));
                if (!pidList) {
                        printf("cfm: Out of memeory!\n");
                        return NULL;
                }
                pidList[0]=-1;
        }
        return pidList;
}

void bcmHidePassword(char *command) {
   char *ptr = NULL;
   char * begin, *end;
   int len = 0;

   /* pppd -i .....  -p password */
   if ((ptr = strstr(command,"pppd")) != NULL) {
     if (!strstr(ptr, "-p")) 
        return;
     begin = strstr(ptr,"-p") + 3;
     end = strchr(begin,' ');
     if (end == NULL) 
       len = strlen(begin);
     else 
       len = end - begin;
   }

   while (len > 0) {
      *begin = '*';
      begin++; len--;
   }
}

int glb_chipType = E_ATHEROS_WLAN;
/***************************************************************************
// Function Name: bcmSystem().
// Description  : launch shell command in the child process.
// Parameters   : command - shell command to launch.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSystemEx (char *command, int printFlag) {
   int pid = 0, status = 0;
   char *newCommand = NULL;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];

      if ( E_RALINK_WLAN == glb_chipType )
      {
          struct sched_param param;
          param.sched_priority = 0;
          sched_setscheduler(getpid(), 0, &param);
      }
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
#ifdef BRCM_DEBUG
    /*规避网关升级后偶现多个httpd的问题*/
    if (printFlag) 
    {
        printf("app: %s\r\n", command);
        if ((newCommand = strdup(command)) != NULL) 
        {
            bcmHidePassword(newCommand);
            syslog(LOG_DEBUG, newCommand);
            free(newCommand);
        }
    }
#endif
      execve("/bin/sh", argv, environ);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;
}

#define DLNA_STATUS_FILE "/var/dlna_status"
void setDlnaStatus(int status)
{
    char cmd[128];

    memset(cmd, 0, sizeof(cmd));

    sprintf(cmd,"echo %d >%s",status,DLNA_STATUS_FILE);
    system(cmd);
}

void stopDlna( void)
{
    int pid = 0;
    char cmd[512];
    
    memset(cmd, 0, sizeof(cmd));
/*BEGIN: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/
	system("rm -rf /var/mnt/*/twonkymedia.db > /dev/null");
/*BEGIN: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/


    pid = bcmGetPid("/sbin/vp/twonkymedia");
    if(pid > 0)
    {
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }
     
    pid = bcmGetPid("/sbin/vp/twonkymediaserver");
    if(pid > 0)
    {
        sprintf(cmd, "kill -9 %d", pid);
        system(cmd);
    }
}
/*BEGIN: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/
void startDlna(char *cDlnaDevice  )
{
    int status = 0;
    char cmd[512];
    
    memset(cmd, 0, sizeof(cmd));

   //system("rm -rf /var/mnt/*/twonkymedia.db > /dev/null");
    #if 1    
    system("rm -rf /var/mnt/USBDisk_1/twonkymedia.db");

    sprintf(cmd, "mkdir /var/mnt/%s/twonkymedia.db",cDlnaDevice );
    status = system(cmd);
    if ( 0 == status )
    {
        sprintf(cmd, "/sbin/vp/twonkymedia -appdata /var/mnt/%s/twonkymedia.db &",cDlnaDevice);        
        bcmSystemNice(cmd,5);
    }
    else
    {
        printf("DLNA:%s:%s:%d:twonkymedia.db can't be created![%d]\n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    #else
   
    sprintf(cmd, "/sbin/vp/twonkymedia -appdata /var/mnt/%s/twonkymedia.db &",cDlnaDevice);
    
    printf("%s\n",cmd);
    bcmSystemNice(cmd,5);
    #endif
    
    //strcpy(cmd, "/sbin/vp/twonkymedia -logfile /var/twonkymedia-log.txt -v 8191 &");
}


void restartDlna(char *cDlnaDevice )
{
    stopDlna();
    //strcpy(cmd, "/sbin/vp/twonkymedia -logfile /var/twonkymedia-log.txt -v 8191 &");
    printf("%s\n",cDlnaDevice );
    startDlna( cDlnaDevice);
 }
/*END: modify the mechanism of USB mount by DHS00169988; 2011-02-11*/
int bcmSystemNice (char *command, int iNice) 
{
    int pid = 0, status = 0;
    char *newCommand = NULL;

    if ( command == 0 )
    {
        return 1;
    }

    pid = fork();
    if ( pid == -1 )
    {
        return -1;
    }

    if ( pid == 0 ) 
    {
        char *argv[4];

        if ( E_RALINK_WLAN == glb_chipType )
        {
            struct sched_param param;
            param.sched_priority = 0;
            sched_setscheduler(getpid(), 0, &param);
        }
        nice(iNice);
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = 0;
#ifdef BRCM_DEBUG
        printf("app: %s\r\n", command);
#endif
        execve("/bin/sh", argv, environ);
        exit(127);
    }

    /* wait for child process return */
    do 
    {
        if ( waitpid(pid, &status, 0) == -1 ) 
        {
            if ( errno != EINTR )
                return -1;
        } 
        else
            return status;
    } while ( 1 );

    return status;
}

/***************************************************************************
// Function Name: bcmGetPidForFork().
// Description  : get process PID which is forked by using process name.
// Parameters   : command - command that launch the process.
// Returns      : process ID number.
****************************************************************************/
int bcmGetPidForFork(char * command)
{
    char cmdline[128], *p1, *p2;
    pid_t *pid = NULL;
    int ret = 0;

    p1 = command;
    p2 = cmdline;
    while ( *p1 != '\0') {
        if (*p1 != ' ') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

    pid = find_pid_by_name_from_stat(cmdline);
    if ( pid != NULL ) {
       ret = (int)(*pid);
       free(pid);
    }

    return ret;
}

/***************************************************************************
// Function Name: bcmGetPid().
// Description  : get process PID which is created by command executing by using process name.
// Parameters   : command - command that launch the process.
// Returns      : process ID number.
****************************************************************************/
int bcmGetPid(char * command)
{
    char cmdline[128], *p1, *p2;
    pid_t *pid = NULL;
    int ret = 0;

    p1 = command;
    p2 = cmdline;
    while ( *p1 != '\0') {
        if (*p1 != ' ') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

    pid = find_pid_by_name(cmdline);
    if ( pid != NULL ) {
       ret = (int)(*pid);
       free(pid);
    }

    return ret;
}


/***************************************************************************
// Function Name: bcmGetPidList().
// Description  : get process PID by using process name.
// Parameters   : command - command that launch the process.
// Returns      : process ID list
****************************************************************************/
int *bcmGetPidList(char * command)
{
    char cmdline[128], *p1, *p2;
    pid_t *pid = NULL;

    p1 = command;
    p2 = cmdline;
    while ( *p1 != '\0') {
        if (*p1 != ' ') {
           *p2 = *p1;
           p2++;
         }
         p1++;
    }
    *p2='\0';

    pid = find_pid_by_name(cmdline);

    return (int*)pid;
}

/***************************************************************************
// Function Name: bcmGetIntfNameSocket.
// Description  : Return the interface name a socket is bound to
// Parameters   : socketfd: Socket descriptor, intfname: Network interface name
// Returns      : Failed: -1. Succeeded: 0
****************************************************************************/
int bcmGetIntfNameSocket(int socketfd, char *intfname)
{
   int i = 0, fd = 0;
   int numifs = 0, bufsize = 0;
   struct ifreq *all_ifr = NULL;
   struct ifconf ifc;
   struct sockaddr local_addr;
   socklen_t local_len = sizeof(struct sockaddr_in);

   memset(&ifc, 0, sizeof(struct ifconf));
   memset(&local_addr, 0, sizeof(struct sockaddr));

   if (getsockname(socketfd, &local_addr,&local_len) < 0) {
     printf("bcmGetIntfNameSocket: Error in getsockname!\n");
     return -1;
   }

   //printf("bcmGetIntfNameSocket: Session comes from: %s\n",inet_ntoa(((struct sockaddr_in *)&local_addr)->sin_addr));
   
   if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
     printf("bcmGetIntfNameSocket: Error openning socket when getting socket intface info\n");
     printf("errno: %d\n", errno);    /* z60003055 added 2006-06-21 */
     return -1;
   }

   numifs = 16;

   bufsize = numifs*sizeof(struct ifreq);
   all_ifr = (struct ifreq *)malloc(bufsize);
   if (all_ifr == NULL) {
      printf("bcmGetIntfNameSocket: out of memory!\n");
      close(fd);
      return -1;
   }

   ifc.ifc_len = bufsize;
   ifc.ifc_buf = (char *)all_ifr;
   if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
      printf("bcmGetIntfNameSocket: Error getting interfaces\n");
      close(fd);
      free(all_ifr);
      return -1;
   }

   numifs = ifc.ifc_len/sizeof(struct ifreq);
   //printf("bcmGetIntfNameSocket: numifs=%d\n",numifs);
   for (i = 0; i < numifs; i ++) {
	   //printf("bcmGetIntfNameSocket: intface name=%s\n",all_ifr[i].ifr_name); 
	   struct in_addr addr1,addr2;
	   addr1 = ((struct sockaddr_in *)&(local_addr))->sin_addr;
	   addr2 = ((struct sockaddr_in *)&(all_ifr[i].ifr_addr))->sin_addr;
	   if (addr1.s_addr == addr2.s_addr) {
		strcpy(intfname, all_ifr[i].ifr_name);
	   	break;
	   }
   }

   close(fd);
   free(all_ifr);
   return 0;
}


static int getLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask)
{
   int socketfd;
   struct ifreq lan;

   if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
     printf("app: Error openning socket when getting LAN info\n");
     return -1;
   }

   strcpy(lan.ifr_name,lan_ifname);
   if (ioctl(socketfd,SIOCGIFADDR,&lan) < 0) {
     printf("app: Error getting LAN IP address\n");
     close(socketfd);
     return -1;
   }
   *lan_ip = ((struct sockaddr_in *)&(lan.ifr_addr))->sin_addr;

   if (ioctl(socketfd,SIOCGIFNETMASK,&lan) < 0) {
     printf("app: Error getting LAN subnet address\n");
     close(socketfd);
     return -1;
   }

   *lan_subnetmask = ((struct sockaddr_in *)&(lan.ifr_netmask))->sin_addr;

   close(socketfd);
   return 0;
}

static int isIpExtension(void)
{
   FILE *fp;
   int ipextension = 0;

   if ((fp=fopen("/var/ipextension","r")) != NULL) {
      fscanf(fp,"%d",&ipextension);
      fclose(fp);
   }

   return ipextension;
}

static void getIpExtIp(char *buf)
{
   FILE* fs;
   char wan[64], gateway[64], dns[64], str[256];

   if ( buf == NULL ) return;

   buf[0] = '\0';
   fs = fopen("/var/ipextinfo", "r");
   if ( fs != NULL ) {
      fgets(str, 256, fs);
      fclose(fs);
      sscanf(str, "%s %s %s\n", wan, gateway, dns);
      strcpy(buf, wan);
   }
}

/* Start of ACL porting by c47036 20060508 */
/*************************************************
  Function:       bcmGetLanInfo
  Description:    get lan ipaddress and mask.
  Calls:          getLanInfo
  Called By:      SecCfgMngr::createServiceControlFile
  Input:          lan_ifname: interface name                  
  Output:         lan_ip: lan ipaddress
                  lan_subnetmask: lan subnetmask
  Return:         0: OK  -1: ERR
*************************************************/
int bcmGetLanInfo(char *lan_ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask)
{
    if (strcmp(lan_ifname, "br0:0") == 0)
    {
        if (bcmCheckInterfaceUp("br0:0") == 0)
        {
            return -1;
        }
    }
    return getLanInfo(lan_ifname, lan_ip, lan_subnetmask);
}
/* End of ACL porting by c47036 20060508 */

int isAccessFromLan(struct in_addr clntAddr)
{
   int ret = 0;
   struct in_addr inAddr, inMask;
   char wan[64];

   getLanInfo("br0", &inAddr, &inMask);
   /* check ip address of support user to see it is in LAN or not */
   if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
      ret = 1;
   else {
      /* check ip address of support user to see if it is from secondary LAN */
      if (bcmCheckInterfaceUp("br0:0")) {
         getLanInfo("br0:0", &inAddr, &inMask);
         if ( (clntAddr.s_addr & inMask.s_addr) == (inAddr.s_addr & inMask.s_addr) )
            ret = 1;
      }

      /* Last option it must be from WAN side */
      if (isIpExtension()) {
         getIpExtIp(wan);
      if ( clntAddr.s_addr == inet_addr(wan) )
         ret = 1;
      }
   }

   return ret;
}

// return 0, ok. return -1 = wrong chip
// used by upload.c and ftpd, tftpd, tftp utilities.
int checkChipId(char *strTagChipId, char *sig2)
{
    int tagChipId = 0;
    unsigned int chipId = (int) sysGetChipId();
    int result;

    tagChipId = strtol(strTagChipId, NULL, 16);

    if (tagChipId == chipId)
        result = 0;
    else {
        printf("Chip Id error.  Image Chip Id = %04x, Board Chip Id = %04x.\n", tagChipId, chipId);
        result = -1;
    }

    return result;
}

/***************************************************************************
// Function Name: bcmCheckForRedirect(void)
// Description  : check for nat redirect for .
// Parameters   : none
// Returns      : 0 --> tcp port 21, 22, 23, 80 is redirected. -1 --> not redirected
****************************************************************************/
int bcmCheckForRedirect(void)
{
   char col[11][32];
   char line[512];
   FILE* fs;  
   int count = 0;

   if (bcmIsModuleInserted("iptable_nat") == FALSE)
      return FALSE;

   bcmSystem("iptables -t nat -L PREROUTING_UTILITY > /var/nat_redirect");

   fs = fopen("/var/nat_redirect", "r");
   if ( fs != NULL ) {
      while ( fgets(line, sizeof(line), fs) ) {
         // read pass 3 header lines
         if ( count++ < 3 )
            continue;
         sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
               col[0], col[1], col[2], col[3], col[4], col[5],
               col[6], col[7], col[8], col[9], col[10]);
        if ((strcmp(col[0], "REDIRECT") == 0) && (strcmp(col[1], "tcp") == 0) && (strcmp(col[8], "ports") == 0))
          if (strcmp(col[9], "80") == 0 || strcmp(col[9], "23") == 0 || strcmp(col[9], "21") == 0 ||
              strcmp(col[9], "22") == 0 || strcmp(col[9], "69") == 0) {
              return TRUE;
          }
      }
      fclose(fs);
   }
   unlink("/var/nat_redirect");
   return FALSE;
}

/***************************************************************************
// Function Name: bcmRemoveModules(int lanIf)
// Description  : remove not used modules to free memory.
// Parameters   : none
// Returns      : none.
****************************************************************************/
void bcmRemoveModules(int lanIf)
{
   char *modList[]=
   {
      "bcm_enet",
      "bcm_usb",
      "ipt_state",
      "ipt_mark",
      "ipt_limit",
      "ipt_tcpmss_j",
      "ipt_REDIRECT",
      "ipt_MASQUERADE",
      "ipt_mark_j",
      "ipt_LOG",
      "ipt_FTOS",
      "ip_nat_tftp",
      "ip_nat_irc",
      "ip_nat_ftp",
      "ip_nat_h323",
      "ip_nat_pptp",
      "ip_nat_gre",
      "ip_nat_rtsp",
      "ip_nat_ipsec",
      "ip_conntrack_tftp",
      "ip_conntrack_irc",
      "ip_conntrack_ftp",
      "ip_conntrack_h323",
      "ip_conntrack_pptp",
      "ip_conntrack_gre",
      "ip_conntrack_rtsp",
      "ip_conntrack_ipsec",
      "iptable_mangle",
      "ip_conntrack",
      "ip_tables",
      NULL,
   };

   char cmd[SYS_CMD_LEN];
   int i = 0;
   int saveNat = FALSE;

   if (lanIf == 0)         // if lan, do not kill bcm_usb and bcm_enet
      i = 2;
   else // if in ipow mode, leave bcm_enet out   
   {
      FILE *fs = fopen("/proc/var/fyi/wan/eth0/pid", "r");
      if (fs != NULL) 
      {
         i = 1;        
         fclose(fs);
      }
   }

   saveNat = bcmCheckForRedirect();
   if (bcmIsModuleInserted("iptable_filter") == TRUE)
   {  
       strncpy(cmd, "iptables -t filter -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_nat") == TRUE)
   {  
       strncpy(cmd, "iptables -t nat -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }
   if (bcmIsModuleInserted("iptable_mangle") == TRUE)
   {  
       strncpy(cmd, "iptables -t mangle -F", SYS_CMD_LEN-1);
       bcmSystem(cmd);
   }

   while (modList[i] != NULL)
   {
      if (bcmIsModuleInserted(modList[i]) == TRUE) 
      {
         if (!(saveNat && strcmp(modList[i], "iptable_nat") == 0))
         {
            sprintf(cmd, "rmmod %s", modList[i]);
            bcmSystem(cmd);
         }
      }
      i++;
   }
   printf("\nRemaining modules:\n");
   bcmSystemMute("cat /proc/modules");
   printf("\nMemory info:\n");
   bcmSystemMute("sysinfo");
   sleep(1);
}

/***************************************************************************
// Function Name: bcmGetIfcIndexByName().
// Description  : get interface index by its name.
// Parameters   : ifcIdx -- interface index.
//                ifcName -- interface name.
// Returns      : interface index
****************************************************************************/
int bcmGetIntf(char *ifcName) {
   struct ifreq ifr;
   int s = 0;

   if ( ifcName == NULL ) return -1;

   if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) return -1;
   strcpy(ifr.ifr_name, ifcName);
   if ( ioctl(s, SIOCGIFINDEX, &ifr) < 0 ) {
      close(s);
      return 0;
   }
   close(s);
   return ifr.ifr_ifindex;
}

int bcmWaitIntfExists(char *ifName) {
   int retry = 0;
   int ret;

   while (retry < 50) {
      if ((ret =bcmGetIntf(ifName)) <= 0) {
         usleep(5000);
         //printf("not exist,retry %d, ret %d\n",retry,ret);
         retry++;
      }
      else {
         return 1;
      }
   } /* while */
   return 0;
}

//**************************************************************************
// Function Name: bcmMacStrToNum
// Description  : convert MAC address from string to array of 6 bytes.
//                Ex: 0a:0b:0c:0d:0e:0f -> 0a0b0c0d0e0f
// Returns      : status.
//**************************************************************************
int bcmMacStrToNum(char *macAddr, char *str) {
   char *pToken = NULL, *pLast = NULL;
   char *buf;
   UINT16 i = 1;
   int len;
   
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   len = strlen(str) + 1;
   if (len > 20)
     len = 20;
   buf = (char*)malloc(len);
   memset(buf,0,len);

   if ( buf == NULL ) return FALSE;

   /* need to copy since strtok_r updates string */
   strncpy(buf, str,len-1);

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   pToken = strtok_r(buf, ":", &pLast);
   macAddr[0] = (char) strtol(pToken, (char **)NULL, 16);
   for ( i = 1; i < 6; i++ ) {
      pToken = strtok_r(NULL, ":", &pLast);
      macAddr[i] = (char) strtol(pToken, (char **)NULL, 16);
   }

   free(buf);

   return TRUE;
}

//**************************************************************************
// Function Name: bcmMacNumToStr
// Description  : convert MAC address from array of 6 bytes to string.
//                Ex: 0a0b0c0d0e0f -> 0a:0b:0c:0d:0e:0f
// Returns      : status.
//**************************************************************************
int bcmMacNumToStr(char *macAddr, char *str) {
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   sprintf(str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macAddr[0], (UINT8) macAddr[1], (UINT8) macAddr[2],
           (UINT8) macAddr[3], (UINT8) macAddr[4], (UINT8) macAddr[5]);

   return TRUE;
}
/*start of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
int bcmOption61EthToStr(char *macAddr, char *str) {
   if ( macAddr == NULL ) return FALSE;
   if ( str == NULL ) return FALSE;

   sprintf(str, "Hardware type:Ethernet  Client MAC address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macAddr[1], (UINT8) macAddr[2], (UINT8) macAddr[3],
           (UINT8) macAddr[4], (UINT8) macAddr[5], (UINT8) macAddr[6] );

   return TRUE;
}
/*end of HG553 2008.08.09 V100R001C02B022 AU8D00864 by c65985 */
#ifdef SUPPORT_POLICY
int bcmUpperStr(char * pString)
{
    if(NULL == pString)
        return -1;
    int iLen = strlen(pString);
    int n = (int)('A' - 'a');
    int i = 0;
    for(i = 0; i < iLen; i++)
    {
        if(('a' <= *(pString + i)) && ('z' >= *(pString + i)))
        {
            *(pString + i) += n;
        }
    }

    return 0;
}
int bcmUpperMacStr(char * pString)
{
    if(NULL == pString)
        return -1;
    int n = (int)('A' - 'a');

    if( ('a' <= *(pString)) && ('z' >= *(pString)) )
    {
        *pString += n;
    }
    if( ('a' <= *(pString + 1)) && ('z' >= *(pString + 1)) )
    {
        *(pString + 1) += n;
    }
    if( ('a' <= *(pString + 3)) && ('z' >= *(pString + 3)) )
    {
        *(pString + 3) += n;
    }
    if( ('a' <= *(pString + 4)) && ('z' >= *(pString + 4)) )
    {
        *(pString + 4) += n;
    }
    if( ('a' <= *(pString + 6)) && ('z' >= *(pString + 6)) )
    {
        *(pString + 6) += n;
    }
    if( ('a' <= *(pString + 7)) && ('z' >= *(pString + 7)) )
    {
        *(pString + 7) += n;
    }
    if( ('a' <= *(pString + 9)) && ('z' >= *(pString + 9)) )
    {
        *(pString + 9) += n;
    }
    if( ('a' <= *(pString + 10)) && ('z' >= *(pString + 10)) )
    {
        *(pString + 10) += n;
    }
    if( ('a' <= *(pString + 12)) && ('z' >= *(pString + 12)) )
    {
        *(pString + 12) += n;
    }
    if( ('a' <= *(pString + 13)) && ('z' >= *(pString + 13)) )
    {
        *(pString + 13) += n;
    }
    if( ('a' <= *(pString + 15)) && ('z' >= *(pString + 15)) )
    {
        *(pString + 15) += n;
    }
    if( ('a' <= *(pString + 16)) && ('z' >= *(pString + 16)) )
    {
        *(pString + 16) += n;
    }
    return 0;
}
#endif
#ifdef USE_ALL

#ifdef PORT_MIRRORING
/***************************************************************************
 * Function Name: OpenBlaaDD
 * Description  : Opens the bcmatm device.
 * Returns      : device handle if successsful or -1 if error
 ***************************************************************************/
static int OpenBlaaDD( void )
{
    int nFd ;
   	
	 if( (nFd = open("/dev/bcmatm0", O_RDWR)) < 0 )
		 printf( "OpenBlaaDD : open error %d\n", errno );

    return( nFd );
} /* OpenBcmAtm */
#endif
#endif

/***************************************************************************
// Function Name: bcmConfigPortMirroring.
// Description  : Configure the Port Mirroring feature dynamically.
// Parameters   : MirrorCfg structure pointer.
// Returns      : Failed: -1. Succeeded: 0
****************************************************************************/
int bcmConfigPortMirroring (void *pCfg)
{
#ifdef USE_ALL
	 MirrorCfg *pMirrorCfg = (MirrorCfg *) pCfg ;
	 int fd ;

#ifdef PORTMIRROR_DEBUG
	 printf ("ENGDBG:- In BcmConfigPortMirroring \n") ;
#endif

	 if ((fd = OpenBlaaDD ()) < 0) {
			printf ("Config Port Mirroring Failed \n") ;
			return -1 ;
	 }

   if (ioctl(fd, ATMIOCTL_PORT_MIRRORING, pMirrorCfg) < 0) {

      printf("IOCTL to BLAA Drive for Port Mirror CFG failed . Fatal \n") ;
      close(fd) ;
      return -1 ;
   }

   close(fd);
#endif
   return 0;
}

#ifdef USE_ALL
/***************************************************************************
// Function Name: bcmRemoveTrafficControlRules.
// Description  : remove tc rules for this interface if QoS is enabled.
// Returns      : none.
****************************************************************************/
void bcmRemoveTrafficControlRules(UINT16 vpi, UINT16 vci, UINT16 conId, UINT8 protocol) {
    /* start of protocol QoS for TdE by z45221 zhangchen 2006年8月16日
    char ifc[16];
    char cmd[SYS_CMD_LEN];

    if (protocol != PROTO_PPPOA) {
        return;
    }

    ifc[0] = '\0';
    snprintf(ifc, 16, "ppp_%d_%d_%d", vpi, vci, conId);
    sprintf(cmd, "tc qdisc del dev %s root", ifc);
    bcmSystem(cmd);
    */
#if ((!defined(SUPPORT_KPN_QOS))&&(!defined(SUPPORT_TDE_QOS))) // 没有定义SUPPORT_TDE_QOS和SUPPORT_KPN_QOS
    char ifc[16];
    char cmd[SYS_CMD_LEN];

    if (protocol != PROTO_PPPOA) {
        return;
    }

    ifc[0] = '\0';
    snprintf(ifc, 16, "ppp_%d_%d_%d", vpi, vci, conId);
    sprintf(cmd, "tc qdisc del dev %s root", ifc);
    bcmSystem(cmd);
#elif (defined(SUPPORT_TDE_QOS)) // 如果定义了SUPPORT_TDE_QOS
    char ifc[16];
    char cmd[SYS_CMD_LEN];
    WAN_CON_ID wanId;
    IFC_ATM_VCC_INFO vccInfo;

    ifc[0] = '\0';

    // 获取vccinfo
    wanId.vpi = vpi;
    wanId.vci = vci;
    wanId.conId = conId;
    BcmDb_getVccInfo(&wanId, &vccInfo);

    if ((vccInfo.enblQos == TRUE) &&  ((protocol == PROTO_PPPOE) || (protocol == PROTO_PPPOA) || (protocol == PROTO_PPPOU)) )/*modify by sxg ,add PPPOU*/
    {
        sprintf(cmd, "ebtables -t broute -D BROUTING -j mark --set-mark 0x0001 --mark-target CONTINUE -p IPv4");
        bcmSystemNoHang(cmd);
        snprintf(ifc, 16, "ppp_%d_%d_%d", vpi, vci, conId);
        sprintf(cmd, "tc qdisc del dev %s root", ifc);
        bcmSystem(cmd);
    }
#endif
    /* end of protocol QoS for TdE by z45221 zhangchen 2006年8月16日 */
}

/***************************************************************************
// Function Name: bcmAddTrafficControlRules.
// Description  : add tc rules for this interface if QoS is enabled.
// Returns      : none.
****************************************************************************/
void bcmAddTrafficControlRules(UINT16 vpi, UINT16 vci, UINT16 conId, UINT8 protocol) {
#if ((!defined(SUPPORT_KPN_QOS))&&(!defined(SUPPORT_TDE_QOS))) // z45221 没有定义SUPPORT_TDE_QOS和SUPPORT_KPN_QOS
    char ifc[16];
    char cmd[SYS_CMD_LEN];
    ADSL_CONNECTION_INFO adslInfo;
    int lineRate = 0;

    if (protocol != PROTO_PPPOA) {
        return;
    }

    ifc[0] = '\0';
    sprintf(ifc, "ppp_%d_%d_%d", vpi, vci, conId);
    // Get the actual upstream line rate from the ADSL driver.
    BcmAdslCtl_GetConnectionInfo(&adslInfo);
    if (adslInfo.LinkState != BCM_ADSL_LINK_UP) {
        return;
    }
    if ( adslInfo.ulInterleavedUpStreamRate != 0 )
         lineRate = adslInfo.ulInterleavedUpStreamRate;
    else
         lineRate = adslInfo.ulFastUpStreamRate;

    // Before we do anythng, lets add an ebtable rule at the bottom, that
    // marks the packets with low priority mark as default 0x0001.
    sprintf(cmd, "ebtables -t broute -A BROUTING -j mark --set-mark 0x0001 -p IPv4");
    bcmSystemNoHang(cmd);

    // Create the root. This also creates the classes 1:1, 1:2 and 1:3
    // automatically
    sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 1", ifc);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate %lukbit",
            ifc, adslInfo.ulInterleavedUpStreamRate);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc qdisc add dev %s parent 1:1 handle 10: prio bands 3 "
            "priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0", ifc);
    bcmSystemNoHang(cmd);
    // Create the htb's under each class.
    sprintf(cmd, "tc qdisc add dev %s parent 10:1 handle 100: pfifo limit 10", ifc);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc qdisc add dev %s parent 10:2 handle 200: pfifo limit 10", ifc);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc qdisc add dev %s parent 10:3 handle 300: pfifo limit 10", ifc);
    bcmSystemNoHang(cmd);
    // Now add the filters for each sfq using the default handles.
    sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:1",
            ifc, PRIORITY_HIGH);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:2",
            ifc, PRIORITY_MEDIUM);
    bcmSystemNoHang(cmd);
    sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:3",
            ifc, PRIORITY_LOW);
    bcmSystemNoHang(cmd);
    /* start of protocol QoS for TdE by z45221 zhangchen 2006年8月16日 */
#elif (defined(SUPPORT_TDE_QOS)) // 如果定义了SUPPORT_TDE_QOS
    // 对ppp掉线进行处理
    char ifc[16];
    char cmd[SYS_CMD_LEN];
    IFC_ATM_VCC_INFO vccInfo;
    IFC_ATM_TD_INFO tdInfo;
    char lrate[16];
    char lCeil[16];
    char lburst[16];
    WAN_CON_ID wanId;
    UINT32 category = 0;
    int iQosClsSize = 0;
    PQOS_CLASS_ENTRY pQosTbl = NULL;
    PQOS_CLASS_ENTRY pCurQosItm = NULL;
    int i = 0;

    char tcFilterSrcIp[IFC_MEDIUM_LEN];
    char tcFilterSport[IFC_MEDIUM_LEN];
    char tcFilterDstIp[IFC_MEDIUM_LEN];
    char tcFilterDport[IFC_MEDIUM_LEN];
    char tcFilterProtocol1[IFC_MEDIUM_LEN];
    char tcFilterProtocol2[IFC_MEDIUM_LEN];

    char* pszTemp = 0;
    int frmPort = 0, toPort = 0;

    tcFilterSport[0] = '\0';
    tcFilterSrcIp[0] = '\0';
    tcFilterDport[0] = '\0';
    tcFilterDstIp[0] = '\0';
    tcFilterProtocol1[0] = '\0';
    tcFilterProtocol2[0] = '\0';

#if 0
    BcmDb_getQosClassSize(&iQosClsSize);
    if (iQosClsSize > 0)
    {
        printf("zhangchen line %d file %s\n", __LINE__, __FILE__);
        pQosTbl = (PQOS_CLASS_ENTRY)malloc(sizeof(QOS_CLASS_ENTRY) * iQosClsSize);
        if ( NULL != pQosTbl)
        {
            if ( DB_GET_OK == BcmDb_getQosClassInfo(pQosTbl, iQosClsSize))
            {
                pCurQosItm = pQosTbl;
                // 循环显示所有的信息
                for ( i = 0; i < iQosClsSize; i++)
                {
                    printf("num %d limit = %d\n", i, pCurQosItm->limitRate);
                    pCurQosItm++;
                    // pCurQosItm = pQosTbl;
                }
            }
            printf("zhangchen line %d file %s\n", __LINE__, __FILE__);
            free(pQosTbl);
            pQosTbl = NULL;
        }
    }
#endif

    // 获取vccinfo
    wanId.vpi = vpi;
    wanId.vci = vci;
    wanId.conId = conId;
    BcmDb_getVccInfo(&wanId, &vccInfo);
    AtmTd_getTdInfoFromObjectId(vccInfo.tdId, &tdInfo);
    if ((protocol == PROTO_PPPOE) || (protocol == PROTO_PPPOA) || (protocol == PROTO_PPPOU)) {/*modify by sxg ,add PPPOU*/
        // 获取接口名称
        ifc[0] = '\0';
        sprintf(ifc, "ppp_%d_%d_%d", vpi, vci, conId);

        // 限速设置
        memset(lrate, 0, 16);
        memset(lCeil, 0, 16);
        // Before we do anythng, lets add an ebtable rule at the bottom, that
        // marks the packets with low priority mark as default 0x0001.
        // 当一个skb匹配到了当前的规则,就不会执行其他的规则.因此需要加上--mark-target CONTINUE
        sprintf(cmd, "ebtables -t broute -A BROUTING -j mark --set-mark 0x0001 --mark-target CONTINUE -p IPv4");
        bcmSystemNoHang(cmd);
#ifdef SUPPORT_VDSL
		// PTM
        if ((TM_PTM_VLAN == g_trafficmode) 
           || (TM_PTM_PRTCL == g_trafficmode)
           || (TM_PTM_BR == g_trafficmode))
        {
            if ( vccInfo.enblQos == TRUE ) {
                snprintf(lrate, 16, "%lu", vccInfo.QosIfcRate);
                snprintf(lCeil, 16, "%lu", vccInfo.QosIfcCeil);
                strcat(lrate, "kbit");
                strcat(lCeil, "kbit");
                
                // Create the root. This also creates the classes 1:1, 1:2 and 1:3
                // automatically
                sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 2", ifc);
                bcmSystemNoHang(cmd);
                sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate %s",
                          ifc, lCeil);
                bcmSystemNoHang(cmd);
                // 添加子class借用带宽
                sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s",
                          ifc, lrate, lCeil);
                bcmSystemNoHang(cmd);
            }
        }
        else // ATM
        {
            category = AtmTd_strToTrffDscrCategory(tdInfo.serviceCategory);
            if (SC_OTHER != category)
            {
                sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 2", ifc);
                bcmSystemNoHang(cmd);
                snprintf(lrate, 16, "%lu", (tdInfo.sustainedCellRate * 49 * 8) / 1000 );
                snprintf(lCeil, 16, "%lu", (tdInfo.peakCellRate * 49 * 8) / 1000 );
                snprintf(lburst, 16, "%lu", tdInfo.maxBurstSize * 49 );
                strcat(lrate, "kbit");
                strcat(lCeil, "kbit");
                sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate %s",
                          ifc, lCeil);
                bcmSystemNoHang(cmd);
                // 添加子class借用带宽
                switch (category) {
                    case SC_CBR:
                        sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s",
                            ifc, lCeil, lCeil);
                        break;
                    case SC_RT_VBR:
                    case SC_NRT_VBR:
                        sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s burst %s",
                            ifc, lrate, lCeil, lburst);
                        break;
                    default :
                        if ( tdInfo.peakCellRate != 0 )
                        {
                            sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s",
                                ifc, lCeil, lCeil);
                        }
                        break;
                    }
                bcmSystemNoHang(cmd);
            }
            else
            {
                sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 2", ifc);
                bcmSystemNoHang(cmd);
                sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate 100000kbit", ifc);
                bcmSystemNoHang(cmd);
                sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate 100000kbit", ifc);
                bcmSystemNoHang(cmd);
            }
        }
#else
		// ATM
        category = AtmTd_strToTrffDscrCategory(tdInfo.serviceCategory);
        if (SC_OTHER != category)
        {
            sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 2", ifc);
            bcmSystemNoHang(cmd);

            snprintf(lrate, 16, "%lu", (tdInfo.sustainedCellRate * 49 * 8) / 1000 );
            snprintf(lCeil, 16, "%lu", (tdInfo.peakCellRate* 49 * 8) / 1000 );
            snprintf(lburst, 16, "%lu", tdInfo.maxBurstSize * 49 );
            strcat(lrate, "kbit");
            strcat(lCeil, "kbit");
            sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate %s",
                      ifc, lCeil);
            bcmSystemNoHang(cmd);

            // 添加子class借用带宽
            switch (category) {
                case SC_CBR:
                    sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s",
                        ifc, lCeil, lCeil);
                    break;
                case SC_RT_VBR:
                case SC_NRT_VBR:
                    sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s burst %s",
                        ifc, lrate, lCeil, lburst);
                    break;
                default :
                    if ( tdInfo->peakCellRate != 0 )
                    {
                        sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate %s ceil %s",
                            ifc, lCeil, lCeil);
                    }
                    break;
            }
            bcmSystemNoHang(cmd);
        }
        else
        {
            sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 2", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate 100000kbit", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 1:1 classid 1:2 htb rate 100000kbit", ifc);
            bcmSystemNoHang(cmd);
        }
#endif

#if 1
        if ( vccInfo.enblQos == TRUE ) {
            sprintf(cmd, "tc qdisc add dev %s parent 1:2 handle 10: prio bands 3 "
                    "priomap 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2", ifc);
            bcmSystemNoHang(cmd);
            // Create the htb's under each class.
            // 再挂htb对每个流进行限速
            sprintf(cmd, "tc qdisc add dev %s parent 10:1 handle 100: htb default 2", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc qdisc add dev %s parent 10:2 handle 200: htb default 2", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc qdisc add dev %s parent 10:3 handle 300: htb default 2", ifc);
            bcmSystemNoHang(cmd);

            // Now add the filters for each sfq using the default handles.
            sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:1",
                    ifc, PRIORITY_HIGH);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:2",
                    ifc, PRIORITY_MEDIUM);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:3",
                    ifc, PRIORITY_LOW);
            bcmSystemNoHang(cmd);

            // 对每个htb挂root class确保子类之间可以借用带宽
            sprintf(cmd, "tc class add dev %s parent 100: classid 100:1 htb rate 100000kbit", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 200: classid 200:1 htb rate 100000kbit", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 300: classid 300:1 htb rate 100000kbit", ifc);
            bcmSystemNoHang(cmd);

            // 在root class下挂子class,为默认class,并限制下挂pfifo队列长度,保证优先级
            sprintf(cmd, "tc class add dev %s parent 100:1 classid 100:2 htb rate 100000kbit",
                      ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 200:1 classid 200:2 htb rate 100000kbit",
                      ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc class add dev %s parent 300:1 classid 300:2 htb rate 100000kbit",
                      ifc);
            bcmSystemNoHang(cmd);

            // 限制pfifo队列长度,保证优先级
            sprintf(cmd, "tc qdisc add dev %s parent 100:2 handle 1000: pfifo limit 40", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc qdisc add dev %s parent 200:2 handle 2000: pfifo limit 40", ifc);
            bcmSystemNoHang(cmd);
            sprintf(cmd, "tc qdisc add dev %s parent 300:2 handle 3000: pfifo limit 40", ifc);
            bcmSystemNoHang(cmd);

            // 在root class下挂子class,限制每个class的速率
            BcmDb_getQosClassSize(&iQosClsSize);
            if (iQosClsSize > 0)
            { 
                pQosTbl = (PQOS_CLASS_ENTRY)malloc(sizeof(QOS_CLASS_ENTRY) * iQosClsSize);
                if ( NULL != pQosTbl)
                {
                    if ( DB_GET_OK == BcmDb_getQosClassInfo(pQosTbl, iQosClsSize))
                    {
                        pCurQosItm = pQosTbl;
                        // 循环查询每条规则是否启用traffic limit
                        for ( i = 0; i < iQosClsSize; i++)
                        {
                            if (pCurQosItm->enbTrafficLimit == TRUE)
                            {
                                // source address/mask
                                if ( pCurQosItm->srcAddr[0] != '\0' ) 
                                {
                                    if ( pCurQosItm->srcMask[0] != '\0' )
                                    {
                                        sprintf(tcFilterSrcIp, "match ip src %s/%d", 
                                            pCurQosItm->srcAddr, BcmScm_getLeftMostOneBitsInMask(pCurQosItm->srcMask));
                                    }
                                    else
                                    {
                                        sprintf(tcFilterSrcIp, "match ip src %s/32", 
                                            pCurQosItm->srcAddr);
                                    }
                                }

                                // source port
                                if ( pCurQosItm->srcPort[0] != '\0' && (NULL == strchr(pCurQosItm->srcPort, ':')))
                                {
                                    sprintf(tcFilterSport, "match ip sport %s 0xffff", pCurQosItm->srcPort);
                                }
                                else if( (pCurQosItm->srcPort[0] != '\0') && (NULL != strchr(pCurQosItm->srcPort, ':')) )
                                {
                                    // a port range.
                                    pszTemp = strchr(pCurQosItm->srcPort, ':');
                                    *pszTemp = '\0';
                                    frmPort = atoi(pCurQosItm->srcPort);
                                    toPort = atoi(++pszTemp);
                                    pszTemp = NULL;
                                    sprintf(tcFilterSport, "match ip sport %d 0xffff", frmPort);
                                }
                               
                                // destination address/mask
                                if ( pCurQosItm->dstAddr[0] != '\0' ) 
                                {
                                    if ( pCurQosItm->dstMask[0] != '\0' )
                                    {
                                        sprintf(tcFilterDstIp, "match ip dst %s/%d", pCurQosItm->dstAddr, BcmScm_getLeftMostOneBitsInMask(pCurQosItm->dstMask));
                                    }
                                    else
                                    {
                                        sprintf(tcFilterDstIp, "match ip dst %s/32", pCurQosItm->dstAddr);
                                    }
                                }

                                // destination port
                                if ( pCurQosItm->dstPort[0] != '\0' && (NULL == strchr(pCurQosItm->dstPort, ':')))
                                {
                                    sprintf(tcFilterDport, "match ip dport %s 0xffff", pCurQosItm->dstPort);
                                }
                                else if ( (pCurQosItm->dstPort[0] != '\0') && (NULL != strchr(pCurQosItm->dstPort, ':')) )
                                {
                                    // a port range.
                                    pszTemp = strchr(pCurQosItm->dstPort, ':');
                                    *pszTemp = '\0';
                                    frmPort = atoi(pCurQosItm->dstPort);
                                    toPort = atoi(++pszTemp);
                                    pszTemp = NULL;
                                    sprintf(tcFilterDport, "match ip dport %d 0xffff", frmPort);
                                }

                                // protocol
                                switch ( pCurQosItm->protocol ) 
                                {
                                    case SEC_PROTO_TCP:  // TCP
                                        sprintf(tcFilterProtocol1, "match ip protocol 6 0xff");
                                        break;
                                    case SEC_PROTO_UDP: // UDP
                                        sprintf(tcFilterProtocol1, "match ip protocol 17 0xff");
                                        break;
                                    case SEC_PROTO_BOTH: // Both
                                        sprintf(tcFilterProtocol1, "match ip protocol 6 0xff");
                                        sprintf(tcFilterProtocol2, "match ip protocol 17 0xff"); 
                                        break;
                                    case SEC_PROTO_ICMP: // ICMP
                                        sprintf(tcFilterProtocol1, "match ip protocol 1 0xff");
                                        break;
                                }
                                // 启用了traffic限速才需要再挂子class
                                switch (pCurQosItm->priority)
                                {
                                    case QOS_PRIORITY_HIGH :
                                        // 设置子class从3开始
                                        sprintf(cmd, "tc class replace dev %s parent 100:1 classid 100:%d htb rate %dkbit", ifc, (pCurQosItm->clsId+ 2), pCurQosItm->limitRate);
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc filter replace dev %s parent 100:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 100:%d", 
                                            ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol1, (pCurQosItm->clsId + 2));
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc qdisc add dev %s parent 100:%d handle %d000: pfifo limit 40", 
                                            ifc, (pCurQosItm->clsId + 2), (pCurQosItm->clsId + 3));
                                        bcmSystemNoHang(cmd);
                                        break;
                                    case QOS_PRIORITY_MED :
                                        sprintf(cmd, "tc class replace dev %s parent 200:1 classid 200:%d htb rate %dkbit", ifc, (pCurQosItm->clsId+ 2), pCurQosItm->limitRate);
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc filter replace dev %s parent 200:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 200:%d", 
                                            ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol1, (pCurQosItm->clsId + 2));
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc qdisc add dev %s parent 200:%d handle %d000: pfifo limit 40", 
                                            ifc, (pCurQosItm->clsId + 2), (pCurQosItm->clsId + 3));
                                        bcmSystemNoHang(cmd);
                                        break;
                                    case QOS_PRIORITY_LOW :
                                        sprintf(cmd, "tc class replace dev %s parent 300:1 classid 300:%d htb rate %dkbit", ifc, (pCurQosItm->clsId+ 2), pCurQosItm->limitRate);
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc filter replace dev %s parent 300:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 300:%d", 
                                            ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol1, (pCurQosItm->clsId + 2));
                                        bcmSystemNoHang(cmd);
                                        sprintf(cmd, "tc qdisc add dev %s parent 300:%d handle %d000: pfifo limit 40", 
                                            ifc, (pCurQosItm->clsId + 2), (pCurQosItm->clsId + 3));
                                        bcmSystemNoHang(cmd);
                                        break;
                                    default :
                                        break;
                                }
                                if (tcFilterProtocol2[0] != '\0')
                                {
                                    // 支持tcp和udp同时选择
                                    switch (pCurQosItm->priority)
                                    {
                                        case QOS_PRIORITY_HIGH :
                                            // 设置子class从minor id从3开始
                                            sprintf(cmd, "tc filter replace dev %s parent 100:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 100:%d", 
                                                ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol2, (pCurQosItm->clsId + 2));
                                            bcmSystemNoHang(cmd);
                                            break;
                                        case QOS_PRIORITY_MED :
                                            sprintf(cmd, "tc filter replace dev %s parent 200:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 200:%d", 
                                                ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol2, (pCurQosItm->clsId + 2));
                                            bcmSystemNoHang(cmd);
                                            break;
                                        case QOS_PRIORITY_LOW :
                                            sprintf(cmd, "tc filter replace dev %s parent 300:0 protocol ip prio 1 u32 %s %s %s %s %s flowid 300:%d", 
                                                ifc, tcFilterSrcIp, tcFilterSport, tcFilterDstIp, tcFilterDport, tcFilterProtocol2, (pCurQosItm->clsId + 2));
                                            bcmSystemNoHang(cmd);
                                            break;
                                        default :
                                            break;
                                    }
                                }
                            }
                            pCurQosItm++;
                        }
                    }
                    free(pQosTbl);
                    pQosTbl = NULL;
                }
            }
#endif
        }
    }
#endif // 定义了SUPPORT_TDE_QOS
    /* end of protocol QoS for TdE by z45221 zhangchen 2006年8月16日 */
}

/* start PPPOE/PPPOA方式时，QOS不生效 for A36D03573 */
void bcmAddTrafficControlRules_doDefaultPolicy()
{
   char cmd[IFC_GIANT_LEN];
#define MARK_USE_IPTABLES   
#ifdef MARK_USE_IPTABLES   
   char *protocols[]={"-p udp --dport 53", "-p 2", "-p udp --sport 161",
                      "-p udp --sport 520", "-p udp --dport 67:68"};
   //  DNS (generated by DNS probe), IGMP, SNMP, RIP, DHCP relay
#else
   char *protocols[]={"17 --ip-dport 53", "2", "17 --ip-sport 161",
                      "17 --ip-sport 520", "17 --ip-dport 67:68"};
#endif		      
//#ifdef SUPPORT_TC
   WAN_CON_ID wanId;
   WAN_CON_INFO wanInfo;
   IFC_ATM_VCC_INFO vccInfo;
   char interface[IFC_TINY_LEN];
   ADSL_CONNECTION_INFO adslInfo;
   int lineRate = 0;
   UINT16 i = 0;
   char lrate[16];
   /* Add HTB qdisc to limit the rate for the upstream to 90% of the upstream rate.
   // Attach default traffic control PRIO qdisc with handle 10: the following classes
   // 10:1 high priority
   // 10:2 medium priority
   // 10:3 low priority
   // Attach 3 qdiscs at the end of each class with SFQ queue discipline
   // Now add filters to each of the classes with the following handles
   // handle 3 high priority assigned to flowid 1:1
   // handle 2 medium priority assigned to flowid 1:2
   // handle 1 low priority assigned to flowid 1:3
   // Each priority band is attached to a HTB qdisc whose ceiling rate
   // is the MAX rate which is fixed at 90% of upstream line rate
   // The tree looks like this
   //               root 1:0 htb
   //                    |
   //                   1:1
   //                    |
   //                   10: PRIO
   //                  / | \
   //                /   |   \
   //              /     |    \
   //             /      |     \
   //            10:1   10:2   10:3
   //             |      |      |
   //             |      |      |
   //            100:   200:   300:
   //           sfq     sfq    sfq
   //             3      2      1
   //           high   medium  low
   */

   // Before we do anythng, lets add an ebtable rule at the bottom, that
   // marks the packets with low priority mark as default 0x0001.
   //sprintf(cmd, "ebtables -t broute -A BROUTING -j mark --set-mark 0x0001 -p IPv4");
   //bcmSystem(cmd);
   wanId.vpi = wanId.vci = wanId.conId = 0;
   while ( BcmDb_getWanInfoNext(&wanId, &wanInfo) == DB_WAN_GET_OK ) {
      BcmDb_getVccInfo(&wanId, &vccInfo);
      if ( vccInfo.enblQos == TRUE ) {
          if (wanInfo.protocol != PROTO_PPPOA && wanInfo.protocol != PROTO_PPPOE && wanInfo.protocol != PROTO_PPPOU) /*modify by sxg, add PPPOU*/
          {
             // The rules must be launched when the PPPoA interface comes up
             continue;
          }
          #if 0
          if (wanInfo.protocol == PROTO_PPPOE) {
             memset(interface, 0, IFC_TINY_LEN);
             snprintf(interface, IFC_TINY_LEN, "nas_%d_%d", wanId.vpi, wanId.vci);
          } else
          #endif
             BcmDb_getWanInterfaceName(&wanId, wanInfo.protocol, interface);
          // Get the actual upstream line rate from the ADSL driver.
          BcmAdslCtl_GetConnectionInfo(&adslInfo);
          if (adslInfo.LinkState != BCM_ADSL_LINK_UP) {
             return;
          }
          if ( adslInfo.ulInterleavedUpStreamRate != 0 )
             lineRate = adslInfo.ulInterleavedUpStreamRate;
          else
             lineRate = adslInfo.ulFastUpStreamRate;

          memset(lrate, 0, 16);
          snprintf(lrate, 16, "%d", (((lineRate * 9)/10000)));
          strcat(lrate, "kbit");
          // Create the root. This also creates the classes 1:1, 1:2 and 1:3
          // automatically
          sprintf(cmd, "tc qdisc add dev %s root handle 1: htb default 1", interface);
          bcmSystemNoHang(cmd);
          // Use 90% of the upstream line rate.
          sprintf(cmd, "tc class add dev %s parent 1: classid 1:1 htb rate %s",
                  interface, lrate);
          bcmSystemNoHang(cmd);
          // sprintf(cmd, "tc qdisc add dev %s parent 1:1 handle 10: prio bands 3 "
          //              "priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0", interface);
          sprintf(cmd, "tc qdisc add dev %s parent 1:1 handle 10: prio bands 3 "
                        "priomap 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2", interface);
          bcmSystemNoHang(cmd);
          // Create the htb's under each class.
          sprintf(cmd, "tc qdisc add dev %s parent 10:1 handle 100: pfifo limit 10", interface);
          bcmSystemNoHang(cmd);
          sprintf(cmd, "tc qdisc add dev %s parent 10:2 handle 200: pfifo limit 10", interface);
          bcmSystemNoHang(cmd);
          sprintf(cmd, "tc qdisc add dev %s parent 10:3 handle 300: pfifo limit 10", interface);
          bcmSystemNoHang(cmd);
          // Now add the filters for each sfq using the default handles.
          sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:1",
                  interface, 3);
          bcmSystemNoHang(cmd);
          sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:2",
                  interface, 2);
          bcmSystemNoHang(cmd);
          sprintf(cmd, "tc filter add dev %s protocol ip parent 10:0 prio 1 handle %d fw classid 10:3",
                  interface, 1);
          bcmSystemNoHang(cmd);
      }
   }
//#endif /* SUPPORT_TC */


   // do nothing if all PVCS are in bridge mode 
   if ( BcmDb_isRoutedWanExisted() == FALSE )
	return;

   // do nothing if there is no QoS-enabled PVC
   /*
   if ( m_QosEnabledRoutingPVC == FALSE && m_QosEnabledBridgePVC == FALSE )
	return;*/

   
   // Launch implicit QoS policies on the OUTPUT chains to allow the packets generated
   // by the router itself to be able to go out when the upstream link is saturated by
   // certain high-priority bursty packets such as FTP. In other words, we need to
   // set the highest priority for such packets as DNS probe, SNMP, IGMP, RIP, DHCP relay
   // so that the router can still respond to those protocols. 
   // Note: output chain will NOT affect routing performance.
   //sprintf(cmd, "iptables -t mangle -A OUTPUT -j MARK --set-mark %d", HIGHEST_PRIORITY);
   for ( i = 0; i < sizeof(protocols)/sizeof(char *); i ++ ) {
#ifdef MARK_USE_IPTABLES	   
       sprintf(cmd, "iptables -t mangle -A OUTPUT -j MARK --set-mark %d %s", 3, protocols[i]);
#else
       sprintf(cmd, "ebtables -A OUTPUT -j mark --set-mark %d -p IPV4 --ip-proto %s", 3, protocols[i]);
#endif 
#undef MARK_USE_IPTABLES      
       bcmSystem(cmd);
   }
   //printf("========QoS  ppp up===========\n");

}
/* end PPPOE/PPPOA方式时，QOS不生效 for A36D03573 */


#endif

/* start of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */
/*start of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/
 int msgid ;
 static pthread_mutex_t work_mutex;
void tmpfile_WritePro()
{
	FILE *psFile1, *psFile2;
	char achFileBuf[TMPFILE_MAX_ITEM_LEN], achStrBuf[TMPFILE_MAX_ITEM_LEN], *pchBuf;
	int  iFlag;
        /*start   of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
	int   iRet = 0;
        /*end   of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */

	S_TMPFILE_MSG sMsgIn;
	memset(&sMsgIn, 0, sizeof(sMsgIn));
#if (TMPFILE_DEBUG == 1)
        printf("<Lazyjack> tmpfile_WritePro\n");
#endif
        while(1)
        {
            if (-1 == msgrcv(msgid, 
                             &sMsgIn, 
                             sizeof(sMsgIn) - sizeof(long),
                             TMPFILE_WRITE_OPT,
                             0))
            {
                continue;
            }
            /* 从/var/homegateway.conf文件中查找输入的数据名字符串
             * if 数据名已存在
             * 修改该字段的值
             * else //数据不存在
             *     增加一项，并写入文件
             */
#if (TMPFILE_DEBUG == 1)
            printf("<Lazyjack> rcv write msg: %s = %s\n", sMsgIn.achName, sMsgIn.achValue);
#endif
   	  iRet = pthread_mutex_lock(&work_mutex);
          /*start   of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
         if (0 != iRet)
         {
              printf("pthread_mutex_lock    erorr   \n");
              continue;
         }
         /*end  of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
	  psFile1 = fopen(TMPFILE_FILE_NAME, "r");
            if (NULL == psFile1)
            {
                printf("Open file failed.\n");
                exit(EXIT_FAILURE);
            }

            psFile2 = fopen(TMPFILE_TMPFILE_NAME, "wt+");
            if (NULL == psFile2) 
            {
                printf("Open tmp file failed.\n");
                exit(EXIT_FAILURE);
            }

            iFlag = TMPFILE_NOT_FOUND_ITEM;
            while (!feof(psFile1)) 
            {
                memset(achFileBuf, 0, sizeof(achFileBuf));
                pchBuf = fgets(achFileBuf, TMPFILE_MAX_ITEM_LEN, psFile1);
                /*start of 修改tmpfile 死机引起的问题 by s53329  at  20080303
                if (0 == strlen(achFileBuf))
                end  of 修改tmpfile 死机引起的问题 by s53329  at  20080303*/
                if(NULL == pchBuf)
                {
                    continue;
                }

                sprintf(achStrBuf, "%s = ", sMsgIn.achName);
                if (0 == strncmp(achStrBuf, achFileBuf, strlen(achStrBuf)))
                {
                    sprintf(achStrBuf, "%s = %s\n", sMsgIn.achName, sMsgIn.achValue);
                    fputs(achStrBuf, psFile2);
                    iFlag = TMPFILE_FOUND_ITEM;
                }
                else
                {
                    /*start of 修改tmpfile 死机引起的问题 by s53329  at  20080303
                    fputs(pchBuf, psFile2);
                    end  of 修改tmpfile 死机引起的问题 by s53329  at  20080303*/
                    fputs(achFileBuf, psFile2);
                }
            } 

            if (TMPFILE_NOT_FOUND_ITEM == iFlag)
            {
                sprintf(achStrBuf, "%s = %s\n", sMsgIn.achName, sMsgIn.achValue);
                fputs(achStrBuf, psFile2);
            }
            fclose(psFile2);
            fclose(psFile1);


            unlink(TMPFILE_FILE_NAME);
            rename(TMPFILE_TMPFILE_NAME, TMPFILE_FILE_NAME);
           
            pthread_mutex_unlock(&work_mutex);
            memset(&sMsgIn, 0, sizeof(sMsgIn));
        }

}
void tmpfile_ReadPro()
{
    char achFileBuf[TMPFILE_MAX_ITEM_LEN], achStrBuf[TMPFILE_MAX_ITEM_LEN], *pchBuf;
    FILE *psFile1, *psFile2;
    S_TMPFILE_MSG sMsgIn, sMsgOut;
    int  iFlag;
    /*start   of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
    int  iRet = 0;
    /*end    of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */

    memset(&sMsgIn, 0, sizeof(sMsgIn));
#if (TMPFILE_DEBUG == 1)
        printf("<Lazyjack> tmpfile_ReadPro\n");
#endif
            /* 接收读操作的消息 */
    while(1)
    {
       if  (-1 == msgrcv(msgid,
                     &sMsgIn,
                     sizeof(sMsgIn) - sizeof(long),
                     TMPFILE_READ_OPT,
                     0))
        {
            continue;
        }
        /* 从/var/homegateway.conf文件中查找输入的数据名字符串
         * if 数据名已存在
         *     返回数据值
         * else //数据不存在
         *     返回错误;
         */
#if (TMPFILE_DEBUG == 1)
        printf("<Lazyjack> rcv read msg: %d serial %d read %s\n", sMsgIn.iPid, sMsgIn.iSerial, sMsgIn.achName);
#endif
	 iRet = pthread_mutex_lock(&work_mutex);
        /*start   of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
        if (0 != iRet)
        {
            printf("pthread_mutex_lock    erorr\n");
            continue;
        }
        /*end    of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */

        psFile1 = fopen(TMPFILE_FILE_NAME, "r");
        if (NULL == psFile1)
        {
            printf("Open file failed.\n");
            exit(EXIT_FAILURE);
        }

        memset(&sMsgOut, 0, sizeof(sMsgOut));
        sMsgOut.lMsgType = sMsgIn.iPid; //进程号作为回应消息的类型
        sMsgOut.iSerial = sMsgIn.iSerial;

        iFlag = TMPFILE_NOT_FOUND_ITEM;
        while (!feof(psFile1)) 
        {
            /*start of 修改tmpfile 文件挂死问题 by s53329 at  20080303  */
            memset(achFileBuf, 0, sizeof(achFileBuf));
            /*end  of 修改tmpfile 文件挂死问题 by s53329 at  20080303  */
            pchBuf = fgets(achFileBuf, TMPFILE_MAX_ITEM_LEN, psFile1);
            /*start of 修改tmpfile 文件挂死问题 by s53329 at  20080303
            if (0 == strlen(achFileBuf)) 
            end  of 修改tmpfile 文件挂死问题 by s53329 at  20080303*/
            if (0 == pchBuf) 
            {
                continue;
            }

#if (TMPFILE_DEBUG == 1)
            printf("<Lazyjack> file content: %s\n", achFileBuf);
#endif
            sprintf(achStrBuf, "%s = ", sMsgIn.achName);
            if (0 == strncmp(achStrBuf, achFileBuf, strlen(achStrBuf)))
            {
                sMsgOut.iResult = 0;
                strcpy(sMsgOut.achName, sMsgIn.achName);
                strcpy(sMsgOut.achValue, &achFileBuf[strlen(achStrBuf)]);
                 /*start of 修改tmpfile 文件挂死问题 by s53329 at  20080303    */
                if (strlen(sMsgOut.achValue) != 0)
                {
                 /*end  of 修改tmpfile 文件挂死问题 by s53329 at  20080303    */
                    sMsgOut.achValue[strlen(sMsgOut.achValue) - 1] = '\0'; //去掉最后的换行符
                 /*start of 修改tmpfile 文件挂死问题 by s53329 at  20080303    */
                }
                 /*end  of 修改tmpfile 文件挂死问题 by s53329 at  20080303    */
                iFlag = TMPFILE_FOUND_ITEM;
                break;
            }
        } 
        fclose(psFile1);
    	 pthread_mutex_unlock(&work_mutex);

        memset(&sMsgIn, 0, sizeof(sMsgIn));

        if (TMPFILE_NOT_FOUND_ITEM == iFlag)
        {
            sMsgOut.iResult = -1;
        }

#if (TMPFILE_DEBUG == 1)
        printf("<Lazyjack> read ack: %d serial %d read %s result %d get %s\n",
            sMsgOut.lMsgType, sMsgOut.iSerial, sMsgOut.achName, sMsgOut.iResult, sMsgOut.achValue);
#endif
        if (-1 == msgsnd(msgid, &sMsgOut, sizeof(sMsgOut) - sizeof(long), IPC_NOWAIT))
        {
            continue;
        }

    }
}
/*end  of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/

/***********************************************************
Function:       void tmpfile_init(void)
Description:    var目录下文件操作初始化，创建一个监控进程处理读写消息
Calls:          NULL
Called By:      init
Input:          NULL
Output:         NULL
Return:         NULL
Others:         NULL
************************************************************/
void tmpfile_init(void)
{
    int iPid;
    int icPid;
    key_t iKey;
    int fd;
    pthread_t ReadThread = NULL;
    pthread_t WriteThread = NULL;
    FILE *psFile1;
    psFile1 = fopen(TMPFILE_FILE_NAME, "w");
    if (NULL == psFile1)
    {
        usleep(100000);
        printf("Open file %s faieled,retry!\n",TMPFILE_FILE_NAME);
        psFile1 = fopen(TMPFILE_FILE_NAME, "w");
        if (NULL == psFile1)
        {
            printf("Create template file failed.\n");
            return;
        }
    }
    fputs("BEGIN\n", psFile1);
    fclose(psFile1);

    iPid = fork();
    if (-1 == iPid) 
    {
        printf("Fork process for template file r/w failed.\n");
        return;
    }

    if (0 == iPid) 
    {

	  icPid = fork();

	  if (icPid == -1)
  	 {
  	  	printf("Fork process for template file r/w failed.\n");
		return;
  	 }

	  if (icPid)
  	 {
  	 	exit(0);
  	 }

        if ((fd = open("/dev/bcm", O_RDWR)) < 0)
            perror("Tmpfile:open");
        if (ioctl(fd, RENAME_TMPFILE, 0) < 0)
            perror("ioctl");
        if ( fd > 0)
            close(fd);

        int iRetryTimes = 0;
        iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
        if ( -1 == iKey)
        {
            usleep(100000);
            iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
        }
        /*避免初始化时先调用了tmpfilewrite，再init导致init失败*/
        while (-1 == (msgid = msgget(iKey, IPC_CREAT | 0660)))
        {
            iRetryTimes++;
            if (10 <= iRetryTimes)
            {
                printf("Create tmpfile message queue failed.msgid: %d ,Errno: %d\n",msgid,errno);
                exit(EXIT_FAILURE);
            }
            printf("Create tmpfile message queue failed,errno %d,try again!\n",errno);
            usleep(100000);
        }

        /*start of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/
	 if (0 != pthread_mutex_init(&work_mutex,NULL))
	 {
	      return 0;
	 }
	  /*end  of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/

#if (TMPFILE_DEBUG == 1)
        printf("<Lazyjack> Start to receive message\n");
#endif
        /*start of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/
        pthread_create(&ReadThread,  NULL, tmpfile_ReadPro, NULL);
        /*start of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301 
        pthread_create(&WriteThread,  NULL, tmpfile_WritePro, NULL);
        pause();
        */
        tmpfile_WritePro();

	 exit(0);

    	 /*end  of 修改tmpfile 读写流程，保证效率 by s53329 at  20080226*/
        /*end  of 修改tmpfile ，规避可能存在的进程挂死 by s53329  at  20080301  */
    }
    else
    {
        waitpid(iPid, NULL, 0);
        /*确保消息队列先初始化*/
        sleep(2);
    }
    /* 父进程(cfm)不等待，继续运行 */
}
/***********************************************************
Function:       int tmpfile_writevalue(PS_TMPFILE_CONTENT)
Description:    写文件
Calls:          NULL
Called By:      需要写临时文件的函数
Input:          需要写入的数据名称、长度、值
Output:         NULL
Return:         0		成功
                其他    失败
Others:         NULL

------------------------------------------------------------
实现：
使用消息队列实现，消息处理优先处理写消息
************************************************************/
int tmpfile_writevalue(char *pchName, char *pchValue)
{
    int msgid;
    int iKey;
    S_TMPFILE_MSG sMsg;

    if ((TMPFILE_MAX_ITEM_NAME <= strlen(pchName))
        || (TMPFILE_MAX_ITEM_VALUE <= strlen(pchValue)))
    {
        printf("Input string is too long.\n");
        return -1;
    }

    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    sMsg.lMsgType = TMPFILE_WRITE_OPT;
    strcpy(sMsg.achName, pchName);
    strcpy(sMsg.achValue, pchValue);

    int iRetryTimes = 0;
    iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    if ( -1 == iKey)
    {
        usleep(100000);
        iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    }
    while (-1 == (msgid = msgget(iKey, IPC_CREAT | 0660)))
    {
        iRetryTimes++;
        if (3 <= iRetryTimes)
        {
            printf("Get tmpfile message queue failed.msgid: %d ,Errno: %d\n",msgid,errno);
            return -1;
        }
        printf("Get tmpfile message queue failed,errno %d,try again!\n",errno);
        usleep(100000);
    }
    iRetryTimes = 0;
    while (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), IPC_NOWAIT))
    {
        iRetryTimes++;
        if (10 <= iRetryTimes)
        {
            return -1;
        }
        usleep(100000);
    }
    return 0;
}

/***********************************************************
Function:       int tmpfile_getvalue(PS_TMPFILE_CONTENT)
Description:    读文件
Calls:          NULL
Called By:      需要从临时文件读取数据的函数
Input:          需要读出的数据名称、预计长度
Output:         返回的数据值，实际长度
Return:         0		成功
                其他	失败
Others:         NULL

----------------------------------------------------------------------------
实现：
使用消息队列实现，消息处理优先处理写消息
************************************************************/
int tmpfile_getvalue(char *pchName, char *pchValue, int *piLen)
{
    int msgid;
    int iKey;
    int iRetryTimes;
    int iPid;
    S_TMPFILE_MSG sMsg;
    static int iSerial = 0;

    if (TMPFILE_MAX_ITEM_NAME <= strlen(pchName))
    {
        printf("Input string is too long.\n");
        return -1;
    }

    iRetryTimes = 0;
    iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    if ( -1 == iKey)
    {
        usleep(100000);
        iKey = ftok(TMPFILE_KEY_PATH, TMPFILE_KEY_SEED);
    }
    while (-1 == (msgid = msgget(iKey, IPC_CREAT | 0660)))
    {
        iRetryTimes++;
        if (3 <= iRetryTimes)
        {
            printf("Get tmpfile getmessage queue failed.msgid: %d ,Errno: %d\n",msgid,errno);
            return -1;
        }
        printf("Get tmpfile getmessage queue failed,errno %d,try again!\n",errno);
        usleep(100000);
    }
    
    iPid = getpid();
    
    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    sMsg.lMsgType = TMPFILE_READ_OPT;
    sMsg.iPid = iPid;
    sMsg.iSerial = iSerial;
    iSerial++;
    strcpy(sMsg.achName, pchName);
    iRetryTimes = 0;
    while (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), IPC_NOWAIT))
    {
        iRetryTimes++;
        if (10 <= iRetryTimes)
        {
            return -1;
        }
        usleep(100000);
    }

    /* 接收返回消息 */
    memset(&sMsg, 0, sizeof(S_TMPFILE_MSG));
    iRetryTimes = 0;
    /* 若消息体中序列号与要读取的不一致，再次读取消息 */
    while (1)
    {
        /* 若接收消息失败，重试3次 */
        while (-1 == msgrcv(msgid, &sMsg, sizeof(sMsg) - sizeof(long), iPid, IPC_NOWAIT))
        {
            iRetryTimes++;
            if (10 <= iRetryTimes)
            {
                //perror("tmpfile_getvalue");
                return -1;
            }
            usleep(100000);
        }
        if (sMsg.iSerial == iSerial - 1)
        {
            break;
        }
    }

    if (0 != sMsg.iResult)
    {
        return -1;
    }

    if (*piLen < strlen(sMsg.achValue))
    {
        printf("Value string is too long.\n");
        return -1;
    }
    
    strcpy(pchValue, sMsg.achValue);
    *piLen = strlen(pchValue);

#if (TMPFILE_DEBUG == 1)
    printf("<Lazyjack> serial %d get %s value %s\n", iSerial - 1, pchName, pchValue);
#endif
    return 0;
}
/* end of maintain var目录清理 by liuzhijie 00028714 2006年5月8日" */

/* Start of telnet by c47036 20060905*/
/* telnet攻击时原读取状态的接口会出现问题，暂时用直接读取文件的方法代替 */
int tmpfile_getRadiusStatus(char *name) 
{
    char str[SYS_CMD_LEN];
    FILE* fs = NULL;
    char radius[TMPFILE_MAX_ITEM_NAME] = {0};
    char status[TMPFILE_MAX_ITEM_VALUE] = {0};

    fs = fopen("/var/homegateway.conf", "r");
    if (fs != NULL) 
    {
        while(fgets(str, SYS_CMD_LEN, fs) > 0) 
        {
            sscanf(str, "%s = %s", radius, status);
            if (strcmp(name, radius) == 0)
            {
                fclose(fs);
                return (atoi(status));
            }
        } 
        fclose(fs);     
    }
    //文件读取失败返回0有些不妥
    return 0;
}
/* End of telnet by c47036 20060905*/

#ifdef SUPPORT_TR69C
/* start of maintain tr069消息处理 by liuzhijie 00028714 2006年7月26日 */
void TR069_CreateMsgQueue(void)
{
    int iKey;
    int msgid;
    
    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | IPC_EXCL | 0660);
    if (-1 == msgid) 
    {
        printf("Create message queue failed.\n");
    }
}

int TR069_SendMessage(long msg_type, void *msg_content, int content_len)
{
    int msgid;
    int iKey;
    int iPid;
    S_TR069_MSG sMsg;
  
    if (content_len > TR069_MAX_CONTENT)
    {
        return -1;
    }
    /* start of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日*/
    FILE *pidfile;
    int  tr69c_pid = 0;
    pidfile = fopen(TR69_PID_FILE, "r");
    if (pidfile == NULL) 
    {
        return -1;
    }  
    else
    {
        if (fscanf(pidfile, "%d", &tr69c_pid) != 1) 
        {
           fclose(pidfile);
    	    return -1;
        }
        fclose(pidfile);
    }
    /* end of maintain 不启动tr69c进程时多次从网页上设置tr069相关配置信息会导致网页不能正常访问 by xujunxia 43813 2006年9月13日 */
    
    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    iPid = getpid();
    
    memset(&sMsg, 0, sizeof(S_TR069_MSG));
    sMsg.lMsgType = msg_type;
    if (msg_content != NULL)
    {
        memcpy(sMsg.achContent, msg_content, content_len);
    }
    int iRetryTimes = 0;
    while (-1 == msgsnd(msgid, &sMsg, sizeof(sMsg) - sizeof(long), IPC_NOWAIT))
    {
        iRetryTimes++;
        if (10 <= iRetryTimes)
        {
            printf("TR069_SendMessage send error\n");
            return -1;
        }
        usleep(100000);
    }
    return 0;
}    

int TR069_ReceiveMessage(long *msg_type, void *msg_content, int content_len)
{
    int msgid;
    int iKey;
    S_TR069_MSG sMsg;
    int result;

    if (content_len > TR069_MAX_CONTENT)
    {
        return -1;
    }

    iKey = ftok(TR069_KEY_PATH, TR069_KEY_SEED);
    msgid = msgget(iKey, IPC_CREAT | 0660);
    if (-1 == msgid) 
    {
        printf("Get message queue failed.\n");
        return -1;
    }

    while (-1 == msgrcv(msgid, &sMsg, sizeof(sMsg) - sizeof(long), 0, 0))
    {
        /* retry after 500ms. <tanyin 2009.5.22> */
        usleep(500);
    }
    *msg_type = sMsg.lMsgType;
    memcpy(msg_content, sMsg.achContent, content_len);

    return 0;
}
/* end of maintain tr069消息处理 by liuzhijie 00028714 2006年7月26日 */
#endif

/*
 生成AT模块的配置文件
 格式 : 配置项=配置值
 */
int HW_CreatePppouCfg(char *pszProfile, char *pszPhoneNumber, char *pszAPN, 
                                   unsigned int uOperator, unsigned int uConnType, char * uChannel)
{
    
	FILE* fs = fopen(ATCFG_PROFILE, "w");
	char cmd[1280] = {0};//临时使用的缓冲区
	char val[256] = {0};//临时使用的缓冲区
	
	if(fs)
	{
	    sprintf(cmd, "profile=%s\n", pszProfile ? pszProfile : "");
		fputs(cmd, fs);	
		
	    sprintf(cmd, "phone_number=%s\n", pszPhoneNumber ? pszPhoneNumber : "");
		fputs(cmd, fs);	

	    sprintf(cmd, "ap_name=%s\n", pszAPN ? pszAPN : "");
		fputs(cmd, fs);	

		switch(uOperator)
		{
		case ATCFG_OPTOR_AUTO:
			sprintf(val, "auto");
			break;
		case ATCFG_OPTOR_MANUAL:
			sprintf(val, "manual");			
			break;
		default:
			sprintf(val, "%d", uOperator);	
		}
		sprintf(cmd, "operator=%s\n", val);
		fputs(cmd, fs);	

		switch(uConnType)
		{
		case ATCFG_CONNTYPE_GPRS_FIRST :
			sprintf(val, "gprs_first");
			break;              
		case ATCFG_CONNTYPE_3G_FIRST:
			sprintf(val, "3g_first");
			break;              
		case ATCFG_CONNTYPE_GPRS_ONLY:
			sprintf(val, "gprs_only");
			break;             
		case ATCFG_CONNTYPE_3G_ONLY :
			sprintf(val, "3g_only");
			break;
        case  ATCFG_CONNTYPE_AUTO :
            sprintf(val, "auto");
            break;
		default:
			sprintf(val, "%d", uConnType);
		}
		sprintf(cmd, "conn_type=%s\n", val);
		fputs(cmd, fs);	

		/*

		switch(uChannel)
		{
		
		case ATCFG_CHANNEL_UNLIMITED :
			sprintf(val, "unlimited");
			break;
		case ATCFG_CHANNEL_GSM900_GSM1800_WCDMA2100:
			sprintf(val, "gsm900/gsm1800/wcdma2100");
			break;
		case ATCFG_CHANNEL_GSM1900:
			sprintf(val, "gsm1900");
			break;
		default:
			sprintf(val, "%d", uChannel);
		
    			case ATCFG_CHANNEL_GSM_850:
				sprintf(val, "0");
				break;
			case ATCFG_CHANNEL_GSM_DCS_systems:
				sprintf(val, "1");
				break;
			case ATCFG_CHANNEL_Extended_GSM_900:
				sprintf(val, "2");
				break;
			case ATCFG_CHANNEL_Primary_GSM_900:
				sprintf(val, "3");
				break;
			case ATCFG_CHANNEL_Railway_GSM_900:
				sprintf(val, "4");
				break;
			case ATCFG_CHANNEL_GSM_PCS:
				sprintf(val, "5");
				break;
			case ATCFG_CHANNEL_WCDMA_IMT_2000:
				sprintf(val, "6");
				break;
			case ATCFG_CHANNEL_WCDMA_II_PCS_1900:
				sprintf(val, "7");
				break;
			case ATCFG_CHANNEL_WCDMA_V_850:
				sprintf(val, "8");
				break;
			case ATCFG_CHANNEL_UNLIMITED:
				sprintf(val, "9");
				break;
			default:
				sprintf(val, "10");
				break;
		}
		*/
		sprintf(cmd, "channel=%s\n", uChannel);
		fputs(cmd, fs);	
		
		fclose(fs);
		return FILE_OPEN_OK;
	}
	return FILE_OPEN_ERR;
}

/*只收一遍，用于避免队列中有多余的消息*/
int AtRcvOnce(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    /*start of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
  
    if (-1 == msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        return -1;
    }
    /*end of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    return 0;
}

/*Start of Motive 对接，查询HSPA节点慢，sc&d00104343 , 2008.4.2*/
int AtRcv(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    /*start of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    int iRetryTimes = 0;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
  
    /* 若接收消息失败，重试15次 */
    while (-1 == msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (20 <= iRetryTimes)
        {
            return -1;
        }
        usleep(200000);
    }
    /*end of 修改hspa配置参数阻塞问题 by s53329 at  20080328*/
    return 0;
}
/*end of Motive 对接，查询HSPA节点慢，sc&d00104343 , 2008.4.2*/



/*start of 接收消息失败，最多尝试5次 by w00135351 at  20090105*/
int AtRceive(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    
    int iRetryTimes = 0;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
  
    /* 若接收消息失败，重试5次 */
    while (-1 == msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (5 <= iRetryTimes)
        {
            return -1;
        }
        usleep(200000);
    }
    //fprintf(stderr,"[%s:%d]rcv message success.........\n",__FILE__,__LINE__);
    
    return 0;
}
/*end of 接收消息失败，最多尝试5次 by w00135351 at  20090105*/

#if 0 // uncomment if you need automatically operator selection
int AtRcv_Cops(int lMsgKey, struct stAtRcvMsg *pstAtRcvMsg, int lMsgType)
{
    
    int iRetryTimes = 0;

    memset(pstAtRcvMsg, 0, sizeof(*pstAtRcvMsg));
  
    /* 若接收消息失败，重试5次 */
    while (-1 == msgrcv(lMsgKey, (void*)pstAtRcvMsg, AT_RCV_MSG_LENGTH, lMsgType, IPC_NOWAIT))
    {
        iRetryTimes++;
        if (150 <= iRetryTimes)
        {
            return -1;
        }
        usleep(200000);
    }
    fprintf(stderr,"[%s:%d]rcv message success.........\n",__FILE__,__LINE__);
    fprintf(stderr,"Times is .....%d\n",iRetryTimes);
    
    return 0;
}
#endif



int AtSend(int lMsgKey, struct stAtSndMsg *pstAtSndMsg, char cFrom,  int lMsgType,  const char *pszParam)
{
	memset(pstAtSndMsg, 0, sizeof(*pstAtSndMsg));	
	pstAtSndMsg->lMsgType = lMsgType;
	pstAtSndMsg->acParam[0] = cFrom;
	if(NULL!= pszParam)
		strcpy(pstAtSndMsg->acParam+1, pszParam);
	return msgsnd(lMsgKey, (void*)pstAtSndMsg, AT_SND_MSG_LENGTH, IPC_NOWAIT);
}

/*****************************************************************************
FUNCTION:Kick off dialing on HSPA by setting flag in file under /var.
INPUT:   void
OUTPUT:  NONE
RETURN:  0 success,<0 failed
*****************************************************************************/
int Hspa_StartDial(void)
{
    int lRet = 0;
    char cmd[SYS_CMD_LEN];
    
    printf("[%s:%d]Start HSPA dialing...\n",__FILE__,__LINE__);
    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd, "echo 1 > %s", HSPA_DIAL_FLAG_FILE);
    lRet = bcmSystem(cmd);

    return lRet;
}


/*****************************************************************************
FUNCTION:Release line on HSPA by setting flag in file under /var.
INPUT:   void
OUTPUT:  NONE
RETURN:  0 success,<0 failed
*****************************************************************************/

 /*start of 修改http 页面跳转太慢问题 by s53329  at  20080717 */
void  Hspa_StopDial(int  sig)
{
    char cmd[SYS_CMD_LEN];
    char buffer[SYS_CMD_LEN];
    char pidStr[SYS_CMD_LEN];
    int pid = 0;
    int lRet = -1;
    FILE *fs;
    char  *pcPos= NULL;
    char psList[] = "/var/sdpslist";
    int iVpi = 0;
    int iVci =0;
    int iConId = 0;

    sleep(2);

    bcmSystem("ps > /var/sdpslist");
    fs = fopen(psList, "r");
    while (fgets(buffer, SYS_CMD_LEN, fs) > 0) 
    {
        if ((pcPos = strstr(buffer, "pppd")) != NULL) // found command line with match app name
        {  
             sscanf(pcPos, "pppd -c %d.%d.%d", &iVpi, &iVci, &iConId);
             if (iVpi == WDF_HSPA_DATA_VPI && iVci == WDF_HSPA_DATA_VCI)
             {
                 sscanf(buffer, "%s\n", pidStr);
                 pid = atoi(pidStr);

                 printf("[%s:%d]Send SIGHUP to process pppd [pid: %d]...\n",__FILE__,__LINE__, pid);
                 memset(cmd, 0x00, sizeof(cmd));
                 sprintf(cmd, "echo 0 > %s", HSPA_DIAL_FLAG_FILE);
                 bcmSystem(cmd);
                 memset(cmd, 0x00, sizeof(cmd));
                 sprintf(cmd, "kill -SIGHUP %d", pid);
                 bcmSystem(cmd);
                 lRet = 0;
            }
        }
    }

   fclose(fs);
   return lRet;
}
/*end  of 修改http 页面跳转太慢问题 by s53329  at  20080717 */

/*start of VDF 2008.7.15 V100R001C02B020 j00100803 AU8D00798 AU8D00818 */

void getWanServiceName(char *serviceName, char *ifName) {
   char fileName[IFC_LARGE_LEN];
   char str[SYS_CMD_LEN];
   FILE* fs = NULL;

   if (ifName[0] != 0) {
      sprintf(fileName, "/var/fyi/sys/%s/servicename", ifName);
      fs = fopen(fileName, "r");
      if (fs != NULL) 
	  {
         if (fgets(str, SYS_CMD_LEN, fs) > 0)
         {
            sscanf(str, "%s\n", serviceName);
         }
	     fclose(fs);
      }
   }
}



int bcmIsDataPvc(char * pszWanIfcName)
{

    char szWanServiceName[IFC_LARGE_LEN] = {0};

    if(NULL == pszWanIfcName)
    {		
        return 0;
    }
	
    /*HSPA 是数据接口*/
    if (0 == strcasecmp(pszWanIfcName, WAN_HSPA_INTERFACE))
    {
        //printf("bcmIsDataPvc: %s data ifc\r\n",pszWanIfcName);
        return 1;
    }
	
    getWanServiceName(szWanServiceName, pszWanIfcName);	
    if(0 == strcasecmp(szWanServiceName, WAN_DATA_INTERFACE) ||0== strcasecmp(szWanServiceName, WAN_DATA_VOICE_INTERFACE))
    {
        //printf("bcmIsDataPvc: %s data ifc(%s)\r\n",pszWanIfcName,szWanServiceName);
        return 1;
    }
    else
    {
        return 0;
    }
}
int bcmIsVideoPvc(char * pszWanIfcName)
{
    char szWanServiceName[IFC_LARGE_LEN] = {0};

    if(NULL == pszWanIfcName)
    {		
        return 0;
    }
   
    getWanServiceName(szWanServiceName, pszWanIfcName);	
    if(0 == strcasecmp(szWanServiceName, WAN_VIDEO_INTERFACE))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*end of VDF 2008.7.15 V100R001C02B020 j00100803 AU8D00798 AU8D00818 */
#ifdef SUPPORT_VDF_WANINFO

int hspasysCreateFile(HSPA_SYS_ST *pHspaSysInfo)
{
    FILE* Fs = NULL;
    
    if (pHspaSysInfo)
    {
        Fs = fopen(HSPA_SYSCFG_FILE, "w+");

        if (!Fs)
        {
            printf("FILE %s : hspasys_CreateFile failed, can't open file. \r\n", __FILE__);
            return -1;
        }
/* BEGIN: Added by s00125931, 2008/9/12   问题单号:vhg556*/
        fprintf(Fs, HSPA_SYS_CFG_FORMAT_W, pHspaSysInfo->usConType,
                                         pHspaSysInfo->cBand,
                                         pHspaSysInfo->usVoiceEnable,
                                         pHspaSysInfo->cOperator);
/* END:   Added by s00125931, 2008/9/12 */
        fclose(Fs);
        return 0;
    }
    
    return -1;
}

int SendSigToTtyusb(void)
{
#define TTYPID    "/var/ttypid"

    FILE* fs = NULL;
    int pid = 0;
    fs = fopen(TTYPID, "r");

    if (!fs)
    {
        return -1;
    }

    fscanf(fs, "%d", &pid);
    fclose(fs);

    kill(pid, SIGUSR1);

    return 0;
}


/*****************************************************************************
 函 数 名  : hspasys_init
 功能描述  : 获取hspa配置信息，发送信号给ttyusb通知更新配置
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年5月12日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
void hspasysInit( void )
{
    E220_INFO       stE220Info;
	BCMCFM_VIBINTERFACECFG stVibInfo;
    HSPA_SYS_ST HspaSys;
    HSPA_INFO HspaInfo;

    memset(&stE220Info, 0, sizeof(stE220Info));
    memset(&stVibInfo, 0, sizeof(stVibInfo));
    memset(&HspaSys, 0, sizeof(HspaSys));
    memset(&HspaInfo, 0, sizeof(HspaInfo));

    if(DB_GET_OK != BcmDb_getE220Info(IFC_E220_ID, &stE220Info))
    {
        printf("FILE %s : getE220Info failed. \r\n", __FILE__);
        return;
    }

    if(DB_GET_OK != HWDb_getVDFWanIfCfgInfo(&stVibInfo))
	{
        strcpy(stVibInfo.cBand, "3FFFFFFF");
    }

    if(DB_GET_OK != BcmDb_getHspaInfo(&HspaInfo))
    {
        printf("FILE %s : BcmDb_getHspaInfo failed. \r\n", __FILE__);
        strcpy(HspaInfo.cOperSelect, "auto");
    }

    HspaSys.usConType = stE220Info.usConType;
    HspaSys.usVoiceEnable = stE220Info.usVoiceEnable;
    strncpy(HspaSys.cBand, stVibInfo.cBand, sizeof(HspaSys.cBand)-1);
    strncpy(HspaSys.cOperator, HspaInfo.cOperSelect, sizeof(HspaSys.cOperator)-1);
    if (hspasysCreateFile(&HspaSys))
    {
        printf("FILE %s : hspasys_CreateFile failed. \r\n", __FILE__);
        return;
    }

    if (SendSigToTtyusb())
    {
        printf("FILE %s : send sig failed. \r\n", __FILE__);
        return;
    }

    return;
}
/* BEGIN: Added by y67514, 2008/7/5   问题单号:没拨上号时不能将状态置为开始拨*/
/*****************************************************************************
 函 数 名  : Hspa_Link_up
 功能描述  : 判断数据卡是否插上
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int  1-数据卡插上；0－数据卡没插
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年7月3日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int Hspa_Link_up()
{
    FILE * fd = NULL;

    int ulReadBytes = 0;
    char usbTmp[HSPA_MAX_LINE_LEN];
    memset(usbTmp, 0, HSPA_MAX_LINE_LEN);
	int ulHspaAvailable = 0;
    
    fd = open("/proc/UsbModem", O_RDONLY);
    if (fd < 0)
    {
        printf("Can't open /proc/UsbModem........\n");
        return 0;
    }
    ulReadBytes = read(fd, usbTmp, sizeof(usbTmp));
    if(ulReadBytes == -1)
    {
        printf("Read /proc/UsbModem error.........\n");
        close(fd);
        return 0;
    }
	/* start of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 
		 for 只有识别到2或3或4口才拨号,Usb-Serial Port Num增加一个口变成4个口, 增加这个口为了QXDM透传抓log信息.
		     原来port的作用还是不变:0用来数据,1用来语音,2用来AT指令,3用来QXDM. 
		     E180从11.104.18.00.00版本改成是4口,E169从11.314.17.31.00版本改成4口.*/
    sscanf(usbTmp,HSPA_PORT_NUM_STR,&ulHspaAvailable);
    close(fd);
 
    /*2,3,4口都要能拨*/    
	if ((ulHspaAvailable >= HSPA_MIN_AVALABLE) && (ulHspaAvailable <= HSPA_MAX_AVALABLE))
    {
        return 1;
    }
    else
    {
        return 0;
    }    
	/* end of VOICE 语音驱动组 by pengqiuming(48166), 2008/10/14 */
}
/* END:   Added by y67514, 2008/7/5 */

/*add by z67625 清除防火墙日志 start*/
#define MAX_SIZE 10*1024
void Reset_FwLog(void)
{
    char acBuf[MAX_SIZE];
    
    printf("reset fwlog\n");
    memset(acBuf, 0, sizeof(acBuf));
    sysFirewallLogSet(acBuf, sizeof(acBuf), 0);
}
/*add by z67625 清除防火墙日志 end*/

#define SLEEPEX(seconds, nanoseconds) ({              \
                struct timespec tv;                \
                tv.tv_sec = (seconds);             \
                tv.tv_nsec = (nanoseconds);        \
                while (nanosleep(&tv, &tv) == -1); \
        }) 

/* BEGIN: Added by y67514, 2008/7/23   PN:AU8D00825:将hspa模块中Mobile Connection Type从3G only切换为其他模式时,hspa不能拨号成功*/
/*****************************************************************************
 函 数 名  : Reboot_HSPAStick
 功能描述  : 重启数据卡
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年7月23日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
void Reboot_HSPAStick(void)
{
    int iMsgKeySend;
    int iSleep = 0;
    int time = 0;
    struct stAtSndMsg stSndFunMsg;                      //发送消息(放入发送消息队列) 
    iMsgKeySend = msgget(MSG_AT_QUEUE, 0666);
    //printf("%s:%s:%d:reset the HspaStick!***\n",__FILE__,__FUNCTION__,__LINE__);
    AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=4");
	
#if 0
     iSleep = 2;
    time = iSleep;
    while ( iSleep)
    {
        iSleep = sleep(iSleep);
        if ( 0 == time )
        {
            break;
        }
        time--;
    }
#endif
	
     SLEEPEX(3,1000);
	 
    AtSend(iMsgKeySend, &stSndFunMsg, HTTP_MODULE, CFUN_SET, "AT+CFUN=6");
    SLEEPEX(5,1000);
#if 0
    iSleep = 5;
    time = iSleep;
    while ( iSleep)
    {
        iSleep = sleep(iSleep);
        if ( 0 == time )
        {
            break;
        }
        time--;
    }
#endif
}
/* END:   Added by y67514, 2008/7/23 */
#endif

/* BEGIN: Added by y67514, 2008/9/18   PN:GLB:DNS需求*/
/*****************************************************************************
 函 数 名  : creatDnsCfg
 功能描述  : 创建MAC域名对应关系的临时文件供DHCP使用
 输入参数  : void
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月18日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
#if 0
int creatDnsCfg( void )
{
    FILE *fp = NULL;
    int NumOfDns = 0;
    int i = 0;
    PDNS_CFG pdnsInfo = NULL;
     BcmDb_getDnsdomainSize(&NumOfDns);
    if (0 != NumOfDns)
    {
        pdnsInfo = malloc(sizeof(DNS_CFG)*NumOfDns);
        BcmDb_getDnsDomainInfo(pdnsInfo, sizeof(DNS_CFG)*NumOfDns);
    }
    fp = fopen(DNS_CFG_FILE,"w");
    if ( NULL != fp)
    {
        for ( i = 0 ; i < NumOfDns; i++ )
        {
            fprintf(fp,"%s %s\n",pdnsInfo[i].cMac,pdnsInfo[i].cDnsDomain);
        }
        fclose(fp);
    }
}

/* END:   Added by y67514, 2008/9/18 */

#endif
/* 获取br0的mac地址 */
int bcmGetBrZeroMacAddr(unsigned char * pMacAddr)
{
	int sock;
	struct ifreq ifr;

	if(NULL == pMacAddr)
	{
		printf("\nLine:%d, input param error!\n", __LINE__);
		return -1;
	}
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		printf("\nLine:%d, get br0 mac addr failed!\n", __LINE__);
		return -1;
	}

	strcpy(ifr.ifr_name, "br0");
	/* 获了mac地址 */
	if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		printf("\nLine:%d, ioctl failed!\n", __LINE__);
		return -1;
	}

	memcpy(pMacAddr, ifr.ifr_hwaddr.sa_data, 6);
	return 0;
}

int bcmCodingAdminPwdForCd(char * pInString, int iInLen, char * pOutString, int iOutLen)
{
	int i = 0;
	int iStrLen = 0;
	char * p = NULL;
	char szMac[IFC_LARGE_LEN];
	char szTmp[IFC_LARGE_LEN];

	if((NULL == pInString) || (NULL == pOutString))
	{
		return -1;
	}

	memset(szMac, 0, sizeof(szMac));
	memset(szTmp, 0, sizeof(szTmp));
	strcpy(szMac, pInString);
	iStrLen = strlen(szMac);

	if((iStrLen + 2 > iOutLen) || (iStrLen + 2 > sizeof(szMac)))
	{
		return -1;
	}
	while(i < iStrLen)
	{
		switch(szMac[i])
		{
		case '0':
			szMac[i] = 'f';
			break;
		case '1':
			szMac[i] = '0';
			break;
		case '2':
			szMac[i] = 'e';
			break;
		case '3':
			szMac[i] = '1';
			break;
		case '4':
			szMac[i] = 'd';
			break;
		case '5':
			szMac[i] = '2';
			break;
		case '6':
			szMac[i] = 'c';
			break;
		case '7':
			szMac[i] = '3';
			break;
		case '8':
			szMac[i] = 'b';
			break;
		case '9':
			szMac[i] = '4';
			break;
		case 'a':
		case 'A':
			szMac[i] = 'a';
			break;
		case 'b':
		case 'B':
			szMac[i] = '5';
			break;
		case 'c':
		case 'C':
			szMac[i] = '9';
			break;
		case 'd':
		case 'D':
			szMac[i] = '6';
			break;
		case 'e':
		case 'E':
			szMac[i] = '8';
			break;
		case 'f':
		case 'F':
			szMac[i] = '7';
			break;
		default:
			szMac[i] = 'x';
			break;
		}
		i ++;
	}
	
	strcpy(szTmp, szMac);
	strcpy(szMac + 1, szTmp);
	iStrLen = strlen(szMac);
	szMac[iStrLen] = '0';
	szMac[iStrLen + 1] = '\0';

	strncpy(pOutString, szMac, iOutLen);
	
	return 0;
}

int bcmGetWlanChipType(int bMute)
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

        if (!bMute)
        {
            printf("Sean@WLAN CHIP Vendor=<%s>, ProductId=<0x%x>\n", acVendorStr[iVendorType], iVendorProduct & 0xFFFF);
        }
        
    }

    fclose(fp);

    return iVendorType;
}

/* BEGIN: Added by y67514, 2009/12/11   PN:APNSecurity Enhancement*/
int base64Encode(const void *inp, int insize, char **outptr);

void decodeBlock(unsigned char *dest, char *src)
{
  unsigned int x = 0;
  int i;
  for(i = 0; i < 4; i++) {
    if(src[i] >= 'A' && src[i] <= 'Z')
      x = (x << 6) + (unsigned int)(src[i] - 'A' + 0);
    else if(src[i] >= 'a' && src[i] <= 'z')
      x = (x << 6) + (unsigned int)(src[i] - 'a' + 26);
    else if(src[i] >= '0' && src[i] <= '9') 
      x = (x << 6) + (unsigned int)(src[i] - '0' + 52);
    else if(src[i] == '+')
      x = (x << 6) + 62;
    else if(src[i] == '/')
      x = (x << 6) + 63;
    else if(src[i] == '=')
      x = (x << 6);
  }

  dest[2] = (unsigned char)(x & 255); x >>= 8;
  dest[1] = (unsigned char)(x & 255); x >>= 8;
  dest[0] = (unsigned char)(x & 255); x >>= 8;
}

/* base64Decode
 * Given a base64 string at src, decode it into the memory pointed
 * to by dest. If rawLength points to a valid address (ie not NULL),
 * store the length of the decoded data to it.
 */
void base64Decode(unsigned char *dest, char *src, int *rawLength)
{
  int length = 0;
  int equalsTerm = 0;
  int i;
  int numBlocks;
  unsigned char lastBlock[3];
	
  while((src[length] != '=') && src[length])
    length++;
  while(src[length+equalsTerm] == '=')
    equalsTerm++;
  
  numBlocks = (length + equalsTerm) / 4;
  if(rawLength)
    *rawLength = (numBlocks * 3) - equalsTerm;
  
  for(i = 0; i < numBlocks - 1; i++) {
    decodeBlock(dest, src);
    dest += 3; src += 4;
  }

  decodeBlock(lastBlock, src);
  for(i = 0; i < 3 - equalsTerm; i++)
    dest[i] = lastBlock[i];
	
}

/* ---- Base64 Encoding --- */
static char table64[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  
/*
 * base64Encode()
 *
 * Returns the length of the newly created base64 string. The third argument
 * is a pointer to an allocated area holding the base64 data. If something
 * went wrong, -1 is returned.
 *
 */
int base64Encode(const void *inp, int insize, char **outptr)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;
  char *output;
  char *base64data;

  char *indata = (char *)inp;

  if(0 == insize)
    insize = strlen(indata);

  base64data = output = (char*)malloc(insize*4/3+4);
  if(NULL == output)
    return -1;
  /* start of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  memset(output, 0, insize*4/3+4);
  /* end of maintain PSI移植:  分配内存之后应该置空 by xujunxia 43813 2006年5月5日 */
  
  while(insize > 0) {
    for (i = inputparts = 0; i < 3; i++) { 
      if(insize > 0) {
        inputparts++;
        ibuf[i] = *indata;
        indata++;
        insize--;
      }
      else
        ibuf[i] = 0;
    }
                       
    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;

    switch(inputparts) {
    case 1: /* only one byte read */
      sprintf(output, "%c%c==", 
              table64[obuf[0]],
              table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      sprintf(output, "%c%c%c=", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]]);
      break;
    default:
      sprintf(output, "%c%c%c%c", 
              table64[obuf[0]],
              table64[obuf[1]],
              table64[obuf[2]],
              table64[obuf[3]] );
      break;
    }
    output += 4;
  }
  *output=0;
  *outptr = base64data; /* make it return the actual data memory */

  return strlen(base64data); /* return the length of the new data */
}

/*****************************************************************************
 函 数 名  : DecodeStr
 功能描述  : 用base64解密
 输入参数  : char * EncodedStr  
             char * DecodedStr  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月30日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
void DecodeStr( char * EncodedStr ,char * DecodedStr)
{
    int iStrLen = 0;
    int ret = FALSE;

    base64Decode(DecodedStr, EncodedStr, &iStrLen);
    DecodedStr[iStrLen] = '\0';
}

/*****************************************************************************
 函 数 名  : HspaSetApn
 功能描述  : 将apn的密文内容写入flash
 输入参数  : char * cApn
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetEncodedApn( char * cEnCodedApn )
{
    int ret = -1;
    if ( NULL == cEnCodedApn )
    {
        return FALSE;
    }

    if ( strlen(cEnCodedApn) > HSPA_APN_LEN )
    {
        return FALSE;
    }

    ret = sysFixSet(cEnCodedApn, HSPA_APN_LEN, HSPA_PARAMETERS_OFFSET);
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*****************************************************************************
 函 数 名  : HspaSetApn
 功能描述  : 设置apn的值到flash
 输入参数  : char* cApn
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetApn( char* cApn )
{
    int ret = FALSE;
    char * cEnCodedApn = NULL;
    if ( NULL == cApn )
    {
        return FALSE;
    }
    if ( strlen(cApn) > PPP_PROFILE_APN_LEN )
    {
        return FALSE;
    }

    if ( -1 == base64Encode(cApn, strlen(cApn), &cEnCodedApn) )
    {
        if ( cEnCodedApn)
        {
            free(cEnCodedApn);
        }
        return FALSE;
    }
    
    ret = HspaSetEncodedApn(cEnCodedApn);
    free(cEnCodedApn);

    return ret;
}


/*****************************************************************************
 函 数 名  : HspaGetApn
 功能描述  : 从flash中获取apn的密文内容
 输入参数  : char * cApn
 输出参数  : 无
 返 回 值  : INT
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaGetEncodedApn( char * cEnCodedApn )
{
    int ret = -1;
    if ( NULL == cEnCodedApn )
    {
        return FALSE;
    }

    ret = sysFixGet(cEnCodedApn, HSPA_APN_LEN,HSPA_PARAMETERS_OFFSET );
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}

/*****************************************************************************
 函 数 名  : HspaGetApn
 功能描述  : 从flash中获取apn的值
 输入参数  : char * cApn
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaGetApn( char * cApn )
{
    int iApnLen = 0;
    int ret = FALSE;
    char acEncodedApn[HSPA_APN_LEN];

    ret = HspaGetEncodedApn(acEncodedApn);
    if ( TRUE == ret  )
    {
        base64Decode(cApn, acEncodedApn, &iApnLen);
        cApn[iApnLen] = '\0';
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : HspaSetEncodedUserName
 功能描述  : 将hspa用户名的密文写入flash
 输入参数  : char * cEnCodedUserName  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetEncodedUserName( char * cEnCodedUserName )
{
    int ret = -1;
    if ( NULL == cEnCodedUserName )
    {
        return FALSE;
    }

    if ( strlen(cEnCodedUserName) > HSPA_USER_NAME_LEN)
    {
        return FALSE;
    }

    ret = sysFixSet(cEnCodedUserName, HSPA_USER_NAME_LEN, HSPA_PARAMETERS_OFFSET + HSPA_APN_LEN);
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*****************************************************************************
 函 数 名  : HspaSetUserName
 功能描述  : 将hspa用户名写入flash
 输入参数  : char* cUserName  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetUserName( char* cUserName )
{
    int ret = FALSE;
    char * cEnCodedUserName = NULL;
    if ( NULL == cUserName )
    {
        return FALSE;
    }
    if ( strlen(cUserName) > PPP_PROFILE_USERNAME_LEN)
    {
        return FALSE;
    }

    if ( -1 == base64Encode(cUserName, strlen(cUserName), &cEnCodedUserName) )
    {
        if ( cEnCodedUserName)
        {
            free(cEnCodedUserName);
        }
        return FALSE;
    }
    
    ret = HspaSetEncodedUserName(cEnCodedUserName);
    free(cEnCodedUserName);

    return ret;
}

/*****************************************************************************
 函 数 名  : HspaGetEncodedUserName
 功能描述  : 从flash中读取hspa用户名的密文
 输入参数  : char * cEnCodedUserName  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaGetEncodedUserName( char * cEnCodedUserName )
{
    int ret = -1;
    if ( NULL == cEnCodedUserName )
    {
        return FALSE;
    }

    ret = sysFixGet(cEnCodedUserName, HSPA_USER_NAME_LEN,HSPA_PARAMETERS_OFFSET + HSPA_APN_LEN);
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}

/*****************************************************************************
 函 数 名  : HspaGetUserName
 功能描述  : 从flash中获取hspa用户名
 输入参数  : char * cUserName  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaGetUserName( char * cUserName )
{
    int iUserNameLen = 0;
    int ret = FALSE;
    char acEncodedUserName[HSPA_USER_NAME_LEN];

    ret = HspaGetEncodedUserName(acEncodedUserName);
    if ( TRUE == ret  )
    {
        base64Decode(cUserName, acEncodedUserName, &iUserNameLen);
        cUserName[iUserNameLen] = '\0';
    }
    return ret;
}


/*****************************************************************************
 函 数 名  : HspaSetEncodedPwd
 功能描述  : 将hspa密码的密文设置到flash中
 输入参数  : char * cEnCodedPwd  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetEncodedPwd( char * cEnCodedPwd )
{
    int ret = -1;
    if ( NULL == cEnCodedPwd )
    {
        return FALSE;
    }

    if ( strlen(cEnCodedPwd) > HSPA_PWD_LEN)
    {
        return FALSE;
    }

    ret = sysFixSet(cEnCodedPwd, HSPA_PWD_LEN, HSPA_PARAMETERS_OFFSET + HSPA_APN_LEN + HSPA_USER_NAME_LEN);
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


/*****************************************************************************
 函 数 名  : HspaSetPwd
 功能描述  : 保持hspa密码到flash
 输入参数  : char* cPwd  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaSetPwd( char* cPwd )
{
    int ret = FALSE;
    char * cEnCodedPwd = NULL;
    if ( NULL == cPwd )
    {
        return FALSE;
    }
    if ( strlen(cPwd) > PPP_PROFILE_PASSWORD_LEN)
    {
        return FALSE;
    }

    if ( -1 == base64Encode(cPwd, strlen(cPwd), &cEnCodedPwd) )
    {
        if ( cEnCodedPwd)
        {
            free(cEnCodedPwd);
        }
        return FALSE;
    }
    
    ret = HspaSetEncodedPwd(cEnCodedPwd);
    free(cEnCodedPwd);

    return ret;
}

/*****************************************************************************
 函 数 名  : HspaGetEncodedPwd
 功能描述  : 从flash中获取hspa密码的密文
 输入参数  : char * cEnCodedPwd  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
int HspaGetEncodedPwd( char * cEnCodedPwd )
{
    int ret = -1;
    if ( NULL == cEnCodedPwd )
    {
        return FALSE;
    }

    ret = sysFixGet(cEnCodedPwd, HSPA_PWD_LEN,HSPA_PARAMETERS_OFFSET + HSPA_APN_LEN + HSPA_USER_NAME_LEN);
    if ( -1 == ret )
    {
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}

int HspaGetPwd( char * cPwd )
{
    int iPwdLen = 0;
    int ret = FALSE;
    char acEncodedPwd[HSPA_PWD_LEN];

    ret = HspaGetEncodedPwd(acEncodedPwd);
    if ( TRUE == ret  )
    {
        base64Decode(cPwd, acEncodedPwd, &iPwdLen);
        cPwd[iPwdLen] = '\0';
    }
    return ret;
}
/* END:   Added by y67514, 2009/12/11 */
