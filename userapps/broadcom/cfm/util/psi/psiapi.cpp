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
//  Filename:       psiapi.cpp
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      It defines the Persistent Storage Interface APIs.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdlib.h>
#include <unistd.h>
#include "psiifc.h"
#include "psiutil.h"
#include "psisema.h"
#ifdef SUPPORT_TR69C
#include "tr69cdefs.h"
#endif

/********************** APIs Definitions ************************************/

extern "C" {
   PSI_HANDLE BcmPsi_appOpen(char* appId);
   PSI_STATUS BcmPsi_appClose(PSI_HANDLE appId);
   PSI_STATUS BcmPsi_appRename(char *oldAppId, char *newAppId);
   PSI_STATUS BcmPsi_appRemove(PSI_HANDLE appId);
   PSI_STATUS BcmPsi_objRemove(PSI_HANDLE appId, UINT16 objectId);
   PSI_STATUS BcmPsi_objStore(PSI_HANDLE appId, UINT16 objectId,
                              PSI_VALUE value, UINT16 length);
   PSI_STATUS BcmPsi_objRetrieve(PSI_HANDLE appId, UINT16 objectId,
                                 PSI_VALUE value, UINT16 len,
                                 UINT16* retLen);
   PSI_STATUS BcmPsi_flush(void);
   PSI_STATUS BcmPsi_writeFileToFlash(char *filename);
   PSI_STATUS BcmPsi_writeStreamToFlash(char *filestream, UINT32 len);
   PSI_STATUS BcmPsi_writeFlashToFile(void);
   /* BEGIN: Added by y67514, 2009/12/30   PN:APNSecurity Enhancement*/
    PSI_STATUS BcmPsi_dumpFlashToFile(void);
   /* END:   Added by y67514, 2009/12/30 */
   PSI_STATUS BcmPsi_profileStore(void);
   PSI_STATUS BcmPsi_profileErase(void);
   PSI_STATUS BcmPsi_profileRetrieve(void);
   void BcmPsi_init(void);
   void BcmPsi_unInit(void);
   unsigned long BcmPsi_getNumberOfChanges(void);
   void BcmPsi_setNumberOfChanges(UINT32 num);
}

/***************************************************************************
// Function Name: BcmPsi_appOpen
// Description  : create or open the application that is associated with
//                the given name..
// Parameters   : appId - application name.
// Returns      : handle to application.
****************************************************************************/
PSI_HANDLE BcmPsi_appOpen(char* appId) {
   PsiIfc* ifc = PsiIfc::getInstance();
   PSI_HANDLE ret=NULL;
        
   if ( semop(psi_semid, &lock[0], 2) == -1 )
      perror("appOpen: semdown");

   ret = ifc->open((const char*)appId);

   if ( semop(psi_semid, &unlock[0], 1) == -1 )
      perror("appOpen: semup");

   return ret;
}

/***************************************************************************
// Function Name: BcmPsi_appClose
// Description  : close the application that is associated with given handle.
// Parameters   : appId - application handle.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_appClose(PSI_HANDLE appId) {
   PsiIfc* ifc = PsiIfc::getInstance();
   PSI_STATUS ret;
        

        //printf("1._appClose: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("appClose: semdown");

        ret =ifc->close((const PSI_HANDLE)appId);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("appClose: semup");

        return ret;

}

/***************************************************************************
// Function Name: BcmPsi_appRename
// Description  : remove the application that is associated with the given
//                handle out of PSI.
// Parameters   : appId - application handle.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_appRename(char *oldAppId, char *newAppId) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        
        //printf("1._appRemove: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("appRemove: semdown");

        ret= ifc->rename((const char *) oldAppId, (const char *) newAppId);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("appRemove: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_appRemove
// Description  : remove the application that is associated with the given
//                handle out of PSI.
// Parameters   : appId - application handle.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_appRemove(PSI_HANDLE appId) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        
        //printf("1._appRemove: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("appRemove: semdown");

        ret= ifc->remove((const PSI_HANDLE)appId);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("appRemove: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_objRemove
// Description  : remove the object that is associated with the given
//                application handle and object Id out of the application.
// Parameters   : appId - application handle.
//                objectId - object Id.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_objRemove(PSI_HANDLE appId, UINT16 objectId) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;

        //printf("1._ObjRemove: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("objRemove: semdown");

        ret = ifc->remove((const PSI_HANDLE)appId, (const UINT16)objectId);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("objRemove: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_objStore
// Description  : store the object to the application.
// Parameters   : appId - application handle.
//                objectId - object Id.
//                value - value of object.
//                length - length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_objStore(PSI_HANDLE appId, UINT16 objectId,
                           PSI_VALUE value, UINT16 length) {
   PsiIfc* ifc = PsiIfc::getInstance();
   PSI_STATUS ret;
        
   //printf("1._ObjStore: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
   if ( semop(psi_semid, &lock[0], 2) == -1 )
            perror("objStore: semdown");

   ret =ifc->set((const PSI_HANDLE)appId, (const UINT16)objectId,
               (const PSI_VALUE)value, (const UINT16)length);

   if ( semop(psi_semid, &unlock[0], 1) == -1 )
            perror("objStore: semup");

#ifdef SUPPORT_TR69C
   BcmTr69c_CfmMsg(PARAM_CHANGE);
#endif

   return ret;
}

/***************************************************************************
// Function Name: BcmPsi_objRetrieve
// Description  : retrieve the object from the application.
// Parameters   : appId - application handle.
//                objectId - object Id.
//                value - buffer contains the value of object.
//                length - length of buffer.
//                retLen - actual length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_objRetrieve(PSI_HANDLE appId, UINT16 objectId,
                              PSI_VALUE value, UINT16 len,
                              UINT16* retLen) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;

        //printf("1._ObjRetrieve: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("objRetrieve: semdown");

    ret = ifc->get((const PSI_HANDLE)appId, (const UINT16)objectId,
                   value, (const UINT16)len, retLen);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("objRetrieve: semup");

      return ret;
}

/***************************************************************************
// Function Name: BcmPsi_flush
// Description  : write the PSI information to the active profile.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_flush() {
   PsiIfc* ifc = PsiIfc::getInstance();
   PSI_STATUS ret;
        
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("flush: semdown");
        
   ret=ifc->profileStore();
   
        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("flush: semup");   

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_writeStreamToFlash
// Description  : Write to flash from a byte stream
// Parameters   : filestream - the full byte stream of the config file
//                len - the stream length
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_writeStreamToFlash(char *filestream, UINT32 len) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("streamCopyToFlash: semdown");

   ret= ifc->writeStreamToFlash(filestream, len);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("streamCopyToFlash: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_writeFileToFlash
// Description  : Write the whole 16k PSI flash from the config file
// Parameters   : filename - the full path name of the config file
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_writeFileToFlash(char *filename) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("fileCopyToFlash: semdown");

   ret= ifc->writeFileToFlash(filename);

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("fileCopyToFlash: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_profileStore
// Description  : write the PSI information to the specified profile.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_profileStore(void) {
   PsiIfc* ifc = PsiIfc::getInstance();
   PSI_STATUS ret;
        

        //printf("1._Store: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("profileStore: semdown");

        ret=ifc->profileStore();

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("profileStore: semup");


        return ret;

}

/***************************************************************************
// Function Name: BcmPsi_profileErase
// Description  : invalidate the PSI information in the given profile.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_profileErase(void) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        

        //printf("1._Erase: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("profileErase: semdown");

   ret= ifc->profileErase();

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("profileErase: semup");

        return ret;

}

/***************************************************************************
// Function Name: BcmPsi_profileRetrieve
// Description  : read information from the given profile and save to PSI.
// Parameters   : none.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_profileRetrieve(void) {
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret;
        
        //printf("1.ProfileRetrieve: semval=%d %d\n",semctl(psi_semid,0,GETVAL,0),getpid());
        if ( semop(psi_semid, &lock[0], 2) == -1 )
                perror("profileRetrieve: semdown");

   ret= ifc->profileRetrieve();

        if ( semop(psi_semid, &unlock[0], 1) == -1 )
                perror("profileRetrieve: semup");

        return ret;
}

/***************************************************************************
// Function Name: BcmPsi_init
// Description  : read information from the active profile and store to PSI.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void BcmPsi_init() {
   PsiIfc* ifc = PsiIfc::getInstance();

   ifc->init();
}

/***************************************************************************
// Function Name: BcmPsi_unInit
// Description  : destroy and clean up the PSI.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void BcmPsi_unInit() {
   PsiIfc* ifc = PsiIfc::getInstance();

   ifc->unInit();
}

/***************************************************************************
// Function Name: BcmPsi_writeFlashToFile
// Description  : write configuration from flash to xml files.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS BcmPsi_writeFlashToFile(void) {
    //保证导出的内容是psi中的内容
    //BcmPsi_flush();

   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret = PSI_STS_ERR_GENERAL;

   if ( semop(psi_semid, &lock[0], 2) == -1 )
     perror("xmlProfileDump: semdown");

   ret = ifc->writeFlashToFile(PSI_XML_PATH_FLASHCONFIG);
   
   if ( semop(psi_semid, &unlock[0], 1) == -1 )
     perror("xmlProfileDump: semup");

   return ret;
}

/* BEGIN: Added by y67514, 2009/12/30   问题单号:APNSecurity Enhancement*/
/*****************************************************************************
 函 数 名  : BcmPsi_dumpFlashToFile
 功能描述  : 从flash中读取配置文件，但将apn相关信息扣掉
 输入参数  : void  
 输出参数  : 无
 返 回 值  : PSI_STATUS
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2009年12月30日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
PSI_STATUS BcmPsi_dumpFlashToFile(void) 
{
   PsiIfc* ifc = PsiIfc::getInstance();

   PSI_STATUS ret = PSI_STS_ERR_GENERAL;

   if ( semop(psi_semid, &lock[0], 2) == -1 )
     perror("xmlProfileDump: semdown");

   ret = ifc->dumpFlashToFile(PSI_XML_PATH_FLASHCONFIG);
   
   if ( semop(psi_semid, &unlock[0], 1) == -1 )
     perror("xmlProfileDump: semup");

   return ret;
}
/* END:   Added by y67514, 2009/12/30 */

/***************************************************************************
// Function Name: BcmPsi_getNumberOfChanges
// Description  : get number of times that PSI is changed.
// Returns      : number of changes.
****************************************************************************/
UINT32 BcmPsi_getNumberOfChanges(void) {
   UINT32 num = 0;
   char str[PSI_BUFF_SIZE];
   FILE *fp = NULL;
   
   if ( (fp = fopen(PSI_NUMBER_OF_CHANGES, "r")) != NULL ) {
      fgets(str, PSI_BUFF_SIZE, fp);
      fclose(fp);
      num = (UINT32) strtoul(str, NULL, 10);
   }
   
   return num;
}

/***************************************************************************
// Function Name: BcmPsi_setNumberOfChanges
// Description  : set number of times that PSI is changed.
// Returns      : none.
****************************************************************************/
void BcmPsi_setNumberOfChanges(UINT32 num) {
   char str[PSI_BUFF_SIZE];
   FILE *fp = NULL;
   
   if ( (fp = fopen(PSI_NUMBER_OF_CHANGES, "w")) != NULL ) {
      sprintf(str, "%lu\n", num);
      fputs(str, fp);
      fclose(fp);
   }
}
