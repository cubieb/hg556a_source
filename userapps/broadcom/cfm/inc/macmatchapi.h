#ifdef SUPPORT_MACMATCHIP


/*****************************************************************************
//
//  Filename:       macmatchapi.h
//
******************************************************************************
//  Description:
//      Define the BcmMacMatchip functions.
//
*****************************************************************************/

#ifndef __MACMATCH_API_H__
#define __MACMATCH_API_H__

#include "ifcdefs.h"

#define MAC_LEN     6
#define IP_ADDR_LEN 16

#define MACMATCH_APP_ID             "MacMatchCfg"
#define MACMATCH_NUM_ENTRIES        1
#define	MACMATCH_ENTRY_TABLE        0

typedef struct MacMatchEntry {
  char username[IFC_MEDIUM_LEN];
  char mac[MAC_LEN];
  char ipaddress[16];
} MACMATCH_ENTRY, *PMACMATCH_ENTRY;

typedef enum
{
    MACMATCH_COMMAND_TYPE_ADD = 0,
    MACMATCH_COMMAND_TYPE_INSERT = 1,
    MACMATCH_COMMAND_TYPE_REMOVE = 2
} MACMATCH_COMMAND_TYPE_EN;

// Return codes for the various functions
typedef enum _macMatchStatus {
   MACMATCH_OK,
   MACMATCH_STORE_FAILED,
   MACMATCH_NOT_INIT,
   MACMATCH_BAD_RULE,
   MACMATCH_RULE_EXISTS,
   MACMATCH_RULE_NAME_EXISTS,
   MACMATCH_RULE_IP_EXISTS,
   MACMATCH_ADD_FAIL,
   MACMATCH_INIT_FAILED
} MACMATCH_STATUS;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern MACMATCH_STATUS BcmMacMatch_init(void);
extern int BcmMacMatch_isInitialized(void);
extern MACMATCH_STATUS BcmMacMatch_add( char *username, char *mac, char *ipaddress);
extern MACMATCH_STATUS BcmMacMatch_remove( char *mac );
void *BcmMacMatch_get( void *previous, char *username, char *mac, char *ipaddress);
extern MACMATCH_STATUS BcmMacMatch_Store(void);
void BcmMacMatch_unInit(void);
extern int getMacMatchNumberOfEntries(void);
/*start of 支持global lanDhcpSrv配置页面 by l129990 2009,2,13*/
extern MACMATCH_STATUS BcmMacMatch_removeAll(void);
/*end of 支持global lanDhcpSrv配置页面 by l129990 2009,2,13*/
extern MACMATCH_STATUS BcmMacMatch_clearAllMacList(void);
#if defined(__cplusplus)
}
#endif

#endif

#endif
