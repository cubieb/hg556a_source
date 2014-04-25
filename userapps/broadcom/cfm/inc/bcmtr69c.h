/*****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  Filename:       bcmtr69c.h
//  Author:         Peter T. Tran
//  Creation Date:  06/15/05
//
******************************************************************************
//  Description:
//      Define the tr69c data struct to be saved in psi
//
*****************************************************************************/

#ifndef __BCM_TR69C_H__
#define __BCM_TR69C_H__
/********************** Global Functions ************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#include "bcmcfm.h"

#define TR69C_KEY_LEN             34
#define TR69C_CODE_LEN          66
#define TR69C_URL_LEN           258

/*start  of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */
#define TR69C_PASS_LEN         350  
/*end of 修改问题单AU8D00801 配置密码256个字符导致网关恢复出厂设置 by s53329  at  20080715 */

typedef struct tr69cInfo {
    BcmCfm_CfgStatus     status;
    uint8                         upgradesManaged;
    uint8                         upgradeAvailable;
    uint8                         informEnable;   
    uint32                        informTime;    
    uint32                        informInterval;
    char                          acsURL[TR69C_URL_LEN];
    char                          acsUser[TR69C_URL_LEN];
    char                          acsPwd[TR69C_URL_LEN];
    char                          parameterKey[TR69C_KEY_LEN]; 
    char                          connReqURL[TR69C_URL_LEN];
    char                          connReqUser[TR69C_URL_LEN];
    char                          connReqPwd[TR69C_URL_LEN];
    char                          kickURL[TR69C_URL_LEN];
    char                          provisioningCode[TR69C_CODE_LEN];
} BCM_TR69C_INFO, *PBCM_TR69C_INFO;

#if defined(__cplusplus)
}
#endif

#endif /* __BCM_TR69C_H__ */
