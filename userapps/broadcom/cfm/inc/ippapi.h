/*****************************************************************************
//
//  Copyright (c) 2000-2004  Broadcom Corporation
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
//  Filename:       ippapi.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  06/16/04
//
******************************************************************************
//  Description:
//      Define the BcmIpp functions.
//
*****************************************************************************/

#ifndef __IPP_API_H__
#define __IPP_API_H__

#include "ifcdefs.h"

#define IPP_APP_ID         "IPPCfg"
#define IPP_CONFIG         1
#define IPP_PRINTER_DEV_DEFAULT "/dev/printer0"
#define IPP_MAX_LENGTH      128
#define IPP_NAME_MAX_LENGTH 40

typedef struct _ippConfig {
  char enabled;
  char make[IPP_MAX_LENGTH];
  char dev[IPP_MAX_LENGTH];
  char name[IPP_NAME_MAX_LENGTH];
} ippConfig, *pIppConfig;

// Return codes for the various functions
typedef enum _ippStatus {
   IPP_OK,
   IPP_STORE_FAILED,
   IPP_NOT_INIT,
   IPP_BAD_RULE,
   IPP_RULE_EXISTS,
   IPP_ADD_FAIL,
   IPP_INIT_FAILED
} IPP_STATUS;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern IPP_STATUS BcmIpp_init(void);
extern int BcmIpp_isInitialized(void);
extern IPP_STATUS BcmIpp_GetVar( char *var, char *value );
extern IPP_STATUS BcmIpp_SetVar( char *var, char *value );
extern void BcmIpp_serverRestart(void);
extern IPP_STATUS BcmIpp_Store(void);
extern void BcmIpp_close(void);
#if defined(__cplusplus)
}
#endif

#endif
