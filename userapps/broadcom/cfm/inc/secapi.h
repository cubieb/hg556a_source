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
//  Filename:       secapi.h
//  Author:         Peter T. Tran
//  Creation Date:  06/11/02
//
******************************************************************************
//  Description:
//      Define the ScmApi functions.
//
*****************************************************************************/

#ifndef __SCM_API_H__
#define __SCM_API_H__

/********************** Include Files ***************************************/

#include "secdefs.h"

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
extern void BcmScm_init(void);
extern void BcmScm_unInit(void);
extern void *BcmScm_getVirtualServer(void *pVoid, PSEC_VRT_SRV_ENTRY pObject);
extern SEC_STATUS BcmScm_addVirtualServer(PSEC_VRT_SRV_ENTRY pObject);
extern SEC_STATUS BcmScm_removeVirtualServer(PSEC_VRT_SRV_ENTRY pObject);
/*start of HG553 2008.06.11 V100R001C02B017SP01 AU8D00718 by c65985 */
extern SEC_STATUS BcmScm_removeVirtualServerIptable(PSEC_VRT_SRV_ENTRY pObject);
/*end of HG553 2008.06.11 V100R001C02B017SP01 AU8D00718 by c65985 */
extern SEC_STATUS BcmScm_findVirtualServerByPort(char *srvPort);
extern void *BcmScm_getPortTrigger(void *pVoid, PSEC_PRT_TRG_ENTRY pObject);
extern SEC_STATUS BcmScm_addPortTrigger(PSEC_PRT_TRG_ENTRY pObject);
extern SEC_STATUS BcmScm_removePortTrigger(PSEC_PRT_TRG_ENTRY pObject);
extern void *BcmScm_getFilterOut(void *pVoid, PSEC_FLT_ENTRY pObject);

/*modified by z67625 为支持开关增加函数接口mode参数 start*/
extern SEC_STATUS BcmScm_addFilterOut(PSEC_FLT_ENTRY pObject, int mode);
extern SEC_STATUS BcmScm_removeFilterOut(char *fltName, int mode);
/*modified by z67625 为支持开关增加函数接口mode参数 end*/

extern void *BcmScm_getFilterIn(void *pVoid, PSEC_FLT_ENTRY pObject);

/*modified by z67625 为支持开关增加函数接口mode参数 start*/
extern SEC_STATUS BcmScm_addFilterIn(PSEC_FLT_ENTRY pObject, int mode);
extern SEC_STATUS BcmScm_removeFilterIn(char *fltName, int mode);
/*modified by z67625 为支持开关增加函数接口mode参数 end*/

extern void *BcmScm_getAccessControl(void *pVoid, PSEC_ACC_CNTR_ENTRY pObject);
extern SEC_STATUS BcmScm_addAccessControl(PSEC_ACC_CNTR_ENTRY pObject);
extern SEC_STATUS BcmScm_removeAccessControl(PSEC_ACC_CNTR_ENTRY pObject);
extern int BcmScm_isInAccessControlList(char *address);
extern int BcmScm_getAccessControlMode(void);
extern void BcmScm_setAccessControlMode(int mode);
//add by z67625 acl保存问题
extern uint32 BcmScm_getMaxHostId(void);
extern uint32 BcmScm_getHostId(PSEC_ACC_CNTR_ENTRY pObject);
//add by z67625 acl保存问题
extern void BcmScm_setSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo);
/* BEGIN: Added by weishi kf33269, 2011/6/25   PN:Issue409:CPE open an close the telnet from access outside of access control list*/
extern void BcmScm_setSrvTelnetRule();
/* END:   Added by weishi kf33269, 2011/6/25 */

extern void BcmScm_getSrvCntrInfo(PSEC_SRV_CNTR_INFO pInfo);
extern int BcmScm_isServiceEnabled(char *name, int where);  
/*start of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
extern void BcmScm_setDmzHost(char *enable, char *address);
extern void BcmScm_getDmzHost(PSEC_DMZ_ENTRY pstSecDmzEntry);
/*end of 支持global dmz新页面兼容功能 by l129990,2009,2,9*/
extern void *BcmScm_getMacFilter(void *pVoid, PSEC_MAC_FLT_ENTRY pObject);
extern SEC_STATUS BcmScm_addMacFilter(PSEC_MAC_FLT_ENTRY pObject);
extern SEC_STATUS BcmScm_removeAllMacFilter(void);
extern SEC_STATUS BcmScm_removeMacFilter(PSEC_MAC_FLT_ENTRY pObject);
extern int BcmScm_getMacPolicy(void);
extern int BcmScm_toggleMacPolicy(void);
extern void *BcmQos_getQosClass(void *pVoid, PQOS_CLASS_ENTRY pObject);
extern SEC_STATUS BcmQos_addQosClass(PQOS_CLASS_ENTRY pObject);
extern SEC_STATUS BcmQos_removeQosClass(char *clsName, int type);
/* start of protocol 模式切换需要删除所有的QoS规则 by zhangchen z45221 2006年7月28日 */
extern SEC_STATUS BcmQos_removeAllQosClass();
/* end of protocol 模式切换需要删除所有的QoS规则 by zhangchen z45221 2006年7月28日 */

extern SEC_STATUS BcmQos_findQosClassByName(char *clsName, int type);
extern void BcmQos_doWirelessQosPolicy(void);
extern void BcmQos_removeWirelessQosPolicy(void);
extern void BcmQos_doDefaultDSCPMarkPolicy(void);
extern void BcmQos_removeDefaultDSCPMarkPolicy(void);
/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年7月1日" */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
extern void BcmScm_setWredInfo(PSEC_WRED_INFO pInfo);
extern void BcmScm_getWredInfo(PSEC_WRED_INFO pInfo);
#endif
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年7月1日" */
extern int BcmScm_getLeftMostOneBits(int num);
extern int BcmScm_getLeftMostOneBitsInMask(char *mask);

/*start add for vdf qos by p44980 2008.1.2*/
#ifdef SUPPORT_VDF_QOS
extern SEC_STATUS BcmQos_addQosClassEx(PQOS_CLASS_ENTRY pObject, int lNeedCfg) ;
extern SEC_STATUS BcmQos_removeQosClassById(uint32 ulQosClsId, int type);
extern SEC_STATUS BcmQos_addQosQueue(PQOS_QUEUE_ENTRY pObject);
extern SEC_STATUS BcmQos_removeQosQueue(uint32 ulQosQueueId);
extern void* BcmQos_getQosQueue(void *pVoid, PQOS_QUEUE_ENTRY pObject);
extern void  BcmQos_getQosCfg(int *bEnable);
extern SEC_STATUS BcmQos_setQosCfg(int bEnable);

extern SEC_STATUS BcmQos_addQosService(PQOS_SERVICE_ENTRY pObject);
extern SEC_STATUS BcmQos_removeQosService(uint32 ulInstanceId);
extern void* BcmQos_getQosService(void *pVoid, PQOS_SERVICE_ENTRY pObject);
extern SEC_STATUS BcmQos_addQosServiceCfg(PQOS_SERVICE_CFG_ENTRY pObject);
extern SEC_STATUS BcmQos_removeQosServiceCfg(PQOS_SERVICE_CFG_ENTRY pObject);
extern void* BcmQos_getQosServiceCfg(void *pVoid, PQOS_SERVICE_CFG_ENTRY pObject);
extern uint32 BcmQos_getQosMaxQueue();
extern uint32 BcmQos_getQosDefaultQueue();
extern SEC_STATUS BcmQos_getQosClsStatus(PQOS_CLASS_ENTRY pObject);
extern SEC_STATUS BcmQos_getQosQueueStatus();

#endif
/*end add for vdf qos by p44980 2008.1.2*/

#if defined(__cplusplus)
}
#endif

#endif
