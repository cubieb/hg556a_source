/*
<:copyright-gpl
 Copyright 2004 Broadcom Corp. All Rights Reserved.

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

/*****************************************************************************
//
//
******************************************************************************
//
//  Filename:       wldefs.h
//  Author:         Peter T. Tran
//  Creation Date:  10/17/02
//
******************************************************************************
//  Description:
//      Define the WlDefs functions.
//
*****************************************************************************/

#ifndef __WL_DEFS_H__
#define __WL_DEFS_H__

/********************** Include Files ***************************************/

#include <stdio.h>
#include <netinet/in.h>

/********************** Global Constants ************************************/

#define WL_NUM_SSID      (BcmWl_GetMaxMbss())

#define WL_NUM_ADAPTER	 (BcmWl_getNumAdaptor())
#define INTERFACE_WL0_INDEX       0 
#define INTERFACE_WL1_INDEX       1

#define WLAN_PREFIX	"wl"
#define IS_WLAN_IFC(x)	(!strncmp(x, WLAN_PREFIX, strlen(WLAN_PREFIX)))

#define WL_MAX_NUM_SSID  4
#define WL_SIZE_2_MAX    2  
#define WL_SIZE_4_MAX    4  
#define WL_SIZE_8_MAX    8  
#define WL_SIZE_132_MAX  132
#define WL_SIZE_256_MAX  256
#define WL_SIZE_512_MAX  512
#define WL_SM_SIZE_MAX   16
#define WL_MID_SIZE_MAX  32
#define WL_SSID_SIZE_MAX 96
#define WL_MIN_PSK_LEN	8
#define WL_MAX_PSK_LEN	64
#define WL_WPA_PSK_SIZE_MAX  72  // max 64 hex or 63 char
#define WL_RADIUS_KEY_SIZE_MAX  88 // max is 80, limited by nas_wksp.h.  IAS allow up to 128 char
#define WL_LG_SIZE_MAX   1024
#define WL_KEY_NUM       4
#define WL_WDS_NUM       4
#define WL_MAXVALIDSSID_LEN 32
/* BEGIN: Added by s00125931, 2008/11/10   问题单号: AU8D01107 */
#define WL_SHELLSSID_LEN 200
/* END:   Added by s00125931, 2008/11/10 */
#define WL_MIN_FRAGMENTATION_THRESHOLD  256
#define WL_MAX_FRAGMENTATION_THRESHOLD  2346
#define WL_MIN_RTS_THRESHOLD            0
/* BEGIN: Modified by c106292, 2008/10/4   PN:Global RTS Atheros 最大值为2346*/
#ifdef SUPPORT_ATHEROSWLAN
#define WL_MAX_RTS_THRESHOLD            2346
#else
#define WL_MAX_RTS_THRESHOLD            2347
#endif
/* END:   Modified by c106292, 2008/10/4 */
#define WL_MIN_DTIM                     1
#define WL_MAX_DTIM                     255
#define WL_MIN_BEACON_INTERVAL          1
#define WL_MAX_BEACON_INTERVAL          65535
#define WL_KEY64_SIZE_HEX               10
#define WL_KEY64_SIZE_CHAR              5
#define WL_KEY128_SIZE_HEX              26
#define WL_KEY128_SIZE_CHAR             13
#define WL_MAX_ASSOC_STA                128

#define WL_MAX_STA                      32
#define WL_FLT_MAC_OFF   "disabled"
#define WL_FLT_MAC_ALLOW "allow"
#define WL_FLT_MAC_DENY  "deny"

#define AUTO_MODE -1
#define ON         1
#define OFF        0

#define WL_BRIDGE_RESTRICT_ENABLE      0
#define WL_BRIDGE_RESTRICT_DISABLE     1
#define WL_BRIDGE_RESTRICT_ENABLE_SCAN 2

/* authentication mode */
#define WL_AUTH_OPEN     		"open"
#define WL_AUTH_SHARED   		"shared"
#define WL_AUTH_RADIUS   		"radius"
#define WL_AUTH_WPA      		"wpa"
#define WL_AUTH_WPA_PSK  		"psk"
#define WL_AUTH_WPA2     		"wpa2"
#define WL_AUTH_WPA2_PSK 		"psk2"
#define WL_AUTH_WPA2_MIX 		"wpa2mix"
#define WL_AUTH_WPA2_PSK_MIX 	"psk2mix"
#define WL_WPA_AUTH_DISABLED		0x0000	/* Legacy (i.e., non-WPA) */
#define WL_WPA_AUTH_NONE			0x0001	/* none (IBSS) */
#define WL_WPA_AUTH_UNSPECIFIED		0x0002	/* over 802.1x */
#define WL_WPA_AUTH_PSK				0x0004	/* Pre-shared key */
#define WL_WPA_AUTH_8021X 			0x0020	/* 802.1x, reserved */
#define WL_WPA2_AUTH_UNSPECIFIED	0x0040	/* over 802.1x */
#define WL_WPA2_AUTH_PSK			0x0080	/* Pre-shared key */
#define WL_WSC 0x10

/* wireless security bitvec */
#define WEP_ENABLED      1
#define TKIP_ENABLED     2
#define AES_ENABLED      4
#define WSEC_SWFLAG      8

#define TKIP_ONLY 	"tkip"
#define AES_ONLY  	"aes"
#define TKIP_AND_AES 	"tkip+aes"


#define	WPA_CAP_WPA2_PREAUTH		1

#define WL_BIT_KEY_128   0
#define WL_BIT_KEY_64    1
#define WL_KEY_IDX_MIN   1
#define WL_KEY_IDX_MAX   4

#define WL_PHY_TYPE_A    "a"
#define WL_PHY_TYPE_B    "b"
#define WL_PHY_TYPE_G    "g"
#define WL_PHY_TYPE_N    "n"
#define WL_PHY_TYPE_LP   "l"

#define WL_BASIC_RATE_DEFAULT    "default"
#define WL_BASIC_RATE_ALL        "all"
#define WL_BASIC_RATE_1_2        "12"
#define WL_BASIC_RATE_WIFI_2     "wifi2"

#define WL_MODE_G_AUTO           1
#define WL_MODE_G_PERFORMANCE    4
#define WL_MODE_G_LRS            5
#define WL_MODE_B_ONLY           0
/* BEGIN: Modified by c106292, 2008/9/18   PN:Global增加两种模式*/
#define WL_MODE_NG         6
#define WL_MODE_NA           7
/* END:   Modified by c106292, 2008/9/18 */

#define WL_AUTO                  "auto"
#define WL_OFF                   "off"
#define WL_ON                    "on"
#define WL_DISABLED              "disabled"
#define WL_ENABLED               "enabled"
#define WL_WSC_ENABLED           "enabled"
#define WL_WME_ON		 		1 
#define WL_WME_OFF		 		0

#define WL_PREAUTH_ON			1
#define WL_PREAUTH_OFF			0

#define BAND_A                   1
#define BAND_B                   2

#define WL_CHANSPEC_2G		2
#define WL_CHANSPEC_5G		5

#define WIRELESS_APP_FMT       "Wireless%dCfg"

#define WIRELESS_APPID           "WirelessCfg"
#define WIRELESS1_APPID          "Wireless1Cfg"
#define WIRELESS2_APPID          "Wireless2Cfg"
#define WIRELESS3_APPID          "Wireless3Cfg"
#define WL_VARS_ID              1
#define WL_MSSID_VARS_NUM_ID    2
#define WL_MSSID_VARS_TBL_ID    3
#define WL_FLT_MAC_NUM_ID       4
#define WL_FLT_MAC_TBL_ID       5
#define WL_WDS_MAC_NUM_ID       6
#define WL_WDS_MAC_TBL_ID       7
#define WL_SCAN_WDS_MAC_NUM_ID  8
#define WL_SCAN_WDS_MAC_TBL_ID  9

#define RESET_WLAN	 "/var/reset.wlan"
#define RELOAD_VARS_WLAN "/var/reloadvars.wlan"

#define MAIN_ADPT_IDX   0

#define MAIN_BSS_IDX	0
#define GUEST_BSS_IDX	1
#define GUEST1_BSS_IDX	2
#define GUEST2_BSS_IDX	3


#define WL0_MAIN_BSS_IDX	0
#define WL0_GUEST_BSS_IDX   1
#define WL0_GUEST1_BSS_IDX	2
#define WL0_GUEST2_BSS_IDX	3

#define WL1_MAIN_BSS_IDX    4
#define WL1_GUEST_BSS_IDX   5
#define WL1_GUEST1_BSS_IDX	6
#define WL1_GUEST2_BSS_IDX	7

#define WLAN_MACADDR_IDX      2
#define WLAN_MACADDR_GUEST1   3
#define WLAN_MACADDR_GUEST2   4
#define WLAN_MACADDR_GUEST3   5


/* 为减少修改, 兼容方案将RaLink接口统一为和Atheros一致的形式 ath0-ath3 */
#if 0
#define Fir_OF_INTERFACE   "wl0"
#define Other_OF_INTERFACE   "wl0."
#else
#define Fir_OF_INTERFACE   "ath0"
#define Other_OF_INTERFACE   "ath"
#endif

#define MAP_FROM_NVRAM	0
#define MAP_TO_NVRAM	1
#define WL_SES_ENTRY	99
#define MAX_NVPARSE 100
#define SES_WDS_MODE_DISABLED           0 /* disabled */
#define SES_WDS_MODE_AUTO               1 /* dynamic cf/cl selection */
#define SES_WDS_MODE_ENABLED_ALWAYS     2 /* enabled always */
#define SES_WDS_MODE_ENABLED_EXCL       3 /* WDS-only i.e no regular STAs */
#define SES_WDS_MODE_CLIENT             4 /* always wds client mode */
#define REG_MODE_OFF                    0 /* disabled 11h/d mode */
#define REG_MODE_H                      1 /* use 11h mode */
#define REG_MODE_D                      2 /* use 11d mode */

#define WL_HT20MODE                     0
#define WL_HTATUOMODE                   1
#define WL_HT40MODE                     2
#define WL_EXTMODE_LOWER                      -1
#define WL_EXTMODE_NONE                        0
#define WL_EXTMODE_UPPER                       1

#define WL_OPMODE_AP                    "ap" 
#define WL_OPMODE_WDS                   "wds" 
#define WL_PREAMBLE_LONG                "long"
#define WL_PREAMBLE_SHORT               "short"

#define WL_EXCHANNEL_BELOW_NUM          5

// TR69 overrides
#define WL_TR69_BEACONTYPE_NONE                             0
#define WL_TR69_BEACONTYPE_BASIC                            1
#define WL_TR69_BEACONTYPE_WPA                              2
#define WL_TR69_BEACONTYPE_11I                              3
#define WL_TR69_BEACONTYPE_BASIC_AND_WPA                    4
#define WL_TR69_BEACONTYPE_BASIC_AND_11I                    5
#define WL_TR69_BEACONTYPE_WPA_AND_11I                      6
#define WL_TR69_BEACONTYPE_BASIC_AND_WPA_AND_11I            7

#define WL_TR69_BASIC_ENCRYPTIONMODE_NONE                   0
#define WL_TR69_BASIC_ENCRYPTIONMODE_WEP                    1

#define WL_TR69_BASIC_AUTHENTICATION_NONE                   0
#define WL_TR69_BASIC_AUTHENTICATION_EAP                    1

#define WL_TR69_WPA_ENCRYPTION_MODES_WEP                    0
#define WL_TR69_WPA_ENCRYPTION_MODES_TKIP                   1
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_TKIP           2
#define WL_TR69_WPA_ENCRYPTION_MODES_AES                    3
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_AES            4
#define WL_TR69_WPA_ENCRYPTION_MODES_TKIP_AND_AES           5
#define WL_TR69_WPA_ENCRYPTION_MODES_WEP_AND_TKIP_AES       6

#define WL_TR69_WPA_AUTHENTICATION_MODE_PSK                 0
#define WL_TR69_WPA_AUTHENTICATION_MODE_EAP                 1

#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP                0
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_TKIP               1
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_TKIP       2
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_AES                3
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_AES        4
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_TKIP_AND_AES       5
#define WL_TR69_IEEE11I_ENCRYPTION_MODES_WEP_AND_TKIP_AES   6

#define WL_TR69_IEEE11I_AUTHENTICATION_MODE_PSK             0
#define WL_TR69_IEEE11I_AUTHENTICATION_MODE_EAP             1

/* mimo */
#define WL_CTL_SB_LOWER         -1
#define WL_CTL_SB_NONE           0
#define WL_CTL_SB_UPPER          1
/* defines used by the nrate iovar */
#define NRATE_STF_MASK	0x0000ff00	/* stf mode mask: siso, cdd, stbc, sdm */
#define NRATE_STF_SHIFT	8		/* stf mode shift */
#define NRATE_MCS_INUSE	0x00000080	/* MSC in use,indicates b0-6 holds an mcs */
#define NRATE_RATE_MASK 0x0000007f	/* rate/mcs value */
/* notify the average dma xfer rate (in kbps) to the driver */
#define AVG_DMA_XFER_RATE 100000

/* A chanspec holds the channel number, band, bandwidth and control sideband */
typedef int chanspec_t;
#define WL_CHANSPEC_CHAN_MASK		0x00ff

#define WL_CHANSPEC_CTL_SB_MASK		0x0300
#define WL_CHANSPEC_CTL_SB_SHIFT	     8
#define WL_CHANSPEC_CTL_SB_LOWER	0x0100
#define WL_CHANSPEC_CTL_SB_UPPER	0x0200
#define WL_CHANSPEC_CTL_SB_NONE		0x0300

#define WL_CHANSPEC_BW_MASK		0x0C00
#define WL_CHANSPEC_BW_SHIFT    10
#define WL_CHANSPEC_BW_10		0x0400
#define WL_CHANSPEC_BW_20		0x0800
#define WL_CHANSPEC_BW_40		0x0C00

#define WL_CHANSPEC_BAND_MASK		0xf000
#define WL_CHANSPEC_BAND_SHIFT		12
#define WL_CHANSPEC_BAND_5G		0x1000
#define WL_CHANSPEC_BAND_2G		0x2000
#define INVCHANSPEC	255

#define WL_N_BW_20ALL			0
#define WL_N_BW_40ALL			1
#define WL_N_BW_20IN2G_40IN5G		2

/* values for g_protection_override and n_protection_override */
#define WLC_PROTECTION_AUTO		-1
#define WLC_PROTECTION_OFF		0
#define WLC_PROTECTION_ON		1
#define WLC_PROTECTION_MMHDR_ONLY	2
#define WLC_PROTECTION_CTS_ONLY		3

/* values for g_protection_control and n_protection_control */
#define WLC_PROTECTION_CTL_OFF		0
#define WLC_PROTECTION_CTL_LOCAL	1
#define WLC_PROTECTION_CTL_OVERLAP	2

/* values for n_protection */
#define WLC_N_PROTECTION_OFF		0
#define WLC_N_PROTECTION_OPTIONAL	1
#define WLC_N_PROTECTION_20IN40		2
#define WLC_N_PROTECTION_MIXEDMODE	3

/*start of HG556_WLAN 2008.08.15 for 不插MiniPCI-WIFI模块时无法恢复默认配置问题 by lihua 68693 */
//#define WL_LEGACY_MSSID_NUMBER		2
#define WL_LEGACY_MSSID_NUMBER		4
/*end of HG556_WLAN 2008.08.15 for 不插MiniPCI-WIFI模块时无法恢复默认配置问题 by lihua 68693 */

#define MAX_HOSTAPD_CHECK_TIMES     30


#define WL_IFC_ENABLED			"wlEnbl_wl0v0"
#define WL_DEFAULT_IFC			"ath0"

#define WL_LANG_EN           1
#define WL_LANG_SPAIN        2

#ifdef  SUPPORT_WLAN_PRNTCTL
#define WLAN_PARAM_LEN       12
#define IN_WLAN_OFF_TIME     1
#define NOTIN_WLAN_OFF_TIME  0
#define NORMAL_YEAR_BASE     108
#define WLAN_DBG_LEVEL       1
#endif
 
//#define DEBUG_WLAN

#ifdef DEBUG_WLAN
#define WLANDBG_PRINT(format, args...)  printf(format, ## args)

#define WLAN_DBG(Level, Format...) \
{ \
    fprintf(stderr, "\n[<Level:%d>%s, %s, %d] ", Level, __FILE__, __FUNCTION__, __LINE__); \
    fprintf(stderr, Format); \
}
#else
#define WLANDBG_PRINT(format, args...) 

#define WLAN_DBG(Level, Format...)
#endif


/********************** Global Structs ****************************************/

typedef enum {
   WL_STS_OK = 0,
   WL_STS_ERR_GENERAL,
   WL_STS_ERR_MEMORY,
   WL_STS_ERR_OBJECT_NOT_FOUND
} WL_STATUS;

typedef enum {
   WL_SETUP_ALL = 0,
   WL_SETUP_BASIC,
   WL_SETUP_SECURITY,
   WL_SETUP_MAC_FILTER,
   WL_SETUP_WDS,
   WL_SETUP_ADVANCED,
   /* BEGIN: Modified by c106292, 2008/10/8   PN:Global*/
#ifdef SUPPORT_ATHEROSWLAN  
   WL_SETUP_SES, /*SUPPORT_SES*/
   WL_SETUP_WPSPIN_Enable
 #else
      WL_SETUP_SES /*SUPPORT_SES*/
#endif     
/* END:   Modified by c106292, 2008/10/8 */
} WL_SETUP_TYPE;

/* wl_add_var_check */
typedef struct {
   int  wlSsidIdx;
   char wlMode[WL_SM_SIZE_MAX];
   char wlCountry[WL_SM_SIZE_MAX];
   char wlFltMacMode[WL_SM_SIZE_MAX];
   char wlPhyType[WL_SIZE_2_MAX];
   char wlBasicRate[WL_SIZE_8_MAX];
   char wlProtection[WL_SIZE_8_MAX];
   char wlPreambleType[WL_SM_SIZE_MAX];
   char wlAfterBurnerEn[WL_SIZE_8_MAX];
   char wlFrameBurst[WL_SIZE_4_MAX];
   char wlWlanIfName[WL_SIZE_4_MAX];
   char wlWds[WL_WDS_NUM][WL_MID_SIZE_MAX];
   int  wlCoreRev;
   int  wlEnbl;
   /* start of l68693 added 20080707: 支持局方全局使能WLAN以及用户临时使能WLAN*/   
   int  wlCurEnbl;    //当前WLAN业务是否使能标志
   /* end of l68693 added 20080707 */      
   int  wlChannel;
//BEGIN:add by zhourongfei to periodically check the best channel
   int	wlchannelcheck;
//END:add by zhourongfei to periodically check the best channel
   int  wlFrgThrshld;
   int  wlRtsThrshld;
   int  wlDtmIntvl;
   int  wlBcnIntvl;
   long wlRate;
   /* BEGIN: Modified by c106292, 2008/9/18   PN:Global*/
   /*brcm 原来的:1--54gAuto, 4---54g Performance, 5--(54g LRS),0---802.11b Only
	增加:6--24.11ng, 7--24.11n,    */
   int  wlgMode;
   /* END:   Modified by c106292, 2008/9/18 */
   int  wlLazyWds;
   int  wlBand;
   int  wlMCastRate;
   int  ezc_version;
   int  ezc_enable;
   int  wlInfra;
   int  wlAntDiv;
   int	wlWme;
   /*start of enable or disable the access point radio by l129990,2009-10-9*/
   int  wlRadioCtl;
   /*end of enable or disable the access point radio by l129990,2009-10-9*/
   int  wlWmeNoAck;
   int  wlWmeApsd;
   int  wlTxPwrPcnt;
   int  wlRegMode;
   int  wlDfsPreIsm;
   int  wlDfsPostIsm; 
   int  wlTpcDb;       
   int  wlCsScanTimer;  
   int  wlGlobalMaxAssoc;
#ifdef  SUPPORT_WLAN_PRNTCTL
   int  wlAutoSwOffdays;
   int  wlStarOfftTime;
   int  wlEndOffTime;
#endif   
   /*  Modified by c106292, 2008/9/12   兼顾brcm与atheros方案*/
#if  defined( SUPPORT_SES ) || defined (SUPPORT_ATHEROSWLAN)
   int	wlSesEnable;
   int  wlSesEvent;
   char wlSesStates[WL_SIZE_8_MAX];
   char wlSesSsid[WL_SSID_SIZE_MAX];
   char wlSesWpaPsk[WL_WPA_PSK_SIZE_MAX];  
   int  wlSesHide;
   int  wlSesAuth; 
   char wlSesAuthMode[WL_SIZE_8_MAX]; 
   char wlSesWep[WL_SM_SIZE_MAX];   
   char wlSesWpa[WL_SM_SIZE_MAX];      
   int  wlSesWdsMode;      
   int	wlSesClEnable;   
   int  wlSesClEvent;   
   char wlWdsWsec[WL_SIZE_132_MAX];   
#endif   
#ifdef SUPPORT_MIMO
   int wlNBw; /* runtime */
   int wlNBwCap;
   int wlNCtrlsb;
   int wlNBand;
   int wlNMcsidx;
   char wlNProtection[WL_SIZE_8_MAX];
   char wlRifs[WL_SIZE_8_MAX];
   char wlAmpdu[WL_SIZE_8_MAX];
   char wlAmsdu[WL_SIZE_8_MAX];
   char wlNmode[WL_SIZE_8_MAX];
   int wlNReqd;
#endif      
/* BEGIN: Modified by c106292, 2008/10/8   PN:Global*/
   int wlCountryCode;
   int wlWPSPINEnabled; //0:disabled  1:enables
   char wlPIN[WL_SM_SIZE_MAX];
   int wlWPSEmroleeAddEnabled;  /*//0:disabled  1:enables 注意:是一次性的,用完后立即恢复为0*/
   int wlNeedReloadDrv;   /* 1: NeedReload, 0: Don't need for HT20/40 Bug */
   /* END:   Modified by c106292, 2008/10/8 */
} WIRELESS_VAR, *PWIRELESS_VAR;

/* wl_add_var_check */
typedef struct {
   char wlSsid[WL_SSID_SIZE_MAX];
   char wlKeys128[WL_KEY_NUM][WL_MID_SIZE_MAX];
   char wlKeys64[WL_KEY_NUM][WL_SM_SIZE_MAX];
   char wlWpaPsk[WL_WPA_PSK_SIZE_MAX];
   char wlRadiusKey[WL_RADIUS_KEY_SIZE_MAX];
   char wlWep[WL_SM_SIZE_MAX];
   char wlWpa[WL_SM_SIZE_MAX];
   struct in_addr wlRadiusServerIP;
   char wlAuthMode[WL_SIZE_8_MAX];
   int  wlWpaGTKRekey;
   int  wlRadiusPort;
   int  wlAuth;
   int  wlEnblSsid;
   int  wlKeyIndex128;
   int  wlKeyIndex64;
   int  wlKeyBit;
   int  wlPreauth;
   int  wlNetReauth;
   int  wlNasWillrun; /*runtime*/
   int  wlHide;
   int  wlAPIsolation;
   int  wlMaxAssoc;
   int  wlDisableWme; /* this is per ssid advertisement in beacon/probe_resp */
   char wlFltMacMode[WL_SM_SIZE_MAX];   
#ifdef SUPPORT_TR69C   
   int  tr69cBeaconType;                 
   int  tr69cBasicEncryptionModes;
   int  tr69cBasicAuthenticationMode;	
   int  tr69cWPAEncryptionModes;
   int  tr69cWPAAuthenticationMode;
   int  tr69cIEEE11iEncryptionModes;
   int  tr69cIEEE11iAuthenticationMode;
#endif    
   /*  Modified by c106292, 2008/9/12  兼顾BRCM和ATHEROS*/
#if defined (SUPPORT_WSC) || defined (SUPPORT_ATHEROSWLAN)
   char wsc_mode[12]; // enabled or disabled
   char wsc_config_state[4]; /*0: unconfig 1:Configed*/
#endif
} WIRELESS_MSSID_VAR, *PWIRELESS_MSSID_VAR;


typedef struct {
   char *varName;
   char *varValue;
} WIRELESS_ITEM, *PWIRELESS_ITEM;

typedef struct {
   char macAddress[WL_MID_SIZE_MAX];
   char ssid[WL_SSID_SIZE_MAX];
   char ifcName[WL_SM_SIZE_MAX];
} WL_FLT_MAC_ENTRY, *PWL_FLT_MAC_ENTRY;

typedef struct { // don't want store ssid
   char macAddress[WL_MID_SIZE_MAX];
   char ifcName[WL_SM_SIZE_MAX];
} WL_FLT_MAC_STORE_ENTRY, *PWL_FLT_MAC_STORE_ENTRY;

typedef struct {
   char macAddress[WL_MID_SIZE_MAX];
   char associated;
   char authorized;
   char ssid[WL_SSID_SIZE_MAX];
   char ifcName[WL_SM_SIZE_MAX];
} WL_STATION_LIST_ENTRY, *PWL_STATION_LIST_ENTRY;

// PSI Data
typedef struct {
   int adaptor_index; 
   WIRELESS_VAR wlVar;
   WIRELESS_MSSID_VAR wlMssidVar[WL_MAX_NUM_SSID];  
} WIRELESS_PSI_VARS;

#endif
