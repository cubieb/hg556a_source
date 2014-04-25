/****************************************************************************
  版权信息: 版权所有(C) 1988-2009, 华为技术有限公司.
  文件名:   udpechoserver.c
  作者:       
  版本:     HG556aV100R001
  创建日期: 2010-01-07
  完成日期: 
  功能描述: 
  修改记录:
*****************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <getopt.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define MAX_MSG_LEN        1024
#define IP_ADDR_LEN        16
#define BUFFER_LEN         256
#define UDP_HEADER_LEN     8
#define STATICS_FILE       "/var/udpechostatics"

typedef struct
{
    char szFirstRcvPktTime[BUFFER_LEN];
    char szLastRcvPktTime[BUFFER_LEN];
    int  iPktRcvNum;
    int  iBytesRcvNum;
    int  iPktRespNum;
    int  iBytesRespNum;    
}STATIC_INFO_ST, *PSTATIC_INFO_ST;

STATIC_INFO_ST stEchoStatics = {{0}};

void RecordUTCTime(PSTATIC_INFO_ST pstStaticInfo, int iFlag)
{    
    struct tm *pstTmTmp;
	struct timeval tv;
    char szTmp[BUFFER_LEN]     = {0};
	char szUTCTime[BUFFER_LEN] = {0};

	gettimeofday(&tv, NULL);    
	pstTmTmp = gmtime(&tv.tv_sec);
	strftime(szTmp, BUFFER_LEN, "%Y-%m-%dT%H:%M:%S" ,pstTmTmp);
    sprintf(szUTCTime, "%s.%d", szTmp, tv.tv_usec);

    if (iFlag)
    {
        printf("\nhaha, first packet recicved!\n\n");
        strcpy(pstStaticInfo->szFirstRcvPktTime, szUTCTime);
    }
    else
    {
        strcpy(pstStaticInfo->szLastRcvPktTime, szUTCTime);
    }
}

int WriteStaticInfoToVar(char *pFile, PSTATIC_INFO_ST pstStaticInfo)
{    
    FILE *fs = NULL;

    if ((NULL == pFile) || (NULL == pstStaticInfo))
    {
        return -1;
    }   
       
    fs = fopen(pFile, "w");

    if (fs != NULL)
    {
        fprintf(fs, "PacketsReceived=%d\n", pstStaticInfo->iPktRcvNum);
        fprintf(fs, "PacketsResponded=%d\n", pstStaticInfo->iPktRespNum);
        fprintf(fs, "BytesReceived=%d\n", pstStaticInfo->iBytesRcvNum);
        fprintf(fs, "BytesResponded=%d\n", pstStaticInfo->iBytesRespNum);
        fprintf(fs, "TimeFirstPacketReceived=%s\n", pstStaticInfo->szFirstRcvPktTime);
        fprintf(fs, "TimeLastPacketReceived=%s\n", pstStaticInfo->szLastRcvPktTime);
        fclose(fs);
        
        return 0;
    }
   
    return -1;
}

int main(int argc, char *argv[]) 
{
        
    int  iFirstRcv              = 0;    
    int  iPlusEnable            = 0;
    int  iOptionIndex           = 0;
    int  iLocalServerPort       = 0;
    char szIfc[IP_ADDR_LEN]     = {0};    
    char szSrcIP[IP_ADDR_LEN]   = {0};
    char szMessage[MAX_MSG_LEN] = {0};
    int  c, sd, rc, n, cliLen, flags, len;
    struct sockaddr_in cliAddr, servAddr;
    struct ifreq ifr;

    static struct option options[] = 
    {
        {"interface",	        required_argument,	0, 'i'},
        {"sourceipaddress", 	required_argument,	0, 's'},
        {"udpport",	            required_argument,	0, 'p'},
        {"udpechoplus",	        required_argument,	0, 'e'},
        {0, 0, 0, 0}
    };

	/* get options */
	while (1) 
    {        
		c = getopt_long(argc, argv, "i:s:p:e:", options, &iOptionIndex);
				
		if (c == -1)
		{
            break;
		}
		
		switch (c) 
        {
    		case 'p':
                iLocalServerPort = atoi(optarg);
    			break;		
    		case 's':
    			len = strlen(optarg) > 15 ? 15 : strlen(optarg);			
    			strncpy(szSrcIP, optarg, len);
    			break;
    		case 'i':
    			len = strlen(optarg) > 15 ? 15 : strlen(optarg);			
    			strncpy(szIfc, optarg, len);
    			break;
            case 'e':
    			iPlusEnable = atoi(optarg);
    			break;
		}
	}

    //printf("\nport %d srcip %s ifc %s\n", iLocalServerPort, szSrcIP, szIfc);

    /* socket creation */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sd < 0) 
    {
        printf("%s: cannot open socket \n",argv[0]);
        exit(1);
    }

    /* bind local server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(iLocalServerPort);

    if ('\0' != szIfc[0])
    {
        memset(&ifr, 0, sizeof(struct ifreq));
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_ifrn.ifrn_name, szIfc, IP_ADDR_LEN);
        
        if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&ifr, sizeof(ifr)) < 0)
        {
   		    printf("%s: cannot bind ifc \n",argv[0]);
            close(sd);
            exit(1);
   	    }
    }
    
    rc = bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    
    if (rc < 0) 
    {
        printf("%s: cannot bind port number %d \n", argv[0], iLocalServerPort);
        exit(1);
    }

    //printf("%s: waiting for data on port UDP %u\n", argv[0], iLocalServerPort);

    flags = 0;

    /* server infinite loop */
    while (1) 
    {
        WriteStaticInfoToVar(STATICS_FILE, &stEchoStatics);
        
        /* init buffer */
        memset(szMessage, 0x0, MAX_MSG_LEN);

        /* receive message */
        cliLen = sizeof(cliAddr);
        
        n = recvfrom(sd, szMessage, MAX_MSG_LEN, flags,(struct sockaddr *) &cliAddr, &cliLen);

        if (n < 0)
        {
            printf("%s: cannot receive data \n",argv[0]);
            continue;
        }

        if (0 == iFirstRcv)
        {
            iFirstRcv = 1;
            RecordUTCTime(&stEchoStatics, 1);
        }
        else
        {
            RecordUTCTime(&stEchoStatics, 0);
        }

        stEchoStatics.iPktRcvNum++;
        stEchoStatics.iBytesRcvNum += (n + UDP_HEADER_LEN);

        if ('\0' != szSrcIP[0])
        {
            if (htonl(cliAddr.sin_addr.s_addr) != htonl(inet_addr(szSrcIP)))
            {
                //printf("%s: cannot respone data, wrong srcip \n",argv[0]);
                continue;
            }
        }
          
        /* print received message */
        //printf("%s: from %s:UDP%u : %s \n", argv[0],inet_ntoa(cliAddr.sin_addr),
          //     ntohs(cliAddr.sin_port),szMessage);
        
        if (iPlusEnable)
        {
            //printf("\nplus function support!\n");
        }

        n = sendto(sd,szMessage,n,flags,(struct sockaddr *)&cliAddr,cliLen);

        if (n < 0) 
        {
            printf("%s: cannot send data \n",argv[0]);            
            continue;
        }

        stEchoStatics.iPktRespNum++;
        stEchoStatics.iBytesRespNum += (n + UDP_HEADER_LEN);
    } /* end of server infinite loop */

    return 0;
}
