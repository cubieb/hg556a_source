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
//  Filename:       psiapp.cpp
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Implement the PsiApp class and functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <string.h>
#include "psiifc.h"
#include "psiapp.h"
#include "psiutil.h"
#include "ifcdefs.h"
#include "bget.h"
#include "stdlib.h"

/********************** Class Method Implementations ************************/

/***************************************************************************
// Function Name: PsiApp constructor.
// Description  : create PsiApp with the given name without any ohjects.
// Parameters   : appId - application name.
// Returns      : n/a.
****************************************************************************/
PsiApp::PsiApp(const char* appName) {
   if ( appName != NULL ) {
      m_appName = (char *)bget(strlen(appName) + 1);
      strcpy(m_appName, appName);
   } else
      m_appName = NULL;
   m_objects = NULL;
}

/***************************************************************************
// Function Name: PsiApp constructor.
// Description  : create PsiApp with the given name with one object.
// Parameters   : appId - application name.
//                id - object Id.
//                value - object value.
//                length - object length.
// Returns      : n/a.
****************************************************************************/
PsiApp::PsiApp(const char* appName, const UINT16 id, 
               const PSI_VALUE value, const UINT16 length) {
   if ( appName != NULL ) {
      m_appName = (char *)bget(strlen(appName) + 1);
      strcpy(m_appName, appName);
   } else
      m_appName = NULL;
   m_objects = NULL;
   PsiObject* object = new PsiObject(id, value, length);
   add(object);
}

/***************************************************************************
// Function Name: PsiApp destructor.
// Description  : free memory of objects, and application name.
// Parameters   : n/a.
// Returns      : n/a.
****************************************************************************/
PsiApp::~PsiApp() {
   ObjectNode* node = NULL;
   brel((void *)m_appName);
   while ( m_objects != NULL ) {
      node = m_objects;
      m_objects = m_objects->next;
      delete node->object;
      node->object = NULL;
      brel((void *)node);
      node = NULL;
   }
}

/********************** Public Method Implementations **********************/

/***************************************************************************
// Function Name: remove.
// Description  : remove the object with the given id out of application.
// Parameters   : id - object Id.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiApp::remove(const UINT16 id) {
   PSI_STATUS status = PSI_STS_OK;
   ObjectNode* curr = NULL;
   ObjectNode* prev = NULL;

   for ( prev = curr = m_objects;
         curr != NULL &&
         (curr->object)->getId() != id;
         curr = curr->next )
      prev = curr;

   if ( curr != NULL ) {
      prev->next = curr->next;
      /* move head pointer to the next one
         if head node is removed */
      if ( m_objects == curr )
         m_objects = curr->next;
      curr->next = NULL;
      delete curr->object;
      curr->object = NULL;
      curr = NULL;
   } else
      status = PSI_STS_ERR_OBJECT_NOT_FOUND;

//display();

   return status;
}

/***************************************************************************
// Function Name: set.
// Description  : create or modify the object.
// Parameters   : id - object Id.
//                value - object value.
//                length - object length.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiApp::set(const UINT16 id,
                       const PSI_VALUE value,
                       const UINT16 length) {
   PSI_STATUS status = PSI_STS_OK;
   ObjectNode* node = find(id);

   /* if node does not exist then create and add new node */
   if ( node == NULL ) {
      PsiObject* object = new PsiObject(id, value, length);
      if ( object != NULL )
         status = add(object);
      else
         status = PSI_STS_ERR_MEMORY;
   } else /* modify the existing node */
      status = (node->object)->set(value, length);

//display();

   return status;
}

/***************************************************************************
// Function Name: get.
// Description  : retrieve the value of the object.
// Parameters   : id - object Id.
//                value - buffer that contains object value.
//                length - buffer length.
//                retLen - actual length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiApp::get(const UINT16 id, PSI_VALUE value,
                       const UINT16 len, UINT16* retLen) {
   PSI_STATUS status = PSI_STS_OK;
   ObjectNode* node = find(id);

   if ( node != NULL )
      status = (node->object)->get(value, len, retLen);
   else
      status = PSI_STS_ERR_OBJECT_NOT_FOUND;

   return status;
}

/***************************************************************************
// Function Name: display.
// Description  : show each object in object lists in octet format.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void PsiApp::display() {
   char buf[PSI_PROFILE_LEN];
   int size = 0;
   ObjectNode* node = NULL;

   for ( node = m_objects;
         node != NULL;
         node = node->next ) {
      printf("Node Id = %d\n", (node->object)->getId());
      size = (node->object)->toOctets(buf, 0);
      PsiUtil::octprint(buf, size);
      printf("\n");
   }
}

/***************************************************************************
// Function Name: setAppName.
// Description  : set application name.
// Parameters   : name - application name
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiApp::setAppName(const char *name) {
   PSI_STATUS status = PSI_STS_ERR_MEMORY;

   if( m_appName && strlen(name) > strlen(m_appName) ) {
      brel((void *)m_appName);
      m_appName = NULL;
   }

   if( m_appName == NULL )
      m_appName = (char *)bget(strlen(name) + 1);
    
   if( m_appName ) {
      strcpy( m_appName, name );
      status = PSI_STS_OK;
   }

   return status;
}

/***************************************************************************
// Function Name: getAppSize.
// Description  : get the size of the application.
// Parameters   : none.
// Returns      : size of application.
****************************************************************************/
UINT16 PsiApp::getAppSize() {
   UINT16 shSize = sizeof(UINT16);
   UINT16 len = (UINT16)strlen(m_appName);
   UINT16 sizeObjs = 0;

   for ( ObjectNode* node = m_objects;
         node != NULL;
         node = node->next ) {
      // object size = size of object ID + size of object length + size of object value
      sizeObjs += shSize + shSize + (node->object)->getLength();
   }

   // app = len of app + len of appName + appName + objects list
   return (shSize + shSize + len + sizeObjs);
}

/***************************************************************************
// Function Name: toOctets.
// Description  : copy application with its object lists to PSI octet string.
// Parameters   : data - PSI octet string
//                pos - start position of application in the PSI octet string.
// Returns      : size of application.
****************************************************************************/
UINT16 PsiApp::toOctets(char *data, UINT32 pos) {
   UINT16 shSize = sizeof(UINT16);
   UINT16 len = (UINT16)strlen(m_appName);
   UINT16 sizeObjs = 0, sizeApp  = 0;

   for ( ObjectNode* node = m_objects;
         node != NULL;
         node = node->next ) {
      // object size = size of object ID + size of object length + size of object value
      sizeObjs += shSize + shSize + (node->object)->getLength();
   }

   // sizeApp = len of appName + appName + objects list
   sizeApp = shSize + len + sizeObjs;

   // length of app
   memcpy(&data[pos], (char *)&sizeApp, shSize);

   // length of appName
   pos += shSize;
   memcpy(&data[pos], (char *)&len, shSize);

   // appName
   pos += shSize;
   memcpy(&data[pos], m_appName, len);

   // objects list
   pos += len;
   for ( ObjectNode* node = m_objects;
         node != NULL;
         node = node->next ) {
      pos += (node->object)->toOctets(data, pos);
   }

   // app = len of app + sizeApp
   return (shSize + sizeApp);
}

/***************************************************************************
// Function Name: toXml
// Description  : copy application with its object lists from PSI to xml file
// Parameters   : data - PSI octet string
//                pos - start position of application in the PSI octet string.
// Returns      : size of application.
****************************************************************************/
UINT16 PsiApp::toXml(FILE *file) {
   UINT16 shSize = sizeof(UINT16);
   UINT16 len = (UINT16)strlen(m_appName);
   UINT16 sizeObjs = 0, sizeApp  = 0;

   for ( ObjectNode* node = m_objects;
         node != NULL;
         node = node->next ) {
      // object size = size of object ID + size of object length + size of object value
      sizeObjs += shSize + shSize + (node->object)->getLength();
   }

   // sizeApp = len of appName + appName + objects list
   sizeApp = shSize + len + sizeObjs;

   /* start tag of this app */
   if (sizeObjs == 0) {
     /* empty app element */
     /* start of maintain 应用的长度为0时，不写应用名 by xujunxia 43813 2006年10月8日
     fprintf(file,"<%s/>\n",m_appName);
     end of maintain 应用的长度为0时，不写应用名 by xujunxia 43813 2006年10月8日 */
     return (sizeApp);
   }
        
   fprintf(file,"<%s>\n",m_appName);

   /* beginning of objects */
   // objects list
   for ( ObjectNode* node = m_objects;
	 node != NULL;
	 node = node->next ) {
     (node->object)->toXml(file,m_appName);
   }

   fprintf(file,"</%s>\n",m_appName);

   // app = len of app + sizeApp
   return (shSize + sizeApp);
}
#if 0
/**************************************************************************
// Function Name: displayXml.
// Description  : show each object in object lists in octet format.
// Parameters   : none.
// Returns      : none.
****************************************************************************/
void PsiApp::displayXml() {
   char buf[PSI_PROFILE_LEN];
   int size = 0;
   ObjectNode* node = NULL;

   /* cat current psi.xml */
   /* display running config psiRunning.xml */
}
#endif 
/********************* Private Method Implementations **********************/

/***************************************************************************
// Function Name: find.
// Description  : find the object that matchs with the given id.
// Parameters   : id - object Id.
// Returns      : object or NULL if not found.
****************************************************************************/
ObjectNode* PsiApp::find(const UINT16 id) {
   ObjectNode* node = NULL;

   for ( node = m_objects;
         node != NULL &&
         (node->object)->getId() != id;
         node = node->next )
      ;

   return node;
}

/***************************************************************************
// Function Name: add.
// Description  : add the object to the object lists.
// Parameters   : object - the object that needs to add.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiApp::add(PsiObject* object) {
   PSI_STATUS status = PSI_STS_OK;
   ObjectNode* node = (ObjectNode *)bget(sizeof(OBJECT_NODE));
     if ( node != NULL ) {
      node->object = object;
      node->next = NULL;
      append(node);
   } else
      status = PSI_STS_ERR_MEMORY;

   return status;
}

/***************************************************************************
// Function Name: append.
// Description  : append item to the end of the item list.
// Parameters   : node - item that needs to append.
// Returns      : none.
****************************************************************************/
void PsiApp::append(ObjectNode* node) {
   if ( m_objects == NULL )
      m_objects = node;
   else {
      ObjectNode* curr;
      for ( curr = m_objects;
            curr->next != NULL;
            curr = curr->next )
         ;
      curr->next = node;
   }
}
