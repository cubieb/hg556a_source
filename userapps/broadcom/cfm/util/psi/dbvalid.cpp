/*****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
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
//  Filename:       dbvalid.cpp
//  Author:         Peter Tran
//  Creation Date:  05/06/04
//
******************************************************************************
//  Description:
//      Implement the BcmDb_validate* functions.
//
*****************************************************************************/

/********************** Include Files ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "bcmtypes.h"
#include "bcmatmapi.h"
#include "dbvalid.h"
#include "ifcdefs.h"
#include "syscall.h"
#include "wlapi.h"

/* start of PROTOCOL QoS for KPN by zhouyi 00037589 2006年7月24日 */
#include "dbapi.h"
/* end of PROTOCOL QoS for KPN by zhouyi 00037589 2006年7月24日 */


#if VOXXXLOAD
#   include <vrgCountryCfg.h>
#   include <voipLocale.h>
#endif

static const DB_OBJ_VALID validObjs[] = {
    { DB_TYPE_BOOLEAN, BcmDb_validateBoolean },
    { DB_TYPE_NUMBER, BcmDb_validateNumber },
    { DB_TYPE_STRING, BcmDb_validateString },
    { DB_TYPE_IP_ADDRESS, BcmDb_validateIpAddress },
    { DB_TYPE_SUBNET_MASK, BcmDb_validateSubnetMask },
    { DB_TYPE_MAC_ADDRES, BcmDb_validateMacAddress },
    { DB_TYPE_ATM_VPI, BcmDb_validateAtmVpi },
    { DB_TYPE_ATM_VCI, BcmDb_validateAtmVci },
    { DB_TYPE_ATM_CATEGORY, BcmDb_validateAtmCategory },
    { DB_TYPE_ATM_AAL_TYPE, BcmDb_validateAtmAalType },
    { DB_TYPE_ATM_ADMIN_STATUS, BcmDb_validateAtmAdminStatus },
    { DB_TYPE_ATM_ENCAPSULATION, BcmDb_validateAtmEncapsulation },
    { DB_TYPE_USERNAME, BcmDb_validateUserName },
    { DB_TYPE_PASSWORD, BcmDb_validatePassword },
    { DB_TYPE_PPP_USERNAME, BcmDb_validatePppUserName },
    { DB_TYPE_PPP_PASSWORD, BcmDb_validatePppPassword },
    { DB_TYPE_PPP_TIMEOUT, BcmDb_validatePppTimeout },
    { DB_TYPE_PPP_AUTH_METHOD, BcmDb_validatePppAuthMethod },
    { DB_TYPE_PROTOCOL, BcmDb_validateProtocol },
    { DB_TYPE_IP_PROTOCOL, BcmDb_validateIpProtocol },
    { DB_TYPE_MAC_PROTOCOL, BcmDb_validateMacProtocol },
    { DB_TYPE_INTERFACE_NAME, BcmDb_validateInterfaceName },
    { DB_TYPE_SERVICE_NAME, BcmDb_validateServiceName },
    { DB_TYPE_STATE, BcmDb_validateState },
    { DB_TYPE_DHCP_STATE, BcmDb_validateDhcpState },
    { DB_TYPE_ENCAPSULATION, BcmDb_validateEncapsulation },
    { DB_TYPE_ACCESS, BcmDb_validateAccess },
    { DB_TYPE_SYSLOG_LEVEL, BcmDb_validateSysLogLevel },
    { DB_TYPE_SYSLOG_OPTION, BcmDb_validateSysLogOption },
    { DB_TYPE_RIP_OPERATION, BcmDb_validateRipOperation },
    { DB_TYPE_MAC_FILTER_DIRECTION, BcmDb_validateMacFilterDirection },
    { DB_TYPE_MAC_FILTER_POLICY, BcmDb_validateMacFilterPolicy },
    { DB_TYPE_QOS_PRIORITY, BcmDb_validateQosPriority },
#ifdef WIRELESS
    { DB_TYPE_WIRELESS_COUNTRY, BcmDb_validateWirelessCountry },
    { DB_TYPE_WIRELESS_AUTH_MODE, BcmDb_validateWirelessAuthMode },
    { DB_TYPE_WIRELESS_ENCRYPTION, BcmDb_validateWirelessEncryption },
    { DB_TYPE_WIRELESS_AP, BcmDb_validateWirelessAP },
    { DB_TYPE_WIRELESS_XPRESS, BcmDb_validateWirelessXpress },
    { DB_TYPE_WIRELESS_AFTERBURNER, BcmDb_validateWirelessAfterBurner },
    { DB_TYPE_WIRELESS_BAND, BcmDb_validateWirelessBand },
    { DB_TYPE_WIRELESS_RATE, BcmDb_validateWirelessRate },
    { DB_TYPE_WIRELESS_BASIC_RATE, BcmDb_validateWirelessBasicRate },
    { DB_TYPE_WIRELESS_GMODE, BcmDb_validateWirelessGmode },
    { DB_TYPE_WIRELESS_GPROTECTION, BcmDb_validateWirelessOnOffAuto },
    { DB_TYPE_WIRELESS_KEYBIT, BcmDb_validateWirelessKeybit },
    { DB_TYPE_WIRELESS_FLT_MAC_MODE, BcmDb_validateWirelessFltMacMode },
    { DB_TYPE_WIRELESS_AP_MODE, BcmDb_validateWirelessApMode },
    { DB_TYPE_WIRELESS_BRIDGE_RESTRICT_MODE, BcmDb_validateWirelessBridgeRestrictMode },
#ifdef SUPPORT_MIMO    
    { DB_TYPE_WIRELESS_BANDWIDTH, BcmDb_validateWirelessBandwidth },
    { DB_TYPE_WIRELESS_CTLSIDEBAND, BcmDb_validateWirelessCtrlSideband },    
#endif    
#endif /* #ifdef WIRELESS */
    { DB_TYPE_ADSL_PAIR, BcmDb_validateAdslPair },
#ifdef SUPPORT_TOD
    { DB_TYPE_ADSL_PAIR, BcmDb_validateTime },
    { DB_TYPE_ADSL_PAIR, BcmDb_validateDay },
#endif
#ifdef ETH_CFG
    { DB_TYPE_ETH_SPEED, BcmDb_validateEthSpeed },
    { DB_TYPE_ETH_TYPE, BcmDb_validateEthType },
#endif
    { DB_TYPE_NONE, NULL }
};

#if defined(__cplusplus)
extern "C" {
#endif

DB_OBJ_VALID_STATUS BcmDb_validateObj(DB_OBJ_TYPE objType, const char *objValue) {
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_TYPE;

    if ( objValue == NULL ) return DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( int i = 0; validObjs[i].validateObjFnc != NULL; i++ )
        if ( validObjs[i].objType == objType ) {
            status = (*(validObjs[i].validateObjFnc))(objValue);
            break;
        }

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateRange(const char *objValue, int min, int max) {
    DB_OBJ_VALID_STATUS status = BcmDb_validateNumber(objValue);

    if ( status == DB_OBJ_VALID_OK ) {
        int value = atoi(objValue);
        if ( value > max || value < min )
            status = DB_OBJ_VALID_ERR_OUT_OF_RANGE;
    }

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateLength(const char *objValue, unsigned int max) {
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_LENGTH;

    if ( strlen(objValue) < max )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateBoolean(const char *objValue) {
    long value = strtol(objValue, (char **)NULL, 10);
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if ( value == 1 || value == 0 )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateNumber(const char *objValue) {
    int i = 0, size = 0;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    char *str = (char *)objValue;
    
    /*start of 修改该函数不支持负数问题 by l68693 at 20080822 */
#if 0
    if ( objValue == NULL ) return status;

    size = strlen(objValue);

    for ( i = 0; i < size; i++ ) {
        if ( isdigit(objValue[i]) == FALSE )
            break;
    }
#else

    if ( str == NULL ) return status;

    if ('-' == str[0])
    {
        str = str + 1;
    }
    size = strlen(str);

    for ( i = 0; i < size; i++ ) {
        if ( isdigit(str[i]) == FALSE ) 
            break;
    }
#endif
    /*end of 修改该函数不支持负数问题 by l68693 at 20080822 */

    if ( size > 0 && i == size )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateHexNumber(const char *objValue) {
    int i = 0, size = 0;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if ( objValue == NULL ) return status;

    size = strlen(objValue);

    for ( i = 0; i < size; i++ ) {
        if ( isxdigit(objValue[i]) == FALSE )
            break;
    }

    if ( size > 0 && i == size )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateString(const char *objValue) {
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateIpAddress(const char *objValue) {
    int i = 0, num = 0;
    char *pToken = NULL, *pLast = NULL;
    char *buf;
    int len;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    len = strlen(objValue)+1;
    if (len > 20)
      len = 20;
    buf =  new char[len];
    memset(buf,0,len);

    /* need to copy since strtok_r updates string */
    strncpy(buf, objValue,len-1);

    /* IP address has the following format
       xxx.xxx.xxx.xxx where x is decimal number */
    for ( i = 0, pToken = strtok_r(buf, ".", &pLast);
          i < 4;
          i++, pToken = strtok_r(NULL, ".", &pLast) ) {
        if ( BcmDb_validateNumber(pToken) == DB_OBJ_VALID_ERR_INVALID_VALUE )
            break;
        num = atoi(pToken);
        if ( num < 0 || num > 255 )
            break;
    }

    if ( i == 4 )
        status = DB_OBJ_VALID_OK;

    delete[] buf;

    return status;
}

/* j00100803 Add Begin 2008-03-09 */
#ifdef SUPPORT_POLICY
DB_OBJ_VALID_STATUS BcmDb_validateIpAddressAndSubnetMask(const char *objValue, int iSubnetMask)
{
	
    return DB_OBJ_VALID_OK;
}
#endif
/* j00100803 Add Begin 2008-03-09 */

int getLeftMostZeroBitPos(int num) {
    int i = 0, numArrSize = 8;
    int numArr[] = {128, 64, 32, 16, 8, 4, 2, 1};

    for ( i = 0; i < numArrSize; i++ )
        if ( (num & numArr[i]) == 0 )
            return i;

    return numArrSize;
}

int getRightMostOneBitPos(int num) {
    int i = 0, numArrSize = 8;
    int numArr[] = {1, 2, 4, 8, 16, 32, 64, 128};

    for ( i = 0; i < numArrSize; i++ )
        if ( ((num & numArr[i]) >> i) == 1 )
            return (numArrSize - i - 1);

    return -1;
}

DB_OBJ_VALID_STATUS BcmDb_validateSubnetMask(const char *objValue) {
    int i = 0, num = 0;
    int zeroBitPos = 0, oneBitPos = 0;
    int zeroBitExisted = FALSE;
    char *pToken = NULL, *pLast = NULL;
    char *buf;
    int len;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if ( strcmp(objValue, "0.0.0.0") == 0 )
        return status;

    len = strlen(objValue) + 1;
    if (len > 20)
      len = 20;
    buf = new char[len];
    memset(buf,0,len);

    /* need to copy since strtok_r updates string */
    strncpy(buf, objValue, len-1);

    /* subnet mask has the following format
       xxx.xxx.xxx.xxx where x is decimal number */
    for ( i = 0, pToken = strtok_r(buf, ".", &pLast);
          i < 4;
          i++, pToken = strtok_r(NULL, ".", &pLast) ) {
        if ( BcmDb_validateNumber(pToken) == DB_OBJ_VALID_ERR_INVALID_VALUE )
            break;
        num = atoi(pToken);
        if ( num < 0 || num > 255 )
            break;
        if ( zeroBitExisted == TRUE && num != 0 )
            break;
        zeroBitPos = getLeftMostZeroBitPos(num);
        oneBitPos = getRightMostOneBitPos(num);
        if ( zeroBitPos < oneBitPos )
            break;
        if ( zeroBitPos < 8 )
            zeroBitExisted = TRUE;
    }

    if ( i == 4 )
        status = DB_OBJ_VALID_OK;

    delete[] buf;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateMacAddress(const char *objValue) {
    int i = 0;
    char *pToken = NULL, *pLast = NULL;
    char *buf;
    int len;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    len = strlen(objValue) + 1;
    if (len > 20)
       len = 20;
    buf = new char[len];
    memset(buf,0,len);

    // need to copy since strtok_r updates string
    //    strcpy(buf, objValue);
    strncpy(buf, objValue,len-1);

    // Mac address has the following format
    // xx:xx:xx:xx:xx:xx where x is hex number
    for ( i = 0, pToken = strtok_r(buf, ":", &pLast);
          i < 6;
          i++, pToken = strtok_r(NULL, ":", &pLast) ) {
        if ( BcmDb_validateHexNumber(pToken) == DB_OBJ_VALID_ERR_INVALID_VALUE ||
             strlen(pToken) > 2 )
            break;
    }

    if ( i == 6 )
        status = DB_OBJ_VALID_OK;

    delete[] buf;

    return status;
}

/* BEGIN: Added by y67514, 2008/9/11   PN:GLB:Ip Extension*/
/*****************************************************************************
 函 数 名  : BcmDb_validateIpExtMac
 功能描述  : 校验mac是否和法，只能为空，或"XX:XX:XX:XX:XX:XX"
 输入参数  : const char *objValue  
 输出参数  : 无
 返 回 值  : DB_OBJ_VALID_STATUS
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月11日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
DB_OBJ_VALID_STATUS BcmDb_validateIpExtMac(const char *objValue) 
{
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if ( NULL == objValue )
    {
        return status;
    }

    if ( 0 == strlen(objValue) )
    {
        return DB_OBJ_VALID_OK;
    }

    if ( DB_OBJ_VALID_OK == BcmDb_validateMacAddress(objValue))
    {
        status = DB_OBJ_VALID_OK;
    }

    return status;
}

/* END:   Added by y67514, 2008/9/11 */

DB_OBJ_VALID_STATUS BcmDb_validateAtmVpi(const char *objValue) {
    int VPI_MIN = 0, VPI_MAX = 255;

    return BcmDb_validateRange(objValue, VPI_MIN, VPI_MAX);
}

DB_OBJ_VALID_STATUS BcmDb_validateAtmVci(const char *objValue) {
    int VCI_MIN = 32, VCI_MAX = 65535;

    return BcmDb_validateRange(objValue, VCI_MIN, VCI_MAX);
}

DB_OBJ_VALID_STATUS BcmDb_validateAtmCategory(const char *objValue) {
    int i = 0;
    const char* categories[] = { "UBR", "UBRwPCR", "CBR",
                                 "NRT_VBR", "RT_VBR", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; categories[i] != NULL; i++ ) {
        if ( strcasecmp(categories[i], objValue) == 0 )
            break;
    }

    if ( categories[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateAtmAalType(const char *objValue) {
    int i = 0;
    const char* types[] = { "aal5", "aal2", "aal0pkt",
                            "aal0cell", "aaltransparent", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; types[i] != NULL; i++ ) {
        if ( strcasecmp(types[i], objValue) == 0 )
            break;
    }

    if ( types[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateAtmAdminStatus(const char *objValue) {
    int i = 0;
    const char* admins[] = { "up", "down", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; admins[i] != NULL; i++ ) {
        if ( strcasecmp(admins[i], objValue) == 0 )
            break;
    }

    if ( admins[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateAtmEncapsulation(const char *objValue) {
    int i = 0;
    const char* encaps[] = { "vcMuxRouted", "vcMuxBr8023", "vcMuxBr8025",
                             "vcMuxBr8026", "vcMuxLanEmu8023",
                             "vcMuxLanEmu8025", "llc", "frameRelaySscs",
                             "other", "unknown",  NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; encaps[i] != NULL; i++ ) {
        if ( strcasecmp(encaps[i], objValue) == 0 )
            break;
    }

    if ( encaps[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateFtpPath(const char *objValue) {

    return BcmDb_validateLength(objValue, IFC_SMALL_LEN);
}
DB_OBJ_VALID_STATUS BcmDb_validateUserName(const char *objValue) {

    return BcmDb_validateLength(objValue, IFC_TINY_LEN);
}

DB_OBJ_VALID_STATUS BcmDb_validatePassword(const char *objValue) {

    return BcmDb_validateLength(objValue, IFC_PASSWORD_LEN);
}

DB_OBJ_VALID_STATUS BcmDb_validatePppUserName(const char *objValue) {

    return BcmDb_validateLength(objValue, IFC_LARGE_LEN);
}

DB_OBJ_VALID_STATUS BcmDb_validatePppPassword(const char *objValue) {

    /*start of 修改问题单AU8D00817  ppp 密码过长问题by s53329 at   20080715
    return BcmDb_validateLength(objValue, IFC_MEDIUM_LEN);
    */
    return BcmDb_validateLength(objValue, IFC_PWD_LEN);
    /*end of 修改问题单AU8D00817  ppp 密码过长问题by s53329 at   20080715 */
}

DB_OBJ_VALID_STATUS BcmDb_validatePppTimeout(const char *objValue) {
    int PPP_TIMEOUT_MIN = 1, PPP_TIMEOUT_MAX = 4320;

    return BcmDb_validateRange(objValue, PPP_TIMEOUT_MIN, PPP_TIMEOUT_MAX);
}

DB_OBJ_VALID_STATUS BcmDb_validatePppAuthMethod(const char *objValue) {
    int i = 0;
    const char* methods[] = { "AUTO", "PAP", "CHAP", "MSCHAP", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; methods[i] != NULL; i++ ) {
        if ( strcasecmp(methods[i], objValue) == 0 )
            break;
    }

    if ( methods[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

DB_OBJ_VALID_STATUS BcmDb_validateProtocol(const char *objValue) {
    int i = 0;
#if SUPPORT_ETHWAN
    const char* protocols[] = { "PPPOE", "PPPOA", "MER",
                                "BRIDGE", "IPOA", "IPOW", NULL };
#else
#ifdef VDF_PPPOU
    const char* protocols[] = { "PPPOE", "PPPOA", "MER",
                                "BRIDGE", "IPOA",
                                "PPPOU", //add by sxg, pppousb
                                NULL };
#else
    const char* protocols[] = { "PPPOE", "PPPOA", "MER",
                                "BRIDGE", "IPOA", NULL };
#endif
#endif
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; protocols[i] != NULL; i++ ) {
        if ( strcasecmp(protocols[i], objValue) == 0 )
            break;
    }

    if ( protocols[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}



/*w00135358 start-add for Global QoS 20080908*/
DB_OBJ_VALID_STATUS BcmDb_validateEtheProtocol(const char *objValue) {
    int i = 0;

    const char* protocols[] = { "IP", "802.1Q", "PPPoE", NULL};

    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; protocols[i] != NULL; i++ ) {
        if ( strcasecmp(protocols[i], objValue) == 0 )
            break;
    }

    if ( protocols[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
/*w00135358 end-add for Global QoS 20080908*/

DB_OBJ_VALID_STATUS BcmDb_validateIpProtocol(const char *objValue) {
    int i = 0;
    /*start modifying for vdf qos by p44980 2008.01.11*/
    #ifdef SUPPORT_VDF_QOS
    const char* protocols[] = { "TCP/UDP", "TCP", "UDP",  "ICMP", "IGMP" ,NULL};
    #else
    const char* protocols[] = { "TCP/UDP", "TCP", "UDP",  "ICMP", NULL };
    #endif
    /*end modifying for vdf qos by p44980 2008.01.11*/
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; protocols[i] != NULL; i++ ) {
        if ( strcasecmp(protocols[i], objValue) == 0 )
            break;
    }

    if ( protocols[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

/*j00100803 Add Begin 2008-02-27*/
#ifdef SUPPORT_POLICY
DB_OBJ_VALID_STATUS BcmDb_validateIpProtocolForRouting(const char *objValue) {
    int i = 0;
    const char* protocols[] = { "TCP", "UDP", "ICMP", "IGMP", "HTTP", "SIP", NULL};
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; protocols[i] != NULL; i++ ) {
        if ( strcasecmp(protocols[i], objValue) == 0 )
            break;
    }

    if ( protocols[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
#endif
/*j00100803 Add End 2008-02-27*/
DB_OBJ_VALID_STATUS BcmDb_validateMacProtocol(const char *objValue) {
    int i = 0;
    const char* protocols[] = { "none", "PPPoE", "IPv4", "IPv6", "AppleTalk", "IPX", "NetBEUI", "IGMP", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; protocols[i] != NULL; i++ ) {
        if ( strcasecmp(protocols[i], objValue) == 0 )
            break;
    }

    if ( protocols[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateInterfaceName(const char *objValue) {

    return BcmDb_validateLength(objValue, IFC_TINY_LEN);
}

DB_OBJ_VALID_STATUS BcmDb_validateServiceName(const char *objValue) {

    /*start of 修改问题单AU8D00817 by  s53329  at   20080715
    return BcmDb_validateLength(objValue, IFC_MEDIUM_LEN);
    */
    return BcmDb_validateLength(objValue, IFC_PWD_LEN);
    /*end  of 修改问题单AU8D00817 by  s53329  at   20080715  */

}

DB_OBJ_VALID_STATUS BcmDb_validateState(const char *objValue) {
    int i = 0;
    const char* states[] = { "enable", "disable", "enabled", "disabled", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; states[i] != NULL; i++ ) {
        if ( strcasecmp(states[i], objValue) == 0 )
            break;
    }

    if ( states[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateDhcpState(const char *objValue) {
    int i = 0;
    const char* states[] = { "enable", "disable", "relay", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; states[i] != NULL; i++ ) {
        if ( strcasecmp(states[i], objValue) == 0 )
            break;
    }

    if ( states[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateEncapsulation(const char *objValue) {
    int i = 0;
    const char* encaps[] = { "LLC", "VCMUX", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; encaps[i] != NULL; i++ ) {
        if ( strcasecmp(encaps[i], objValue) == 0 )
            break;
    }

    if ( encaps[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateAccess(const char *objValue) {
    int i = 0;
    const char* where[] = { "disable", "enable", "lan", "wan", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; where[i] != NULL; i++ ) {
        if ( strcasecmp(where[i], objValue) == 0 )
            break;
    }

    if ( where[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateSysLogLevel(const char *objValue) {
    int i = 0;
    const char* levels[] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING",
                             "NOTICE", "INFO", "DEBUG",  NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; levels[i] != NULL; i++ ) {
        if ( strcasecmp(levels[i], objValue) == 0 )
            break;
    }

    if ( levels[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateSysLogOption(const char *objValue) {
    int i = 0;
    const char* options[] = { "local", "remote", "both", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; options[i] != NULL; i++ ) {
        if ( strcasecmp(options[i], objValue) == 0 )
            break;
    }

    if ( options[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateRipOperation(const char *objValue) {
    int i = 0;
    const char* opers[] = { "active", "passive", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; opers[i] != NULL; i++ ) {
        if ( strcasecmp(opers[i], objValue) == 0 )
            break;
    }

    if ( opers[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateMacFilterDirection(const char *objValue) {
    int i = 0;
    const char* dirs[] = {"LanToWan", "WanToLan", "both", NULL};
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; dirs[i] != NULL; i++ ) {
        if ( strcasecmp(dirs[i], objValue) == 0 )
            break;
    }

    if ( dirs[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateMacFilterPolicy(const char *objValue) {
    int i = 0;
    const char* policies[] = {"forward", "block", NULL};
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; policies[i] != NULL; i++ ) {
        if ( strcasecmp(policies[i], objValue) == 0 )
            break;
    }

    if ( policies[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateQosPriority(const char *objValue) {
    int i = 0;
#ifdef FOUR_LEVEL_QOS
    const char* priority[] = {"", "low", "medium", "high", "highest",  NULL};
#else
    /* start of protocol QoS for KPN by zhangchen z45221 2006年7月17日
    const char* priority[] = {"", "low", "medium", "high",  NULL};
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; priority[i] != NULL; i++ ) {
        if ( strcasecmp(priority[i], objValue) == 0 )
            break;
    }

    if ( priority[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
    */
    #ifdef SUPPORT_VDSL
        #ifdef SUPPORT_KPN_QOS
        int trafficmode = 0;
        int ret = 0;
    
        ret = BcmDb_getTrafficMode(&trafficmode); //mode of flash data
        if (TM_ATM_BR == trafficmode || TM_PTM_BR == trafficmode)
        {
            const char* priority[] = {"", "low and low weight", "low and high weight", "medium", "high",  NULL};
            DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

            for ( i = 0; priority[i] != NULL; i++ ) {
                if ( strcasecmp(priority[i], objValue) == 0 )
                    break;
            }

            if ( priority[i] != NULL )
                status = DB_OBJ_VALID_OK;

            return status;
        }
        else
        {
            const char* priority[] = {"", "low", "medium", "high",  NULL};
            DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

            for ( i = 0; priority[i] != NULL; i++ ) {
                if ( strcasecmp(priority[i], objValue) == 0 )
                    break;
            }

            if ( priority[i] != NULL )
                status = DB_OBJ_VALID_OK;

            return status;
        }
        #else
        const char* priority[] = {"", "low", "medium", "high",  NULL};
        DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

        for ( i = 0; priority[i] != NULL; i++ ) {
            if ( strcasecmp(priority[i], objValue) == 0 )
                break;
        }

        if ( priority[i] != NULL )
            status = DB_OBJ_VALID_OK;

        return status;
        #endif
    #else
        const char* priority[] = {"", "low", "medium", "high",  NULL};
        DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

        for ( i = 0; priority[i] != NULL; i++ ) {
            if ( strcasecmp(priority[i], objValue) == 0 )
                break;
        }

        if ( priority[i] != NULL )
            status = DB_OBJ_VALID_OK;

        return status;

    #endif
    /* end of protocol QoS for KPN by zhangchen z45221 2006年7月17日 */
#endif
}


/* BEGIN: Modified by y67514, 2008/10/24   PN:lanDomain*/
/*****************************************************************************
 函 数 名  : BcmDb_validateDomain
 功能描述  : 检测是否为合法域名
 输入参数  : const char *objValue  
 输出参数  : 无
 返 回 值  : DB_OBJ_VALID_STATUS
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年10月24日
    作    者   : y67514
    修改内容   : 新生成函数

*****************************************************************************/
DB_OBJ_VALID_STATUS BcmDb_validateDomain(const char *objValue) 
{
    int i = 0;
    int dotFlag = 0;
    int len = 0;

    if ( NULL == objValue )
    {
        return DB_OBJ_VALID_ERR_INVALID_VALUE;
    }

    len = strlen(objValue);

    if ( (len < 1) || (len > 32) )
    {
        return DB_OBJ_VALID_ERR_INVALID_LENGTH;
    }

    if ( '.' == objValue[0] )
    {
        /*第一个字符不能是点*/
        return DB_OBJ_VALID_ERR_INVALID_VALUE;
    }
    
    for ( i = 0 ; i < len ; i++ )
    {
        if ( '.' == objValue[i] )
        {
            if ( (i + 1) < len)
            {
                if (  '.' == objValue[i + 1] )
                {
                 /*不能出现连续点*/
                    return DB_OBJ_VALID_ERR_INVALID_VALUE;
                }
            }
            dotFlag = 1;
        }
        else if ( FALSE== isalnum(objValue[i]) )
            {
           return DB_OBJ_VALID_ERR_INVALID_VALUE;
            }
    }

   if (1 == dotFlag )
   {     
       return DB_OBJ_VALID_OK;
   }
   else
   {
       return DB_OBJ_VALID_ERR_INVALID_VALUE;
   }
}
/* END:   Modified by y67514, 2008/10/24 */

#ifdef WIRELESS
/* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
DB_OBJ_VALID_STATUS BcmDb_validateWirelessCountryEx(const char *objValue, int index_adaptor) {
    #define MAX_COUNTRIES 200
    char cmd[32], abbrv[3];
    int i = 0, l, found = 0;
    char *list, *ptr;
    char type[2];
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if (strlen(objValue) > 3)
      return status;

    list = (char*)malloc(4*MAX_COUNTRIES);
printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
    for (l = 0; l < 2; l++) {
      ptr = list;
      if (l == 0)
        strcpy(type,"a\0");
      else
        strcpy(type,"b\0");
      sprintf(cmd, "wlctl -i wl%d country list %s > /var/wl%dclist", index_adaptor, type, index_adaptor);
      BCMWL_WLCTL_CMD(cmd);
	  printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
      sprintf(cmd, "/var/wl%dclist", index_adaptor);

      FILE *fp = fopen(cmd, "r");
	  printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
      if ( fp != NULL ) {
	  	printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
        fscanf(fp, "%*[^\n]");   // skip first line
        fscanf(fp, "%*1[\n]");

        for (i=0;i<MAX_COUNTRIES;i++) {
          if (fscanf(fp, "%s %*1[\n]", abbrv) != 2) {
            break;
          }
		  printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
          sprintf(ptr, "%s ",abbrv);
          ptr += 4; /* 2-3 characters abbreviation + 1 space */
        }
		printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
        *ptr = 0;
        fclose(fp);
printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
        if (strstr(list,objValue) != 0) {
          found = 1;
          break;
        }
        else
			printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
          memset(list,0,sizeof(list));
		  printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
      } /* fp */
    } /* two lists loop */
    if (!found)
      status = DB_OBJ_VALID_OK;
printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
    free(list);
printf("\r\nwlandebug:file[%s] line[%d] ", __FILE__, __LINE__);
    return status;
}


DB_OBJ_VALID_STATUS BcmDb_validateWirelessCountry(const char *objValue) {

    return BcmDb_validateWirelessCountryEx(objValue, 0);    

}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessAuthMode(const char *objValue) {
    int i = 0;
    const char* mode[] = { "open", "shared", "radius", "wpa", "psk","wpa2","psk2","wpa2mix","psk2mix", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessEncryption(const char *objValue) {
    int i = 0;
    const char* mode[] = { "tkip", "aes", "tkip+aes", "disabled", "enabled", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessGenOnOff(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "on", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessAP(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "on", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessXpress(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "on", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessAfterBurner(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "auto", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessBand(const char *objValue) {
    int i = 0;
    const char* mode[] = { "auto", "a", "b", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessRate(const char *objValue) {
    double value = 0;
    int i;
    double rates[13] = { 0, 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54 };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    value = atof(objValue);
    for ( i = 0; i < 13; i++ ) {
       if (value == rates[i]) {
          status = DB_OBJ_VALID_OK;
          break;
       }
    }
    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessBasicRate(const char *objValue) {
    int i = 0;
    const char* mode[] = { "default", "all", "12", "wifi2", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessGmode(const char *objValue) {
    int i = 0;
   /* BEGIN: Modified by c106292, 2008/9/28   PN:Global 增加了"802.11ng","802.11na"*/
    const char* mode[] = { "auto", "performance", "lrs", "802.11b", "802.11ng","802.11na", NULL};
   /* END:   Modified by c106292, 2008/9/28 */
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessOnOffAuto(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "on", "auto", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
/*del by y68191
DB_OBJ_VALID_STATUS BcmDb_validateWirelessGprotection(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "auto", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
*/
DB_OBJ_VALID_STATUS BcmDb_validateWirelessNmode(const char *objValue) {
    int i = 0;
    const char* mode[] = { "off", "auto", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessKeybit(const char *objValue) {
    int i = 0;
    const char* mode[] = { "64-bit", "128-bit", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessFltMacMode(const char *objValue) {
    int i = 0;
    const char* mode[] = { "disabled", "allow", "deny", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessBridgeRestrictMode(const char *objValue) {
    int i = 0;
    const char* mode[] = { "enabled", "disabled", "enabled_scan", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
DB_OBJ_VALID_STATUS BcmDb_validateWirelessApMode(const char *objValue) {
    int i = 0;
    const char* mode[] = { "ap", "wds", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessLength(const char *objValue, unsigned int length,
                                                 unsigned int lenHex) {
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_LENGTH;
    unsigned int len = strlen(objValue);

    if ((len == length) || (len == lenHex) )
        status = DB_OBJ_VALID_OK;

    return status;
}

#ifdef SUPPORT_MIMO
DB_OBJ_VALID_STATUS BcmDb_validateWirelessBandwidth(const char *objValue)
{
    int i = 0;
    const char* nbw[] = { "0", "1", "2", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; nbw[i] != NULL; i++ ) {
        if ( strcasecmp(nbw[i], objValue) == 0 )
            break;
    }
    
    if ( nbw[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;	
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessCtrlSideband(const char *objValue)
{
    int i = 0;
    const char* nctrlsb[] = { "none", "lower", "upper", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; nctrlsb[i] != NULL; i++ ) {
        if ( strcasecmp(nctrlsb[i], objValue) == 0 )
            break;
    }
    
    if ( nctrlsb[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;	
	
}
#endif
        
DB_OBJ_VALID_STATUS BcmDb_validateXmlAndGetWirelessTR69cObjs(const char *objValue, const char *objName, 
                                                  int *outVal, char* outName) {

    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;
    int i = 0;
    int retval = -1;
    char *retptr = NULL;
  
    char* tr69cBeaconType[] ={"None","Basic","WPA","11i","BasicandWPA","Basicand11i","WPAand11i","BasicandWPAand11i",NULL};
    char* tr69cBasicEncryptionModes[]={"None","WEPEncryption",NULL};
	/*start of VDF  2008.03.07 HG553V100R001  w45260:AU8D00478 增加对share认证方式的支持 */	
    char* tr69cBasicAuthenticationMode[]={"None","EAPAuthentication","SharedAuthentication",NULL};
    /*end of VDF  2008.04.17 HG553V100R001  w45260 */
    char* tr69cWPAEncryptionModes[]={"WEPEncryption","TKIPEncryption","WEPandTKIPEncryption","AESEncryption",\
                                     "WEPandAESEncryption","TKIPandAESEncryption","WEPandTKIPandAESEncryption",NULL};
    char* tr69cWPAAuthenticationMode[]={"PSKAuthentication","EAPAuthentication",NULL};
    char* tr69cIEEE11iEncryptionModes[]={"WEPEncryption","TKIPEncryption","WEPandTKIPEncryption","AESEncryption",\
                                     "WEPandAESEncryption","TKIPandAESEncryption","WEPandTKIPandAESEncryption",NULL};
    char* tr69cIEEE11iAuthenticationMode[]={"PSKAuthentication","EAPAuthentication","EAPandPSKAuthentication",NULL};
    
    if(!strcasecmp(objName, "tr69cBeaconType")){
    	for (i = 0; tr69cBeaconType[i] != NULL ; i++) {
            if (!strcasecmp(tr69cBeaconType[i], objValue)){
            	retval = i;
            	retptr = tr69cBeaconType[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cBasicEncryptionModes")){
    	for (i = 0; tr69cBasicEncryptionModes[i] != NULL; i++) {
            if (!strcasecmp(tr69cBasicEncryptionModes[i], objValue)){
            	retval = i;
            	retptr = tr69cBasicEncryptionModes[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cBasicAuthenticationMode")){
    	for (i = 0; tr69cBasicAuthenticationMode[i] != NULL; i++) {
            if (!strcasecmp(tr69cBasicAuthenticationMode[i], objValue)){
            	retval = i;
            	retptr = tr69cBasicAuthenticationMode[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cWPAEncryptionModes")){
    	for (i = 0; tr69cWPAEncryptionModes[i] != NULL; i++) {
            if (!strcasecmp(tr69cWPAEncryptionModes[i], objValue)){
            	retval = i;
            	retptr = tr69cWPAEncryptionModes[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cWPAAuthenticationMode")){
    	for (i = 0; tr69cWPAAuthenticationMode[i] != NULL; i++) {
            if (!strcasecmp(tr69cWPAAuthenticationMode[i], objValue)){
            	retval = i;
            	retptr = tr69cWPAAuthenticationMode[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cIEEE11iEncryptionModes")){
    	for (i = 0; tr69cIEEE11iEncryptionModes[i] != NULL; i++) {
            if (!strcasecmp(tr69cIEEE11iEncryptionModes[i], objValue)){
            	retval = i;
            	retptr = tr69cIEEE11iEncryptionModes[i];
                break;    	    	
            }        
    	}
    } 

    else if(!strcasecmp(objName, "tr69cIEEE11iAuthenticationMode")){
    	for (i = 0; tr69cIEEE11iAuthenticationMode[i] != NULL; i++) {
            if (!strcasecmp(tr69cIEEE11iAuthenticationMode[i], objValue)){
            	retval = i;
            	retptr = tr69cIEEE11iAuthenticationMode[i];
                break;    	    	
            }        
    	}
    } 
    
    if(retval >= 0) {    	
        status = DB_OBJ_VALID_OK;
        if(outVal)
            *outVal = retval;
        if(outName)
            strcpy(outName, retptr);            
    }
    
  return status;	
	
}
   
DB_OBJ_VALID_STATUS BcmDb_validateWirelessPreamble(const char *objValue) {
    int i = 0;
    const char* mode[] = { "long", "short", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateWirelessGenStr(const char *objValue, const char *validStr[], int *index) {
    int i = 0;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    *index = 0;    
    for ( i = 0; validStr[i] != NULL; i++ ) {
        if ( strcasecmp(validStr[i], objValue) == 0 ){
            *index = i;
            break;
        }        
    }

    if ( validStr[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
#endif /* #ifdef WIRELESS */

DB_OBJ_VALID_STATUS BcmDb_validateAdslPair(const char *objValue)
{
    int i = 0;
    const char* mode[] = { "inner", "outer", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strcasecmp(mode[i], objValue) == 0 )
            break;
    }

    if ( mode[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

#ifdef SUPPORT_TOD
/* time is in the 24 hour format hh:mm */
DB_OBJ_VALID_STATUS BcmDb_validateTime(const char *objValue)
{
    int i = 0;
    char *pToken = NULL, *pLast = NULL;
    char *buf;
    int len;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;
    int hour = 0, min = 0;

    len = strlen(objValue) + 1;
    if (len > 6)
      len = 6;
    buf = new char[len];
    memset(buf,0,len);
    
    // need to copy since strtok_r updates string
    strncpy(buf, objValue,len-1);

    for ( i = 0, pToken = strtok_r(buf, ":", &pLast);
          i < 2;
          i++, pToken = strtok_r(NULL, ":", &pLast) ) {
        if ( BcmDb_validateNumber(pToken) == DB_OBJ_VALID_ERR_INVALID_VALUE ||
             strlen(pToken) > 2 )
            break;
        else {
          if (i == 0) {
             hour = atoi(pToken);
             if ((hour < 0) || (hour > 23))
                break;
          } /* hour */
          if (i == 1) {
             min = atoi(pToken);
             if ((min < 0) || (min > 59))
               break;
          } /* hour */
        } /* valid number */
    }

    if ( i == 2 )
        status = DB_OBJ_VALID_OK;

    delete[] buf;

    return status;
}

/* days string is passed in with 3 character of abbreviation for each day concatenated together.
   For example, MonTue represents Monday and Tuesday */
DB_OBJ_VALID_STATUS BcmDb_validateDay(const char *objValue)
{
    int i = 0, len = strlen(objValue);
    char *ptr = (char*)objValue, d[3];
    const char* mode[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    if ((len == 0) || ((len % 3) != 0))
      return status;

    while (len >= 3) {
      strncpy(d,ptr,3);
      for ( i = 0; mode[i] != NULL; i++ ) {
        if ( strncasecmp(mode[i], d, 3) == 0 )
          break;
      }
      if ( mode[i] == NULL )
        break;
      ptr += 3;
      len -= 3;
    }
    if (len == 0)
      status = DB_OBJ_VALID_OK;

    return status;
}
#endif /* #ifdef SUPPORT_TOD */

/* port could come as just an integer, or in a range format such as portBegin:portEnd */
DB_OBJ_VALID_STATUS BcmDb_validatePort(const char *objValue, int min, int max) {
    int i = 0;
    char *pToken = NULL, *pLast = NULL;
    char *buf;
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;
    int error = 0, len;
    
    len = strlen(objValue) + 1;
    if (len > 40)
      len = 40;
    buf = new char[len];
    memset(buf,0,len);
    /* need to copy since strtok_r updates string */
    strncpy(buf, objValue,len-1);

    for ( i = 0, pToken = strtok_r(buf, ":", &pLast); i < 2;
          i++, pToken = strtok_r(NULL, ":", &pLast) ) {
      if ((pToken == NULL) && (i != 0))
        break;

      if ( BcmDb_validateNumber(pToken) != DB_OBJ_VALID_ERR_INVALID_VALUE ) {
        if (BcmDb_validateRange(pToken,min,max) == DB_OBJ_VALID_ERR_INVALID_VALUE) {
          error = 1;
          break;
        }
      } /* number */
      else {
        error = 1;
        break;
      }
    } /* loop */

    delete[] buf;

    if (!error)
       status = DB_OBJ_VALID_OK;
    return status;
}

#ifdef ETH_CFG
DB_OBJ_VALID_STATUS BcmDb_validateEthSpeed(const char *objValue)
{
    int i = 0;
    const char* speed[] = { "auto", "100Mbps", "10Mbps", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; speed[i] != NULL; i++ ) {
        if ( strcasecmp(speed[i], objValue) == 0 )
            break;
    }

    if ( speed[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateEthType(const char *objValue)
{
    int i = 0;
    const char* type[] = { "auto", "fullDuplex", "halfDuplex", NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; type[i] != NULL; i++ ) {
        if ( strcasecmp(type[i], objValue) == 0 )
            break;
    }

    if ( type[i] != NULL )
        status = DB_OBJ_VALID_OK;

    return status;
}
#endif /* ETH_CFG */

#ifdef VOXXXLOAD
DB_OBJ_VALID_STATUS BcmDb_validateCodec(const char *objValue)
{
	    int i = 0;
	        const char* mode[] = { "G711U", "G711A", "G723", "G726", "G729", "G728", "BV16", "iLBC", "auto", NULL };
		    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

		        for ( i = 0; mode[i] != NULL; i++ ) {
			        if ( strcasecmp(mode[i], objValue) == 0 )
				   break;
			 }

			    if ( mode[i] != NULL )
				            status = DB_OBJ_VALID_OK;

			        return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateVoiceCodec(const char *objValue)
{
    int i = 0;
    const char* mode[] = { "none",
#if XCFG_G711_SUPPORT
                           "G711U",
                           "G711A",
#endif
#if XCFG_G7231_SUPPORT
                           "G723",
#endif
#if XCFG_G726_SUPPORT
                           "G726",
#endif
#if XCFG_G729_SUPPORT
                           "G729",
#endif
#if XCFG_G728_SUPPORT
                           "G728",
#endif
#if XCFG_BV16_SUPPORT
                           "BV16",
#endif
#if XCFG_ILBC_SUPPORT
                           "iLBC",
#endif
                           NULL };
    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; mode[i] != NULL; i++ )
    {
       if ( strcasecmp(mode[i], objValue) == 0 )
          break;
    }

    if ( mode[i] != NULL )
       status = DB_OBJ_VALID_OK;

    return status;
}

DB_OBJ_VALID_STATUS BcmDb_validateLocale(const char *objValue)
{
    int i = 0;
    const char* locale[] = {
#if VRG_COUNTRY_CFG_BELGIUM
                             CC_COUNTRY_STR_BELGIUM,
#endif
#if VRG_COUNTRY_CFG_BRAZIL
                             CC_COUNTRY_STR_BRAZIL,
#endif
#if VRG_COUNTRY_CFG_CHILE
                             CC_COUNTRY_STR_CHILE,
#endif
#if VRG_COUNTRY_CFG_CHINA
                             CC_COUNTRY_STR_CHINA,
#endif
#if VRG_COUNTRY_CFG_DENMARK
                             CC_COUNTRY_STR_DENMARK,
#endif
#if VRG_COUNTRY_CFG_ETSI
                             CC_COUNTRY_STR_ETSI,
#endif
#if VRG_COUNTRY_CFG_FINLAND
                             CC_COUNTRY_STR_FINLAND,
#endif
#if VRG_COUNTRY_CFG_FRANCE
                             CC_COUNTRY_STR_FRANCE,
#endif
#if VRG_COUNTRY_CFG_GERMANY
                             CC_COUNTRY_STR_GERMANY,
#endif
#if VRG_COUNTRY_CFG_HUNGARY
                             CC_COUNTRY_STR_HUNGARY,
#endif
#if VRG_COUNTRY_CFG_ITALY
                             CC_COUNTRY_STR_ITALY,
#endif
#if VRG_COUNTRY_CFG_JAPAN
                             CC_COUNTRY_STR_JAPAN,
#endif
#if VRG_COUNTRY_CFG_NETHERLANDS
                             CC_COUNTRY_STR_NETHERLANDS,
#endif
#if VRG_COUNTRY_CFG_NORTH_AMERICA
                             CC_COUNTRY_STR_NORTH_AMERICA,
#endif
#if VRG_COUNTRY_CFG_SPAIN
                             CC_COUNTRY_STR_SPAIN,
#endif
#if VRG_COUNTRY_CFG_SWEDEN
                             CC_COUNTRY_STR_SWEDEN,
#endif
#if VRG_COUNTRY_CFG_SWITZERLAND
                             CC_COUNTRY_STR_SWITZERLAND,
#endif
#if VRG_COUNTRY_CFG_TR57
                             CC_COUNTRY_STR_TR57,
#endif
#if VRG_COUNTRY_CFG_UK
                             CC_COUNTRY_STR_UK,
#endif
#if VRG_COUNTRY_CFG_SINGAPORE
                             CC_COUNTRY_STR_SINGAPORE,
#endif
                            NULL };

    DB_OBJ_VALID_STATUS status = DB_OBJ_VALID_ERR_INVALID_VALUE;

    for ( i = 0; locale[i] != NULL; i++ )
    {
       if ( strcasecmp(locale[i], objValue) == 0 )
          break;
    }

    if ( locale[i] != NULL )
       status = DB_OBJ_VALID_OK;

    return status;
}
#endif /* VOXXXLOAD */

#if defined(__cplusplus)
}
#endif


