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
//  Filename:       certdefs.h
//  Author:         Jeffrey Chiao
//  Creation Date:  06/10/02
//
//  Description:
//      Define the Global structs, enumerations for certificate configuations.
//
*****************************************************************************/

#ifndef __CERT_DEFS_H
#define __CERT_DEFS_H

#include "ifcdefs.h"

/********************** Include Files ***************************************/

/********************** Global Types ****************************************/

typedef enum {
   CERT_STS_OK = 0,
   CERT_STS_ERR_GENERAL,
   CERT_STS_ERR_MEMORY,
   CERT_STS_ERR_FILE,
   CERT_STS_ERR_KEYGEN,
   CERT_STS_ERR_OBJECT_NOT_FOUND
} CERT_STATUS;

typedef enum {
   CERT_COMMAND_TYPE_ADD = 0,
   CERT_COMMAND_TYPE_INSERT,
   CERT_COMMAND_TYPE_REMOVE
} CERT_COMMAND_TYPE;

/********************** Global Constants ************************************/

#define CERT_APP_ID                "CertCfg"

#define CERT_LOCAL_TBL_SIZE       1
#define CERT_LOCAL_TBL            2
#define CERT_CA_TBL_SIZE          3
#define CERT_CA_TBL               4

#define CERT_LOCAL_MAX_ENTRY      4
#define CERT_CA_MAX_ENTRY         4

#define CERT_BUFF_MAX_LEN         300
#define CERT_NAME_LEN             64
#define CERT_TYPE_SIGNING_REQ     "request"
#define CERT_TYPE_SIGNED          "signed"
#define CERT_TYPE_CA              "ca"

#define CERT_KEY_MAX_LEN          1400

#define CERT_TYPE_LEN             8

#define CERT_KEY_SIZE             1024

#define CERT_LOCAL 1
#define CERT_CA 2

#define CERT_PATH             "/var/cert"
#define CERT_CONF_PATH        "/var/cert/openssl.cnf"
#define CERT_REQUEST          "BEGIN CERTIFICATE REQUEST"
#define CERT_TEMP_NAME        "__temp__"


/********************** Global Structs ************************************/

typedef struct {
   char *varName;
   char *varValue;
} CERT_ITEM, *PCERT_ITEM;

typedef struct {
   char certName[CERT_NAME_LEN];
   char commonName[CERT_NAME_LEN];
   char country[CERT_BUFF_MAX_LEN];
   char state[CERT_NAME_LEN];
   char organization[CERT_NAME_LEN];
} CERT_ADD_INFO, *PCERT_ADD_INFO;

typedef struct {
   char certName[CERT_NAME_LEN];
   char certSubject[CERT_NAME_LEN];
   char certType[CERT_TYPE_LEN];
   char certPassword[CERT_BUFF_MAX_LEN];
   char certContent[CERT_KEY_MAX_LEN];
   //add by z67625 for vpn support rcert
   char certRContent[CERT_KEY_MAX_LEN];
   //add by z67625 for vpn support rcert
   char certPrivKey[CERT_KEY_MAX_LEN];
   char certReqPub[CERT_KEY_MAX_LEN];
   int refCount;
} CERT_DATA_ENTRY, *PCERT_DATA_ENTRY;

#endif

