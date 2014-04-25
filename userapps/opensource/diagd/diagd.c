/***********************************************************************
版权信息: 版权所有(C) 1988-2005, 华为技术有限公司.
文件名: Diagd.c
作者: l65457
版本: V100R001
创建日期: 2007-3-20
完成日期: 2007-3-27
功能描述: 实现PC 网口和DIAG口透传功能;MSM的强制升级；路由器模块的后台升级；
          多端口升级协商；虚拟串口数据透传。
  
主要函数列表:
    int diag_daemon( int nochdir, int noclose )
    实现后台程序
    
    static int diag_init_tcp_listen_socket( usockaddr* uSockaddr_tcp )
    实现创建tcp侦听套接字
    
    static int diag_udpinit_socket( )
    初始化UDP套接字
    
    void close_pcui_shake( )
    升级时关闭pppd等进程
    
    void net_to_diag()
    接收网口数据，把它输入到Diag口
    
    void diag_to_net ( )
    接收Diag口数据，把它输入到网口，这是一个子进程
    
    int open_usb_port()
    打开usb端口
    
    int diag_accept_connect (int listen_fd )
    接受对端tcp连接请求
    
    void tcp_timeout ( int sig )
    tcp连接定时器，超时设置g_disconnect=1
    
    static void main_coordinate ( int sig )
    主进程信号处理程序

    static void sub_coordinate ( int sig )
    子进程信号处理程序
    
    long analyse_cmd(unsigned char* buf,long *img_len)
    分析hdlc报文payload中命令，返回升级路由器、升级无线模块、强制升级、其他情况。

    static EN_UPGRADE_ERRCODE route_fwt_upgrade(char *part_img, long len, long *total)
    本函数负责将接受到tcp数据写入下层函数

    void udp_server_main(pid_t parent_pid)
    侦听udp的1280端口，处理virtual console和多端口协商报文

    nego_state parse_nego_cmd (unsigned char *buf, int len, unsigned char *report_buf)
    多端口升级协商中，分析升级协商命令内容。

    int ip_change(char * Reportbuf)
    更改IP
    
    int report_to_pc(int sock_fd, char* report_buf, struct sockaddr_in* pClientSock)
    使用hdlc报文，把执行结果报告给PC

    void report_upgrade_result(int sock_fd, int result)
    向pc报告路由器模块FWT升级结果
    
    void  ip_restore( int flag ) 
    多端口升级完成后恢复网关IP地址

    static inline unsigned char str2hexnum(unsigned char c)
    将字符转换成数字

    static int str2ipaddr(const char *str, unsigned char *ip)
    将"192.168.1.1"形式的ip地址转换存成数字，存入4字节的数组

    int scratch_ip_mac(char* report_buf)
    从nvram中获取ip和mac地址

    void multi_upgrade_nego(int udp_fd,unsigned char *buf,int len, 
        struct sockaddr_in* client_udp_addr )
    装备升级协商的主程序。分析udp中的hdlc格式命令，执行相应动作，报告执行情况

    <D00030.DOWN: lichenglong 2007-4-10 ADD BEGIN
    void gen_hdlc_packet(char* payload,long size,char* hdlc)
    D00030.DOWN: lichenglong 2007-4-10 ADD END> 

    产生hdlc报文，计算crc，转义payload中的0x7e和0x7d字节
    char parse_tcp_payload (unsigned char *buf,long len, 
        unsigned char* out_buf,long* out_len)
    解析tcp中封装的一个hdlc报文

    long parse_udp_payload (unsigned char *buf,long len, 
        unsigned char* out_buf,long* out_len)
    解析udp中封装的一个hdlc报文
    
    int main(int argc, char **argv)
    主函数是路由器模块、无线模块升级入口和主流程；
    实现端口侦听，解析diag命令，根据命令写路由器模块flash或者，转发Diag数据到网口

修改历史记录列表: 
<作  者>    lichenglong
<修改时间>
<版本>      1.0
<修改描述>

备注: 
************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/errno.h>
#include <linux/errno.h>
#include <sys/signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/tcp.h>
#include <sys/time.h>
#include "netjet.h"
#include "dump.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#ifdef ORG_DIAG
#include "atptag.h"
#include "atpflash.h"
#include <bcmnvram.h>
#include <shutils.h>

#define IMAGE_MAIN_S             "MAIN_S"
#endif

#define MAX_NVLINESTR     256 

#define SIGNAL(s, handler)	do { \
	sa.sa_handler = handler; \
	if (sigaction(s, &sa, NULL) < 0) \
	    printf("Couldn't establish signal handler (%d): %m", s); \
    } while (0)

#ifndef ORG_DIAG
typedef enum
{
    MAIN_F,
    SLAVE_F,
    MAIN_S,
    SLAVE_S,
}ATP_UPG_EN;
#endif


int        g_listen_fd = -1;
int        g_netfd = -1;  //tcp socket描述符
int        g_usbfd = -1;  //usb设备描述符
int        g_udpfd = -1;  //udp socket描述符
int        g_logsys = -1; //日志文件描述符

pid_t     g_udp_pid   = -1;
pid_t     g_sub_pid   = -1;
pid_t     g_main_pid = -1;

int        g_Serverport = SERVER_PORT;  //tcp、udp侦听端口号
int        g_current_udp_port = -1;     //记录的当前udp对端端口号

char       g_i_upgrade = NOT_UPGRADE;   //状态机是否处于某种升级状态或MSM透传状态
int        g_net_alive = NET_TIMEOUT;   //tcp 连接超时timer值
USB_PORT_E g_ePort = _DIAG;             //usb设备号

static int g_disconnect = NOT_DISCONNET;//是否断开tcp连接

static diag_status  g_state;            //升级状态机的状态



#ifndef ORG_DIAG
extern unsigned int BSP_MSM_On( );

extern unsigned int BSP_MSM_Off( );

extern unsigned int BSP_MSM_Force_On( );

extern unsigned int BSP_MSM_Force_Off( );
#endif


#define      NVRAM_PARTITION    "mtd5"  
static   int   g_current_tcp_port = -1;
#define      EXPECT_UDP_PORT  "expect_udp_port"
#define      VALID_CONNECT      0
#define      INVALID_CONNECT  -1
static   int   g_next_connect     = VALID_CONNECT;

static int g_waitMsmTimeout = 0;            //是否断开tcp连接
static int g_reboot_all = 0;            //是否重启无线和路由器模块        

#ifndef ORG_DIAG

/*hdlc crc compute table*/
unsigned short crc_16_l_table[ 256 ] = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
  0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
  0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
  0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
  0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
  0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
  0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
  0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
  0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
  0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
  0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
  0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
  0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
  0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
  0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

static unsigned long Crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
#endif

#define assert(cond) do { \
    if (!(cond)) \
    { \
        syslog("##cond is not"); \
        exit(0); \
    } \
    else \
    { \
        ; \
    } \
}while(0)


#ifndef ORG_DIAG

#define NV_FILE               "/var/nvfile"
#define NV_TMP_FILE       "/var/nvoldfile"
#define NV_ITEM_LEN     			256
#define NV_ITEM_NOT_FOUND 		0
#define NV_ITEM_FOUND 			1


/*****************************************************************************
 函 数 名  : file_set
 功能描述  : 替代nvram_set
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月6日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
void file_set( char* pcKey,   char* pcValue)
{
	FILE* fs = NULL;
	FILE* fsTmp = NULL;
	char acBuf[NV_ITEM_LEN];
	char acKey[NV_ITEM_LEN/2];
	int iFlag = NV_ITEM_NOT_FOUND;

	memset(acBuf, 0, sizeof(acBuf));
	
	fs = fopen(NV_FILE, "r");
	if (!fs)
	{
		fs = fopen(NV_FILE, "wt+");
		if (!fs)
		{
			printf("Can't create file for nvfile\r\n");
			return;
		}

		sprintf(acBuf, "%s=%s\n", pcKey, pcValue);

		fputs(acBuf, fs);
		fclose(fs);
		return;
	}

	fsTmp = fopen(NV_TMP_FILE, "wt+");
	if (!fsTmp)
	{
		fclose(fs);
		return;
	}

	while (!feof(fs)) 
	{
		memset(acBuf, 0, sizeof(acBuf));

		if (!fgets(acBuf, NV_ITEM_LEN, fs))
		{
			continue;
		}

		sprintf(acKey, "%s=", pcKey);
		if (0 == strncmp(acKey, acBuf, strlen(acKey)))
             {
			sprintf(acBuf, "%s=%s\n", pcKey, pcValue);
			fputs(acBuf, fsTmp);
			iFlag = NV_ITEM_FOUND;
             }
		else
		{
			fputs(acBuf, fsTmp);
		}
	}
	
	if (NV_ITEM_NOT_FOUND == iFlag)
	{
		sprintf(acBuf, "%s=%s\n", pcKey, pcValue);
		fputs(acBuf, fsTmp);
	}

	fclose(fs);
	fclose(fsTmp);

	unlink(NV_FILE);
	rename(NV_TMP_FILE, NV_FILE);

}

/*****************************************************************************
 函 数 名  : file_get
 功能描述  : 替代nvram_get
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月6日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
char*  file_get(char* pcKey)
{
	FILE* fs = NULL;
	char acBuf[NV_ITEM_LEN];
	char acKey[NV_ITEM_LEN/2];
	int iFlag = NV_ITEM_NOT_FOUND;

	fs = fopen(NV_FILE, "r");
	if (!fs)
	{
		return NULL;
	}

	while(!feof(fs))
	{
		memset(acBuf, 0, sizeof(acBuf));

		if (!fgets(acBuf, NV_ITEM_LEN, fs))
		{
			continue;
		}

		sprintf(acKey, "%s=", pcKey);
		if (0 == strncmp(acKey, acBuf, strlen(acKey)))
             {
			iFlag = NV_ITEM_FOUND;
			break;
             }
	}

	fclose(fs);

	acBuf[strlen(acBuf) - 1] = '\0';

	if (NV_ITEM_NOT_FOUND == iFlag)
	{
		return NULL;
	}

	return acBuf+strlen(acKey);
}

#endif

#ifdef ORG_DIAG
/*****************************************************************************
 函 数 名  : UPG_GetCrc
 功能描述  : 
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2007年9月8日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
unsigned int upgrade_bcm_getCrc( unsigned char* pcData, unsigned long ulSize, unsigned long ulCrc )
{
    while (ulSize-- > 0)
    {
      ulCrc = (ulCrc >> 8) ^ Crc32_table[(ulCrc ^ *pcData++) & 0xff];
    }
    
    return ulCrc;
}

/*****************************************************************************
 函 数 名  : UPG_FlashImage
 功能描述  : 升级Image
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  : 
 被调函数  : 
 修改历史      :
  1.日    期   : 2008年3月29日
    作    者   : yangmuhan 00115589
    修改内容   : 新生成函数
*****************************************************************************/
 int upgrade_bcm_image(const char *pcImage, int ulImageLen )
{
    PFILE_TAG  pstTag;
    unsigned int ulNowGrade    = 0;
    ATP_UPG_EN enUpgState    = MAIN_F;
    char *pcKernelFs    = pcImage;
    int   bBootloader   = 0;
    unsigned int ulMainOffset  = 0;
    unsigned int ulSlaveOffset = 0;
    char*  pcTmp         = NULL;
    unsigned int ulLen = 0;
   unsigned int ulCrc = CRC32_INIT_VALUE;
    unsigned char  cCrc[CRC_LEN];
    unsigned int ImageLen = 0;	
    unsigned int ImageCrcLen = 0;

    syslog ("upgrade_bcm_image  ulImageLen=%d",ulImageLen);
    /*Start of comment :镜像文件不能小于校验crc的长度*/
    if (NULL  == pcImage || 0x200000 > ulImageLen)
    {
        printf("pcImage is Illegal.\n");
        printf("\r\nUpgrade image failed. image file's empty.\r\n");
        return -1;
    }
    
   ImageLen=pcImage + ulImageLen -TOKEN_LEN ;
   ImageCrcLen=ulImageLen -TOKEN_LEN ; 	 
   memcpy((void *)cCrc , (void *)(ImageLen), 4);  

   ulCrc = upgrade_bcm_getCrc(pcImage, ImageCrcLen  , ulCrc); 

    if(((unsigned char)(ulCrc>>24) == cCrc[0]) && ((unsigned char)(ulCrc>>16) == cCrc[1]) 
    	&& ((unsigned char)(ulCrc>>8) == cCrc[2]) && ((unsigned char)(ulCrc) == cCrc[3]))
	{	 
		printf("\n crc check ok\n");
		BSP_FLASH_Write(pcImage, 0x20000, (ulImageLen-TOKEN_LEN));
		printf("\r\nwriting Main success flag:%s\r\n", IMAGE_MAIN_S);
		BSP_FLASH_WriteArea(BSP_E_FLASH_AREA_TEMP_PARAM,
		                        IMAGE_MAIN_S,
		                        0,
		                        UGPINFO_SIZE);
		 return 0;
	}
    else
	{
		printf("all crc error:ulCrc =  %x \r\n", ulCrc);
		return -1;
	}
	
}
    

/*---------------------------------------------------------------------------
  函数原型:int nvram_restore_defaults(void)
  描     述:此函数用于读取运营商要求定制的资源文件中的NV
                   项进行nv 设置.
  输     入:无.
  输     出:把资源文件中的nv设置到mtd5中
  返回值:-1 错误
                    0 正确
  ----------------------------------------------------------------------------*/
int nvram_restore_defaults(void)
{
    FILE *fp;
    int i = 0, value_len = 0;
    char *get_buf = NULL,  *nv_name = NULL, *nv_value = NULL;
     
    if ((fp = fopen("/var/custom/res_custom/vendor_nvram.res","r")) == NULL)
    {
        printf("cannot open file\n");
        return -1;
    }
    
    get_buf = (char*)malloc(MAX_NVLINESTR);
    if (get_buf == NULL)  
    {
        free(get_buf);
        get_buf = NULL;
        fclose(fp);
        return -1;
    }
     
    memset(get_buf, 0, sizeof(MAX_NVLINESTR));
    while (fgets(get_buf, MAX_NVLINESTR, fp) != NULL) 
    {
        nv_name = get_buf;
        
        while ((get_buf[i] != '=') && (get_buf[i] !='\0'))
        {
            i++;
        }
        
        nv_value = &get_buf[i + 2];
        get_buf[i] = '\0';
        value_len = strlen(nv_value) -3;
        nv_value[value_len] = '\0'; 
        
        nvram_set(nv_name,nv_value);
        
        memset(get_buf, 0, sizeof(MAX_NVLINESTR));
        i = 0;               
    }       
     
    free(get_buf); 
    get_buf =  NULL;
    fclose(fp); 
    return 0;
}
#endif

/*------------------------------------------------------------
  函数原型 : int __msleep(int millisecond )
  描述     : 实现以毫秒为单位的不被唤醒的等待
  输入     : millisecond(睡眠时间)
  输出     : 无
  返回值   : OPER_SUCCESS: 成功
-------------------------------------------------------------*/
static int __msleep(int millisecond) 
{
    int ret;
    struct timeval tvSelect;
    
    tvSelect.tv_sec  = millisecond / 1000;
    tvSelect.tv_usec = (millisecond % 1000) * 1000;

    while (1)
    {
        ret = select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0,
                    (struct timeval*)&tvSelect);
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                DEBUGPRINT("interrupt by isr in sleep \n");
                continue;
            }
        }
        break;
    }

    return OPER_SUCCESS;
}

/*------------------------------------------------------------
函数原型: int daemon( int nochdir, int noclose )
描述: 实现linux 后台函数
输入:  int nochdir ＝ 1， 不切换当前目录，＝ 0 切换
       int noclose ＝ 1， 不关闭标志输入、输出、出错文件句柄，＝ 0 关闭
输出:  无
返回值: 成功返回0，错误返回－1
-------------------------------------------------------------*/
int diag_daemon(int nochdir, int noclose )
{
    int fd = -1;

    switch ( fork() ) 
    {
        case -1:
    	        return OPER_FAILURE;
        case 0:
            break;
        default:
            exit(0);
    }

    if (setsid() == OPER_FAILURE)
    {
        return OPER_FAILURE;
    }

    if (!nochdir)
    {
        chdir("/");
    }
    
    if (!noclose && (fd = open(_PATH_DEVNULL, O_RDWR, 0)) != OPER_FAILURE) 
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2)
        {
            close(fd);
        }
    }
    
    return 0;
}

/*------------------------------------------------------------
  函数原型: static int   init_listen_socket( usockaddr* usaP )
  描述: 创建侦听套接字
  输入: 侦听套接字结构
  输出: 侦听套接字结构
  返回值: 侦听套接字描述符
-------------------------------------------------------------*/
static int diag_init_tcp_listen_socket( usockaddr* uSockaddr_tcp )
{
    int listen_fd = -1;

    if (NULL == uSockaddr_tcp)
    {
        syslog("para error");
        return OPER_FAILURE;
    }
    
    memset( uSockaddr_tcp, 0, sizeof(usockaddr) );
    uSockaddr_tcp->sa.sa_family = AF_INET;
    uSockaddr_tcp->sa_in.sin_addr.s_addr = htonl( INADDR_ANY );
    uSockaddr_tcp->sa_in.sin_port = htons( SERVER_PORT );

    listen_fd = socket( uSockaddr_tcp->sa.sa_family, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
    {
        syslog ("create socket failure");
        return OPER_FAILURE;
    }
    if ( bind( listen_fd, &uSockaddr_tcp->sa, sizeof(struct sockaddr_in) ) < 0 )
    {
        syslog ("bind socket failure");
        close ( listen_fd );
        return OPER_FAILURE;
    }
    if ( listen( listen_fd, 1 ) < 0 )
    {
        syslog ("create listen socket failure");
        close ( listen_fd );
        return OPER_FAILURE;
    }
    return listen_fd; 
  }

/*------------------------------------------------------------
  函数原型: static int diag_udpinit_socket( )
  描述: 初始化udp套接字，用来控制PC端虚拟串口波特率和状态;
        同时，装备升级协商也使用这个端口。
  输入: 无
  输出: 无
  返回值: 初始化后的服务器端(即本地)UDP套接字
-------------------------------------------------------------*/
static int diag_udpinit_socket( void )
{
    struct sockaddr_in servaddr;                                   //IPv4套接口地址定义
    bzero (&servaddr, sizeof (servaddr) );                        //地址结构清零
    servaddr.sin_family        = AF_INET;                           //IPv4协议
    servaddr.sin_addr.s_addr= htonl (INADDR_ANY);        //内核指定地址
    servaddr.sin_port            = htons (g_Serverport);    //端口
    
    g_udpfd  = socket (AF_INET, SOCK_DGRAM, 0);       //建立UDP套接字
    if ( g_udpfd < 0 )
    {
        syslog("create socket failure");
        return OPER_FAILURE;
    }
    if (bind(g_udpfd , (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) /*分配协议地址,绑定端口*/
    {
        syslog("bind udp socket failure\n");
        close ( g_udpfd );
        g_udpfd = -1;
        return OPER_FAILURE;
    }
    return OPER_SUCCESS;
}


/*------------------------------------------------------------
  函数原型: void close_pcui_shake()
  描述: 关闭ppp程序
  输入: 无
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void close_pcui_shake( )
{
    system ("killall pppc");
    system ("killall usbserver");
}

/*------------------------------------------------------------
  函数原型: int open_usb_port()
  描述:  打开usb diag端口
  输入:  无
  输出:  无
  返回值:成功返回OPER_SUCCESS，失败返回OPER_FAILURE
-------------------------------------------------------------*/
int open_usb_port( void )
{
    char* pcReal_port = NULL;
    switch( g_ePort )
    {
        case _DIAG:
  	        pcReal_port = DIAG_PORT;
  	        break;
        case _PCUI:
  	        pcReal_port = PCUI_PORT;
  	        break;
        case _MODEM:
  	        pcReal_port = MODEM_PORT;
  	        break;
	 case _VOICE:
	 	 pcReal_port = VOICE_PORT;
        default:
  	        break;
    }

    if ((g_usbfd = open (pcReal_port, O_RDWR)) < 0)
    {
        if (errno == ENODEV )
        {
            syslog ( USB_ERR);
        } 
        else if (errno == ENODIAG)
        {
            syslog ( INTERFACE_ERR);
        }
        else
        {
            syslog ("%s \t ERROR CODE = [%d]", OTHER_ERR, errno);
        }
        return OPER_FAILURE;
    } 
    else 
    {
        syslog ( OPEN_SUCCESS );
        return OPER_SUCCESS;
    }
}

/*------------------------------------------------------------
  主进程
  函数原型: int net_to_diag()  
  描述: 转发数据到Diag 口，usb设备写出错就返回。否则，写完输入数据
  输入:   转发到MSM的网口数据
  输出:   无
  返回值: usb设备写操作的是否成功 
          成功:OPER_SUCCESS
          失败:OPER_FAILURE
-------------------------------------------------------------*/
int net_to_diag (unsigned char* buf,long len)
{
    int   send_len = 0;
    char  *pCur      = buf;
    int   retval     = OPER_SUCCESS;

    if (NULL == buf)
    {
        syslog("para error");
        return OPER_FAILURE;
    }
		
    /*如果USB 能够正确读写就会循环发送，直至把网口接收的数据全部
       发送出去，如果USB 端口不能发送则退出DIAG口发送内循环*/
    while ( len > 0 )
    {
        send_len = 0;
        send_len = write (g_usbfd, pCur, len);

        if (send_len > 0)
        {
            pCur += send_len;
            len -= send_len;
        }
        else if ((send_len < 0 ))
        {
            DEBUGPRINT("len=%ld errno=%d\n",len,errno);
            if (errno == EINTR)
            {
              continue;
            }
            else
            {
                close( g_usbfd );
                g_usbfd = -1;
                int  g_wait_usb = 0;
                struct stat s_stat;
                
                do
                {
                    if (( stat ( DIAG_PORT, &s_stat) == -1) 
                        || (open_usb_port() != OPER_SUCCESS))
                    { 
                        syslog( "Diag is Not exist !" ); 

                        __msleep(2000);
                        if ( ++g_wait_usb > 20)
                        {
                            syslog( "Can not open diag after 40s !" );
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }while(1);
                
                if ( -1 != g_usbfd )
                {
                    continue;
                }

               retval = OPER_FAILURE;
               break;
            }
        }
        else /*==0*/
        {
            ;/*empty sentence*/
        }
    }

    return retval;
}
/*------------------------------------------------------------
  函数原型: char diag_to_net()
  描述: 子进程从diag口接收数据，转发到网口
  输入: 无
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void diag_to_net ( void )
{
    char    rcv_buf [BUFFER_LEN]; //DIAG 接收buffer
    int     rcv_len = 0;
    int     retval = OPER_SUCCESS;

    while (1)
    {
        rcv_len = read ( g_usbfd, rcv_buf, BUFFER_LEN-1);
        if ( (rcv_len < 0))
        {
            if ( errno == EINTR)
            {
            	  syslog("receive EINTR!");
                retval = OPER_SUCCESS;
                continue;
            }
        }
        else if(rcv_len == 0)
        {
            int  g_wait_usb = 0;
            struct stat s_stat;
            do
            {
                if (( stat ( DIAG_PORT, &s_stat) == -1) 
                    || (open_usb_port() != OPER_SUCCESS))
                { 
                    syslog( "Diag is Not exist !" ); 

                    __msleep(2000);
                    if ( ++g_wait_usb > 20)
                    {
                        syslog( "can not open diag after 40s !" );
                        break;
                    }
                }
                else
                {
                    break;
                }
            }while(1);
            
            if ( -1 != g_usbfd )
            {
                continue;
            }
        
            syslog ("Usb errno = [%d]", errno);
            retval = OPER_FAILURE;
            exit(0);
        }
        else
        {
            switch ( g_ePort )
            {
                case _MODEM:
                case _PCUI:
                    retval = echo_to_net (rcv_buf, rcv_len, 0);
                    break;
                case _DIAG:
                    retval = send_to_net (rcv_buf, rcv_len, 0);
                    break;
                default:
                    syslog("Usb port are not exist!");
                    break;
            }
            if( OPER_SUCCESS == retval )
            {
                g_net_alive = NET_TIMEOUT;
            }
        }
    }
}

/*------------------------------------------------------------
  函数原型: int diag_accept_connect (int listen_fd)
  描述: 接收对方TCP请求，用全局描述g_netfd作为TCP连接套接字描述符
  输入: int listen_fd 侦听套接字
  输出: 无
  返回值: 成功 返回OPER_SUCCESS
          失败 返回OPER_FAILURE
-------------------------------------------------------------*/
int diag_accept_connect (int listen_fd )
{
    struct sockaddr_in client_addr;
    socklen_t socket_Len = sizeof(client_addr);

    if((g_netfd = accept (listen_fd, (struct sockaddr* )&client_addr, &socket_Len)) < 0)
    { 
        if (errno != EINTR)
        {
            syslog ("Tcp connect failure! errno = [%d]", errno);
        }
        else
        {
            __msleep (100);
        }
        return OPER_FAILURE;
    }
    g_current_tcp_port = ntohs(client_addr.sin_port);
    if ( INVALID_CONNECT == g_next_connect )
    {
        g_next_connect = VALID_CONNECT;
        return OPER_FAILURE;
    }
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: void tcp_timeout( )
  描述: 1秒的定时器，老化g_net_alive值;为等待MSM定时
  输入: 无
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
void tcp_timeout ( int sig )
{
    /*TCP_INIT_S,WAIT_CONNECT_S,WRITE_BCM_S cann't timeout tcp connection*/
    if (SIGALRM != sig || (TCP_INIT_S == g_state) 
         || (WAIT_CONNECT_S == g_state) || (WRITE_BCM_S == g_state))
    {
        return;
    }

    //强制升级状态，这里为等待MSM定时，不老化tcp连接
    if ( FORCE_MSM_S == g_state )
    {
        g_waitMsmTimeout += 1;
        return;
    }
    
    if ( g_net_alive-- < 0 )
    {
        g_disconnect = TCP_TIMEOUT;/*disconnect tcp connection*/
    }
}

/*------------------------------------------------------------
  函数原型: static void main_coordinate(int sig)
  描述: 主进程信号处理程序
  输入: sig  接收信号值
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
static void main_coordinate ( int sig )
{
    syslog("Procedure %d recv Signal = %d", getpid (), sig );

    if (SIGPIPE == sig)
    {
        if ( NOT_UPGRADE != g_i_upgrade )
        {
            g_reboot_all = 1;/*reboot msm and bcm*/
        }
    }
    else if (SIGTERM == sig)
    {
        if ( -1 != g_usbfd )
        {
            close(g_usbfd);
            g_usbfd = -1;
        }

        if ( -1 != g_netfd )
        {
            close(g_netfd);
            g_netfd = -1;
        }

        if ( -1 != g_listen_fd )
        {
            close(g_listen_fd);
            g_listen_fd = -1;
        }
        exit(0);
    }
    else if (SIGUSR1 == sig)
    {
#ifdef ORG_DIAG 
        int  expect_udp_port = atoi(nvram_get(EXPECT_UDP_PORT));
#else
	  int  expect_udp_port = atoi(file_get(EXPECT_UDP_PORT));
#endif
	  
	  if (-1 != g_current_tcp_port)
        {
            if ((g_current_tcp_port - 1) <  expect_udp_port)
            {
                g_disconnect = UDP_NOTIFY_DISCONNECT;/*notify tcp need re-connect*/
                g_current_tcp_port = -1;
                g_next_connect = INVALID_CONNECT;
            }
            else
            {
               ;
            }
        }
    }
    else if (SIGCHLD == sig)
    {
        int  retval = 0;
        if( g_udp_pid == waitpid(-1, &retval, WNOHANG) )
        {
            syslog ("Exit main process due to udp server [%d] has down", g_udp_pid);
            exit(0);
        }
    }
    else
    {
        ;
    }

}

/*------------------------------------------------------------
  函数原型: static void sub_coordinate(int sig)
  描述: 子进程信号处理程序
  输入: sig  接收信号值
  输出: 无
  返回值: 无
-------------------------------------------------------------*/
static void sub_coordinate ( int sig )
{
    syslog("Procedure %d recv Signal = %d", getpid (), sig );

    if ( (SIGPIPE == sig) || (SIGTERM == sig) )
    {
        if ( -1 != g_usbfd )
        {
            close(g_usbfd);
            g_usbfd = -1;
        }

        if ( -1 != g_netfd )
        {
            close(g_netfd);
            g_netfd = -1;
        }
        
        if ( -1 != g_listen_fd )
        {
            close(g_listen_fd);
            g_listen_fd = -1;
        }
        exit(0);
    }
}

/*------------------------------------------------------------
  函数原型: static void udp_coordinate ( int sig )
  描述:     udp子进程信号处理程序
  输入:     sig  接收信号值
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
static void udp_coordinate ( int sig )
{
    syslog("Procedure %d recv Signal = %d", getpid (), sig );

    if ( (SIGPIPE == sig) || (SIGTERM == sig) )
    {
        if ( -1 != g_udpfd )
        {
            close(g_udpfd);
            g_udpfd = -1;
        }
        exit(0);
    }
}

/*------------------------------------------------------------
Function:     analyse_cmd
Description:分析hdlc报文payload，找出升级路由器，升级无线模块，强制升级，其他情况。
Calls:           无
Data Accessed: 无
Data Updated:   无
Input:   
     unsigned char* buf 表示输入的需要分析的hdlc payload.		
Output: 
     long *img_len: 如果为升级路由器模块，表示路由器模块的映象长度；
                    对于其他命令类型，无意义。
Return:
 	函数返回值为BCM_UPGRADE：表示收到升级路由器模块的命令，可以接收后续路由器image,进入路由器升级操作了。
 	函数返回值为MSM_UPGRADE：表示收到升级无线模块的命令，可以透传无线升级映像到usb口了。
 	函数返回值为MSM_FORCE_UPGRADE：表示收到强制升级无线模块的命令，可以发命令重启无线模块，透传接收到后续无线模块升级image了。
 	函数返回值为NOT_UPGRADE：表示无线模块的其他命令（除了升级），可以直接透传到usb口。
Others:          无
-------------------------------------------------------------*/
long analyse_cmd(unsigned char* buf,long *img_len)
{
    long retval=0;
    
    if (NULL == buf || NULL == img_len)
    {
        syslog("impossible,para error");
        return OPER_FAILURE;
    }
    
    if (UPGRADE_CMD_TYPE != *buf)
    {
        retval = NOT_UPGRADE;
    }
    else if (0 == strncmp(MSM_UPGRADE_STR,buf+1,29))
    {
        DEBUGPRINT(" FIND MSM_UPGRADE cmd\n");
        retval = MSM_UPGRADE;
    }
    else if (0 == strncmp(MSM_FORCE_UPGRADE_STR,buf+1,29))
    {
        DEBUGPRINT(" FIND MSM_FORCE_UPGRADE_STR cmd\n");
        retval = MSM_FORCE_UPGRADE;
    }
    else if (0 == strncmp(BCM_UPGRADE_STR,buf+1,29))
    {
        DEBUGPRINT(" FIND BCM_UPGRADE_STR cmd\n");
        
        /*get bcm image len after 1 byte cmd_type and len of BCM_UPGRADE_STR*/
        buf = buf+1+strlen(BCM_UPGRADE_STR);
        
        *img_len = *(buf+3) * (1<<24) 
                  + *(buf+2) * (1<<16) 
                  + *(buf+1) * (1<<8) 
                  + *(buf);
        DEBUGPRINT("bcm_img_len=%ld\n",*img_len);
        retval = BCM_UPGRADE;
    }
    else
    {
        ;//empty sentence
    }
    return retval;
}

#ifdef ORG_DIAG
extern unsigned int UPG_FlashImage(const unsigned char *pcImage, unsigned int ulImageLen);
/*------------------------------------------------------------
  函数原型: static EN_UPGRADE_ERRCODE route_fwt_upgrade(char *part_img, long len, long *total)
  描述:     本函数负责将接受到tcp数据写入下层函数
  输入:     接受的数据
  输出:     路由器映象剩余长度
  返回值:   操作错误类型
-------------------------------------------------------------*/
static int route_fwt_upgrade(char *part_img, long img_len)
{
   return upgrade_bcm_image(part_img,img_len);
}
#endif
/*------------------------------------------------------------
  函数原型: void udp_server_main(pid_t parent_pid)
  描述:     又一个子进程，侦听udp的1280端口，处理virtual console和多端口协商报文
  输入:     父进程的进程号，用于发信号反馈tcp重连信息
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
void udp_server_main(pid_t parent_pid)
{
    fd_set rfds;
    struct timeval tv;
    int retval = 0; 
    tv.tv_sec  = _interval;
    tv.tv_usec = UDP_SRV_TIMEOUT;

    struct sockaddr_in client_udp_addr;
    socklen_t socket_Len = sizeof(client_udp_addr);
    char    udp_port[10] = {0};

#ifndef FEATURE_E960
    sigset_t sigmask, orig_sigmask;
#endif

    signal (SIGTERM,  udp_coordinate);
    
    if ( diag_udpinit_socket() !=  OPER_SUCCESS)
    {
        syslog ("Udp initialize failed!");
        exit(0);
    }
    else
    {
        syslog ("Udp initialize successful");
    }

    DEBUGPRINT(" g_udpfd=%d parent_pid=%d\n",g_udpfd,parent_pid);

    while (1)
    {
        bzero ( g_udp_buf, MAX_NEGRO_LEN );
        FD_ZERO ( &rfds );
        if ( g_udpfd > 0 )
        {
            FD_SET ( g_udpfd,  &rfds );
        }
        else
        {
            syslog("g_udpfd become error value %d!",g_udpfd);
            exit(0);
        }
        
        tv.tv_sec  = _interval;
        tv.tv_usec = UDP_SRV_TIMEOUT;
        retval = select ( g_udpfd + 1, &rfds, NULL, NULL, &tv);

        if (0 >= retval) 
        {
            continue;
        }
        
        if (FD_ISSET (g_udpfd, &rfds))
        {
            retval = recvfrom (g_udpfd, g_udp_buf, MAX_NEGRO_LEN, 0,\
                      (struct sockaddr *)&client_udp_addr, &socket_Len);

            //处理错误
            if (retval < 0)
            {
                syslog("udp recvfrom fail, g_udpfd=%d errno=%d", g_udpfd, errno);
                continue;
            }


           retval = multi_upgrade_nego(g_udpfd, g_udp_buf, retval, &client_udp_addr);


           if ( (0 == retval) || (EQUIP_RUN == g_i_upgrade))
           {
                continue;
           }

            /*如果后续的udp连接源端口变化了，通知重新连接tcp*/          
            if ( -1 != g_current_udp_port && ntohs (client_udp_addr.sin_port) != g_current_udp_port)
            {
                sprintf(udp_port, "%d", g_current_udp_port);
#ifdef ORG_DIAG		
                nvram_set(EXPECT_UDP_PORT, udp_port);
#else
		   file_set(EXPECT_UDP_PORT, udp_port);
#endif
		   kill (parent_pid,SIGUSR1);
            }

            g_current_udp_port = ntohs (client_udp_addr.sin_port);
            DEBUGPRINT(" recvfrom udp_port:%d\n",g_current_udp_port);

            PNETJET_FRAME  pFrame = (PNETJET_FRAME) g_udp_buf;
            if ( pFrame )
            {
                switch ( pFrame->Type )
                {
                    case NETJET_COMMAND_OPEN:
                    case NETJET_COMMAND_SET_LINE_CONTROL:
                    case NETJET_COMMAND_SET_BAUD_RATE:
                    case NETJET_COMMAND_SET_HARDLINE:
                    case HARDLINE_CLRDTR:
                    case HARDLINE_CLRRTS:
                    case HARDLINE_SETDTR:
                    case HARDLINE_SETRTS:
                    case HARDLINE_SETBREAK:
                    case HARDLINE_CLRBREAK:
                    case HARDLINE_RESETDEV:
                        break;
                    case NETJET_COMMAND_CLOSE:
                        syslog ("The peer close socket own to virtual serial disconnect!");
                        sprintf(udp_port, "%d", g_current_udp_port + 1);
#ifdef ORG_DIAG
                        nvram_set(EXPECT_UDP_PORT, udp_port);
#else
			     file_set(EXPECT_UDP_PORT, udp_port);
#endif
                        kill (parent_pid, SIGUSR1);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return ;
}

/*------------------------------------------------------------
Function:      parse_nego_cmd
Description:   多端口升级中，分析升级协商命令内容。
Calls:         无
Data Accessed: 无
Data Updated:  无
Input:         unsigned char *buf：udp payload的存放buf
 	           int len：udp payload的长度
Output:        char*   report_buf，后续使用到的值，如hdlc的头尾，
               ip_change时的ip地址等，存放到里面，以供report 给pc时使用。
Return:        协商命令类型
Others:        无
-------------------------------------------------------------*/
nego_state parse_nego_cmd (unsigned char *buf, int len, unsigned char *report_buf)
{
    long out_len = 0;
    int retval = 0; 

    if (NULL == buf || NULL == report_buf)
    {
        syslog("parameter error");
        return EQUIP_INVALID;
    }
    
    retval = parse_udp_payload( buf, len, report_buf, &out_len);
    if((OPER_SUCCESS != retval)
        || (out_len - 2 != sizeof (s_report_data)))    /*not get complete hdlc payload*/
    {
        return EQUIP_INVALID;
    }

    return   report_buf[0];
}

/*------------------------------------------------------------
  函数原型: int  diagd_ip_set() 
  描述:    单板启动后， 重新设置IP地址
  输入:     无
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
void  diagd_ip_set() 
{
    char *value = NULL;
	char  buf[50];
#ifdef ORG_DIAG 
   value = nvram_get("lan_ipaddr");
#else
   value = file_get("lan_ipaddr");
#endif


	printf("\ndiagd_ip_set value = %s\n",value);
	if((NULL != value) && (value[0] != '\0'))
	{
		memset(buf,0,50);
		sprintf(buf,"ifconfig br0 %s",value);
		system(buf);
	}
}

/*------------------------------------------------------------
  函数原型: int ip_change(char * Reportbuf)
  描述: 更改IP
  输入: char * Reportbuf, 包含要更改的IP地址
  输出: 无
  返回值: 更改成功返回 : OPER_SUCCESS 
                  失败 : OPER_FAILURE
-------------------------------------------------------------*/
int ip_change(char * Reportbuf)
{
    if (NULL == Reportbuf)
    {
        syslog("para error");
        return OPER_FAILURE;
    }
    
    s_report_data*  pReport = (s_report_data *)Reportbuf;
    char ip_addr[17];
	memset(ip_addr,0,17);

    sprintf(ip_addr, "%d.%d.%d.%d", pReport->ip[0], pReport->ip[1],
        pReport->ip[2],pReport->ip[3]);
  
#ifdef ORG_DIAG
    nvram_set("lan_ipaddr", ip_addr);
    nvram_commit();
#else
    file_set("lan_ipaddr", ip_addr);
#endif
    DEBUGPRINT("The gateway ip address have success to change to %s\n", ip_addr);

	diagd_ip_set();
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: int report_to_pc(int sock_fd, char* report_buf, struct sockaddr_in* pClientSock)
  描述:     使用hdlc报文，把udp协商的执行结果报告给PC
  输入:     int sock_fd                      UDP 套接字 
            char* report_buf                 UDP协商命令buf，没有转义的，长度是确定的
            struct sockaddr_in* pClientSock  PC的IP地址
  输出:     无
  返回值:   上报成功返回OPER_SUCCESS，失败返回OPER_FAILURE
-------------------------------------------------------------*/
int report_to_pc(int sock_fd, char* report_buf, struct sockaddr_in* pClientSock)
{
    int   iNet_sLen = 0;
    long   hdlc_len=0;
    char  hdlc[MAX_CMD_TRANS_LEN] = {0},*phdlc=hdlc;
    socklen_t socket_Len = sizeof(struct sockaddr_in);

    if (NULL == report_buf || NULL == pClientSock)
    {
        syslog("para error");
        return OPER_FAILURE;
    }

    hdlc_len = gen_hdlc_packet(report_buf, sizeof(s_report_data), phdlc);
    if ( -1 == hdlc_len )
    {
        syslog("gen_hdlc_packet fail");
        return OPER_FAILURE;
    }
    
    while (hdlc_len != 0)
    {
        iNet_sLen = sendto(sock_fd, phdlc, hdlc_len, 0, (struct sockaddr *)pClientSock, socket_Len);                

        if (iNet_sLen == SOCKET_ERROR && errno != EINTR)
        {
            syslog("failure when report status to pc %d", errno);
            return OPER_FAILURE;
        }

        hdlc_len -= iNet_sLen;
        phdlc  += iNet_sLen;
    }
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: int report_upgrade_result(int sock_fd, int result)
  描述:     上报升级路由器模块的结果给PC    
  输入:     tcp socket描述符；
            result: OPER_SUCCESS表示升级成功,OPER_FAILURE表示升级失败
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
void report_upgrade_result(int sock_fd, int result)
{
    char payload[UPGRADE_RESULT_PAYLOAD_LEN] = {0};
    char hdlc[UPGRADE_RESULT_PAYLOAD_LEN*2] = {0},*phdlc=hdlc;
    long hdlc_len = 0,iNet_sLen = 0;
    
    payload[0] = REPORT_CMD_WORD;
    memcpy(payload+CMD_WORD_LEN,BCM_UPGRADE_STR,strlen(BCM_UPGRADE_STR));

    if ( OPER_SUCCESS == result )
    {
        payload[CMD_WORD_LEN + strlen(BCM_UPGRADE_STR)] = UPGRADE_RESULT_SUCCESS;
    }
    else
    {
        payload[CMD_WORD_LEN + strlen(BCM_UPGRADE_STR)] = UPGRADE_RESULT_FAILURE;
    }
    
    hdlc_len = gen_hdlc_packet(payload, UPGRADE_RESULT_PAYLOAD_LEN, phdlc);
    if ( -1 == hdlc_len )
    {
        syslog("gen_hdlc_packet fail");
        return;
    }

    while (hdlc_len != 0)
    {
        iNet_sLen = send(sock_fd, phdlc, hdlc_len, 0);                

        if (iNet_sLen == SOCKET_ERROR ||iNet_sLen == 0)
        {
            syslog("Can't report upgrade result!");
            return;
        }

        hdlc_len -= iNet_sLen;
        phdlc    += iNet_sLen;
    }
    return;
}

/*------------------------------------------------------------
  函数原型: int  ip_restore() 
  描述:     多端口升级完成后恢复网关IP地址
  输入:     无
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
void  ip_restore() 
{
	system("ifconfig br0 192.168.1.1");
#ifdef ORG_DIAG
	nvram_set("lan_ipaddr", "192.168.1.1");
	nvram_commit();
#else
	file_set("lan_ipaddr", "192.168.1.1");
#endif

    return ;
}



/*------------------------------------------------------------
  函数原型: static inline unsigned char str2hexnum(unsigned char c)
  描述:     将字符转换成数字
  输入:     字符
  输出:     无
  返回值:   数字
-------------------------------------------------------------*/
static inline unsigned char str2hexnum(unsigned char c)
{
    if(c >= '0' && c <= '9')
       return c - '0';
    if(c >= 'a' && c <= 'f')
       return c - 'a' + 10;
    
    if(c >= 'A' && c <= 'F')
       return c - 'A' + 10;

    return 0; /* foo */
}

/*------------------------------------------------------------
  函数原型: static inline void str2eaddr(unsigned char *ea, unsigned char *str)
  描述:     将字符串转换为mac地址
  输入:     字符串形式的mac地址  
  输出:     数字形式的mac地址
  返回值:   无
-------------------------------------------------------------*/
static inline void str2eaddr(unsigned char *ea, unsigned char *str)
{
    int i;
	
    if (NULL == ea || NULL == str)
    {
        syslog("para error");
        return ;
    }

    for (i = 0; i < 6; i++) {
        unsigned char num;

        if(*str == ':')
        {
            str++;
        }
        num = str2hexnum(*str++) << 4;
        num |= (str2hexnum(*str++));
        ea[i] = num;
    }
}

/*------------------------------------------------------------
  函数原型: static void str2ipaddr(const char *str, unsigned char *ip)
  描述:     将"192.168.1.1"形式的ip地址转换存成数字，存入4字节的数组
  输入:     需转换的字符串
  输出:     4字节数组
  返回值:   成功 : OPER_SUCCESS
            失败 : OPER_FAILURE
-------------------------------------------------------------*/
static int str2ipaddr(const char *str, unsigned char *ip)
{
    if (NULL == str || NULL == ip)
    {
        syslog("para error!");
        return OPER_FAILURE;
    }
    
    int split[4];
    
    sscanf(str, "%d.%d.%d.%d", &split[0], &split[1], &split[2], &split[3]);

    if ( split[0]>255 || split[1]>255 || split[2]>255 || split[3]>255 )
    {
        syslog("Invalid IP address");
        return OPER_FAILURE;
    }

    ip[0] = split[0] & 0xff;
    ip[1] = split[1] & 0xff;
    ip[2] = split[2] & 0xff;
    ip[3] = split[3] & 0xff;

    return OPER_SUCCESS;
}

/*------------------------------------------------------------------------------
  函数原型: int scratch_ip_mac(char* report_buf)
  描述:     从nvram中获取ip和mac地址
  输入:     无
  输出:     获得的ip和mac地址
  返回值:   操作成功返回OPER_SUCCESS，失败返回OPER_FAILURE
/*通过ifconfig来获取MAC，IP地址其格式如下
* br0     Link encap:Ethernet  HWaddr 00:11:22:33:AB:DE
*           inet addr:192.168.1.3  Bcast:192.168.1.255  Mask:255.255.255.0
*           UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
            RX packets:0 errors:0 dropped:0 overruns:0 frame:0
            TX packets:71 errors:0 dropped:0 overruns:0 carrier:0
            collisions:0 txqueuelen:0
            RX bytes:0 (0.0 B)  TX bytes:14220 (13.8 KiB)
																																							*/
/*----------------------------------------------------------------------------*/

int scratch_ip_mac(char* report_buf)
{
	FILE* diagd_tmp_file = NULL;
	char rcv_buf[600];
	char tempBuf[50];
	char *cpread = NULL,*head = NULL,*temp = NULL;
	int retval;
	int i =0;

	memset(rcv_buf,0,600);
	s_report_data*  pReport = (s_report_data *)report_buf;	
	
	system("ifconfig br0 > var/tmp/diagd_macip.txt");
	__msleep(10);
	diagd_tmp_file  = fopen("var/tmp/diagd_macip.txt","a+");
	if(NULL == diagd_tmp_file)
	{
		return OPER_FAILURE;
	}
	retval = fread(rcv_buf, 600, 1, diagd_tmp_file);	
	head = rcv_buf;
	cpread = strstr(head,"HWadd");
	memset(tempBuf,0,50);
	cpread = cpread+strlen("HWadd")+2;
	memcpy(tempBuf,cpread,17);
	str2eaddr(pReport->mac,tempBuf);
	head = cpread +17;
	cpread = strstr(head,"inet addr:");
	temp = strstr(head,"Bcast");
	cpread = cpread+strlen("inet addr:");
	memset(tempBuf,0,50);
	memcpy(tempBuf,cpread,temp-cpread);
	retval = str2ipaddr(tempBuf,pReport->ip);
	fclose(diagd_tmp_file);
	system("rm -f var/tmp/diagd_macip.txt");
	if(-1 == retval)
	{
		printf("\r\n IP :%s,retval = %d\r\n",tempBuf,retval);
		return OPER_FAILURE;
	}

	return OPER_SUCCESS;
	
}

/*------------------------------------------------------------
  函数原型: int mac_match(char*localmac, char * reqmac)
  描述:     检查地址是否匹配
  输入:     无
  输出:     无
  返回值:  匹配成功返回OPER_SUCCESS，匹配失败返回OPER_FAILURE
-------------------------------------------------------------*/
int mac_match(char*localmac, char * reqmac)
{
     char  zero[6] = {0};
    if ( ((localmac[0] != reqmac[0])
        || (localmac[1] != reqmac[1])
        || (localmac[2] != reqmac[2])
        || (localmac[3] != reqmac[3])
        || (localmac[4] != reqmac[4])
        || (localmac[5] != reqmac[5]))
        && (0 != memcmp(reqmac, zero, 6)))
    {
        return OPER_FAILURE;
    }
    else
    {
        return OPER_SUCCESS;
    }
}

/*------------------------------------------------------------
  函数原型: int multi_upgrade_nego(int udp_fd,unsigned char *buf,
            int len, struct sockaddr_in* client_udp_addr )
  描述:     装备升级协商的主程序。分析udp中的hdlc格式命令，执行相应动作，报告执行情况
  输入:     udp报文payload，和udp端口描述符以及地址；
  输出:     无
  返回值:   0: 表示当前UDP 包是多端口升级数据包，-1 : 表示虚拟串口的数据包
-------------------------------------------------------------*/
int multi_upgrade_nego(int udp_fd,unsigned char *buf,int len, 
    struct sockaddr_in* client_udp_addr )
{
    static nego_state  nego_state = EQUIP_INVALID;

    /*保存从nv中取得的ip/mac等*/
    s_report_data     report_buf;
    s_report_data     struct_report;
    int   is_equip_pkg = -1;
    struct stat s_stat;
    int   reported = 0;
    int   index  = -1;

    if (NULL == buf || NULL == client_udp_addr)
    {
        syslog("para error!");
        return -1;
    }

    /*parse_net_package return machine state value*/
    nego_state = parse_nego_cmd(buf, len,(unsigned char*)&report_buf);
    if( EQUIP_INVALID != nego_state )
    {        
        is_equip_pkg = 0;
    }
    else
    {
        return -1;
    }
    
    /*检查report_buf中的mac地址，判断是否为自己的mac或空，如果是才改变ip，否则返回*/
    scratch_ip_mac((char*)&struct_report);
    if(OPER_FAILURE == mac_match(struct_report.mac, report_buf.mac))
    {
        syslog("Is not my Mac address, do not change anything");
        return -1;
    }

    g_i_upgrade = EQUIP_RUN;
    switch (nego_state)
    {
        case EQUIP_IP_MAC_REQUERY:
            DEBUGPRINT("Enter EQUIP_IP_MAC_REQUERY\n");
            /*获取软件版本*/
            index = scratch_ip_mac( (char*)&report_buf );
            if (OPER_FAILURE == index)
            {
                syslog("Multi upgrad: mac scratch failed!");
                break;
            }
            break;
			
#ifdef ORG_DIAG
        case EQUIP_IP_CHANGE:                    
            DEBUGPRINT("Enter EQUIP_IP_CHANGE \n");
            //修改ip地址之后，不能通过udp_fd向pc报告。这里先报告，后修改ip    
            if( OPER_SUCCESS != report_to_pc(udp_fd, (char*)&report_buf, client_udp_addr))
            {
                syslog("Run Udp command %d failed!", nego_state);
            }
            else
            {
                syslog("Run Udp command %d success!", nego_state);
            }
            reported = 1;
            if (OPER_SUCCESS == ip_change((char*)&report_buf))
            {
                syslog("Multi upgrade: ip change success");
            }
            else
            {
                syslog("impossible, ip change failed");
            }
            break;
			

        case EQUIP_IP_RESTORE:
            DEBUGPRINT(" enter EQUIP_IP_RESTORE\n");
            /*收到IP请求包以后先恢复再上报*/
            ip_restore();

            /*获取ip和mac地址*/
            scratch_ip_mac((char*)&report_buf);

            /*获取软件版本*/
            memset (report_buf.version, 0, sizeof (report_buf.version));
            break;
#endif

        case EQUIP_RESTART_DIAGD:
             kill( getpid(), SIGTERM );            
            break;
			
#ifdef ORG_DIAG
        case EQUIP_REBOOT_GW:
             if ( OPER_SUCCESS != report_to_pc(udp_fd, (char*)&report_buf, client_udp_addr) )
             {
                syslog("Multi upgrade: reboot report failed");
             }
             else
             {
                syslog("Multi upgrade: reboot report success");
             }
             reported = 1;
			 
/* <BB5D00010.VOICE: yangjianping 2008-8-4 MODIFY BEGIN */
            BSP_SYS_SoftReboot();
/* BB5D00010.VOICE: yangjianping 2008-8-4 MODIFY END> */
		break;
#endif
        case EQUIP_RESTORE_GW:
					/*恢复NV*/
					/*system( "restoredefault" );     */
            break;
					
#ifdef ORG_DIAG
        case EQUIP_ENFORCE_DOWN:

				/*调用函数,拉高GPIO脚,使得无线模块处于强制升级状态*/
				BSP_MSM_Force_On();

				/*拉高GPIO脚,重新启动无线模块;*/
				BSP_MSM_Off();
				__msleep(1000);
				BSP_MSM_On();
    
            break;

        case EQUIP_EXIT_ENFORCE_DOWN:
            /*调用函数,拉高GPIO脚,使得无线模块处于退出强制升级状态*/
            //sb_enforce_down();
            BSP_MSM_Force_Off();
            
            break;

        case EQUIP_RESTART_MSM:
			/*拉高GPIO脚,重新启动无线模块;*/
			BSP_MSM_Off();
			__msleep(1000);
			BSP_MSM_On();
            break;
#endif
        case EQUIP_DIAG_STATUS:
            if (stat ( DIAG_PORT, &s_stat) == -1) 
            { 
                syslog( "usb diag port is down!" ); 
                report_buf.reserved[0] = 1;
            }
            else
            {
                syslog( "usb diag port is exist!" ); 
                report_buf.reserved[0] = 0;
            }
            break;
			
#ifdef ORG_DIAG            
        case EQUIP_DOWNLOAD:
            close_pcui_shake();
            break;
#endif

        default:
            DEBUGPRINT(" enter default\n");
            break;
    }

    if( 1 != reported )
    {        
        if( OPER_SUCCESS != report_to_pc(udp_fd, (char*)&report_buf, client_udp_addr))
        {
            syslog("Run Udp command %d failed!", nego_state);
        }
        else
        {
            syslog("Run Udp command %d success!", nego_state);
        }
    }
    
    return is_equip_pkg;
}

/*------------------------------------------------------------
  函数原型: void gen_hdlc_packet(char* payload,long size,char cmdWord,char* hdlc)
  描述:     产生hdlc报文，计算crc，转义payload中的0x7e和0x7d字节。
  输入:     hdlc报文payload
  输出:     char* hdlc中的报文
  返回值:   参数正确，返回hdlc报文长度，转义之后，有可能比变得比较长
            参数错误，返回-1
-------------------------------------------------------------*/
long gen_hdlc_packet(char* payload,long size,char* hdlc)
{
    long off=0;
    /* CRC in its initial state */
    unsigned short  crc = CRC_16_L_SEED;

    char one_char=0;

    if (NULL == payload || NULL == hdlc)
    {
        syslog("para error!");
        return -1;
    }

    hdlc[0] = ASYNC_HDLC_FLAG;
    hdlc++;

    long tmp = 0;
    for (off = 0, tmp = 0; off < size; off++)
    {
        one_char = *( payload + off );

        if (ASYNC_HDLC_FLAG == one_char
          || ASYNC_HDLC_ESC == one_char)/*need translate*/
        {
            hdlc[tmp++] = ASYNC_HDLC_ESC;
            hdlc[tmp++] = one_char ^ ASYNC_HDLC_ESC_MASK;
            crc = CRC_16_L_STEP (crc, (unsigned short)one_char);/* Update the CRC     */
        }
        else
        {
            hdlc[tmp++] = one_char;
            crc = CRC_16_L_STEP (crc, (unsigned short)one_char);/* Update the CRC     */
        }
    } 
    off = tmp;

    crc = ~crc;
    unsigned char halfcrc = crc & 0xFF;
    
    if (ASYNC_HDLC_FLAG == halfcrc
      || ASYNC_HDLC_ESC == halfcrc)/*need translate*/
    {
        hdlc[off++] = ASYNC_HDLC_ESC;
        hdlc[off++] = halfcrc ^ ASYNC_HDLC_ESC_MASK;
    }
    else
    {
        hdlc[off++] = halfcrc;
    }
    
    halfcrc = crc >> 8;
    
    if (ASYNC_HDLC_FLAG == halfcrc
      || ASYNC_HDLC_ESC == halfcrc)/*need translate*/
    {
        hdlc[off++] = ASYNC_HDLC_ESC;
        hdlc[off++] = halfcrc ^ ASYNC_HDLC_ESC_MASK;
    }
    else
    {
        hdlc[off++] = halfcrc;
    }
        
    hdlc[off] = ASYNC_HDLC_FLAG;

    return off+2;//hdlc的指针自加了1，所以其大小为off+2
}

/*------------------------------------------------------------
  函数原型: char parse_tcp_payload (unsigned char *buf,long len, 
  描述:     解析tcp中封装的一个hdlc报文
  输入:     需要parse的tcp payload
  输出:     解析出的合法hdlc payload
  返回值:   获得了一个完整的hdlc payload 返回OPER_SUCCESS；
            参数错误、到达buf末尾却没有获得一个完整的hdlc payload，返回OPER_FAILURE
-------------------------------------------------------------*/
int parse_tcp_payload (unsigned char *buf,long len, 
    unsigned char* out_buf,long* out_len)
{
    unsigned char  chr;
    unsigned short crc = 0;
    hdlc_state state = HDLC_HUNT_FOR_FLAG;
    long off=0;
    if ( len == 0 )
    {
        DEBUGPRINT("no data!\n");
        return OPER_FAILURE;        
    }
    
    if ((NULL == buf) || (NULL == out_buf) || (NULL == out_len))
    {
        assert("para error\n");
        return OPER_FAILURE;
    }
    
    //reset out len,avoid caller maintain this
    *out_len = 0;
    
    DEBUGPRINT("enter\n");

    /* Look at characters and try to find a valid async-HDLC packet of
    ** length at least MIN_PACKET_LEN with a valid CRC.
    ** Keep looking until we find one, or to the end of buf. */
    for (state = HDLC_HUNT_FOR_FLAG; state != HDLC_PACKET_RCVD; /* nil */)
    {
        chr=buf[off++];/*get a char,off value increment*/

        switch( state )                  /* Process according to which state */
        {
            /*lint -esym(788,HDLC_PACKET_RCVD)  No need to deal with HDLC_PACKET_RCVD
            since we're in a loop that guarantees we're not in that state. */
            case HDLC_HUNT_FOR_FLAG:            /* We're looking for a flag ... */
                if (chr == ASYNC_HDLC_FLAG)     /*   and we got one ...         */
                {
                    state = HDLC_GOT_FLAG;      /*   so go on to the next step. */
                }
                break;

            case HDLC_GOT_FLAG:                   /* Had a flag, now expect a packet */
                if (chr == ASYNC_HDLC_FLAG)    /* Oops, another flag.  No change. */
                {
                    break;
                }
                else
                {                                                    /* Ah, we can really begin a packet */
                    crc = CRC_16_L_SEED;               /* and the CRC in its initial state */
                    state = HDLC_GATHER;               /* and we begin to gather a packet  */
                }
            /*pass through*/
            case HDLC_GATHER:                         /* We're gathering a packet      */
                if (chr == ASYNC_HDLC_FLAG)       /* We've reached the end         */
                {
                    if (*out_len < MIN_PACKET_LEN)        /* Reject any too-short packets  */
                    {
                        state = HDLC_HUNT_FOR_FLAG;           /* Start over     */
                    }
                    else  /* Yay, it's a good packet! */
                    {
                        DEBUGPRINT("parse ok\n");
                        state = HDLC_PACKET_RCVD;                    /* Done for now   */
                        return OPER_SUCCESS;     /*return 0: get complete hdlc payload */
                    }
                    break;           /* However it turned out, this packet is over.  */
                }

                /* It wasn't a flag, so we're still inside the packet. */
                if (chr == ASYNC_HDLC_ESC)               /* If it was an ESC       */
                {
                    chr=buf[off++];/*get next char,off value increment*/

                    chr ^= ASYNC_HDLC_ESC_MASK;       /* de-mask it  */
                    
                    out_buf[ (*out_len)++ ] = chr;                    /* Add byte to out_buf */

                    crc = CRC_16_L_STEP (crc, (unsigned short) chr);      /* Update the CRC     */
                    /* No break; process the de-masked byte normally */
                }
                else
                {
                    out_buf[ (*out_len)++ ] = chr; 
                    crc = CRC_16_L_STEP (crc, (unsigned short) chr);      /* Update the CRC     */
                }
                break;

            default:       /* Shouldn't happen with an enum, but for safety ...  */
                //DEBUGPRINT("enter default\n");
                state = HDLC_HUNT_FOR_FLAG;             /* Start over         */
                break;
        }/* switch on state */
        if  ( off >= len )
        {
            return OPER_FAILURE;/*reach to the end of buf*/
        }
    }
  
    return OPER_FAILURE;/*return uncomplete*/
} /* parse_tcp_payload() */

/*------------------------------------------------------------
  函数原型: char parse_udp_payload (unsigned char *buf,long len, 
  描述:     解析udp中封装的一个hdlc报文
  输入:     需要parse的udp payload
  输出:     解析出的合法hdlc payload
  返回值:   获得了一个完整的hdlc payload 返回OPER_SUCCESS；
            参数错误、到达buf末尾却没有获得一个完整的hdlc payload，返回OPER_FAILURE
-------------------------------------------------------------*/
long parse_udp_payload (unsigned char *buf,long len, 
    unsigned char* out_buf,long* out_len)
{
    unsigned char  chr;
    unsigned short crc = 0;
    hdlc_state state = HDLC_HUNT_FOR_FLAG;
    long off=0;
    
    if ((NULL == buf) || (NULL == out_buf) || (NULL == out_len))
    {
        syslog("para error\n");
        return OPER_FAILURE;
    }

    /* Look at characters and try to find a valid async-HDLC packet of
    ** length at least MIN_PACKET_LEN with a valid CRC.
    ** Keep looking until we find one, or to the end of buf. */
    for (state = HDLC_HUNT_FOR_FLAG; state != HDLC_PACKET_RCVD; /* nil */)
    {
        chr=buf[off++];/*get a char,off value increment*/

        switch( state )                  /* Process according to which state */
        {
            /*lint -esym(788,HDLC_PACKET_RCVD)  No need to deal with HDLC_PACKET_RCVD
            since we're in a loop that guarantees we're not in that state. */
            case HDLC_HUNT_FOR_FLAG:            /* We're looking for a flag ... */
                if (chr == ASYNC_HDLC_FLAG)     /*   and we got one ...         */
                {
                    state = HDLC_GOT_FLAG;      /*   so go on to the next step. */
                }
                break;

            case HDLC_GOT_FLAG:                   /* Had a flag, now expect a packet */
                if (chr == ASYNC_HDLC_FLAG)    /* Oops, another flag.  No change. */
                {
                    break;
                }
                else
                {                                                    /* Ah, we can really begin a packet */
                    crc = CRC_16_L_SEED;               /* and the CRC in its initial state */
                    state = HDLC_GATHER;               /* and we begin to gather a packet  */
                }
            /*pass through*/
            case HDLC_GATHER:                         /* We're gathering a packet      */
                if (chr == ASYNC_HDLC_FLAG)       /* We've reached the end         */
                {
                    if (*out_len < MIN_PACKET_LEN)        /* Reject any too-short packets  */
                    {
                        state = HDLC_HUNT_FOR_FLAG;           /* Start over     */
                    }
                    /*because reuse udp port, must crc check*/
                    else if (crc != CRC_16_L_OK_NEG)          /* Reject any with bad CRC */
                    {
                        state = HDLC_HUNT_FOR_FLAG;           /* Start over     */
                    }
                    else  /* Yay, it's a good packet! */
                    {
                        state = HDLC_PACKET_RCVD;                    /* Done for now   */
                        return OPER_SUCCESS;     /*return 0: get complete hdlc payload */
                    }
                    break;           /* However it turned out, this packet is over.  */
                }

                /* It wasn't a flag, so we're still inside the packet. */
                if (chr == ASYNC_HDLC_ESC)               /* If it was an ESC       */
                {
                    chr=buf[off++];/*get next char,off value increment*/

                    chr ^= ASYNC_HDLC_ESC_MASK;       /* de-mask it  */
                    
                    out_buf[ (*out_len)++ ] = chr;                    /* Add byte to out_buf */

                    crc = CRC_16_L_STEP (crc, (unsigned short) chr);      /* Update the CRC     */
                    /* No break; process the de-masked byte normally */
                }
                else
                {
                    out_buf[ (*out_len)++ ] = chr; 
                    crc = CRC_16_L_STEP (crc, (unsigned short) chr);      /* Update the CRC     */
                }
                break;

            default:       /* Shouldn't happen with an enum, but for safety ...  */
                //DEBUGPRINT("enter default\n");
                state = HDLC_HUNT_FOR_FLAG;             /* Start over         */
                break;
        }/* switch on state */
        if  ( off >= len )
        {
            return OPER_FAILURE;/*reach to the end of buf*/
        }
    }
  
    return OPER_FAILURE;/*return uncomplete*/
}

/*------------------------------------------------------------
  函数原型: int usb_init(void)
  描述:     初始化usb端口
  输入:     无
  输出:     无
  返回值:   成功返回OPER_SUCCESS,失败返回OPER_FAILURE
-------------------------------------------------------------*/
static int usb_init(void)
{
    struct stat s_stat;

    if (-1 != g_usbfd)
    {
        close(g_usbfd);
    }

    int g_wait_usb = 0;
    do
    {
        if (( stat ( DIAG_PORT, &s_stat) == -1) 
            || (open_usb_port() != OPER_SUCCESS))
        { 
            syslog( "Diag is Not exist !" ); 

            __msleep(2000);
            if ( ++g_wait_usb > 2)
            {
                syslog( "can not open diag after 4s !" );
                return OPER_FAILURE;
            }
        }
        else
        {
            break;
        }
    }while(1);
    
    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型 : void diagd_hup_signal_exception(int sig)
  描述     : client发送信号处理函数
  输入     : sig(信号编号); 
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void diagd_hup_signal_exception(int sig)
{
     syslog("Receive signal = [%d]", sig);
	  close( g_usbfd );
	  g_usbfd = -1;
}

/*------------------------------------------------------------
  函数原型: int bifurcate(void)
  描述:     创建d2n子进程
  输入:     pold_timer进程原始timer
  输出:     成功后，输出fork返回值
  返回值:   成功 : OPER_SUCCESS
            失败 : OPER_FAILURE
-------------------------------------------------------------*/
static int bifurcate(pid_t* ppid, struct itimerval* pold_timer)
{
    *ppid = fork();
    
    if (*ppid == -1) //创建d2n子进程不成功
    {
        syslog ("Create sub procedure failure");
					 __msleep(4000);
        exit(0);
    }
    else if( *ppid == 0 )//子进程，进入D2N_LOOP_S状态
    {
        signal (SIGCHLD,  sub_coordinate);
        signal (SIGPIPE,   sub_coordinate);
        signal (SIGTERM,  sub_coordinate);
        signal (SIGUSR1,  sub_coordinate);
		 signal(SIGHUP,    diagd_hup_signal_exception);

        if (setitimer (ITIMER_REAL, pold_timer, NULL))
        {
            syslog ("Can't cancel the timer!");
        }

        syslog ("Start sub branch transfer data from Usb to Net!");
    }
    else //父进程，调用者决定进入那个状态
    { 
        syslog ("Main branch fork sub %d OK!",*ppid);
    }

    DEBUGPRINT(" BIFURCATE_S OK\n");

    return OPER_SUCCESS;
}

/*------------------------------------------------------------
  函数原型: void exit_function(void)
  描述:     进程退出时，关闭打开资源
  输入:     无
  输出:     无
  返回值:   无
-------------------------------------------------------------*/
void exit_function(void)
{
    DEBUGPRINT("PID=%d exit\n",getpid());
    DEBUGPRINT("listen_fd=%d netfd=%d usbfd=%d\n",g_listen_fd,g_netfd,g_usbfd);
    if ( -1 != g_usbfd )
    {
        close(g_usbfd);
        g_usbfd = -1;
    }

    if ( -1 != g_netfd )
    {
        close(g_netfd);
        g_netfd = -1;
    }

    if ( -1 != g_listen_fd )
    {
        close(g_listen_fd);
        g_listen_fd = -1;
    }
    if ( -1 != g_logsys )
    {
        close(g_logsys);
        g_logsys = -1;
    }
    if( g_main_pid == getpid())
    {
        if( -1 != g_udp_pid)
        {
            kill( g_udp_pid, SIGTERM );
            g_udp_pid = -1;
        }
        if( -1 != g_sub_pid)
        {
            kill( g_sub_pid, SIGTERM );
            g_sub_pid = -1;
        }
    }
}





/*------------------------------------------------------------
  函数原型 : void diagd_signal_exception(int sig)
  描述     : client发送信号处理函数
  输入     : sig(信号编号); 
  输出     : 无
  返回值   : 无
-------------------------------------------------------------*/
void diagd_signal_exception(int sig)
{
     syslog("Receive signal = [%d]", sig);
}


/*------------------------------------------------------------
  函数原型 : void diagd_server_bad_signal(int signum)
  描述     : 大多数错误信号处理函数
  输入     : signum 信号编号
  输出     : g_child_exited 设置为AT_CHLD_EXITED
  返回值   : 无
-------------------------------------------------------------*/
void diagd_server_bad_signal(int sig)
{
	syslog("Receive signal = [%d]", sig);
		 __msleep(4000);
	exit(1);
}

/*------------------------------------------------------------
  函数原型: void at_setup_sig()
  描述    : 安装信号及信号处理函数
  输入    : 命令行参数
  输出    : 无
  返回值  : 无
-------------------------------------------------------------*/
void diagd_setup_sig(void)
{
    struct sigaction sa;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);

    sa.sa_mask = mask;
    sa.sa_flags = 0;
    
	SIGNAL(SIGHUP,    diagd_hup_signal_exception);
	SIGNAL(SIGINT,    diagd_signal_exception);
	SIGNAL(SIGABRT,   diagd_server_bad_signal);
	SIGNAL(SIGFPE,    diagd_server_bad_signal);
	SIGNAL(SIGILL,    diagd_server_bad_signal);
	SIGNAL(SIGQUIT,   diagd_server_bad_signal);
	SIGNAL(SIGSEGV,   diagd_signal_exception);
	SIGNAL(SIGBUS,    diagd_server_bad_signal);    
//	SIGNAL(SIGEMT,    diagd_server_bad_signal);
	SIGNAL(SIGPOLL,   diagd_server_bad_signal);
	SIGNAL(SIGPROF,   diagd_server_bad_signal);
	SIGNAL(SIGSYS,    diagd_server_bad_signal);
	SIGNAL(SIGTRAP,   diagd_server_bad_signal);
	SIGNAL(SIGVTALRM, diagd_server_bad_signal);
	SIGNAL(SIGXCPU,   diagd_server_bad_signal);
	SIGNAL(SIGXFSZ,   diagd_server_bad_signal);
	SIGNAL(SIGALRM,  NULL);
	SIGNAL(SIGPIPE,  main_coordinate);
	SIGNAL(SIGTERM,  main_coordinate);
	SIGNAL(SIGCHLD,  main_coordinate);
	SIGNAL(SIGUSR1,  main_coordinate);
	SIGNAL(SIGCONT, NULL);
	
    int i = SIGRTMIN;
    for(; i < SIGRTMAX; i++)
    {
        SIGNAL(i, diagd_signal_exception);
    }
}



/*------------------------------------------------------------
  函数原型: int main(int argc, char **argv)
  描述:  实现Diag数据透传,路由器模块升级。
  输入:  无
  输出:  无
  返回值: 成功返回0，失败返回非0
-------------------------------------------------------------*/
int main (int argc, char **argv)
{
    int    retval = 0;
    pid_t  d2n_pid = 0, udp_pid=0;
    long   rcv_len = 0, payload_len = 0;
    long   bcm_img_len = 0,bcm_img_left=0;//bcm映象总长和剩余长度
    long   upgrade_history = 0;//记录升级历史，路由器模块升级成功后可能需要干别的事后重起系统
    
    char   rcv_buf[BUFFER_LEN];
    char   payload_buf[BUFFER_LEN];
    usockaddr uSockaddr_tcp;
  //  EN_UPGRADE_ERRCODE
    int err = 0;
    int  usb_state = OPER_FAILURE;/*usb not open*/
    int  bifurcate_state = OPER_FAILURE;/*是否创建d2n子进程*/    
    sigset_t mask;
    struct sigaction sigact, old_alarm_handler;
    struct itimerval timerv,old_timer;

    char* imageTemp = NULL ;

    char* imageNode = NULL ;
    int   nBcmMolloc = -1;
    pid_t parent_pid = 0;
    
    g_state = TCP_INIT_S;

#ifndef ORG_DIAG
    FILE* fsNv = NULL;
#endif

    if (( g_logsys = open(_DIAG_LOG, O_RDWR|O_CREAT, 644)) < 0)
    {
        exit(0);
    }

    if (diag_parse_lines(argc, argv) != OPER_SUCCESS)
    {
        exit(0);
    }
    
    if (g_ePort == _DIAG)
    {
        if (diag_daemon(1, 1) != OPER_SUCCESS)
        {
            syslog ( "can't establish daemon program");
            exit(errno);
        }
    }

    if ( 0 == atexit(exit_function))
    {
        syslog ("exit func regist ok");
    }


	diagd_setup_sig();


#ifdef ORG_DIAG
    nvram_unset( EXPECT_UDP_PORT );
#else
    fsNv = fopen(NV_FILE, "w+");
    if (fsNv)
    {
    	fclose(fsNv);
	fsNv = NULL;
    }
#endif

	
    /*get current pid*/
    parent_pid = getpid ();
    udp_pid = fork();
    if (udp_pid == -1)
    {
        syslog ("Create sub procedure failure");
        exit(0);
    }
    else if( udp_pid == 0 )
    {
        udp_server_main(parent_pid);
        syslog ("impossible,udp_server_main go here!");     
        exit(0);
    }
    else
    { 
        g_udp_pid   = udp_pid;
        g_main_pid = parent_pid;
        syslog ("Main branch continue execute!");                    
    }

    sigact.sa_handler = tcp_timeout;
    sigact.sa_flags = 0;
    memset((void *)&sigact.sa_mask, 0, sizeof(sigset_t));
    sigemptyset(&mask);
    sigaddset (&mask, SIGALRM);
    sigact.sa_mask = mask;
    sigaction(SIGALRM, &sigact, &old_alarm_handler);
    
    timerv.it_interval.tv_sec = _interval;
    timerv.it_interval.tv_usec = 0;
    timerv.it_value = timerv.it_interval; 
    if (setitimer (ITIMER_REAL, &timerv, &old_timer) != 0)
    {
        exit (0);
    }

    do
    {
        switch( g_state )
        {
            case TCP_INIT_S:

                if ((g_listen_fd = diag_init_tcp_listen_socket(&uSockaddr_tcp)) < 0)
                {
                    sleep(2);
                    syslog ( "Tcp initialize failure");
                    DEBUGPRINT(" err=%d\n",errno);

                    if ( 0 < udp_pid )
                    {
                        //杀死udp server进程
                        kill (udp_pid, SIGTERM);
                        waitpid(udp_pid,&retval,0);
                        udp_pid = 0;
                    }
                    exit(0);
                }
                syslog ( "Tcp initialize successful");
                g_state = WAIT_CONNECT_S;
                break;
                
            case WAIT_CONNECT_S: 
                g_i_upgrade = NOT_UPGRADE;
                g_net_alive = NET_TIMEOUT;
                g_disconnect = NOT_DISCONNET;
                
                //杀死D2N透传子进程
                if ( 0 < d2n_pid)
                {
                    syslog ("kill d2n_pid=%d ",d2n_pid);
                    kill(d2n_pid, SIGTERM);
                    waitpid(d2n_pid,&retval,0);
                    d2n_pid = 0;
                    bifurcate_state = OPER_FAILURE;
                }

                /*关闭虚拟串口*/
                if ( -1 != g_netfd )
                {
                    close(g_netfd);
                    g_netfd = -1;
                }
                
                //关闭usb口
                if ( -1 != g_usbfd )
                {
                    close(g_usbfd);
                    g_usbfd = -1;
                    usb_state = OPER_FAILURE;
                }
                
                //初始化全局套接字描述符g_netfd
                if (diag_accept_connect ( g_listen_fd ) == OPER_SUCCESS)
                {
                    g_state = RECV_S;/*jump to recv packet*/
                    syslog ("Client access successful");
                }
                break;

            case RECV_S:      
                /*receive TCP packet*/
                rcv_len = recv(g_netfd, rcv_buf, BUFFER_LEN-1, 0);
                /*check if Virtual console request to disconnect tcp*/
                if (TCP_TIMEOUT == g_disconnect)
                {
                    g_disconnect = NOT_DISCONNET;
                    
                    if( NOT_UPGRADE == g_i_upgrade && 0 != upgrade_history )
                    {
                        DEBUGPRINT("In upgrade, Jump to TCP_CON_CLOSE_S\n");
                        g_state = TCP_CON_CLOSE_S;
                        break;
                    }
                    
                    DEBUGPRINT("Not in upgrade, Jump to WAIT_CONNECT_S\n");
                    g_state = WAIT_CONNECT_S;
                    break;
                }
                else if (UDP_NOTIFY_DISCONNECT == g_disconnect)//udp notify
                {
                    g_disconnect = NOT_DISCONNET;
                    
                    if ( NOT_UPGRADE != g_i_upgrade || 0 != upgrade_history )
                    {
                        syslog("In upgrading, ignore udp notification");
                        break;
                    }
                    
                    DEBUGPRINT("Not in upgrade,Jump to WAIT_CONNECT_S\n");
                    g_state = WAIT_CONNECT_S;
                    break;
                }

                /*check if sig_pipe(Tcp_connect_close signal) request to reboot all*/
                if (1 == g_reboot_all)
                {
                    g_state = TCP_CON_CLOSE_S;
                    break;
                }        

                //如果是信号打断，继续接受    
                if ( rcv_len == SOCKET_ERROR && errno == EINTR )
                {
                    break;/*continue to recv*/
                }

                if ( rcv_len > BUFFER_LEN - 1 )
                {
                    syslog ("The recv data too long!");
                    break;
                }
                else if ( rcv_len <= 0 )//不是信号打断的错误 
                {
                    if ( BCM_UPGRADE == g_i_upgrade )
                    {
                        syslog("In upgrading, ignore tcp disconnect notification");
                        g_state = WAIT_CONNECT_S;
                        break;
                     }

                    syslog ( "The peer close socket!");
                    g_state = TCP_CON_CLOSE_S;
                    break;
                }
                else /*recv good data,keep alive*/
                {
                    g_net_alive = NET_TIMEOUT;
                }

                //判断是否升级，决定转移状态
                if (NOT_UPGRADE == g_i_upgrade)
                {
                    g_state = PARSE_S;
                }
                else if (BCM_UPGRADE == g_i_upgrade)
                {
                    g_state = WRITE_BCM_S;
                }                      
                else if (MSM_UPGRADE == g_i_upgrade)
                {
                    g_state = N2D_LOOP_S;
                }
                else
                {
                    ;/*empty sentence*/
                }
                break;
              
            case PARSE_S:
                DEBUGPRINT(" PARSE_S\n");
                retval = parse_tcp_payload(rcv_buf,rcv_len, 
                  payload_buf, &payload_len);

                if(OPER_SUCCESS != retval)/*don't get a complete hdlc payload*/
                {
                    g_state = N2D_LOOP_S;
                    break;
                }


                /*分析命令字*/    
                g_i_upgrade = analyse_cmd(payload_buf,&bcm_img_len);
                assert (-1 != g_i_upgrade);/*impossible situation*/



                if ( NOT_UPGRADE == g_i_upgrade )
                {
                    g_state = N2D_LOOP_S;
                }
                else if (BCM_UPGRADE == g_i_upgrade)
                {
                    /*initialize bcm_img_left before write bcm flash*/
                    bcm_img_left = bcm_img_len;
                    close_pcui_shake();

                    //先去WRITE_BCM_S状态，建立跟mtd_write的管道等
                    g_state = WRITE_BCM_S;
                }                      
                else /*MSC_UPGRADE or MSM_FORCE_UPGRADE*/
                {
                    syslog ("start upgrade msm!");
                    close_pcui_shake();
                    
                    //去等待MSM重启，然后初始化usb口
                    g_state = FORCE_MSM_S;
                }
                
                break;
            
            case FORCE_MSM_S:
							
                if ( -1 != g_usbfd)
                {
                    close(g_usbfd);
                    g_usbfd = -1;
                }
                
#ifdef ORG_DIAG                 
                if (MSM_FORCE_UPGRADE == g_i_upgrade)
                {
                     syslog("Force MSM reboot");
                   
						/*调用函数,拉高GPIO脚,使得无线模块处于强制升级状态*/
						syslog("BSP_MSM_Force_On");
						BSP_MSM_Force_On();


						/*拉高GPIO脚,重新启动无线模块;*/
						syslog("BSP_MSM_On");
						BSP_MSM_Off();
						
						__msleep(1000);
						syslog("BSP_MSM_Off");
			
						BSP_MSM_On();

                    upgrade_history |= 8;//记录强制升级过
                    //杀死D2N透传子进程
                    /*if ( 0 < d2n_pid)
                    {
                        syslog( "kill d2n" );
                        kill(d2n_pid,SIGTERM);
                        waitpid(d2n_pid,&retval,0);
                        bifurcate_state = OPER_FAILURE;
                        d2n_pid = 0;
                    }
                    */
                    syslog("Wait MSM be ready");    
                    g_waitMsmTimeout = 0;//开始计时
                    while (1)//等待无线模块重新启动
                    {
                        pause();
                        if ( errno == EINTR && 2 == g_waitMsmTimeout )//等待msm 2*1=2秒
                        {
                            break;
                        }
                    }
                }

                /*next is same,so set to MSM_UPGRADE*/
                g_i_upgrade = MSM_UPGRADE;
#endif
                g_state = RECV_S;

                //记录升级过MSM
                upgrade_history |= 4;
                break;
            case N2D_LOOP_S:
                if (OPER_SUCCESS != usb_state)/*usb state not ok*/
                {
                    syslog ("usb is init!");
                    usb_state=usb_init();
                    
                    //初始化usb不成功
                    if (OPER_SUCCESS != usb_state)
                    {
                        g_state = RECV_S;
                        break;
                    }
                }
                
                 if ( OPER_FAILURE == bifurcate_state )
                {
                    bifurcate(&d2n_pid,&old_timer);
                    bifurcate_state = OPER_SUCCESS;      
                    syslog ("bifurcate!");
                    if (0 == d2n_pid)//D2N子进程
                    {
                        DEBUGPRINT(" D2N_LOOP_S\n");
                        /*子进程读取DIAG口数据转发到网口*/
                        diag_to_net(); 
                        syslog ("impossible! D2N sub proc return\n");
                        exit(0);/*sub progress exit*/
                    }
                    g_sub_pid = d2n_pid;
                }
                
                /*主进程读取网口数据转发到DIAG口*/
                retval = net_to_diag(rcv_buf,rcv_len);
                if (retval == OPER_FAILURE)
                {
                    syslog ("usb dev is down!");
                    DEBUGPRINT(" N2D failure\n");
                    //直接返回WAIT_CONNECT_S状态，在那关闭打开的资源
                    g_state = WAIT_CONNECT_S;
                    break;
                }
                DEBUGPRINT(" N2D OK\n");

                g_state = RECV_S;
                break;
				
#ifdef ORG_DIAG
            case WRITE_BCM_S:
					if( -1 == nBcmMolloc)
					{
						syslog("BCM begin!");
						imageNode =  malloc(bcm_img_len);
						imageTemp = imageNode;
						bcm_img_left = bcm_img_len;
						nBcmMolloc = 0;
						g_state = RECV_S;
                     			break;
					}
	          	  
					if(bcm_img_left>0)
					{
						memcpy(imageTemp,rcv_buf, rcv_len);
						imageTemp = imageTemp +rcv_len;
						bcm_img_left = bcm_img_left- rcv_len;
						
						if(0 == bcm_img_left)
						{
							err = upgrade_bcm_image(imageNode,bcm_img_len);
						}
						else
						{
							g_state = RECV_S;
							break;
						}
					}


                if ((0 == bcm_img_left) && (0 == err))
                {
                    syslog ("upgrade router success");
                    report_upgrade_result(g_netfd,OPER_SUCCESS);
                    nvram_set("is_upgrade_restore", "1");
                    nvram_commit();

                    //升级成功清掉g_i_upgrade，设置曾经升级BCM成功
                    g_i_upgrade = NOT_UPGRADE;
                    upgrade_history |= 1;
                }
                else if (0 != err)
                {
                    report_upgrade_result(g_netfd,OPER_FAILURE);
                    
                    //升级有错，清掉g_i_upgrade,设置曾经升级BCM失败
                    g_i_upgrade = NOT_UPGRADE;
                    upgrade_history |= 2;
                }
                
                g_state = RECV_S;
                break;
#endif

            case TCP_CON_CLOSE_S:
                DEBUGPRINT(" TCP_CON_CLOSE_S\n");
                
                //g_reboot_all为1重启;目前没有升级或者曾经也没有升级过，不重启系统
                if ((g_reboot_all==0) && (NOT_UPGRADE==g_i_upgrade) 
                    && (0==upgrade_history))
                {
                    g_state = WAIT_CONNECT_S;
                    break;
                }

                g_reboot_all = 0;

                /*now, in upgrade state*/
                if ( -1 != g_usbfd )
                {
                    close (g_usbfd);
                    g_usbfd = -1;
                    usb_state == OPER_FAILURE;
                }
                if ( -1 != g_netfd )
                {
                    close (g_netfd);
                    g_netfd = -1;
                }
#ifdef ORG_DIAG
                DEBUGPRINT("Reboot all system\n");
                //强制升级过，恢复强制升级GPIO，否则，无线模块启动还滞留在boot中
                if ( 0x8 & upgrade_history ) 
                {
                    BSP_MSM_Force_Off();
                }

                /*拉高GPIO脚,重新启动无线模块;*/
                BSP_MSM_Off();
               __msleep(1000);
                BSP_MSM_On();
                
                /*reboot BCM*/
/* <BB5D00010.VOICE: yangjianping 2008-8-4 MODIFY BEGIN */
                BSP_SYS_SoftReboot();
/* BB5D00010.VOICE: yangjianping 2008-8-4 MODIFY END> */
#endif
                break;
                
           default:
                DEBUGPRINT(" abnormally enter here\n");
                exit(0);
                break;
        }

    }while(1);

    return 0;
}

