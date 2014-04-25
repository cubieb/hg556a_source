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
//  Filename:       psidefs.h
//  Author:         Peter T. Tran
//  Creation Date:  12/26/01
//
******************************************************************************
//  Description:
//      Define the PsiDefs functions.
//
*****************************************************************************/

#ifndef __PSIDEFS_H__
#define __PSIDEFS_H__

/********************** Include Files ***************************************/

#include <stdio.h>
#include "bcmtypes.h"

/********************** Global Constants ************************************/
#define PSI_MAGIC_LEN   16
#define PSI_HEADER_LEN  (PSI_MAGIC_LEN + sizeof(UINT16))
#define PSI_VERSION_LEN 6

#define PSI_OPTIONS 			0
#define PSI_PRIORITY			20
#define PSI_STACK_SIZE			5000
#define PSI_PROFILE_NUM_MAX             1
#define PSI_PROFILE_NAME_LEN_MAX        16

// ratio of PSI buffer size in RAM and xml PSI size
// xml PSI is more compact. As data have variable size, no space is wasted 
#define PSI_BUFFER_TO_XML_FACTOR        4

#define PSI_VALUE	void*
#define PSI_HANDLE	void*

#define NUM_PORTS       1
#define PSI_PATH_DEFAULTCONFIG		"/etc/default.cfg"
#define PSI_PATH_RUNTIMECONFIG		"/var/backupsettings.cfg"
#define PSI_XML_PATH_FLASHCONFIG        "/var/psi.xml"
/*start:modified by kf19988 for improve restore*/
#define PSI_XML_CHECK_RESTORE			"/var/checkrestore.xml"
#define DIAG_MESSAGE					"/var/message"
#define LINE_LEN							1024
#define CHECK_ERROR						-1
#define COUNT							2
#define PA_LEN							128
/*end:modified by kf19988 for improve restore*/
#define PSI_XML_PATH_RUNTIMECONFIG      "/var/psiRunning.xml"
#define PSI_XML_FLASH_CONFIG            1
#define PSI_XML_RUNNING_CONFIG          2
#define PSI_XML_ALL_CONFIG              3
#define PSI_NUMBER_OF_CHANGES           "/var/psiNumChanges"
#define PSI_BUFF_SIZE                   128

/********************** Global Enums ****************************************/

typedef enum _psiStatus {
   PSI_STS_OK = 0,
   PSI_STS_ERR_GENERAL,
   PSI_STS_ERR_MEMORY,
   PSI_STS_ERR_OBJECT_NOT_FOUND,
   PSI_STS_ERR_APP_NOT_FOUND,
   PSI_STS_ERR_OBJECT_EXIST,
   PSI_STS_ERR_APP_EXIST,
   PSI_STS_ERR_INVALID_LENGTH,
   PSI_STS_ERR_INVALID_VALUE,
   PSI_STS_ERR_INVALID_CRC,
   PSI_STS_ERR_INVALID_MAGIC_VALUE,
   PSI_STS_ERR_INVALID_PROFILE_NUM,
   PSI_STS_ERR_PROFILE_NAME,
   PSI_STS_ERR_PROFILE_TOO_LARGE,
   PSI_STS_ERR_CONFIGFILE_UNKNOWN,
   PSI_STS_ERR_CONFIGFILE_SIZE
} PSI_STATUS;

#endif
