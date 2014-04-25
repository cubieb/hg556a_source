/*
<:copyright-gpl 
 Copyright 2002 Broadcom Corp. All Rights Reserved. 
 
 This program is free software; you can distribute it and/or modify it 
 under the terms of the GNU General Public License (Version 2) as 
 published by the Free Software Foundation. 
 
 This program is distributed in the hope it will be useful, but WITHOUT 
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details. 
 
 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA. 
:>
*/
/***************************************************************************
 * File Name  : board.c
 *
 * Description: This file contains Linux character device driver entry 
 *              for the board related ioctl calls: flash, get free kernel
 *              page and dump kernel memory, etc.
 *
 *1.增加FLASH VA分区by l39225 2006-5-8
  2.增加对RESET BUTTON的处理by l39225 2006-5-8
 ***************************************************************************/
/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <bcm_map_part.h>
#include <board.h>
#include <bcmTag.h>
#include "boardparms.h"
#include "flash_api.h"
#include "bcm_intr.h"
#include "board.h"
#include "bcm_map_part.h"
/* start of maintain dying gasp by liuzhijie 00028714 2006年5月9日" */
#include <linux/dyinggasp.h>
/* end of maintain dying gasp by liuzhijie 00028714 2006年5月9日" */

/* Typedefs. */

#if defined (WIRELESS)
#if 1 /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
#define SES_EVENT_BTN_PRESSED      0x00000001
#define SES_EVENTS                 SES_EVENT_BTN_PRESSED /*OR all values if any*/
#endif
#define SES_LED_OFF     0
#define SES_LED_ON      1
#define SES_LED_BLINK   2
#endif

/*start of增加MAC地址扩充功能 by c00131380 at 080912*/
#define EXT_MAC_1 1
#define EXT_MAC_2 2
#define EXT_MAC_3 3
#define EXT_MAC_4 4

//MAC地址状态信息
#define EXT_MAC_UNUSED   0  //MAC地址未被使用
#define EXT_MAC_INUSED   1  //MAC地址已被使用
#define EXT_MAC_NULL     3  //空MAC信息

#define INIT_MAC_FLAG "MACAINIT"

/*end of增加MAC地址扩充功能 by c00131380 at 080912*/



typedef struct
{
    unsigned long eventmask;    
} BOARD_IOC, *PBOARD_IOC;

#define POLLCNT_1SEC        HZ
#define BP_ACTIVE_MASK   0x8000
#define FLASH_WRITING      1
#define FLASH_IDLE             0
static struct timer_list   poll_timer;
/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
/* Add: Support HG556 (vodafone global) hardware requirement */
static struct timer_list   poll_restorebtn_timer;
static struct timer_list   poll_diagnosebtn_timer;
static struct timer_list	 ethernet_port_led_on_timer;
/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */
int s_iFlashWriteFlg = 0;
EXPORT_SYMBOL(s_iFlashWriteFlg);
/* start of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
extern int igmpw_syslog;
/* end of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
static struct timer_list   poll_dataCard;
int g_iReset = 1;
EXPORT_SYMBOL(g_iReset);

/* 记录硬件类型, 避免每次在时钟中断中读取IO来区分,   */
/* 这样修改能够解决每次读IO时, 3G下载Atheros WPS导致oops问题 */
static unsigned char hardware_type = HG55VDFA;

/* 记录telnetd pid, 按键诊断完毕之后通知该PID */
static int telnetd_pid = 0;

#define NOTIFY_TELNETD();   \
do {    \
    struct task_struct *user_app = NULL; \
    if (0 != telnetd_pid ) \
    { \
        user_app = find_task_by_pid(telnetd_pid); \
    } \
    if (NULL != user_app) \
    { \
        send_sig(SIGHUP, user_app, 0); \
    } \
    else \
    { \
        printk("telnet pid[%d] not exist\n", telnetd_pid); \
    } \
} while (0); 

#if defined (WIRELESS)
static int WPS_FLAG = 0 ;
static struct timer_list   switch_wlan_timer;
#endif

/*Dyinggasp callback*/
typedef void (*cb_dgasp_t)(void *arg);
typedef struct _CB_DGASP__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_dgasp_t cb_dgasp_fn;
    void *context;
}CB_DGASP_LIST , *PCB_DGASP_LIST;

static struct workqueue_struct *board_workqueue;
static struct work_struct board_work;

/*恢复出场配置标志*/
static int g_iRestoreFlag = 0;


static LED_MAP_PAIR LedMapping[MAX_VIRT_LEDS] =
{   // led name     Initial state       physical pin (ledMask)
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},    
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},   
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32},            
    {kLedEnd,       kLedStateOff,       0, 0, 0, 0, 0, 0, GPIO_LOW32} // NOTE: kLedEnd has to be at the end.
};

/* Externs. */
extern struct file fastcall *fget_light(unsigned int fd, int *fput_needed);
extern unsigned int nr_free_pages (void);
extern const char *get_system_type(void);
extern void kerSysFlashInit(void);
extern unsigned long get_nvram_start_addr(void);
extern unsigned long get_scratch_pad_start_addr(void);
extern unsigned long getMemorySize(void);
extern void __init boardLedInit(PLED_MAP_PAIR);
extern void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern void kerSysLedRegisterHandler( BOARD_LED_NAME ledName,
    HANDLE_LED_FUNC ledHwFunc, int ledFailType );
extern UINT32 getCrc32(byte *pdata, UINT32 size, UINT32 crc);

/* Prototypes. */
void __init InitNvramInfo( void );
PNVRAM_INFO g_pNvramInfo = NULL;

/* start of MAC地址扩容 c00131380 at 080909*/
void initExtendMacAddrInfo( void );
void firstInitExtendMACInfo( void );
PMACAdditionInfo g_pExtendMacInfo = NULL;
EXPORT_SYMBOL(g_pExtendMacInfo);
EXPORT_SYMBOL(g_pNvramInfo);
/* end of MAC地址扩容 c00131380 at 080909*/

DECLARE_MUTEX(flash_sem_lock);

/*规格变更，为减少动态扩展MAC地址带来的错误或者问题，直接提供10个MAC地址*/
/*start of 取消扩展MAC处理 c00131380 at 081119*/
static int g_iReservedMacBeUsedNum = 0; //保留地址被占用数
/*end of 取消扩展MAC处理 c00131380 at 081119*/

static int board_open( struct inode *inode, struct file *filp );
static int board_ioctl( struct inode *inode, struct file *flip, unsigned int command, unsigned long arg );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos); 
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);                        

static BOARD_IOC* borad_ioc_alloc(void);
static void borad_ioc_free(BOARD_IOC* board_ioc);

/* DyingGasp function prototype */
static void __init kerSysDyingGaspMapIntr(void);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs);
#else
static unsigned int kerSysDyingGaspIsr(void);
#endif
static void __init kerSysInitDyingGaspHandler( void );
static void __exit kerSysDeinitDyingGaspHandler( void );
/* -DyingGasp function prototype - */

static int ConfigCs(BOARD_IOCTL_PARMS *parms);
static void SetPll(int pll_mask, int pll_value);
static void SetGpio(int gpio, GPIO_STATE_t state);
static int kerSysGetResetStatus(void);
static void bcm63xx_poll_reset_button(void);
/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
/* Add: Support HG556 (vodafone global) hardware requirement */
static void bcm63xx_poll_restore_default_config_button(void);
static void bcm63xx_poll_diagnose_ethernet_port_button(void);
static void bcm63xx_poll_ethernet_port_led_on(void);
static int kerSysGetLswLinkState(void);
void kerSysSetLswLinkState(int iState);
/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */
static void bcm63xx_poll_switch_wlan_key(unsigned long ulData);

void restore_do_tasklet(void* ptr);

#if defined (WIRELESS)
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs);
static void __init sesBtn_mapGpio(void);
static void __init sesBtn_mapIntr(int context);
static Bool sesBtn_pressed(void); //for WPS feature by yiheng
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait);
static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos);
static void __init sesLed_mapGpio(void);
static void sesLed_ctrl(int action);
static void __init ses_board_init(void);
static void __exit ses_board_deinit(void);
#endif


static int g_ledInitialized = 0;
static wait_queue_head_t g_board_wait_queue;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

static int g_wakeup_monitor = 0;
static struct file *g_monitor_file = NULL;
static struct task_struct *g_monitor_task = NULL;
static unsigned int (*g_orig_fop_poll)
    (struct file *, struct poll_table_struct *) = NULL;

static struct file_operations board_fops =
{
  open:       board_open,
  ioctl:      board_ioctl,
  poll:       board_poll,
  read:       board_read,
  release:    board_release,
};

uint32 board_major = 0;

static int g_sWlanFlag = 0;

static int g_sVoipServiceStatus = 0;  /*1: 正在进行VOIP业务; 0: 没有进行VOIP业务*/

#if defined (WIRELESS)
static unsigned short sesBtn_irq = BP_NOT_DEFINED;
static unsigned short sesBtn_gpio = BP_NOT_DEFINED;
static unsigned short sesLed_gpio = BP_NOT_DEFINED;
#endif


/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
int g_nHspaTrafficMode = MODE_NONE;
EXPORT_SYMBOL(g_nHspaTrafficMode);
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
/*定义单板装备按键数*/
						
int g_nEquipTestMode = FALSE;
EXPORT_SYMBOL(g_nEquipTestMode);
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/
int g_nKeyPressed = 0;

#define EQUIP_KEY_TIME			15
int g_nKeyWaitTimes = 0;

extern int g_iDataCardIn;
extern int iUsbTtyState;

/* j00100803 save fwlog to flash for HG556A */
int gl_nIsRebootBtn = 0;
/* j00100803 save fwlog to flash for HG556A */
#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );              
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE 


#ifdef VDF_HG556
void setAllLedOn(void)
{
	kerSysLedCtrl(kLedMessage, kLedStateOn);
	kerSysLedCtrl(kLedAdsl, kLedStateOn);
	kerSysLedCtrl(kLedHspa, kLedStateOn);
	kerSysLedCtrl(kLedPower, kLedStateOn);
	
	kerSysLedCtrl(kLedLan1, kLedStateOn);
	kerSysLedCtrl(kLedLan2, kLedStateOn);
	kerSysLedCtrl(kLedLan3, kLedStateOn);
	kerSysLedCtrl(kLedLan4, kLedStateOn);
}
#endif


static void clear_port_register(int port)
{
#define EHCI_PORT_SC_0_ADDR		0xfffe1354
#define EHCI_PORT_SC_1_ADDR		0xfffe1358
#define EHCI_PORT_STAT_MASK		0x00000F00
#define EHCI_PORT_J_STAT			0x00000900
#define EHCI_PORT_K_STAT			0x00000500
#define EHCI_BAD_LIMIT                      0x01


	static unsigned int siMulAdd[2] = {0, 0};
	volatile unsigned int *ulReg = port ? EHCI_PORT_SC_1_ADDR : EHCI_PORT_SC_0_ADDR;
	int iRegVal = 0;

	if (port > 1)
	{
		return;
	}

	//printk("ulReg %x %x \n", ulReg, *ulReg);


	iRegVal = *ulReg & EHCI_PORT_STAT_MASK;

	if (iRegVal == EHCI_PORT_J_STAT || iRegVal == EHCI_PORT_K_STAT)
	{
		siMulAdd[port]++;
	}

	if (siMulAdd[port] > EHCI_BAD_LIMIT)
	{
		*ulReg = 0x00001002;
		siMulAdd[port] = 0;
		printk("ly: ......clear usb. \n");
	}

	return;
}

/*l65130 2008-09-20 start*/
int kerRecoverDataCard(void)
{
	unsigned short usGpioHspa;
	if (BP_SUCCESS == BpGetHspaEnableGpio(&usGpioHspa))
	{
		SetGpio(usGpioHspa, GPIO_HIGH);
		return 0;
	}
	else
	{
		return -1;
	}
}
/*l65130 2008-09-20 end*/

/*****************************************************************************
 函 数 名  : fault_detect
 功能描述  : 检测数据卡是否识别正确
 输入参数  : unsigned long ulData  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年4月18日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
void fault_detect(unsigned long ulData)
{
    static int iDo = 0;
    static int iOldUsbState = 3;
    static int iRecoverTimes = 0;
    int i;

#define USB_RECOVER_TIMES			3

    /*检测 按键10s l65130 2008-09-20 start*/
    if (g_nEquipTestMode)
    {
    	    g_nKeyWaitTimes++;
	    if (EQUIP_KEY_TIME < g_nKeyWaitTimes)
	    {
	    		if (EQUIP_KEY_MASK != g_nEquipTestMode)
    			{
    				printk("keytest fail\r\n");
				
				g_nEquipTestMode = FALSE;
				g_nKeyPressed = 0;
				g_nKeyWaitTimes = 0;
			    NOTIFY_TELNETD();
    			}
	    }
    }
    /*检测 按键10s l65130 2008-09-20 end*/	
	
    if (g_iDataCardIn)
    {
        if (iUsbTtyState < 2 && (!iDo))
        {  
            if (iOldUsbState != iUsbTtyState)
            {
            	   for (i = 0; i < 2; i++)
		    {
				clear_port_register(i);
		    }
                iOldUsbState = iUsbTtyState;
                poll_dataCard.expires = jiffies + HZ;
                add_timer(&poll_dataCard);
                return;
            }

	      g_iDataCardIn = 3;
          /* useless code. <tanyin 2009.6.9> */
          //kerSysLedCtrl(kLedHspa, kLedStateFail);

		/*Start -- w00135358 delete for HG556a-ISO - 20090715*/
		#if 0
	     /*增加USB数据卡自动恢复机制l65130 2008-09-20 start*/
		if (USB_RECOVER_TIMES <= iRecoverTimes)
		{
			if (!kerRecoverDataCard())
			{
				iRecoverTimes++;
			}
			
		}
	     /*增加USB数据卡自动恢复机制l65130 2008-09-20 end*/
		#endif
		/*End -- w00135358 delete for HG556a-ISO - 20090715*/

        }
	  else
  	 {	/*l65130 2008-09-20 start*/
  	 	iRecoverTimes = 0;
		/*l65130 2008-09-20 end*/
	 }
        iDo = 1;
    }
    else
    {
        if (iDo)
        {
            kerSysLedCtrl(kLedHspa, kLedStateOff);
            iDo = 0;
	     iOldUsbState = 3;
        }
    }

    for (i = 0; i < 2; i++)
    {
		clear_port_register(i);
    }

    	
    poll_dataCard.expires = jiffies +  HZ;
    add_timer(&poll_dataCard);
    
    return;
}


/* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
static int map_external_irq (int irq)
{
   int map_irq ;

   switch (irq) {
      case BP_EXT_INTR_0   :
         map_irq = INTERRUPT_ID_EXTERNAL_0;
         break ;
      case BP_EXT_INTR_1   :
         map_irq = INTERRUPT_ID_EXTERNAL_1;
         break ;
      case BP_EXT_INTR_2   :
         map_irq = INTERRUPT_ID_EXTERNAL_2;
         break ;
      case BP_EXT_INTR_3   :
         map_irq = INTERRUPT_ID_EXTERNAL_3;
         break ;
#if defined(CONFIG_BCM96358) || defined(CONFIG_BCM96368)
      case BP_EXT_INTR_4   :
         map_irq = INTERRUPT_ID_EXTERNAL_4;
         break ;
      case BP_EXT_INTR_5   :
         map_irq = INTERRUPT_ID_EXTERNAL_5;
         break ;
#endif
      default           :
         printk ("Invalid External Interrupt definition \n") ;
         map_irq = 0 ;
         break ;
   }
     return (map_irq) ;
}

static int __init brcm_board_init( void )
{
    /* HUAWEI VHG WangXinfeng 69233 2008-05-22 Modify begin */
    /* Modify: Support HG556-Global hardware front panel led requirement */
    typedef int (*BP_LED_FUNC) (unsigned short *);
    static struct BpLedInformation
    {
        BOARD_LED_NAME ledName;
        BP_LED_FUNC bpFunc;
        BP_LED_FUNC bpFuncFail;
    } bpLedInfo[] =
    {{kLedMessage, BpGetMessageLedGpio, NULL},
     {kLedAdsl, BpGetAdslLedGpio, BpGetAdslFailLedGpio},
     {kLedHspa, BpGetHspaLedGpio, BpGetHspaFailLedGpio}, 
     /*start of  GPIO 3.4.2 porting by l39225 20060504*/
     /*s48571 modify  for VDF HG553 hardware porting begin*/
     //{kLedPower,BpGetBootloaderPowerOnLedGpio,NULL},
     {kLedPower,BpGetBootloaderPowerOnLedGpio,BpGetBootloaderStopLedGpio},
     /*s48571 modify  for VDF HG553 hardware porting end*/
     /*end of  GPIO 3.4.2 porting by l39225 20060504*/
     /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
     /* Add: Support HG556 (vodafone global) hardware requirement */
     {kLedLan1, BpGetLan1LinkUpLedGpio, BpGetLan1LinkDownLedGpio},
     {kLedLan2, BpGetLan2LinkUpLedGpio, BpGetLan2LinkDownLedGpio},
     {kLedLan3, BpGetLan3LinkUpLedGpio, BpGetLan3LinkDownLedGpio},
     {kLedLan4, BpGetLan4LinkUpLedGpio, BpGetLan4LinkDownLedGpio},
     /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */
     /* HUAWEI HGW s48571 2008年1月18日 hardware porting modify begin:*/
     {kLedWireless, NULL, BpGetWirelessFailLedGpio},
     /* HUAWEI HGW s48571 2008年1月18日 hardware porting modify end. */
#ifndef VDF_HG556	
     {kLedUsb, BpGetUsbLedGpio, NULL},
     {kLedHpna, BpGetHpnaLedGpio, NULL},
     {kLedWanData, BpGetWanDataLedGpio, NULL},
     {kLedWanDataFail, BpGetWanDataFailLedGpio, NULL},             
     {kLedPPP, BpGetPppLedGpio, BpGetPppFailLedGpio},
     {kLedVoip, BpGetVoipLedGpio, NULL},
     {kLedSes, BpGetWirelessSesLedGpio, NULL}, 
     /*start of y42304 added 20061109: 支持2路电话线路等的支持 */
     {kLedLine0,BpGetTelLine0LedGpio,NULL},
     {kLedLine1,BpGetTelLine1LedGpio,NULL},
     {kLedPSTN, BpGetPstnLedGpio,    NULL}, 
     /*end of y42304 added 20061109: 支持2路电话线路等的支持 */
     /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add begin:*/
     {kLedInternet, BpGetInternetLedGpio, BpGetInternetFailLedGpio}, 
     /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/
#endif
     {kLedEnd, NULL, NULL}
    };
    /* HUAWEI VHG WangXinfeng 69233 2008-05-22 Modify end */

    int ret;
        
    ret = register_chrdev(BOARD_DRV_MAJOR, "bcrmboard", &board_fops );
    if (ret < 0)
    {
        printk( "brcm_board_init(major %d): fail to register device.\n",BOARD_DRV_MAJOR);
    }
    else 
    {
        PLED_MAP_PAIR pLedMap = LedMapping;
        unsigned short gpio;
        struct BpLedInformation *pInfo;

        printk("brcmboard: brcm_board_init entry\n");
        board_major = BOARD_DRV_MAJOR;
        InitNvramInfo();

        for( pInfo = bpLedInfo; pInfo->ledName != kLedEnd; pInfo++ )
        {
            if( pInfo->bpFunc && (*pInfo->bpFunc) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledActiveLow = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerial = (gpio & BP_GPIO_SERIAL) ? 1 : 0;

                /* start of board added by y42304 20060517: 解决能用GPIO32或以上点灯的问题 */
                if ((gpio & GPIO_NUM_TOTAL_BITS_MASK) >= 32)
                {                                        
                    pLedMap->ledMask = GPIO_NUM_TO_MASK_HIGH(gpio);
                    pLedMap->ledlow32GPIO = GPIO_HIGH; /* 所用的GPIO号为32或以上 */
                }
                else
                {                    
                    pLedMap->ledMask = GPIO_NUM_TO_MASK(gpio);
                }
                /* end of board added by y42304 20060517 */
            }
            if( pInfo->bpFuncFail && (*pInfo->bpFuncFail) (&gpio) == BP_SUCCESS )
            {
                pLedMap->ledName = pInfo->ledName;
                pLedMap->ledActiveLowFail = (gpio & BP_ACTIVE_LOW) ? 1 : 0;
                pLedMap->ledSerialFail = (gpio & BP_GPIO_SERIAL) ? 1 : 0;
                
                /* start of board added by y42304 20060517: 解决能用GPIO32或以上点灯的问题 */
                if ((gpio & GPIO_NUM_TOTAL_BITS_MASK) >= 32)
                {                                        
                    pLedMap->ledMaskFail = GPIO_NUM_TO_MASK_HIGH(gpio);                    
                    pLedMap->ledlow32GPIO = GPIO_HIGH; /* 所用的GPIO号为32或以上 */
                }
                else
                {                    
                    pLedMap->ledMaskFail = GPIO_NUM_TO_MASK(gpio);
                }
                /* end of board added by y42304 20060517 */
            }
            if( pLedMap->ledName != kLedEnd )
            {
                pLedMap++;
            }
        }
        
        init_waitqueue_head(&g_board_wait_queue);

        board_workqueue = create_singlethread_workqueue("board");
        
#if defined (WIRELESS)
        ses_board_init();
#endif        
        kerSysInitDyingGaspHandler();
        kerSysDyingGaspMapIntr();

        boardLedInit(LedMapping);
        g_ledInitialized = 1;
        /* start of y42304 added 20061230: 系统启动时先关闭Voip, 线路LED */    
         kerSysLedCtrl(kLedVoip, kLedStateOff);        
         kerSysLedCtrl(kLedLine0,kLedStateOff);        
         kerSysLedCtrl(kLedLine1,kLedStateOff);
        /* end of y42304 added 20061230 */     
        /*START ADD : liujianfeng 37298 2007-01-14 for [A36D03423]*/
         kerSysLedCtrl(kLedPSTN,kLedStateOn);
        /*END ADD : liujianfeng 37298 2007-01-14 for [A36D03423]*/

        /*start of  GPIO 3.4.3 porting by l39225 20060504*/
        init_timer(&poll_timer);
        poll_timer.function = bcm63xx_poll_reset_button;
        poll_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&poll_timer);
        /*end of  GPIO 3.4.3 porting by l39225 20060504*/

        /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
        /* Add: Support HG556 (vodafone global) hardware requirement */
        init_timer(&poll_restorebtn_timer);
        poll_restorebtn_timer.function = bcm63xx_poll_restore_default_config_button;
        poll_restorebtn_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&poll_restorebtn_timer);

        init_timer(&poll_diagnosebtn_timer);
        poll_diagnosebtn_timer.function = bcm63xx_poll_diagnose_ethernet_port_button;
        poll_diagnosebtn_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&poll_diagnosebtn_timer);

        init_timer(&ethernet_port_led_on_timer);
        ethernet_port_led_on_timer.function = bcm63xx_poll_ethernet_port_led_on;
        /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */

    /* Start of y42304 added 20061223: 支持HG550i ADM6996+Si3215单板wlan开关按键功能 */
    #if defined (WIRELESS)    	
        init_timer(&switch_wlan_timer);
        switch_wlan_timer.function = bcm63xx_poll_switch_wlan_key;
        switch_wlan_timer.expires = jiffies + 10*POLLCNT_1SEC;
        add_timer(&switch_wlan_timer);
        hardware_type = GetHarewareType();
        printk("@@Board@@ GetHarewareType = 0x%x\n", hardware_type);
        
    #endif        
    /* end of y42304 added 20061223 */


        /*start of l65130 hspa故障检测20080112*/
        init_timer(&poll_dataCard);
        poll_dataCard.function = fault_detect;
        poll_dataCard.expires = jiffies + 30*HZ;
        add_timer(&poll_dataCard);
        /* end of l65130 20080112 */
    
        kerSysLedCtrl(kLedPower, kLedStateOn);    
    }
    return ret;
} 

/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
/* Add: Support HG556 (vodafone global) hardware requirement */
static int kerSysGetRestoreDefCfgBtnStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

	if( BpGetRestoreDefCfgGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		printk(" Hardware no restore default configuration button!\n");
	}

	return iRet;
}

#define RESTORE_DEFAULT_SETTING_TIME      5
static void bcm63xx_poll_restore_default_config_button(void)
{
  static int s_iRestoreDefCfgBtnCnt = 0;
  int  iRet;


  if (FLASH_WRITING ==s_iFlashWriteFlg)
  {
        s_iRestoreDefCfgBtnCnt = 0;
        poll_restorebtn_timer.expires = jiffies + POLLCNT_1SEC;
	      add_timer(&poll_restorebtn_timer);
        return;
  }
	
	iRet = kerSysGetRestoreDefCfgBtnStatus();
	
	if(iRet == 0) 
	{
        /* key test */
        if (TRUE <= g_nEquipTestMode)
        {
            printk("resetkey is pressed\r\n");
            g_nEquipTestMode |= EQUIP_RESET_KEY;
            g_nKeyPressed = EQUIP_RESET_KEY;
            
            if (EQUIP_KEY_MASK == g_nEquipTestMode)
            {
                printk("keytest success\r\n");
            }
            else
            {
                printk("keytest fail\r\n");
            }
            g_nEquipTestMode = FALSE;
            g_nKeyWaitTimes = 0;
            NOTIFY_TELNETD();
        }
		else
        {
       		s_iRestoreDefCfgBtnCnt++;
            if(s_iRestoreDefCfgBtnCnt >= RESTORE_DEFAULT_SETTING_TIME) /* actual function */
		    {
    			g_sVoipServiceStatus = 0;

    			if (!g_iRestoreFlag)
    	      	{
    	      		g_iRestoreFlag = 1;
            		INIT_WORK(&board_work, restore_do_tasklet, NULL);
            		queue_work(board_workqueue, &board_work);
    	      	}
            }
		}
	} 

    if(iRet >= 0)
    {
        if(iRet>0) {
            s_iRestoreDefCfgBtnCnt = 0;
        }
        poll_restorebtn_timer.expires = jiffies + POLLCNT_1SEC;
        add_timer(&poll_restorebtn_timer);
    }
}

static int kerSysGetDiagnoseEthPortBtnStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

	if( BpGetDiagnoseEthPortGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		printk(" Hardware no diagnose ethernet port status button!\n");
	}

	return iRet;
}

/* HUAWEI VHG WangXinfeng 69233 2008-05-22 Add begin */
/* Add: Support diagnose ethernet port link state requrement */
static int g_iLswLinkState = 0;
static int kerSysGetLswLinkState()
{
    return g_iLswLinkState;
}	

void kerSysSetLswLinkState(int iState)
{
    g_iLswLinkState = iState;
}

EXPORT_SYMBOL(kerSysSetLswLinkState);

static void bcm63xx_poll_ethernet_port_led_on(void)
{
	kerSysLedCtrl(kLedLan1, kLedStateOff);
	kerSysLedCtrl(kLedLan2, kLedStateOff);
	kerSysLedCtrl(kLedLan3, kLedStateOff);
	kerSysLedCtrl(kLedLan4, kLedStateOff);

	poll_diagnosebtn_timer.expires = jiffies + POLLCNT_1SEC;
	add_timer(&poll_diagnosebtn_timer);
}
/* HUAWEI VHG WangXinfeng 69233 2008-05-22 Add begin */

#define LAN_1_LINKUP      1
#define LAN_2_LINKUP      2
#define LAN_3_LINKUP      4
#define LAN_4_LINKUP      8

#define DIAGNOSE_ETHERNET_PORT_STATUS_TIME      1
static void bcm63xx_poll_diagnose_ethernet_port_button(void)
{
	static int s_iDiagnoseEthPortBtnCnt = 0;
	int linkState = 0;
	int  iRet;
	
	iRet = kerSysGetDiagnoseEthPortBtnStatus();

	if (iRet == 0) 
	{
		s_iDiagnoseEthPortBtnCnt++;
		if(s_iDiagnoseEthPortBtnCnt >= DIAGNOSE_ETHERNET_PORT_STATUS_TIME) 
		{
			/*l65130 2008-09-20 start*/
			if (TRUE <= g_nEquipTestMode )
			{
				g_nEquipTestMode |= EQUIP_LANDIAG_KEY;
				g_nKeyPressed = EQUIP_LANDIAG_KEY;

			    NOTIFY_TELNETD();

				printk("landiagkey is pressed\r\n");
				g_nKeyWaitTimes = 0;
			}
			/*l65130 2008-09-20 end*/
			else
			{
				linkState = kerSysGetLswLinkState();
			
				if (linkState & LAN_1_LINKUP){
					kerSysLedCtrl(kLedLan1, kLedStateOn);
				}else{
					kerSysLedCtrl(kLedLan1, kLedStateFail);
				}

				if (linkState & LAN_2_LINKUP){
					kerSysLedCtrl(kLedLan2, kLedStateOn);
				}else{
					kerSysLedCtrl(kLedLan2, kLedStateFail);
				}
				
				if (linkState & LAN_3_LINKUP){
					kerSysLedCtrl(kLedLan3, kLedStateOn);
				}else{
					kerSysLedCtrl(kLedLan3, kLedStateFail);
				}
				
				if (linkState & LAN_4_LINKUP){
					kerSysLedCtrl(kLedLan4, kLedStateOn);
				}else{
					kerSysLedCtrl(kLedLan4, kLedStateFail);
				}

				ethernet_port_led_on_timer.expires = jiffies + 5*POLLCNT_1SEC;
				add_timer(&ethernet_port_led_on_timer);
				
			}
		
		}
	} 

  if(iRet >= 0)
  {
	poll_diagnosebtn_timer.expires = jiffies + POLLCNT_1SEC;
	add_timer(&poll_diagnosebtn_timer);
  }
}
/* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */

#if defined (WIRELESS)      
/*************************************************
  Function:       kerSysGetWlanButtonStatus
  Description:    获取wlan 开关按键按键的状态
  Input:          无
  Output:         无
  Return:          0:  低电平
                   >0: 高电平
                   -1: 无wlan开关按键
*************************************************/
static int kerSysGetWlanButtonStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

    /* 得到wlan 开关按键GPIO*/
	//if( BpGetWirelessLedGpio( &unGpio ) == BP_SUCCESS )
	//modify by y68191 无线状态按钮变更
if( BpGetWirelessSesBtnGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		//printk(" Hardware no wlan button\n");
	}

	return iRet;
}

#ifdef  SUPPORT_ATHEROSWLAN
/* add for Atheros 11n wsccmd :w45260 2008-08-25 */
#define MAX_ATH_NETS 100
static int g_netdevice[MAX_ATH_NETS];
static int g_callbacks[MAX_ATH_NETS];     //static irqreturn_t jumpstart_intr(int cpl, void *dev_id, struct pt_regs *regs);
static unsigned  regIndex = 0;

void unregister_simple_config_callback(void)
{
    printk("\nunregister_simple_config_callback is called \n");
    regIndex --;	
}	

void register_simple_config_callback (void *callback, void *arg)
{
     if(regIndex > MAX_ATH_NETS)
	 	regIndex = 0;
	 
	if(NULL != arg) 
	{
	    g_netdevice [ regIndex] = arg;
	    g_callbacks[ regIndex] = callback;	
	}	
	regIndex ++;
}
EXPORT_SYMBOL(unregister_simple_config_callback);
EXPORT_SYMBOL(register_simple_config_callback);

void do_wps_pbc_notify_atheros(void)
{
    static irqreturn_t (*callbackfun_jumpstart_intr)(int cpl, void *dev_id, struct pt_regs *regs);
    void *dev_id = NULL;
    int i = 0;
	
    for(i = 0;i<regIndex;i++)
    {
        printk("board.c:notify PBC event to Atheros wsccmd...\n",regIndex);
        callbackfun_jumpstart_intr = g_callbacks[i] ;
        dev_id = g_netdevice [i];
        (* callbackfun_jumpstart_intr)(0, dev_id, NULL);    
    }
}	
/*add end */
#endif

/* start of vdf HG556  l68693 added 20090105: 满足CR20081213008需求 WPS 3->4
 WiFi 按键，如果长按 <4s: wifi on/off ; >4s: wifi WPS
*/
#define REVERSE_WLAN_STATE_TIME       1  
#define WPS_WLAN_TIME                 4  
#define COUNTS_PSEC                   4  
/* end of vdf HG556  l68693 added 20090105: 满足CR20081213008需求 WPS 3->4 */
static void bcm63xx_poll_switch_wlan_key(unsigned long ulData)
{
    static unsigned char s_ucWlanButtonCnt = 0;    
    static unsigned char s_ucIsChangeWlanStatus = 0;
    static int  iStateReverseCounts = 1;
    static int  iWPSCounts      = WPS_WLAN_TIME * COUNTS_PSEC;
    static int  iJiffiesCounts  = POLLCNT_1SEC/COUNTS_PSEC;
    int  iRet;

    iRet = kerSysGetWlanButtonStatus();
    if (iRet == 0) 
    {            
        s_ucWlanButtonCnt++;
    }
    else if (iRet > 0)
    {
        if (s_ucWlanButtonCnt > iWPSCounts)
        {
            if (TRUE <= g_nEquipTestMode )
            {
                g_nEquipTestMode |= EQUIP_WLAN_KEY;
    			g_nKeyPressed = EQUIP_WLAN_KEY;
                
                NOTIFY_TELNETD();

                printk("wlankey is pressed\r\n");
                g_nKeyWaitTimes = 0;
            }
            else
            {
                printk(" wps button has been pressed!\n");
                if (HG55VDFA == hardware_type)
                {
            #ifdef  SUPPORT_ATHEROSWLAN
                    do_wps_pbc_notify_atheros();
            #else
                    WPS_FLAG = 1;
                    wake_up_interruptible(&g_board_wait_queue); 
            #endif  
                }
                else
                {
        			WPS_FLAG = 1;
        			kerSysWakeupMonitorTask();   
                }
            }
        }
        else if (s_ucWlanButtonCnt >= iStateReverseCounts)
        {    
            if (TRUE <= g_nEquipTestMode )
            {
                g_nEquipTestMode |= EQUIP_WLAN_KEY;
    			g_nKeyPressed = EQUIP_WLAN_KEY;
                
                NOTIFY_TELNETD();

                printk("wlankey is pressed\r\n");
                g_nKeyWaitTimes = 0;
            }
            else
            {
#ifdef  SUPPORT_ATHEROSWLAN
                printk(" wlan reset button has been pressed!!\n");
                g_sWlanFlag = (0 == g_sWlanFlag)?1:0;
                kerSysWakeupMonitorTask();
#else   
                printk(" wlan reset button has been pressed!!\n");
                g_sWlanFlag = (0 == g_sWlanFlag)?1:0;
                //s_ucIsChangeWlanStatus = 1;
                kerSysWakeupMonitorTask();
#endif
            }
        }
        s_ucWlanButtonCnt = 0;
    }

    if (iRet >= 0)
    {
        /* Start of w45260 added 20080417: AU8D00593 定时间隔设定为 (1/COUNTS_PSEC) S */
        switch_wlan_timer.expires = jiffies + iJiffiesCounts;
        /* end of w45260 added 20080417*/
        add_timer(&switch_wlan_timer);
    }    
}
#endif

/*start of  GPIO 3.4.3 porting by l39225 20060504*/
/*************************************************
  Function:        kerSysGetResetStatus
  Description:    获取复位按键的状态
  Input:            无
  Output:          无
  Return:          0: 低电平
                       >0 :高电平
                       -1:无复位按键
  Others:         无
*************************************************/

static int kerSysGetResetStatus(void)
{
	unsigned short unGpio;
	unsigned long ulGpioMask;
	volatile unsigned long *pulGpioReg;
	int iRet = -1;

	if( BpGetPressAndHoldResetGpio( &unGpio ) == BP_SUCCESS )
	{
		ulGpioMask = GPIO_NUM_TO_MASK(unGpio);
		pulGpioReg = &GPIO->GPIOio;

	#if !defined(CONFIG_BCM96338)
		if( (unGpio & BP_GPIO_NUM_MASK) >= 32 )
		{
			ulGpioMask = GPIO_NUM_TO_MASK_HIGH(unGpio);
			pulGpioReg = &GPIO->GPIOio_high;
		}
	#endif

		iRet = (int) *pulGpioReg & ulGpioMask;
	}
	else 
	{
		printk(" Hardware no  reset button\n");
	}

	return iRet;
}





/*************************************************
  Function:        bcm63xx_poll_reset_button
  Description:    定时检测复位按键状态并采取对应动作
  Input:            无
  Output:          无
  Return:          无
  Others:          无
*************************************************/
#define FAILURE_TAG_LEN            10
#define FAILURE_TAG                "5aa5a55a"

void restore_do_tasklet(void* ptr)
{
    char cUpgradeFlags[FAILURE_TAG_LEN];
    int iRet = -1;

    
    kerSysAvailSet("resdeft", 8, 2400);
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
	/* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
    //dg_storeDyingGaspInfo();
	/* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
    /* 由于写flash时进程调度可能会被暂停，所以开始写flash时需要禁止进程调度状态监控 */
    dg_setScheduleState(DG_DISABLE_SCHED_MON);
    kerSysPersistentClear();
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    kerSysPsiBackupClear();
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
    kerSysScratchPadClearAll();

    /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
    iRet = kerSysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
    */
    iRet = kerSysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
    /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
    if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
        && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
    {
        /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
        kerSysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
        */
        kerSysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
        /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
    }     
    /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */            

    /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add begin:*/
    char pinTemp[IFC_PIN_PASSWORD_LEN];
    char *pcBuf;
    char tmp[8];

     pcBuf = (char *)kmalloc( CWMP_PARA_LEN, GFP_KERNEL );
     memset(pcBuf, 0x00, CWMP_PARA_LEN);
     memset(tmp, 0x00, sizeof(tmp));
     memset(pinTemp, 0xff, sizeof(pinTemp));
     kerSysAvailSet(pinTemp, IFC_PIN_PASSWORD_LEN, PINCODE_OFFSET);
     kerSysAvailSet("resdeft", RESTORE_FLAG_LEN, WLAN_PARAMETERS_OFFSET + WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN);
    /* BEGIN: Modified by y67514, 2009/11/29   问题单号:cwmp 0 bootstrap 问题*/
     //kerSysAvailSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
     kerSysVariableSet("resetok" , UPG_INFO_STATUS, (CFG_STATE_DATA_START + UPG_INFO_STATUS) );//cwmp reset flag
    /* END:   Modified by y67514, 2009/11/29 */
     kerSysAvailSet("restore", TELNET_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN );
     kerSysAvailSet("resping", PING_TAG_LEN, FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN );
     kerSysAvailSet(pcBuf, CWMP_PARA_LEN, CWMP_PARA_OFFSET);
     kerSysAvailSet("HGReset", CWMP_PARA_RESET, CWMP_PARA_OFFSET+CWMP_PARA_LEN);
     /* WebUI language. <tanyin 2009.2.16> */
     kerSysAvailSet(tmp, WEB_LANGUAGE_LEN, WEB_LANGUAGE_OFFSET);
     kerSysAvailSet(tmp, WEB_ADMIN_LANGUAGE_LEN, WEB_ADMIN_LANGUAGE_OFFSET);

     g_sVoipServiceStatus = 1 ;
    dg_setScheduleState(DG_ENABLE_SCHED_MON);
    /* HUAWEI HGW s48571 2008年4月8日" AU8D00456 按reset按钮3秒以上没有完全恢复到出厂配置 add end.*/

	printk("Restore default configuration and Reset the system!!!\n");
//    kerSysMipsSoftReset ();


    /*start of y42304 20070124 added: 解决按复位键无法恢复wlan led状态 . */
         #if defined (WIRELESS)      
            g_sWlanFlag = 2;      
            kerSysWakeupMonitorTask();   
         #else
            kerSysMipsSoftReset ();
         #endif
         /*end of y42304 20061201 added */

}




#ifdef CONFIG_WLAN_REVERSE_PRESS_RESET_KEY   
 
/* start of y42304 added 20060907: 支持通过按复位键10秒内开启/关闭wlan */
#define RESTORE_DEFAULT_SETTING_TIME  10
#define REVERSE_WLAN_STATE_TIME       3

static void bcm63xx_poll_reset_button(void)
{
	static int s_iResetButtoncnt = 0;
	int  iRet;
	if (FLASH_WRITING ==s_iFlashWriteFlg)
    {
        s_iResetButtoncnt = 0;
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
        return;
    }
	
	iRet = kerSysGetResetStatus();
	
	if(iRet == 0) 
	{
		s_iResetButtoncnt++;
		if(s_iResetButtoncnt >= RESTORE_DEFAULT_SETTING_TIME) 
		{
            char cUpgradeFlags[FAILURE_TAG_LEN];
            
			printk("Restore default configuration and Reset the system!!!\n");

            g_iReset = 1; 

            /* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
            //dg_storeDyingGaspInfo();
			/* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
            kerSysPersistentClear();
            /* begin --- Add Persistent storage interface backup flash partition by w69233 */
            kerSysPsiBackupClear();
            /* end ----- Add Persistent storage interface backup flash partition by w69233 */
            kerSysScratchPadClearAll();

            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
            iRet = kerSysVariableGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            */
            iRet = kerSysAvailGet(cUpgradeFlags, FAILURE_TAG_LEN, 0);
            /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            if ((-1 != iRet) && (NULL != strstr(cUpgradeFlags, FAILURE_TAG)) 
                && (0 != strcmp(cUpgradeFlags, "5aa5a55a0"))) 
            {
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify begin:
                kerSysVariableSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                */
                kerSysAvailSet("5aa5a55a0", FAILURE_TAG_LEN, 0);
                /* HUAWEI HGW s48571 2008年3月28日 FLASH 整理 modify end. */
            }     
            /*start of y42304 20060727 added: 恢复出厂设置时清除升级标识 */    

            
            /*start of y42304 20070124 added: 解决按复位键无法恢复wlan led状态 . */
           #if defined (WIRELESS)      
               g_sWlanFlag = 2;      
               kerSysWakeupMonitorTask();   
           #else
               kerSysMipsSoftReset ();
           #endif
		}
	} 
	else if (iRet > 0)
	{
        /* reset按键时长<3s 更改wlan开关状态 */
		if( s_iResetButtoncnt>=1 && s_iResetButtoncnt < REVERSE_WLAN_STATE_TIME ) 
		{            
            g_sWlanFlag = (0 == g_sWlanFlag)?1:0;
            kerSysWakeupMonitorTask();
		}
        s_iResetButtoncnt = 0;
    }

    g_iReset = 1;

    if(iRet >= 0)
    {
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
    }
}

/* end of y42304 added 20060907 */
#else  


static void bcm63xx_poll_reset_button(void)
{
	static int s_iResetButtoncnt = 0;
	int  iRet;
	static int iRestoreFlag = 0;
	
	if (FLASH_WRITING ==s_iFlashWriteFlg || g_iRestoreFlag)
    {
        s_iResetButtoncnt = 0;
        poll_timer.expires = jiffies + POLLCNT_1SEC;
	    add_timer(&poll_timer);
        return;
    }
	
	iRet = kerSysGetResetStatus();
	if (iRet == 0) 
	{
		s_iResetButtoncnt++;
	} 
	else if (iRet > 0)
	{
		if (s_iResetButtoncnt>=1) 
		{
			/*l65130 2008-09-20 start*/
			if (TRUE <= g_nEquipTestMode )
			{
				g_nEquipTestMode |= EQUIP_RESTART_KEY;
				g_nKeyPressed = EQUIP_RESTART_KEY;

			    NOTIFY_TELNETD();

				printk("restartkey is pressed\r\n");
				g_nKeyWaitTimes = 0;
			}
			/*l65130 2008-09-20 end*/
			else
			{
				printk("Reset the system!!!\n");
				/* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
	            		//dg_storeDyingGaspInfo();
				/* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
				/* j00100803 add for HG556A to reboot HG from user */
				//kerSysMipsSoftReset ();
				/* write the flag to show reboot button is pushed */
				gl_nIsRebootBtn = 1;
				kerSysWakeupMonitorTask();
				/* j00100803 add for HG556A to reboot HG from user */
			}
			
		}
        s_iResetButtoncnt = 0;
    }

    if (iRet >= 0)
    {
        poll_timer.expires = jiffies + POLLCNT_1SEC;
        add_timer(&poll_timer);
    }

}
 /*end of  GPIO 3.4.3 porting by l39225 20060504*/ 
#endif

/*规格变更，为减少动态扩展MAC地址带来的错误或者问题，直接提供10个MAC地址*/
/*start of 取消扩展MAC处理 c00131380 at 081118*/
#if 0    
/* start of MAC地址扩容 c00131380 at 080909*/

/*****************************************************************************
 函 数 名  : firstInitExtendMACInfo
 功能描述  : 初始化FLASH扩展MAC地址数据
 输入参数  : void  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月22日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
void firstInitExtendMACInfo( void )
{
    char InitjudgeFlag[EXT_MAC_INIT_FLAG_LEN];
    unsigned char macAddress[NVRAM_MAC_ADDRESS_LEN];
    
    memset(InitjudgeFlag, 0, sizeof(InitjudgeFlag));
    memset(macAddress,0x00,NVRAM_MAC_ADDRESS_LEN);
    
    //读出系统是否已经初始化MAC地址
    kerSysAvailGet(InitjudgeFlag, EXT_MAC_INIT_FLAG_LEN, EXT_MAC_INIT_FLAG);
    if (0 == strcmp(InitjudgeFlag, INIT_MAC_FLAG))
    {
        printk("MAC already init.\n");
    }
    else   //将FALSH中的MAC信息初始化为0x00
    {
        kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_1_START);
        kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_2_START);
        kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_3_START);
        kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_4_START);
        kerSysAvailSet(INIT_MAC_FLAG, EXT_MAC_INIT_FLAG_LEN, EXT_MAC_INIT_FLAG);
    }
    return;
}

/*****************************************************************************
 函 数 名  : initExtendMacAddrInfo
 功能描述  : 初始化扩展MAC地址数据
 输入参数  : void  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月9日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
void initExtendMacAddrInfo( void )
{
    int iLen;
    int i;
    unsigned char macAddress[NVRAM_MAC_ADDRESS_LEN];
    unsigned char nullMacAdd[NVRAM_MAC_ADDRESS_LEN];
    char baseMac[NVRAM_MAX_MAC_STR_LEN];

    //第一次初始化FLASH扩展MAC地址数据，如果已设置过不做任何处理
    firstInitExtendMACInfo();
    
    //分配内存空间
    g_pExtendMacInfo = (PMACAdditionInfo) kmalloc( sizeof(MACAdditionInfo), GFP_KERNEL );

    if (NULL != g_pExtendMacInfo)
    {
        //初始化值开始
        g_pExtendMacInfo->inUsed1 = EXT_MAC_NULL; //0:未使用 1:已使用 3:无MAC
        g_pExtendMacInfo->inUsed2 = EXT_MAC_NULL;
        g_pExtendMacInfo->inUsed3 = EXT_MAC_NULL;
        g_pExtendMacInfo->inUsed4 = EXT_MAC_NULL;
        g_pExtendMacInfo->totalNum  = 0;
        g_pExtendMacInfo->unUsedNum = 0;
        memset(g_pExtendMacInfo->addMACAddress1,0x00,NVRAM_MAC_ADDRESS_LEN);
        memset(g_pExtendMacInfo->addMACAddress2,0x00,NVRAM_MAC_ADDRESS_LEN);
        memset(g_pExtendMacInfo->addMACAddress3,0x00,NVRAM_MAC_ADDRESS_LEN);
        memset(g_pExtendMacInfo->addMACAddress4,0x00,NVRAM_MAC_ADDRESS_LEN);
        //初始化值结束
        

        memset(macAddress,0x00,NVRAM_MAC_ADDRESS_LEN);
        memset(nullMacAdd,0x00,NVRAM_MAC_ADDRESS_LEN);
        memset(baseMac,0x00,NVRAM_MAX_MAC_STR_LEN);
      
        
        //读第1个MAC的值
        if (-1 != kerSysAvailGet(macAddress, MACADDITION_LEN, MACADDITION_VAR_1_START))
        {
            if (0 != memcmp(macAddress,nullMacAdd,NVRAM_MAC_ADDRESS_LEN))
            {
                memcpy(g_pExtendMacInfo->addMACAddress1,macAddress,NVRAM_MAC_ADDRESS_LEN);
                g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum + 1;
                g_pExtendMacInfo->unUsedNum= g_pExtendMacInfo->unUsedNum + 1;
                g_pExtendMacInfo->inUsed1 = EXT_MAC_UNUSED;
            }
        }
        else
        {
            printk("++GetMacAddress 1 is failure!\n");
        }
        sprintf(baseMac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", g_pExtendMacInfo->addMACAddress1[0],g_pExtendMacInfo->addMACAddress1[1], 
                g_pExtendMacInfo->addMACAddress1[2], g_pExtendMacInfo->addMACAddress1[3],g_pExtendMacInfo->addMACAddress1[4],g_pExtendMacInfo->addMACAddress1[5]);

        iLen = strlen(baseMac);
        for (i = 0; i < iLen ; i++)
        {
            printk("%c",baseMac[i]);
        }
        printk("\n");

        
        //读第2个MAC的值   
        memset(macAddress,0x00,NVRAM_MAC_ADDRESS_LEN);
                
        if (-1 != kerSysAvailGet(macAddress, MACADDITION_LEN,MACADDITION_VAR_2_START))
        {
            if (0 != memcmp(macAddress,nullMacAdd,NVRAM_MAC_ADDRESS_LEN))
            {
                memcpy(g_pExtendMacInfo->addMACAddress2,macAddress,NVRAM_MAC_ADDRESS_LEN);
                g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum + 1;
                g_pExtendMacInfo->unUsedNum= g_pExtendMacInfo->unUsedNum + 1;
                g_pExtendMacInfo->inUsed2 = EXT_MAC_UNUSED;
            }
        }
        else
        {
            printk("++GetMacAddress 2 is failure!\n");
        }
        
        sprintf(baseMac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", macAddress[0],macAddress[1], 
                macAddress[2], macAddress[3],macAddress[4],macAddress[5]);

        iLen = strlen(baseMac);
        for (i = 0; i < iLen ; i++)
        {
            printk("%c",baseMac[i]);
        }
        printk("\n");

        //读第3个MAC的值
        memset(macAddress,0x00,NVRAM_MAC_ADDRESS_LEN);
        
        if (-1 != kerSysAvailGet(macAddress, MACADDITION_LEN,MACADDITION_VAR_3_START))
        {
            if (0 != memcmp(macAddress,nullMacAdd,NVRAM_MAC_ADDRESS_LEN))
            {
                memcpy(g_pExtendMacInfo->addMACAddress3,macAddress,NVRAM_MAC_ADDRESS_LEN);
                g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum + 1;
                g_pExtendMacInfo->unUsedNum= g_pExtendMacInfo->unUsedNum + 1;
                g_pExtendMacInfo->inUsed3 = EXT_MAC_UNUSED;
            }
        }
        else
        {
            printk("++GetMacAddress 3 is failure!\n");
        }
         sprintf(baseMac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", macAddress[0],macAddress[1], 
                macAddress[2], macAddress[3],macAddress[4],macAddress[5]);

        iLen = strlen(baseMac);
        for (i = 0; i < iLen ; i++)
        {
            printk("%c",baseMac[i]);
        }
        printk("\n");
        
        //读第4个MAC的值
        memset(macAddress,0x00,NVRAM_MAC_ADDRESS_LEN);
        
        if (-1 != kerSysAvailGet(macAddress, MACADDITION_LEN,MACADDITION_VAR_4_START))
        {
            if (0 != memcmp(macAddress,nullMacAdd,NVRAM_MAC_ADDRESS_LEN))
            {
                memcpy(g_pExtendMacInfo->addMACAddress4,macAddress,NVRAM_MAC_ADDRESS_LEN);
                g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum + 1;
                g_pExtendMacInfo->unUsedNum= g_pExtendMacInfo->unUsedNum + 1;
                g_pExtendMacInfo->inUsed4 = EXT_MAC_UNUSED;
            }
        }
        else
        {
            printk("++GetMacAddress 4 is failure!\n");
        }
        sprintf(baseMac, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", macAddress[0],macAddress[1], 
                macAddress[2], macAddress[3],macAddress[4],macAddress[5]);

        iLen = strlen(baseMac);
        for (i = 0; i < iLen ; i++)
        {
            printk("%c",baseMac[i]);
        }
        printk("\n");
    }

}
/* end of MAC地址扩容 c00131380 at 080909*/
#endif
/*end of 取消扩展MAC处理 c00131380 at 081118*/

void __init InitNvramInfo( void )
{
    PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
    unsigned long ulNumMacAddrs = pNvramData->ulNumMacAddrs;

    if( ulNumMacAddrs > 0 && ulNumMacAddrs <= NVRAM_MAC_COUNT_MAX )
    {
        /*此处申请如此内存是保证4字节对齐*/
        unsigned long ulNvramInfoSize =
            sizeof(NVRAM_INFO) + ((sizeof(MAC_ADDR_INFO) - 1) * ulNumMacAddrs);

        g_pNvramInfo = (PNVRAM_INFO) kmalloc( ulNvramInfoSize, GFP_KERNEL );

        if( g_pNvramInfo )
        {
            unsigned long ulPsiSize;
            if( BpGetPsiSize( &ulPsiSize ) != BP_SUCCESS )
                ulPsiSize = NVRAM_PSI_DEFAULT;
            memset( g_pNvramInfo, 0x00, ulNvramInfoSize );
            g_pNvramInfo->ulPsiSize = ulPsiSize * 1024;
            g_pNvramInfo->ulNumMacAddrs = pNvramData->ulNumMacAddrs;
            memcpy( g_pNvramInfo->ucaBaseMacAddr, pNvramData->ucaBaseMacAddr,
                NVRAM_MAC_ADDRESS_LEN );
            g_pNvramInfo->ulSdramSize = getMemorySize();

            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC add begin:*/
            int i = 0;
            unsigned long reserveId = 0;
            unsigned char acMacAddr[NVRAM_MAC_ADDRESS_LEN];
            memset(acMacAddr, 0x00,NVRAM_MAC_ADDRESS_LEN );
            for( i = 0 ; i < g_pNvramInfo->ulNumMacAddrs ; i++ )
            {
                switch(i)
                {
                    case 0:
                        reserveId = RESERVE_MAC_MASK;
                        kerSysGetMacAddress(acMacAddr, reserveId);
                        break;
                    /*ralink wifi  require four eth mac 
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    ralink wifi  require four eth mac */
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        reserveId = RESERVE_MAC_MASK & (i << 24);
                        kerSysGetMacAddress(acMacAddr, reserveId);
                        break;
                    default:
                        break;
                }
            }
            /* HUAWEI HGW s48571 2008年2月1日" VDF requirement: Fixed MAC add end.*/
        }
        else
            printk("ERROR - Could not allocate memory for NVRAM data\n");
    }
    else
        printk("ERROR - Invalid number of MAC addresses (%ld) is configured.\n",
            ulNumMacAddrs);
   /*start of 规格变更，取消扩展MAC处理 c00131380 at 081118*/
   #if 0
   /* start of MAC地址扩容 c00131380 at 080909*/
   //从FLASH获得扩从的MAC地址信息放入内存中
   initExtendMacAddrInfo();
   /* end of MAC地址扩容 c00131380 at 080909*/
   #endif
   /*end of 规格变更，取消扩展MAC处理 c00131380 at 081118*/

}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");
	
    if (board_major != -1) 
    {
       /*start of  GPIO 3.4.3 porting by l39225 20060504*/
      del_timer_sync(&poll_timer);
      /*end of  GPIO 3.4.3 porting by l39225 20060504*/
      /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add begin */
      /* Add: Support HG556 (vodafone global) hardware requirement */
      del_timer_sync(&poll_restorebtn_timer);
      del_timer_sync(&poll_diagnosebtn_timer);
      del_timer_sync(&ethernet_port_led_on_timer);
      /* HUAWEI VHG WangXinfeng 69233 2008-05-20 Add end */

#if defined (WIRELESS)    	
    	ses_board_deinit();
        del_timer_sync(&switch_wlan_timer);
#endif    	
        kerSysDeinitDyingGaspHandler();
        unregister_chrdev(board_major, "board_ioctl");
    }
} 

static BOARD_IOC* borad_ioc_alloc(void)
{
    BOARD_IOC *board_ioc =NULL;
    board_ioc = (BOARD_IOC*) kmalloc( sizeof(BOARD_IOC) , GFP_KERNEL );
    if(board_ioc)
    {
        memset(board_ioc, 0, sizeof(BOARD_IOC));
    }
    return board_ioc;
}

static void borad_ioc_free(BOARD_IOC* board_ioc)
{
    if(board_ioc)
    {
        kfree(board_ioc);
    }	
}


static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = borad_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;
            
    return( 0 );
} 

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;
    
    wait_event_interruptible(g_board_wait_queue, 1);    
    borad_ioc_free(board_ioc);

    return( 0 );
} 


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)        	
    BOARD_IOC *board_ioc = filp->private_data;    	
#endif
    	
    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)        	
    if(board_ioc->eventmask & SES_EVENTS){
	if(WPS_FLAG)
      {
          printk("board_poll is woke up\n\r"); 
        mask |= sesBtn_poll(filp, wait);
    	}
    }			
#endif    

    return mask;
}


static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)    
    ssize_t ret=0;
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
	 ret=sesBtn_read(filp, buffer, count, ppos);
	 WPS_FLAG=0;
	 return ret;
    }
#endif    
    return 0;
}

//**************************************************************************************
// Utitlities for dump memory, free kernel pages, mips soft reset, etc.
//**************************************************************************************

/***********************************************************************
 * Function Name: dumpaddr
 * Description  : Display a hex dump of the specified address.
 ***********************************************************************/
void dumpaddr( unsigned char *pAddr, int nLen )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned long ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) pAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(long), nLen -= sizeof(long))
        {
            ul = *(unsigned long *) &pAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(long); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printk( "%s\r\n", szLine );

        pAddr += i;
    }
    printk( "\r\n" );
} /* dumpaddr */


void kerSysMipsSoftReset(void)
{
#if defined(CONFIG_BCM96348)
    if (PERF->RevID == 0x634800A1) {
        typedef void (*FNPTR) (void);
        FNPTR bootaddr = (FNPTR) FLASH_BASE;
        int i;

        /* Disable interrupts. */
        cli();

        /* Reset all blocks. */
        PERF->BlockSoftReset &= ~BSR_ALL_BLOCKS;
        for( i = 0; i < 1000000; i++ )
            ;
        PERF->BlockSoftReset |= BSR_ALL_BLOCKS;
        /* Jump to the power on address. */
        (*bootaddr) ();
    }
    else
        PERF->pll_control |= SOFT_RESET;    // soft reset mips
#else
    PERF->pll_control |= SOFT_RESET;    // soft reset mips
#endif
}

#if 0
int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL )
                {
                    /* This is an available MAC address that has never been
                     * used.
                     */
                    pMaiFreeNoId = pMai;
                    ulIdxNoId = i;
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                         * before.  Use addresses that have never been used
                         * first, before using this one.
                         */
                        pMaiFreeId = pMai;
                        ulIdxId = i;
                    }
            }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            pMaiFreeNoId->chInUse = 1;
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            pMaiFreeId->chInUse = 1;
        }
    }
    else
        if( i == g_pNvramInfo->ulNumMacAddrs )
            nRet = -EADDRNOTAVAIL;

    return( nRet );
} /* kerSysGetMacAddr */
#else
int kerSysGetMacAddress( unsigned char *pucaMacAddr, unsigned long ulId )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = 0;
    int indexNode = 0;
    PMAC_ADDR_INFO pMai = NULL;
    PMAC_ADDR_INFO pMaiFreeNoId = NULL;
    PMAC_ADDR_INFO pMaiFreeId = NULL;
    unsigned long i = 0, ulIdxNoId = 0, ulIdxId = 0, baseMacAddr = 0;

    unsigned long  reservedId = ulId & RESERVE_MAC_MASK ;
//start of modify by y68191
/* 
    if ( reservedId > 0 )
    {
        if( RESERVE_MAC_MASK == reservedId )
        {
            reservedId = 0;
        }
        else
        {
            reservedId = reservedId >> 24;
        }
    }
    else 
    {
        reservedId = NO_RESERVE_MAC;
    }
   */
    if( RESERVE_MAC_MASK == reservedId )
    {
        reservedId = 0;
    }
    else
    {
        reservedId = reservedId >> 24;
	   if ((reservedId != 0x05)&&(reservedId != 0x06)&&(reservedId != 0x07)&&(reservedId != 0x08))
    	  {
             reservedId = NO_RESERVE_MAC;
    	  }
    }

// end of modify by y68191
    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
        i++, pMai++ )
    {
        if( ulId == pMai->ulId || ulId == MAC_ADDRESS_ANY )
        {
            /* This MAC address has been used by the caller in the past. */
            baseMacAddr = (baseMacAddr + i) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMai->chInUse = 1;
            pMaiFreeNoId = pMaiFreeId = NULL;
            break;
        }
        else
        {
            //MAC初始化
            if( pMai->chInUse == 0 )
            {
                if( pMai->ulId == 0 && pMaiFreeNoId == NULL  )
                {
                    /* This is an available MAC address that has never been
                     * used.
                     */
                    if( NO_RESERVE_MAC == reservedId )
                    {
                        pMaiFreeNoId = pMai;
                        ulIdxNoId = i;
                        break;
                    }
                    else if (i == reservedId)//d00104343 Ricky 预留地址和当前序号必须相同
                    {
                        pMaiFreeNoId = pMai;
                        ulIdxNoId = reservedId;
                        break;
                    }
                }
                else
                    if( pMai->ulId != 0 && pMaiFreeId == NULL )
                    {
                        /* This is an available MAC address that has been used
                         * before.  Use addresses that have never been used
                         * first, before using this one.
                         */
                            if( NO_RESERVE_MAC == reservedId )
                            {
                                pMaiFreeId = pMai;
                                ulIdxId = i;
                                
                                break;
                            }
                            else if (i == reservedId)
                            {
                                pMaiFreeId = pMai;
                                ulIdxId = reservedId;
                                break;
                            }
                    }
            }
        }
    }

    if( pMaiFreeNoId || pMaiFreeId )
    {
        /* An available MAC address was found. */
        memcpy(pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,NVRAM_MAC_ADDRESS_LEN);
        if( pMaiFreeNoId )
        {
            baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeNoId->ulId = ulId;
            if( NO_RESERVE_MAC == reservedId )//是否是给wifi的MAC
            {
                pMaiFreeNoId->chInUse = 1; //让1的MAC分配后，不再分配给别人使用

            }
            else
            {
                pMaiFreeNoId->chInUse = 0;
            }
        }
        else
        {
            baseMacAddr = (baseMacAddr + ulIdxId) << 8;
            memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                constMacAddrIncIndex);
            memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
            pMaiFreeId->ulId = ulId;
            if( NO_RESERVE_MAC == reservedId )//是否是给wifi的MAC
            {
                pMaiFreeId->chInUse = 1; //让1的MAC分配后，不再分配给别人使用

            }
            else
            {
                pMaiFreeId->chInUse = 0;
            }
        }
    }
    else
        if ( i > (g_pNvramInfo->ulNumMacAddrs - 1) )//开始使用扩展MAC地址
        {
            /*如果保留MAC被占用，则用指定方式从10到7降序进行添加MAC*/
            /*start of 规格变更，指定10个MAC处理 c00131380 at 081119*/        
            /*
            if ((0x02 == reservedId)||(0x03 == reservedId)||(0x04 == reservedId)
                ||(0x05 == reservedId))
                */
              if ((0x05 == reservedId)||(0x06 == reservedId)||(0x07 == reservedId)
                ||(0x08== reservedId))
    	    {
                /*开始为预留PVC(10/35 10/36 10/37 10/40)指定MAC地址*/
                /*根据被占用数获得对应的节点*/
                pMai = NULL;
                pMaiFreeNoId = NULL;
                
                indexNode = g_pNvramInfo->ulNumMacAddrs - g_iReservedMacBeUsedNum -1;
                if (indexNode > 0)
                {
                    for (i = 0, pMai = g_pNvramInfo->MacAddrs; i < indexNode; i++)
                    {
                        pMai++;
                    }
                    pMaiFreeNoId = pMai;
                    ulIdxNoId = indexNode;
                    //为节点赋值                    
                    baseMacAddr = (baseMacAddr + ulIdxNoId) << 8;
                    memcpy( pucaMacAddr, g_pNvramInfo->ucaBaseMacAddr,
                        constMacAddrIncIndex);
                    
                    memcpy( pucaMacAddr + constMacAddrIncIndex, (unsigned char *)
                        &baseMacAddr, NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex );
                    
                    pMaiFreeNoId->ulId    = ulId;
                    pMaiFreeNoId->chInUse = 1;
                
                    /*被占用数加1*/
                    g_iReservedMacBeUsedNum = g_iReservedMacBeUsedNum + 1;
                }
                //返回
                return( nRet );
    	    }
            /*end of 规格变更，指定10个MAC处理 c00131380 at 081119*/
            
            /*start of 规格变更，取消扩展MAC处理 c00131380 at 081118*/
            #if 0

            /* start of MAC地址扩容 c00131380 at 080910*/
            //printk("###i == g_pNvramInfo->ulNumMacAddrs %u\n",ulId);
            if (NULL != g_pExtendMacInfo)
            {
                int iHasMacFlag; //匹配成功标记，默认为0
                unsigned char macAddress[NVRAM_MAC_ADDRESS_LEN];

                memset(macAddress,0,NVRAM_MAC_ADDRESS_LEN);
                iHasMacFlag = 0;
                //获得第1个值
                if ((0 != memcmp(g_pExtendMacInfo->addMACAddress1,macAddress,NVRAM_MAC_ADDRESS_LEN)) && (0 == g_pExtendMacInfo->inUsed1))
                {
                    memcpy(pucaMacAddr,g_pExtendMacInfo->addMACAddress1,NVRAM_MAC_ADDRESS_LEN);
                    g_pExtendMacInfo->inUsed1 = EXT_MAC_INUSED;
                    g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1; 
                    iHasMacFlag = 1;
                    return( nRet );//匹配成功返回
                }

                //获得第2个值
                if ((0 != memcmp(g_pExtendMacInfo->addMACAddress2,macAddress,NVRAM_MAC_ADDRESS_LEN)) && (0 == g_pExtendMacInfo->inUsed2))
                {
                    memcpy(pucaMacAddr,g_pExtendMacInfo->addMACAddress2,NVRAM_MAC_ADDRESS_LEN);
                    g_pExtendMacInfo->inUsed2 = EXT_MAC_INUSED;
                    g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1; 
                    iHasMacFlag = 1;
                    return( nRet );//匹配成功返回
                }
                //获得第3个值
                if ((0 != memcmp(g_pExtendMacInfo->addMACAddress3,macAddress,NVRAM_MAC_ADDRESS_LEN)) && (0 == g_pExtendMacInfo->inUsed3))
                {
                    memcpy(pucaMacAddr,g_pExtendMacInfo->addMACAddress3,NVRAM_MAC_ADDRESS_LEN);
                    g_pExtendMacInfo->inUsed3 = EXT_MAC_INUSED;
                    g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1; 
                    iHasMacFlag = 1;
                    return( nRet );//匹配成功返回
                }
                //获得第4个值
                if ((0 != memcmp(g_pExtendMacInfo->addMACAddress4,macAddress,NVRAM_MAC_ADDRESS_LEN)) && (0 == g_pExtendMacInfo->inUsed4))
                {
                    memcpy(pucaMacAddr,g_pExtendMacInfo->addMACAddress4,NVRAM_MAC_ADDRESS_LEN);
                    g_pExtendMacInfo->inUsed4 = EXT_MAC_INUSED;
                    g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1; 
                    iHasMacFlag = 1;
                    return( nRet );//匹配成功返回
                }

                if (!iHasMacFlag)//没有找到可用MAC地址
                {
                    nRet = -EADDRNOTAVAIL;
                }
            }
            else
            {
                nRet = -EADDRNOTAVAIL; 
            }
            /* end of MAC地址扩容 c00131380 at 080909*/
            #endif
            nRet = -EADDRNOTAVAIL;
            /*end of 规格变更，取消扩展MAC处理 c00131380 at 081118*/
        }

    return( nRet );
} /* kerSysGetMacAddr */

#endif

int kerSysReleaseMacAddress( unsigned char *pucaMacAddr )
{
    const unsigned long constMacAddrIncIndex = 3;
    int nRet = -EINVAL;
    int iHasNoMacFlag = 1;//匹配成功标记，默认为1
    unsigned long ulIdx = 0;
    unsigned long baseMacAddr = 0;
    unsigned long relMacAddr = 0;

    /* baseMacAddr = last 3 bytes of the base MAC address treated as a 24 bit integer */
    memcpy((unsigned char *) &baseMacAddr,
        &g_pNvramInfo->ucaBaseMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    baseMacAddr >>= 8;

    /* Get last 3 bytes of MAC address to release. */
    memcpy((unsigned char *) &relMacAddr, &pucaMacAddr[constMacAddrIncIndex],
        NVRAM_MAC_ADDRESS_LEN - constMacAddrIncIndex);
    relMacAddr >>= 8;

    ulIdx = relMacAddr - baseMacAddr;

    if( ulIdx < g_pNvramInfo->ulNumMacAddrs )
    {
        PMAC_ADDR_INFO pMai = &g_pNvramInfo->MacAddrs[ulIdx];
        if( pMai->chInUse == 1 )
        {
            pMai->chInUse = 0;
            nRet = 0;
            iHasNoMacFlag = 0;
        }
    }
    
   /*start of 规格变更，取消扩展MAC处理 c00131380 at 081118*/
    #if 0  

    /* start of MAC地址扩容 c00131380 at 080910*/
    if(iHasNoMacFlag)
    {
        //查找到要释放的MAC地址
        if((0 == strcmp(g_pExtendMacInfo->addMACAddress1,pucaMacAddr)) && (1 == g_pExtendMacInfo->inUsed1))
        {
            g_pExtendMacInfo->inUsed1 = EXT_MAC_UNUSED;
            g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1; 
            return( nRet );//匹配成功返回
        }
    
        //获得第2个值
        if((0 == strcmp(g_pExtendMacInfo->addMACAddress2,pucaMacAddr)) && (1 == g_pExtendMacInfo->inUsed2))
        {
            g_pExtendMacInfo->inUsed2 = EXT_MAC_UNUSED;
            g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1; 
            return( nRet );//匹配成功返回
        }
        //获得第3个值
        if((0 == strcmp(g_pExtendMacInfo->addMACAddress3,pucaMacAddr)) && (1 == g_pExtendMacInfo->inUsed3))
        {            
            g_pExtendMacInfo->inUsed3 = EXT_MAC_UNUSED;
            g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1; 
            return( nRet );//匹配成功返回
        }
        //获得第4个值
        if((0 == strcmp(g_pExtendMacInfo->addMACAddress4,pucaMacAddr)) && (1 == g_pExtendMacInfo->inUsed4))
        {            
            g_pExtendMacInfo->inUsed4 = EXT_MAC_UNUSED;
            g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1; 
            return( nRet );//匹配成功返回
        }
    }
    /* end of MAC地址扩容 c00131380 at 080909*/
    #endif
    /*end of 规格变更，取消扩展MAC处理 c00131380 at 081118*/
    return( nRet );
} /* kerSysReleaseMacAddr */

int kerSysGetSdramSize( void )
{
    return( (int) g_pNvramInfo->ulSdramSize );
} /* kerSysGetSdramSize */


void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting delete begin:*/
#if 0
     /* start of y42304 added 20061128: 处理ADSL单色LED */
     if (ledName == kLedAdsl)
     {
         if (ledState == kLedStateFail)
         {
             ledState = kLedStateOff;
         }
     }    
     /* end of y42304 added 20061128*/
#endif    
/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting delete end.*/

    //printk("ledName: %d BOARD_LED_STATE: %d\r\n", ledName, ledState);

    if (g_ledInitialized)
      boardLedCtrl(ledName, ledState);
}

/**************************************************************************
 * 函数名  :   kerSysGetGPIO
 * 功能    :   读取给定GPIO的值
 *
 * 输入参数:   ucGpioNum:  GPIO号
 * 输出参数:   无
 *
 * 返回值  :    正确  :  GPIO值，高电平或低电平
 *
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
unsigned char kerSysGetGPIO(unsigned short ucGpioNum)
{  
    unsigned char ucGpioValue = 0;

    ucGpioNum = (ucGpioNum & BP_GPIO_NUM_MASK);
    
#if !defined (CONFIG_BCM96338)
    /* 6348: GPIO32-GPIO36
     * 6358: GPIO32-GPIO37 */
    if (ucGpioNum >= 32)
    {
        ucGpioNum = ucGpioNum - 32;
    	ucGpioValue = GPIO->GPIOio_high;        
    }
    else
#endif
    {
    	ucGpioValue = GPIO->GPIOio;        
    }
    ucGpioValue = (ucGpioValue >> ucGpioNum) & 1;
    return ucGpioValue; 
}

/**************************************************************************
 * 函数名  :   kerSysGetBoardVersion
 * 功能    :   GPIO获取Board版本号(提供给内核态调用)
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    正确  :  Board版本号 
 *              错误  :  NOT_SUPPROT_GET_VERSION
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
                l68693 2009-11-11修改
 *  GPIO4  GPIO5
 *    0     0   -->    HG556a 55VDFA  (Atheros WLAN)
 *    1     1   -->    HG556a 56ZBRB (RaLink WLAN cost down)
 ***************************************************************************/
unsigned char kerSysGetBoardVersion(void)
{
    int iBoardVerNum = 0, iTmpVersion = 0;  
#if 0
    int i = 0;
    int iRet = 0;      
    unsigned short usGPIOValue;

    /* 判断硬件是否支持通过读取GPIO获取Board version*/
    if(BP_SUCCESS != BpIsSupportBoardVersion())
    {
        return NOT_SUPPROT_GET_VERSION;
    }

    for (i = 0; i < BOARD_VERSION_GPIO_NUMS; i++)
    {        
        iRet = BpGetBoardVesionGpio(i, &usGPIOValue);
        
        if (BP_SUCCESS == iRet)
        {
            iTmpVersion = kerSysGetGPIO(usGPIOValue);
            
            iBoardVerNum = ((iTmpVersion << i) | iBoardVerNum);                
        }
        else
        {
            return NOT_SUPPROT_GET_VERSION;
        }
    }
#else
    iTmpVersion = kerSysGetGPIO(BP_GPIO_5_AL) << 1;
    iBoardVerNum = iTmpVersion | kerSysGetGPIO(BP_GPIO_4_AL);
#endif

    return iBoardVerNum;        
}

/**************************************************************************
 * 函数名  :   kerSysGetBoardManufacturer
 * 功能    :   GPIO获取单板的制造商
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    单板制造商号 
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-08-10创建  

 * GPIO35  GPIO33
 *   1      1        -->    Huawei
 *   1      0        -->    Alpha  
 ***************************************************************************/
unsigned char kerSysGetBoardManufacturer(void)
{
    unsigned char ucTmpManufactuer = 0;

    ucTmpManufactuer = kerSysGetGPIO(BP_GPIO_35_AH);
    ucTmpManufactuer = ((ucTmpManufactuer << 1) | kerSysGetGPIO(BP_GPIO_33_AH));    

    return  ucTmpManufactuer;
}


/**************************************************************************
 * 函数名  :   kerSysGetPCBVersion
 * 功能    :   GPIO获取PCB版本号(提供给内核态调用)
 *
 * 输入参数:   无           
 * 输出参数:   无
 *
 * 返回值  :    正确  :  PCB版本号 
 *              错误  :  NOT_SUPPROT_GET_VERSION
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
unsigned char kerSysGetPCBVersion(void)
{    
    int i = 0;
    int iRet = 0;      
    unsigned short usGPIOValue;
    int iPCBVerNum = 0, iTmpVersion = 0;        
    char pPCBVersion[] = {'A','B','C','D','E','F', 'G','0'};
    
    /* 判断硬件是否支持通过读取GPIO获取Board version*/
    if(BP_SUCCESS != BpIsSupportBoardVersion())
    {
        printk("\r\n Not support get board version.\n");
        return NOT_SUPPROT_GET_VERSION;
    }

    for (i = 0; i < PCB_VERSION_GPIO_NUMS; i++)
    {        
        iRet = BpGetPCBVesionGpio(i, &usGPIOValue);
        if (BP_SUCCESS == iRet)
        {
            iTmpVersion = kerSysGetGPIO(usGPIOValue);
            iPCBVerNum = ((iTmpVersion << i) | iPCBVerNum);                
        }
        else
        {
            return NOT_SUPPROT_GET_VERSION;
        }
    }
    return (char)pPCBVersion[iPCBVerNum];       
}

unsigned int kerSysMonitorPollHook( struct file *f, struct poll_table_struct *t)
{
    int mask = (*g_orig_fop_poll) (f, t);

    if( g_wakeup_monitor == 1 && g_monitor_file == f )
    {
        /* If g_wakeup_monitor is non-0, the user mode application needs to
         * return from a blocking select function.  Return POLLPRI which will
         * cause the select to return with the exception descriptor set.
         */
        mask |= POLLPRI;
        g_wakeup_monitor = 0;
    }

    return( mask );
}

/* Put the user mode application that monitors link state on a run queue. */
void kerSysWakeupMonitorTask( void )
{
    g_wakeup_monitor = 1;
   // printk(KERN_NOTICE "monitor waked up by \"%s\" (pid %i)\n", current->comm, current->pid);
    if( g_monitor_task )
        wake_up_process( g_monitor_task );
}

PFILE_TAG getTagFromPartition(int imageNumber)
{
    static unsigned char sectAddr1[sizeof(FILE_TAG)];
    static unsigned char sectAddr2[sizeof(FILE_TAG)];
    int blk = 0;
    UINT32 crc;
    PFILE_TAG pTag = NULL;
    unsigned char *pBase = flash_get_memptr(0);
    unsigned char *pSectAddr = NULL;

    /* The image tag for the first image is always after the boot loader.
     * The image tag for the second image, if it exists, is at one half
     * of the flash size.
     */
    if( imageNumber == 1 )
    {
        blk = flash_get_blk((int) (pBase + FLASH_LENGTH_BOOT_ROM));
        pSectAddr = sectAddr1;
    }
    else
        if( imageNumber == 2 )
        {
            blk = flash_get_blk((int) (pBase + (flash_get_total_size() / 2)));
            pSectAddr = sectAddr2;
        }

    if( blk )
    {
        memset(pSectAddr, 0x00, sizeof(FILE_TAG));
        flash_read_buf((unsigned short) blk, 0, pSectAddr, sizeof(FILE_TAG));
        crc = CRC32_INIT_VALUE;
        crc = getCrc32(pSectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
        pTag = (PFILE_TAG) pSectAddr;
        if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
            pTag = NULL;
    }

    return( pTag );
}

int getPartitionFromTag( PFILE_TAG pTag )
{
    int ret = 0;

    if( pTag )
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        int sequence = simple_strtoul(pTag->imageSequence,  NULL, 10);
        int sequence1 = (pTag1) ? simple_strtoul(pTag1->imageSequence, NULL, 10)
            : -1;
        int sequence2 = (pTag2) ? simple_strtoul(pTag2->imageSequence, NULL, 10)
            : -1;

        if( pTag1 && sequence == sequence1 )
            ret = 1;
        else
            if( pTag2 && sequence == sequence2 )
                ret = 2;
    }

    return( ret );
}

static PFILE_TAG getBootImageTag(void)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        char *p;
        char bootPartition = BOOT_LATEST_IMAGE;
        NVRAM_DATA nvramData;

        memcpy((char *) &nvramData, (char *) get_nvram_start_addr(),
            sizeof(nvramData));
        for( p = nvramData.szBootline; p[2] != '\0'; p++ )
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }

        if( bootPartition == BOOT_LATEST_IMAGE )
            pTag = (sequence2 > sequence1) ? pTag2 : pTag1;
        else /* Boot from the image configured. */
            pTag = (sequence2 < sequence1) ? pTag2 : pTag1;
    }
    else
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;

    return( pTag );
}

static void UpdateImageSequenceNumber( unsigned char *imageSequence )
{
    int newImageSequence = 0;
    PFILE_TAG pTag = getTagFromPartition(1);

    if( pTag )
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    pTag = getTagFromPartition(2);
    if(pTag && simple_strtoul(pTag->imageSequence, NULL, 10) > newImageSequence)
        newImageSequence = simple_strtoul(pTag->imageSequence, NULL, 10);

    newImageSequence++;
    sprintf(imageSequence, "%d", newImageSequence);
}

static int flashFsKernelImage( int destAddr, unsigned char *imagePtr,
    int imageLen )
{
    int status = 0;
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int rootfsAddr = simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
    int kernelAddr = simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
    char *p;
    char *tagFs = imagePtr;
    unsigned int baseAddr = (unsigned int) flash_get_memptr(0);
    unsigned int totalSize = (unsigned int) flash_get_total_size();
    unsigned int availableSizeOneImg = totalSize -
        ((unsigned int) rootfsAddr - baseAddr) - FLASH_RESERVED_AT_END;
    unsigned int reserveForTwoImages =
        (FLASH_LENGTH_BOOT_ROM > FLASH_RESERVED_AT_END)
        ? FLASH_LENGTH_BOOT_ROM : FLASH_RESERVED_AT_END;
    unsigned int availableSizeTwoImgs =
        (totalSize / 2) - reserveForTwoImages;
    unsigned int newImgSize = simple_strtoul(pTag->rootfsLen, NULL, 10) +
        simple_strtoul(pTag->kernelLen, NULL, 10);
    PFILE_TAG pCurTag = getBootImageTag();
    UINT32 crc = CRC32_INIT_VALUE;
    unsigned int curImgSize = 0;
    NVRAM_DATA nvramData;
    unsigned int ulSectorSize = 0;
    int fsBlk = 0;
    char acFlag[8];
    int  lFlag = 0;

    memset(acFlag, 0, sizeof(acFlag));
    strcpy(acFlag, "3");

    memcpy((char *)&nvramData, (char *)get_nvram_start_addr(),sizeof(nvramData));

    if( pCurTag )
    {
        curImgSize = simple_strtoul(pCurTag->rootfsLen, NULL, 10) +
            simple_strtoul(pCurTag->kernelLen, NULL, 10);
    }

    if( newImgSize > availableSizeOneImg)
    {
        printk("Illegal image size %d.  Image size must not be greater "
            "than %d.\n", newImgSize, availableSizeOneImg);
        return -1;
    }

    // If the current image fits in half the flash space and the new
    // image to flash also fits in half the flash space, then flash it
    // in the partition that is not currently being used to boot from.
    /*if( curImgSize <= availableSizeTwoImgs &&
        newImgSize <= availableSizeTwoImgs &&
        getPartitionFromTag( pCurTag ) == 1 )
    {
        // Update rootfsAddr to point to the second boot partition.
        int offset = (totalSize / 2) + TAG_LEN;

        printk("update slave image.......\r\n");
        lFlag = 1;
        strcpy(acFlag, "4");
        
        sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset + (kernelAddr - rootfsAddr));
        kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr);

        sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%lu",
            (unsigned long) IMAGE_BASE + offset);
        rootfsAddr = baseAddr + offset;
    }*/

 
    kerSysVariableSet(acFlag, sizeof(acFlag), 256);


    UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
    crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
    *(unsigned long *) &((PFILE_TAG) tagFs)->tagValidationToken[0] = crc;

#if 0
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs,
        TAG_LEN + newImgSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }
#endif

    /*l65130 2008-01-17 如果写最后一块时，系统掉电，而TAG已经写完毕，那就没
      招了，BCM FLASH架构存在问题 */

    /*修改写FLASH顺序，最后烧写包含TAG的一块*/
    fsBlk = flash_get_blk(rootfsAddr-TAG_LEN);
    ulSectorSize = flash_get_sector_size(fsBlk);
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN+ulSectorSize), tagFs+ulSectorSize,
        TAG_LEN + newImgSize - ulSectorSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }

    /*烧写包含TAG的一块*/
    if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs, ulSectorSize)) != 0 )
    {
        printk("Failed to flash root file system. Error: %d\n", status);
        return status;
    }

#ifdef VDF_HG556
    setAllLedOn();
#endif

    for( p = nvramData.szBootline; p[2] != '\0'; p++ )
        if( p[0] == 'p' && p[1] == '=' && p[2] != BOOT_LATEST_IMAGE )
        {
            UINT32 crc = CRC32_INIT_VALUE;
    
            // Change boot partition to boot from new image.
            p[2] = BOOT_LATEST_IMAGE;

            nvramData.ulCheckSum = 0;
            crc = getCrc32((char *)&nvramData, (UINT32) sizeof(NVRAM_DATA), crc);      
            nvramData.ulCheckSum = crc;
            kerSysNvRamSet( (char *) &nvramData, sizeof(nvramData), 0);
            break;
        }

    if (lFlag)
    {
        strcpy(acFlag, "2");
    }
    else
    {
        strcpy(acFlag, "1");
    }

    kerSysVariableSet(acFlag, sizeof(acFlag), 256);
    
    return(status);
}

PFILE_TAG kerSysImageTagGet(void)
{
    return( getBootImageTag() );
}

//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
static int board_ioctl( struct inode *inode, struct file *flip,
                        unsigned int command, unsigned long arg )
{
    int ret = 0;
    BOARD_IOCTL_PARMS ctrlParms;
    unsigned char ucaMacAddr[NVRAM_MAC_ADDRESS_LEN];
    /* HUAWEI HGW s48571 2008年3月10日 HSPA记录log add begin:*/
    static int iHspaTrafficMode = MODE_NONE;
    /* HUAWEI HGW s48571 2008年3月10日 HSPA记录log add end.*/
    /*minpengwei 20100526*/
	int iWriteRetryTimes = 0;
	/*minpengwei 20100526*/

    switch (command) 
    {
        case BOARD_IOCTL_FLASH_INIT:
            // not used for now.  kerSysBcmImageInit();
            break;

        case BOARD_IOCTL_FLASH_WRITE:
            
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
                down_interruptible(&flash_sem_lock);
                /*minpengwei 20100526*/
				while(FLASH_WRITING == s_iFlashWriteFlg)
				{
				
					 printk("Flash Write confliction, wait...\n");
					 iWriteRetryTimes++;
					 if(10 <= iWriteRetryTimes)
					 {
						printk("Flash Write failed over 10 times.\n");
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        up(&flash_sem_lock);
						return ret;
					 }
					 msleep(1000);
				}
                /*minpengwei 20100526*/
                s_iFlashWriteFlg = FLASH_WRITING;
                NVRAM_DATA SaveNvramData;
                PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();

                /* start of y42304 20061230: 有VOIP业务期间禁止除了升级之外其它写FLASH操作 */
                if( (ctrlParms.action == SCRATCH_PAD) 
                   || (ctrlParms.action == PERSISTENT)                    
                   || (ctrlParms.action == VARIABLE) 
                   || (ctrlParms.action == NVRAM)
                   /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                   || (ctrlParms.action == PSI_BACKUP)
                   /* end ----- Add Persistent storage interface backup flash partition by w69233 */
                   || (ctrlParms.action == FIX)
                   || (ctrlParms.action == AVAIL))
                {
                    if (g_sVoipServiceStatus == 1)
                    {
                        printk("\nSystem is on VOIP service.\n");
						/*minpengwei 20100526 remove func*/
						/*
                        printk("\nSystem is on VOIP service, forbid to save flash,\nPlease try again later...\n");                                                
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        s_iFlashWriteFlg = FLASH_IDLE;                          
                       return ret;
					   */
                    }                    
                }
                /* end of y42304 20061230 */

                /* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
                /* 由于写flash时进程调度可能会被暂停，所以开始写flash时需要禁止进程调度状态监控 */
                dg_setScheduleState(DG_DISABLE_SCHED_MON);
                /* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */  

                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        
                        if (ctrlParms.offset == -1)
                        {
                              ret =  kerSysScratchPadClearAll();
                        }
                        else
                        {
                              ret = kerSysScratchPadSet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        }
                        break;

                    case PERSISTENT:              
                        ret = kerSysPersistentSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                   
                        break;

                    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                    case PSI_BACKUP:
                        ret = kerSysPsiBackupSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                   
                        break;
                    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

                     /*start of  增加FLASH VA分区porting by l39225 20060504*/
                    case  VARIABLE:
                    	      ret = kerSysVariableSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                          break;
                    /*end of  增加FLASH VA分区porting by l39225 20060504*/
                    
                    case NVRAM:                 
                        ret = kerSysNvRamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case BCM_IMAGE_CFE:
                        g_iReset = 0;
			            ctrlParms.result = -1;
                        
                        if( ctrlParms.strLen <= 0 || ctrlParms.strLen > FLASH_LENGTH_BOOT_ROM )
                        {
                            printk(KERN_ERR "Illegal CFE size [%d]. Size allowed: [%d]\n",
                                ctrlParms.strLen, FLASH_LENGTH_BOOT_ROM);
                            ret = -1;
                            g_iReset = 1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        // set memory type field
                        BpGetSdramSize( (unsigned long *) &ctrlParms.string[SDRAM_TYPE_ADDRESS_OFFSET] );
                        // set thread number field
                        BpGetCMTThread( (unsigned long *) &ctrlParms.string[THREAD_NUM_ADDRESS_OFFSET] );

                        /* HUAWEI HGW l39225 2005年10月19日 poweron  add begin:*/                    

                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:
                        kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                        */
                        //kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */

                        /* HUAWEI HGW l39225 2005年10月19日 poweron  add end:*/
                        ret = kerSysBcmImageSet(ctrlParms.offset + BOOT_OFFSET, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }

			     ctrlParms.result = 0;
                        g_iReset = 1;
                        break;
                        
                    case BCM_IMAGE_FS:
			     ctrlParms.result = -1;
                        g_iReset = 0;
			    
                    	   /*start of GPIO 3.4.2 porting by l39225 20060504*/
                    
                          //kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                           
				/*
                           kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                          */
                          
                           /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
                    	   /*end of GPIO 3.4.2 porting by l39225 20060504*/
                        if( (ret = flashFsKernelImage( ctrlParms.offset,
                            ctrlParms.string, ctrlParms.strLen)) == 0 )
                        {
                            ctrlParms.result = 0;
                            kerSysMipsSoftReset();
                        }

                        g_iReset = 1;
                        break;

                    case BCM_IMAGE_KERNEL:  // not used for now.
                        break;
                    case BCM_IMAGE_WHOLE:
                        g_iReset = 0;
                        if(ctrlParms.strLen <= 0)
                        {
                            printk("Illegal flash image size [%d].\n", ctrlParms.strLen);
                            ret = -1;
                            break;
                        }

                        // save NVRAM data into a local structure
                        memcpy( &SaveNvramData, pNvramData, sizeof(NVRAM_DATA) );

                        if (ctrlParms.offset == 0) {
                            ctrlParms.offset = FLASH_BASE;
                        }

                        /*start of GPIO 3.4.2 porting by l39225 20060504*/
                        /* HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify begin:*/
                        //kerSysLedCtrl(kLedPower,kLedStateFastBlinkContinues);
                        
                        /* kerSysLedCtrl(kLedAdsl,kLedStateFastBlinkContinues);
                         HUAWEI HGW s48571 2008年1月18日 Hardware Porting modify end. */
                    	   /*end of GPIO 3.4.2 porting by l39225 20060504*/
                        ret = kerSysBcmImageSet(ctrlParms.offset, ctrlParms.string, ctrlParms.strLen);

                        // if nvram is not valid, restore the current nvram settings
                        if( BpSetBoardId( pNvramData->szBoardId ) != BP_SUCCESS &&
                            *(unsigned long *) pNvramData == NVRAM_DATA_ID )
                        {
                            kerSysNvRamSet((char *) &SaveNvramData, sizeof(SaveNvramData), 0);
                        }
                        g_iReset = 1;
			  
                        kerSysMipsSoftReset();
                        break;

                    /*start l65130 20080328 for more partitions*/
                    case FIX:
                        ret = kerSysFixSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    case AVAIL:
                        ret = kerSysAvailSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end l65130 20080328 for more partitions*/

                    /*start of 增加防火墙日志写空间 by c00131380 at 080911*/
                    case FIREWALL_LOG:
                        ret = kerSysFirewalllogSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end of 增加防火墙日志写空间 by c00131380 at 080911*/

                    /*start of 增加系统日志保存by c00131380 at 080926*/
                    case SYSTEM_LOG:
                        ret = kerSysSystemLogSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end of 增加系统日志保存by c00131380 at 080926*/                        
                    
                    /*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
                    case WLAN_PARAM:
                        ret = kerSysWlanParamSet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
                       
                    default:
                        ret = -EINVAL;
                        printk("flash_ioctl_command: invalid command %d\n", ctrlParms.action);
                        break;
                }

                /* start of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */
                /* 写完flash在打开监控标志 */
                dg_setScheduleState(DG_ENABLE_SCHED_MON);
                /* end of maintain dying gasp by liuzhijie 00028714 2006年5月20日 */

                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                s_iFlashWriteFlg = FLASH_IDLE;
                up(&flash_sem_lock);
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_FLASH_READ:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                down_interruptible(&flash_sem_lock);
                switch (ctrlParms.action)
                {
                    case SCRATCH_PAD:
                        ret = kerSysScratchPadGet(ctrlParms.string, ctrlParms.buf, ctrlParms.offset);
                        break;

                    case PERSISTENT:
                        ret = kerSysPersistentGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
                    case PSI_BACKUP:
                        ret = kerSysPsiBackupGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /* end ----- Add Persistent storage interface backup flash partition by w69233 */

                    /*start of  增加FLASH VA分区porting by l39225 20060504*/
                    case  VARIABLE:
                    	      ret = kerSysVariableGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                          break;
                    /*end of  增加FLASH VA分区porting by l39225 20060504*/

                    case NVRAM:
                        ret = kerSysNvRamGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;

                    case FLASH_SIZE:
                        ret = kerSysFlashSizeGet();
                        break;

                    /* start of y42304 added 20060814: 提供获取flash FILE_TAG的接口给应用*/                                         
                    case GET_FILE_TAG_FROM_FLASH:
                    {
                        PFILE_TAG stFileTag = getTagFromPartition(1);
                        if (NULL != stFileTag)
                        {                    
                            __copy_to_user(ctrlParms.string, stFileTag, sizeof(FILE_TAG));                    
                            ctrlParms.result = 0;
                            ret = 0;
                        }
                        else
                        {  
                            ctrlParms.result = -1;
                            ret = -EFAULT;                    
                        }                
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));   
                        
                        break;
                    }                               
                    /* start of y42304 added 20060814 */
                    
                    case BCM_IMAGE_FS:
                    {                        
                        ctrlParms.result = kerSysGetFsImageFromFlash(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                                    
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                        break;
                    }

                    case BCM_IMAGE_KERNEL:
                    {
                        ctrlParms.result = kerSysGetKernelImageFromFlash(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);                                                            
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,  sizeof(BOARD_IOCTL_PARMS));                           
                        ret = 0;
                        break;
                    }
                    
                    /*start l65130 20080328 for more partitions*/
                    case FIX:
                        ret = kerSysFixGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    case AVAIL:
                        ret = kerSysAvailGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end l65130 20080328 for more partitions*/
                    
                    /*start of 增加防火墙日志写空间 by c00131380 at 080911*/
                    case FIREWALL_LOG:
                        ret = kerSysFirewalllogGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end of 增加防火墙日志写空间 by c00131380 at 080911*/

                    /*start of 增加系统日志保存by c00131380 at 080926*/
                    case SYSTEM_LOG:
                        ret = kerSysSystemLogGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                        break;
                    /*end of 增加系统日志保存by c00131380 at 080926*/
                    
                    /*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
                    case WLAN_PARAM:
                         ret = kerSysWlanParamGet(ctrlParms.string, ctrlParms.strLen, ctrlParms.offset);
                         break;
                    /*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/

                    default:
                        ret = -EINVAL;
                        printk("Not supported.  invalid command %d\n", ctrlParms.action);
                        break;
                }
                ctrlParms.result = ret;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                up(&flash_sem_lock);
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_NR_PAGES:
            ctrlParms.result = nr_free_pages() + get_page_cache_size();
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_DUMP_ADDR:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                dumpaddr( (unsigned char *) ctrlParms.string, ctrlParms.strLen );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_SET_MEMORY:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                unsigned long  *pul = (unsigned long *)  ctrlParms.string;
                unsigned short *pus = (unsigned short *) ctrlParms.string;
                unsigned char  *puc = (unsigned char *)  ctrlParms.string;
                switch( ctrlParms.strLen )
                {
                    case 4:
                        *pul = (unsigned long) ctrlParms.offset;
                        break;
                    case 2:
                        *pus = (unsigned short) ctrlParms.offset;
                        break;
                    case 1:
                        *puc = (unsigned char) ctrlParms.offset;
                        break;
                }
                dumpaddr( (unsigned char *) ctrlParms.string, sizeof(long) );
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;
      
        case BOARD_IOCTL_MIPS_SOFT_RESET:
            kerSysMipsSoftReset();
            break;

        case BOARD_IOCTL_LED_CTRL:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
	            kerSysLedCtrl((BOARD_LED_NAME)ctrlParms.strLen, (BOARD_LED_STATE)ctrlParms.offset);
	            ret = 0;
	        }
            break;

        case BOARD_IOCTL_GET_ID:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                if( ctrlParms.string )
                {
                    char *p = (char *)get_system_type();                      
                    unsigned char ucHardwareType = GetHarewareType();

                    switch(ucHardwareType)
                    {    
                     case NOT_SUPPROT_GET_VERSION: /* 非华为自研硬件 */    
                        if( strlen(p) + 1 < ctrlParms.strLen )
                        {
                            ctrlParms.strLen = strlen(p) + 1;
                        }
                        __copy_to_user(ctrlParms.string, p, ctrlParms.strLen);                        
                        break;

                     case HG55VDFA:
			         case HG56BZRB:
			  	         ctrlParms.strLen = sizeof("HW556");                        
                         __copy_to_user(ctrlParms.string, "HW556", ctrlParms.strLen);
 
                    default: 
                        break;            
                    }    
                }            
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }    
            
            break;                        

       case BOARD_IOCTL_GET_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                ctrlParms.result = kerSysGetMacAddress( ucaMacAddr,
                    ctrlParms.offset );

                if( ctrlParms.result == 0 )
                {
                    __copy_to_user(ctrlParms.string, ucaMacAddr,
                        sizeof(ucaMacAddr));
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_RELEASE_MAC_ADDRESS:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                if (copy_from_user((void*)ucaMacAddr, (void*)ctrlParms.string, \
                     NVRAM_MAC_ADDRESS_LEN) == 0) 
                {
                    ctrlParms.result = kerSysReleaseMacAddress( ucaMacAddr );
                }
                else
                {
                    ctrlParms.result = -EACCES;
                }

                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,
                    sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
                ret = -EFAULT;
            break;

        case BOARD_IOCTL_GET_PSI_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulPsiSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

        case BOARD_IOCTL_GET_SDRAM_SIZE:
            ctrlParms.result = (int) g_pNvramInfo->ulSdramSize;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

       
        /* start of BOARD update by y42304 20060519: 解决通过Telnet做装备测试*/            
        case BOARD_IOCTL_EQUIPMENT_TEST:            
        {
            down_interruptible(&flash_sem_lock);
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
            {
                PNVRAM_DATA pNvramData = (PNVRAM_DATA) get_nvram_start_addr();
                
                /* start of y42304 20061230: 有VOIP业务期间禁止除了升级之外其它写FLASH操作 */
                if( (ctrlParms.action == SET_BASE_MAC_ADDRESS) 
                   || (ctrlParms.action == SET_MAC_AMOUNT)                    
                   || (ctrlParms.action == SET_BOARD_ID) 
                   || (ctrlParms.action == SET_SERIAL_NUMBER) )
                {
                    if (g_sVoipServiceStatus == 1)
                    {
                          printk("\nSystem is on VOIP service.\n");
						/*minpengwei 20100526 remove func*/
						/*
                        printk("\nSystem is on VOIP service, forbid to save flash,\nPlease try again later...\n");                                                
                        ret = -1;
                        ctrlParms.result = ret;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       return ret;
					   */
                    }                    
                }
                /* end of y42304 20061230 */
                
                switch (ctrlParms.action)
                {
                    case SET_BASE_MAC_ADDRESS:       /* 设置基mac地址*/
                    {      
                        NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                 
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.ucaBaseMacAddr, 0, NVRAM_MAC_ADDRESS_LEN);
                        memcpy(SaveNvramData.ucaBaseMacAddr, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;                           	                        	         	
                    	break;
                    }   
                    case GET_BASE_MAC_ADDRESS:     /* 获取基mac地址 */
                    {
                    	unsigned char tmpMacAddress[NVRAM_MAC_ADDRESS_LEN+1];
                        tmpMacAddress[0]='\0';
               
                        memcpy(tmpMacAddress,(unsigned char *)(pNvramData->ucaBaseMacAddr),NVRAM_MAC_ADDRESS_LEN);
                        tmpMacAddress[NVRAM_MAC_ADDRESS_LEN]='\0';
                        
                        ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, tmpMacAddress, NVRAM_MAC_ADDRESS_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        ret = 0;
                    	break;
                    }
                    case SET_MAC_AMOUNT:    /* 更改mac数目(11 - 32) */
                    {
                    	NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                          
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        SaveNvramData.ulNumMacAddrs = ctrlParms.offset;
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;                    	
                    	break;
                    }
                    case GET_MAC_AMOUNT:    /* 获取mac地址数目 */
                    {
                    	ctrlParms.result = pNvramData->ulNumMacAddrs;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;   
                    	break;
                    }
                    case GET_ALL_BOARD_ID_NAME:
                    {
                    	ctrlParms.result = BpGetBoardIds(ctrlParms.string, ctrlParms.strLen);                 
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0; 
                    	break;
                    }
                    case GET_PCB_VERSION:
                    {
                    	ctrlParms.result  = (int)kerSysGetPCBVersion();                     
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0; 
                    	break;
                    }
                    case GET_BOARD_VERSION:
                    {           
                        /*nobody use this now*/
                        /*
                         * HG556 修改获取版本号为GPIO5 GPIO4
                         *  0        0          --> HG55VDFA   Atheros 
                         *  1        1          --> HG56BZRB   RaLink
                         */
                        char tmpBoardVersion[16];                        
                        unsigned char ucHardwareType = GetHarewareType();
                        memset(tmpBoardVersion, 0, sizeof(tmpBoardVersion));

			            switch (ucHardwareType)
                        {
                         case HG55VDFA: /* HG556a Atheros板 */
                             memcpy(tmpBoardVersion, "HG55VDFA", sizeof("HG55VDFA"));
                             break;
                         case HG56BZRB: /* HG556a RaLink板 */
                              memcpy(tmpBoardVersion, "HG56BZRB", sizeof("HG56BZRB"));
                              break;
                         case NOT_SUPPROT_GET_VERSION: /* 未知硬件版本号 */                                
                             memcpy(tmpBoardVersion, "HG556Unkown", sizeof("HG556Unkown"));                                                                                   
                             break;
                                                           
                         default:                             
                              memcpy(tmpBoardVersion, "N/A", sizeof("N/A"));                                 
                              break;            
                         }

/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/

                        __copy_to_user(ctrlParms.string, tmpBoardVersion, sizeof(tmpBoardVersion));
                    	ctrlParms.result = 0;              
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0;   
                    	break;
                    }
                    case SET_BOARD_ID: /* 提供给telnet更改BOARD ID */
                    {
                    	NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;
                
                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.szBoardId, 0, NVRAM_BOARD_ID_STRING_LEN);
                        memcpy(SaveNvramData.szBoardId, ctrlParms.string, NVRAM_BOARD_ID_STRING_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                 
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                 
                        ret = 0;                    	
                    	break;
                    }
                    case GET_BOARD_ID:
                    {
                    	char tmpBoardId[NVRAM_BOARD_ID_STRING_LEN+1];
                        tmpBoardId[0]='\0';
                
                        memcpy(tmpBoardId,(unsigned char *)(pNvramData->szBoardId), NVRAM_BOARD_ID_STRING_LEN);
                        tmpBoardId[NVRAM_BOARD_ID_STRING_LEN]='\0';
                        ctrlParms.strLen= NVRAM_BOARD_ID_STRING_LEN;
                        __copy_to_user(ctrlParms.string, tmpBoardId, NVRAM_BOARD_ID_STRING_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

#ifdef VDF_HG556
    			        /*新增获取boardid不正确，点灯指示，Global需求start*/
    			        if (strcmp(tmpBoardId, "HW556"))
    		    	    {
		    	    		kerSysLedCtrl(kLedAdsl, kLedStateFastBlinkContinues);
			            }

			    /* 新增获取boardid不正确，点灯指示，Global需求end*/
#endif
				
                        ret = 0;  
                    	break;
                    }
                    case SET_SERIAL_NUMBER:   /* 设置单板序列号 */
                    {
                        NVRAM_DATA SaveNvramData;
                        unsigned long crc = CRC32_INIT_VALUE;

                        memcpy((char *)&SaveNvramData, (char *)pNvramData, sizeof(SaveNvramData));
                        memset(SaveNvramData.szSerialNumber, 0, NVRAM_SERIAL_NUMBER_LEN);
                        memcpy(SaveNvramData.szSerialNumber, ctrlParms.string,(ctrlParms.strLen < NVRAM_SERIAL_NUMBER_LEN) ? ctrlParms.strLen : NVRAM_SERIAL_NUMBER_LEN);
                        SaveNvramData.ulCheckSum = 0;
                        crc = getCrc32((char *)&SaveNvramData, (UINT32) sizeof(NVRAM_DATA), crc);
                        SaveNvramData.ulCheckSum = crc;
                        kerSysNvRamSet((char *)&SaveNvramData, sizeof(SaveNvramData), 0);
                        
                        /* start of BOARD added by y42304 20060713: */
                        ctrlParms.result = 0;
                        /* end of BOARD added by y42304 */                        
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;
                        break;
                    }
                    case GET_SERIAL_NUMBER:   /* 获取单板序列号 */
                    {                  
                        char tmpSerialNumber[NVRAM_SERIAL_NUMBER_LEN+1];                        
                        memset(tmpSerialNumber, 0, NVRAM_SERIAL_NUMBER_LEN+1);
                        tmpSerialNumber[0]='\0';
                        
                        memcpy(tmpSerialNumber,(unsigned char *)(pNvramData->chReserved+sizeof(pNvramData->chReserved)),NVRAM_SERIAL_NUMBER_LEN);
                        tmpSerialNumber[NVRAM_SERIAL_NUMBER_LEN]='\0';
                        ctrlParms.strLen = NVRAM_SERIAL_NUMBER_LEN;
                        __copy_to_user(ctrlParms.string, tmpSerialNumber, NVRAM_SERIAL_NUMBER_LEN);
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                        
                        ret = 0;                        
                        break;
                    }       
                    case GET_CPU_REVISION_ID:
                    {                        
                        ctrlParms.result = (int)(PERF->RevID & 0xFF);
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                        ret = 0;
                        
                        break;
                    }           
                    /* 提供给硬件人员查看单板的制造商 */
                    case GET_BOARD_MANUFACTURER_NAME:
                    {                        
                        /*
                         * GPIO35  GPIO33
                         *   1      1        -->    Huawei
                         *   1      0        -->    Alpha
                         */
                        char tmpManufactuerName[16];                        
                        unsigned char ucBoardManufacturer = kerSysGetBoardManufacturer();
                        memset(tmpManufactuerName, 0, sizeof(tmpManufactuerName));

                        switch(ucBoardManufacturer)
                        {    
                        case MANUFACTURER_HUAWEI:                                
                            memcpy(tmpManufactuerName, "Huawei", sizeof("Huawei"));                                 
                            break;
                            
                        case MANUFACTURER_ALPHA:                            
                            memcpy(tmpManufactuerName, "Alpha", sizeof("Alpha"));                                 
                            break;    

                        default:
                            memcpy(tmpManufactuerName, "N/A", sizeof("N/A"));                                                             
                            break;    
                           
                        }                        
                        __copy_to_user(ctrlParms.string, tmpManufactuerName, sizeof(tmpManufactuerName));
                    	ctrlParms.result = 0;              
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));

                        ret = 0;   
                        
                        break;
                    }       
                }               
            }           
            else
            {
                ret = -EFAULT;
            }
            up(&flash_sem_lock);
            break;	        	
        }   
        /* end of BOARD update by y42304 20060519: 解决通过Telnet做装备测试*/            

        case BOARD_IOCTL_GET_CHIP_ID:
            ctrlParms.result = (int) (PERF->RevID & 0xFFFF0000) >> 16;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;

    #ifdef WIRELESS   
        /* start of y42304 added 20060907:  通过复位键更改wlan的状态 */
        case BOARD_IOCTL_CHECK_WLAN_STATUS:            
            ctrlParms.result = g_sWlanFlag;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
	     WPS_FLAG = 0;
            break;
        /* start of y42304 added 20060907*/            
	/*start of 增加 ralink wps 检测 功能  by s53329 at  20090720*/
	case  BOARD_IOCTL_CHECK_WLAN_WPS_STATUS:

	     ctrlParms.result = WPS_FLAG;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
            break;
	/*end of 增加 ralink wps 检测 功能  by s53329 at  20090720*/
    #endif
        
        /* start of y42304 added 20061230: 提供给TAPI 更新是否在进行VOIP业务, 期间不允许写FLASH */
        case BOARD_IOCTL_SET_VOIP_SERVICE:                           
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
            {
                g_sVoipServiceStatus = ctrlParms.offset;      

                printk("Process[%s] set voip status to [%d]", current->comm, g_sVoipServiceStatus);
                
                ctrlParms.result = 0;              
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                ret = 0;      
            }
            
            break;   
        case BOARD_IOCTL_GET_VOIP_SERVICE:
            ctrlParms.result = g_sVoipServiceStatus;              
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
            ret = 0;    
            
            break;
        /* end of y42304 added 20061230 */
        case BOARD_IOCTL_GET_NUM_ENET: {
            ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
            int i, numeth = 0;
            if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
            for( i = 0; i < BP_MAX_ENET_MACS; i++) {
                if (EnetInfos[i].ucPhyType != BP_ENET_NO_PHY) {
                numeth++;
                }
            }
            ctrlParms.result = numeth;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,	 sizeof(BOARD_IOCTL_PARMS));   
            ret = 0;
            }
	        else {
	            ret = -EFAULT;
	        }
	        break;
            }

        case BOARD_IOCTL_GET_CFE_VER:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                char *vertag =  (char *)(FLASH_BASE + CFE_VERSION_OFFSET);
                if (ctrlParms.strLen < CFE_VERSION_SIZE) {
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = -EFAULT;
                }
                else if (strncmp(vertag, "cfe-v", 5)) { // no tag info in flash
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
                else {
                    ctrlParms.result = 1;
                    __copy_to_user(ctrlParms.string, vertag+CFE_VERSION_MARK_SIZE, CFE_VERSION_SIZE);
                    ctrlParms.string[CFE_VERSION_SIZE] = '\0';
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                    ret = 0;
                }
            }
            else {
                ret = -EFAULT;
            }
            break;

        case BOARD_IOCTL_GET_ENET_CFG:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ETHERNET_MAC_INFO EnetInfos[BP_MAX_ENET_MACS];
                if (BpGetEthernetMacInfo(EnetInfos, BP_MAX_ENET_MACS) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(EnetInfos)) {
                        __copy_to_user(ctrlParms.string, EnetInfos, sizeof(EnetInfos));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	            else {
	                ret = -EFAULT;
	            }
	            break;
            }
            else {
                ret = -EFAULT;
            }
            break;            

#if defined (WIRELESS)
        case BOARD_IOCTL_GET_WLAN_ANT_INUSE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                unsigned short antInUse = 0;
                if (BpGetWirelessAntInUse(&antInUse) == BP_SUCCESS) {
                    if (ctrlParms.strLen == sizeof(antInUse)) {
                        __copy_to_user(ctrlParms.string, &antInUse, sizeof(antInUse));
                        ctrlParms.result = 0;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                        ret = 0;
                    } else
	                    ret = -EFAULT;
                }
	        else {
	           ret = -EFAULT;
	        }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif            
        case BOARD_IOCTL_SET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {            	
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;            	
                ctrlParms.result = -EFAULT;
                ret = -EFAULT;
                if (ctrlParms.strLen == sizeof(unsigned long)) {                 	                    
                    board_ioc->eventmask |= *((int*)ctrlParms.string);                    
#if defined (WIRELESS)                    
                    if((board_ioc->eventmask & SES_EVENTS)) {
                            ctrlParms.result = 0;
                            ret = 0;
                    } 
#endif                                                
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));                        
                }
	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                        

        case BOARD_IOCTL_GET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
            	BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    __copy_to_user(ctrlParms.string, &board_ioc->eventmask, sizeof(unsigned long));
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;                
            
        case BOARD_IOCTL_UNSET_TRIGGER_EVENT:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(unsigned long)) {
                    BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;                	
                    board_ioc->eventmask &= (~(*((int*)ctrlParms.string)));                  
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            } 
            else {
                ret = -EFAULT;
            }
            break;      
            
#if defined (WIRELESS)

        case BOARD_IOCTL_SET_SES_LED:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.strLen == sizeof(int)) {
                    sesLed_ctrl(*(int*)ctrlParms.string);
                    ctrlParms.result = 0;
                    __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));   
                    ret = 0;
                } else
	            ret = -EFAULT;

	        break;
            }
            else {
                ret = -EFAULT;
            }
            break;            
#endif                                                            

        case BOARD_IOCTL_SET_MONITOR_FD:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                int fput_needed = 0;

                g_monitor_file = fget_light( ctrlParms.offset, &fput_needed );
                if( g_monitor_file ) {
                    /* Hook this file descriptor's poll function in order to set
                     * the exception descriptor when there is a change in link
                     * state.
                     */
                    g_monitor_task = current;
                    g_orig_fop_poll = g_monitor_file->f_op->poll;
                    g_monitor_file->f_op->poll = kerSysMonitorPollHook;
                }
            }
            break;

        case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
            kerSysWakeupMonitorTask();
            break;

        case BOARD_IOCTL_GET_VCOPE_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ((ctrlParms.result = BpGetVcopeGpio(ctrlParms.offset)) != BP_NOT_DEFINED) ? 0 : -EFAULT;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            }
            else {
                ret = -EFAULT;  
                ctrlParms.result = BP_NOT_DEFINED;
            }

            break;

        case BOARD_IOCTL_SET_CS_PAR: 
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                ret = ConfigCs(&ctrlParms);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            } 
            else {
                ret = -EFAULT;  
            }
            break;

        case BOARD_IOCTL_SET_PLL: 
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                SetPll(ctrlParms.strLen, ctrlParms.offset);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } 
            else {
                ret = -EFAULT;  
            }
            break;

        case BOARD_IOCTL_SET_GPIO:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                SetGpio(ctrlParms.strLen, ctrlParms.offset);
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            } 
            else {
                ret = -EFAULT;  
            }
            break;
         
                    
        /* start of maintain dying gasp by liuzhijie 00028714 2006年5月10日 */
        case BOARD_IOCTL_GET_DYINGGASP:
            if (0 == copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)))
            {
                __copy_to_user(ctrlParms.buf, 
                               DG_RESERVED_SDRAM_START_ADDR, 
                               DG_RESERVED_SDRAM_SIZE);
                ctrlParms.result = 0;
                __copy_to_user((BOARD_IOCTL_PARMS*)arg, 
                               &ctrlParms, 
                               sizeof(BOARD_IOCTL_PARMS));
                ret = 0;
            }
            else
            {
                ret = -EFAULT;
            }
        break;

        case BOARD_IOCTL_PUT_WEB_OP:
            if (0 == copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)))
            {
                dg_recordWebOp(ctrlParms.string, ctrlParms.strLen);
                ret = 0;
            }
            else
            {
                ret = -EFAULT;
            }
        break;

        case BOARD_IOCTL_PUT_CLI_OP:
            if (0 == copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)))
            {
                dg_recordCliOp(ctrlParms.string, ctrlParms.strLen);
                ret = 0;
            }
            else
            {
                ret = -EFAULT;
            }
        break;

        case BOARD_IOCTL_PUT_ITF_STATE:
            if (0 == copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)))
            {
                dg_recordIntfState(ctrlParms.string, ctrlParms.strLen);
                ret = 0;
            }
            else
            {
                ret = -EFAULT;
            }
        break;

        /* end of maintain dying gasp by liuzhijie 00028714 2006年5月10日 */
    
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
        case BOARD_IOCTL_SET_TRAFFIC_MODE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if (ctrlParms.offset < MODE_UNDEFINED && ctrlParms.offset >= MODE_NONE) 
                {
                    if(( MODE_PASSBY == g_nHspaTrafficMode )&& ( MODE_NORM != ctrlParms.offset ))
                    {
                        printk(KERN_INFO "HSPA: Origin HSPA mode is PASSBY, set error.\n");
                        ret = -EFAULT;
                    }
                    else
                    {
                    g_nHspaTrafficMode = ctrlParms.offset;
                    //printk("g_nHspaTrafficMode: %d\n",g_nHspaTrafficMode);
                    if ( iHspaTrafficMode != g_nHspaTrafficMode )
                    {
                        iHspaTrafficMode = g_nHspaTrafficMode;
                        if ( MODE_NONE == iHspaTrafficMode )
                        {
                            printk(KERN_CRIT "HSPA: No Available Network.\n");
                        }
                        else if ( MODE_2G == iHspaTrafficMode )
                        {
                            printk(KERN_CRIT "HSPA: 2G Network connected.\n");
                        }
                        else if (MODE_3G == iHspaTrafficMode)
                        {
                            printk(KERN_CRIT "HSPA: 3G Network connected.\n");
                            }
                            else if (MODE_NORM == iHspaTrafficMode)
                            {
                                printk(KERN_INFO "HSPA: enter NORM state.\n");
                            }
                            else if (MODE_PASSBY == iHspaTrafficMode)
                            {
                                printk(KERN_INFO "HSPA: enter passby state.\n");
                            }
                        }
                    }
                    ret = 0;
                } 
                else
                {
    	            ret = -EFAULT;
                }
	        break;
                
            }
            else {
                ret = -EFAULT;
            }
            break;            

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
          case BOARD_IOCTL_SET_EQUIPTEST_MODE:
		if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
		{
			/*l65130 2008-09-20 start*/

			if (ctrlParms.offset == TRUE) 
			{
				if (FALSE == g_nEquipTestMode)
				{
					g_nEquipTestMode = TRUE;
				}
				ret = 0;

			}
			else if (ctrlParms.offset == FALSE)
			{
			   	g_nEquipTestMode = FALSE;
				ret = 0;
			}
                   else
                   {
                    	ret = -EFAULT;
                	}
			/*l65130 2008-09-20 end*/
             	break;
                
            }
            else 
	      {
            		ret = -EFAULT;
            }
            break;            
    
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/

/* start of 增加扩展的MAC地址 by c00131380 at 080910 */
           case BOARD_IOCTL_SET_MEMMACINUSESTATUS://设置MAC的使用状态
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {                                
                   if (EXT_MAC_1 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed1 = ctrlParms.strLen;
                       if(1 == ctrlParms.strLen)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       }
                       else
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                       }
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_2 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed2 = ctrlParms.strLen;
                       if(1 == ctrlParms.strLen)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       }
                       else
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                       }
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_3 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed3 = ctrlParms.strLen;
                       if(1 == ctrlParms.strLen)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       }
                       else
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                       }
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_4 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed4 = ctrlParms.strLen;
                       if(1 == ctrlParms.strLen)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       }
                       else
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                       }
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }
               }
               else 
               {
                   ret = -EFAULT;
               }
               break;

           case BOARD_IOCTL_GET_MEMMACINUSESTATUS://获得内存MAC地址使用状态信息
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {
                   if (EXT_MAC_1 == ctrlParms.offset)
                   {
                       ctrlParms.result = g_pExtendMacInfo->inUsed1;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_2 == ctrlParms.offset)
                   {
                       ctrlParms.result = g_pExtendMacInfo->inUsed2;
                        __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_3 == ctrlParms.offset)
                   {
                       ctrlParms.result = g_pExtendMacInfo->inUsed3;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_4 == ctrlParms.offset)
                   {
                       ctrlParms.result = g_pExtendMacInfo->inUsed4;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }
               }
               else 
               {
                   ret = -EFAULT;
               }
               break;
           case BOARD_IOCTL_SET_MEMMACADDRESS://设置内存MAC地址
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {
                   int iLen;
                   int i;
                   unsigned char baseMac[NVRAM_MAX_MAC_STR_LEN];
                   if (EXT_MAC_1 == ctrlParms.offset)
                   {
                       //判断内存原MAC是0x00，还是有值，如果是原来是0x00则说明是新增MAC,需要增加
                       //内存可用MAC数和MAC总数，如果原来有值则只修改内容，不改变计数
                       if (EXT_MAC_NULL== g_pExtendMacInfo->inUsed1)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                           g_pExtendMacInfo->totalNum  = g_pExtendMacInfo->totalNum + 1;
                           g_pExtendMacInfo->inUsed1   = EXT_MAC_UNUSED;
                       }
                       memcpy(g_pExtendMacInfo->addMACAddress1, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);                     
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                           	                        	         	
                   	   break;
                   }
                   
                   if (EXT_MAC_2 == ctrlParms.offset)
                   {
                       if (EXT_MAC_NULL == g_pExtendMacInfo->inUsed2)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                           g_pExtendMacInfo->totalNum  = g_pExtendMacInfo->totalNum + 1;
                           g_pExtendMacInfo->inUsed2   = EXT_MAC_UNUSED;
                       }

                       memcpy(g_pExtendMacInfo->addMACAddress2, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                           	                        	         	
                   	   break;
                   }
                   
                   if (EXT_MAC_3 == ctrlParms.offset)
                   {
                       if (EXT_MAC_NULL == g_pExtendMacInfo->inUsed3)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                           g_pExtendMacInfo->totalNum  = g_pExtendMacInfo->totalNum + 1;
                           g_pExtendMacInfo->inUsed3   = EXT_MAC_UNUSED;
                       }

                       memcpy(g_pExtendMacInfo->addMACAddress3, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                           	                        	         	
                   	   break;
                   }
                   
                   if (EXT_MAC_4 == ctrlParms.offset)
                   {
                       if (EXT_MAC_NULL == g_pExtendMacInfo->inUsed4)
                       {
                           g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum + 1;
                           g_pExtendMacInfo->totalNum  = g_pExtendMacInfo->totalNum + 1;
                           g_pExtendMacInfo->inUsed4   = EXT_MAC_UNUSED;
                       }

                       memcpy(g_pExtendMacInfo->addMACAddress4, ctrlParms.string, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                           	                        	         	
                   	   break;
                   }
               }
               else 
               {
                   ret = -EFAULT;
               }
               break;
           case BOARD_IOCTL_GET_MEMMACADDRESS:// 获得内存MAC地址
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {
                   if (EXT_MAC_1 == ctrlParms.offset)
                   {                                             
                       ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, g_pExtendMacInfo->addMACAddress1, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                                                                     
                       break;
                   }
                   
                   if (EXT_MAC_2 == ctrlParms.offset)
                   {
                       ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, g_pExtendMacInfo->addMACAddress2, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                                                                     
                       break;
                   }
                   
                   if (EXT_MAC_3 == ctrlParms.offset)
                   {
                       ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, g_pExtendMacInfo->addMACAddress3, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                                                                     
                       break;
                   }
                   
                   if (EXT_MAC_4 == ctrlParms.offset)
                   {
                       ctrlParms.strLen= NVRAM_MAC_ADDRESS_LEN;
                        __copy_to_user(ctrlParms.string, g_pExtendMacInfo->addMACAddress4, NVRAM_MAC_ADDRESS_LEN);
                       ctrlParms.result = 0;
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                       
                       ret = 0;                                                                     
                       break;
                   }
               }
               else 
               {
                   ret = -EFAULT;
               }
               break;
           case BOARD_IOCTL_GET_NUMOFUNUSEDMACADDR://获得可用MAC地址数
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {
                   ctrlParms.result = StatUnUsedMacNum();
                   __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms,sizeof(BOARD_IOCTL_PARMS));
                   //打印当前内存可用MAC数和总数
                   #if 0
                   printk("the memory unUsedMacNum is %d\n",g_pExtendMacInfo->unUsedNum);
                   printk("the memory totalMacNum is %d\n",g_pExtendMacInfo->totalNum);
                   printk("the StatUnUsedMacNum is %d\n",StatUnUsedMacNum());
                   #endif
                   
                   ret = 0;                                                                     
                   break;
               }
               else 
               {
                   ret = -EFAULT;
               }
               break;
           case BOARD_IOCTL_DEL_EXTENDMACADDR://删除扩展MAC地址
               if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0)
               {
                   unsigned char macAddress[NVRAM_MAC_ADDRESS_LEN];   
                   memset(macAddress, 0x00, NVRAM_MAC_ADDRESS_LEN);
                   
                   if (EXT_MAC_1 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed1 = EXT_MAC_NULL;
                       g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum - 1;
                       //清除内存数据
                       memset(g_pExtendMacInfo->addMACAddress1,0x00,NVRAM_MAC_ADDRESS_LEN);
                       //清除FLASH数据
                       kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_1_START);
                       
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_2 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed2 = EXT_MAC_NULL;
                       g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum - 1;
                       memset(g_pExtendMacInfo->addMACAddress2,0x00,NVRAM_MAC_ADDRESS_LEN);
                       kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_2_START);
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_3 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed3 = EXT_MAC_NULL;
                       g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum - 1;
                       memset(g_pExtendMacInfo->addMACAddress3,0x00,NVRAM_MAC_ADDRESS_LEN);
                       kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_3_START);
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   }

                   if (EXT_MAC_4 == ctrlParms.offset)
                   {
                       g_pExtendMacInfo->inUsed4 = EXT_MAC_NULL;
                       g_pExtendMacInfo->unUsedNum = g_pExtendMacInfo->unUsedNum - 1;
                       g_pExtendMacInfo->totalNum = g_pExtendMacInfo->totalNum - 1;
                       memset(g_pExtendMacInfo->addMACAddress4,0x00,NVRAM_MAC_ADDRESS_LEN);
                       kerSysAvailSet(macAddress, MACADDITION_LEN,MACADDITION_VAR_4_START);
                       __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
                       ret = 0;
                       break;
                   } 
               }
               else 
               {
                  ret = -EFAULT;
               }
               break;
/* end of 增加扩展的MAC地址 by c00131380 at 080910 */
/* start of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
        case BOARD_IOCTL_SET_IGMPLOG_MODE:
            if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
                if ( ( ctrlParms.offset == TRUE) || ( ctrlParms.offset == FALSE )) 
                {
                    igmpw_syslog = ctrlParms.offset;
                    ret = 0;
                }
                else 
                {
                    ret = -EFAULT;
                }
                
            }else{
                 ret = -EFAULT;
            }
            break; 
/* end of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
	case BOARD_IOCTL_SET_TELNETD_PID:
        if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
        {
            if (ctrlParms.offset != 0)
            {
                telnetd_pid = ctrlParms.offset;
            }
            else
            {
                ctrlParms.result = -EFAULT;
            }
        }
        else
        {
            ret = -EINVAL;
        }
        break;
        
	case BOARD_IOCTL_GET_KEY_MODE:
		if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
		{
            ctrlParms.result = g_nKeyPressed;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
		else
		{
			ret = -EINVAL;
		}
		break;
        
	case BOARD_IOCTL_GET_EQUIPTEST_MODE:
		if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
		{
            ctrlParms.result = g_nEquipTestMode;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
		else
		{
			ret = -EINVAL;
		}
		break;
	/* j00100803 save fwlog to flash for HG556A */
	case BOARD_IOCTL_GET_VAR_REBOOT_BUTTON:
		if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) 
		{
            ctrlParms.result = gl_nIsRebootBtn;
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
        }
		else
		{
			ret = -EINVAL;
		}
	break;
	/* j00100803 save fwlog to flash for HG556A */
    default:
            ret = -EINVAL;
            ctrlParms.result = 0;
            printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
            break;

  } /* switch */

  return (ret);

} /* board_ioctl */

/***************************************************************************
 * SES Button ISR/GPIO/LED functions.
 ***************************************************************************/
#if defined (WIRELESS) 
#if 1 /* 2008/01/28 Jiajun Weng : New code from 3.12L.01 */
static Bool sesBtn_pressed(void)
{
//start of modify by y68191 for VDFC02_WPS
/*
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(sesBtn_gpio);
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;
    if( (sesBtn_gpio & BP_GPIO_NUM_MASK) >= 32 )
    {
    
    	printk("try to  cp gpio to reg\n");
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(sesBtn_gpio);
        gpio_io_reg = &GPIO->GPIOio_high;
    }	
    if (!(*gpio_io_reg & gpio_mask)){
        return POLLIN;
    }	
 */
 //end of modify by y68191 for VDFC02_WPS
     if(WPS_FLAG)
     return POLLIN;
     return 0;
}
static irqreturn_t sesBtn_isr(int irq, void *dev_id, struct pt_regs *ptregs)
{   
    if (sesBtn_pressed()){
        wake_up_interruptible(&g_board_wait_queue); 
        return IRQ_RETVAL(1);
    } else {
        return IRQ_RETVAL(0);    	
    }
	
    return IRQ_RETVAL(0);
}
static void __init sesBtn_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesBtn_gpio) == BP_SUCCESS )
    {
        printk("SES: Button GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void __init sesBtn_mapIntr(int context)
{    	
    if( BpGetWirelessSesExtIntr(&sesBtn_irq) == BP_SUCCESS )
    {
    	printk("SES: Button Interrupt 0x%x is enabled\n", sesBtn_irq);
    }
    else
    	return;
    	    
    sesBtn_irq = map_external_irq (sesBtn_irq) ;
    printk("sesBtn_irq==%d\n\r",sesBtn_irq);
    		
    if (BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, context, sesBtn_irq)) {
    	printk("SES: Interrupt mapping failed\n");
    }    
    BcmHalInterruptEnable(sesBtn_irq);
}

//modify by yinheng for WPS 080316
static unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{	

    if (sesBtn_pressed()){
	 
        return POLLIN;
    }	
    return 0;
}

static ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
//start of modify by y68191 for VDFC02_wps
    volatile unsigned int event=0;
    ssize_t ret=0;	

    if(!sesBtn_pressed()){
	//BcmHalInterruptEnable(sesBtn_irq);		
	    return ret;
    }	
    event = SES_EVENTS;
    __copy_to_user((char*)buffer, (char*)&event, sizeof(event));	
 //   BcmHalInterruptEnable(sesBtn_irq);	
    count -= sizeof(event);
    buffer += sizeof(event);
    ret += sizeof(event);	
    return ret;	
 //end  of modify by y68191 for VDFC02_wps
}

static void __init sesLed_mapGpio()
{	
    if( BpGetWirelessSesBtnGpio(&sesLed_gpio) == BP_SUCCESS )
    {
        printk("SES: LED GPIO 0x%x is enabled\n", sesBtn_gpio);    
    }
}

static void sesLed_ctrl(int action)
{

    //char status = ((action >> 8) & 0xff); /* extract status */
    //char event = ((action >> 16) & 0xff); /* extract event */        
    //char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */
    
    BOARD_LED_STATE led;
    
    if(sesLed_gpio == BP_NOT_DEFINED)
        return;
    	
    action &= 0xff; /* extract led */

    //printk("blinktype=%d, event=%d, status=%d\n",(int)blinktype, (int)event, (int)status);
            	
    switch (action) 
    {
        case SES_LED_ON:
            //printk("SES: led on\n");
            led = kLedStateOn;                                          
            break;
        case SES_LED_BLINK:
            //printk("SES: led blink\n");
            led = kLedStateSlowBlinkContinues;           		
            break;
        case SES_LED_OFF:
            default:
            //printk("SES: led off\n");
            led = kLedStateOff;  						
    }	
    
    kerSysLedCtrl(kLedSes, led);
}

static void __init ses_board_init()
{
    sesBtn_mapGpio();
//    sesBtn_mapIntr(0);
    sesLed_mapGpio();
}
static void __exit ses_board_deinit()
{
    if(sesBtn_irq)
        BcmHalInterruptDisable(sesBtn_irq);
}
#endif
#endif

/***************************************************************************
 * Dying gasp ISR and functions.
 ***************************************************************************/
#define KERSYS_DBG	printk

#if defined(CONFIG_BCM96348) || defined(CONFIG_BCM96338)
/* The BCM6348 cycles per microsecond is really variable since the BCM6348
 * MIPS speed can vary depending on the PLL settings.  However, an appoximate
 * value of 120 will still work OK for the test being done.
 */
#define	CYCLE_PER_US	120
#elif defined(CONFIG_BCM96358)
#define	CYCLE_PER_US	150
#endif
#define	DG_GLITCH_TO	(100*CYCLE_PER_US)
 
static void __init kerSysDyingGaspMapIntr()
{
    unsigned long ulIntr;
    	
    if( BpGetAdslDyingGaspExtIntr( &ulIntr ) == BP_SUCCESS ) 
    {
		BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, 0, INTERRUPT_ID_DG);
		BcmHalInterruptEnable( INTERRUPT_ID_DG );
    }
} 

void kerSysSetWdTimer(ulong timeUs)
{
	TIMER->WatchDogDefCount = timeUs * (FPERIPH/1000000);
	TIMER->WatchDogCtl = 0xFF00;
	TIMER->WatchDogCtl = 0x00FF;
}

ulong kerSysGetCycleCount(void)
{
    ulong cnt; 
#ifdef _WIN32_WCE
    cnt = 0;
#else
    __asm volatile("mfc0 %0, $9":"=d"(cnt));
#endif
    return(cnt); 
}

static Bool kerSysDyingGaspCheckPowerLoss(void)
{
    ulong clk0;
    ulong ulIntr;

    ulIntr = 0;
    clk0 = kerSysGetCycleCount();

    UART->Data = 'D';
    UART->Data = '%';
    UART->Data = 'G';

    do {
        ulong clk1;
        
        clk1 = kerSysGetCycleCount();		/* time cleared */
	/* wait a little to get new reading */
        while ((kerSysGetCycleCount()-clk1) < CYCLE_PER_US*2)
            ;
     } while ((PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET))) && ((kerSysGetCycleCount() - clk0) < DG_GLITCH_TO));

    if (!(PERF->IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET)))) {
        BcmHalInterruptEnable( INTERRUPT_ID_DG );
        KERSYS_DBG(" - Power glitch detected. Duration: %ld us\n", (kerSysGetCycleCount() - clk0)/CYCLE_PER_US);
        return 0;
    }

    return 1;
}

static void kerSysDyingGaspShutdown( void )
{
    kerSysSetWdTimer(1000000);
#if defined(CONFIG_BCM96348)
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | USBH_CLK_EN | SAR_CLK_EN);
#elif defined(CONFIG_BCM96358) 
    PERF->blkEnables &= ~(EMAC_CLK_EN | USBS_CLK_EN | SAR_CLK_EN);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id, struct pt_regs * regs)
#else
static unsigned int kerSysDyingGaspIsr(void)
#endif
{	
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dsl = NULL;	

    if (kerSysDyingGaspCheckPowerLoss()) {        

        /* first to turn off everything other than dsl */        
        list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
            tmp = list_entry(pos, CB_DGASP_LIST, list);
    	    if(strncmp(tmp->name, "dsl", 3)) {
    	        (tmp->cb_dgasp_fn)(tmp->context); 
    	    }else {
    		dsl = tmp;    		    	
    	    }       
        }  
        
        /* now send dgasp */
        if(dsl)
            (dsl->cb_dgasp_fn)(dsl->context); 

        /* reset and shutdown system */
        kerSysDyingGaspShutdown();

        // If power is going down, nothing should continue!

        while (1)
            ;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
return( IRQ_HANDLED );
#else
    return( 1 );
#endif
}

static void __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return;	
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);    
    g_cb_dgasp_list_head = new_node; 
		
} /* kerSysInitDyingGaspHandler */

static void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp; 
     	
    if(g_cb_dgasp_list_head == NULL)
        return;
        
    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
	kfree(tmp);
    }       

    kfree(g_cb_dgasp_list_head);	
    g_cb_dgasp_list_head = NULL;
    
} /* kerSysDeinitDyingGaspHandler */

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysRegisterDyingGaspHandler: list head is null\n");	
        return;    
    }
    
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: kerSysRegisterDyingGaspHandler: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);	    	
        return;
    }
       
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));    
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ);
    new_node->cb_dgasp_fn = (cb_dgasp_t)cbfn;
    new_node->context = context;
    list_add(&new_node->list, &g_cb_dgasp_list_head->list);
    
    printk("dgasp: kerSysRegisterDyingGaspHandler: %s registered \n", devname);
        	
} /* kerSysRegisterDyingGaspHandler */

void kerSysDeregisterDyingGaspHandler(char *devname)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;    
    
    if(g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: list head is null\n");
        return;	
    }

    if(devname == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: devname is null\n");
        return;	
    }
    
    printk("kerSysDeregisterDyingGaspHandler: %s is deregistering\n", devname);

    list_for_each(pos, &g_cb_dgasp_list_head->list) {    	
    	tmp = list_entry(pos, CB_DGASP_LIST, list);
    	if(!strcmp(tmp->name, devname)) {
            list_del(pos);
	    kfree(tmp);
	    printk("kerSysDeregisterDyingGaspHandler: %s is deregistered\n", devname);
	    return;
	}
    }	
    printk("kerSysDeregisterDyingGaspHandler: %s not (de)registered\n", devname);
	
} /* kerSysDeregisterDyingGaspHandler */

static int ConfigCs (BOARD_IOCTL_PARMS *parms)
{
    int                     retv = 0;
#if !defined(CONFIG_BCM96338)
    int                     cs, flags;
    cs_config_pars_t        info;

    if (copy_from_user(&info, (void*)parms->buf, sizeof(cs_config_pars_t)) == 0) 
    {
        cs = parms->offset;

        MPI->cs[cs].base = ((info.base & 0x1FFFE000) | (info.size >> 13));	

        if ( info.mode == EBI_TS_TA_MODE )     // syncronious mode
            flags = (EBI_TS_TA_MODE | EBI_ENABLE);
        else
        {
            flags = ( EBI_ENABLE | \
                (EBI_WAIT_STATES  & (info.wait_state << EBI_WTST_SHIFT )) | \
                (EBI_SETUP_STATES & (info.setup_time << EBI_SETUP_SHIFT)) | \
                (EBI_HOLD_STATES  & (info.hold_time  << EBI_HOLD_SHIFT )) );
        }
        MPI->cs[cs].config = flags;
        parms->result = BP_SUCCESS;
        retv = 0;
    }
    else
    {
        retv -= EFAULT;
        parms->result = BP_NOT_DEFINED; 
    }
#endif
    return( retv );
}

static void SetPll (int pll_mask, int pll_value)
{
    PERF->pll_control &= ~pll_mask;   // clear relevant bits
    PERF->pll_control |= pll_value;   // and set desired value
}

static void SetGpio(int gpio, GPIO_STATE_t state)
{
    unsigned long gpio_mask = GPIO_NUM_TO_MASK(gpio);
    volatile unsigned long *gpio_io_reg = &GPIO->GPIOio;
    volatile unsigned long *gpio_dir_reg = &GPIO->GPIODir;
    
#if !defined (CONFIG_BCM96338)
    if( gpio >= 32 )
    {
        gpio_mask = GPIO_NUM_TO_MASK_HIGH(gpio);
        gpio_io_reg = &GPIO->GPIOio_high;
        gpio_dir_reg = &GPIO->GPIODir_high;
    }
#endif 

    *gpio_dir_reg |= gpio_mask;

    if(state == GPIO_HIGH)
        *gpio_io_reg |= gpio_mask;
    else
        *gpio_io_reg &= ~gpio_mask;
}

/* start of y42304 added 20060814: 为了实现atoi()函数的功能 */
unsigned long board_xtoi(const char *dest)
{
    unsigned long  x = 0;
    unsigned int digit;

    if ((*dest == '0') && (*(dest+1) == 'x')) dest += 2;

    while (*dest)
    {
	    if ((*dest >= '0') && (*dest <= '9'))
        {
            digit = *dest - '0';
	    }
	    else if ((*dest >= 'A') && (*dest <= 'F')) 
        {
            digit = 10 + *dest - 'A';
	    }
    	else if ((*dest >= 'a') && (*dest <= 'f')) 
        {
            digit = 10 + *dest - 'a';
	    }
	    else 
        {
            break;
	    }
    	x *= 16;
	    x += digit;
	    dest++;
	}
    return x;
}


int board_atoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest+1) == 'x')) 
    {
	    return board_xtoi(dest+2);
	}

    while (*dest) 
    {
	    if ((*dest >= '0') && (*dest <= '9')) 
        {
	        digit = *dest - '0';
	    }
    	else 
        {
            break;
	    }
   	    x *= 10;
	    x += digit;
	    dest++;
	}
    return x;
}
/* end of y42304 added 20060814: 为了实现atoi()函数的功能 */

/**************************************************************************
 * 函数名  :   kerSysGetFsImageFromFlash
 * 功能    :   提供给应用从FLASH读取fs文件系统
 *
 * 输入参数:   string:  存储内容的buf
               strLen:  buf的字节长度
               offSet:  从文件系统开始位置的偏移量
 * 输出参数:   无
 *
 * 返回值  :   0:   正确
               -1:  错误
 * 作者    :   yuyouqing42304
 * 修改历史:   2006-08-15创建  
 ***************************************************************************/
int kerSysGetFsImageFromFlash(char *string, int strLen, int offSet)
{    
    unsigned long ulFsflashAddr = 0;
    PFILE_TAG stFileTag = getTagFromPartition(1);   
    if ((string == NULL) || (stFileTag == NULL))
    {
        return -1;
    }

    ulFsflashAddr = board_atoi(stFileTag->rootfsAddress) + BOOT_OFFSET;     
   
    kerSysReadFromFlash((void *)string, ulFsflashAddr+offSet, strLen);      

    return 0;
        
}

/**************************************************************************
 * 函数名  :   kerSysGetKernelImageFromFlash
 * 功能    :   提供给应用从FLASH读取kernel
 *
 * 输入参数:   string:  存储内容的buf
               strLen:  buf的字节长度
               offSet:  从kernel开始位置的偏移量
 * 输出参数:   无
 *
 * 返回值  :   0:   正确
               -1:  错误
 * 作者    :   yuyouqing42304
 * 修改历史:   2006-08-15创建  
 ***************************************************************************/
int kerSysGetKernelImageFromFlash(char *string, int strLen, int offSet)
{    
    unsigned long ulKernelflashAddr = 0;
    PFILE_TAG stFileTag = getTagFromPartition(1);
    
    if ((string == NULL) || (stFileTag == NULL))
    {
        return -1;
    }    
    ulKernelflashAddr = board_atoi(stFileTag->kernelAddress) + BOOT_OFFSET;

    kerSysReadFromFlash((void *)string, ulKernelflashAddr+offSet, strLen);    
    
    return 0;    
}

/**************************************************************************
 * 函数名  :   kerSysSetGPIO
 * 功能    :   读取给定GPIO的值
 *
 * 输入参数:   ucGpioNum:  GPIO号
 * 输出参数:   无
 *
 * 返回值  :    正确  :  GPIO值，高电平或低电平
 *
 * 作者    :    yuyouqing42304
 * 修改历史:    2006-05-16创建  
 ***************************************************************************/
void kerSysSetGPIO(unsigned short ucGpioNum,GPIO_STATE_t state)
{  
    SetGpio( ucGpioNum, state);
}

static PFILE_TAG getBootImageTagAndSeq(unsigned long* pulSeq)
{
    PFILE_TAG pTag = NULL;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);

    if( pTag1 && pTag2 )
    {
        /* Two images are flashed. */
        int sequence1 = simple_strtoul(pTag1->imageSequence, NULL, 10);
        int sequence2 = simple_strtoul(pTag2->imageSequence, NULL, 10);
        char *p;
        char bootPartition = BOOT_LATEST_IMAGE;
        NVRAM_DATA nvramData;

        memcpy((char *) &nvramData, (char *) get_nvram_start_addr(),
            sizeof(nvramData));
        for( p = nvramData.szBootline; p[2] != '\0'; p++ )
            if( p[0] == 'p' && p[1] == '=' )
            {
                bootPartition = p[2];
                break;
            }

        if( bootPartition == BOOT_LATEST_IMAGE )
        {
            pTag    = (sequence2 > sequence1) ? pTag2 : pTag1;
            *pulSeq = (sequence2 > sequence1) ? 2 : 1;
        }
        else /* Boot from the image configured. */
        {
            pTag    = (sequence2 < sequence1) ? pTag2 : pTag1;
            *pulSeq = (sequence2 < sequence1) ? 2 : 1;
        }
    }
    else
    {
        /* One image is flashed. */
        pTag = (pTag2) ? pTag2 : pTag1;
        *pulSeq = (pTag2) ? 2 : 1;
    }

    return( pTag );
}

#if 0
/*****************************************************************************
 函 数 名  : kerSysCheckFsCrc
 功能描述  : 校验文件系统crc
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年1月16日
    作    者   : liuyang 65130
    修改内容   : 新生成函数

*****************************************************************************/
PFILE_TAG  kerSysGetBootFs(void)
{
    u_int32_t rootfs_addr, kernel_addr, root_len;
    unsigned long seq = 1;
    PFILE_TAG pTag  = getBootImageTagAndSeq(&seq);
    char* pSectAddr = NULL;
    int blk = 0;
    UINT32 crc = CRC32_INIT_VALUE;

    if (pTag)
    {
        rootfs_addr = board_atoi(pTag->rootfsAddress) + BOOT_OFFSET;
        kernel_addr = board_atoi(pTag->kernelAddress) + BOOT_OFFSET;

        printk("^^^^^^^^TAG1 rootfs_addr: %x \r\n", rootfs_addr);
        
        root_len = board_atoi(pTag->rootfsLen);
        
        if (root_len > 0)
        {

            printk("^^^^^^^^root_len: %d \r\n", root_len);
            pSectAddr = retriedKmalloc(root_len);

            if (NULL == pSectAddr)
            {
                printk("~~~~~~~~~~Not enough memory.\r\n");
                return NULL;
            }
            memset(pSectAddr, 0, root_len);

            kerSysReadFromFlash(pSectAddr, rootfs_addr, root_len);
            crc = getCrc32(pSectAddr, root_len, crc);      
            if (crc != *(unsigned long *) (pTag->imageValidationToken+CRC_LEN))
            {

                printk("^^^^^^^^ tag%d crc wrong\r\n", seq);
                seq = ((seq == 1) ? 2 : 1);
                pTag = getTagFromPartition(&seq);
                printk("^^^^^^^^TAG2 rootfs_addr: %x \r\n", rootfs_addr);
            }

            if (pSectAddr)
            {
                kfree(pSectAddr);
            }
        }
    }
    else
    {
        seq = ((seq == 1) ? 2 : 1);
        pTag = getTagFromPartition(2);
    }
    
    printk("^^^^^^^^TAG rootfs_addr: %d \r\n", rootfs_addr);
    return pTag;
}
EXPORT_SYMBOL(kerSysGetBootFs);
#endif
/*****************************************************************************
 函 数 名  : StatUnUsedMacNum
 功能描述  : 统计当前可用的MAC地址数，包括预留和扩充
 输入参数  : void  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2008年9月11日
    作    者   : c00131380
    修改内容   : 新生成函数

*****************************************************************************/
int StatUnUsedMacNum(void)
{
    int totalNum;
    int ramNum;
    int i;

    //ramNum = 4;
    ramNum = 8;  //需求变更，固定分配8个MAC
    totalNum = 0;
    /*   
    if (NULL == g_pNvramInfo && NULL == g_pExtendMacInfo)
    {
        return totalNum;
    }
#if 0	
    //读取预留的MAC地址可用数
    for( i = 0, pMai = g_pNvramInfo->MacAddrs; i < g_pNvramInfo->ulNumMacAddrs;
            i++, pMai++ )
    {
        //第一个MAC预留分配给eth,所以需要从总数中减1
        //第二个MAC预留分配给wan,但无WAN时也可分配给PVC，所以没有从总数中减1
        if (0 == pMai->chInUse && 0 < i)
        {
             ramNum = ramNum + 1;   
        }
        printk("***the %d chInUse is %d,Uid is %u\n",i+1,pMai->chInUse,pMai->ulId);
    }
#endif
    totalNum = g_pExtendMacInfo->unUsedNum + ramNum;
    */
    totalNum = ramNum;        
    return totalNum;

}
/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(kerSysNvRamGet);
EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysLedRegisterHwHandler);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetSdramSize);
EXPORT_SYMBOL(BpGetPsiSize);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetPressAndHoldResetGpio);
EXPORT_SYMBOL(BpGetVoipResetGpio);
EXPORT_SYMBOL(BpGetVoipIntrGpio);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetWirelessLedGpio);
EXPORT_SYMBOL(BpGetUsbLedGpio);
EXPORT_SYMBOL(BpGetHpnaLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetPppLedGpio);
EXPORT_SYMBOL(BpGetPppFailLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetAdslDyingGaspExtIntr);
EXPORT_SYMBOL(BpGetVoipExtIntr);
EXPORT_SYMBOL(BpGetHpnaExtIntr);
EXPORT_SYMBOL(BpGetHpnaChipSelect);
EXPORT_SYMBOL(BpGetVoipChipSelect);
EXPORT_SYMBOL(BpGetWirelessSesBtnGpio);
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(BpGetWirelessFlags);
EXPORT_SYMBOL(BpUpdateWirelessSromMap);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysGetCycleCount);
EXPORT_SYMBOL(kerSysSetWdTimer);
EXPORT_SYMBOL(kerSysWakeupMonitorTask);
EXPORT_SYMBOL(GetBoardVersion);
EXPORT_SYMBOL(GetHarewareType);
EXPORT_SYMBOL(kerSysReadFromFlash);
/*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
EXPORT_SYMBOL(kerSysWlanParamGet);
/*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
EXPORT_SYMBOL(BpGetTelLine0LedGpio);
EXPORT_SYMBOL(BpGetTelLine1LedGpio);
EXPORT_SYMBOL(BpGetPotsResetGpio);
EXPORT_SYMBOL(BpGetFxoLoopDetcGpio);
EXPORT_SYMBOL(BpGetFxoRingDetcGpio);
EXPORT_SYMBOL(BpGetFxoRelayCtlGpio);
EXPORT_SYMBOL(kerSysSetGPIO);
EXPORT_SYMBOL(kerSysGetGPIO);
EXPORT_SYMBOL(BpGetSlicType);
EXPORT_SYMBOL(BpGetDAAType);
