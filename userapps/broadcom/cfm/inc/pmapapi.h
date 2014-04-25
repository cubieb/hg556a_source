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
//  Filename:       pmapapi.h
//  Author:         Pavan Kumar
//  Creation Date:  09/03/04
//
******************************************************************************
//  Description:
//      Define the Port Mapping Api functions.
//
*****************************************************************************/

#ifndef __PMAP_API_H__
#define __PMAP_API_H__

/********************** Include Files ***************************************/

#include "pmapdefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmPMcm_init(void);
extern void BcmPMcm_unInit(void);
extern void BcmPMcm_setupPMap(void);
extern PORT_MAP_STATUS BcmPMcm_addPMapCfg(char *groupName, char *grpedIntf,
                                          PORT_MAP_DHCP_VENDORID_ENTRY *vid);
extern PORT_MAP_STATUS BcmPMcm_removePMapCfg(char *groupName);
extern void *BcmPMcm_getPMapCfg(void *pVoid, char *groupName, char *grpedIntf);
extern int BcmPMcm_isInitialized(void);
extern int BcmDb_applyBridgeCfg(void);

/*Start add by w00135358 2009.6.27*/
unsigned char BcmGetVportSts();
void BcmVportEnable(char *ifName);
void BcmVportDisable(char *ifName);
/*End add by w00135358 2009.6.27*/



#if defined(__cplusplus)
}
#endif

#endif
