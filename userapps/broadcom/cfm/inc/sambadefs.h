/*****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
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
//  Filename:       SAMBA Defs.
//  Author:         ...
//  Creation Date:  2005 Aug
//
******************************************************************************
//  Description:
//      Samba Configuration Structures.
//
*****************************************************************************/

#ifndef __SAMBA_DEFS_H
#define __SAMBA_DEFS_H

#include "ifcdefs.h"

#define SAMBA_APPID                 "SambaCfg"
#define SAMBA_STATUS_ID             1
#define SAMBA_ACCOUNT_NUM_ID        2
#define SAMBA_ACCOUNT_TBL_ID        3

#define SAMBA_MAX_ACCOUNT           5

#define SAMBA_PRIVILEGE_RONLY       1
#define SAMBA_PRIVILEGE_RW          2

#define SAMBA_CONF_FILE             "/etc/samba/smb.conf"
#define SAMBA_PASSWD_FILE           "/etc/samba/smbpasswd"
#define SAMBA_USER_FILE             "/etc/samba/smbusers"

typedef enum {
   SMB_STS_OK = 0,
   SMB_STS_ERR_GENERAL,
   SMB_STS_ERR_MEMORY,
   SMB_STS_ERR_OBJECT_NOT_FOUND
} SMB_STATUS;

typedef struct {
   uint32 enableSamba;
   char   workgrp[IFC_HOST_LEN];
   char   netbname[IFC_HOST_LEN];
   char   svrname[IFC_HOST_LEN];
}SAMBA_MGMT_CFG, *PSAMBA_MGMT_CFG;

typedef struct {
   uint32 instId;
   char   userName[IFC_SMALL_LEN];
   char   passwd[IFC_SMALL_LEN];
   char   dir[IFC_LARGE_LEN];
   uint32 privilege;
}SAMBA_ACCNT_ENTRY, *PSAMBA_ACCNT_ENTRY;

#endif