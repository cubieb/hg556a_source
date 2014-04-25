/***********************************************************
Copyright (C), 2006-2007, Huawei Tech. Co., Ltd.
FileName: dnsserverapi.h
Author: huawei  Version: v1.0  Date: 2007-02-07
History:
<author>  <time>   <version >   <desc>
***********************************************************/

#ifndef __DNS_SERVER_API_H__
#define __DNS_SERVER_API_H__

#include "ifcdefs.h"

#define IFC_URL_LEN 24
#define DNS_PID_FILE "/var/run/dnsmasq.pid"
/*GLB:y67514,for DNS*/
#define DNS_TEMP_FILE    "/var/dnsTemp.conf"
/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
void killDnsServer();
void reloadDnsHosts();
void startUpDnsServer(const char *hostsfile);
#if defined(__cplusplus)
}
#endif

#endif
