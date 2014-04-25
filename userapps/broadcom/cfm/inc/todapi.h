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
//  Filename:       todapi.h
//  Author:         Paul J.Y. Lahaie
//  Creation Date:  02/25/04
//
******************************************************************************
//  Description:
//      Define the BcmTod functions.
//
*****************************************************************************/

#ifndef __TOD_API_H__
#define __TOD_API_H__

#include "ifcdefs.h"

#define MAC_LEN 6

#define TOD_APP_ID             "ToDCfg"
#define TOD_NUM_ENTRIES        1
#define	TOD_ENTRY_TABLE        0

typedef struct TodEntry {
  char username[IFC_MEDIUM_LEN];
  char mac[MAC_LEN];
  unsigned short int start_time;
  unsigned short int end_time;
  unsigned char days; // bit mask
} TOD_ENTRY, *PTOD_ENTRY;

// Return codes for the various functions

typedef enum _todStatus {
   TOD_OK,
   TOD_STORE_FAILED,
   TOD_NOT_INIT,
   TOD_BAD_RULE,
   TOD_RULE_EXISTS,
   TOD_RULE_NAME_EXISTS,
   TOD_ADD_FAIL,
   TOD_INIT_FAILED
} TOD_STATUS;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern TOD_STATUS BcmTod_init(void);
extern int BcmTod_isInitialized(void);
extern TOD_STATUS BcmTod_add( char *username, char *mac, unsigned char days, unsigned short int start_time, unsigned short int end_time );
extern TOD_STATUS BcmTod_remove( char *username );
void *BcmTod_get( void *previous, char *username, char *mac, unsigned char *days, unsigned short int *start_time, unsigned short int *end_time );
extern TOD_STATUS BcmTod_Store(void);

void BcmTod_unInit(void);
//add by l66195 for VDF start
int getTodNumberOfEntries(void);
//add by l66195 for VDF end    
#if defined(__cplusplus)
}
#endif

#endif
