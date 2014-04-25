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
//  Filename:       cliApi.h
//  Author:         Peter T. Tran
//  Creation Date:  02/02/01
//
******************************************************************************
//  Description:
//      Define the PsiApi functions.
//
******************************************************************************/

#ifndef __CLIAPI_H__
#define __CLIAPI_H__

/********************** Include Files ***************************************/
#include "clidefs.h"
 #include <stdio.h>
/*start: by xkf19988 2009.11.11*/
#define MIN_RATE 	21
/*end:by xkf19988 2009.11.11 */ 

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef BUILD_MENU_DRIVEN_CLI
extern CLI_STATUS BcmCli_addSubMenu2RootMenu(char* menu, char* nextMenu);
extern CLI_STATUS BcmCli_addSubMenu2Menu(char* menu, char* nextMenu,
                                         char* prevMenu);
extern CLI_STATUS BcmCli_addCommand(char* itemName, char* itemMenu,
                                    char* cmdName, char* cmdHelp,
                                    void* cmdObject,
                                    CLI_FNC cliFnc, char* menuName);
extern CLI_STATUS BcmCli_addMenu(const PCLI_MENU menu);
extern void BcmCli_init(void);
extern void BcmCli_dispose(void);

extern void BcmCli_cliPrintf(char* msgp, ...);
extern void BcmCli_cliRdString(char* buf, int size);

extern CLI_BOOL BcmCli_isNumber(char* buf);
extern CLI_BOOL BcmCli_isHexNumber(char* buf);
extern void BcmCli_getArgs(char *cmdLine, char *cmdSyntax,
                           char *cmdArgs, int size);
extern void BcmCli_getParams(char *cmdPrompt, char *cmdParams, int size);
extern void BcmCli_parseCmdLine(char *cmdLine, char *cmdName, char *cmdArgs);
extern CLI_BOOL BcmCli_getParameter(PPARAMS_SETTING inParam);
extern void BcmCli_showParameter(PPARAMS_SETTING inParam);
extern void BcmCli_getItemFromList(char **list, UINT16 numList, char *item);
extern CLI_BOOL BcmCli_isValidPwdLen(char* buf);
#endif /* BUILD_MENU_DRIVEN_CLI */

/* start of maintain dying gasp by liuzhijie 00028714 2006年5月13日
extern void BcmCli_run(int where);
*/
extern void BcmCli_run(int where, unsigned long in_addr);
/* end of maintain dying gasp by liuzhijie 00028714 2006年5月13日 */
extern void BcmCli_cliPrintf(char* msgp, ...);
extern CLI_BOOL BcmCli_isIpAddress(char* buf);
extern CLI_BOOL BcmCli_isMacAddress(char* buf);
extern void BcmTelnet_Init( void  );
extern void BcmTelnet_Uninit( void  );
extern void BcmSSHD_Init(void);
extern void BcmBFTPD_Init(int lPort); 
extern void StartFtpFromWeb(FILE *fs,int lPort);
#if defined(__cplusplus)
}
#endif

#endif
