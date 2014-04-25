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
//  Filename:       psixml.h
//  Author:         yt
//  Creation Date:  05/11/04
//
******************************************************************************
//  Description:
//      Define the global types, enums. and constants for PSI in XML format
//
*****************************************************************************/

#ifndef __PSIXML_H__
#define __PSIXML_H__

#include "bcmtypes.h"
#include "psidefs.h"
#include "objectdefs.h"
#include "wldefs.h"
#include "nanoxml.h"
/*start of 用户密码不加密功能129990 */
#ifndef  SUPPORT_SYSPWD_UNENCRYPTED
#define  SUPPORT_SYSPWD_UNENCRYPTED  1
#endif
/*end of 用户密码不加密功能129990 */
/*modified by w00135351 09.3,18 密码明文暂不合入*/

/*add by w00135351 09.3.13*/
#define SUPPORT_TR69CPWD_UNENCRYPTED
#define SUPPORT_FTPPWD_UNENCRYPTED
#define SUPPORT_SIPBSCPWD_UNENCRYPTED
#define SUPPORT_PPPPWD_UNENCRYPTED
#define SUPPORT_PPPSRVPWD_UNENCRYPTED
#define SUPPORT_SAMBA_ACCOUNT_UNENCRYPTED
#define SUPPORT_DDNSWD_UNENCRYPTED
/*end by w00135351 09.3.13*/

#define  TRUE                                    1
#define  FALSE                                  0
#define PSX_NAME_LEN                   24
#define PSX_MAGIC_LEN                  16
#define PSX_MAGIC_VALUE                "-$-PSI-$02.12r01"
#define PSX_CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define PSX_CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */
#define PSX_STR_LEN                    50 
#define PSX_VERIFY                     1
#define PSX_STORE                      0
#define PSX_TABLE_TYPE                 2
#define PSX_APP_TYPE                   1
#define PSX_OBJ_TYPE                   0
#define IP_LEN                                16
#define MONDAY        0x1
#define TUESDAY       0x2
#define WEDNSDAY      0x4
#define THURSDAY      0x8
#define FRIDAY        0x10
#define SATURDAY      0x20
#define SUNDAY        0x40
/* start of 支撑 问题单：A36D03168   by xujunxia 43813 2006年11月28 */
#define ATM_MAX_PCR       255000
#define ATM_MAX_SCR       255000
#define ATM_MAX_MBS       1000000
/* end of 支撑 问题单： A36D03168   by xujunxia 43813 2006年11月28 */

#define XML_PSI_START_TAG      "<psitree>"
#define XML_PSI_END_TAG        "</psitree>"

#define XML_PORT_RANGE_MIN     1
#define XML_PORT_RANGE_MAX     65535
#define XML_MAX_SNMP_STR_LEN   15
#define XML_MAX_TOD_ENTRY      16
#define XML_MAX_MAC_ENTRY      32 // add by l66195 for VDF
#ifdef SUPPORT_MACMATCHIP
#define XML_MAX_MACMATCH_ENTRY 16 
#endif


/* start of 增加tr143功能*/
typedef struct {
	char diag_state[256];		/* down DiagnosticsState */
	char diag_inter[256];		/* Interface */
	char diag_downURL[256];		/*  DownloadURL*/
	uint32 diag_dscp;			/* DSCP */
	uint32 diag_etherPriority;		/* EthernetPriority */
	
}DIAG_TR143_Down, *pDIAG_TR143_Down;

typedef struct {
	char diag_state_up[256];	/* up DiagnosticsState */
	char diag_inter_up[256];	/* Interface */
	char diag_upURL[256]; 	/* UploadURL */
	uint32  diag_dscp_up;		/* DSCP */
	uint32 diag_ether_up;		/* EthernetPriority */
	uint32 diag_test_up;		/* TestFileLength */
}DIAG_TR143_UP, *pDIAG_TR143_UP;

typedef struct {
    uint32 Enable;
    uint32 UDPPort;
    char Interface[256];
    char SourceIPAddress[256];
    uint32 EchoPlusEnabled;
}DIAG_TR143_UDP;

/* end of 增加tr143 功能*/


/* string  */
#define PSX_ATTRNAME_SET       1
#define PSX_ATTRSTR_SET        2

/* object and attribute names definitions */
#define PSX_ATTR_STATE             "state"

typedef enum {
   PSX_STS_OK = 0,
   PSX_STS_ERR_GENERAL,
   PSX_STS_ERR_MEMORY,
   PSX_STS_ERR_FIND_APP,
   PSX_STS_ERR_FIND_OBJ,
   PSX_STS_ERR_FIND_HDL_FNC,
   PSX_STS_ERR_FATAL,
   PSX_STS_ERR_MILD
} PSX_STATUS;

/* internal callback statemachine */
typedef enum {
   state_start,
   state_psi_begin,  
   state_psi_end
} xml_state;


typedef struct xmlParseState {
   xml_state state;
   char appName[32];
   PSI_HANDLE appHandle;
   char objName[32];
   char attrName[32];
   char tableName[32];
   int tableSize;
   int tableIndex;
   PSX_STATUS errorFlag;
   int verify;                    /* verify=1 verifcation only, verfify=0 conversion only */
} XML_PARSE_STATE, *pXML_PARSE_STATE;

typedef PSX_STATUS (*PSX_SET_OBJ_FNC) (FILE *file, const char *appName, const UINT16 id,
                                       const PSI_VALUE value, const UINT16 length);
typedef PSX_STATUS (*PSX_GET_OBJ_FNC) (char *appName, char *objName, char *attrName,
				       char* attrValue);
typedef void (*PSX_SET_ATTR_FNC) (FILE *file, const char *name, const UINT16 id,
				  const PSI_VALUE value, const UINT16 length);

typedef PSX_STATUS (*PSX_GET_ATTR_FNC) (char *attrName, char *attrValue);
typedef PSX_STATUS (*PSX_OBJ_FNC) (char *appName,  char *objName);



typedef struct {
   char  appName[PSX_NAME_LEN];
   PSX_SET_OBJ_FNC setObjFnc;
   PSX_GET_OBJ_FNC getObjFnc;     /* base on appName, find the object's getObjFnc */
   PSX_OBJ_FNC startObjFnc;       /* base on appName, find the object's startObjFnc */
   PSX_OBJ_FNC endObjFnc;         /* base on appName, find the object's endObjFnc */
} PSX_APP_ITEM, *PPSX_APP_ITEM;

typedef struct {
   char  objName[PSX_NAME_LEN];
   PSX_SET_ATTR_FNC setAttrFnc;
   PSX_GET_ATTR_FNC getAttrFnc;  /* getting object from XML file, do verification/store */
   PSX_OBJ_FNC startObjFnc;     /* beginning of object, resource allocated */
   PSX_OBJ_FNC endObjFnc;       /* end of object, do verification/storeToPSI, oresource deallocated */
} PSX_OBJ_ITEM, *PPSX_OBJ_ITEM;

#if defined(__cplusplus)
extern "C" {
#endif

/* start of 增加tr143 功能*/
extern PSX_STATUS xmlSetTR143ObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length) ;
extern PSX_STATUS xmlGetTR143ObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue);
extern PSX_STATUS xmlStartTR143ObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndTR143ObjNode(char *appName, char *objName);



/* end of 增加tr143 功能*/

extern PSX_STATUS xmlSetObjNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetSysObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetRouteObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetPMapObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetLanNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetWirelessNode(FILE* file, const char *appName, const UINT16 id,
			      const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetWanNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetAtmTdNode(FILE* file, const char *appName, const UINT16 id,
			   const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetAtmVccNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetAtmCfgNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetSecObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetAdslObjNode(FILE* file, const char *appName, const UINT16 id,
			     const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetSntpNode(FILE* file, const char *appName, const UINT16 id,
			  const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetToDNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetDhcpClntNode(FILE* file, const char *appName, const UINT16 id,
			      const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetPppNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlSetDdnsNode(FILE* file, const char *appName, const UINT16 id,
			  const PSI_VALUE value, const UINT16 length);

extern void xmlSetTr69cAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
	       	 const UINT16 length);
extern void xmlSetMgmtLockAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
	       	 const UINT16 length);
extern void xmlSetStrObjAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                      const UINT16 length);
extern void xmlSetSysLogAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);
extern void xmlSetDftGwAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		     const UINT16 length);
extern void xmlSetDnsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern void xmlSetOtherSysAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern void xmlSetSnmpAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
extern void xmlSetRouteAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
extern void xmlSetRipIfcAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);
extern void xmlSetRipAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern void xmlSetPMapAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern void xmlSetLanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
//modified by z67625 for lint
extern void xmlWlSetWebAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars);
extern void xmlWlSetEncrytionAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars);
extern void xmlWlSetEncryptionStrengthAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars);
extern void xmlWlSetRadiusServerAttr(FILE *file, PWIRELESS_MSSID_VAR wlVars);
//modified by z67625 for lint
extern void xmlSetWirelessVarsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
extern void xmlSetWirelessMacFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				const UINT16 length);
extern void xmlSetAtmInitAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern void xmlSetAtmTrafficModeAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length);
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern void xmlSetAtmPortAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length);
extern void xmlSetAtmTdNumAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern void xmlSetAtmTdTblAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern void xmlSetAtmIfcAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);
extern void xmlSetAtmVccNumAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			 const UINT16 length);
extern void xmlSetAtmVccTblAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern void xmlSetDmzHostAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length);
extern void xmlSetServiceCtrlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
 /*start of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */
extern void xmlSetHttpObjAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                      const UINT16 length);
extern PSX_STATUS xmlGetHttpPortAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSysHttpPortObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSysHttpPortObj(char *appName, char *objName);
 /*end of 增加西班牙wan 侧http 登陆port 需求 by s53329  at  20080708  */
/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#ifdef  SUPPORT_KPN_QOS     // macro QoS for KPN
extern void xmlSetWredInfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
#endif
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
extern void xmlSetVirtualSrvAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);
extern void xmlSetIpFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);
extern void xmlSetPrtTriggerAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			  const UINT16 length);
extern void xmlSetMacFilterAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			 const UINT16 length);
extern void xmlSetAccessControlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			     const UINT16 length);
extern void xmlSetAccessControlModeAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				 const UINT16 length);
extern void xmlSetQosAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern void xmlSetToDCntrlAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern void xmlSetDdnsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
extern void xmlSetEthAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern PSX_STATUS xmlParse(char *xmlBuffer, int xmlSize, int verify);
extern void xmlTagBegin(nxml_t handle, const char *tag_name, unsigned len);
extern void xmlTagEnd(nxml_t handle, const char *tag_name, unsigned len);
extern void xmlAttrBegin(nxml_t handle, const char *tag_name, unsigned len);
extern void xmlAttrValue(nxml_t handle, const char *tag_name, unsigned len);
extern void xmlData(nxml_t handle, const char *tag_name, unsigned len);
extern PSX_STATUS xmlGetObjNode(char *appName, char *objTagName,
	         		char *attrName, char *value);
extern PSX_STATUS xmlVerifyObjNode(char *appName, char *objTagName,
			    char *attrName, char *value);
extern PSX_STATUS xmlStartObjNode(char *appName, char *objTagName);
extern PSX_STATUS xmlEndObjNode(char *appName, char *objTagName);

extern PSX_STATUS xmlGetSysObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSysObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSysObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetSysUserNameAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlGetSysPasswordAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSysUserNamePasswordObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSysUserNamePasswordObj(char *appName, char *objName);
extern PSX_STATUS xmlGetTr69cAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartTr69cObj(char *appName, char *objName);
extern PSX_STATUS xmlEndTr69cObj(char *appName, char *objName);
extern PSX_STATUS xmlGetMgmtLockAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartMgmtLockObj(char *appName, char *objName);
extern PSX_STATUS xmlEndMgmtLockObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDftGwAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartDftGwObj(char *appName, char *objName);
extern PSX_STATUS xmlEndDftGwObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDnsAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartDnsObj(char *appName, char *objName);
extern PSX_STATUS xmlEndDnsObj(char *appName, char *objName);
extern PSX_STATUS xmlGetSyslogAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSyslogObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSyslogObj(char *appName, char *objName);
extern PSX_STATUS xmlGetSnmpAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSnmpObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSnmpObj(char *appName, char *objName);
extern PSX_STATUS xmlGetOtherSysAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartOtherSysObj(char *appName, char *objName);
extern PSX_STATUS xmlEndOtherSysObj(char *appName, char *objName);
extern PSX_STATUS xmlGetAtmVccObjNode(char *appName, char *objName, char *attrName,
                                      char* attrValue);
extern PSX_STATUS xmlGetAtmVccAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartAtmVccObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndAtmVccObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetWanObjNode(char *appName, char *objName, char *attrName,
                                   char* attrValue);
extern PSX_STATUS xmlGetWanAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartWanObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndWanObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetPppObjNode(char *appName, char *objName, char *attrName,
                                   char* attrValue);
extern PSX_STATUS xmlGetPppAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartPppObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndPppObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetSecObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSecObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSecObjNode(char *appName, char *objName);
/*add by z67625 过滤开关参数设置函数声明 start*/
extern PSX_STATUS xmlStartSecEnblObj(char * appName, char * objName);
extern PSX_STATUS xmlEndSecEnblObj(char * appName, char * objName);
extern PSX_STATUS xmlGetSecEnblAttr(char * attrName, char * attrValue);
extern void xmlSetSecEnblAttr(FILE * file, const char * name, const UINT16 id, const PSI_VALUE value, 
            const UINT16 length);
/*add by z67625 过滤开关参数设置函数声明 end*/
extern PSX_STATUS xmlGetVrtSrvAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndVrtSrvObj(char *appName, char *objName);
extern PSX_STATUS xmlGetVrtSrvEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndVrtSrvEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetIpFilterAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndIpFilterObj(char *appName, char *objName);
extern PSX_STATUS xmlGetIpFilterEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndIpFilterEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetPrtTriggerAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndPrtTriggerObj(char *appName, char *objName);
extern PSX_STATUS xmlGetPrtTriggerEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndPrtTriggerEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetMacFilterAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndMacFilterObj(char *appName, char *objName);
extern PSX_STATUS xmlGetMacFilterEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndMacFilterEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetQosClsAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartQosClsEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndQosClsObj(char *appName, char *objName);
extern PSX_STATUS xmlGetQosClsEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndQosClsEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetAccCntrlAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndAccCntrlObj(char *appName, char *objName);
extern PSX_STATUS xmlGetAccCntrlEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndAccCntrlEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetAccCntrlModeAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartAccCntrlModeObj(char *appName, char *objName);
extern PSX_STATUS xmlEndAccCntrlModeObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDmzHostAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartDmzHostObj(char *appName, char *objName);
extern PSX_STATUS xmlEndDmzHostObj(char *appName, char *objName);
extern PSX_STATUS xmlGetServiceCtrlAttr(char *attrName, char* attrValue);
/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
extern PSX_STATUS xmlGetWredInfoAttr(char *attrName, char* attrValue);
#endif
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
extern PSX_STATUS xmlStartServiceCtrlObj(char *appName, char *objName);
/* start of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
extern PSX_STATUS xmlStartWredInfoObj(char *appName, char *objName);
#endif
#endif
/* end of protocol 加入的 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
extern PSX_STATUS xmlEndServiceCtrlObj(char *appName, char *objName);
/* start of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */
#ifdef SUPPORT_VDSL
#ifdef SUPPORT_KPN_QOS // macro QoS for KPN
extern PSX_STATUS xmlEndWredInfoObj(char *appName, char *objName);
#endif
#endif
/* end of protocol 加入对 QoS for KPN的支持 by z45221 zhangchen 2006年6月27日 */

#ifdef SUPPORT_MAC //add by l66195 for VDF
extern PSX_STATUS xmlSetMacNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetMacObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue);
extern PSX_STATUS xmlStartMacObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndMacObjNode(char *appName, char *objName);
extern void xmlSetMacAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern PSX_STATUS xmlGetMacEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlGetMacAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndMacEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndMacObj(char *appName, char *objName);
#endif

#ifdef SUPPORT_MACMATCHIP 
extern PSX_STATUS xmlSetMacMatchNode(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetMacMatchObjNode(char *appName, char *objName, char *attrName,
			     char* attrValue);
extern PSX_STATUS xmlStartMacMatchObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndMacMatchObjNode(char *appName, char *objName);
extern void xmlSetMacMatchAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			const UINT16 length);
extern PSX_STATUS xmlGetMacMatchEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlGetMacMatchAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndMacMatchEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndMacMatchObj(char *appName, char *objName);
#endif

extern PSX_STATUS xmlGetDhcpClntNode(char *appName, char *objName, char *attrName,
                                   char* attrValue);
extern PSX_STATUS xmlGetDhcpClntAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartDhcpClntNode(char *appName, char *objName);
extern PSX_STATUS xmlEndDhcpClntNode(char *appName, char *objName);
extern PSX_STATUS xmlGetAtmTdObjNode(char *appName, char *objName, char *attrName,
                                   char* attrValue);
extern PSX_STATUS xmlGetAtmTdAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartAtmTdObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndAtmTdObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetLanNode(char *appName, char *objName, char *attrName,
                                char* attrValue);
extern PSX_STATUS xmlGetLanAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartLanObj(char *appName, char *objName);
extern PSX_STATUS xmlEndLanObj(char *appName, char *objName);
extern PSX_STATUS xmlGetWirelessObjNode(char *appName, char *objName, char *attrName,
                                        char* attrValue);
extern PSX_STATUS xmlStartWirelessObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndWirelessObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetWirelessVarsAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartWirelessVarsObj(char *appName, char *objName);
extern PSX_STATUS xmlEndWirelessVarsObj(char *appName, char *objName);
extern PSX_STATUS xmlGetWirelessMacFilterEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndWirelessMacFilterEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetWirelessMacFilterAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndWirelessMacFilterObj(char *appName, char *objName);
extern void xmlSetWirelessMssidVarsAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetWirelessMssidVarsAttr(char * attrName, char * attrValue);
extern PSX_STATUS xmlEndWirelessMssidVarsObj(char *appName, char *objName);
extern PSX_STATUS xmlGetWirelessMssidVarsEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndWirelessMssidVarsEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetRouteObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartRouteObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndRouteObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetRouteAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndRouteObj(char *appName, char *objName);
extern PSX_STATUS xmlGetRouteEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndRouteEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetRipAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartRipObj(char *appName, char *objName);
extern PSX_STATUS xmlEndRipObj(char *appName, char *objName);
extern PSX_STATUS xmlGetRipIfcAttr(char *attrName, char* attrValue);
/*GLB:s53329,start add for DNS*/
extern  PSX_STATUS xmlEndDnsInfoObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDnsInfoAttr(char *attrName, char* attrValue);
extern void xmlSetDnsEntryAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		      const UINT16 length);

extern PSX_STATUS xmlDnsEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDnsEntryAttr(char *attrName, char* attrValue);
extern  PSX_STATUS xmlEndDnsObjNode(char *appName, char *objName);
extern  PSX_STATUS xmlSetPDnsObjNode(FILE* file, const char *appName, const UINT16 id,
                            const PSI_VALUE value, const UINT16 length);
extern  PSX_STATUS xmlGetDnsObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue);
extern  PSX_STATUS xmlStartDnsObjNode(char *appName, char *objName);
/*GLB:s53329,end add for DNS*/
extern PSX_STATUS xmlEndRipIfcObj(char *appName, char *objName);
extern PSX_STATUS xmlGetRipIfcEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndRipIfcEntryObj(char *appName, char *objName);

extern PSX_STATUS xmlGetPMapObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartPMapObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndPMapObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetPMapAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndPMapObj(char *appName, char *objName);
extern PSX_STATUS xmlGetPMapEntryAttr(char *attrName, char* attrValue);
extern void xmlSetPMapIfcCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                   const UINT16 length);
extern PSX_STATUS xmlGetPMapIfcCfgAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartPMapIfcCfgObj(char *appName, char *objName);
extern PSX_STATUS xmlEndPMapIfcCfgObj(char *appName, char *objName);
extern PSX_STATUS xmlEndPMapEntryObj(char *appName, char *objName);

extern PSX_STATUS xmlGetDdnsObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartDdnsObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndDdnsObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetDdnsAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndDdnsObj(char *appName, char *objName);
extern PSX_STATUS xmlGetDdnsEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndDdnsEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetAtmCfgNode(char *appName, char *objName, char *attrName,
                                   char* attrValue);
extern PSX_STATUS xmlStartAtmCfgNode(char *appName, char *objName);
extern PSX_STATUS xmlEndAtmCfgNode(char *appName, char *objName);
extern PSX_STATUS xmlGetAtmInitAttr(char *attrName, char* attrValue);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern PSX_STATUS xmlGetAtmTrafficModeAttr(char *attrName, char* attrValue);
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern PSX_STATUS xmlStartAtmInitObj(char *appName, char *objName);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern PSX_STATUS xmlStartAtmTrafficModeObj(char *appName, char *objName);
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern PSX_STATUS xmlEndAtmInitObj(char *appName, char *objName);
/* start of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */
extern PSX_STATUS xmlEndAtmTrafficModeObj(char *appName, char *objName);
/* end of PROTOCOL PTM by zhouyi 00037589 2006年7月11日 */

extern PSX_STATUS xmlGetAdslNode(char *appName, char *objName, char *attrName,
                                 char* attrValue);
extern PSX_STATUS xmlGetAdslAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartAdslObj(char *appName, char *objName);
extern PSX_STATUS xmlEndAdslObj(char *appName, char *objName);

extern PSX_STATUS xmlGetSntpNode(char *appName, char *objName, char *attrName,
                                 char* attrValue);
extern PSX_STATUS xmlGetSntpAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSntpObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSntpObj(char *appName, char *objName);
extern PSX_STATUS xmlGetToDObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartToDObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndToDObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetToDAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndToDObj(char *appName, char *objName);
extern PSX_STATUS xmlGetToDEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndToDEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlSetVoiceNode(FILE* file, const char *appName, const UINT16 id,
                                  const PSI_VALUE value, const UINT16 length);

extern PSX_STATUS xmlGetVoiceNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartVoiceNode(char *appName, char *objName);
extern PSX_STATUS xmlEndVoiceNode(char *appName, char *objName);
extern void  xmlSetMgcpAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                            const UINT16 length);
extern void  xmlSetSipAttr(FILE* file, const char *appName, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlStartMgcpObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndMgcpObjNode(char *appName, char *objName);
extern PSX_STATUS xmlGetMgcpAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSipObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSipObjNode(char *appName, char *objName);
/* start of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日*/
// SIP Server CFG table
extern void  xmlSetSipSrvAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSipSrvAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSipSrvObjNode(char *appName, char *objName);
// SIP Server CFG entry
extern PSX_STATUS xmlGetSipSrvEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSipSrvObjEntryNode(char *appName, char *objName);

// SIP BASIC table
extern void  xmlSetSipBscAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSipBscAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSipBscObjNode(char *appName, char *objName);
// SIP BASIC entry
extern PSX_STATUS xmlGetSipBscEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSipBscObjEntryNode(char *appName, char *objName);
// SIP Advanced
extern void  xmlSetSipAdvAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSipAdvAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSipAdvObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSipAdvObjNode(char *appName, char *objName);
// SIP QoS
extern void  xmlSetSipQosAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSipQosAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSipQosObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSipQosObjNode(char *appName, char *objName);
/* end of voice 增加SIP配置项 by z45221 zhangchen 2006年11月7日 */
/* BEGIN: Added by y67514, 2009/11/16   PN:voice provision*/
extern void  xmlSetSipProvAttr(FILE* file, const char *name, const UINT16 id,
                           const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSipProvAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSipProvObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSipProvObjNode(char *appName, char *objName);
/* END:   Added by y67514, 2009/11/16 */
extern PSX_STATUS xmlGetSipAttr(char *attrName, char* attrValue);

extern PSX_STATUS xmlGetEthAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlSetIppNode(FILE* file, const char *appName, const UINT16 id,
                                const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetIppNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlGetIppAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartIppObj(char *appName, char *objName);
extern PSX_STATUS xmlEndIppObj(char *appName, char *objName);
extern PSX_STATUS xmlSetIPSecNode(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetIPSecNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartIPSecNode(char *appName, char *objName);
extern PSX_STATUS xmlEndIPSecNode(char *appName, char *objName);
extern PSX_STATUS xmlSetCertNode(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetCertNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartCertNode(char *appName, char *objName);
extern PSX_STATUS xmlEndCertNode(char *appName, char *objName);


extern char *xmlWirelessGetPhyType(void);
extern int xmlWirelessGetCoreRev(void);
extern int xmlWirelessGetBands(void);
extern int xmlWirelessGetValidBand(int band);


extern PSX_STATUS xmlGetPortMirrorCfgAttr(char *attrName, char* attrValue) ;
extern PSX_STATUS xmlEndPortMirrorCfgObj(char *appName, char *objName) ;
extern void xmlSetPortMirrorStatusAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length) ;
extern PSX_STATUS xmlGetPortMirrorStatusAttr(char *attrName, char* attrValue) ;
extern PSX_STATUS xmlStartPortMirrorStatusAttr(char *appName, char *objName) ;
extern PSX_STATUS xmlEndPortMirrorStatusAttr(char *appName, char *objName) ;
extern void xmlSetPortMirrorCfgEntryAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length) ;
extern PSX_STATUS xmlGetPortMirrorCfgEntryAttr(char *attrName, char* attrValue) ;
extern PSX_STATUS xmlEndPortMirrorCfgEntryAttrObj(char *appName, char *objName) ;
extern PSX_STATUS xmlSetPortMirrorObjNode (FILE* file, const char *appName, const UINT16 id, const PSI_VALUE value, const UINT16 length) ;
extern PSX_STATUS xmlGetPortMirrorObjNode(char *appName, char *objName, char *attrName, char* attrValue) ;
extern PSX_STATUS xmlStartPortMirrorObjNode(char *appName, char *objName) ;
extern PSX_STATUS xmlEndPortMirrorObjNode(char *appName, char *objName) ;
/* start of maintain PSI 移植: 允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
extern PSX_STATUS xmlStartBrNode(char *appName, char *objName);
extern PSX_STATUS xmlSetBrNode(FILE* file, const char *appName, const UINT16 id, const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetBrAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlGetBrNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlEndBrNode(char *appName, char *objName);
/* end of maintain PSI 移植: 允许在纯桥方式下使能dhcp来获取ip地址，便于管理终端 by xujunxia 43813 2006年5月5日 */
/*start   of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
extern PSX_STATUS xmlSetHspaNode(FILE * file, const char *appName, const UINT16 id,
			const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetHspaNode(char *appName, char *objName, char *attrName,
                         char* attrValue);
extern PSX_STATUS xmlStartHspaObj(char *appName, char *objName);
extern PSX_STATUS xmlEndHspaObj(char *appName, char *objName);
/*end  of 增加 vdf  hspa 配置需求by s53329  at   20080910*/
#ifdef SUPPORT_DHCPOPTIONS
PSX_STATUS xmlStartDhcpOptionNode(char *appName, char *objName);
PSX_STATUS xmlSetDhcpOptionNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetDhcpOptionNode(char *appName, char *objName, 
                char *attrName, char* attrValue);
PSX_STATUS xmlEndDhcpOptionNode(char *appName, char *objName);
#endif
/* start of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */
PSX_STATUS xmlStartAutoUpgradeNode(char *appName, char *objName);
PSX_STATUS xmlSetAutoUpgradeNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetAutoUpgradeNode(char *appName, char *objName, 
                char *attrName, char* attrValue);
PSX_STATUS xmlEndAutoUpgradeNode(char *appName, char *objName);
/* end of maintain PSI移植：允许在网页上配置自动升级服务器，版本描述文件。 by xujunxia 43813 2006年5月10日 */

/*start  of 增加Radius 认证功能by s53329  at  20060714 */
extern void xmlSetRadiusAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
extern PSX_STATUS xmlGetRadiusAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartRadiusObj(char *appName, char *objName);
extern PSX_STATUS xmlEndRadiusObj(char *appName, char *objName);
/*end   of 增加Radius 认证功能by s53329  at  20060714 */
extern int base64Encode(const void *inp, int insize, char **outptr);
extern void base64Decode(unsigned char *dest, char *src, int *rawLength);
//#ifdef SUPPORT_BRVLAN
/*start vlan bridge, s60000658, 20060627*/

PSX_STATUS xmlSetVlanObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetVlanObjNode(char *appName, char *objName, char *attrName,
			    char* attrValue);
PSX_STATUS xmlStartVlanObjNode(char *appName, char *objName);
PSX_STATUS xmlEndVlanObjNode(char *appName, char *objName);
PSX_STATUS xmlStartVlanAttr(char *appName, char *objName);
void xmlSetVlanAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
PSX_STATUS xmlGetVlanAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndVlanObj(char *appName, char *objName);
void xmlSetVltbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
PSX_STATUS xmlGetVltbAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndVltbObj(char *appName, char *objName);
PSX_STATUS xmlGetVlanEntryAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndVlanEntryObj(char *appName, char *objName);
void xmlSetVLPortAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
			   const UINT16 length);
PSX_STATUS xmlGetVLPortAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndVLPortObj(char *appName, char *objName);
PSX_STATUS xmlGetVLPortEntryAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndVLPortEntryObj(char *appName, char *objName);
/*end vlan bridge, s60000658, 20060627*/
//#endif
#ifdef VOXXXLOAD
void xmlSetSipPhoneCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetSipPhoneCfgAttr(char *attrName, char* attrValue); 
PSX_STATUS xmlStartSipPhoneCfgObjNode(char *appName, char *objName); 
PSX_STATUS xmlEndSipPhoneCfgObjNode(char *appName, char *objName);
void xmlSetSipSpeedDialAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetSipSpeedDialTbAttr(char *attrName, char* attrValue); 
PSX_STATUS xmlEndSipSpeedDialTbNode(char *appName, char *objName);
PSX_STATUS xmlGetSipSpeedDialEntryAttr(char *attrName, char* attrValue); 
PSX_STATUS xmlEndSipSpeedDialEntryNode(char *appName, char *objName);
void xmlSetSipCallFWDAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length); 
PSX_STATUS xmlGetSipCallFWDTbAttr(char *attrName, char* attrValue); 
PSX_STATUS xmlEndSipCallFWDTbNode(char *appName, char *objName);
PSX_STATUS xmlGetSipCallFWDEntryAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndSipCallFWDEntryNode(char *appName, char *objName);
#endif

/*start of 增加dhcp主机标识功能by l129990 */
#ifdef SUPPORT_DHCPHOSTID
PSX_STATUS xmlStartDhcpHost12Node(char *appName, char *objName);
PSX_STATUS xmlSetDhcpHostNameNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetDhcpHostNameNode(char *appName, char *objName, 
                char *attrName, char* attrValue);
PSX_STATUS xmlEndDhcpHostNameNode(char *appName, char *objName); 
#endif
/*end of 增加dhcp主机标识功能by l129990 */
/*start of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
#ifdef SUPPORT_IGMPCONFIG
PSX_STATUS xmlStartIgmpProxyNode(char *appName, char *objName);
PSX_STATUS xmlSetIgmpProxytNode(FILE* file, const char *appName, const UINT16 id,
		        const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetIgmpProxyNode(char *appName, char *objName, char *attrName, char* attrValue);
PSX_STATUS xmlEndIgmpProxyNode(char *appName, char *objName);
#endif
/*end of增加igmp版本选择和组播组数配置功能 by l129990 2008,9,11*/
/*Begin: Add para of sip proxy port, by d37981 2006.12.12*/
#ifdef SUPPORT_SIPROXD 
extern void xmlSetSiproxdAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
extern PSX_STATUS xmlGetSiproxdAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSiproxdObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSiproxdObj(char *appName, char *objName);
#endif
/*Endof: Add para of sip proxy port, by d37981 2006.12.12*/
void xmlSetFtpinfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
PSX_STATUS xmlGetFtpinfoAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartFtpinfoObj(char *appName, char *objName);
PSX_STATUS xmlEndFtpinfoObj(char *appName, char *objName);


/*Begin:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
void xmlSetHttpInterceptAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                                 const UINT16 length);
PSX_STATUS xmlGetHttpInterceptAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartHttpInterceptAttr(char *appName, char *objName);
PSX_STATUS xmlEndHttpInterceptAttr(char *appName, char *objName);
/*End:Added by luokunling 2013/03/14 PN:增加配置配置节点开关http tercept.*/
/*Begin:Added by luokunling l192527 for IMSI inform,2011/9/23*/
void xmlSetIMSIAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
                                 const UINT16 length);
PSX_STATUS xmlGetIMSIAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndIMSIObj(char *appName, char *objName);
PSX_STATUS xmlStartIMSIObj(char *appName, char *objName);
/*End:Added by luokunling l192527,2011/9/23*/

/*d00104343 Webcamera start*/
#ifdef WEBCAM
void xmlSetCamerainfoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
            const UINT16 length);
PSX_STATUS xmlGetCamerainfoAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartCamerainfoObj(char *appName, char *objName);
PSX_STATUS xmlEndCamerainfoObj(char *appName, char *objName);
#endif
/*d00104343 Webcamera end*/

#ifdef SUPPORT_GLB_ALG
void xmlSetNatalgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
            const UINT16 length);
PSX_STATUS xmlGetNatalgAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartNatalgObj(char *appName, char *objName);
PSX_STATUS xmlEndNatalgObj(char *appName, char *objName);
#endif /* SUPPORT_GLB_ALG */

/* BEGIN: Added by y67514, 2008/10/27   PN:GLB:AutoScan*/
void xmlSetAutoserchAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,const UINT16 length);
PSX_STATUS xmlGetAutoserchAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartAutoserchObj(char *appName, char *objName);
PSX_STATUS xmlEndAutoserchObj(char *appName, char *objName);
/* END:   Added by y67514, 2008/10/27 */

/*start of add the Upnp Video Provision function by l129990,2010,2,21*/
void xmlSetUpnpVideoAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetUpnpVideoAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartUpnpVideoObj(char *appName, char *objName);
PSX_STATUS xmlEndUpnpVideoObj(char *appName, char *objName);
/*end of add the Upnp Video Provision function by l129990,2010,2,21*/

/*BEGIN: add the web config function by kf34753 zhourongfei */
void xmlSetWebAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetWebAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartWebObj(char *appName, char *objName);
PSX_STATUS xmlEndWebObj(char *appName, char *objName);
/*END: add the web config function by kf34753 zhourongfei */

#ifndef  VDF_PPPOU
#define VDF_PPPOU
#endif

#ifdef VDF_PPPOU

//l66195 add begin for pppousb
extern PSX_STATUS xmlGetE220Node(char *appName, char *objName, char *attrName,
                                char* attrValue);
extern PSX_STATUS xmlSetE220Node(FILE* file, const char *appName, const UINT16 id,
			 const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlStartE220Obj(char *appName, char *objName);
extern PSX_STATUS xmlEndE220Obj(char *appName, char *objName);
extern PSX_STATUS xmlGetE220Data(char *attrName, char *attrValue);
extern void xmlSetE220Data(FILE *file, const char *name, const UINT16 id, 
             const PSI_VALUE value, const UINT16 length);

extern PSX_STATUS xmlSetPppProfileObjNode(FILE* file, const char *appName, const UINT16 id,
			    const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetPppProfileObjNode(char *appName, char *objName, char *attrName, char* attrValue);
extern PSX_STATUS xmlStartPppProfileObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndPppProfileObjNode(char *appName, char *objName);
extern void xmlSetPppProfileAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern PSX_STATUS xmlGetPppProfileAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndPppProfileObj(char *appName, char *objName);
extern PSX_STATUS xmlGetPppProfileEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndPppProfileEntryObj(char *appName, char *objName);

//l66195 add end for pppousb

//d00104343 HSDPA
extern PSX_STATUS xmlSetVIBCfgInfoNode(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		    const UINT16 length);
extern PSX_STATUS xmlGetVIBCfgInfoNode(char *appName, char *objName, char *attrName,
                         char* attrValue);
extern PSX_STATUS xmlStartVIBCfgInfoNode(char *appName, char *objName);
extern PSX_STATUS xmlEndVIBCfgInfoNode(char *appName, char *objName);
#endif
//start add for vdf qos by p44980 2008.01.08
#ifdef SUPPORT_VDF_QOS
extern void xmlSetQosQueueAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern PSX_STATUS xmlGetQosQueueAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndQosQueueObj(char *appName, char *objName);
extern PSX_STATUS xmlGetQosQueueEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartQosQueueEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndQosQueueEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetQosCfgAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartQosCfgObj(char *appName, char *objName);
extern PSX_STATUS xmlEndQosCfgObj(char *appName, char *objName);
extern void xmlSetQosCfgAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		       const UINT16 length);
		       
extern void xmlSetQosServiceAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
		   const UINT16 length);
extern PSX_STATUS xmlGetQosServiceAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndQosServiceObj(char *appName, char *objName);
extern PSX_STATUS xmlGetQosServiceEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartQosServiceEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndQosServiceEntryObj(char *appName, char *objName);		       
#endif
//end add for vdf qos by p44980 2008.01.08
extern PSX_STATUS xmlSetSambaObjNode (FILE* file, const char *appName, const UINT16 id,
                                      const PSI_VALUE value, const UINT16 length);
extern PSX_STATUS xmlGetSambaObjNode(char *appName, char *objName, char *attrName,
                                     char* attrValue);
extern PSX_STATUS xmlStartSambaObjNode(char *appName, char *objName);
extern PSX_STATUS xmlEndSambaObjNode(char *appName, char *objName);
extern void xmlSetSambaAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,
				                const UINT16 length);
extern PSX_STATUS xmlGetSambaAccntAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSambaAccntObj(char *appName, char *objName);
extern PSX_STATUS xmlStartSambaAccntEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetSambaAccntEntryAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlEndSambaAccntEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlGetSambaStatusAttr(char *attrName, char* attrValue);
extern PSX_STATUS xmlStartSambaStatusObj(char *appName, char *objName);
extern PSX_STATUS xmlEndSambaStatusObj(char *appName, char *objName);

/* j00100803 Add Begin 2008-02-23 */
#ifdef SUPPORT_POLICY
extern void xmlSetRtPolicyAttr(FILE* file, 
                                         const char *appName, 
                                         const UINT16 id,
                                         const PSI_VALUE value, 
                                         const UINT16 length);
extern PSX_STATUS xmlGetRtPolicyAttr(char *attrName,
                                                      char* attrValue);	
extern PSX_STATUS xmlEndRtPolicyObj(char *appName, char *objName);   
extern PSX_STATUS xmlGetRtPolicyEntryObj(char *appName, char *objName);	
extern PSX_STATUS xmlStartRtPolicyEntryObj(char *appName, char *objName);
extern PSX_STATUS xmlEndRtPolicyEntryObj(char *appName, char *objName);
#endif
/* j00100803 Add End 2008-02-23 */
#pragma SECFW_CFG
void xmlSetSecFwAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetSecFwAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartSecFwObj(char *appName, char *objName);
PSX_STATUS xmlEndSecFwObj(char *appName, char *objName);
#pragma SECFW_TB
void xmlSetSecFwTbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetSecFwTbAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndSecFwTbObj(char *appName, char *objName);

#pragma SECFW_ENTRY
PSX_STATUS xmlGetSecFwEntryAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartSecFwEntryObj(char *appName, char *objName);
PSX_STATUS xmlEndSecFwEntryObj(char *appName, char *objName);
#pragma SECDOS_CFG
void xmlSetSecDosAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value, const UINT16 length);
PSX_STATUS xmlGetSecDosAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartSecDosObj(char *appName, char *objName);
PSX_STATUS xmlEndSecDosObj(char *appName, char *objName);
#pragma SECURLBLK_TB
void xmlSetSecUrlBlkTbAttr(FILE *file, const char *name, const UINT16 id, const PSI_VALUE value,	   const UINT16 length);
PSX_STATUS xmlGetSecUrlBlkTbAttr(char *attrName, char* attrValue);
PSX_STATUS xmlEndSecUrlBlkTbObj(char *appName, char *objName);
#pragma SECURLBLK_ENTRY
PSX_STATUS xmlGetSecUrlBlkEntryAttr(char *attrName, char* attrValue);
PSX_STATUS xmlStartSecUrlBlkEntryObj(char *appName, char *objName);
PSX_STATUS xmlEndSecUrlBlkEntryObj(char *appName, char *objName);
#if defined(__cplusplus)
}
#endif

#endif /* __PSIXML_H__ */
