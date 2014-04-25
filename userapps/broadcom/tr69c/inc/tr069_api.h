/*
*   File Description: Common Data Structure for RPC method defined by TR-069 spec
*/

#ifndef TR069_API_H
#define TR069_API_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

typedef enum tagTR069ErrorType_E
{
    TR069ERR_SUCCESS_E,
    TR069ERR_NOTENOUGHMEM_E,
    TR069ERR_INVALIDPARA_E
}TR069ERRORTYPE_E;

/*  错误码*/
#define MAX_ERRORINFO_SIZE 16

#define METHOD_NOT_SUPPORTED_D 9000
#define REQUEST_DENIED_D 9001
#define INTERNAL_ERROR_D 9002
#define INVALID_ARGUMENTS_D 9003
#define RESOURCES_EXCEEDED_D 9004
#define INVALID_PARAMETER_NAME_D 9005
#define INVALID_PARAMETER_TYPE_D 9006
#define INVALID_PARAMETER_VALUE_D 9007
#define ATTEMPT_TO_SET_A_NONWRITABLE_PARAMETER_D 9008
#define NOTIFICATION_REQUEST_REJECTED_D 9009
#define DOWNLOAD_FAILURE_D 9010
#define UPLOAD_FAILURE_D 9011
#define FILE_TRANSFER_SERVER_AUTHENTICATION_FAILURE_D 9012
#define UNSUPPORTED_PROTOCOL_FOR_FILE_TRANSFER_D 9013
#define XML_PARSE_ERROR_D 9800
#define XML_STRUCTURE_ERROR_D 9801

typedef struct tagErrorInfo_S
{
    ULONG             ERRORCODE;
    CHAR *      ERRORSTRING;
}ErrorInfo_S;

extern ErrorInfo_S g_ErrorInfo[MAX_ERRORINFO_SIZE];

typedef enum tagDOWNLOADRESPONSE
{
    DOWNLOADRESPONSE_SUCCESS_E,
    DOWNLOADRESPONSE_INVALIDURL_E,
    DOWNLOADRESPONSE_FAILED_E,
    DOWNLOADRESPONSE_UNAUTHOR_E
}DOWNLOADRESPONSE_E;

// Common struct
typedef struct tagValueStruct
{
    CHAR*               pcInArgName;
    CHAR*               pcInArgValue;     // TODO: need changes
    /*start of 增加参数类型上报通知功能 by s53329  at  20080406*/
    ULONG                ulType;
    /*end of 增加参数类型上报通知功能 by s53329  at  20080406*/
    struct tagValueStruct*  pstNext;
}ValueStruct;

typedef struct tagerrStruct
{
    ULONG                            ulErrCode;
    CHAR*                                      pcErrStr;
    struct tagSetParaValuesFault*    pstNext;
}errStruct;

typedef struct tagSetParaValuesFault
{
    ULONG                             ulErrCode;
    CHAR*                                    pcErrStr;
    CHAR*                                    pcParaName ;
    struct tagSetParaValuesFault*  pstNext;
}SetParaValuesFault;

typedef enum tagRESPONSETYPE_E
{
    RESPONSE_ARGUMENTS,
    RESPONSE_ERRORINFO,
    RESPONSE_NOTCONNECTED,       // Connect Failure
    RESPONSE_UNAUTHORIZED,       // Unauthorized
    RESPONSE_TIMEOUT,               // Request Timeout in The Request Queue
    RESPONSE_FATALERROR            // Fatal Error
}RESPONSETYPE_E;

typedef struct tagResponseStruct
{
    RESPONSETYPE_E eFlag;
    union errOrOutArg
    {
        errStruct*    pstError;        //
        VOID*           pvOutArg;     //
    }uRes;
}ResponseStruct;

typedef enum tagRequiedParaNameIdx
{
    DEVICEID_MANUFACTURER,
    DEVICEID_OUI,
    DEVICEID_PRODUCTCLASS,
    DEVICEID_SERIALNUMBER,
    SPEC_VERSION,
    CPE_HARDWARE_VERSION,
    CPE_SOFTWARE_VERSION,
    PROVISIONING_CODE,
    CONNECTION_REQUESTURL,

}REQUIREDPARANAMEIDX_E;

typedef enum tagConnectionType
{
    IP_CONNECTION,
    PPP_CONNECTION
}CONNECTIONTYPE_E;


typedef struct tagExtIPAddr
{
    USHORT                    usWanDevice;
    USHORT                    usWanConnectionDev;
    USHORT                    usWanConnection;
    ULONG                          ulIpAddr;
    CONNECTIONTYPE_E  eConnectionType;

    struct tagExtIPAddr*    pstNext;
}ExtIPAddr;

typedef struct tagEventQueue
{
    CHAR*                           pcEventCode;
    CHAR*                           pcCmdKey;
    struct tagEventQueue*   pstNext;
}EventQueue;
extern EventQueue*  g_pstInformEvtQ;
typedef struct tagParaList
{
    CHAR*                                   reqiredList[CONNECTION_REQUESTURL+1];          // Should be inited when powerup
    CHAR*                                   pcParaKey;               // This variable is changed frequently
                                                                            // It does not need Inform except explicitly told
    ValueStruct *                        ExternalIpaddr;
    ValueStruct*                        optionalList;        // Need to be deleted after each Inform
}ParaList;

//extern ParaList*  g_pstInformPara;

//typedef ResponseStruct* (*rpc_method)(VOID *pvInArg);
typedef VOID (*rpc_method)(VOID *pvInArg, ResponseStruct *outArg);

/*-----------------------------CPE Methods-------------------------------*/
typedef struct tagOutArgGetRPCMethods
{
    CHAR**            ppMethods;
    USHORT            usListNum;
}OutArgGetRPCMethods;

// SetParameterValue
typedef struct tagInArgSetParaValue
{
    ValueStruct*     pstArgList;
    CHAR*               pcParaKey;
}InArgSetParaValue;

//GetParameterValue
typedef struct tagInArgGetParaValue
{
    CHAR**              ppcArgName;
    USHORT              usArgNum;
}InArgGetParaValue;

typedef ValueStruct OutArgGetParaValue;
/*
typedef struct tagOutArgGetParaValue
{
    ValueStruct *pstArgList;
    LONG usArgNum;
}OutArgGetParaValue;
*/

// GetParameterNames
typedef struct tagInArgGetParaNames
{
    CHAR*               pcParaPath;
    BOOL                 bNextLevel;      // Boolean
}InArgGetParaNames;

typedef struct tagOutArgGetParaNames
{
    CHAR*               pcParaName;
    BOOL                 bWritable;
    struct tagOutArgGetParaNames*   pstNext;
}OutArgGetParaNames;

// SetParameterAttribute

/*
typedef enum tagParaAttr
{
    ParaAttr_Change,
    ParaAttr_Access
}ParaAttr;

enum Para_Attributes
{
    Para_Attrib_NotificationType0,
    Para_Attrib_NotificationType1,
    Para_Attrib_NotificationType2,
    Para_Attrib_AccessList
};
*/

typedef struct tagInArgSetParaAttr
{
    CHAR*             pcName;
    //ParaAttr          attrFlag;               // Allowed value must be of type Para_Attributes
    BOOL               bNotificationChange; // Boolean, whether can be changed
    BOOL               bAccessListChange;   // Boolean, whether can be accessed
    USHORT           usNotification;           // Notification frequency
    CHAR**           ppcAccessList;
    USHORT           usListLength;         // list Length

    struct tagInArgSetParaAttr *pstNext;
}InArgSetParaAttr;

// GetParameterAttribute
typedef struct tagInArgGetParaAttr
{
    CHAR**         ppcParaNames;
    USHORT         usParaNum;
}InArgGetParaAttr;

typedef struct tagOutArgGetParaAttr
{
    CHAR*             pcName;
    USHORT           usNotification;           // Notification frequency
    CHAR**           ppcAccessList;
    USHORT           usListLength;         // list Length

    struct tagOutArgGetParaAttr *pstNext;
}OutArgGetParaAttr;


// AddObject
typedef struct tagInArgAddObject
{
    CHAR*               pcObjName;
    CHAR*               pcParaKey;
}InArgAddObject;

typedef struct tagOutArgAddObject
{
    USHORT            usInstanceNum;
    BOOL                bStatus;
}OutArgAddObject;

// DeleteObject
typedef InArgAddObject  InArgDelObject;

typedef USHORT*            OutArgDelObject;

// DownLoad
typedef struct tagInArgDownload
{
    CHAR*                     pcCmdKey;
    CHAR*                     pcFileType;
    CHAR*                     pcUrl;
    CHAR*                     pcUserName;
    CHAR*                     pcPassWord;
    CHAR*                     pcTargetFileName;
    CHAR*                     pcSuccessUrl;
    CHAR*                     pcFailureUrl;
    ULONG           ulFileSize;
    ULONG           ulDelaySec;
}InArgDownload;

typedef struct tagOutArgDownload
{
    BOOL                    bStatus;
    time_t                   tmStartTime;
    time_t                   tmCompleteTime;
}OutArgDownload;

// Reboot, no out put arguments
typedef CHAR*           InArgReboot;

// Upload
typedef struct tagInArgUpload
{
    CHAR*                     pcCmdKey;
    CHAR*                     pcFileType;
    CHAR*                     pcUrl;
    CHAR*                     pcUserName;
    CHAR*                     pcPassWord;
    ULONG           ulDelaySec;
}InArgUpload;

typedef OutArgDownload  OutArgUpload;


/*-----------------------------ACS Methods-------------------------------*/
// Common struct


typedef struct tagFaultStruct
{
    USHORT                   usFaultCode;
    CHAR*                     pcFaultString;
}FaultStruct;

// TransferComlete
typedef struct tagInArgTransferComlete
{
    CHAR*               pcCmdKey;
    time_t                tmStartTime;
    time_t                tmCompleteTime;
    FaultStruct          pstFault;
}InArgTransferComlete;
// No output

// RequestDownload
typedef struct tagInArgRequestDownload
{
    CHAR*                      pcFileType;
    ValueStruct*              pstFileTypeArg;
}InArgRequestDownload;
// No output


/*-----------------------------Inform Structs-------------------------------*/




typedef enum tagMethodIndex_E
{
    GET_RPCMETHODS,
    SET_PARAMETER_VALUES,
    GET_PARAMETER_VALUES,
    GET_PARAMETER_NAMES,
    SET_PARAMETER_ATTRIBUTES,
    GET_PARAMETER_ATTRIBUTES,
    ADD_OBJECT,
    DELETE_OBJECT,
    DOWNLOAD,
    REBOOT,
    UPLOAD,
    FACTORY_RESET,
    GET_RPCMETHODS_RESPONSE,
    REQUEST_DOWNLOAD_RESPONSE,
    TRANSFER_COMPLETE_RESPONSE,
    INFORM_RESPONSE,
    ERROR_RESPONSE
}METHODINDEX_E;


typedef enum tagE_FileTransferInfo
{
    FILETRANSFER_SUCCEED,
    FILETRANSFER_CONNECTIONFAILED,
    FILETRANSFER_INTERRUPTTED,
    FILETRANSFER_SENDFAILED,
    FILETRANSFER_OPENFILEERROR,
    FILETRANSFER_BUSY,
    FILETRANSFER_SENDING,
    FILETRANSFER_UNAUTHOR
}E_FileTransferInfo;

#ifdef __cplusplus
}
#endif

#endif


