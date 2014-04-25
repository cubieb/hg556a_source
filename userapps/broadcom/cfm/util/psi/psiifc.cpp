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
//  Filename:       psiifc.cpp
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
//  Description:
//      Implement the PsiIfc class and functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "psiifc.h"
#include "psiutil.h"
#include "board_api.h"
#include "version.h"
#include "psixml.h"
#include "psidl.h"
#include "bget.h"

static void* m_glbShm = NULL;

/***************************************************************************
// Function Name: initSharedData
// Description  : allocate shared data memory, and create memory pool.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS initSharedData(void) {
   // shared memory key
   const long KEY_ID = 0x12344321;
   // memory buffer needs to me larger than xml
   // xml PSI is more compact. As data have variable size, no space is wasted 
   // * 2 for CFM API memory allocations
   int len = (sysGetPsiSize()*PSI_BUFFER_TO_XML_FACTOR) * 2;
   int shmid = shmget(KEY_ID, len, IPC_CREAT | 0777);
   PSI_STATUS status = PSI_STS_OK;   

   if ( shmid == -1 ) {
      perror("Failed to get shared memory - shmget():");
      return status;
   }
   
   m_glbShm = shmat(shmid, NULL, 0);
   if ( m_glbShm == NULL ) {
      perror("Failed to attach shared memory - shmat():");
      return status;
   }

   // create shared memory pool
   bpool(m_glbShm, len);

   return status;   
}

/***************************************************************************
// Function Name: unInitSharedData
// Description  : deallocate shared data memory.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void unInitSharedData(void) {   
   if ( m_glbShm != NULL ) {
      shmdt(m_glbShm);
      m_glbShm = NULL;
   }
}

/************************* Static Local Variables ***************************/

PsiIfc* PsiIfc::m_instance = NULL;

/********************* Static Method Implementations ************************/

/***************************************************************************
// Function Name: getInstance().
// Description  : create only one instance of PsiIfc.
// Parameters   : none.
// Returns      : instance of PsiIfc.
****************************************************************************/
PsiIfc* PsiIfc::getInstance() {
   if ( m_instance == NULL ) {
      // allocate shared data memory, and create memory pool   
      initSharedData();
      // create PsiIfc using shared memory in pool
      m_instance = new PsiIfc();
   }

   return m_instance;
}

/********************** Class Method Implementations ************************/

/***************************************************************************
// Function Name: PsiIfc constructor.
// Description  : initialize the flash profile address.
// Parameters   : none.
// Returns      : n/a.
****************************************************************************/
PsiIfc::PsiIfc() {
   m_apps = NULL;
}

/***************************************************************************
// Function Name: PsiIfc destructor.
// Description  : free memory of applications.
// Parameters   : n/a.
// Returns      : n/a.
****************************************************************************/
PsiIfc::~PsiIfc() {
   unInit();
   unInitSharedData();
}

/********************** Public Method Implementations **********************/

/***************************************************************************
// Function Name: unInit.
// Description  : free memory of applications.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void PsiIfc::unInit() {
   AppNode* node;

   m_instance = NULL;

   /* clean up memory */
   while ( m_apps != NULL ) {
      node = m_apps;
      m_apps = m_apps->next;
      remove(node->app);
      node->app = NULL;
      brel((void*)node);
      node = NULL;
   }
}

/***************************************************************************
// Function Name: find.
// Description  : find the application that matchs with the given name.
// Parameters   : appId - application name.
// Returns      : application or NULL if not found.
****************************************************************************/
AppNode* PsiIfc::find(const char* appId) {
   AppNode* node = NULL;

   for ( node = m_apps;
         node != NULL &&
         strcmp((node->app)->getAppName(), appId) != 0;
         node = node->next )
      ;

   return node;
}

/***************************************************************************
// Function Name: open.
// Description  : find application and return its application handle. If the
//                application does not exist then create it.
// Parameters   : appId - application name.
// Returns      : application handle.
****************************************************************************/
PSI_HANDLE PsiIfc::open(const char* appId) {
   AppNode* node = find(appId);
   PsiApp* app = NULL;

   /* only add new application if it does not exist */
   if ( node == NULL ) {
      app = new PsiApp(appId);
      if ( app != NULL ) {
         node = (AppNode *)bget(sizeof(APP_NODE));
         node->app = app;
         node->next = NULL;
         append(node);
      }
   }

   return ((PSI_HANDLE)node);
}

/***************************************************************************
// Function Name: close.
// Description  : none.
// Parameters   : appId - application handle.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::close(const PSI_HANDLE appId) {
   return PSI_STS_OK;
}

/***************************************************************************
// Function Name: append.
// Description  : append item to the end of the item list.
// Parameters   : node - item that needs to append.
// Returns      : none.
****************************************************************************/
void PsiIfc::append(AppNode* node) {
   if ( m_apps == NULL )
      m_apps = node;
   else {
      AppNode* curr;
      for ( curr = m_apps;
            curr->next != NULL;
            curr = curr->next )
         ;
      curr->next = node;
   }
}

/***************************************************************************
// Function Name: rename.
// Description  : Rename application.
// Parameters   : oldAppId - old application name.
//                newAppId - new application name.
// Returns      : operation status
****************************************************************************/
PSI_STATUS PsiIfc::rename(const char* oldAppId, const char* newAppId) {
   PSI_STATUS status;
   AppNode* node = find(oldAppId);

   if( node )
      status = (node->app)->setAppName(newAppId);
   else
      status = PSI_STS_ERR_APP_NOT_FOUND;

   return (status);
}

/***************************************************************************
// Function Name: remove.
// Description  : remove application out of PSI.
// Parameters   : appId - application handle.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::remove(const PSI_HANDLE appId) {
   if ( appId == NULL )
      return PSI_STS_ERR_APP_NOT_FOUND;

   PSI_STATUS status = PSI_STS_OK;
   AppNode* curr = NULL;
   AppNode* prev = NULL;
   AppNode* node = (AppNode *)appId;

   for ( prev = curr = m_apps;
         curr != NULL && curr != node;
         curr = curr->next ) {
      prev = curr;
   }

   if ( curr != NULL ) {
      prev->next = curr->next;
      /* move head pointer to the next one
         if head node is removed */
      if ( m_apps == curr )
         m_apps = curr->next;
      curr->next = NULL;
      delete curr->app;
      curr->app = NULL;
      brel((void*)curr);
      curr = NULL;
   } else
      status = PSI_STS_ERR_APP_NOT_FOUND;

   return status;
}

/***************************************************************************
// Function Name: remove.
// Description  : remove object out of application.
// Parameters   : appId - application handle.
//                objectId - object Id.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::remove(const PSI_HANDLE appId,
                          const UINT16 objectId) {
   if ( appId == NULL )
      return PSI_STS_ERR_APP_NOT_FOUND;

   PSI_STATUS status = PSI_STS_OK;
   AppNode* node = (AppNode *)appId;

   status = (node->app)->remove(objectId);

   return status;
}

/***************************************************************************
// Function Name: set.
// Description  : create or modify the object.
// Parameters   : appId - application handle.
//                objectId - object Id.
//                value - value of object.
//                length - length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::set(const PSI_HANDLE appId,
                       const UINT16 objectId,
                       const PSI_VALUE value,
                       const UINT16 length) {
   if ( appId == NULL )
      return PSI_STS_ERR_APP_NOT_FOUND;

   PSI_STATUS status = PSI_STS_OK;
   AppNode* node = (AppNode *)appId;
   PsiApp* app = node->app;

   /* add object to the existing application */
   if ( app != NULL )
      app->set(objectId, value, length);
   else
      status = PSI_STS_ERR_APP_NOT_FOUND;

#ifdef PSI_SHOW_FREEBUF
   bufsize curalloc, totfree, maxfree;
   long nget, nrel;
   bstats(&curalloc, &totfree, &maxfree, &nget, &nrel);
   printf("PSI: free buf %d\n", totfree);
#endif

   return status;
}

/***************************************************************************
// Function Name: get
// Description  : retrieve the object from the application.
// Parameters   : appId - application handle.
//                objectId - object Id.
//                value - buffer contains the value of object.
//                length - length of buffer.
//                retLen - actual length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::get(const PSI_HANDLE appId,
                       const UINT16 objectId,
                       PSI_VALUE value,
                       const UINT16 len,
                       UINT16* retLen) {
   if ( appId == NULL )
      return PSI_STS_ERR_APP_NOT_FOUND;
   /* start of maintain WEB模块－同步问题单，问题单号：A36D00658 by xujunxia 43813 2006年5月12日 */
   if (NULL == value)
   {
       return PSI_STS_ERR_INVALID_MAGIC_VALUE;
   }
   if (NULL == retLen)
   {
       return PSI_STS_ERR_INVALID_LENGTH;
   }
   /* end of maintain WEB模块－同步问题单，问题单号：A36D00658 by xujunxia 43813 2006年5月12日 */
   
   PSI_STATUS status = PSI_STS_OK;
   AppNode* node = (AppNode *)appId;

   status = (node->app)->get(objectId, value, len, retLen);

   return status;
}

/***************************************************************************
// Function Name: init
// Description  : read information from the active profile in the flash and
                  store them to PSI.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
/* HUAWEI HGW w69233 2008-03-29 Add begin */
/* Fix AU8D00334: EquipmentTest command "SetSsid/SetWepKey/SetWpaKey" for wlan does not work */
//extern void BcmDb_setRestoreWlanDefaultFlag(void) ;
/* HUAWEI HGW w69233 2008-03-29 Add end */

PSI_STATUS PsiIfc::init() {
   PSI_STATUS status = profileGetFromFlash(PROFILE_VERIFY_YES);

#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */
   if ( status != PSI_STS_OK ) {
      status = profileGetFromBackupFlash(PROFILE_VERIFY_YES);
/*end of backup profile by c65985 for vdf 080303 */
#endif

   if ( status != PSI_STS_OK ) {
      // If default config file exists and is valid, initialize
      // the PSI flash using the config file
      /*add of clean sp of flash by l39225 2006-7-17 */
      sysScratchPadClearAll();
      /*end of clean sp of flash by l39225 2006-7-17*/
      status = profileGetFromFile(PSI_PATH_DEFAULTCONFIG, PROFILE_VERIFY_YES);

      /* HUAWEI HGW w69233 2008-03-29 Modify begin */
      /* Fix AU8D00334: EquipmentTest command "SetSsid/SetWepKey/SetWpaKey" for wlan does not work */     
      if ( status == PSI_STS_OK ){
         writeFileToFlash(PSI_PATH_DEFAULTCONFIG);
         //BcmDb_setRestoreWlanDefaultFlag();
      }
      /* HUAWEI HGW w69233 2008-03-29 Modify end */
   }

#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */
      else //write backup cfg to main cfg.
      { 
         UINT32 len = sysGetPsiSize();
         char profileBuffer[len];  
      
         // Read the whole profile from Backup flash to a buffer

         if ( PSI_STS_OK == sysPsiBackupGet(profileBuffer, len, 0))
            sysPersistentSet(profileBuffer, len, 0);
      }
   }
/*end of backup profile by c65985 for vdf 080303 */
#endif 
   // if flash has valid configuration file then create PSI tree
   if ( status == PSI_STS_OK )
      profileRetrieve();
         
   return status;
}

/***************************************************************************
// Function Name: profileRetrieve
// Description  : read the specified profile information from the flash
//                and store them to PSI without verification.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileRetrieve(void) {
      
   return profileGetFromFlash(PROFILE_VERIFY_NO);
}

/***************************************************************************
// Function Name: writeFlashToFile
// Description  : read the data in flash and write it to file.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::writeFlashToFile(char *filename) {
   UINT32 len = sysGetPsiSize();
   char *pc = NULL;
   PSI_STATUS status = PSI_STS_ERR_GENERAL;
   FILE *fp = NULL;

   if ( (fp = fopen(filename, "w" )) == NULL )
      status = PSI_STS_ERR_CONFIGFILE_UNKNOWN;
   else {
      char profileBuffer[len];
      // Read the whole profile from flash to a buffer
      sysPersistentGet(profileBuffer, len, 0);

      // if <psitree> cannot be found then return
      if ( memcmp(profileBuffer, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) != 0 )
         status = PSI_STS_ERR_GENERAL;
      else {
         // determine data length
         pc = strstr(profileBuffer, XML_PSI_END_TAG);   
         if ( pc != NULL )
         len = pc - profileBuffer + strlen(XML_PSI_END_TAG);

         // write the data in the buffer to file
         if ( fwrite(profileBuffer, 1, len, fp) == len )
            status = PSI_STS_OK;
      }
      fclose(fp);
   }
   
   return status;
}

/* BEGIN: Added by y67514, 2009/12/30   PN:APNSecurity Enhancement*/
/*****************************************************************************
 函 数 名  : PsiIfc.dumpFlashToFile
 功能描述  : 从flash中导出配置文件，需要去掉apn安全相关参数
 输出参数  : 无
 返 回 值  : PSI_STATUS PsiIfc::
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月30日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
PSI_STATUS PsiIfc::dumpFlashToFile(char *filename) 
{
    UINT32 len = sysGetPsiSize();
    char *pc = NULL;
    char *profileBuffer = NULL;
    char *buf = NULL;
    char *cStart = NULL;
    char *cEnd = NULL;
    char *cTmp1 = NULL;
    char *cTmp2 = NULL;
    PSI_STATUS status = PSI_STS_ERR_GENERAL;
    FILE *fp = NULL;

    if ( (fp = fopen(filename, "w" )) == NULL )
    {
        status = PSI_STS_ERR_CONFIGFILE_UNKNOWN;
        return status;
    }
    else 
    {
        profileBuffer = (char*) malloc(len);
        if ( NULL == profileBuffer )
        {
            profileBuffer = (char*) malloc(len);
        }
        
        buf = (char*) malloc(len);
        if ( NULL == buf )
        {
            buf = (char*) malloc(len);
        }

        if ( (NULL == buf) || (NULL == profileBuffer) )
        {
            if ( NULL != buf )
            {
                free(buf);
            }
            if ( NULL != profileBuffer )
            {
                free(profileBuffer);
            }
            status = PSI_STS_ERR_MEMORY;
            fclose(fp);
            return status;
        }
        
        memset(profileBuffer,0,len);
        memset(buf,0,len);
        // Read the whole profile from flash to a buffer
        sysPersistentGet(profileBuffer, len, 0);

        // if <psitree> cannot be found then return
        if ( memcmp(profileBuffer, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) != 0 )
        {
            status = PSI_STS_ERR_GENERAL;
        }
        else 
        {
            cStart = strstr(profileBuffer,"pppProEntry id=\"1\"");
            cEnd = strstr(profileBuffer,"</pppPro>");
            if ( cStart && cEnd && (cStart < cEnd))
            {
                cTmp1 = strstr(cStart,"UserName");
                cTmp2 = strstr(cStart,"PhoneNum");
                if ( cTmp1 && cTmp2 && (cTmp1 < cTmp2) && (cTmp2 < cEnd))
                {
                    memcpy(buf,profileBuffer,cTmp1 - profileBuffer);
                    memcpy(buf + (cTmp1 - profileBuffer),cTmp2,len - (cTmp2 - profileBuffer));
                    memcpy(profileBuffer,buf,len);
                }
            }

            cStart = strstr(profileBuffer,"pppProEntry id=\"1\"");
            cEnd = strstr(profileBuffer,"</pppPro>");
            if ( cStart && cEnd && (cStart < cEnd))
            {
                cTmp1 = strstr(cStart," acApn");
                cTmp2 = strstr(cStart,"/>");
                if ( cTmp1 && cTmp2 && (cTmp1 < cTmp2) && (cTmp2 < cEnd))
                {
                    memcpy(buf,profileBuffer,cTmp1 - profileBuffer);
                    memcpy(buf + (cTmp1 - profileBuffer),cTmp2,len - (cTmp2 - profileBuffer));
                    memcpy(profileBuffer,buf,len);
                }
            }
            
            /*start of support to set up the pppou username and password by l129990,2009,1,19 */
            cStart = strstr(profileBuffer,"<pppsrv_255_65535>");
            cEnd = strstr(profileBuffer,"</pppsrv_255_65535>");
            if ( cStart && cEnd && (cStart < cEnd))
            {
                cTmp1 = strstr(cStart,"userName");
                cTmp2 = strstr(cStart,"serviceName");
                if ( cTmp1 && cTmp2 && (cTmp1 < cTmp2) && (cTmp2 < cEnd))
                {
                    memcpy(buf,profileBuffer,cTmp1 - profileBuffer);
                    memcpy(buf + (cTmp1 - profileBuffer),cTmp2,len - (cTmp2 - profileBuffer));
                    memcpy(profileBuffer,buf,len);
                }
            }
            /*end of support to set up the pppou username and password by l129990,2009,1,19 */

            // determine data length
            pc = strstr(profileBuffer, XML_PSI_END_TAG);   
            if ( pc != NULL )
            {
                len = pc - profileBuffer + strlen(XML_PSI_END_TAG);
            }

            // write the data in the buffer to file
            if ( fwrite(profileBuffer, 1, len, fp) == len )
            {
                status = PSI_STS_OK;
            }
        }
        fclose(fp);
    }

    free(profileBuffer);
    free(buf);
    return status;
}

/* END:   Added by y67514, 2009/12/30 */

/***************************************************************************
// Function Name: writeFileToFlash
// Description  : Write to flash from the given config file.
// Parameters   : filename - the full path name of the config file.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::writeFileToFlash(char *filename) {
   struct stat StatBuf;
   FILE *fp = NULL;
   PSI_STATUS status = PSI_STS_OK;

   if ((stat(filename, &StatBuf ) == -1) 
	|| ((fp = fopen(filename, "rb" )) == NULL))
       return PSI_STS_ERR_CONFIGFILE_UNKNOWN;

   if (StatBuf.st_size <= 0 || StatBuf.st_size >  sysGetPsiSize()) 
   {
      if (fp != NULL)
         fclose(fp);
      return PSI_STS_ERR_CONFIGFILE_SIZE;
   }
   /*
    *  start of A36D02088 y42304 20060624: 写PSI参数区需要写结束字符串
    *  "\0\0"， 所以需要多申请2个字符的空间 
    */
   char octets[StatBuf.st_size+2];
   /*  end of A36D02088 y42304 20060624 */

   if (fread(octets, 1, StatBuf.st_size, fp) == (size_t) StatBuf.st_size) 
   {
       /* start of A36D02088 y42304 20060624: 在PSI参数末尾加2个结束字符 */
       octets[StatBuf.st_size]   = '\0';
       octets[StatBuf.st_size+1] = '\0';        
      
       status = writeStreamToFlash(octets, (UINT32) StatBuf.st_size+2);
       /* end of A36D02088 y42304 20060624 */
   }
   else 
   {
      status = PSI_STS_ERR_CONFIGFILE_SIZE;
   }

   fclose(fp);
   return status;
}

/***************************************************************************
// Function Name: writeStreamToFlash
// Description  : Write to flash from the byte stream of config file
// Parameters   : filestream - the full byte stream of the config file
//                len - the stream length
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::writeStreamToFlash(char *filestream, UINT32 len) {
   PSI_STATUS status = PSI_STS_OK;   

   UINT32 psiSize = sysGetPsiSize();
   
   /* start of A36D02088 y42304 20060624: 入口文件流中已包含"\0\0"结束符 */
#if 0    
   if ( psiSize < (len + 2) )    // 2 for "\0\0" terminate string characters
#endif

   if ( psiSize <  len )   
   {
       return PSI_STS_ERR_CONFIGFILE_SIZE;
   }
	   	   
   // store stream file to flash
   //add by z67625 for restore
   if(0 != sysPersistentSet(filestream, len, 0))
   {
       status = PSI_STS_ERR_GENERAL;
   }

#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */
   //add by z67625 for restore
   if(0 != sysPsiBackupSet(filestream, len, 0))
   {
       status = PSI_STS_ERR_GENERAL;
   }
/*end of backup profile by c65985 for vdf 080303 */
#endif
   
   // write terminate string characters to mark end of file
   
   /* start of A36D02088 使能/去使能无线功能，会导致组播短时间暂停 y42304 20060624: */
   //sysPersistentSet("\0\0", sizeof(UINT16), len);
   /* end of A36D02088 y42304 20060624 */ 
   
   return status;
}

/***************************************************************************
// Function Name: profileStore
// Description  : write the PSI information to the flash.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileStore(void) {
   // Load the data in the PSI to the file
   PSI_STATUS status = writeConfigToFile(PSI_XML_PATH_FLASHCONFIG);
   
   // Write the data in the file to flash
   if (status == PSI_STS_OK)
      status = writeFileToFlash(PSI_XML_PATH_FLASHCONFIG);
   
   return status;
}

/***************************************************************************
// Function Name: profileErase
// Description  : invalid the information in the given profile.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileErase(void) {
   PSI_STATUS status = PSI_STS_OK;
   UINT32 psiSize = sysGetPsiSize();
   char buf[psiSize];
   
   memset(buf, 0, psiSize);
   //add by z67625 for restore
   if( 0 != sysPersistentSet(buf, psiSize, 0))
  {
     status = PSI_STS_ERR_GENERAL;
  }

#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */ 
   //add by z67625 for restore
   if(0 != sysPsiBackupSet(buf, psiSize, 0))
   {
      status = PSI_STS_ERR_GENERAL;
    }
/*end of backup profile by c65985 for vdf 080303 */
#endif   

   return status;
}

/********************* Private Method Implementations **********************/

/***************************************************************************
// Function Name: profileGetFromFile
// Description  : verify and retrieve from the given config file.
// Parameters   : filename - the full path name of the config file.
//                verification - 1 - yes, 0 - no.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileGetFromFile(char *filename, int verification) {
   struct stat StatBuf;
   FILE *fp = NULL;
   PSI_STATUS status = PSI_STS_ERR_CONFIGFILE_UNKNOWN;

   if (stat(filename, &StatBuf ) == 0 && (fp = fopen(filename, "rb" )) == NULL)
       return status;

   if (StatBuf.st_size <= 0 || StatBuf.st_size >  sysGetPsiSize()) 
   {
      if (fp != NULL)
         fclose(fp);
      return PSI_STS_ERR_CONFIGFILE_SIZE;
   }

   char octets[StatBuf.st_size];

   if (fread(octets, 1, StatBuf.st_size, fp) == (size_t) StatBuf.st_size)
      status = dlXmlParse(octets, StatBuf.st_size, verification);
   else 
      status = PSI_STS_ERR_CONFIGFILE_SIZE;

   fclose(fp);
   return status;
}

/***************************************************************************
// Function Name: profileGetFromFlash
// Description  : verify and retrieve the data in flash.
// Parameters   : verification: 1 - yes, 0 - no.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileGetFromFlash(int verification) {
   UINT32 len = sysGetPsiSize();
   char profileBuffer[len];
   char *pc = NULL;
   PSI_STATUS status = PSI_STS_ERR_GENERAL;
   
   // Read the whole profile from flash to a buffer
   sysPersistentGet(profileBuffer, len, 0);
   
   // if <psitree> cannot be found then return
   if ( memcmp(profileBuffer, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) == 0 ) 
  {
   /*add of judge "trafficmode " by 139225 2006-7-15*/
   #ifdef SUPPORT_VDSL
        pc = strstr(profileBuffer, "trafficmode");  
       if(pc == NULL)
        {
            printf("\n******* no trafficmode ----err fatal**********\n");
            return status;
        }
    #endif
    /*end of judge "trafficmode " by 139225 2006-7-15*/
      // determine buffer length
      pc = strstr(profileBuffer, XML_PSI_END_TAG);   
      if ( pc != NULL )
         len = pc - profileBuffer + strlen(XML_PSI_END_TAG);
      // Load the data in the buffer to PSI
      status = dlXmlParse(profileBuffer, len, verification);
   }

   return status;
}


#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */
/***************************************************************************
// Function Name: profileGetFromFlash 
// Description  : verify and retrieve the data in flash.
// Parameters   : verification: 1 - yes, 0 - no.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::profileGetFromBackupFlash(int verification) {
   UINT32 len = sysGetPsiSize();
   char profileBuffer[len];
   char *pc = NULL;
   PSI_STATUS status = PSI_STS_ERR_GENERAL;
   
   // Read the whole profile from Backup flash to a buffer
   sysPsiBackupGet(profileBuffer, len, 0);
   
   // if <psitree> cannot be found then return
   if ( memcmp(profileBuffer, XML_PSI_START_TAG, strlen(XML_PSI_START_TAG)) == 0 ) 
  {
   /*add of judge "trafficmode " by 139225 2006-7-15*/
   #ifdef SUPPORT_VDSL
        pc = strstr(profileBuffer, "trafficmode");  
       if(pc == NULL)
        {
            printf("\n******* no trafficmode ----err fatal**********\n");
            return status;
        }
    #endif
    /*end of judge "trafficmode " by 139225 2006-7-15*/
      // determine buffer length
      pc = strstr(profileBuffer, XML_PSI_END_TAG);   
      if ( pc != NULL )
         len = pc - profileBuffer + strlen(XML_PSI_END_TAG);
      // Load the data in the buffer to PSI
      status = dlXmlParse(profileBuffer, len, verification);
   }

   return status;
}
/*end of backup profile by c65985 for vdf 080303 */
#endif


/***************************************************************************
// Function Name: writeConfigToFile
// Description  : write the PSI information to the text file in xml format
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiIfc::writeConfigToFile(char *fileName) {
   FILE *fp = NULL;
   UINT32 sizeApps = 0;
   UINT32 len = sysGetPsiSize()*PSI_BUFFER_TO_XML_FACTOR;
   PSI_STATUS status = PSI_STS_OK;

   if (access(fileName,F_OK) == 0) {
     unlink(fileName);
   }
   if ( (fp = fopen(fileName, "a+")) == NULL )
      return PSI_STS_ERR_CONFIGFILE_UNKNOWN;
      
   /* if there is error, remember to close fp before returning */
   // get apps size
   for ( AppNode* node = m_apps;
         node != NULL;
         node = node->next ) {
      sizeApps += (node->app)->getAppSize();
   }

   // if no application then return
   if ( sizeApps == 0 ) {
     /* empty psitree element */
     fprintf(fp,"<%s/>\n","psitree");
     close(fp);
     return status;
   }
   
   if ( sizeApps > len )
      return PSI_STS_ERR_PROFILE_TOO_LARGE;

   fprintf(fp,"<%s>\n","psitree");

   /* filling in content */
   for ( AppNode* node = m_apps;
         node != NULL;
         node = node->next ) {
      (node->app)->toXml(fp);
   }

    /* end tag */
   fprintf(fp,"</%s>\n","psitree");

   fclose(fp);
   
   return status;
}

