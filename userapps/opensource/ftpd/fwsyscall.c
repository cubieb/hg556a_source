/*****************************************************************************
//
//  Copyright (c) 2000-2003  Broadcom Corporation
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
//  Filename:       fwsyscall.c
//  Author:         seanl
//  Creation Date:  10/10/03
//                  The system call for kill process and firmware flashing 
//                  functions (from syscall.c and dltftp.c)
//
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <net/if.h>
#include <ctype.h>

#include "fwsyscall.h"
#define  BCMTAG_EXE_USE
#include "bcmTag.h"

extern char **environ;
char glbIfName[IFC_NAME_LEN];

#define READ_BUF_SIZE       128
#define CMD_LEN             256


/***************************************************************************
// Function Name: bcmSocketIfPid().
// Description  : return the socket if pid
// Parameters   : none.
// Returns      : pid
****************************************************************************/
int bcmSocketIfPid(void)
{
   int pid = 0;
   char buffer[CMD_LEN];
   char cmd[CMD_LEN];
   FILE *fs;

   if (glbIfName[0] == '\0')
      return pid;    

   sprintf(cmd, "/proc/var/fyi/wan/%s/pid", glbIfName);
   if ((fs = fopen(cmd, "r")) != NULL)
   {
      fgets(cmd, SYS_CMD_LEN, fs);
      pid = atoi(cmd);
      fclose(fs);
   }

   if (strstr(glbIfName, "ppp") != NULL)     // only do it if it is ppp
   {
      // add the default route with the ifc the socket is sitting on
      sprintf(buffer, "route add default dev %s", glbIfName);
      bcmSystem(buffer);
   }

   return pid;
}

/***************************************************************************
// Function Name: killAllApps().
// Description  : kill all available applications.
// Parameters   : socketPid    - if none 0, the pid for the interface the applicat
//							     is using.
//                skipApp      - check if tftp is on telnet or sshd and skip the app kill
//                telnetIfcPid, ssshdIfcPid  - the pid of the interface telnet or sshd is on
// Returns      : none.
****************************************************************************/
void killAllApps(int socketPid, int skipApp, int telnetIfcPid, int sshdIfcPid) {
   // NOTE: Add the app name before NULL. Order is important
   char *apps[]=
   {
      "telnetd",        // NOTE: has to be the first for tftp client (no kill)
      "sshd",           //       and sshd on the second (will not kill if sshd exist)
      /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月24日
      为了能在升级时弹出警告窗口，不杀掉httpd
      //       and httpd on the third -- will not kill if update xml file (processed in httpd)
      "httpd",          
      end of maintain Auto Upgrade by zhangliang 60003055 2006年5月24日 */
      "bftpd",
      "tftpd",
      "snmp",
      "upnp",
      "sntp",
      "ddnsd",
      "reaim",
      "klogd",
      "syslogd",
      "ripd",
      "zebra ",
      /* start of maintain Auto Upgrade 升级时不能断掉ppp连接 by zhangliang 60003055 2006年5月8日"
      "pppd",
      end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
      "dnsprobe",
      "dhcpc",
      "igmp", 
      "radiusd",
      NULL,
   };
   char cmd[CMD_LEN], app[CMD_LEN];
   char buffer[CMD_LEN];
   char pidStr[CMD_LEN];
   int pid = 0;
   int i = 0;
   FILE *fs;
   int curPid = getpid();
   char psList[] = "/var/ftpdpslist";

   bcmSystem("ps > /var/ftpdpslist");
   fs = fopen(psList, "r");
   //bcmSystem("cat /var/pslist");
   

   // get ps line and parse for the pid for killing
   do 
   {
      if (i == 0 && skipApp & SKIP_TELNETD_APP)
          continue;
      if (i == 1 && skipApp & SKIP_SSHD_APP)
          continue;
      if (i == 2 && skipApp & SKIP_HTTPD_APP)
          continue;
      strcpy(app, apps[i]);
      while (fgets(buffer, CMD_LEN, fs) > 0) 
      {
                  
         if (strstr(buffer, app) != NULL) // found command line with match app name
         {  
            // find pid string
            sscanf(buffer, "%s\n", pidStr);
            pid = atoi(pidStr);
            if (pid != curPid && pid != socketPid && pid != telnetIfcPid && pid !=sshdIfcPid)
            {
               printf("kill process [pid: %d] [name: %s]...\n", pid, app);
               sprintf(cmd, "kill -9 %d", pid);
               bcmSystem(cmd);
            }
         }
      }
      rewind(fs);
   } while (apps[++i] != NULL);

   fclose(fs);
   unlink(psList);
   bcmRemoveModules(socketPid);

}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}


/***************************************************************************
// Function Name: verifyTag
// Description  : check if the image is brcm image or not. If passNumber == 1,
//                just check the tag crc for memory allocation.  If passNumber == 2, 
//                check the image crc and tagVersion.
// Parameters   : pTag and passNumber.
// Returns      : UPLOAD_OK or UPLOAD_FAIL_ILLEGAL_IMAGE
****************************************************************************/
UPLOAD_RESULT verifyTag(PFILE_TAG pTag, int passNumber)
{
    UINT32 crc;
    UPLOAD_RESULT status = UPLOAD_OK;
    int totalImageSize;
    int tagVer, curVer, tagVerLast;

    // check tag validate token first
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((char*) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);

    if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
    {
        printf("Not regular image file\n");
        return UPLOAD_FAIL_ILLEGAL_IMAGE;
    }
    if (passNumber == 1)
        return status;

    // if second pass, check imageCrc and tagVersion
    totalImageSize = atoi(pTag->totalImageLen);
    crc = CRC32_INIT_VALUE;
    crc = getCrc32(((char*)pTag + TAG_LEN), (UINT32) totalImageSize, crc);      
    if (crc != (UINT32) (*(UINT32*)(pTag->imageValidationToken)))
        return UPLOAD_FAIL_ILLEGAL_IMAGE;

    // only allow same or greater tag versions (up to tagVerLast - 26) to be uploaded
    tagVer = atoi(pTag->tagVersion);
    curVer = atoi(BCM_TAG_VER);
    tagVerLast = atoi(BCM_TAG_VER_LAST);

    if (tagVer < curVer || tagVer > tagVerLast)
    {
       printf("Firmware tag version [%d] is not compatible with the current Tag version [%d]\n", tagVer, curVer);
       return UPLOAD_FAIL_ILLEGAL_IMAGE;
    }

    return status;
}



/***************************************************************************
// Function Name: psiVerify
// Description  : Verify whether it is a kind of valid xml file by just check <psitree> </psitree>
// Parameters   : psiBuf.
// Returns      : PSI_STS_OK -- psi kind of verified -- need xmlparse in cfm.
****************************************************************************/

PSI_STATUS psiVerify(char *psiBuf, int imageSize) {

   if (imageSize > sysGetPsiSize())
       return PSI_STS_ERR_INVALID_LENGTH;
 
   // if <psitree>  </psitree> cannot be found then return
   if ((memcmp(psiBuf, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) != 0) || !(strstr(psiBuf, XML_PSI_END_TAG)))
       return PSI_STS_ERR_GENERAL;

   return PSI_STS_OK;
}

// create the xml file and signal httpd to process it
// return: UPLOAD_RESULT
UPLOAD_RESULT createXmlAndSignalUser(char *imagePtr, int imageLen)
{
    char xmlFileName[] = "/var/psi_rcv.xml";
    char httpPidFile[] = "/var/run/httpd_pid";
    FILE *fp = NULL;
    int httpdPid =0;
    char cmd[CMD_LEN];

    if (access(xmlFileName, F_OK) == 0) 
        unlink(xmlFileName);
    if ((fp = fopen(xmlFileName, "w")) == NULL)
    {
        printf("Failed to create %s\n", xmlFileName);
        return UPLOAD_FAIL_FTP;
    }
       
    if (fwrite(imagePtr, 1, imageLen, fp) != imageLen)
    {
        printf("Failed to write %s\n", xmlFileName);
        fclose(fp);
        return UPLOAD_FAIL_FTP;
    }    
    fclose(fp);
    
    // need to signal httpd to process this xml file
    if ((fp = fopen(httpPidFile, "r")) == NULL)
    {
        printf("Failed to read %s\n", httpPidFile);
        return UPLOAD_FAIL_FTP;
    }
    else 
    {
        fgets(cmd, CMD_LEN, fp);
        httpdPid = atoi(cmd);
        fclose(fp);
    }

    if (httpdPid > 0)
    {
        kill(httpdPid, SIGUSR1);
        return UPLOAD_OK;
    }
    else
        return UPLOAD_FAIL_FTP;
}


// depending on the image type, do the brcm image or whole flash image and the configuration data
// return: UPLOAD_RESULT
UPLOAD_RESULT flashImage(char *imagePtr, PARSE_RESULT imageType, int imageLen)
{
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int cfeSize, rootfsSize, kernelSize;
    unsigned long cfeAddr, rootfsAddr, kernelAddr;
    UPLOAD_RESULT status = UPLOAD_OK;
    PSI_STATUS psiStatus = PSI_STS_OK;

    switch (imageType) 
    {
    case PSI_IMAGE_FORMAT:
        if (imageLen > sysGetPsiSize() || (psiStatus = psiVerify(imagePtr, imageLen)) != PSI_STS_OK)
        {
            printf("Failed to verify configuration data\n");
            status = UPLOAD_FAIL_FLASH;
        }
        if ((status = createXmlAndSignalUser(imagePtr, imageLen)) == UPLOAD_OK)
           return status;
        else
           printf("\nFailed to create configuration file and signal user - error[%d]\n", (int) status);
        break;

    case FLASH_IMAGE_FORMAT:
        printf("\nFlash whole image...");
        // Pass zero for the base address of whole image flash. It will be filled by kernel code
        if ((sysFlashImageSet(imagePtr, imageLen-TOKEN_LEN, 0, BCM_IMAGE_WHOLE)) == -1)
        {
            printf("Failed to flash whole image\n");
            status = UPLOAD_FAIL_FLASH;
        }
        break;

    case BROADCOM_IMAGE_FORMAT:
        cfeSize = rootfsSize = kernelSize = 0;
        // must be tagged Broadcom image.  Check cfe's existence
        cfeAddr = (unsigned long) strtoul(pTag->cfeAddress, NULL, 10);
        cfeSize = atoi(pTag->cfeLen);
        // check kernel existence
        kernelAddr = (unsigned long) strtoul(pTag->kernelAddress, NULL, 10);
        kernelSize = atoi(pTag->kernelLen);
        // check root filesystem existence
        rootfsAddr = (unsigned long) strtoul(pTag->rootfsAddress, NULL, 10);
        rootfsSize = atoi(pTag->rootfsLen);
        if (cfeAddr) 
        {
           printf("\nFlashing CFE...\n");
           if ((sysFlashImageSet(imagePtr+TAG_LEN, cfeSize, (int) cfeAddr,
                 BCM_IMAGE_CFE)) == -1)
           {
                 printf("Failed to flash CFE\n");
                 status = UPLOAD_FAIL_FLASH;
           }
        }
        if (rootfsAddr && kernelAddr) 
        {
           char *tagFs = imagePtr;
           // tag is alway at the sector start of fs
           if (cfeAddr)
           {
               tagFs = imagePtr + cfeSize;       // will trash cfe memory, but cfe is already flashed
               memcpy(tagFs, imagePtr, TAG_LEN);
           }
           printf("Flashing root file system and kernel...\n");
           if ((sysFlashImageSet(tagFs, TAG_LEN+rootfsSize+kernelSize, 
              (int)(rootfsAddr-TAG_LEN), BCM_IMAGE_FS)) == -1)      
           {
               printf("Failed to flash root file system\n");
               status = UPLOAD_FAIL_FLASH;
           }
        }
        break;

    default:
        printf("Illegal image data\n");
        break;
    }

    if (status != UPLOAD_OK)
        printf("**** IMAGE FLASH FAILED ****\n");
    else 
        printf("Image flash done.\n");
    
    return status;
}


// Check for configuration data
PARSE_RESULT checkConfigData(char *image_start_ptr, int imageSize)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;
    PSI_STATUS psiResult = PSI_STS_ERR_GENERAL;

    if ((psiResult = psiVerify(image_start_ptr, imageSize)) == PSI_STS_OK) 
    {
        printf("Could be configuration data...\n");
	    result = PSI_IMAGE_FORMAT;
    }
    return result;
}

// Check Broadcom image format
PARSE_RESULT checkBrcmImage(char *image_start_ptr)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;

    /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日"
    if (verifyTag((FILE_TAG *) image_start_ptr, 2) == UPLOAD_OK)
    {
        printf("Broadcom format verified.\n");
        result = BROADCOM_IMAGE_FORMAT;
    }
    */
    PFILE_TAG filetag_ptr = (PFILE_TAG)image_start_ptr;
    if (verifyTag((FILE_TAG *) image_start_ptr, 2) == UPLOAD_OK)
    {
        printf("CRC check verified.\n");
//        printf("Leo_DBG:%s\n", filetag_ptr->productName);
        if (0 == strcmp(filetag_ptr->productName, HGW_PRODUCTNAME))
        {
            printf("Broadcom format verified.\n");
            result = BROADCOM_IMAGE_FORMAT;
        }
        else
        {
            printf("Product type not matched!\n");
        }
    }
    /* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
    return result;
}


// Check flash image format
PARSE_RESULT checkFlashImage(char *image_start_ptr, int bufSize)
{
    unsigned char *image_ptr = image_start_ptr;
    unsigned long image_len = bufSize - TOKEN_LEN;
    unsigned long crc = CRC32_INIT_VALUE;
    PARSE_RESULT result = NO_IMAGE_FORMAT;
    
    /* start of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日"
    crc = getCrc32(image_ptr, image_len, crc);      
    if (memcmp(&crc, image_ptr + image_len, CRC_LEN) == 0)
    {
        printf("Flash image format verified.\n");
        result = FLASH_IMAGE_FORMAT;
    }
    */
    // FLASH 镜像前端有64kB 用来保存CFE 程序,需要跳过这段再获取文件头信息
    PFILE_TAG filetag_ptr = (PFILE_TAG)(image_start_ptr + 64*1024);
    crc = getCrc32(image_ptr, image_len, crc);      
    if (memcmp(&crc, image_ptr + image_len, CRC_LEN) == 0)
    {
        printf("CRC check verified.\n");
//        printf("Leo_DBG:%s\n", filetag_ptr->productName);
        if (0 == strcmp(filetag_ptr->productName, HGW_PRODUCTNAME))
        {
            printf("Flash image format verified.\n");
            result = FLASH_IMAGE_FORMAT;
        }
        else
        {
            printf("Product type not matched!\n");
        }
    }
    /* end of maintain Auto Upgrade by zhangliang 60003055 2006年5月8日" */
    return result;
}


// parseImageData
// parse the image data to see if it Broadcom flash format or flash image format file or psi data
// fBufType definition: BUF_ALL_TYPES:   check all types (2 images and psi)
//                      BUF_IMAGES:      check images only
//                      BUF_CONFIG_DATA: check psi only
// return: NO_IMAGE_FORMAT  
//         BROADCOM_IMAGE_FORMAT       
//         FLASH_IMAGE_FORMAT          
//         PSI_IMAGE_FORMAT

PARSE_RESULT parseImageData(char *image_start_ptr, int bufSize, BUFFER_TYPE fBufType)
{
    PARSE_RESULT result = NO_IMAGE_FORMAT;

    switch (fBufType) 
    {
    case BUF_ALL_TYPES:
        if ((result = checkBrcmImage(image_start_ptr)) == BROADCOM_IMAGE_FORMAT)
            break;
        if ((result = checkFlashImage(image_start_ptr, bufSize)) == FLASH_IMAGE_FORMAT)
            break;
        if ((result = checkConfigData(image_start_ptr, bufSize)) == PSI_IMAGE_FORMAT)
            break;

    case BUF_IMAGES:
        if ((result = checkBrcmImage(image_start_ptr)) == BROADCOM_IMAGE_FORMAT)
            break;
        if ((result = checkFlashImage(image_start_ptr, bufSize)) == FLASH_IMAGE_FORMAT)
            break;
    case BUF_CONFIG_DATA:
        result = checkConfigData(image_start_ptr, bufSize);
        break;
    
    default:
        printf("Illegal buffer type?\n");
        break;
    }

    return result;
}


/***************************************************************************
// Function Name: BcmScm_isServiceEnabled.
// Description  : check the given service is enabled.
// Parameters   : name -- the given service name.
//                where -- local or remote.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmScm_isServiceEnabled(char *name, int where) {
   int enbl = FALSE, len = 0, lan = 0, wan = 0;
   char line[CLI_MAX_BUF_SZ];
   FILE* fs = fopen("/var/services.conf", "r");

   // if list is empty then return default which is
   // return true if access from lan and
   // return false if access from wan
   if ( fs == NULL ) {
      if ( where == CLI_ACCESS_LOCAL )
         return TRUE;
      else
         return FALSE;
   }

   len = strlen(name);

   while ( fscanf(fs, "%s\t%d\t%d", line, &lan, &wan) != EOF )
      if ( strncasecmp(line, name, len) == 0 ) {
         if ( where == CLI_ACCESS_LOCAL )
            enbl = lan;
         else if ( where == CLI_ACCESS_REMOTE )
            enbl = wan;
         break;
      }

   fclose(fs);

   return enbl;
}


/***************************************************************************
// Function Name: BcmScm_isInAccessControlList.
// Description  : check the given IP address in the access control list.
// Parameters   : addr -- the given IP address.
// Returns      : TRUE or FALSE.
****************************************************************************/
int BcmScm_isInAccessControlList(char * addr) {
   int in = TRUE, len = 0;
   char line[SEC_BUFF_MAX_LEN];
   FILE* fs = fopen("/var/acl.conf", "r");

    /* add by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  begin*/
    char *pszIPEnd = NULL;
    struct in_addr stIPStart = {0L};
    struct in_addr stIPEnd = {0L};
    struct in_addr stIP = {0L};  
    /* add by sxg (60000658), ACL IP地址支持按网段设置,2006/02/20,  end*/
   // if list is empty then return true
   // since allow to access for all by default
   if ( fs == NULL )
      return in;

   // get access control mode
   fgets(line, SEC_BUFF_MAX_LEN, fs);

   // if mode is disabled (zero) then return true
   // since allow to access for all by default
   if ( atoi(line) == 0 ) {
      fclose(fs);
      return in;
   }

   // mode is enabled then access control list is active
   // so in should be initialized to false
   in = FALSE;
   len = strlen(addr);

   while ( fgets(line, SEC_BUFF_MAX_LEN, fs) ){
    /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/22,  begin*/
        /*只定义起始地址*/
        if (NULL == (pszIPEnd = strstr(line, "-")))
        {
            if(inet_aton( line, &stIPStart ) 
                && inet_aton( addr, &stIP ))
            {
                if ((stIP.s_addr) == (stIPStart.s_addr))
                {
                    in = TRUE;
                    break;
                }
            }
        }
        else/*起始地址和结束地址都存在*/
        {
            *pszIPEnd = '\0';
            if (inet_aton( line, &stIPStart ) 
                && inet_aton( pszIPEnd + 1, &stIPEnd ) 
                && inet_aton( addr, &stIP ))
            {                
                if ( ((stIPEnd.s_addr <= stIP.s_addr) && (stIP.s_addr <= stIPStart.s_addr))
                    ||((stIPEnd.s_addr >= stIP.s_addr) && (stIP.s_addr >= stIPStart.s_addr)))
                {
                    in = TRUE;
                    *pszIPEnd = '-';
                    break;
                }
            }
            
            *pszIPEnd = '-';
        }
        /* modify by sxg (60000658), ACL IP地址支持按网段设置,2006/02/22,  end*/
    }
   fclose(fs);

   return in;
}

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
   // retrieve PSI from flash to make sure it's up-to-date
   //BcmPsi_profileRetrieve(0);

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
//start to modify by zhourongfei kf34753 at 2010.11.23
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, begin*/
      //else if ( BcmScm_isInAccessControlList(inet_ntoa(clntAddr)) == FALSE )
          //return CLI_ACCESS_DISABLED;
      /*add by sxg(60000658) ACL IP 仅对wan侧进行限制, 2006/02/22, end*/
//end to modify by zhourongfei kf34753 at 2010.11.23
      else
         return CLI_ACCESS_REMOTE;
   }
}


