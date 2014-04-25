
#ifndef __PPP_PROFILE_API_H__
#define __PPP_PROFILE_API_H__

/********************** Include Files ***************************************/

#include "ifcdefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern  void BcmPppProfile_init(void);
extern  void BcmPppProfile_unInit(void);
extern  void* BcmPppProfile_getProfileCfg(void *pVoid, PROFILE_INFO_ST *pPppprofileCfg);
extern  PPP_PROFILE_STATUS BcmPppProfile_addProfileCfg(PROFILE_INFO_ST* pObject);
extern  PPP_PROFILE_STATUS BcmPppProfile_removeProfileCfg(char *ProName);
extern  PPP_PROFILE_STATUS BcmPppProfile_findPronameByIndex(int index, char* Proname);
extern PPP_PROFILE_STATUS BcmPppProfile_findProfileCfgByIndex(int index, PROFILE_INFO_ST *pPppprofileCfg);
extern  PPP_PROFILE_STATUS BcmPppProfile_findpppNameByProname(char *proName, char *acUserName);
extern  PPP_PROFILE_STATUS BcmPppProfile_findpppPasswordByProname(char *proName, char *acPassword);
extern  PPP_PROFILE_STATUS BcmPppProfile_findAuthMethodByProname(char *proName, UINT8* authMethod);
extern  PPP_PROFILE_STATUS BcmPppProfile_findPhoneNumByProname(char *proName, char *acPhoneNum);
extern  PPP_PROFILE_STATUS BcmPppProfile_findPreDnsByProname(char *proName, char *preferredDns);
extern  PPP_PROFILE_STATUS BcmPppProfile_findAlterDnsByProname(char *proName, char *alternateDns);
extern  PPP_PROFILE_STATUS BcmPppProfile_findAutoDnsByProname(char *proName, UINT16* autoDns);
extern  PPP_PROFILE_STATUS BcmPppProfile_findAutoApnByProname(char *proName, UINT16* autoApn); 
extern  PPP_PROFILE_STATUS BcmPppProfile_findApnByProname(char *proName, char *acApn);
//extern int BcmPMcm_isInitialized(void);
//extern int BcmDb_applyBridgeCfg(void);
#if defined(__cplusplus)
}
#endif

#endif
