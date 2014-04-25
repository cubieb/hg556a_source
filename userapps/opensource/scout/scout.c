/* souct进程 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define SCOUT_INTERVAL 5												/* scout default route(have/don't have) per SCOUT_INTERVAL secs */
#define SCOUT_TIMES 3													/* if data ifc status is balance and total up SCOUT_TIMES times, and have no default
																			route, then add default route by ourselves */
#define WAN_DATAIFC_PATH "/proc/var/fyi/wan/%s/status"   				/* data ifc wan status */
#define DATAIFC_NAME_PATH "/var/fyi/sys/dataifc"         				/* data ifc name file */
#define DATAIFC_PROTOCOL_PATH "/proc/var/fyi/wan/%s/daemonstatus"    /* data ifc protocol(pppoe,pppou,dhcp) status file */
#define STATIC_DATAIFC_PATH "/var/fyi/sys/%s/staticdataifc"


#define FILE_OPEN_ERR -1
#define FILE_OPEN_OK 0
#define BCM_PPPOE_CLIENT_STATE_UP 4
#define DHCPC_STS_SUCCESS 1
#define IFC_STATUS_UP 6
#define IFC_TINY_LEN 16
#define IFC_MEDIUM_LEN 240
#define IFC_LARGE_LEN 264
#define SYS_CMD_LEN 512

int ReadFirstLine(char *file, char *data, int len) {
    int ret = FILE_OPEN_ERR;
    FILE* fs = NULL;
    
    if ( (access(file, 0)) != 0 )
       return ret;
       
    fs = fopen(file, "r");

    if ( fs != NULL ) {
        fgets(data, len, fs);
        fclose(fs);
        if ( data != NULL ) {
            int i = 0;
            // remove trailing space characters
            while ( data[i] != '\0' && isspace(data[i]) == 0 )
                i++;
            data[i] = '\0';
        }
        ret = FILE_OPEN_OK;
    }

    return ret;
}

int Scout_Read_Wan_Status(char * pszPath)
{
	int iStatus = 0;
	char szStatus[IFC_TINY_LEN];
	memset(szStatus, 0, sizeof(szStatus));

	if(NULL == pszPath)
	{
		return -1;
	}
	
	if(FILE_OPEN_OK != ReadFirstLine(pszPath, szStatus, IFC_TINY_LEN))
	{
		return -1;
	}
	
	if (szStatus[0] != '\0' )
	{
    	iStatus = atoi(szStatus);
		return iStatus;
	}

	return -1;
}

int  IsHaveDefaultRoute()
{
    char szSaveFile[] = "/var/iproute";
    char szBuf[SYS_CMD_LEN];
    FILE * fp = NULL;

    system("ip route > /var/iproute");
    fp = fopen(szSaveFile, "r");
    if (NULL == fp)
    {
        return 0;
    }
    while (fgets(szBuf, SYS_CMD_LEN, fp) > 0) 
    {
        if (NULL != strstr(szBuf, "default"))
        {
        	/* have default route, return 1 */
        	fclose(fp);
			return 1;
        }
    }
    fclose(fp);
	/* don't have default route, return 0 */
    return 0;
}

int AddDefaultRoute(char * pszWanIfcName)
{
	FILE   *fs = NULL;
    char  buf[IFC_LARGE_LEN];
    char  str[IFC_TINY_LEN];
    char  gtwy[IFC_TINY_LEN];
	if(NULL == pszWanIfcName)
		return -1;

    sprintf(buf, "/var/fyi/sys/%s/gateway", pszWanIfcName);
    fs = fopen(buf, "r");
    if ( fs != NULL ) 
	{
        if ( fgets(str, IFC_TINY_LEN, fs) != NULL ) 
		{
            strcpy(gtwy, str);
            sprintf(buf, "ip route add default  dev  %s via  %s", pszWanIfcName,  gtwy);
            system(buf);
        }
        fclose(fs);   
    }
	return 0;
}

int main()
{
	int index = 0;				/* 连续没有缺省路由的次数 */
	char szDataIfcName[IFC_TINY_LEN];
	char szDataIfcPath[IFC_MEDIUM_LEN];
	char szDataProtocolPath[IFC_MEDIUM_LEN];
	char szDataStaticPath[IFC_MEDIUM_LEN];
	int iStatus = 0;
	memset(szDataIfcName, 0, sizeof(szDataIfcName));
	memset(szDataIfcPath, 0, sizeof(szDataIfcPath));
	memset(szDataProtocolPath, 0, sizeof(szDataProtocolPath));
	memset(szDataStaticPath, 0, sizeof(szDataStaticPath));
	
	while(1)
	{
		/* 获取Data接口名 */
		if(FILE_OPEN_OK != ReadFirstLine(DATAIFC_NAME_PATH, szDataIfcName, sizeof(szDataIfcName)))
		{
			goto GO_TIME;
		}

		/* 获取Data接口状态的文件路径 */
		if(szDataIfcName[0] != '\0')
		{
			sprintf(szDataIfcPath, WAN_DATAIFC_PATH, szDataIfcName);
		}
		else
		{
			goto GO_TIME;
		}

		/* 获取Data接口状态值 */	
		iStatus = Scout_Read_Wan_Status(szDataIfcPath);
		if(iStatus != IFC_STATUS_UP)
		{
			goto GO_TIME;
		}
		
		/* 如果Data接口已up,则检查Data的协议状态(pppoe,pppou,dhcp)是否是UP的 */
		sprintf(szDataProtocolPath, DATAIFC_PROTOCOL_PATH, szDataIfcName);
		sprintf(szDataStaticPath, STATIC_DATAIFC_PATH, szDataIfcName);
		
		if(NULL != strstr(szDataIfcName, "ppp"))
		{
			iStatus = Scout_Read_Wan_Status(szDataProtocolPath);
			if(BCM_PPPOE_CLIENT_STATE_UP != iStatus)
			{
        		goto GO_TIME;
			}
		}
		/* 如果是dhcp方式，需要分成静态pvc和动态pvc两种情况 */
		else if(NULL != strstr(szDataIfcName, "nas"))
		{
			iStatus = Scout_Read_Wan_Status(szDataStaticPath);
			/* 动态pvc的情况 */
			if(-1 == iStatus)
			{
				iStatus = Scout_Read_Wan_Status(szDataProtocolPath);
				if(DHCPC_STS_SUCCESS != iStatus)
				{
					goto GO_TIME;
				}
			}
			/* 静态pvc，是down的情况 */
			else if(0 == iStatus)
			{
				goto GO_TIME;
			}
			/* 静态pvc，是up的情况 */
			else if(1 == iStatus)
			{
				
			}
		}

		/* 如果Data接口和协议都是up的，则查看缺省路由是否存在 */
		if(!IsHaveDefaultRoute())
		{
			/* 不存在，且已连续三次，则添加缺省路由 */
			if(index >= SCOUT_TIMES)
			{
				AddDefaultRoute(szDataIfcName);	
			}
			/* 不存在，且未达到三次，继续 */
			else
			{
				index ++;
				goto GO_SCOUT;
			}
		}
GO_TIME:
		index = 0;
GO_SCOUT:
		sleep(SCOUT_INTERVAL);
	}
	
	return 0;
}
