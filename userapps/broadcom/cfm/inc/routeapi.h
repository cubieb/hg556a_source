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
//  Filename:       routeapi.h
//  Author:         Peter T. Tran
//  Creation Date:  12/17/02
//
******************************************************************************
//  Description:
//      Define the Route Api functions.
//
*****************************************************************************/

#ifndef __ROUTE_API_H__
#define __ROUTE_API_H__

/********************** Include Files ***************************************/

#include "routedefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmRcm_init(void);
extern void BcmRcm_unInit(void);
extern void *BcmRcm_getRipIfc(void *pVoid, PRT_RIP_IFC_ENTRY pObject);
extern void *BcmRcm_getRipInterface(void *pVoid, PRT_RIP_IFC_ENTRY pObject);
extern ROUTE_STATUS BcmRcm_addRipInterface(PRT_RIP_IFC_ENTRY pObject);
extern ROUTE_STATUS BcmRcm_removeRipInterface(char *name);
extern ROUTE_STATUS BcmRcm_getRipConfiguration(PRT_RIP_CFG pObject);
extern ROUTE_STATUS BcmRcm_setRipConfiguration(PRT_RIP_CFG pObject);
extern ROUTE_STATUS BcmRcm_startRip(void);
extern int  BcmRcm_isRipInterfaceEnabled(char *name);
extern void BcmRcm_setupRoute(void);
 /*start of 修改问题单 AU8D00246 语音pvc增加静态路由问题 by s53329 at  20080225*/
extern void BcmRcm_setupStaticRoute(void);
 /*end  of 修改问题单 AU8D00246 语音pvc增加静态路由问题 by s53329 at  20080225*/
/*start of Enable or disable the forwarding entry by l129990,2009-11-6*/
extern void *BcmRcm_getRouteCfg(void *pVoid, uint32 *id, char *addr, char *mask, char *gtwy, char *wanIf, uint32 *entryControl);
extern ROUTE_STATUS BcmRcm_addRouteCfg(uint32 id, char *addr, char *mask, char *gtwy, char *wanIf, uint32 entryControl);
/*end of Enable or disable the forwarding entry by l129990,2009-11-6*/
extern ROUTE_STATUS BcmRcm_removeRouteCfg(char *addr, char *mask);
extern int BcmRcm_isInitialized(void);
extern ROUTE_STATUS BcmRcm_removeRouteCfgByWanIf(char *wanIf);
/* j00100803 Add Begin 2008-02-29 */
#ifdef SUPPORT_POLICY
extern void* BcmRcm_getRtPolicyCfg(void *pVoid, PIFC_RoutePolicy_Info pRtPolicyInfo);
extern ROUTE_STATUS BcmRcm_addRtPolicyCfg(int iType,
												  uint32 id, 
												  char *pMacAddr,
												  char *pSrcAddr, 
												  char *pDestAddr, 
												  char *pSrcIfcName,
												  char *pProtocol,
												  int8  nDscpValue,
												  char *pWanIfcName,
												  unsigned int uIndex);
extern ROUTE_STATUS BcmRcm_removeRtPolicyCfg(int iType,
												  uint32 id, 
												  char *pMacAddr,
												  char *pSrcAddr, 
												  char *pDestAddr, 
												  char *pSrcIfcName,
												  char *pProtocol,
												  int8   nDscpValue,
												  char *pWanIfcName,
												  unsigned int uIndex);
/*begin add by p44980 2008.10.16*/
extern ROUTE_STATUS BcmRcm_removeRtPolicyCfgByWanIf (char* wanIf);
/*end add by p44980 2008.10.16*/
#endif
/* j00100803 Add End 2008-02-29 */
#if defined(__cplusplus)
}
#endif

#endif
