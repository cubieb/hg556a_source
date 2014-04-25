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
//  Filename:       psiapp.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the PsiApp class and functions.
//
*****************************************************************************/

#ifndef __PSIAPP_H__
#define __PSIAPP_H__

/********************** Include Files ***************************************/

#include "psiobject.h"
#include "psishm.h"

/********************** Global Types ****************************************/

typedef struct ObjectNode {
   PsiObject*   object;
   ObjectNode*  next;
} OBJECT_NODE, *POBJECT_NODE;
 
/********************** Class Declaration ***********************************/

class PsiApp : public PsiShm {
public:
   PsiApp() : m_appName(NULL), m_objects(NULL){};
   PsiApp(const char* appName);
   PsiApp(const char* appName, const UINT16 id,
          const PSI_VALUE value, const UINT16 length);
   ~PsiApp();
   void display();
   char* getAppName() { return m_appName; }
   PSI_STATUS setAppName(const char* name);
   UINT16     getAppSize();
   UINT16     toOctets(char* data, UINT32 pos);
   PSI_STATUS remove(const UINT16 id);
   PSI_STATUS set(const UINT16 id, const PSI_VALUE value,
                  const UINT16 length);
   PSI_STATUS get(const UINT16 id, PSI_VALUE value,
                  const UINT16 length, UINT16* retLength);
   UINT16     toXml(FILE* file);
private:
   char*       m_appName;
   ObjectNode* m_objects;
   ObjectNode* find(const UINT16 id);
   PSI_STATUS  add(PsiObject* object);
   void        append(ObjectNode* node);
};

#endif
