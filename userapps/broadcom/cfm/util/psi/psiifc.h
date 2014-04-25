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
//  Filename:       psiifc.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the PsiIfc class and functions.
//
*****************************************************************************/

#ifndef __PSIIFC_H__
#define __PSIIFC_H__

/********************** Include Files ***************************************/

#include "psiapp.h"
#include "cliapi.h"
#include "board.h"
#include "board_api.h"
#include "psidefs.h"
#include "psishm.h"

/********************** Global Constants ************************************/

// Round it down to the nearest even number
const UINT32 PSI_PROFILE_LEN = ((sysGetPsiSize()- PSI_HEADER_LEN) / PSI_PROFILE_NUM_MAX) & 0xfffffffe;
const int PROFILE_VERIFY_YES = 1;
const int PROFILE_VERIFY_NO  = 0;

/********************** Global Types ****************************************/

typedef struct AppNode {
   PsiApp*   app;
   AppNode*  next;
} APP_NODE, *PAPP_NODE;

/********************** Class Declaration ***********************************/

class PsiIfc : public PsiShm {
public:
   static PsiIfc* getInstance();
   ~PsiIfc();
   void       destroy();
   void       unInit();
   AppNode*   find(const char* appId);
   PSI_HANDLE open(const char* appId);
   PSI_STATUS close(const PSI_HANDLE appId);
   PSI_STATUS rename(const char* oldAppId, const char* newAppId);
   PSI_STATUS remove(const PSI_HANDLE appId);
   PSI_STATUS remove(const PSI_HANDLE appId, const UINT16 objectId);
   PSI_STATUS set(const PSI_HANDLE appId, const UINT16 objectId,
                  const PSI_VALUE value, const UINT16 length);
   PSI_STATUS get(const PSI_HANDLE appId, const UINT16 objectId,
                  PSI_VALUE value, const UINT16 len, UINT16* retLen);
   PSI_STATUS init();
   PSI_STATUS profileRetrieve(void);
   PSI_STATUS writeFlashToFile(char *fileName);
   /* BEGIN: Added by y67514, 2009/12/30   PN:APNSecurity Enhancement*/
   PSI_STATUS dumpFlashToFile(char *fileName);
   /* END:   Added by y67514, 2009/12/30 */
   PSI_STATUS writeFileToFlash(char *filename);
   PSI_STATUS writeStreamToFlash(char *filestream, UINT32 len);
   PSI_STATUS profileStore(void);
   PSI_STATUS profileErase(void);
protected:
   PsiIfc();
private:
   static PsiIfc* m_instance;
   AppNode*       m_apps;
   void           append(AppNode* node);
   PSI_STATUS     profileGetFromFile(char *filename, int verification);
   PSI_STATUS     profileGetFromFlash(int verification);
#ifdef SUPPORT_BACKUPCFGFILE
/*start of backup profile by c65985 for vdf 080303 */
   PSI_STATUS     profileGetFromBackupFlash(int verification);
/*end of backup profile by c65985 for vdf 080303 */
#endif
   PSI_STATUS     writeConfigToFile(char *fileName);
};

#endif
