/***********************************************************************
  版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名: dump.c
  作者: y45539
  版本: V500R003
  创建日期: 2005-8-16
  完成日期: 2005-8-16
  功能描述: 打印调试信息到串口
      
  主要函数列表: 
    void show_usuage()
    打印diagd启动方法
    int diag_parse_lines(int argc, char **argv)
    解析命令行传入参数
    int send_to_net(char* acbuf, int irLen, int arg)
    发送数据到网口
    int echo_to_net(char* acbuf, int iLen, int arg)
    打开MODEM,_PCUI口时，打印提示符和回车符
修改历史记录列表: 
<作  者>    <修改时间>  <版本>  <修改描述>
    
  备注: 
************************************************************************/
#include "dump.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void info (char *fmt, ...)
{
    char buf[1024];
    va_list pvar;

    /*记录日志文件的长度*/
    static long file_len=0;

#if defined(__STDC__)
    va_start(pvar, fmt);
#else
    char *fmt;
    va_start(pvar);
    fmt = va_arg(pvar, char *);
#endif
    vsprintf (buf, fmt, pvar);
    va_end(pvar);
    int err;
    err = write( g_logsys, buf, strlen(buf) );
    if (err < 0)
    {
       send_to_net("error logsys", sizeof ("error logsys"), 0);
    }
    else
    {
        file_len += sizeof(buf);
        if ((32*1024) <= file_len)
        {
            //回到文件开头重新写文件
            lseek( g_logsys, 0, SEEK_SET );
        }
    }
}

/*------------------------------------------------------------
  函数原型: void show_usuage()
  描述: 打印diagd程序的使用方法
  输入: 无
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void show_usuage( void )
{
     printf("%s Usage: \n %s\n%s\n%s\n%s\n", "diagd function",
                 "      diagd -d DevPortNum",
                 "      diagd -p NetPortNum",
                 "      Notice! NetPortNum 1024~65535");
}

/*------------------------------------------------------------
  函数原型: int parse_lines(argc, argv)
  描述: 解析命令行参数，如果打开的端口是AT和PCUI端口，则对应_AT,_PCUI枚举变量
        主程序相应地透传AT命令，否则透传DIAG口数据
  输入: int argc    参数个数
        char **argv 参数数组
  输出:  无
  返回值:端口枚举值 
         _MODEM   MODEMAT口
         _DIAG DIAG口
         _PCUI PCUI口
         _VOICE PCSC口
-------------------------------------------------------------*/
int diag_parse_lines (int argc, char **argv)
{
#if 0

    if (argc <= 1)
    {
        g_ePort = _DIAG;
        return OPER_SUCCESS;
    }
    else if (argc > 3)
    {
        show_usuage();
        return OPER_FAILURE;
    }

    if (!strcmp(argv[1],"-modem"))
    {
      g_ePort = _MODEM;
    }
    else if (!strcmp(argv[1],"-diag"))
    {
      g_ePort = _DIAG;
    }
    else if (!strcmp(argv[1],"-pcui"))
    {
      g_ePort = _PCUI;
    }
     if (argc == 3)
     {
        char* ptr = strchr(argv[2], '=');
        int     i = 0;
        char  bFound = 0;

        while ( (NULL != ptr) && (*(ptr+i) != '\0' ))
        {
            if ( !isdigit( *(ptr+i) ) && i >= 5 )
            {
                bFound = -1;
                break;
            }
            i++;
        }
            
        if( (NULL != ptr) && (bFound == 0) && (atoi( ptr ) > 1024) && (atoi( ptr ) < 65535))
        {
            g_Serverport = atoi( ptr );
        }
        else
        {
            return OPER_FAILURE;
        }
     }
#endif
    int opt;

    while ((opt = getopt(argc, argv, "d:p:")) > 0) {
		switch (opt) {
		case 'd':
			g_ePort = atoi(optarg);
			printf("g_ePort %d\r\n", g_ePort);
			break;
		case 'p':
			g_Serverport = atoi(optarg);
			printf("g_Serverport %d\r\n", g_Serverport);
			break;
		default:
			show_usuage();
			return OPER_FAILURE;
		}
    	}
 
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: int send_to_net(char* acbuf, int irLen, int arg)
  描述: 发送数据到网口，如果网口能够正发送数据就会循环发送，
               直至把从diag口接收的数据全部发送出去。
  输入: 
                char*    pbuf           发送缓冲
                int      rcv_len        发送缓冲长度
                int      arg            TCP传输时控制参数
  输     出 : 无
  返回值: 成功返回0 失败返回负数
-------------------------------------------------------------*/
int send_to_net(char* pbuf, int rcv_len, int arg)
{
    int   send_len = 0;
    char *pCur = pbuf;
    
    while( rcv_len != 0 )
    {
        send_len = send (g_netfd, pCur, rcv_len, arg);                
        
        if ( ( (send_len == SOCKET_ERROR) 
            && (errno != EINTR))
            ||(send_len == 0))
        {
            return OPER_FAILURE;
        }
        rcv_len -= send_len;
        pCur    += send_len;
    }
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: int echo_to_net(char* acNet_rcvbuf, int iNet_rlen, int arg)
  描述: 回显到虚拟串口
  输入: char* acNet_rcvbuf 发送缓冲
        int iNet_rlen     发送缓冲长度
        int arg           TCP传输时控制参数
  输出: 无
  返回值: 成功返回0 失败返回负数
-------------------------------------------------------------*/
int echo_to_net(char* acbuf, int iLen, int arg)
{
     int realLen = iLen;
     char echo_buf[BUFFER_LEN+10];
     strncpy(echo_buf, acbuf, iLen);
     static unsigned int alter = 0;

     if ((alter%2 == 1) && (acbuf[iLen-1] == _r || acbuf[iLen-1] == _n))
     {
         echo_buf [iLen-1] = _n;
         echo_buf [iLen] =   _r;
         echo_buf [iLen+1] = _start;
         realLen = iLen + 2;
     }
     alter++;     
     return send_to_net(echo_buf, realLen, arg);
}

