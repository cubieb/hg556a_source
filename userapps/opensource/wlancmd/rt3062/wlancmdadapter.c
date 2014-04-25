/******************************************************************************
  版权所有  : 2007-2020，华为技术有限公司
  文 件 名  : wlancmdadapter.c
  作    者  : l69021
  版    本  : 1.0
  创建日期  : 2007-8-22
  描    述  : 
  函数列表  :
                wlan_assoclist
                wlan_bssid
                wlan_channel
                wlan_cmdverison
                wlan_cnxt
                wlan_down
                wlan_driververison
                wlan_firmwareverison
                wlan_hide
                wlan_mode
                wlan_phytype
                wlan_pskkey
                wlan_rate
                wlan_rateset
                wlan_region
                wlan_secmode
                wlan_ssid
                wlan_status
                wlan_txpwr
                wlan_up
                wlan_wepkey
                wlan_wepkeyindex
                wlan_wlanqos
                wlan_wpaencryption
                wlanChannel2Freq
                wlanCountryAbbrevToCountry
                wlanCountryNameToCountry
                wlanDumpRateset
                wlanEtherEToA
                wlanFind
                wlanFormatSsid
                wlanFreq2Channel
                wlanGet
                wlanGetFileVar
                wlanIoctl
                wlanMwToQdbm
                wlanParseIndex
                wlanParseRateset
                wlanPrintInt
                wlanPrintRate
                wlanQdbmToMw
                wlanRateInt2String
                wlanRateString2Int
                wlanSendIoctlToLed
                wlanSet
                wlanSetFileVar
                wlanSystem
                wlanVer2Str

  历史记录      :
   1.日    期   : 2007-8-22
     作    者   : l69021
     修改内容   : 完成初稿

*********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>    /* for sockaddr_in       */
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>    /* for connect and socket*/
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

//#define WLAN_DBG

#define WLAN_SYS_LEN           256
#define WLAN_8_LEN             8
#define WLAN_16_LEN            16
#define WLAN_32_LEN            32
#define WLAN_64_LEN            64

/*******************************************************************************************/
//Ralink Jimmy 2009-1-5 Create
/*******************************************************************************************/

#include "function.h"
#include "globals.h"
#include "oid.h"


//**************************************************************//
TcWlanMode GVWlanModeList[] =
{
    {  "bonly",    1 },
    {  "gonly",    4 },
    {  "nonly",    6 },        
    {  "bgmixed",  0 },
    {  "gnmixed",  7 },
    {  "bgnmixed", 9 },
};        

TcWlanChannelMode GVWlanChannelList[] =
{
    {"1",      8},
    {"2",      9},
    {"5.5",    10},
    {"6",      0},
    {"9",      1},
    {"11",     11},
    {"12",     2},
    {"18",     3},
    {"24",     4},
    {"36",     5},
    {"48",     6},
    {"54",     7},
    {"6.5",    0},
    {"13",     1},
    {"19.5",   2},
    {"26",     3},
    {"39",     4},
    {"52",     5},
    {"58.5",   6},
    {"65",     7},
    {"13",     8},
    {"26",     9},
    {"39",     10},
    {"52",     11},
    {"78",     12},
    {"104",    13},
    {"117",    14},
    {"130",    15}, 
    {"300",    32}, //(BW=1, SGI=0) HT duplicate 6Mbps
    {"0",      33},
};        
TcWlanCountry WlanCountryChannelList[] =
{
    {  "JP", '5' },    
    {  "US", '0' }, 
    {  "CA", '0' },
    {  "MX", '0' },
/*  {  "Other", '1' },*/
};

#define WLAN_LOG_FILE "/var/wlancmd.log"



#if 0
#define INIC_INFNAME_PREFIX  "wl"
#define INIC_INFNAME_OTHER_PREFIX  "wl0."
#define INIC_INFNAME_MAIN    "wl0"
#else
#define INIC_INFNAME_PREFIX  "ath"
#define INIC_INFNAME_OTHER_PREFIX  "ath"
#define INIC_INFNAME_MAIN    "ath0"
#endif



ULONG RTDebugLevel = RT_DEBUG_ERROR;

#define WLAN_DBG(Level, Format...) \
{ \
    if (Level <= RTDebugLevel)      \
    { \
        fprintf(stderr, "\n[<Level:%d>%s, %s, %d] ", Level, __FILE__, __FUNCTION__, __LINE__); \
        fprintf(stderr, Format); \
    }\
}



//**************************************************************//
// ioctl function                                                //                                        
//**************************************************************//
int OidQueryInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
    struct iwreq wrq;

    strcpy(wrq.ifr_name, DeviceName);
    
    wrq.u.data.length  = PtrLength;
    wrq.u.data.pointer = (caddr_t) ptr;
    wrq.u.data.flags   = OidQueryCode;

    //fprintf(stderr, "query oid=0x%04x\n", OidQueryCode);
    return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}
    
/****************************************************************************
*****************************************************************************/
int OidSetInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
    struct iwreq wrq;

    strcpy(wrq.ifr_name, DeviceName);
    wrq.u.data.length = PtrLength;
    wrq.u.data.pointer = (caddr_t) ptr;
    wrq.u.data.flags = OidQueryCode | OID_GET_SET_TOGGLE;

    //fprintf(stderr, "set oid=0x%04x, flags=%04x\n", OidQueryCode, wrq.u.data.flags);
    return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}

int wlan_exec_cmd(const char *string)
{
    FILE *fp = NULL;
    
    if (NULL != (fp = fopen(WLAN_LOG_FILE, "a")))
    {
        fprintf(fp, "\t[%s]\n", string);
    
        fclose(fp);
    }
    else
    {
        printf("Open wlan log file error %d\n", errno);
    }

    return system(string);
}

/****************************************************************************
*****************************************************************************/
int iwpriv_set(int socket_id, char *DeviceName, char *parameter, char *value)
{
    unsigned char data[WLAN_SYS_LEN];
    struct iwreq wrq;
    int ret;
    
    memset(data, 0x00, WLAN_SYS_LEN);
    sprintf(data, "%s=%s", parameter, value);

    /* add debug log file */
    {
        FILE *fp = NULL;

        if ( NULL != (fp = fopen(WLAN_LOG_FILE, "a")))
        {
            fprintf(fp, "\t[iwpriv %s set %s]\n", DeviceName, data);

            fclose(fp);
        }
        else
        {
            printf("Open wlan log file error %d\n", errno);
        }
    }

    strcpy(wrq.ifr_name, DeviceName);
    wrq.u.data.length  = strlen(data) + 1;
    wrq.u.data.pointer = data;
    wrq.u.data.flags   = 0;
    
    ret = ioctl(socket_id, RTPRIV_IOCTL_SET, &wrq);
    if (ret != 0)
    {
        printf("@@Error, iwpriv_set:set %s \n", parameter);
    }
    
    return 0;
}


int WriteParamToFile(char *name, char *value)
{
    FILE *fp = NULL;    
    int iLen = 0;
    unsigned int bNeedWriteBack = 0;

    char acNameBuf[WLAN_LINE_BUF_LEN];
    char acRdBuf[WLAN_CFG_FILE_SIZE];
    char acWrBuf[WLAN_CFG_FILE_SIZE];

    if (NULL == (fp = fopen(WLAN_CFG_FILE_PATCH, "rb+"))) 
    {
        perror("open wlan config file erro.\n");
        return -1;
    }

    memset(acRdBuf, 0x0, WLAN_CFG_FILE_SIZE);
    memset(acWrBuf, 0x0, WLAN_CFG_FILE_SIZE);
    strcpy(acNameBuf, name);
    
    while (fgets(acRdBuf, WLAN_LINE_BUF_LEN, fp) != NULL) 
    {
        if (NULL != (strstr(acRdBuf, acNameBuf))) 
        {
            sprintf(acNameBuf, "%s=%s\n", acNameBuf, value);
            strcat(acWrBuf, acNameBuf);
            bNeedWriteBack = 1;
            
        }
        else 
        {
            strcat(acWrBuf, acRdBuf);
        }
        
        if ((strlen(acWrBuf) + WLAN_LINE_BUF_LEN) > WLAN_CFG_FILE_SIZE)
        {
            printf("Fatal Error: Config File<%s> too big.\n", WLAN_CFG_FILE_PATCH);
            break;
        }
    }
    if (bNeedWriteBack == 1) 
    {
        iLen = strlen(acWrBuf);
        fseek(fp, 0, SEEK_SET);
        if (fwrite(acWrBuf, 1, iLen, fp) != iLen)
        {
            printf("Write file len %d err, errorno=%d.\n", iLen, errno);
            return FAIL;
        }
        else
        {
            fclose(fp);
            return SUCCE;
        }
    }

    fclose(fp);
    
    return FAIL;
}


int wlancmdGetNum(char **argv)
{
    int iRet = 0;
    while ( NULL != *(argv+iRet) )
    {
        iRet++;
    }
    iRet++;
    return iRet;
}

/******************************************************************************
  函数名称  : wlan_cmdverison
  功能描述  : 显示WLANCMD的版本信息
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_cmdverison(void *wl, char **argv)
{
    if (!strcmp(argv[1] , "display"))
    {
        printf(HuaWeiWLANDriverVer);
    }

    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_firmwareverison
  功能描述  : 显示WLAN芯片firmware的版本信息
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_firmwareverison(void *wl, char **argv)
{
    if (!strcmp(argv[1] , "display"))
    {
        printf(HuaWeiWlancmdVer);
    }
    
    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_driververison
  功能描述  : 显示WLAN驱动的版本信息 (wl_version实现)
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : wlanVer2Str
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_driververison(void *wl, char **argv)
{
    char buf[32];
    int  socket_id;

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFirmwarever!\n");    
                
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (OidQueryInformation(RT_OID_VERSION_INFO, socket_id, INIC_INFNAME_MAIN, buf, 0) < 0)
    {
        printf("ERROR:RT_OID_VERSION_INFO\n");
    }
    else
    {
        printf("%s\n", buf);
    }

    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_up
  功能描述  : 启用WLAN接口,功能相当于ifconfig xxx up (wl_void实现)
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_up(void *wl, char **argv)
{
    unsigned char cmd[WLAN_32_LEN];

    memset(cmd, 0x00, WLAN_32_LEN);
    sprintf(cmd, "ifconfig %s up", INIC_INFNAME_MAIN);
    wlan_exec_cmd(cmd);

    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_down
  功能描述  : 去使能WLAN接口,功能相当于ifconfig xxx down  (wl_void实现)
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿
******************************************************************************/
int wlan_down(void *wl, char **argv)
{
    unsigned char cmd[WLAN_32_LEN];

    memset(cmd, 0x00, WLAN_32_LEN);
    sprintf(cmd, "ifconfig %s down", INIC_INFNAME_MAIN);
    wlan_exec_cmd(cmd);    

    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_mode
  功能描述  : 配置/显示无线的工作模式。IEEE 802.11b, IEEE 802.11g, b/g混合模式
              (wl_gmode实现)
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv: bonly,gonly,mixed
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_mode(void *wl, char **argv)
{
    int   iWMNumbers;
    int   iWMIndex;
    char *cIdeaWM;
    char  cIwprivValue[WLAN_16_LEN];
    int   socket_id;
    
    int argc = wlancmdGetNum(argv);
    
    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdMode!\n");    
                
    iWMNumbers = sizeof(GVWlanModeList)/sizeof(GVWlanModeList[0]);

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
        
    if ((!strcmp(argv[1], "set")) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "\r\n wlan_mode set come in , argv[2] = %s\r\n", argv[2]);
        for (iWMIndex = 0 ; iWMIndex < iWMNumbers ; iWMIndex++)
        {
            cIdeaWM = GVWlanModeList[iWMIndex].cWirelessMode;      // Get Idea Command

            if(strcmp(argv[2], cIdeaWM) == 0)
            {            
                WLAN_DBG(RT_DEBUG_TRACE, "iWirelessModeNumber=%d, iWMIndex=%d\n",
                              GVWlanModeList[iWMIndex].iWirelessModeNumber, iWMIndex);
                memset(cIwprivValue, 0x00, WLAN_16_LEN);
                sprintf(cIwprivValue, "%d", GVWlanModeList[iWMIndex].iWirelessModeNumber);
                iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WirelessMode", cIwprivValue);    
            }
        }
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        int status=0;
        if (OidQueryInformation(RT_OID_GET_PHY_MODE,socket_id, INIC_INFNAME_MAIN ,&status, 4)<0)
        {
            printf("ERROR:RT_OID_GET_PHY_MODE\n");
        }
        else
        {
            printf("PHY mode status=%d\n", status);
        }
    }
    else
    {
        close(socket_id);
        return FAIL;
    }
      
    close(socket_id);
    
    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_rate
  功能描述  : 配置/显示无线的传输速率。
              802.11b模式时，支持的速率包括1, 2, 5.5, 11Mbps
              802.11g模式时，支持的速率包括1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 
               48, 54Mbps
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿
******************************************************************************/
void SetHtMcs(char *Mode)
{
    char cIwprivValue[3];
    int  socket_id;
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);

    if (!strcmp(Mode, "B"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "1");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", "3");        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "CCK");          
    }
    else if (!strcmp(Mode, "G"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "0");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", "351");        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "OFDM");          
    }
    else if (!strcmp(Mode, "N"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "1");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", "15");        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "0");         
    }
    else if (!strcmp(Mode, "BG"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "1");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", "3");        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "1");        
    }
    else if (!strcmp(Mode, "GN"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "0");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", "351");        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "0");        
    }
    else if (!strcmp(Mode, "BGN"))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtOpMode", "0");
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FixedTxMode", "2");
    }
}


int wlan_rate(void *wl, char **argv)
{
    int    socket_id;
    char   cIwprivValue[WLAN_16_LEN];
    char  *cIdeaWM;
    int    iWMNumbers;
    int    iWMIndex;
    int    ret;
    unsigned long Mcs_tmp;
    
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdRate!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3)) 
    {
        return FAIL;
    }
    
    memset(cIwprivValue, 0x00, WLAN_16_LEN);

    iWMNumbers = sizeof(GVWlanChannelList)/sizeof(GVWlanChannelList[0]);
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        ret = GetPhyMode();
        if (ret == 1)    //B only
        {
            SetHtMcs("B");
        }
        else if (ret == 4)    //G only
        {
            SetHtMcs("G");
        }
        else if (ret == 6)    //N only
        {
            SetHtMcs("N");
        }
        else if (ret == 0)    //BG mixed
        {
            SetHtMcs("BG");
        }
        else if (ret == 7)    //GN mixed
        {
            SetHtMcs("GN");
        }
        else if (ret == 9)    //BGN MIXED
        {
            SetHtMcs("BGN");
        }
        for ( iWMIndex = 0 ; iWMIndex < iWMNumbers ; iWMIndex++ )
        {
            cIdeaWM = GVWlanChannelList[iWMIndex].cChannelRate;                // Get Idea channel code
            if(strcmp(argv[2], cIdeaWM) == 0)
            {    
                memset(cIwprivValue, 0x00, WLAN_16_LEN);
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd rate set:%s\n", cIdeaWM);    
                WLAN_DBG(RT_DEBUG_TRACE, "iRateCount = %d  \r\n", GVWlanChannelList[iWMIndex].iRateCount);
                sprintf(cIwprivValue, "%d", GVWlanChannelList[iWMIndex].iRateCount);    
                iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtMcs", cIwprivValue);
            }
        }
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        unsigned char ucname[30];
        unsigned char data[255];
        struct iwreq wrq;
        
        memset(ucname, 0x00, 30);
        sprintf(ucname, "%s", INIC_INFNAME_MAIN);
        memset(data, 0x00, 255);
        strcpy(wrq.ifr_name, ucname);
        wrq.u.data.length = 255;
        wrq.u.data.pointer = data;
        wrq.u.data.flags = 0;
        ioctl(socket_id, SIOCGIWRATE , &wrq);
        printf("\n%dMbps\n", (int)(wrq.u.bitrate.value/1000000));
    }
    else
    {
        close(socket_id);    
        return FAIL;
    }
            
    close(socket_id);    
    return SUCCE;
}


int GetPhyMode(void)
{
#define DATA_LEN 2048
#define CMD_LEN  5
    
    int socket_id;
    int ret;
    char data[DATA_LEN];
    char cmd[CMD_LEN];
    struct iwreq wrq;

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    memset(data, 0x00, DATA_LEN);
    memset(cmd, 0x00, CMD_LEN);
    strcpy(data, "");
    sprintf(cmd, INIC_INFNAME_MAIN);
    strcpy(wrq.ifr_name, cmd);//
    wrq.u.data.length = DATA_LEN;
    wrq.u.data.pointer = data;
    wrq.u.data.flags = 0;
    ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq);
    if(ret != 0)
    {
        printf("GetPhyMode display error\n");
        close(socket_id);
        return FAIL;
    }
    RT_802_11_MAC_TABLE *mp;
    int i,j;
    mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
    ret = mp->PhyMode;
    printf("PhyMode=%d\n",ret);
    close(socket_id);
    
    return ret;
}



/******************************************************************************
  函数名称  : wlan_rateset
  功能描述  : 配置/显示无线的传输速率集
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_rateset(void *wl, char **argv)
{
    int  socket_id;
    char cIwprivValue[WLAN_16_LEN];

    int argc = wlancmdGetNum(argv);

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3) ||
        ((argc != 4) && ((argv[2][0] > 0x3A) || (argv[2][0] < 0x30))))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    memset(cIwprivValue, 0x00, WLAN_16_LEN);
    if (strcmp(argv[2], "default") == 0)
    {
        sprintf(cIwprivValue, "%s", "15");
    }
    if (strcmp(argv[2], "all") == 0)
    {
        sprintf(cIwprivValue, "%s", "4095");
    }

    WLAN_DBG(RT_DEBUG_TRACE, "Basicrate = %s  \r\n", cIwprivValue);
    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BasicRate", cIwprivValue);    
        
    close(socket_id);
    
    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_channel
  功能描述  : 配置/显示无线的工作信道，与国家有关
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_channel(void *wl, char **argv)
{
    int    socket_id;
    char   cIwprivValue[WLAN_8_LEN];
    unsigned long Channel_tmp;

    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdChannel!\n");    
        
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0))|| (argc < 3) ||
        ((argc == 4) && ((argv[2][0] > 0x3A) || (argv[2][0] < 0x30))))
    {
        return FAIL;
    }
    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        Channel_tmp = strtol(argv[2], 0, 10);

        memset(cIwprivValue, 0x00, WLAN_8_LEN);
        sprintf(cIwprivValue, "%d", (int)Channel_tmp);    
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "Channel", cIwprivValue);
        
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        struct iwreq wrq;
        char      ucname[WLAN_8_LEN],data[WLAN_SYS_LEN];
        memset(ucname, 0x00, WLAN_8_LEN);
        sprintf(ucname, "%s", INIC_INFNAME_MAIN);
        memset(data, 0x00, WLAN_SYS_LEN);
        strcpy(wrq.ifr_name, ucname);
            wrq.u.data.length = 255;
            wrq.u.data.pointer = data;
            wrq.u.data.flags = 0;
        ioctl(socket_id, SIOCGIWFREQ , &wrq);
        printf("current mac channel is %d\n", wrq.u.freq.m);
        printf("target channnel %d\n", wrq.u.freq.m);
    }
    else
    {
        close(socket_id);    
        return FAIL;    
    }    

    close(socket_id);    
    return SUCCE;    
}

/******************************************************************************
  函数名称  : wlan_region
  功能描述  : 配置/显示WLAN网络所在的国家或地区。符合ISO 3166的国家缩写代码
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_region(void *wl, char **argv)
{
    int   socket_id;
    int   iWMNumbers,iWMIndex;
    char  cIwprivValue[WLAN_8_LEN];
    char  ucInterface[WLAN_8_LEN];
    int   ret;
    char *cIdeaWM;

    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdRegion!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0))||(argc < 3))
    {
        return FAIL;
    }
    
    iWMNumbers = sizeof(WlanCountryChannelList)/sizeof(WlanCountryChannelList[0]);
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        for (iWMIndex = 0 ; iWMIndex < iWMNumbers ; iWMIndex++)
        {
            cIdeaWM = WlanCountryChannelList[iWMIndex].cCountryString;                // Get Idea Country code
            if(strcmp(argv[2], cIdeaWM) == 0)
            {            
                WLAN_DBG(RT_DEBUG_TRACE, "iWirelessModeNumber=%s, iWMIndex=%d\n", WlanCountryChannelList[iWMIndex].cCountryString, iWMIndex);
                memset(cIwprivValue, 0x00, WLAN_8_LEN);
                sprintf(cIwprivValue, "%c", WlanCountryChannelList[iWMIndex].iRegionChannel);
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd region set:%s\n", cIwprivValue);

                iwpriv_set(socket_id, INIC_INFNAME_MAIN, "CountryRegion", cIwprivValue);    
                break;
            }
        }
        
        if (iWMIndex == iWMNumbers)    
        {
            printf("iWirelessModeNumber=other\n");
            memset(cIwprivValue, 0x00, WLAN_8_LEN);
            sprintf(cIwprivValue, "%s", "1");
            printf("wlancmd region set:%s\n", cIwprivValue);
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "CountryRegion", cIwprivValue);
        }
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        struct iwreq wrq;
        
        strcpy(wrq.ifr_name, INIC_INFNAME_MAIN);
        wrq.u.data.length = 4;
        wrq.u.data.pointer = 0;
        wrq.u.data.flags = RT_OID_CountryRegion;
        ret = ioctl(socket_id, RT_PRIV_IOCTL, &wrq);
        if (ret != 0)
        {
            close(socket_id);
            return FAIL;
        }
        
        for (iWMIndex = 0; iWMIndex < iWMNumbers; iWMIndex++)
        {
            cIdeaWM = WlanCountryChannelList[iWMIndex].iRegionChannel; 
            if (cIdeaWM == wrq.u.data.flags)
            {
                printf("wlancmd Country display=%s\n", WlanCountryChannelList[iWMIndex].cCountryString);
            }
        }
        if (iWMIndex == iWMNumbers)    
        {
            printf("wlancmd Country display=%s\n", "other");
        }
    }    
    else
    {
        close(socket_id);    
        return FAIL;    
    }    
    
    close(socket_id);
    
    return SUCCE;        
}

/******************************************************************************
  函数名称  : wlan_phytype
  功能描述  : 显示无线的物理层类型。
              无线网络的物理层包括802.11b, 802.11g, 802.11n等类型
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_phytype(void *wl, char **argv)
{
    printf("PhyMode=0\n");  //Phy mode

    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_txpwr
  功能描述  : 
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_txpwr(void *wl, char **argv)
{
    int  socket_id;
    char cIwprivValue[WLAN_8_LEN];
    int  testB;
    unsigned long TxPower_tmp;
    
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdTxPower!\n");
    
    if ((strcmp(argv[1], "set") != 0) || (argc < 3))
    {
        return FAIL;
    }
        
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 5) && (strcmp(argv[2], "-d") == 0))
    {
        testB = atoi(argv[3]);
        printf("\r\n Txpower = %d   \r\n", testB);
        TxPower_tmp = strtol(argv[3], 0, 10);        
        printf("\r\n Txpower = %d   \r\n", TxPower_tmp);
        memset(cIwprivValue, 0x00, WLAN_8_LEN);
        
        //TxPower_tmp=(TxPower_tmp/6);//17~20=>EEPROM default,14~17=>-1db,10~14=>-3db,7~10=>-6db,4~7=>-9db,0~4=>-12db
        if (TxPower_tmp>17)
        {
            sprintf(cIwprivValue, "95");
        }
        else  if (TxPower_tmp>14)
        {
            sprintf(cIwprivValue, "70");
        }
        else if (TxPower_tmp>10)
        {
            sprintf(cIwprivValue, "50");
        }
        else if (TxPower_tmp>7)
        {
            sprintf(cIwprivValue, "20");
        }
        else if (TxPower_tmp>4)
        {
            sprintf(cIwprivValue, "10");
        }
        else if (TxPower_tmp>0)
        {
            sprintf(cIwprivValue, "0");
        }
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd txpwr set :%s \n",cIwprivValue);
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "TxPower", cIwprivValue);
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd txpwr set :%s \n",argv[2]);
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "TxPower", argv[2]);
    }
    else
    {
        close(socket_id);    
        return FAIL;        
    }

    close(socket_id);    
    return SUCCE;    
}

/******************************************************************************
  函数名称  : wlan_bssid
  功能描述  : 显示某一个BSSID. 
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_bssid(void *wl, char **argv)
{
    int   socket_id;
    char  ucInterface[WLAN_8_LEN];
    char  ucname[WLAN_32_LEN];
    char  data[WLAN_SYS_LEN];
    struct iwreq wrq;
    int   argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdSsid!\n");
    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0))|| (argc < 2))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "display") == 0) && (argc == 3))
    {
        memset(data, 0x00, WLAN_SYS_LEN);
        strcpy(wrq.ifr_name, INIC_INFNAME_MAIN);
        wrq.u.data.length  = 255;
        wrq.u.data.pointer = data;
        wrq.u.data.flags   = 0;
        ioctl(socket_id, SIOCGIFHWADDR , &wrq);
        printf("BSSID: \"%02X:%02X:%02X:%02X:%02X:%02X\"\n", 
        (unsigned char)wrq.u.ap_addr.sa_data[0],(unsigned char)wrq.u.ap_addr.sa_data[1],
        (unsigned char)wrq.u.ap_addr.sa_data[2],(unsigned char)wrq.u.ap_addr.sa_data[3],
        (unsigned char)wrq.u.ap_addr.sa_data[4],(unsigned char)wrq.u.ap_addr.sa_data[5]); 
    }
    else 
    {
        if ((strcmp(argv[1], "display") == 0) && (argc == 5))
        {
            if (strcmp(argv[2], "-a") == 0)
            {
                memset(ucname, 0x00, WLAN_32_LEN);
                sprintf(ucname, "%s", argv[3]);
                memset(data, 0x00, WLAN_SYS_LEN);
                strcpy(wrq.ifr_name, ucname);
                wrq.u.data.length = 255;
                wrq.u.data.pointer = data;
                wrq.u.data.flags = 0;
                ioctl(socket_id, SIOCGIFHWADDR , &wrq);
                printf("BSSID: \"%02X:%02X:%02X:%02X:%02X:%02X\"\n", 
                    (unsigned char)wrq.u.ap_addr.sa_data[0],(unsigned char)wrq.u.ap_addr.sa_data[1],
                    (unsigned char)wrq.u.ap_addr.sa_data[2],(unsigned char)wrq.u.ap_addr.sa_data[3],
                    (unsigned char)wrq.u.ap_addr.sa_data[4],(unsigned char)wrq.u.ap_addr.sa_data[5]); 
            }
            else
            {
                printf("Bssid display Bssid");    
            }
        }
        else
        {    
            close(socket_id);    
            return FAIL;    
        }
    }
    
    return SUCCE;
}
/******************************************************************************
  函数名称  : wlan_ssid
  功能描述  : 配置/显示某一个SSID
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_ssid(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    struct iwreq wrq;
    char ucname[WLAN_32_LEN];
    char data[WLAN_SYS_LEN];

    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdSsid!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0))|| (argc < 2))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "SSID", argv[2]);            //对第一个SSID进行操作
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {      
            memset(ucInterface, 0x00, WLAN_8_LEN);
            if(strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
            }              
            iwpriv_set(socket_id, ucInterface, "SSID", argv[4]);            
        }    
    }
    else 
    {
        if ((strcmp(argv[1], "display") == 0) && (argc == 3))
        {
            memset(data, 0x00, WLAN_SYS_LEN);
            strcpy(wrq.ifr_name, INIC_INFNAME_MAIN);
            wrq.u.data.length = 255;
            wrq.u.data.pointer = data;
            wrq.u.data.flags = 0;
            ioctl(socket_id, SIOCGIWESSID , &wrq);
            printf("\nSSID[%s]: %s\n", INIC_INFNAME_MAIN , wrq.u.essid.pointer);
        }
        else 
        {
            if ((strcmp(argv[1], "display") == 0) && (argc == 5))
            {
                memset(ucname, 0x00, WLAN_32_LEN);
                if(strcmp(argv[3], "0") == 0)
                {
                    sprintf(ucname, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
                }
                else
                {
                    sprintf(ucname, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
                }  
                memset(data, 0x00, WLAN_SYS_LEN);
                strcpy(wrq.ifr_name, ucname);
                wrq.u.data.length = 255;
                wrq.u.data.pointer = data;
                wrq.u.data.flags = 0;
                ioctl(socket_id, SIOCGIWESSID , &wrq);
                printf("\nSSID[%s]: %s\n", ucname , wrq.u.essid.pointer);
            }    
            else
            {
                close(socket_id);    
                return FAIL;    
            }    
        }
    }

    close(socket_id);
    
    return SUCCE;        
}

/******************************************************************************
  函数名称  : wlan_status
  功能描述  : 使能或禁止某一个SSID
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_status(void *wl, char **argv)
{
#if 0   
    int   socket_id;
    char  ucInterface[5];
    unsigned char cmd[50];
    int argc = wlancmdGetNum(argv);    

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdStatus!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
        
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    
    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        printf("wlancmd status set: %s\n", argv[2]);    
        memset(cmd, 0x00, 50);
        if ((strcmp(argv[2], "Enable") == 0)||(strcmp(argv[2], "1") == 0))
        {
            sprintf(cmd, "ifconfig %s up", INIC_INFNAME_MAIN);
        }
        else if((strcmp(argv[2], "Disable") == 0)||(strcmp(argv[2], "0") == 0))
        {
            sprintf(cmd, "ifconfig %s down", INIC_INFNAME_MAIN);
        }
        else
        {
            close(socket_id);    
            return FAIL;        
        }        
        wlan_exec_cmd(cmd);                    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            printf("interface number: ra%s\n", argv[3]);                    
            printf("wlancmd status set: %s\n", argv[4]);        
            memset(ucInterface, 0x00, 5);
            sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);                    
            if ((strcmp(argv[4], "Enable") == 0)||(strcmp(argv[4], "1") == 0))
            {
                printf("\r\n status enable come in  \r\n");
                sprintf(cmd, "ifconfig %s up", ucInterface);
            }
            else if((strcmp(argv[4], "Disable") == 0)||(strcmp(argv[4], "0") == 0))
            {
                printf("\r\n status disable come in  \r\n");
                sprintf(cmd, "ifconfig %s down", ucInterface);
            }    
            else
            {
                WlanCmdStatusHelpMes();
                close(socket_id);    
                return FAIL;
            }
            wlan_exec_cmd(cmd);
        }
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        printf("wlancmd Wireless interface status display no support yet!\n");
        printf("wlancmd Wireless interface status display=%d\n", 1);
    }    
    else
    {
        close(socket_id);    
        return FAIL;    
    }    
    
    close(socket_id);
#endif
    return SUCCE;    
}

/******************************************************************************
  函数名称  : wlan_hide
  功能描述  : 配置/显示无线的某一个SSID的隐藏状态
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_hide(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int argc = wlancmdGetNum(argv);
    char buf[32];

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdHide!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        if(!(strcmp(argv[2], "enable")))
        {
            strcpy(argv[2],"1");
        }
        if(!(strcmp(argv[2], "disable")))
        {
            strcpy(argv[2],"0");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HideSSID", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {        
            memset(ucInterface, 0x00, WLAN_8_LEN);
            strcpy(ucInterface, argv[3]);
            if(strcmp(argv[4] , "enable")==0)
            {
                strcpy(argv[4],"1");
            }
            if(strcmp(argv[4] , "disable")==0)
            {
                strcpy(argv[4],"0");
            }
            iwpriv_set(socket_id, ucInterface, "HideSSID", argv[4]);                    
        }    
    }
    else if (strcmp(argv[1], "display") == 0)    
    {
        if (OidQueryInformation(RT_OID_HIDESSID_STATUS, socket_id, INIC_INFNAME_MAIN, buf, 0) < 0)
        {
            printf("ERROR:RT_OID_HIDESSID_STATUS\n");
        }
        else
        {
            printf("%s\n", buf);
        }
    }
    else
    {
        close(socket_id);    
        return FAIL;    
    }
  
    close(socket_id);
    
    return SUCCE;    
}

/******************************************************************************
  函数名称  : wlan_assoclist
  功能描述  : 显示连接到某个SSID的STA的MAC地址。
              该函数特殊，"display"后面不带参数时，显示所有连接STA,而不是
              通常的默认值0.
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_assoclist(void *wl, char **argv)
{
    int socket_id;
    int ret, Apindex;
    char data[2048];
    char cmd[WLAN_8_LEN];
    struct iwreq wrq;
    int argc = wlancmdGetNum(argv);
    
    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdAssoclist!\n");    

    if (argc < 5)
    {
        return SUCCE;
    }
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (!(strcmp(argv[1], "display")) && (argc == 5))
    {            
        memset(data, 0x00, 2048);
        memset(cmd, 0x00, WLAN_8_LEN);
        strcpy(data, "");
        if (strcmp(argv[2], "-i") == 0)
        {
            if (!strcmp(argv[3], "0"))
            {
                sprintf(cmd, "%s", INIC_INFNAME_MAIN);
            }
            else
            {
                sprintf(cmd, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            if (!strcmp(argv[3], "0"))
            {
                Apindex=0;
            }
            else if (!strcmp(argv[3], "1"))
            {
                Apindex=1;
            }
            else if (!strcmp(argv[3], "2"))
            {
                Apindex=2;
            }
            else if (!strcmp(argv[3], "3"))
            {
                Apindex=3;
            }
        }
        else if (strcmp(argv[2], "-a") == 0)
        {
            strcpy(cmd, argv[3]);
               
            if (!strcmp(argv[3], "ath0"))
            {
                Apindex=0;
            }
            if (!strcmp(argv[3], "ath1"))
            {
                Apindex=1;
            }
            if (!strcmp(argv[3], "ath2"))
            {
                Apindex=2;
            }
            if (!strcmp(argv[3], "ath3"))
            {
                Apindex=3;
            }
        }
        
        strcpy(wrq.ifr_name, cmd);
        wrq.u.data.length = 2048;
        wrq.u.data.pointer = data;
        wrq.u.data.flags = 0;
        ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq);
        if (ret != 0)
        {
            printf("WlanCmdAssoclist ioctl open fail\n");
            close(socket_id);    
            return SUCCE;
        }
        
        RT_802_11_MAC_TABLE *mp;
        int i,j = 0;
	
        mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
        if (mp->Num == 0)
        {
            printf("WlanCmdAssoclist no sta existance\n");
            close(socket_id);    
            return SUCCE;
        }
        
        for(i=0; i<mp->Num; i++ )
        {        
            //printf("Sean@WlanCmdAssoclist Apindex[%d]-apidx[%d], aid[%d]\n", Apindex, mp->Entry[i].apidx, mp->Entry[i].Aid);
            if (mp->Entry[i].Aid == 0)
            {
    		   break;
            }
		  
	        if (Apindex == mp->Entry[i].apidx)
            {
                j++;
                printf("%s %02X:%02X:%02X:%02X:%02X:%02X\n", "assoclist",
                (unsigned char)mp->Entry[i].Addr[0], (unsigned char)mp->Entry[i].Addr[1],
                (unsigned char)mp->Entry[i].Addr[2], (unsigned char)mp->Entry[i].Addr[3],
                (unsigned char)mp->Entry[i].Addr[4], (unsigned char)mp->Entry[i].Addr[5]);
                
            }
        }
        if (0 == j)
        {
            printf("%s %s\n", "assoclist", "No STA Connected");
            printf("\r\n  Total Assoclist Number:%d \r\n", i);
        }
	else
	{
            printf("\r\n  Total Assoclist Number:%d \r\n", j);
	}
    }
    else
    {
        close(socket_id);    
        return SUCCE;
    }
   
EXIT_ASSOCLIST:
    close(socket_id);
    
    return SUCCE;
}

/******************************************************************************
  函数名称  : wlan_secmode
  功能描述  : 配置/显示某个SSID的安全方式.
              安全方式的设置有先后次序关.先设置基本安全方式none/basic/wpa,
              后设置鉴权，加密方式.
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_secmode(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdSecmode!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
        
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (argc > 3)
    {
        if (!strcmp(argv[4] , "wep"))
        {
            if ((strcmp(argv[1], "set") == 0) && (argc == 4))
            {
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepencryption set: %s\n", argv[2]);        
                iwpriv_set(socket_id, INIC_INFNAME_MAIN, "EncrypType", argv[2]);
                close(socket_id);    
                return SUCCE;    
            }
            else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
            {
                WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);                    
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wpaencryption set: %s\n", argv[4]);    //wepencryption    

                memset(ucInterface, 0x00, WLAN_8_LEN);
                if (strcmp(argv[3], "0") == 0)
                {
                    sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
                }
                else
                {
                    sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
                }
                
                iwpriv_set(socket_id, ucInterface, "EncrypType", argv[4]);
                close(socket_id);    
                return SUCCE;        
            }
            else
            {
                close(socket_id);    
                return FAIL;
            }
                
        }
        else if (!strcmp(argv[4] , "none"))
        {
            if ((strcmp(argv[1], "set") == 0) && (argc == 4))
            {
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepencryption set: %s\n", argv[2]);        
                iwpriv_set(socket_id, INIC_INFNAME_MAIN, "EncrypType", argv[2]);
                close(socket_id);    
                return SUCCE;    
            }
            else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
            {
                WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);                    
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wpaencryption set: %s\n", argv[4]);    //wepencryption
                
                memset(ucInterface, 0x00, WLAN_8_LEN);
                if (strcmp(argv[3], "0") == 0)
                {
                    sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);            
                }
                else
                {
                    sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
                }              
                iwpriv_set(socket_id, ucInterface, "EncrypType", argv[4]);
                close(socket_id);    
                return SUCCE;        
            }
            else
            {
                close(socket_id);    
                return FAIL;
            }
        }
        else if (!strcmp(argv[4] , "open"))
        {
            strcpy(argv[4] , "OPEN");
        }
        else if (!strcmp(argv[4] , "share"))
        {
            strcpy(argv[4] , "SHARED");
        }
        else if (!strcmp(argv[4] , "wepauto"))
        {
            strcpy(argv[4] , "WEPAUTO");
        }
        else if (!strcmp(argv[4] , "wpa-psk"))
        {
            strcpy(argv[4] , "WPAPSK");
        }
        else if (!strcmp(argv[4] , "wpa2-psk"))
        {
            strcpy(argv[4] , "WPA2PSK");
        }
        else if (!strcmp(argv[4] , "mixed-psk"))
        {
            strcpy(argv[4] , "WPAPSKWPA2PSK");
        }
        else if (!strcmp(argv[4] , "wpawpa2"))
        {        
            strcpy(argv[4] , "WPA1WPA2");
        }
        else if ((!strcmp(argv[4] , "11i")) || (!strcmp(argv[4] , "basic")) || (!strcmp(argv[4] , "wpa")) )                
        {   
            printf("wlan_secmode [%s] not support yet!\n", argv[4]);
            return SUCCE;
        }
    }
    
    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd secmode set: %s\n", argv[2]);        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "AuthMode", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd secmode set: %s\n", argv[4]);        
            memset(ucInterface, 0x00, WLAN_8_LEN);
            if (strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
            }  
            iwpriv_set(socket_id, ucInterface, "AuthMode", argv[4]);            
        }
    }
      
    close(socket_id);
    
    return SUCCE;    
}

/******************************************************************************
  函数名称  : wlan_wepkey
  功能描述  : 配置/显示某个SSID的WEP密钥
              命令格式: "Wlancmd set wepkey -i 0 -k 1 abcde"
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_wepkey(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN], ucKeyNumber[WLAN_8_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWepkey!\n");    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0))|| (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if((strcmp(argv[2], "-k") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "Wep Key number: %s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepkey set: %s\n", argv[4]);    
            memset(ucKeyNumber, 0x00, WLAN_8_LEN);
            sprintf(ucKeyNumber, "Key%s", argv[3]);
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, ucKeyNumber, argv[4]);                
        }
        else
        {
            close(socket_id);    
            return FAIL;
        }        
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 8))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);                    

            memset(ucInterface, 0x00, WLAN_8_LEN);
            if(strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
            }  
            if((strcmp(argv[4], "-k") == 0))
            {
                WLAN_DBG(RT_DEBUG_TRACE, "Wep Key number: %s\n", argv[5]);                    
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepkey set: %s\n", argv[6]);    
                memset(ucKeyNumber, 0x00, WLAN_8_LEN);
                sprintf(ucKeyNumber, "Key%s", argv[5]);        
                iwpriv_set(socket_id, ucInterface, ucKeyNumber, argv[6]);                        
            }
            else
            {
                close(socket_id);    
                return FAIL;                
            }
        }
    }
    
    close(socket_id);    
    return SUCCE;            
}


/******************************************************************************
  函数名称  : wlan_wepkeyindex
  功能描述  : 配置/显示某个SSID的WEP密钥序号
              命令格式:""wlancmd wepkeyindex set -i %d %d""
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_wepkeyindex(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWepkeyindex!\n"); 
    
    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepkeyindex set: %s\n", argv[2]);        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "DefaultKeyID", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wepkeyindex set: %s\n", argv[4]);        //wlancmd wepkeyindex set

            memset(ucInterface, 0x00, WLAN_8_LEN);
            if (strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);          
            }                   
            iwpriv_set(socket_id, ucInterface, "DefaultKeyID", argv[4]);            
        }
    }
    
    close(socket_id);    

    return SUCCE;    
}


/******************************************************************************
  函数名称  : wlan_pskkey
  功能描述  : 配置/显示某个SSID的WPA-PSK的密钥
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_pskkey(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    char acCmd[WLAN_64_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdPskkey!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
        
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd pskkey set: %s\n", argv[2]);            //wlancmd pskkey set [-i SsidIndex] {Keyvalue}
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WPAPSK", argv[2]);
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "DefaultKeyID", "2");
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd pskkey set: %s\n", argv[4]);        
            
            memset(ucInterface, 0x00, WLAN_8_LEN);
            if (strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);            
            }            
            iwpriv_set(socket_id, ucInterface, "WPAPSK", argv[4]);
            iwpriv_set(socket_id, ucInterface, "DefaultKeyID", "2");
        }
    }

    close(socket_id);    
    
    return SUCCE;    
}


/******************************************************************************
  函数名称  : wlan_wpaencryption
  功能描述  : 配置/显示某个SSID的WPA/WPA2加密方式
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_wpaencryption(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWpaencryption!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    
    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        if (strcmp(argv[2], "tkip+aes") == 0)
        {
            strcpy(argv[2] , "TKIPAES");
        }
        else if (strcmp(argv[2], "tkip") == 0)
        {
            strcpy(argv[2] , "TKIP");
        }
        else if (strcmp(argv[2], "aes") == 0)
        {
            strcpy(argv[2] , "AES");
        }
        else
        {
            printf("\r\n parse error   \r\n");
            return FAIL;
        }
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wpaencryption set: %s\n", argv[2]);        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "EncrypType", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-i") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: wl%s\n", argv[3]);    
            if (strcmp(argv[4], "tkip+aes") == 0)
            {
                strcpy(argv[4] , "TKIPAES");
            }
            else if (strcmp(argv[4], "tkip") == 0)
            {
                strcpy(argv[4] , "TKIP");
            }
            else if (strcmp(argv[4], "aes") == 0)
            {
                strcpy(argv[4] , "AES");
            }
            else
            {
                printf("\r\n parse error   \r\n");
                return FAIL;
            }
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd wpaencryption set: %s\n", argv[4]);    //wlancmd wpaencryption set -i 0 aes        
            memset(ucInterface, 0x00, WLAN_8_LEN);
            if (strcmp(argv[3], "0") == 0)
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_PREFIX, argv[3]);
            }
            else
            {
                sprintf(ucInterface, "%s%s", INIC_INFNAME_OTHER_PREFIX, argv[3]);            
            }
            iwpriv_set(socket_id, ucInterface, "EncrypType", argv[4]);            
        }
    }

    close(socket_id);  
    
    return SUCCE;        
}

/******************************************************************************
  函数名称  : wlan_wlanqos
  功能描述  : 配置/显示WLAN QoS
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-8-25
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_wlanqos(void *wl, char **argv)
{
    return 0;
}

/*******************************Begin to add for V100R001***********************************************
  函数名称  : wlan_fltmacctl
  功能描述  : 配置/显示WLAN MAC地址过滤开启
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-11-9
     作    者   : x00104602
     修改内容   : 完成初稿

******************************************************************************/
int wlan_fltmacctl(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFltmacctl!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmacctl set: %s\n", argv[2]);    

        if (!strcmp(argv[2],"enable"))        //Allow
        {
            strcpy(argv[2] , "1");
        }
        else if (!strcmp(argv[2],"disable"))    //disable
        {
            strcpy(argv[2] , "0");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "AccessPolicy", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmac set: %s\n", argv[4]);        

            memset(ucInterface, 0x00, WLAN_8_LEN);
            strcpy(ucInterface, argv[3]);    
            if(!strcmp(argv[4],"enable"))        //Allow
            {
                strcpy(argv[4] , "1");
            }
            else if(!strcmp(argv[4],"disable"))    //disable
            {
                strcpy(argv[4] , "0");
            }
            iwpriv_set(socket_id, ucInterface, "AccessPolicy", argv[4]);    
        }
    }

    close(socket_id);
    
    return SUCCE;
}


/*******************************************************************************************************
  函数名称  : wlan_fltmacctl
  功能描述  : 配置/显示WLAN MAC地址过滤启用黑/白名单
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-11-9
     作    者   : x00104602
     修改内容   : 完成初稿

******************************************************************************/
int wlan_fltmacmode(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFltmacmode!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0)) || (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmacctl set: %s\n", argv[2]);    
        if(!strcmp(argv[2],"allow"))        //Allow
        {
            strcpy(argv[2] , "2");
        }
        if(!strcmp(argv[2],"deny"))            //deny
        {
            strcpy(argv[2] , "1");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "AccessPolicy", argv[2]);    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmac set: %s\n", argv[4]);        

            memset(ucInterface, 0x00, WLAN_8_LEN);
            strcpy(ucInterface, argv[3]);    
            if (!strcmp(argv[4],"allow"))        //Allow
            {
                strcpy(argv[4] , "1");
            }
            else if (!strcmp(argv[4],"deny"))            //deny
            {
                strcpy(argv[4] , "2");
            }
            iwpriv_set(socket_id, ucInterface, "AccessPolicy", argv[4]);    
        }    

        close(socket_id);    
        return SUCCE;
    }
}


/*******************************************************************************************************
  函数名称  : wlan_fltmacctl
  功能描述  : 配置/显示WLAN MAC地址过滤启用黑/白名单
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-11-9
     作    者   : x00104602
     修改内容   : 完成初稿

******************************************************************************/
int wlan_fltmac(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_32_LEN];
    char MACAddressList[WLAN_SYS_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFltmac!\n");    

    if (((argc == 3) && (strcmp(argv[1], "set") == 0))|| (argc < 3))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
 
    if ((strcmp(argv[1], "set") == 0) && (argc == 4))
    {
        if (strcmp(argv[2], "clearmac") == 0)
        {
            memset(ucInterface, 0x00, WLAN_32_LEN);
            strcpy(ucInterface, INIC_INFNAME_MAIN);
            iwpriv_set(socket_id, ucInterface, "ACLClearAll", "1");
        }
        else
        {
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmac set: %s\n", argv[2]);    
            memset(MACAddressList, 0x00, WLAN_SYS_LEN);
            sprintf(MACAddressList,"%s", argv[2]);
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "ACLAddEntry", MACAddressList);
        }    
    }
    else if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {
            if(strcmp(argv[4], "clearmac") == 0)
            {
                memset(ucInterface, 0x00, WLAN_32_LEN);
                strcpy(ucInterface, argv[3]);
                iwpriv_set(socket_id, ucInterface, "ACLClearAll", "1");
            }
            else
            {
                WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);                    
                WLAN_DBG(RT_DEBUG_TRACE, "wlancmd fltmac set: %s\n", argv[4]);        

                memset(ucInterface, 0x00, WLAN_32_LEN);
                memset(MACAddressList, 0x00, WLAN_SYS_LEN);
                strcpy(ucInterface, argv[3]);
                sprintf(MACAddressList,"%s", argv[4]);        
                iwpriv_set(socket_id, ucInterface, "ACLAddEntry", MACAddressList);    
            }                
        }    
    }

    close(socket_id);
    
    return SUCCE;    
}

/*******************************************************************************************************
  函数名称  : wlan_assocctl
  功能描述  : 配置/显示SSID接入设备数
  输入参数  : 
              1.  *wl:
              2.  *cmd:
              3.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-11-9
     作    者   : x00104602
     修改内容   : 完成初稿

******************************************************************************/
int wlan_assocctl(void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[WLAN_8_LEN];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdApisolate!\n");
    
    if (((argc != 6) && (strcmp(argv[1], "set") != 0)) || (argc < 6))
    {
        return FAIL;
    }  
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd assocctl set: %s\n", argv[4]);        

            memset(ucInterface, 0x00, WLAN_8_LEN);
            strcpy(ucInterface, argv[3]);
            iwpriv_set(socket_id, ucInterface, "MaxStaNum", argv[4]);                    
        }    
    }
    else
    {
        return FAIL;        
    }    
    
    close(socket_id);
    
    return SUCCE;
}

/************************************************************************************

**************************************************************************************/
#ifdef SUPPORT_MIMO

int wlan_HT_GI(void *wl, char **argv)
{
    int socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- Wlan_HT_GI!\n");    
    if (argc < 3)
    {
        return FAIL;
    }
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {    
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd HT_GI Start: %s\n", argv[2]);    

        //wlancmd gprtctrl {value}
        if(!strcmp(argv[2], "short"))
        {
            strcpy(argv[2], "1");
        }
        else if (!strcmp(argv[2], "long"))
        {
            strcpy(argv[2], "0");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtGi", argv[2]);    
    }

    close(socket_id);
    
    return SUCCE;  
}


/************************************************************************************

**************************************************************************************/

int wlan_HT_AutoBA(void *wl, char **argv)
{
    int socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- Wlan_HT_AutoBA!\n");    
    if (argc < 3)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {             
        if(!strcmp(argv[2], "disable"))
        {
            strcpy(argv[2], "0");
        }
        else if (!strcmp(argv[2], "enable"))
        {
            strcpy(argv[2], "1");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtAutoBa", argv[2]);    
    }

    close(socket_id);    
    return SUCCE;    
}

/************************************************************************************
Value: 0: Below 1: Above
**************************************************************************************/
int wlan_HtExtcha(void *wl, char **argv)
{
    int socket_id;
    int argc = wlancmdGetNum(argv);
    
    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- wlan_HtExtcha!\n"); 
    
    if (argc < 3)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {    
        if(!strcmp(argv[2], "Below"))
        {
            strcpy(argv[2], "0");
        }
        else if (!strcmp(argv[2], "Above"))
        {
            strcpy(argv[2], "1");
        }
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtExtcha", argv[2]);    
    }
    
    close(socket_id);    
    return SUCCE;    
}

/************************************************************************************
**************************************************************************************/

int wlan_HT_BW(void *wl, char **argv)
{
    int socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- Wlan_HT_BW!\n");    
    if (argc < 3)
    {
        return FAIL;
    }
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {    
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd HT_BW Start: %s\n", argv[2]);    
         
        if (!strcmp(argv[2], "20"))
        {
            strcpy(argv[2], "0");
        }
        else if (!strcmp(argv[2], "20/40"))
        {
            strcpy(argv[2], "1");
        }
        
        //wlancmd gprtctrl {value}
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "HtBw", argv[2]);    
    }

    close(socket_id);    
    return SUCCE;    
}

int wlan_HT_RDG(void *wl, char **argv)
{
    if ((strcmp(argv[0], "HT_RDG") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1])!=SUCCE) 
    {
        return FAIL;
    }
    
    return SUCCE;    
}

int wlan_HT_BADecline(void *wl, char **argv)
{
    if ((strcmp(argv[0], "HT_BADecline") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1])!=SUCCE) 
    {
        return FAIL;
    }
    
    return SUCCE;    
}

int wlan_TxBurst(void *wl, char **argv)
{
    if ((strcmp(argv[0], "TxBurst") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1])!=SUCCE) 
    {
        return FAIL;
    }
    
    return SUCCE;    
}

#endif
/*******************************************end of add******************************/
/******************************************************************************
  函数名称  : wlan_cnxt
  功能描述  : CNXT需要设置的命令
  输入参数  : 
              1.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2007-10-27
     作    者   : l69021
     修改内容   : 完成初稿

******************************************************************************/
int wlan_cnxt(void *wl, char **argv)
{
    int val;
    int arr[2];
    char *endptr;
    int cmdget = -1;
    int cmdset = -1;

    int argc = wlancmdGetNum(argv);
    
    /* 解析命令关键字 */   
    if (!strcmp(*argv, "gprtctrl"))
    {
        int socket_id;

        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- Wlancmdgprtctrl!\n");
        if (argc < 3)
        {
            return FAIL;
        }
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
        if (!(strcmp(argv[0], "gprtctrl")) && (argc == 3))
        {
            if(!strcmp(argv[1], "0"))
            {
                strcpy(argv[1], "2");
            }
            else if (!strcmp(argv[1], "1"))
            {
            
            }
            else if(!strcmp(argv[1], "2"))
            {
                strcpy(argv[1], "0");
            }
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BGProtection", argv[1]);    
        }

        close(socket_id);
        
        return SUCCE;    
    }
    else if (!strcmp(*argv, "setplcphdr"))
    {
        int socket_id;

        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlancmdSetplcphdr!\n");    
        if (argc < 3)
        {
            return FAIL;
        }
        
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
        if (!(strcmp(argv[0], "gprtctrl")) && (argc == 3))
        {
            if(!strcmp(argv[1], "long"))
            {
                strcpy(argv[1], "0");
            }
            else if (!strcmp(argv[1], "debug"))
            {
                strcpy(argv[1], "1");
            }
            else if(!strcmp(argv[1], "auto"))
            {
                strcpy(argv[1], "1");
            }
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "ShortSlot", argv[1]);    
        }
        close(socket_id);
        
        return SUCCE;    
   }
    else if (!strcmp(*argv, "rtsthresh"))
    {
        int socket_id;
            
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdRtsThresh!\n");

        if ((argc < 3)|| ((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd rtsthresh:\n\tRTSThreshold {1~2347}::Set RTS Threshold\n");    
            return FAIL;    
        }
        
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

        //wlancmd rtsthresh [value]   
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "RTSThreshold", argv[1]);       
                
        close(socket_id);    

        return SUCCE;    
    }
    else if (!strcmp(*argv, "fragthresh"))
    {
        int             socket_id;
            
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFragThresh!\n");    
        if ((argc < 3) || ((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd fragthresh:\n"
            "\tFragThreshold {256~2346}::Set Fragment Threshold\n");
            return FAIL;    
        }
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "FragThreshold", argv[1]);        //wlancmd fragthresh [value]

        close(socket_id);
        
        return SUCCE;    
    }
    else if (!strcmp(*argv, "dtim"))
    {
        int socket_id;
        
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdDtim!\n");    
        if((argc < 3) || ((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd dtim:\n"
            "\tDtimPeriod {1~255}::Set DtimPeriod\n");    
            return FAIL;            
        }
        
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "DtimPeriod", argv[1]);            //wlancmd dtim [value]

        close(socket_id);
        return SUCCE;    
    }
    else if (!strcmp(*argv, "beaconinterval"))
    {
        int socket_id;
            
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdBeaconInterval!\n");    
        if ((argc < 3) || ((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd WlanCmdBeaconInterval:\n"
            "\tBeaconPeriod {20~1023}::Set BeaconPeriod\n");    
            return FAIL;            
        }
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
        
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "BeaconPeriod", argv[1]);        //wlancmd beaconinterval [value]

        close(socket_id);    
        return SUCCE;        
    }
    else if (!strcmp(*argv, "frameburst"))
    {
        int socket_id;
            
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdFrameBurst!\n");    
        if((argc < 3) || ((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd frameburst:\n\tTxBurst\t0: Disable\n\t\t1: Enable\n");
            return FAIL;            
        }
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);
        //wlancmd frameburst [value]    
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "TxBurst", argv[1]);    
       
        close(socket_id);    
        return SUCCE;    
    }
    else if (!strcmp(*argv, "setwme"))
    {
        int             socket_id;
            
        WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlancmdSetwme!\n");    
        if ((argc < 3) ||((argc == 3) && ((argv[1][0]>0x3A)||(argv[1][0]<0x30))))
        {
            printf("wlancmd Setwme: set WMM\n");    
            return FAIL;            
        }
        
        socket_id = socket(AF_INET, SOCK_DGRAM, 0);                //wlancmd Apisolate [value]
        if (!strcmp(argv[1], "-1"))
        {
            printf("not support WMM AUTO");    
            close(socket_id);    
            return FAIL;
        }
        else
        {
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WmmCapable", argv[1]);
        }    
           
        close(socket_id);
        
        return SUCCE;
    }
    else if (!strcmp(*argv, "setwmestr"))
    {
        return SUCCE;
    }    
    else if (!strcmp(*argv, "reg_domain"))
    {
        return SUCCE;
    }
    
    return FAIL;
}

int wlan_bss(void *wl, char **argv)
{
    int   socket_id;
    char  cmd[15];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdBss!\n");    

    if (argc < 6)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if (!(strcmp(argv[1], "set")) && (argc == 6))
    {
        if (!(strcmp(argv[2], "-i")))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd hide set: %s\n", argv[4]);
            memset(cmd, 0x00, 15);
            sprintf(cmd, "ifconfig wl%s %s", argv[3], argv[4]);        
            wlan_exec_cmd(cmd);                    
        }
    }
    else
    {
        return FAIL;        
    }

    close(socket_id);
    
    return SUCCE;
}


#ifdef SUPPORT_WPS
int wlan_wpsnvram(void *wl, char **argv)
{
    return SUCCE;
}
int wlan_wpsctl(void *wl, char **argv)
{
    int  socket_id;
    char cmd[WLAN_64_LEN];
    int  argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWpswscStart!\n");

    if (argc < 4)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd Wpsctl set: %s\n", argv[2]);  
       /* wlancmd Wpsmode set {ap-pin/ap-pbc} */

        if (!strcmp(argv[2], "enabled"))
        {
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscConfMode", "7");
            /* 2 configured mode, all mode support wps */
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscConfStatus", "2");
        }
        if (!strcmp(argv[2], "disabled"))
        {
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscConfMode", "0");
        }        
    }

    close(socket_id);
    
    return SUCCE;
}
int wlan_wpsmode(void *wl, char **argv)
{
    int socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWpsmode!\n");    
    if (argc < 4)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if (!(strcmp(argv[1], "set")) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd Wpsmode set: %s\n", argv[2]); 
        /* wlancmd Wpsmode set {ap-pin/ap-pbc} */
        if (!strcmp(argv[2], "client-pin"))
        {
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscMode", "1");    
        }
        
        if (!strcmp(argv[2], "client-pbc"))
        {
            iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscMode", "2");
        }
    }
      
    close(socket_id);
    return SUCCE;
}

int wlan_wpspinval(void *wl, char **argv)
{
    int   socket_id;
    char  ucInterface[15];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdWpspinval!\n");  
    
    if(argc < 4)
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if(!(strcmp(argv[1], "set")) && (argc == 4))
    {
        WLAN_DBG(RT_DEBUG_TRACE, "wlancmd Wpspinval set: %s\n", argv[2]);
        /* wlancmd Wpspinval set {value} */
        iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscPinCode", argv[2]);        
    }

    close(socket_id);
    
    return SUCCE;
}



int wlan_wpswsc(void *wl, char **argv)
{
    int     socket_id;

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if(-1 == socket_id)
    {
        printf("\nwlan_wpswsc:create socket fail!\n");
        return FAIL;
    }

    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscGetConf", "1");

    close(socket_id);    

    return SUCCE;
}


int wlan_wpspara(void *wl, char **argv)
{
    return SUCCE;    
}

int wlan_wpsgetstat(void *wl, char **argv)
{
	int socket_id;
	struct iwreq wrq;
	int data = 0;
    
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(wrq.ifr_name, INIC_INFNAME_MAIN);    
	wrq.u.data.length  = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags   = RT_OID_WSC_QUERY_STATUS;
	if ( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
    {
		perror("ioctl error");
		close(socket_id);
		return FAIL;
	}
    
    /* 输出格式不应随意改动，WLANCMD命令会对该输出进行解析 */
    printf("wsc_stat=%d\n", data);
    
	close(socket_id);
    
    return SUCCE;    
}
/*start of change wps minpengwei*/
int wps_set_confstat(void *wl, char **argv)
{
    int     socket_id;

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    
    if(-1 == socket_id)
    {
        printf("\nwlan_wpswsc:create socket fail!\n");
        return FAIL;
    }
    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscConfMode", "7");
    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscMode", "2");
    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "WscConfStatus", "2");            

    close(socket_id);    

    return SUCCE;
}
/*end of change wps minpengwei*/

char *strcat_r(const char *s1, const char *s2, char *buf)
{  
   strcpy(buf, s1);
   strcat(buf, s2);
   
   return buf;
}

int setVarToNvram(void *var, const char *name, const char *type)
{  
    char temp_s[100] = {0};
    int ret=0;
 #ifdef SUPPORT_NVRAM        
    if(!strcmp(type,"int"))
    {    
        sprintf(temp_s,"%d",*(int*)var);            
        ret = nvram_set(name, temp_s);
        return ret;
    } 
    else if(!strcmp(type,"string") && (!strcmp((char*)var,""))) 
    {
        ret = nvram_unset(name);
        return ret;
    } 
    else if(!strcmp(type,"string")) 
    {
        ret = nvram_set(name, (char*)var);
        return ret;
    }
    else 
    {
        printf("setVarToNvram:type not found\n");
        return -1;
    }   
 #endif
    return ret; 
}
#endif

/******************************************************************************
  函数名称  : wlan_MBSS
  功能描述  :打开关闭多SSID功能
  输入参数  : 
              1.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-8-18
     作    者   : p00134244
     修改内容   : 完成初稿

******************************************************************************/
#ifdef SUPPORT_MBSS
int wlan_mbss(void *wl, char **argv)
{
    return 0;
}
#endif

static int
wlan_iovar_set(void *wl, const char *iovar, void *param, int paramlen)
{
    char smbuf[256];

    memset(smbuf, 0, sizeof(smbuf));

    return wlan_iovar_setbuf(wl, iovar, param, paramlen, smbuf, sizeof(smbuf));
}

/******************************************************************************
  函数名称  : wlan_curetheraddr
  功能描述  : 设置多SSID的mac地址
  输入参数  : 
              1.  **argv
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-8-19
     作    者   : p00134244
     修改内容   : 完成初稿

******************************************************************************/
#ifdef SUPPORT_MBSS
int  wlan_curetheraddr(void *wl, char **argv)
{
    return 0;
}

/******************************************************************************
  函数名称  : wlan_isolate
  功能描述  : 配置/显示ssid 隔离
  输入参数  : 
              1.  **argv:
  调用函数  : 
  被调函数  : 
  输出参数  : 无
  返 回 值  : 无

  修改历史      :
   1.日    期   : 2008-8-22
     作    者   : p00134244
     修改内容   : 完成初稿

******************************************************************************/
int wlan_isolate (void *wl, char **argv)
{
    int  socket_id;
    char ucInterface[5];
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanCmdApisolate!\n");
    
    if (((argc != 6) && (strcmp(argv[1], "set") != 0)) || (argc < 6))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    if ((strcmp(argv[1], "set") == 0) && (argc == 6))
    {
        if ((strcmp(argv[2], "-a") == 0))
        {
            WLAN_DBG(RT_DEBUG_TRACE, "interface number: %s\n", argv[3]);                    
            WLAN_DBG(RT_DEBUG_TRACE, "wlancmd isolate set: %s\n", argv[4]);        
            memset(ucInterface, 0x00, 5);
            strcpy(ucInterface, argv[3]);
            iwpriv_set(socket_id, ucInterface, "NoForwarding", argv[4]);                    
        }    
    }
    else
    {    
        close(socket_id);
        return FAIL;        
    }    
  
    close(socket_id);
    
    return SUCCE;    
}
#endif

int wlan_Qosctl(void *wl, char **argv)
{
    int argc = wlancmdGetNum(argv);

    /* wlancmd  <Qosctl set -i %d enable>  argc=6, argv[0]=Qosctl */   
    if ((argc != 6) || (strcmp(argv[0], "Qosctl") != 0))
    {
        return FAIL;
    }
    
    if ( strcmp(argv[4], "enable") == 0)
    {
        if (WriteParamToFile("WmmCapable", "1") != 0) 
        {
            return FAIL;
        }    
    }
    else if ( strcmp(argv[4], "disable") == 0)
    {
        if (WriteParamToFile("WmmCapable", "0") != 0) 
        {
            return FAIL;
        }
    }
    else
    {
        return FAIL;    
    }

    return SUCCE;
}

int wlan_Stactl(void *wl, char **argv)
{
    return 0;
}

int wl_get_dev_type(char *name, void *buf, int len)
{
    if (!strncmp(name, INIC_INFNAME_PREFIX, len))
    {
        strcpy(buf, INIC_INFNAME_PREFIX);
        return 0;
    }
    
    return -1;
}

int wl_check(void *wl)
{
    return 0;
}

int wlan_radar(void *wl, char **argv)
{
    return 0;
}

int wlan_spect(void *wl, char **argv)
{
    return 0;
}

int wlan_regulatory(void *wl, char **argv)
{
    return 0;
}

int wlan_maxassoc (void *wl, char **argv)
{
    return 0;
}

int wlanoption(char ***pargv, char **pifname, int *phelp)
{
    return 0;
}

/*
 * Set the WLAN survey site function, WDS need this.
 */
int wlan_site_survey(void *wl, char **argv)
{
    int  socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- wlan_site_survey!\n");

    /* wlancmd site_survey set 0/1 */
    if ((argc != 4) || (strcmp(argv[1], "set") != 0))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "SiteSurvey", argv[2]);   

    close(socket_id);
 
    return SUCCE;    
}

/**************************************************************
wlancmd WdsEnable x

0: Disable - Disable all WDS function.

1: Restrict mode - Same as Repeater mode.

2: Bridge mode - Turn on WDS function, the peer WDS APs are according to the 
mac address listed in "WdsList" field below. In this mode, AP will not send 
beacon out and will not deal with probe request packets, therefore STA will
not possible to connect with it.

3: Repeater mode - Turn on WDS function, the peer WDS APs are according to the  
mac address listed in "WdsList" field below.

4: Lazy mode - Turn on WDS function, and auto learning from WDS packet which 
with addr4 field.
**************************************************************/
int WlanCmdWdsEnable(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsEnable") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWdsList(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsList") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWdsEncrypType(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsEncrypType") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0)
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWdsKey(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsKey") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1])==0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWds0Key(void *wl, char **argv)
{
    if ((strcmp(argv[0], "Wds0Key") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) !=0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWds1Key(void *wl, char **argv)
{
    if ((strcmp(argv[0], "Wds1Key") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWds2Key(void *wl, char **argv)
{
    if ((strcmp(argv[0], "Wds2Key") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWds3Key(void *wl, char **argv)
{
    if ((strcmp(argv[0], "Wds3Key") != 0))
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWdsDefaultKeyID(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsDefaultKeyID") != 0))
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanWdsPhyMode(void *wl, char **argv)
{
    if ((strcmp(argv[0], "WdsPhyMode") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}


/* WLAN 802.1X和WDS必须要写文件，否则不生效!!! */
int WlanIEEE8021X(void *wl, char **argv)
{    
    int socket_id;
    int argc = wlancmdGetNum(argv);

    WLAN_DBG(RT_DEBUG_TRACE, "\nUtility --- WlanIEEE8021X!\n");

    /* wlancmd IEEE8021X set 0/1 */
    if ((argc != 4) || (strcmp(argv[1], "set") != 0))
    {
        return FAIL;
    }
    
    socket_id = socket(AF_INET, SOCK_DGRAM, 0);    

    iwpriv_set(socket_id, INIC_INFNAME_MAIN, "IEEE8021X", argv[2]);   

    close(socket_id);
 
    return SUCCE;    

}


int WlanEAPifname(void *wl, char **argv)
{
    if ((strcmp(argv[0], "EAPifname") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanPreAuthifname(void *wl, char **argv)
{
    if ((strcmp(argv[0], "PreAuthifname") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0)
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanRADIUS_Server(void *wl, char **argv)
{
    if ((strcmp(argv[0], "RADIUS_Server") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanRADIUS_Port(void *wl, char **argv)
{
    if ((strcmp(argv[0], "RADIUS_Port") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int WlanRADIUS_Key(void *wl, char **argv)
{
    if ((strcmp(argv[0], "RADIUS_Key") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int Wlanown_ip_addr(void *wl, char **argv)
{
    if ((strcmp(argv[0], "own_ip_addr") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1]) != 0) 
    {
        return FAIL;
    }

    return SUCCE;
}

int Wlansession_timeout_interval(void *wl, char **argv)
{
    if ((strcmp(argv[0], "RadiusSessionTimeout") != 0)) 
    {
        return FAIL;
    }

    if (WriteParamToFile(argv[0], argv[1])!=0)
    {
        return FAIL;
    }

    return SUCCE;
}

