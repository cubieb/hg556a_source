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
//  Filename:       dbgcfgapi.h
//  Author:         Srini.
//  Creation Date:  
//
******************************************************************************
//  Description:
//      Define the DBG API functions.
//        Current application, as of this date is, PORT Mirroring on the WAN
//        side.
//
*****************************************************************************/

#ifndef __DBGCFG_API_H
#define __DBGCFG_API_H

/********************** Include Files ***************************************/

#include "engdbgdefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void  BcmDcm_init(void) ;
extern void  BcmDcm_unInit(void) ;
extern PORTMIRROR_STATUS BcmDcm_addPortMirrorCfg (PortMirrorCfg*) ;
extern PORTMIRROR_STATUS BcmDcm_removePortMirrorCfg (int) ;
extern void* BcmDcm_getPortMirrorCfg (void*, PortMirrorCfg*) ;
extern PORTMIRROR_STATUS BcmDcm_setPortMirrorCfg (PortMirrorCfg*) ;
extern PORTMIRROR_STATUS BcmDcm_getPortMirrorStatus(DbgPortMirrorStatus *pObject);
extern PORTMIRROR_STATUS BcmDcm_setPortMirrorStatus(DbgPortMirrorStatus *pObject);
extern int BcmDcm_isPortMirrorCfgEnabled (int) ;
extern int BcmDcm_isInitialized (void);
extern PORTMIRROR_STATUS BcmDcm_effectPortMirrorCfg (void);
#if defined(__cplusplus)
}
#endif

#endif /* end of #ifdef __DBGCFG_API_H */
