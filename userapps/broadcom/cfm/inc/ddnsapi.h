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
//  Filename:       secapi.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  02/03/04
//
******************************************************************************
//  Description:
//      Define the BcmDDns functions.
//
*****************************************************************************/

#ifndef __DDNS_API_H__
#define __DDNS_API_H__

#include "ifcdefs.h"

#define DDNS_APP_ID	"DDNSCfg"
#define DDNS_NUM_SERVICES	1
#define	DDNS_SERVICE_TABLE	2

typedef struct DDnsServiceInfo {
  char hostname[IFC_MEDIUM_LEN];
  char username[IFC_MEDIUM_LEN];
  char password[IFC_PASSWORD_LEN];
  char iface[IFC_TINY_LEN];
  UINT16 service; // 0 for tzo, 1 for dyndns
  UINT16 bSelect;// add by l66195 for VDF
} DDNS_SERVICE_INFO, *PDDNS_SERVICE_INFO;

// Return codes for the various functions

typedef enum _ddnsStatus {
   DDNS_OK,
   DDNS_STORE_FAILED,
   DDNS_NOT_INIT,
   DDNS_BAD_HOSTNAME,
   DDNS_HOSTNAME_EXISTS,
   DDNS_ADD_FAIL,
   DDNS_INIT_FAILED
} DDNS_STATUS;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern DDNS_STATUS BcmDDns_init(void);
extern int BcmDDns_isInitialized(void);
//modified by l66195 for VDF start
//extern void *BcmDDns_getDDnsCfg( void *node, char *hostname, char *username, char *password, char *iface, UINT16 *service );
extern void *BcmDDns_getDDnsCfg( void *node, char *hostname, char *username, char *password, char *iface, UINT16 *service, UINT16 *bSelect);
//extern DDNS_STATUS BcmDDns_add( char *hostname, char *username, char *password,  char *iface, UINT16 service );
extern DDNS_STATUS BcmDDns_add( char *hostname, char *username, char *password,  char *iface, UINT16 service ,UINT16 bSelect);
extern DDNS_STATUS BcmDDns_set( char *hostname, char *username, char *password,  char *iface, UINT16 service ,UINT16 bSelect);
//modified by l66195 for VDF end
extern DDNS_STATUS BcmDDns_remove( char *hostname );
extern DDNS_STATUS BcmDDns_Store(void);
void BcmDDns_serverRestart(void);
extern void BcmDDns_close(void);
void BcmDDns_unInit(void);
#if defined(__cplusplus)
}
#endif

#endif
