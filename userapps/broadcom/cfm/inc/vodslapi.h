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
//  Filename:       voiceapi.h
//
******************************************************************************
//  Description:
//      Define the Voice Configuration Api functions.
//
*****************************************************************************/

#ifndef VOICE_API_H
#define VOICE_API_H

#ifndef PSTN_SUPPORT
#   if (NUM_FXO_CHANNELS > 0)
#      define PSTN_SUPPORT          1
#   else 
#      define PSTN_SUPPORT          0
#   endif 
#endif

#include <voipAppDefs.h>


#define VOICE_APPID           "Voice"
#define VOICE_MGCPINFO_ID     1
#define VOICE_SIPINFO_ID      2
#define VOICE_LESINFO_ID      3
/* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
#define VOICE_SIP_SRV_NUM_ID      4
#define VOICE_SIP_SRV_TBL_ID      5
#define VOICE_SIP_BSC_NUM_ID      6
#define VOICE_SIP_BSC_TBL_ID      7
#define VOICE_SIP_ADV_ID          8
#define VOICE_SIP_QOS_ID          9
#define VOICE_SIP_PHONECFG_ID    10
#define VOICE_SIP_SPEEDDIAL_NUM_ID 11
#define VOICE_SIP_SPEEDDIAL_TBL_ID 12
#define VOICE_SIP_CALLFWD_NUM_ID 13
#define VOICE_SIP_CALLFWD_TBL_ID 14
/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
#define VOICE_SIP_PROV_ID          15
/* END:   Added by y67514, 2009/11/16 */
/* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */

#define CFG_MAX_HOST_STR_LEN           80
#define CFG_MAX_TAG_STR_LEN            5
#define CFG_MAX_IPADDR_STR_LEN         20
#define CFG_MAX_PORT_STR_LEN           8
#define CFG_MAX_GWNAME_STR_LEN         40
#define CFG_MAX_IFNAME_STR_LEN         16
#define CFG_MAX_CODEC_STR_LEN          8
#define CFG_MAX_EXTENSION_STR_LEN      20
#define CFG_MAX_PASSWORD_STR_LEN       20
#define CFG_MAX_LOCALENAME_STR_LEN     4
#define CFG_MAX_CODECNAME_STR_LEN      8
#define CFG_MAX_PTIME_STR_LEN          3


#ifndef XCFG_G729_SUPPORT
#define XCFG_G729_SUPPORT     0
#endif 

#ifndef XCFG_G7231_SUPPORT
#define XCFG_G7231_SUPPORT    0
#endif 

#ifndef XCFG_G726_SUPPORT
#define XCFG_G726_SUPPORT     0
#endif 

#ifndef XCFG_G728_SUPPORT
#define XCFG_G728_SUPPORT     0
#endif 

#ifndef XCFG_FAX_SUPPORT
#define XCFG_FAX_SUPPORT      0
#endif 

#ifndef XCFG_BV16_SUPPORT
#define XCFG_BV16_SUPPORT      0
#endif 

#ifndef XCFG_ILBC_SUPPORT
#define XCFG_ILBC_SUPPORT      0
#endif 

/********************** Include Files ***************************************/

#include <bcmtypes.h>

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmVoice_Init(void);
extern void BcmVoice_Uninit(void);
extern void BcmVoice_Store(void);
extern void BcmVoice_Retrieve(int isDefault);
extern void BcmVoice_GetVar(char *varName, char *varValue);
extern void BcmVoice_SetVar(char *varName, char *varValue);
/*start of add voice info minpengwei 20101104*/
extern void BcmVoice_ShowInfos(void);
extern void BcmVoice_ShowPortStatus( void );
extern void BcmVoice_ShowHistory( void );
extern void BcmVoice_ShowCli( void );
extern void BcmVoice_ShowActive( void );
/*end of add voice info minpengwei 20101104*/
extern void BcmVoice_ShowVars(void);
extern void BcmVoice_ShowCmdSyntax(void);
extern void BcmVoice_ProcessSetCmd(char* cmdLine);
extern void BcmVoice_ProcessDebugCmd(char* cmdLine);
extern void BcmVoice_ProcessSipTestCmd(char* cmdLine);
/* start of l68693 added 20090222: 支持ReInection配置 */ 
extern void BcmVoice_ProcessReInjectionCmd( char* cmdLine );
/* end of l68693 added 20090222: 支持ReInection配置 */ 
/*BEGIN: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
extern void BcmVoice_ProcessVoicePromptVisibleCmd( char* cmdLine );
/*END: Added by l00180792 @20130307 for Spain STICK_NOT_DETECTED_new*/
/*start of 支持sip补充业务配置功能by l129990,2009,3,26 */
extern void BcmVoice_ProcessStsCmd(char* cmdLine);
/*end of 支持sip补充业务配置功能by l129990,2009,3,26 */
extern void BcmVoice_ProcessSetUserCmd( char* cmdLine );
extern void BcmVoice_ProcessSetHspaCmd( char* cmdLine );
extern void BcmVoice_Start(void);
//add by z67625
extern void BcmVoice_Start1(void);
extern void BcmVoice_Start2(void);
//add by z67625
/* BEGIN: Added by chenzhigang, 2008/8/6   问题单号:voice 守护*/
extern void BcmVoice_Numen(void);
/* END:   Added by chenzhigang, 2008/8/6 */
extern void BcmVoice_Stop(void);
extern void BcmVoice_EndptConsoleCmd( char* eptcmd );
extern int  BcmVoice_RunOnThisInterface( char *interface );
extern VOICE_APP_STATUS  BcmVoice_GetAppStatus(void);
#ifdef STUN_CLIENT
extern void BcmVoice_StunLkup(char *cmdLine);
#endif /* STUN_CLIENT */
#if defined(__cplusplus)
}
#endif

#endif
