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
//  Filename:      cfmsocketapi.h
//  Created by:    Xi Wang
//  Creation Date:  12/14/05
//
//  Description: CFM entry point for Unix domain socket based IPC, called from httpd.c
//             (not related to BcmCfm* functions)
//
//  Handlers included: Voice ATM QoS handler
//      
//*****************************************************************************/


#ifndef __CFM_SOCKET_API_H__
#define __CFM_SOCKET_API_H__

#include "ifcdefs.h"

struct CFMAPI_MSGHEADER {
    UINT16 type;
};

/* MSG type */
#define CFMAPI_APP_VOICEQOS             1
#define CFMAPI_APP_VOICEQOS_WANINFO     2
#define CFMAPI_APP_VOICEFIREWALL             3

/* QoS Action */
#define CFMAPI_VOICEQOS_ADDPORT         1
#define CFMAPI_VOICEQOS_REMOVEPORT      2

/* Firewall Action */
#define CFMAPI_VOICEFIREWALL_ADDPORT         1
#define CFMAPI_VOICEFIREWALL_REMOVEPORT      2
                                                                                

struct VOIPQOSMSG {
    UINT16 action;
    UINT16 port;
};

struct VOIPFIREWALLMSG {
    UINT16 action;
    UINT16 port;
};


#if defined(__cplusplus)
extern "C" {
#endif

void BcmCfmApi_requestMain(int api_fd);

void BcmCfmApi_handleVoiceQos(char *m, int msgsize);
void BcmCfmApi_vqosRegisterWanInfo(char *m, int msgsize);

void BcmCfmApi_handleVoiceFirewall(char *m, int msgsize);

#if defined(__cplusplus)
}
#endif

#endif





