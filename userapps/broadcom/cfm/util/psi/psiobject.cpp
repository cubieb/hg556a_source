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
//  Filename:       psiobject.cpp
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Implement the PsiObject class and functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <string.h>
#include <dlfcn.h>

#include "psiobject.h"
#include "psiapi.h"
#include "psidl.h"
#include "bget.h"
#include "stdlib.h"

/********************** Class Method Implementations ************************/

/***************************************************************************
// Function Name: PsiObject constructor.
// Description  : create PsiObject with the given id, value, and length.
// Parameters   : id - object Id.
//                value - object value.
//                length - object length.
// Returns      : n/a.
****************************************************************************/
PsiObject::PsiObject(const UINT16 id,
                     const PSI_VALUE value,
                     const UINT16 length) {
   if ( value == NULL ) return;

   m_id = id;
   m_length = length;

   /* allocate new memory */
 
   m_value = (char *)bget(m_length);
   if ( m_value == NULL ) return;

   /* assign value */
   memcpy(m_value, (char *)value, m_length);
}

/***************************************************************************
// Function Name: PsiObject destructor.
// Description  : free memory of object.
// Parameters   : n/a.
// Returns      : n/a.
****************************************************************************/
PsiObject::~PsiObject() {
   m_id = 0;
   m_length = 0;
   brel((void *)m_value);
   m_value = NULL;
}

/********************** Public Method Implementations **********************/

/***************************************************************************
// Function Name: set.
// Description  : modify the object.
// Parameters   : value - object value.
//                length - object length.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiObject::set(const PSI_VALUE value, const UINT16 len) {
   PSI_STATUS status = PSI_STS_OK;

   if ( value == NULL ) 
      return PSI_STS_ERR_INVALID_VALUE;

   if ( m_length != len ) {
      /* free old memory */
      brel((void *)m_value);
      m_value = NULL;
      /* allocate new memory */
      m_length = len;
      m_value = (char *)bget(m_length);
      if ( m_value == NULL )
         return PSI_STS_ERR_MEMORY;
   }

   /* assign value */
   memcpy(m_value, (char *)value, m_length);

   // increase number of times that PSI is changed by one
   UINT32 num = BcmPsi_getNumberOfChanges() + 1;
   BcmPsi_setNumberOfChanges(num);
   
   return status;
}

/***************************************************************************
// Function Name: get.
// Description  : retrieve the value of the object.
// Parameters   : value - buffer that contains object value.
//                length - buffer length.
//                retLen - actual length of object.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS PsiObject::get(PSI_VALUE value, const UINT16 len, UINT16* retLen) {
   PSI_STATUS status = PSI_STS_OK;

   /* assign value */
   memcpy((char *)value, m_value, len);
   /* return the current length */
   *retLen = m_length;
   /* if len < current length then return error */
   if ( len < m_length )
      status = PSI_STS_ERR_INVALID_LENGTH;

   return status;
}

/***************************************************************************
// Function Name: toOctets.
// Description  : convert object to octet string.
// Parameters   : data - PSI octet string.
//                pPos - start position of object in the PSI octet string.
// Returns      : size of object ID + object length + object value.
****************************************************************************/
UINT16 PsiObject::toOctets(char* data, UINT32 pos) {
   UINT16 shSize = sizeof(UINT16);

   /* object Id */
   memcpy(&data[pos], (char *)&m_id, shSize);
   /* object length */
   pos += shSize;
   memcpy(&data[pos], (char *)&m_length, shSize);
   /* object value */
   pos += shSize;
   memcpy(&data[pos], m_value, m_length);

   return (shSize + shSize + m_length);
}

/***************************************************************************
// Function Name: toXml
// Description  : convert object to xml string and store in the file
// Parameters   : fileName - file xml string will be written to
//                appName - appName where this objects reside under
// Returns      : size of object ID + object length + object value.
****************************************************************************/
UINT16 PsiObject::toXml(FILE *file, char *appName) {
   UINT16 shSize = sizeof(UINT16);

   dlXmlSetObjNode(file,appName,m_id,m_value,m_length);

   return (shSize + shSize + m_length);
}
