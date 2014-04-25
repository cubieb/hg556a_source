/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED 
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM. 
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT 
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : board_api.h (adapted from flash_api.h by Yen Tran)
 *
 * Created on :  02/20/2002  seanl
 ***************************************************************************/

#if !defined(_BOARD_API_H_)
#define _BOARD_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_BOARD_H)

// for the action in BOARD_IOCTL_PARMS for flash operation
typedef enum 
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
    SET_CS_PARAM,    
    GET_FILE_TAG_FROM_FLASH,
    VARIABLE,  
    /* begin --- Add Persistent storage interface backup flash partition by w69233 */
    PSI_BACKUP,
    /* end ----- Add Persistent storage interface backup flash partition by w69233 */
    FIX,
    AVAIL,
    /*start of 增加防火墙日志保存 by c00131380 at 080911*/
    FIREWALL_LOG,
    /*end of 增加防火墙日志保存 by c00131380 at 080911*/
    /*start of 增加系统日志保存by c00131380 at 080926*/
    SYSTEM_LOG,
    /*end of 增加系统日志保存by c00131380 at 080926*/
    
    /*start of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
    WLAN_PARAM,    
    /*end of 增加WLAN ART校准参数区域 by l68693 at 20081115*/
    /*add by aizhanlong 00169427 for DTS2011102004063 wifi can't enable by wifi button before disable wifi using web*/
    ENABLE_WLAN,
    DISABLE_WLAN,
    /*end by aizhanlong 00169427 for DTS2011102004063 wifi can't enable by wifi button before disable wifi using web*/
} BOARD_IOCTL_ACTION;  
    
typedef struct boardIoctParms
{
    char *string;
    char *buf;
    int strLen;
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;

// LED defines 
/* HUAWEI VHG w69233 2008-05-20 Modify begin: support HG556-Global hardware */
typedef enum
{   
    kLedMessage,
    kLedAdsl,
    kLedHspa,
    kLedPower,
    kLedLan1,
    kLedLan2,
    kLedLan3,
    kLedLan4,
    kLedWireless,
    kLedUsb,
    kLedHpna,
    kLedWanData,
    kLedWanDataFail,
    kLedPPP,
    kLedVoip,
    kLedSes,
    kLedLine0,
    kLedLine1,    
    kLedPSTN,
    kLedInternet,
    kLedEnd,                // NOTE: Insert the new led name before this one.  Alway stay at the end.
} BOARD_LED_NAME;
/* HUAWEI VHG w69233 2008-05-20 Modify end: support HG556-Global hardware */


typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateBlinkOnce,                  /* blink once, ~100ms and ignore the same call during the 100ms period */
    kLedStateSlowFlickerOnce,            /* Slow Flicker once, VDF customized and ignore the same call during blink period */
    kLedStateFastFlickerOnce,            /* Fast Flicker once, VDF customized and ignore the same call during blink period */
    kLedStateVerySlowBlinkContiunes,     /* Jiajun 12/18/2007: slow blink contines at ~500ms interval */
    kLedStateSlowBlinkContinues,         /* slow blink continues at ~600ms interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at ~200ms interval */
    kLedStateVerySlowBlinkOnce,          /* blink once at ~500ms <2009.1.5 tanyin>*/
    kLedStateSlowBlinkOnce,               /* blink once at ~250ms <2009.1.5 tanyin>*/
    /* BEGIN: Added by c106292, 2009/3/17   PN:Led light method changes*/
    kLedStateSlowFlickerContinues,   	
    kLedStateFastFlickerContinues,         	
 	kLedStateUnknow
   /* END:   Added by c106292, 2009/3/17 */

} BOARD_LED_STATE;

typedef enum
{
  GPIO_LOW = 0,
  GPIO_HIGH,
} GPIO_STATE_t;


/*l65130 start 清理FLASH - VAR分区使用 2008-01-17*/
#define FLASH_VAR_START         0
/*升级失败次数*/
#define FAILURE_TAG_LEN         10

/* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 add begin:*/
/*Telnet关闭标志*/
#define TELNET_TAG_LEN         8
/* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 add end.*/
/* HUAWEI HGW s48571 2008年3月10日 默认关闭ping功能 add begin:*/
/*Ping关闭标志*/
#define PING_TAG_LEN         8
/* HUAWEI HGW s48571 2008年3月10日 默认关闭ping功能 add end.*/


/*单板序列号*/
#define SYS_PARAMETERS_OFFSET   32
#define BOARD_SERIAL_NUMBER_LEN 32
#define BOARD_MAGIC_LEN         8

#define SNMP_FLAG_OFFSET            (CFG_STATE_DATA_START + CWMP_STATE_DATA_LEN)
#define SNMP_FLAG_LEN				8

/*WLAN WEP KEY*/
#define WLAN_PARAMETERS_OFFSET      (CFG_STATE_DATA_START + CWMP_STATE_DATA_LEN)
#define WLAN_WEP_KEY_LEN            32
#define WLAN_WPA_PSK_LEN            64
#define RESTORE_FLAG_LEN            8
#define WLAN_SSID_LEN               64
#define WLAN_PARA_LEN               (WLAN_WEP_KEY_LEN + WLAN_WPA_PSK_LEN + RESTORE_FLAG_LEN + WLAN_SSID_LEN)

/*Start -- w00135358 modify for HG556a- store Hareware Version - 20090720*/
#define HARDWARE_VERSION_OFFSET      (WLAN_PARAMETERS_OFFSET + WLAN_PARA_LEN)
#define HARDWARE_VERSION_LEN 		 32
#define HARDWARE_VERSION_VALID_LEN 	 16
/*End -- w00135358 modify for HG556a- store Hareware Version - 20090720*/

/* BEGIN: Added by y67514, 2009/12/10   PN:APNSecurity Enhancement*/
#define HSPA_PARAMETERS_OFFSET          (HARDWARE_VERSION_OFFSET + HARDWARE_VERSION_LEN )
/*此3参数都按64位长度限制，因为存储
      的是base64的密文，所以最大长度是(64*4/3+4)*/
#define HSPA_APN_LEN    90
#define HSPA_USER_NAME_LEN  90
#define HSPA_PWD_LEN    90
#define HSPA_PARAMETERS_LEN         ( HSPA_APN_LEN + HSPA_USER_NAME_LEN + HSPA_PWD_LEN )
/* END:   Added by y67514, 2009/12/10 */

/* HUAWEI HGW w69233 2008-03-29 Add begin */
/* Fix AU8D00334: EquipmentTest command "SetSsid/SetWepKey/SetWpaKey" for wlan does not work */
#define WLAN_VALID_WEPKEY_LEN       13
#define WLAN_VALID_WPAKEY_LEN       64
#define WLAN_VALID_SSID_LEN         32
/* HUAWEI HGW w69233 2008-03-29 Add end */

/*配置数据*/
#define CFG_STATE_DATA_START    264//(256+8) 2007-3-19 ly65130与这个冲突,向后迁移8个字节
/*升级成功失败信息*/
#define UPG_INFO_STATUS         8
/*出厂标识*/
#define MAX_UPGRADE_FLAG        8
#define CWMP_STATE_DATA_LEN     2040//(2048-8) 2007-3-19 ly65130与这个冲突,总体减少8个字节
/*参数通知属性保存长度*/
#define CWMP_PARA_OFFSET            8*1024
#define CWMP_PARA_LEN               1024*4
#define CWMP_PARA_RESET             8
#define AVAIL_MAX_LEN                 16*1024

/*SSL证书文件*/
#define CFG_STATE_SSL_STAT      4096
#define CWMP_SSL_CERT_LEN       2048
#define CWMP_SSL_CERT_TAG_LEN   4


/*空闲无人使用*/
/* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 modify begin:
#define FLASH_VAR_1_START       (FLASH_VAR_START + FAILURE_TAG_LEN)
*/
#define FLASH_VAR_1_START       (FLASH_VAR_START + FAILURE_TAG_LEN + TELNET_TAG_LEN + PING_TAG_LEN)
/* HUAWEI HGW s48571 2008年2月20日 默认关闭telnet功能 modify end. */
#define FLASH_VAR_1_LEN         (SYS_PARAMETERS_OFFSET - FLASH_VAR_1_START) //22

#define FLASH_VAR_2_START       (SYS_PARAMETERS_OFFSET + BOARD_SERIAL_NUMBER_LEN + BOARD_MAGIC_LEN)
#define FLASH_VAR_2_LEN         (CFG_STATE_DATA_START - FLASH_VAR_2_START) //216

/* BEGIN: Modified by y67514, 2009/12/10   问题单号:APNSecurity Enhancement*/
#define FLASH_VAR_3_START       (HSPA_PARAMETERS_OFFSET + HSPA_PARAMETERS_LEN)
#define FLASH_VAR_3_LEN         (CFG_STATE_SSL_STAT - FLASH_VAR_3_START) //1792
/* END:   Modified by y67514, 2009/12/10 */

#define FLASH_VAR_4_START       (CFG_STATE_SSL_STAT + CWMP_SSL_CERT_LEN)
#define FLASH_VAR_4_LEN         (8192 - FLASH_VAR_4_START)     //2048

/* BEGIN: Added by chenzhigang, 2008/3/14   问题单号:CW使能关闭由终端来控制*/
#define FLASH_VAR_VOICE_SERVICE_MANAGE_START   128
#define FLASH_VAR_VOICE_SERVICE_MANAGE_LEN     8
/* END:   Added by chenzhigang, 2008/3/14 */

/*l65130 end 清理FLASH - VAR分区使用 2008-01-17*/

/* start of 增加Global MAC 地址扩容 c00131380 0800908 */
//用户增加自定义MAC保存地址
#define MACADDITION_VAR_1_START CFG_STATE_DATA_START + UPG_INFO_STATUS + UPG_INFO_STATUS //280~285
#define MACADDITION_VAR_2_START MACADDITION_VAR_1_START + MACADDITION_LEN                //286~291
#define MACADDITION_VAR_3_START MACADDITION_VAR_2_START + MACADDITION_LEN                //292~297
#define MACADDITION_VAR_4_START MACADDITION_VAR_3_START + MACADDITION_LEN                //298~303
#define MACADDITION_LEN 6
#define EXT_MAC_INIT_FLAG       MACADDITION_VAR_4_START + MACADDITION_LEN                //304~311
#define EXT_MAC_INIT_FLAG_LEN 8

#define MAC_ADDRESS_LEN           6
#define MAX_MAC_STR_LEN           18
#define NVRAM_SERIAL_NUMBER_LEN         32


/* end of 增加Global MAC 地址扩容 c00131380 0800908 */


#define EQUIP_RESET_KEY			2
#define EQUIP_RESTART_KEY		4
#define EQUIP_WLAN_KEY			8
#define EQUIP_LANDIAG_KEY		16
#define EQUIP_KEY_MASK			31

/* start of l68693 added 20081227: ReInection优化修改 */ 
typedef enum
{
   REINJECTION_CTL_RELAY_ON = 0,  //打开继电器
   REINJECTION_CTL_RELAY_OFF = 1, //关闭继电器
   REINJECTION_CTL_RESET = 2      //SLIC复位
} REINJECTIONCTL_CMD;
/* end of l68693 added 20081227: ReInection优化修改 */ 


/* start of MAC地址扩容 c00131380 at 080909*/
typedef struct MACAdditionInfo
{
    int  totalNum;
    int  unUsedNum;
    unsigned char addMACAddress1[MAC_ADDRESS_LEN];
    int  inUsed1;    //0:未用 1:已用
    unsigned char addMACAddress2[MAC_ADDRESS_LEN];
    int  inUsed2;    //0:未用 1:已用
    unsigned char addMACAddress3[MAC_ADDRESS_LEN];
    int  inUsed3;    //0:未用 1:已用
    unsigned char addMACAddress4[MAC_ADDRESS_LEN];
    int  inUsed4;    //0:未用 1:已用
} MACAdditionInfo,*PMACAdditionInfo;

typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulSdramSize;
    unsigned long ulPsiSize;
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[MAC_ADDRESS_LEN];
    char szSerialNumber[NVRAM_SERIAL_NUMBER_LEN];     /* y42304 added:支持序列号 */
    MAC_ADDR_INFO MacAddrs[1];
} NVRAM_INFO, *PNVRAM_INFO;

/* end of MAC地址扩容 c00131380 at 080909*/

/*start of 提供保存网关激活信息的flash存贮接口 by c00131380 at 080917*/
#define GATEWAY_ACTIVATION_INFO EXT_MAC_INIT_FLAG + EXT_MAC_INIT_FLAG_LEN       //312~313
#define GATEWAY_ACTIVATION_INFO_LEN 2                                            //2个字节
/*end of 提供保存网关激活信息的flash存贮接口 by c00131380 at 080917*/

/* WebUI language. <tanyin 2009.2.16> */
#define WEB_LANGUAGE_OFFSET (GATEWAY_ACTIVATION_INFO + GATEWAY_ACTIVATION_INFO_LEN)
#define WEB_LANGUAGE_LEN     8

/* WebUI language for admin. <tanyin 2009.5.3> */
#define WEB_ADMIN_LANGUAGE_OFFSET (WEB_LANGUAGE_OFFSET + WEB_LANGUAGE_LEN)
#define WEB_ADMIN_LANGUAGE_LEN     8

/* BEGIN: Modified by w00135358, 2010/02/21: TR069 FIRMWARE UPGRADE Fault Code-ACS连续升级上报9010问题*/
#define CWMP_LOAD_FAIL_INFO WEB_ADMIN_LANGUAGE_OFFSET+WEB_ADMIN_LANGUAGE_LEN      //330~457
#define CWMP_LOAD_FAIL_INFO_LEN 128                                            //128个字节
/* END: Modified by w00135358, 2010/02/21: TR069 FIRMWARE UPGRADE Fault Code-ACS连续升级上报9010问题*/

/*Begin:Added by luokunling 00192527 for imsi info requirement,2011/9/19*/
#define IMSI_OFFSET CWMP_LOAD_FAIL_INFO+CWMP_LOAD_FAIL_INFO_LEN   //458~473
#define IFC_IMSI_LEN 16
/*Begin:Added by luokunling 00192527,2011/9/19*/

/*****************************************************************************/
/*       Reserve MAC for VDF requirement                                     */
/*****************************************************************************/
#define RESERVE_MAC_MASK 0xFF000000
#define NO_RESERVE_MAC 0xFFFFFFFF

/* Defines. for board driver */
#define BOARD_IOCTL_MAGIC       'B'
#define BOARD_DRV_MAJOR          206

#define BOARD_IOCTL_FLASH_INIT \
    _IOWR(BOARD_IOCTL_MAGIC, 0, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_WRITE \
    _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_FLASH_READ \
    _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_NR_PAGES \
    _IOWR(BOARD_IOCTL_MAGIC, 3, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_DUMP_ADDR \
    _IOWR(BOARD_IOCTL_MAGIC, 4, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MEMORY \
    _IOWR(BOARD_IOCTL_MAGIC, 5, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_MIPS_SOFT_RESET \
    _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_LED_CTRL \
    _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 8, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 9, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_RELEASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 10, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_PSI_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_SDRAM_SIZE \
    _IOWR(BOARD_IOCTL_MAGIC, 12, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MONITOR_FD \
    _IOWR(BOARD_IOCTL_MAGIC, 13, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_WAKEUP_MONITOR_TASK \
    _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 15, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_BOOTLINE \
    _IOWR(BOARD_IOCTL_MAGIC, 16, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_BASE_MAC_ADDRESS \
    _IOWR(BOARD_IOCTL_MAGIC, 17, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CHIP_ID \
    _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)

/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */
#define BOARD_IOCTL_GET_NUM_ENET \
    _IOWR(BOARD_IOCTL_MAGIC, 23, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_CFE_VER \
    _IOWR(BOARD_IOCTL_MAGIC, 24, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_ENET_CFG \
    _IOWR(BOARD_IOCTL_MAGIC, 21, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_WLAN_ANT_INUSE \
    _IOWR(BOARD_IOCTL_MAGIC, 22, BOARD_IOCTL_PARMS)

/* 2008/01/28 Jiajun Weng : Modify its definition for WPS PBC trigger */
#define BOARD_IOCTL_SET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 19, BOARD_IOCTL_PARMS)        

#define BOARD_IOCTL_GET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 20, BOARD_IOCTL_PARMS)        

#define BOARD_IOCTL_UNSET_TRIGGER_EVENT \
    _IOWR(BOARD_IOCTL_MAGIC, 25, BOARD_IOCTL_PARMS) 

#define BOARD_IOCTL_SET_SES_LED \
    _IOWR(BOARD_IOCTL_MAGIC, 26, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_GET_VCOPE_GPIO \
    _IOWR(BOARD_IOCTL_MAGIC, 27, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_CS_PAR \
    _IOWR(BOARD_IOCTL_MAGIC, 28, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_PLL \
    _IOWR(BOARD_IOCTL_MAGIC, 29, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_SET_GPIO \
    _IOWR(BOARD_IOCTL_MAGIC, 30, BOARD_IOCTL_PARMS)
  
#define BOARD_IOCTL_GET_DYINGGASP \
    _IOWR(BOARD_IOCTL_MAGIC, 39, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_PUT_WEB_OP \
    _IOWR(BOARD_IOCTL_MAGIC, 31, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_PUT_CLI_OP \
    _IOWR(BOARD_IOCTL_MAGIC, 32, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_PUT_ITF_STATE \
    _IOWR(BOARD_IOCTL_MAGIC, 33, BOARD_IOCTL_PARMS)
    
#define BOARD_IOCTL_EQUIPMENT_TEST \
    _IOWR(BOARD_IOCTL_MAGIC, 34, BOARD_IOCTL_PARMS)  

#define BOARD_IOCTL_CHECK_WLAN_STATUS \
    _IOWR(BOARD_IOCTL_MAGIC, 35, BOARD_IOCTL_PARMS)  

#define BOARD_IOCTL_SET_VOIP_SERVICE \
    _IOWR(BOARD_IOCTL_MAGIC, 36, BOARD_IOCTL_PARMS)      

#define BOARD_IOCTL_GET_VOIP_SERVICE \
        _IOWR(BOARD_IOCTL_MAGIC, 37, BOARD_IOCTL_PARMS)          

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
#define BOARD_IOCTL_SET_TRAFFIC_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 38, BOARD_IOCTL_PARMS)  

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
#define BOARD_IOCTL_SET_EQUIPTEST_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 40, BOARD_IOCTL_PARMS)  
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/

/* start of 增加Global MAC 地址扩容 c00131380 0800910 */

#define BOARD_IOCTL_GET_MEMMACINUSESTATUS \
        _IOWR(BOARD_IOCTL_MAGIC, 41, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MEMMACINUSESTATUS \
        _IOWR(BOARD_IOCTL_MAGIC, 42, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_MEMMACADDRESS \
        _IOWR(BOARD_IOCTL_MAGIC, 43, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_SET_MEMMACADDRESS \
        _IOWR(BOARD_IOCTL_MAGIC, 44, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_NUMOFUNUSEDMACADDR \
        _IOWR(BOARD_IOCTL_MAGIC, 45, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_DEL_EXTENDMACADDR \
        _IOWR(BOARD_IOCTL_MAGIC, 46, BOARD_IOCTL_PARMS)

/* end of 增加Global MAC 地址扩容 c00131380 0800910 */
#define BOARD_IOCTL_SET_IGMPLOG_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 47, BOARD_IOCTL_PARMS)    

#define BOARD_IOCTL_SET_TELNETD_PID \
        _IOWR(BOARD_IOCTL_MAGIC, 48, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_KEY_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 49, BOARD_IOCTL_PARMS)

#define BOARD_IOCTL_GET_EQUIPTEST_MODE \
        _IOWR(BOARD_IOCTL_MAGIC, 50, BOARD_IOCTL_PARMS)
/* j00100803 save fwlog to flash for HG556A */
#define BOARD_IOCTL_GET_VAR_REBOOT_BUTTON \
        _IOWR(BOARD_IOCTL_MAGIC, 51, BOARD_IOCTL_PARMS)
/*start of 增加 WPS 状态检测 by s53329 at  20090722*/
#define BOARD_IOCTL_CHECK_WLAN_WPS_STATUS \
    _IOWR(BOARD_IOCTL_MAGIC, 52, BOARD_IOCTL_PARMS)
/*end of 增加 WPS 状态检测 by s53329 at  20090722*/        
/* j00100803 save fwlog to flash for HG556A */
/* start of BOARD update by y42304 20060623: 与装备测试有关的动作 */
typedef enum 
{
    SET_BASE_MAC_ADDRESS,
    GET_BASE_MAC_ADDRESS,
    SET_MAC_AMOUNT,
    GET_MAC_AMOUNT,
    GET_ALL_BOARD_ID_NAME,
    GET_PCB_VERSION,
    GET_BOARD_VERSION,
    SET_BOARD_ID,
    GET_BOARD_ID,
    SET_SERIAL_NUMBER,
    GET_SERIAL_NUMBER,    
    GET_CPU_REVISION_ID,
    GET_BOARD_MANUFACTURER_NAME,
    BUTT,    
} BOARD_EQUIPMENT_TEST_ACTION;
/* end of equip test added by y42304 20060623*/


typedef enum 
{
    /*
     *  GPIO5 GPIO4
     *    0    0     --> HG55VDFA
     *    0    0     --> HG56BZRB        
     单板名。
     HG55VDFA实现：BCM6358 + BCM5325 + ATHEROS9223 + Le88266 + BCM6301 + Si3050(+Si3011)   
     HG56BZRB实现：BCM6358 + BCM5325 + RALINK3062 + Le88266 + BCM6301  + LineDectector   
     */
    HG55VDFA  = 0,   
    HG56BZRB  = 3,
}E_BOARD_ADSL_VERSION;



typedef enum 
{
    MODE_NONE = 0,
    MODE_2G,
    MODE_3G,
    MODE_PASSBY,
    MODE_NORM,
    MODE_UNDEFINED,                
}E_HSPA_TRAFFICMODE_VERSION;


/* HUAWEI HGW s48571 2008年1月18日 Hardware Porting add end.*/


#endif  

#if defined(SUPPORT_SLIC_3210) || defined(SUPPORT_SLIC_3215)|| defined(SUPPORT_SLIC_LE88221)
typedef enum
{
    EPCMD_SLIC_HELP = 0,
    EPCMD_SLIC_WRITE_DIRECT_REG,
    EPCMD_SLIC_READ_DIRECT_REG,
    EPCMD_SLIC_BATCH_READ_DIRECT_REG,    
    EPCMD_SLIC_WRITE_INDIRECT_REG,
    EPCMD_SLIC_READ_INDIRECT_REG,
    EPCMD_SLIC_BATCH_READ_INDIRECT_REG,    
    EPCMD_SLIC_MAX
    
}EPCMD_SLIC;

#ifndef EPSTATUS_
#define EPSTATUS_
typedef enum
{
   EPSTATUS_SUCCESS = 0,         /* success */
   EPSTATUS_ENDPT_UNKNOWN,       /* endpoint is unknown */
   EPSTATUS_NOTREADY,            /* endpoint is not ready */
   EPSTATUS_RESOURCES,           /* endpoint has insufficient resources */
   EPSTATUS_SIGNAL_UNKNOWN,      /* endpoint cant generate that signal */
   EPSTATUS_CNX_UNKNOWN,         /* incorrect connection ID */
   EPSTATUS_UNSUPPORTED_MODE,    /* unsupported connection mode */
   EPSTATUS_UNKNOWN_TYPE,        /* unknown endpoint type */
   EPSTATUS_BUSY,                /* endpoint is busy */
   EPSTATUS_ONHOOK,              /* endpoint is onhook, can't perform operation */
   EPSTATUS_BAD_MODE,            /* endpoint is in wrong mode, can't perform operation */
   EPSTATUS_SHUTDOWN_ERR         /* unable to shutdown the endpoint */
} EPSTATUS;
#endif

#if defined(SUPPORT_SLIC_LE88221)
typedef struct SLICCMDPARM
{
    unsigned long    size;       /* Size of the structure (including the size field) */
    int              cmd;
    int              lineId;
    int              arg[3];   
   unsigned char     regValue[16];   
   unsigned short   retVal;   
   EPSTATUS         epStatus;
} SLIC_CMD_PARM, *PSLIC_CMD_PARM;

#else

typedef struct SLICCMDPARM
{
   unsigned long    size;       /* Size of the structure (including the size field) */
   int              cmd;
   int              lineId;
   int              arg[3];   
   unsigned short   retVal;   
   EPSTATUS         epStatus;
} SLIC_CMD_PARM, *PSLIC_CMD_PARM;
#endif

typedef struct CAS_CTL_FXO_PARMS
{
   unsigned long  minHookFlash;           /* minimum hook flash interval         */
   unsigned long  maxHookFlash;           /* maximum hook flash interval         */
   unsigned long  pulseMinMake;           /* pulse dialing min make interval     */
   unsigned long  pulseMaxMake;           /* pulse dialing max make interval     */
   unsigned long  pulseMinBreak;          /* pulse dialing min break interval    */
   unsigned long  pulseMaxBreak;          /* pulse dialing max break interval    */
   unsigned long  minConnect;             /* minimum connection interval         */
   unsigned long  minDisconnect;          /* minimum disconnect interval         */
   unsigned long  minWink;                /* minimum hook wink interval          */
   unsigned long  minPostWink;            /* minimum hook post-wink interval     */
   unsigned long  earlyActiveHookInterval;/* early hook state change interval for active state */
   unsigned long  delayActiveHookInterval;/* delay in active state after offhook detected */

} CAS_CTL_FXO_PARMS;

/* start of l68693 added 20081227: ReInection优化修改 */ 
typedef struct REINJECTIONPARAM
{
   unsigned long  size;       /* Size of the structure (including the size field) */
   int            lineId;
   int            cmd;
   EPSTATUS       epStatus;   
   
} REINJECTION_CMD_PARAM, *PREINJECTION_CMD_PARAM;
/* end of l68693 added 20081227: ReInection优化修改 */ 

/* start of l68693 added 20081227: ReInection优化修改 */ 
/* 以下IOCTL一定要和endpointdrv.h中一致 */
/* start of l68693 added 20081227: ReInection优化修改 */ 
#define ENDPOINTIOCTL_CONFIG_DAA_PARAM \
    _IOWR(209, 26, REINJECTION_CMD_PARAM)

int sysDiagDaaParam( void );
/* end of l68693 added 20081227: ReInection优化修改 */ 

#define ENDPOINTIOCTL_CONTROL_REINJECTION \
    _IOWR(209, 27, REINJECTION_CMD_PARAM)
    
int sysDiagControlReInjection ( int cmd );
/* end of l68693 added 20081227: ReInection优化修改 */ 

/* 17->18  l68693  3.12voice add 1 mute ioctl*/
#define ENDPOINTIOCTL_CONTROL_SLIC \
    _IOWR(209, 18, SLIC_CMD_PARM)

#if defined(SUPPORT_SLIC_LE88221)
int sysDiagSlic(int command, int lineId, unsigned char ucBeginRegAddr, unsigned short usOption, unsigned char *pucRegValue);
#else
int sysDiagSlic(int command, int lineId, unsigned char ucBeginRegAddr, unsigned short usOption);
#endif

#endif

#define BP_NOT_DEFINED  0xffff  
#define NOT_SUPPROT_GET_VERSION  0xFF

int sysScratchPadSet(char *tokenId, char *tokBuf, int bufLen);
int sysScratchPadGet(char *tokenId, char *tokBuf, int bufLen);
int sysScratchPadClearAll(void);
int sysVariableGet(char *string, int strLen, int offset);
int sysVariableSet(char *string, int strLen, int offset);
int sysFixSet(char *string, int strLen, int offset);
int sysFixGet(char *string, int strLen, int offset);
int sysAvailGet(char *string, int strLen, int offset);
int sysAvailSet(char *string, int strLen, int offset);
int sysPersistentGet(char *string,int strLen,int offset);
int sysPersistentSet(char *string,int strLen,int offset);
/* begin --- Add Persistent storage interface backup flash partition by w69233 */
int sysPsiBackupGet(char *string,int strLen,int offset);
int sysPsiBackupSet(char *string,int strLen,int offset);
/* end ----- Add Persistent storage interface backup flash partition by w69233 */
int sysNvRamSet(char *string,int strLen,int offset);
int sysNvRamGet(char *string,int strLen,int offset);
void sysFlashImageInit(void);
int sysFlashImageGet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType);
int sysFlashImageSet(void *image, int size, int addr,
    BOARD_IOCTL_ACTION imageType);
int sysNrPagesGet(void);
int sysDumpAddr(char *addr, int len);
int sysSetMemory(char *addr, int size, unsigned long value );
void sysMipsSoftReset(void);
int sysGetBoardIdName(char *name, int length);
int sysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId );
int sysReleaseMacAddress( unsigned char *pucaAddr );
int sysGetSdramSize( void );
int sysGetPsiSize( void );
int sysGetBootline(char *string,int strLen);
int sysSetBootline(char *string,int strLen);
void sysLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
int sysFlashSizeGet(void);
int sysGetBaseMacAddress(unsigned char *pucaAddr);
int sysGetChipId(void);
int sysGetNumEnet(void);
int sysGetCFEVersion(char *string, int strLen);
int sysGetEnetCfg(char *string, int strLen);
int sysSetMonitorFd(int fd);
void sysWakeupMonitorTask(void);
int sysGetVcopeInfo(int info_type);
int sysConfigCs(int cs_number, void *info);
int sysSetPllClockRegister(int mask, int clock_value);
int sysSetGpioBit(int gpio_bit, GPIO_STATE_t flag);
int sysGetPCBVersion(void);
int sysGetBoardVersion(unsigned char *string);
int sysGetBoardManufacturer(unsigned char *string);
int sysFlashLog(void);

/* start of EquipTest added by y42304 20060619: telnet修改网口地址 */
int sysSetBaseMacAddress(unsigned char *string, int strLen); //y42304 added: 提供装备测试修改基mac 
int sysGetMacNumbers(void);
int sysSetMacNumbers(int macNumbers);
int sysSetBoardId(unsigned char *string, int strLen);
int sysGetBoardId(unsigned char *pucBoardId);
int sysGetAllBoardIdName(char *string, int strLen);
int sysGetBoardSerialNumber(char *string, int strLen);
int sysSetBoardSerialNumber(char *string, int strLen);
int sysGetChipRevisionID(void);
/* end of added by y42304 20060619 */

/* start of y42304 added 20060907: 支持通过按复位键10秒内开启/关闭wlan */
#ifdef WIRELESS   
int sysCheckWirelessStatus(void);
/*add by aizhanlong 00169427 for DTS2011102004063 wifi can't enable by wifi button before disable wifi using web*/
int sysSetWirelessStatus(int status);
/*end by aizhanlong 00169427 for DTS2011102004063 wifi can't enable by wifi button before disable wifi using web*/
#endif
/* end of y42304 added 20060907 */

int sysGetVoipServiceStatus(void);
int sysSetVoipServiceStatus(unsigned char ucStatus);

/* start of y42304 added 20060814: 提供获取flash FILE_TAG的接口给应用*/
int sysGetFileTagFromFlash(char *pstTag);
int sysGetFsImageFromFlash(char *string, int strLen, int offSet);
int sysGetKernelImageFromFlash(char *string, int strLen, int offSet);
int sysGetPsiActualSizeFromFlash(void);
int sysGetFsAndKernleImageFromFlash(char *string, int strLen, int offSet);
/* end of y42304 added 20060814 */

/* start of maintain dying gasp by liuzhijie 00028714 2006年5月9日" */
int sysGetDyingGaspInfo(char * bufs);
int sysPutWebOperate(char *string, int strLen);
int sysPutCliOperate(char *string, int strLen);
int sysPutItfState(char *itf_name, int state);
/* end of maintain dying gasp by liuzhijie 00028714 2006年5月9日" */

/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add begin:*/
int sysSetHspaTrafficMode(int hspaMode);
/* HUAWEI HGW s48571 2008年1月19日 Hardware Porting add end.*/

/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add begin:*/
int sysSetEquipTestMode(int equipTestMode);
/* HUAWEI HGW s48571 2008年2月20日 装备测试状态标志添加 add end.*/

/* start of 增加MAC地址扩充功能 by c00131380 at 080910*/
int sysAvailMacAddrGet(unsigned char *string, int strLen, int offset);

int sysAvailMacAddrSet(unsigned char *string, int strLen, int offset);

int sysGetMemMacInUsedStatus(int macID);

int sysSetMemMacInUsedStatus(int macID,int inUseStatus);

int sysGetMemMacAddrInfo(unsigned char *string,int macID);

int sysSetMemMacAddrInfo(unsigned char *string,int macID);

int sysGetNumofUnUsedMac(void);

int sysDeleteMemMacAddr(int macID);

/* end of 增加MAC地址扩充功能 by c00131380 at 080910*/

int sysSetCliPid(int iPid);
int sysGetKeyMode(void);
int sysGetEquipTestMode(void);

/*****************************************************************************
 函 数 名  : sysFirewallLogGet
 功能描述  : 获得防火墙日志从FLASH
 日    期   : 2008年9月11日by c00131380 
*****************************************************************************/
int sysFirewallLogGet(char *string, int strLen, int offset);

/*****************************************************************************
 函 数 名  : sysFirewallLogSet
 功能描述  : 保存防火墙日志到FLASH
 日    期   : 2008年9月11日by c00131380
*****************************************************************************/
int sysFirewallLogSet(char *string, int strLen, int offset);

/*****************************************************************************
 功能描述  : 获取系统日志从FLASH
 日    期   : 2008年9月26日by c00131380
*****************************************************************************/
int sysSystemLogGet(char *string, int strLen, int offset);

/*****************************************************************************
 功能描述  : 保存系统日志到FLASH
 日    期   : 2008年9月26日by c00131380
*****************************************************************************/
int sysSystemLogSet(char *string, int strLen, int offset);
/* start of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
int sysSetIGMPLogMode(int mode);
/* end of 增加igmp proxy syslog开关功能 by l129990 2008,9,28*/
/* j00100803 save fwlog to flash for HG556A */
int sysGetRebootBtnFlag(void);
/* j00100803 save fwlog to flash for HG556A */
#if defined(__cplusplus)
}
#endif

#endif /* _BOARD_API_H_ */

