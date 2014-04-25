//create by l66195 for VDF

/*****************************************************************************
//
//  Filename:       macapi.h
//
******************************************************************************
//  Description:
//      Define the BcmMac functions.
//
*****************************************************************************/

#ifndef __MAC_API_H__
#define __MAC_API_H__

#include "ifcdefs.h"

#define MAC_LEN 6

#define MAC_APP_ID             "MacCfg"
#define MAC_NUM_ENTRIES        1
#define	MAC_ENTRY_TABLE        0

typedef struct MacEntry {
  char username[IFC_MEDIUM_LEN];
  char mac[MAC_LEN];
} MAC_ENTRY, *PMAC_ENTRY;

#define MAC_BLACK_NAME               0 //黑名单
#define MAC_WHITE_NAME               1 //白名单
typedef enum
{
    MAC_COMMAND_TYPE_ADD = 0,
    MAC_COMMAND_TYPE_INSERT = 1,
    MAC_COMMAND_TYPE_REMOVE = 2
} MAC_COMMAND_TYPE_EN;

// Return codes for the various functions
typedef enum _macStatus {
   MAC_OK,
   MAC_STORE_FAILED,
   MAC_NOT_INIT,
   MAC_BAD_RULE,
   MAC_RULE_EXISTS,
   MAC_RULE_NAME_EXISTS,
   MAC_ADD_FAIL,
   MAC_INIT_FAILED
} MAC_STATUS;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern MAC_STATUS BcmMac_init(void);
extern int BcmMac_isInitialized(void);
extern MAC_STATUS BcmMac_add( char *username, char *mac );
//extern MAC_STATUS BcmMac_remove( char *username );
extern MAC_STATUS BcmMac_remove( char *mac );
void *BcmMac_get( void *previous, char *username, char *mac);
extern MAC_STATUS BcmMac_Store(void);
extern MAC_STATUS SecMacFltPolicyChange(int bMacFltCurPolicy);
extern MAC_STATUS SecMacFltSetDefaultRules(MAC_COMMAND_TYPE_EN enMacCmd);
/*add by z67625 新页面中mac过滤使能规则生效函数声明 start*/
extern MAC_STATUS BcmMac_ControlAllRule(MAC_COMMAND_TYPE_EN mode);
extern MAC_STATUS BcmMac_DoOtherRule(MAC_COMMAND_TYPE_EN mode);
/*add by z67625 新页面中mac过滤使能规则生效函数声明 end*/

void BcmMac_unInit(void);
int getMacNumberOfEntries(void);

extern MAC_STATUS BcmMac_clearAllMacList(void);
#if defined(__cplusplus)
}
#endif

#endif
