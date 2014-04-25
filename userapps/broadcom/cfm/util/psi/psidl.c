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
//  Filename:       psidl.c
//  Author:         Peter T. Tran
//  Creation Date:  11/18/04
//
******************************************************************************
//  Description:
//      It provides dl functions for PSI library.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>

#include "psixml.h"
#include "psidl.h"

/***************************************************************************
// Function Name: dlXmlParse
// Description  : parse and verify the given config file.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS dlXmlParse(char *file, int size, int verification) {
   PSI_STATUS status = PSI_STS_ERR_CONFIGFILE_UNKNOWN;
   PSX_STATUS (*fncXmlParse)(char *, int, int);
   const char *errMsg = NULL;
   
   void *handle = dlopen("libpsixml.so", RTLD_LAZY);
   
   if ( handle == NULL ) {
      fprintf(stderr, "Failed to load libpsixml.so: %s\n", dlerror());
      return status;
   }
   
   dlerror(); // reset error number
   
   fncXmlParse = (PSX_STATUS (*) (char *, int, int))dlsym(handle, "xmlParse");
   
   if ( (errMsg = dlerror()) == NULL ) {
      if ( fncXmlParse(file, size, verification) == PSX_STS_OK )
         status = PSI_STS_OK;
   } else
      fprintf(stderr, "Didn't find xmParse(): %s\n", errMsg);   
   
   dlclose(handle);
   
   return status;
}

/***************************************************************************
// Function Name: dlXmlParse
// Description  : store object to the given config file.
// Returns      : operation status.
****************************************************************************/
PSI_STATUS dlXmlSetObjNode(FILE* file, const char *appName, const UINT16 id,
   const PSI_VALUE value, const UINT16 length) {
   
   PSI_STATUS status = PSI_STS_ERR_CONFIGFILE_UNKNOWN;
   PSX_STATUS (*fncXmlSetObjNode)
              (FILE*, const char *, const UINT16, const PSI_VALUE, const UINT16);
   const char *errMsg = NULL;
   
   void *handle = dlopen("libpsixml.so", RTLD_LAZY);
   
   if ( handle == NULL ) {
      fprintf(stderr, "Failed to load libpsixml.so: %s\n", dlerror());
      return status;
   }
   
   dlerror(); // reset error number
   
   fncXmlSetObjNode = (PSX_STATUS (*)
                       (FILE*, const char *, const UINT16, const PSI_VALUE, const UINT16))
                      dlsym(handle, "xmlSetObjNode");
   
   if ( (errMsg = dlerror()) == NULL ) {
      fncXmlSetObjNode(file, appName, id, value, length);
      status = PSI_STS_OK;
   } else
      fprintf(stderr, "Didn't find xmlSetObjNode(): %s\n", errMsg);   
   
   dlclose(handle);
   
   return status;
}
