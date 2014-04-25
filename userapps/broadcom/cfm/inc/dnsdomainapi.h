/***************************************************************************
* Copyright (C), 2006-2007, Huawei Tech. Co., Ltd.
* FileName: Dnsdomainapi.h
* Author: huawei  Version: v1.0  Date: 2007-02-07
* History:
* <author>  <time>   <version >   <desc>
*
*
* Description: Define the AtpDDns functions.
*
***************************************************************************/

#ifndef __DNS_DOMAIN_API_H__
#define __DNS_DOMAIN_API_H__

#include "ifcdefs.h"


#if defined(__cplusplus)
extern "C" {
#endif
extern void  AtpDnsDomain_init(void);
extern void AtpDnsmasq_serverRestart(int  iFlag);
#if defined(__cplusplus)
}
#endif

#endif
