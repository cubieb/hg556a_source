/*----------------------------------------------------------------------*
<:copyright-broadcom

 Copyright (c) 2005 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
 *----------------------------------------------------------------------*
 * File Name  : tr69cdefs.h
 *
 *----------------------------------------------------------------------*/

#ifndef TR69C_DEFS_H
#define TR69C_DEFS_H

/*
* The CPE parameters are represented by a tree where each node in the
 * tree is an array of TRxObjNode structures. Each array item represents
 * either a paramater item or an object containing a pointer to the next
 * level of TRxObjNode items that make up  parameters that make up the object.
 *
 * Each item that describes an object contains a pointer component, *objDetail,
 * that points to the array of parameter and object items that form the
 * object.
 * The function pointer, getTRxParam, may be NULL or point to
 * a hardware dependent function that returns the value of the parameter in
 * a string variable.
 * The function pointer, setTRxParam, is used to call a hardware dependent
 * function to set the parameter. If the function pointer is NULL the
 * parameter is not writeable. If the item is an object and the pointer is
 * NULL the rpc function AddObject/DeleteObject are not supported by that
 * item.
 *
 * The global variable thisTRxObjNode points to the current TRxObjNode entry
 * when the set/getTRxParam functions are called.
 *
 * If the node contains a single TRxObjNode item with TRxType of tInstance
 * then this item represents all instances of the object. This function is
 * responsible for keeping track of all instances associated with this
 * object. The parameter handling framework expects the following functionality
 * of the getTRxParam function which will need to maintain state information
 * as the framework accesses the instances. The framework will always call
 * the getTRxParam to access an instance on its way to access its
 * sub-objects/parameters. As the framework is traversing the parameter
 * tree it will call the getTRxParam function with one of the following
 * forms to validate and set the instance state before proceeding
 * to the next object/parameter level.
 *
 * To determine the existance of a specific instance:
 *  The getTRxParam function is called as follows:
 *
 *  node->getTRXParam(char **ParamInstance)
 *  If *ParamInstance is not NULL then it points to the value to be found.
 *  Returns: TRx_OK if ParameterInstance found. The function should set
 *                  a global state variable for use by the next level
 *                  get/setTRxParam functions to the ParameterInstance.
 *           TRx_ERR if ParameterInstance not found
 *
 *  To retrieve each of the instances in order:
 *  If *ParamInstance is NULL then it returns the first instance of the
 *  object.
 *  Returns: TRx_OK if any Instances exist. The *ParamInstance pointer points to the
 *                  name (instance number string) of the first instance.
 *                  The global instance state variable is set to the
 *                  instance returned in the value string.
 *           TRx_ERR no instances of  this object exist.
 *
 *  If *ParamInstance is (void *)(-1) then find the next instance relative
 * to the last instance returned.
 *  Returns: TRx_OK The *ParamInstance pointer points to the next instance.
 *                  instance name. Repeated calls with the returned
 *                  instance name from the previous call as the InstanceValue
 *                  will return all instances. The global instance state
 *                  variable is the instance returned in the value string.
 *          TRx_ERR no more instances.
 * See xxx for an example of how this is coded.
 */

#include <time.h>
#include "cfg_typedef.h"

#if 0
typedef enum {
    TRX_OK=0,
    TRX_REBOOT,
    TRX_ERR
}TRX_STATUS;
#endif
typedef enum {
    NOTIFICATION_OFF = 0,
    PASSIVE_NOTIFICATION,
    ACTIVE_NOTIFICATION
} eNotification;

typedef enum {
    tUnknown=0,
    tObject,
    tString,
    tInt,
    tUnsigned,
    tBool,
    tDateTime,
    tBase64,
    tInstance,
    tStringSOnly		/* Set strings only */
} eTRxType;

typedef struct InstanceDesc {
    struct      InstanceDesc *next;
    struct      InstanceDesc *parent;
    int         instanceID;
    void        *hwUserData;
} InstanceDesc;

/* */
typedef struct InstanceDope {
    struct InstanceDope *next;
    InstanceDesc *instance;  /* set to currentInstance Desc*/
    char *pdata;
    unsigned notification:2;
    unsigned accessListIndex:1;
} InstanceDope;



typedef union TRxPAttrib {
    struct Attrib {
    eTRxType    etype:8;
    unsigned    slength:16;
    unsigned    inhibitActiveNotify:1; /* set to always inhibit change notification: use on counters */
    //unsigned    notification:2;
    //unsigned    accessListIndex:1;  /* is 0 for no Subscriber access or 1 to enable */
                                    /* subscriber access */
    } attrib;
    InstanceDesc    *instance;
} TRxPAttrib;
 /*start of 增加存贮参数改变通知功能 by s53329 at  20080314*/
typedef TRX_STATUS (*TRxINDEXGFUNC)(int  *index);
 /*end  of 增加存贮参数改变通知功能 by s53329 at  20080314*/
typedef TRX_STATUS (*TRxSETFUNC)(const char *value);
typedef TRX_STATUS (*TRxGETFUNC)(char **value);
 /*start of 增加存贮参数改变通知功能 by s53329 at  20080314*/
#define TRXINDEXGFUNC(XX) TRX_STATUS XX (int  *)
/*end  of 增加存贮参数改变通知功能 by s53329 at  20080314*/

#define TRXGFUNC(XX) TRX_STATUS XX (char **)
#define TRXSFUNC(XX) TRX_STATUS XX (const char *)

typedef struct TRxObjNode {
    const char  *name;
    TRxPAttrib  paramAttrib;
    TRxSETFUNC  setTRxParam;    /* only set if parameter is writeable */
    TRxGETFUNC  getTRxParam;
    void        *objDetail;
    InstanceDope *instanceDope;
    /*start of 增加存贮参数改变通知功能 by s53329 at  20080314*/
    int   index;
    TRxINDEXGFUNC   getTRParamAttr;
    /*end  of 增加存贮参数改变通知功能 by s53329 at  20080314*/
    
} TRxObjNode;


const char *getValTypeStr(eTRxType);


/*
* Define CPEVARNAMEINSTANCE in standardparams.c to create an
 * instance of all the CPE parameter strings.
 * undef VARINSTANCE to create a extern to the string pointer
 * If CPEVARNAMEINSTANCE is defined
 * SVAR(X) creates a char string constant of X and labels it with
 * the variable name X.
 * SSVAR(X,Y) creates a char string constant of Y and labels it with
 * the variable name X. This is used for strings that can't be C labels.
 *
 * If CPEVARNAMEINSTANCE is NOT defined SVAR generates
 * a extern of the form   extern const char X[];
*/
#ifdef CPEVARNAMEINSTANCE
/*#define mkstr(S) # S  */
#define SVAR(X) const char X[]=#X
#define SSVAR(X,Y) const char X[]=#Y
#else
#define SVAR(X) extern char X[]
#define SSVAR(X,Y) extern char X[]
#endif

/* extern variables and functions that are already defined in TR69 framework */
extern const char instanceIDMASK[];
int getInstanceCountNoPathCheck( TRxObjNode *n);
TRxObjNode  *getCurrentNode(void);
InstanceDesc *getCurrentInstanceDesc(void);
InstanceDesc *getNewInstanceDesc( TRxObjNode *n, InstanceDesc *parent, int id);
InstanceDesc *findInstanceDesc( TRxObjNode *n, int id);
InstanceDesc *findInstanceDescNoPathCheck( TRxObjNode *n, int id);
int deleteInstanceDesc( TRxObjNode *n, int id);

/* must match above enumberation of methods*/
typedef struct RpcMethods {
    unsigned   rpcGetRPCMethods:1;
    unsigned   rpcSetParameterValues:1;
    unsigned   rpcGetParameterValues:1;
    unsigned   rpcGetParameterNames:1;
    unsigned   rpcGetParameterAttributes:1;
    unsigned   rpcSetParameterAttributes:1;
    unsigned   rpcReboot:1;
    unsigned   rpcDownload:1;
    unsigned   rpcFactoryReset:1;
} RpcMethods;

typedef struct ACSState {
    char        *acsURL;        /* URL of ACS */
    char        *acsNewURL;     /* New URL if URL has been changed*/
    char        *acsUser;
    char        *acsPwd;
    time_t      informTime;     /* next ACS inform Time */
    time_t      informInterval; /* inform interval */
    int         informEnable;   /* True if inform to be performed*/
    int         maxEnvelopes;   /* Number of max env returned in inform response*/
    int         holdRequests;   /* hold request to ACS if true */
    int         noMoreRequests; /* don't send any more Req to ACS */
    RpcMethods  acsRpcMethods;  /* methods from GetRPCMethods response*/
    char        *parameterKey;  /* update key for ACS - may be NULL */
    char        *newParameterKey;  /* the pending key */
    char        *rebootCommandKey; /* key for reboot command key */
    char        *downloadCommandKey;    /* key for download cmd*/
    char        *connReqURL;
    char        *connReqPath;   /* path part of connReqURL -- used by listener */
    char        *connReqUser;
    char        *connReqPwd;
    char        *kickURL;
    char        upgradesManaged;
    char        *provisioningCode;
    int         retryCount;     /* reset on each ACS response*/
    int         fault;          /* last operation fault code */
    int         dlFaultStatus;  /* download fault status */
    char        *dlFaultMsg;    /* download fault message */
    time_t      startDLTime;    /* start download time */
    time_t      endDLTime;      /* complete download time*/
} ACSState;


// eCFMMsg is used in cfm/web/cgimain.c, cfm/api/bcmcfmdiag.c, cfm/util/psi/psiapi.cpp
/*start of HG_Support 2007.02.02 HG550V100R002C01B020 A36D03351  by l38280*/
//参数类型值不能为0
typedef enum {
    PARAM_CHANGE = 1,
    WEB_CONFIG_TR069,
    SEND_DIAG_COMPLETE,
    CLI_CONFIG_TR069,
    /* start of maintain 问题单：A36D02911 by xujunxia 43813 2006年10月8日 */
    GET_DHCP_OPTION43 = 5,
    /* end of maintain 问题单：A36D02911 by xujunxia 43813 2006年10月8日 */
    CWMP_MSG_TRANSFERCOMPLETE,
    CWMP_MSG_AUTHMODE,
    CWMP_MSG_WANIP,
    CWMP_MSG_SSLCERT_ENABLE = 9,
    CWMP_MSG_ASSOC_DEVICE,
    CWMP_MSG_TEST,
    TTYUSB_INFORM_TR069,
    /*Begin:Added by luokunling 00192527 for IMSI info requirement,2011/9/19*/
    CWMP_MSG_IMSI,
    CWMP_MSG_CONNECTED_IMSI_INFORM,
    TTYUSB_PLUGOUT_INFORM_TR069,
    /*End:Added by luokunling 00192527*/
	
    /*start of Global V100R001C01B020 AU8D00975 by c00131380 at 2008.11.4*/
    CWMP_MSG_REQUESTDOWNLOAD
    /*end of Global V100R001C01B020 AU8D00975 by c00131380 at 2008.11.4*/
} eCFMMsg;
/*end of HG_Support 2007.02.02 HG550V100R002C01B020 A36D03351  by l38280*/

#if defined(__cplusplus)
extern "C" {
#endif
// BcmTr69c_CfmMsg is used in cfm/web/cgimain.c, cfm/api/bcmcfmdiag.c, cfm/util/psi/psiapi.cpp
void BcmTr69c_CfmMsg(eCFMMsg msg);
#ifndef TESTBOX
// tr69c_main is used in cfm/api/bcmcfm.c
extern int  tr69c_main(void);
#endif
#if defined(__cplusplus)
}
#endif

#ifdef USE_DMALLOC
#include "dmalloc.h"
#endif // USE_DMALLOC

#endif   // TR69C_DEFS_H
