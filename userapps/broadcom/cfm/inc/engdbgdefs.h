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
//  Filename:       Eng Dbg Defs.
//  Author:         Srini.
//  Creation Date:  2005 Aug
//
******************************************************************************
//  Description:
//      Eng Debug Configuration Structures.
//
*****************************************************************************/

#ifndef __ENGDBG_DEFS_H
#define __ENGDBG_DEFS_H

#include <ifcdefs.h>
#include <portMirror.h>

/********************** Global Constants ************************************/

#define ENGDBG_APPID                 "EngDbgCfg"
#define DCM_PORT_MIRROR_STATUS_ID    1
#define DCM_PORT_MIRROR_CFG_TBL_ID   2

#define WAN_PORT_MONITORING          1
#define MAX_MONITOR_PORTS            WAN_PORT_MONITORING

typedef struct _PortMirrorCfg {
    int        recNo ;
    int        monitorPort ;         /* Supported is Wan Port */
    MirrorCfg  mirrorCfg ;
} PortMirrorCfg ;

typedef enum {
   PORTMIRROR_STS_OK = 0,
   PORTMIRROR_STS_ERR_GENERAL,
   PORTMIRROR_STS_ERR_MEMORY,
   PORTMIRROR_STS_ERR_INVALID_OBJECT,
   PORTMIRROR_STS_ERR_OBJECT_NOT_FOUND,
   PORTMIRROR_STS_ERR_OBJECT_EXIST,
   PORTMIRROR_STS_ERR_OPEN_FILE
} PORTMIRROR_STATUS ;

typedef struct _DbgPortMirrorStatus {
   int count ;
} DbgPortMirrorStatus ;

#endif /* __ENGDBG_DEFS_H */
