#ifndef _FUNCTION_H_
#define _FUNCTION_H_

//**************************************************************//
typedef int (*LPF_DO_FUNC)(int, char**, int);
typedef struct 
{
    char*           cCtlCmd;
    LPF_DO_FUNC     lpfCtlCmdFunc;
} TWlanCmd;

typedef struct 
{
    char*           cWirelessMode;
    int             iWirelessModeNumber;
} TcWlanMode;

typedef struct 
{
    char*           cCountryString;
    char            iRegionChannel;
} TcWlanCountry;

typedef struct 
{
    char*           cChannelRate;
    int             iRateCount;
} TcWlanChannelMode;

#define RT_DEBUG_OFF        0
#define RT_DEBUG_ERROR      1
#define RT_DEBUG_WARN       2
#define RT_DEBUG_TRACE      3
#define DBG                 1

#define WLAN_CFG_FILE_PATCH    "/var/wlan/RT3062AP.dat"
#define WLAN_CFG_FILE_SIZE     4096
#define WLAN_LINE_BUF_LEN      80

#endif //_FUNCTION_H_
