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
//  Filename:       psidl.h
//  Author:         Peter T. Tran
//  Creation Date:  11/18/04
//
******************************************************************************
//  Description:
//      Define the global constants, and functions for psiDl utility.
//
*****************************************************************************/

#ifndef __PSI_DL_H__
#define __PSI_DL_H__

#include "psidefs.h"

/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

PSI_STATUS dlXmlParse(char *file, int size, int verification);
PSI_STATUS dlXmlSetObjNode(FILE* file, const char *appName, const UINT16 id,
   const PSI_VALUE value, const UINT16 length);

#if defined(__cplusplus)
}
#endif

#endif
