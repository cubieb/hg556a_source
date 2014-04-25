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
//  Filename:       dbvalid.h
//  Author:         Peter T. Tran
//  Creation Date:  07/06/04
//
******************************************************************************
//  Description:
//      Define the BcmValid_* functions.
//
*****************************************************************************/

#ifndef __DB_VALID_H__
#define __DB_VALID_H__

/********************** Include Files ***************************************/

typedef enum {
    DB_OBJ_VALID_OK = 0,
    DB_OBJ_VALID_ERR_INVALID_TYPE,
    DB_OBJ_VALID_ERR_INVALID_VALUE,
    DB_OBJ_VALID_ERR_INVALID_LENGTH,
    DB_OBJ_VALID_ERR_OUT_OF_RANGE,
    DB_OBJ_VALID_ERR_NOT_EXIST
} DB_OBJ_VALID_STATUS;

typedef enum {
    DB_TYPE_BOOLEAN = 0,
    DB_TYPE_NUMBER,
    DB_TYPE_STRING,
    DB_TYPE_IP_ADDRESS,
    DB_TYPE_SUBNET_MASK,
    DB_TYPE_MAC_ADDRES,
    DB_TYPE_ATM_VPI,
    DB_TYPE_ATM_VCI,
    DB_TYPE_ATM_CATEGORY,
    DB_TYPE_ATM_AAL_TYPE,
    DB_TYPE_ATM_ADMIN_STATUS,
    DB_TYPE_ATM_ENCAPSULATION,
    DB_TYPE_USERNAME,
    DB_TYPE_PASSWORD,
    DB_TYPE_PPP_USERNAME,
    DB_TYPE_PPP_PASSWORD,
    DB_TYPE_PPP_TIMEOUT,
    DB_TYPE_PPP_AUTH_METHOD,
    DB_TYPE_PROTOCOL,
    DB_TYPE_IP_PROTOCOL,
    DB_TYPE_MAC_PROTOCOL,
    DB_TYPE_INTERFACE_NAME,
    DB_TYPE_SERVICE_NAME,
    DB_TYPE_STATE,
    DB_TYPE_DHCP_STATE,
    DB_TYPE_ENCAPSULATION,
    DB_TYPE_ACCESS,
    DB_TYPE_SYSLOG_LEVEL,
    DB_TYPE_SYSLOG_OPTION,
    DB_TYPE_RIP_OPERATION,
    DB_TYPE_MAC_FILTER_DIRECTION,
    DB_TYPE_MAC_FILTER_POLICY,
    DB_TYPE_QOS_PRIORITY,
    DB_TYPE_WIRELESS_COUNTRY,
    DB_TYPE_WIRELESS_AUTH_MODE,
    DB_TYPE_WIRELESS_ENCRYPTION,
    DB_TYPE_WIRELESS_AP,
    DB_TYPE_WIRELESS_XPRESS,
    DB_TYPE_WIRELESS_AFTERBURNER,
    DB_TYPE_WIRELESS_BAND,
    DB_TYPE_WIRELESS_RATE,
    DB_TYPE_WIRELESS_BASIC_RATE,
    DB_TYPE_WIRELESS_GMODE,
    DB_TYPE_WIRELESS_GPROTECTION,
    DB_TYPE_WIRELESS_KEYBIT,
    DB_TYPE_WIRELESS_FLT_MAC_MODE,
    DB_TYPE_WIRELESS_AP_MODE,
    DB_TYPE_WIRELESS_BRIDGE_RESTRICT_MODE,
    DB_TYPE_WIRELESS_LENGTH,
    DB_TYPE_WIRELESS_BANDWIDTH,
    DB_TYPE_WIRELESS_CTLSIDEBAND,    
    DB_TYPE_ADSL_PAIR,
    DB_TYPE_TIME,
    DB_TYPE_DAY,
    DB_TYPE_VOICE_CODEC,
    DB_TYPE_ETH_SPEED,
    DB_TYPE_ETH_TYPE,
    DB_TYPE_NONE
} DB_OBJ_TYPE;

typedef DB_OBJ_VALID_STATUS (*DB_VALIDATE_OBJ_FNC) (const char *objValue);

typedef struct {
   DB_OBJ_TYPE objType;
   DB_VALIDATE_OBJ_FNC validateObjFnc;
} DB_OBJ_VALID, *PDB_OBJ_VALID;

/********************** Global APIs Definitions *****************************/

#if defined(__cplusplus)
extern "C" {
#endif
    DB_OBJ_VALID_STATUS BcmDb_validateObj(DB_OBJ_TYPE objType, const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateRange(const char *objValue, int min, int max);
    DB_OBJ_VALID_STATUS BcmDb_validateLength(const char *objValue, unsigned int max);
    DB_OBJ_VALID_STATUS BcmDb_validateBoolean(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateNumber(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateHexNumber(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateString(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateIpAddress(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateSubnetMask(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateMacAddress(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmVpi(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmVci(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmCategory(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmAalType(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmAdminStatus(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAtmEncapsulation(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateFtpPath(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateUserName(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePassword(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePppUserName(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePppPassword(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePppTimeout(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePppAuthMethod(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateProtocol(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateIpProtocol(const char *objValue);
 	DB_OBJ_VALID_STATUS BcmDb_validateEtheProtocol(const char *objValue);//add by w00135358 for VDF GLOBAL QOS 20080908
    DB_OBJ_VALID_STATUS BcmDb_validateMacProtocol(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateInterfaceName(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateServiceName(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateState(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateDhcpState(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateEncapsulation(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateAccess(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateSysLogLevel(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateSysLogOption(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateRipOperation(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateMacFilterDirection(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateMacFilterPolicy(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateQosPriority(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessCountryEx(const char *objValue, int index_adaptor);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessCountry(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessAuthMode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessEncryption(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessStrength(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessGenOnOff(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessGenStr(const char *objValue, const char *validStr[],int *index);    
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessAP(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessXpress(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessAfterBurner(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessBand(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessRate(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessBasicRate(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessGmode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessOnOffAuto(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessKeybit(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessFltMacMode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessBridgeRestrictMode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessApMode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessBrigdeRestrictMode(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessLength(const char *objValue, unsigned int length,
                                                  unsigned int lenHex);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessPreamble(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessBandwidth(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessCtrlSideband(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateWirelessNmode(const char *objValue);    
    DB_OBJ_VALID_STATUS BcmDb_validateXmlAndGetWirelessTR69cObjs(const char *objValue, const char *objName, int *outVal, char* outName);
    DB_OBJ_VALID_STATUS BcmDb_validateAdslPair(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateTime(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateDay(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validatePort(const char *objValue, int min, int max);
    DB_OBJ_VALID_STATUS BcmDb_validateEthSpeed(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateEthType(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateCodec(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateVoiceCodec(const char *objValue);
    DB_OBJ_VALID_STATUS BcmDb_validateLocale(const char *objValue);

    DB_OBJ_VALID_STATUS BcmDb_validateDomain(const char *objValue) ;
    /* j00100803 Add Begin 2008-02-28*/
    #ifdef SUPPORT_POLICY
    DB_OBJ_VALID_STATUS BcmDb_validateIpAddressAndSubnetMask(const char *objValue, int iSubnetMask);
    DB_OBJ_VALID_STATUS BcmDb_validateIpProtocolForRouting(const char *objValue);
    #endif
    /* j00100803 Add End 2008-02-28 */
    /* BEGIN: Added by y67514, 2008/9/11   PN:GLB:Ip Extension*/
    DB_OBJ_VALID_STATUS BcmDb_validateIpExtMac(const char *objValue) ;
    /* END:   Added by y67514, 2008/9/11 */
#if defined(__cplusplus)
}
#endif

#endif
