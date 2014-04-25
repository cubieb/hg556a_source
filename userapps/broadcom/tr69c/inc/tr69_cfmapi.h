#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/types.h>
#include "bcmtypes.h"
#include "tr69cdefs.h"

#ifndef TR069_CFMAPI_H
#define TR069_CFMAPI_H
/*start of 增加支持Tr069组播统计信息查询功能by l129990  at  2008,10,28*/
#define IGMPPROXY_GRP_MAX         20
#define IGMP_MSG_PATH            "/var"
#define IGMP_MSG_TR069_SEED		 '36'
#define IGMP_QUERY_FAILED          0
#define IGMP_QUERY_SUCCESSFUL      1
#define CLI_MAX_BUF_SZ            512
uint32 lIgmpProxyVersion;
uint32 lGrpCount ;
uint32 lGrpRepAllCount[IGMPPROXY_GRP_MAX];
uint32 lGrpRepOnlineCount[IGMPPROXY_GRP_MAX];
unsigned int lGroupAddr[IGMPPROXY_GRP_MAX];

typedef struct tagIgmpMsg
{
    unsigned int      ulGrpAddr;               /*组IP*/
    unsigned int      ulRepAddr;               /*成员IP*/
    char              acRepMac[32];            /*成员MAC*/
    char              acStime[24];             /*成员加入时间*/
    char              acLtime[24];             /*成员离开时间*/
   // unsigned long     ulPkts;                  /*成员接收报文个数*/
   // unsigned long     ulBytes;                 /*成员接收报文字节数*/
}IGMP_MSG_ST;

typedef struct tagMsgBuf
{
    long mtype;
    int version;
    IGMP_MSG_ST stIgmpMsg[32];
}IGMP_MSG_BUF;

IGMP_MSG_BUF stMsgBuf;
/*end of 增加支持Tr069组播统计信息查询功能by l129990  at  2008,10,28*/

typedef TRX_STATUS (*node_operator)(void *pvNode, void* pstArg);	
#if defined(__cplusplus)
extern "C" {
#endif
TRX_STATUS TR069_GetNodeByName(char *pszParaName, void **pvNode);
int TR069_ParaIsWritable(void* pvNode);
eTRxType TR069_GetParaValueTypeByNode(void* pvNode);
TRxGETFUNC TR069_GetGetParaFunByNode(void* pvNode);
TRxSETFUNC TR069_GetSetParaFunByNode(void* pvNode);
TRX_STATUS TR069_NodeRecursiveDo(void *pvNode, int nextLevel, 
                                    void* pstArg, node_operator function);
unsigned long TR069_GetParaValueLengthByNode(void* pvNode);
/*start  of  增加参数属性保存功能 by s53329  at  20080328*/
void  Cwmp_ValueNotifyPro( );
/*end  of  增加参数属性保存功能 by s53329  at  20080328*/
#if defined(__cplusplus)
}
#endif

#endif
