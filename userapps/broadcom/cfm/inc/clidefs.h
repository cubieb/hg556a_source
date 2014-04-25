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
//  Filename:       clidefs.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the global types, enums. and constants for CLI.
//
*****************************************************************************/

#ifndef __CLIDEFS_H__
#define __CLIDEFS_H__

#include "../../../../bcmdrivers/opensource/include/bcm963xx/bcmtypes.h"

/********************** Global Constants ************************************/

/* start of protocol QoS for TdE cmd by z45221 zhangchen 2006年9月14日
#define CLI_MAX_BUF_SZ       256
*/
#define CLI_MAX_BUF_SZ       512
/* end of protocol QoS for TdE cmd by z45221 zhangchen 2006年9月14日 */
#define CLI_BACKSPACE        '\x08'

#define UTIL_MILI_PER_SECS             1000
#define MAX_WAIT_TIME                     12*1000
/********************** Global Enums ****************************************/

typedef enum {
   CLI_FALSE = 0,
   CLI_TRUE
} CLI_BOOL;

typedef enum {
   CLI_ACCESS_DISABLED = 0,
   CLI_ACCESS_LOCAL,
   CLI_ACCESS_REMOTE,
   CLI_ACCESS_CONSOLE,
   CLI_ACCESS_REMOTE_SSH,     // for SSHD
   CLI_ACCESS_REMOTE_TELNET,
} CLI_ACCESS_WHERE;


typedef enum {
   CLI_STS_OK = 0,
   CLI_STS_ERR_GENERAL,
   CLI_STS_ERR_MEMORY,
   CLI_STS_ERR_FIND,
   CLI_STS_ERR_MENU_EXIST,
   CLI_STS_ERR_CMD_EXIST
} CLI_STATUS;

#ifdef BUILD_MENU_DRIVEN_CLI
typedef enum {
   CLI_PROCESS_PROMPT = 0,
   CLI_PROCESS_HELP,
   CLI_PROCESS_LINE
} CLI_PROCESS;

/********************** Global Types ****************************************/

typedef CLI_BOOL (*CLI_VALIDATE_FNC) (char* inputParam);

typedef struct {
   char* prompt;
   char  param[CLI_MAX_BUF_SZ];
   CLI_VALIDATE_FNC validateFnc;
} PARAMS_SETTING, *PPARAMS_SETTING;

typedef CLI_STATUS (*CLI_FNC) (char* cmdName, int help, void* object);

typedef struct {
   char* itemName;
   char* itemMenu;
   char* cmdName;
   char* cmdHelp;
   void* cmdObject;
   CLI_FNC cliFnc;
} CLI_MENU_ITEM, *PCLI_MENU_ITEM;

typedef struct {
   char*          menuName;
   int            itemNumMax;
   PCLI_MENU_ITEM items;
} CLI_MENU, *PCLI_MENU;
#endif /* BUILD_MENU_DRIVEN_CLI */

#endif
