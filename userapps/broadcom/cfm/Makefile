# make variables
ARCH          = mips

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)
TOPDIR	:= $(shell /bin/pwd)

INC_APP_PATH      = $(TOPDIR)/inc
INC_TIMER_PATH      = $(TOPDIR)/CBB/timer
INC_API_PATH      = $(TOPDIR)/api
INC_PSI_PATH      = $(TOPDIR)/util/psi
INC_ADSL_CFG_PATH = $(TOPDIR)/main/adslcfgmngr
INC_ATM_CFG_PATH  = $(TOPDIR)/main/atmcfgmngr
INC_IFC_CFG_PATH  = $(TOPDIR)/main/ifccfgmngr
INC_SEC_CFG_PATH  = $(TOPDIR)/main/seccfgmngr
INC_ROUTE_CFG_PATH = $(TOPDIR)/main/routecfgmngr
INC_VOICE_CFG_PATH = $(TOPDIR)/main/vodslcfgmngr
INC_VPN_CFG_PATH = $(TOPDIR)/main/vpncfgmngr
INC_CERT_CFG_PATH  = $(TOPDIR)/main/certcfgmngr
INC_SMB_PATH      = $(TOPDIR)/main/smbcfgmngr
INC_WLAN_PATH     = $(TOPDIR)/main/wlcfgmngr
INC_WEB_PATH      = $(TOPDIR)/web
INC_BUSYBOX_PATH  = $(BUSYBOX_DIR)
INC_BRCM_SNMP_PATH = $(TOPDIR)/../snmp/agent/include
INC_BRCM_NVRAM_PATH = $(TOPDIR)/../nvram
INC_BRCM_WLCTL_PATH = $(TOPDIR)/../wlctl
INC_TRC69C_PATH   = $(TOPDIR)/../tr69c/inc
CRYPTO_DIR        = $(SSHD_DIR)/libtomcrypt
MATH_DIR          = $(SSHD_DIR)/libtommath
ATM2684_DIR			= $(TOPDIR)/../../opensource/atm2684/br2684ctl
INC_TR64_PATH   = $(TOPDIR)/../tr064/inc
TR64_DIR  =  $(TOPDIR)/../tr064

CFM_OBJS_DIR		  = $(TOPDIR)/objs

ifeq ($(strip $(WEB_POPUP)),y)
	WEBFLAGS := -DWEB_POPUP
else
	WEBFLAGS :=
endif

ifeq ($(strip $(BUILD_VLAN_CONFIG)),y)
	WEBFLAGS += -DSUPPORT_VLAN_CONFIG
endif

ifeq ($(strip $(BUILD_RT30XX_WLAN)),y)
	WEBFLAGS += -DATP_RT30XX_WLAN
endif
ifeq ($(strip $(BUILD_MBSS)),y)
	WEBFLAGS += -DSUPPORT_MBSS
endif
ifeq ($(strip $(BRCM_CHIP)),6348)
	WEBFLAGS += -DCHIP_6348
endif
ifeq ($(strip $(BRCM_CHIP)),6358)
	WEBFLAGS += -DCHIP_6358
endif
ifeq ($(strip $(BRCM_CHIP)),6338)
	WEBFLAGS += -DCHIP_6338
endif

ifneq ($(strip $(BUILD_SNMP)),)
	WEBFLAGS += -DSUPPORT_SNMP
endif
ifneq ($(strip $(BUILD_RADIUS)),)
	WEBFLAGS += -DSUPPORT_RADIUS
endif

ifneq ($(strip $(BUILD_ZEBRA)),)
	WEBFLAGS += -DSUPPORT_RIP
endif

ifneq ($(strip $(BUILD_ETHWAN)),)
	WEBFLAGS += -DSUPPORT_ETHWAN
endif

ifneq ($(strip $(BUILD_LSWWAN)),)
	WEBFLAGS += -DSUPPORT_LSWWAN
endif

ifneq ($(strip $(BUILD_PORT_MIRRORING)),)
WEBFLAGS += -DPORT_MIRRORING -DDPORTMIRROR_DEBUG -I$(ATM2684_DIR)
endif

ifneq ($(strip $(BUILD_VDSL)),)
WEBFLAGS += -DSUPPORT_VDSL
else
WEBFLAGS += -DSUPPORT_ADSL
endif

ifeq ($(strip $(BUILD_FXO_REINJECTION)),y)
WEBFLAGS += -DSUPPORT_FXO_REINJECTION
endif

ifeq ($(strip $(HRD_HG520V)),y)
	WEBFLAGS += -DCONFIG_HRD_HG520V
endif
ifeq ($(strip $(HRD_HG510V)),y)
	WEBFLAGS += -DCONFIG_HRD_HG510V
endif
ifeq ($(strip $(HRD_HG510VI)),y)
        WEBFLAGS += -DCONFIG_HRD_HG510VI
endif
ifeq ($(strip $(HRD_HG550II)),y)
        WEBFLAGS += -DCONFIG_HRD_HG550II
endif

ifeq ($(strip $(BUILD_WLAN_REVERSE_PRESS_RESET_KEY)),y)
	WEBFLAGS += -DSUPPORT_WLAN_REVERSE_PRESS_RESET_KEY
endif

ifeq ($(strip $(BUILD_SLIC_3210)),y)
	WEBFLAGS += -DSUPPORT_SLIC_3210
endif

ifeq ($(strip $(BUILD_SLIC_3215)),y)
	WEBFLAGS += -DSUPPORT_SLIC_3215
endif

ifeq ($(strip $(BUILD_SLIC_LE88221)),y)
	WEBFLAGS += -DSUPPORT_SLIC_LE88221
endif

ifneq ($(strip $(BUILD_KPN_QOS)),)
WEBFLAGS += -DSUPPORT_KPN_QOS
endif

ifneq ($(strip $(BUILD_TDE_QOS)),)
WEBFLAGS += -DSUPPORT_TDE_QOS
endif

ifneq ($(strip $(BUILD_VDF_QOS)),)
WEBFLAGS += -DSUPPORT_VDF_QOS
endif

ifneq ($(strip $(BUILD_VDF_SEC)),)
WEBFLAGS += -DSUPPORT_VDF_SEC
endif

ifneq ($(strip $(BUILD_SUPPORT_POLICY)),)
WEBFLAGS += -DSUPPORT_POLICY
endif

ifneq ($(strip $(BUILD_SUPPORT_GLB_PRO)),)
	WEBFLAGS += -DSUPPORT_GLB_PRO
	WEBFLAGS += -DSUPPORT_GLB_ALG
	WEBFLAGS += -DSUPPORT_GLB_PVC
	WEBFLAGS += -DSUPPORT_GLB_MTU
endif

ifneq ($(strip $(BUILD_SUPPORT_SNTP_AUTOCONF)),)
WEBFLAGS += -DSUPPORT_SNTP_AUTOCONF
endif

ifneq ($(strip $(BUILD_SUPPORT_VDF_DHCP)),)
WEBFLAGS += -DSUPPORT_VDF_DHCP
endif

ifneq ($(strip $(BUILD_SUPPORT_CD_WEB)),)
WEBFLAGS += -DSUPPORT_CD_WEB
endif
ifneq ($(strip $(BUILD_SUPPORT_VDF_DIAGNOSE)),)
WEBFLAGS += -DSUPPORT_VDF_DIAGNOSE
endif

ifneq ($(strip $(BUILD_SCOUT)),)
WEBFLAGS += -DSUPPORT_SCOUT
endif

ifneq ($(strip $(BUILD_SUPPORT_VDF_DMZ)),)
WEBFLAGS += -DSUPPORT_VDF_DMZ
endif

ifneq ($(strip $(BUILD_DHCP_FRAG)),)
WEBFLAGS += -DSUPPORT_DHCP_FRAG
endif

ifneq ($(strip $(BUILD_CHINATELECOM_DHCP)),)
WEBFLAGS += -DSUPPORT_CHINATELECOM_DHCP
endif

ifneq ($(strip $(BUILD_TFTP)),)
WEBFLAGS += -DSUPPORT_TFTP
endif

ifneq ($(strip $(BUILD_TFTPD)),)
WEBFLAGS += -DSUPPORT_TFTPD
endif
        
ifneq ($(strip $(BUILD_BRCM_VLAN)),)
WEBFLAGS	+= -DSUPPORT_VLAN
endif
ifneq ($(strip $(BUILD_VDSL)),)
WEBFLAGS	+= -DSUPPORT_VDSL
endif
        
ifneq ($(strip $(BUILD_FTPD)),)
	WEBFLAGS += -DSUPPORT_FTPD
endif

ifeq  ($(strip $(BRCM_DRIVER_ETHERNET_CONFIG)),y)
	WEBFLAGS += -DETH_CFG
endif

ifneq ($(strip $(BUILD_UPNP)),)
	WEBFLAGS += -DSUPPORT_UPNP
endif

ifneq ($(strip $(BUILD_DDNSD)),)
	WEBFLAGS += -DSUPPORT_DDNSD
endif

ifneq ($(strip $(BUILD_IGMPCONFIG)),)
	WEBFLAGS += -DSUPPORT_IGMPCONFIG
endif

ifneq ($(strip $(BUILD_SNTP)),)
	WEBFLAGS += -DSUPPORT_SNTP
endif

ifneq ($(strip $(BUILD_IPTABLES)),)
	WEBFLAGS += -DSUPPORT_IPTABLES
endif

ifneq ($(strip $(BUILD_EBTABLES)),)
	WEBFLAGS += -DSUPPORT_EBTABLES
endif

ifneq ($(strip $(BUILD_TOD)),)
	WEBFLAGS += -DSUPPORT_TOD
endif

ifneq ($(strip $(BUILD_SIPROXD)),)
	WEBFLAGS += -DSUPPORT_SIPROXD
endif

ifneq ($(strip $(BUILD_PORTMAPING)),)
	WEBFLAGS += -DSUPPORT_PORTMAPING
endif

ifneq ($(strip $(BUILD_VCONFIG)),)
	WEBFLAGS += -DSUPPORT_PORT_MAP
endif

ifneq ($(strip $(BUILD_IPSEC_TOOLS)),)
	WEBFLAGS += -DSUPPORT_IPSEC
endif

ifneq ($(strip $(BUILD_CERT)),)
	WEBFLAGS += -DSUPPORT_CERT
endif

ifneq ($(strip $(BRCM_KERNEL_NETQOS)),)
	WEBFLAGS += -DSUPPORT_TC
endif

ifneq ($(strip $(BUILD_ETHWAN)),)
	WEBFLAGS += -DSUPPORT_ETHWAN
endif
        WEBFLAGS += -DHSPA_VOICE

ifneq ($(strip $(BUILD_IPPD)),)
	WEBFLAGS += -DSUPPORT_IPP
endif

ifneq ($(strip $(BUILD_WL_MAC_AS_SSID)),)
	WEBFLAGS += -DSUPPORT_WL_MAC_AS_SSID
endif

ifneq ($(strip $(BUILD_SNTP_AUTOCONF)),)
	WEBFLAGS += -DSUPPORT_SNTP_AUTOCONF
endif

ifneq ($(strip $(BUILD_SYSLOG_AUTOCONF)),)
	WEBFLAGS += -DSUPPORT_SYSLOG_AUTOCONF
endif

ifneq ($(strip $(BUILD_ONE_USER)),)
	WEBFLAGS += -DSUPPORT_ONE_USER
endif

ifeq ($(strip $(BUILD_SUPPORT_WEB_AUTH)),y)
	WEBFLAGS += -DSUPPORT_WEB_AUTH
endif

# web language,English by default
ifeq ($(strip $(BUILD_SUPPORT_WEB_LANG)),english)
	WEBFLAGS += -DSUPPORT_WEB_LANG_ENGLISH
	WEB_SOURCE_STATIC_DIR=html/english/static
	WEB_SOURCE_HSPA_DIR=html/english/hspa
endif
ifeq ($(strip $(BUILD_SUPPORT_WEB_LANG)),italy)
	#WEBFLAGS += -DSUPPORT_WEB_LANG_ITALY
	#WEB_SOURCE_STATIC_DIR=html/italy/static
	#WEB_SOURCE_HSPA_DIR=html/italy/hspa
	echo "No web source for italy"
	exit 1;
endif
ifeq ($(strip $(BUILD_SUPPORT_WEB_LANG)),spanish)
	WEBFLAGS += -DSUPPORT_WEB_LANG_SPANISH
	WEB_SOURCE_STATIC_DIR=html/spanish/static
	WEB_SOURCE_HSPA_DIR=html/spanish/hspa
endif

ifneq ($(strip $(BUILD_PASSWORD_UNENCRYPTED)),)
	WEBFLAGS += -DSUPPORT_PASSWORD_UNENCRYPTED
endif

ifneq ($(strip $(BUILD_SAMBA)),)
	WEBFLAGS += -DSUPPORT_SAMBA
endif


ifeq ($(strip $(BUILD_CFM_CLI)),y)
	export CLI_CMD_EDIT = y
	CLIFLAGS := -DCLI_CMD

	ifeq ($(strip $(CLI_CMD_EDIT)),y)
		CLIFLAGS += -DCLI_CMD_EDIT
	endif

	ifeq ($(strip $(BUILD_CFM_TELNETD)),y)
		DIR_TELNETD := telnet/
		OBJS_TELNETD := $(DIR_TELNETD)telnet.o
		CLIFLAGS += -DCLI_TELNETD
	else
		DIR_TELNETD :=
		OBJS_TELNETD :=
	endif
 	ifeq ($(strip $(BUILD_FTPD)),y)
		DIR_BFTPD := $(OPENSOURCE_DIR)/ftpd
		OBJS_BFTPD := $(DIR_BFTPD)/bftpd.o
		CLIFLAGS += -DBUILD_FTPD
	else
		DIR_BFTPD :=
		OBJS_BFTPD :=
	endif
	ifeq ($(strip $(BUILD_CFM_SSHD)),y)
		SSHD_DIR := $(OPENSOURCE_DIR)/sshd
		OBJS_SSHD := $(SSHD_DIR)/sshd.o
		CRYPTO_LIB := $(CRYPTO_DIR)/libtomcrypt.a
		MATH_LIB := $(MATH_DIR)/libtommath.a		
		CLIFLAGS += -DCLI_SSHD
		ifeq ($(strip $(BUILD_SSHD_MIPS_GENKEY)),y)
			MIPS_GENKEY_DIR := $(SSHD_DIR)/genkey
			MIPS_GENKEY := $(MIPS_GENKEY_DIR)/mips_genkey
		else
			MIPS_GENKEY_DIR :=
			MIPS_GENKEY :=
		endif
	else
		SSHD_DIR :=
		OBJS_SSHD :=
		CRYPTO_LIB :=
		MATH_LIB :=
	endif
	endif

ifeq  ($(strip $(BUILD_CFM_MENU)),y)
	CLIFLAGS += -DBUILD_MENU_DRIVEN_CLI
	export BUILD_MENU_DRIVEN_CLI=y

	ifneq ($(strip $(BUILD_CFM_CLI)),y)
		ifeq ($(strip $(BUILD_CFM_TELNETD)),y)
			DIR_TELNETD := telnet/
			OBJS_TELNETD := $(DIR_TELNETD)telnet.o
			CLIFLAGS += -DCLI_TELNETD
		else
			DIR_TELNETD :=
			OBJS_TELNETD :=
		endif
		ifeq ($(strip $(BUILD_FTPD)),y)
			DIR_BFTPD := $(OPENSOURCE_DIR)/ftpd
			OBJS_BFTPD := $(DIR_BFTPD)/bftpd.o
			CLIFLAGS += -DBUILD_FTPD
		else
			DIR_BFTPD :=
			OBJS_BFTPD :=
		endif
		ifeq ($(strip $(BUILD_CFM_SSHD)),y)
			SSHD_DIR := $(OPENSOURCE_DIR)/sshd
			OBJS_SSHD := $(SSHD_DIR)/sshd.o
			CLIFLAGS += -DCLI_SSHD
			ifeq ($(strip $(BUILD_SSHD_MIPS_GENKEY)),y)
				MIPS_GENKEY_DIR := $(SSHD_DIR)/genkey
				MIPS_GENKEY := $(MIPS_GENKEY_DIR)/mips_genkey
			else
				MIPS_GENKEY_DIR :=
				MIPS_GENKEY :=
			endif
		else
			SSHD_DIR :=
			OBJS_SSHD :=
		endif
	endif
endif

ifeq ($(strip $(BUILD_SOAP)),y)
	SOAPCFLAGS=-DSOAPSERVER
endif
ifneq ($(strip $(BUILD_DIAGAPP)),)
	export BUILD_DIAGAPP=y
endif
ifeq ($(strip $(BUILD_SOAP_VER)),1)
	DIR_SOAP := soap/
	SOAP_SERVER := soap.o
	OBJS_SOAP := $(DIR_SOAP)$(SOAP_SERVER)
else
	DIR_SOAP := SoapToolkit/SoapServer/
	OBJ_SAVED_SOAP_EXIST := $(shell find . -name "soapserver_save" -print)
	ifeq ($(OBJ_SAVED_SOAP_EXIST),)
		SOAP_SERVER := soapserver.o
	else
		SOAP_SERVER := soapserver_save
	endif
	OBJS_SOAP := $(DIR_SOAP)$(SOAP_SERVER)
endif

ifeq ($(strip $(ADSL)),)
export ADSL     = ANNEX_A
endif


ifeq ($(strip $(ADSL)),ANNEX_A)
WEBFLAGS += -DADSL_ANNEX_A
endif

ifeq ($(strip $(ADSL)),ANNEX_B)
WEBFLAGS += -DADSL_ANNEX_B
endif

ifeq ($(strip $(VCOPE_TYPE)),CO)
	VCOPE_CO_FLAG := -DCO_VCOPE
else
	VCOPE_CO_FLAG :=
endif

ifeq ($(strip $(WIRELESS)),1)
	WLFLAGS := -DWIRELESS
	WEBFLAGS+= -DDEVICE_ASSOCIATION
   ifeq ($(strip $(BUILD_WL_MIMO)),y)  
	WLFLAGS += -DSUPPORT_MIMO
   endif  
   ifeq ($(strip $(BUILD_WLCTL_SHLIB)),1)
	WLFLAGS += -DDSLCPE_SHLIB
   endif
else
   WLFLAGS :=
endif

ifneq ($(strip $(BRCM_DRIVER_USB)),)
	export USB=1
endif

ifeq ($(strip $(VOXXXLOAD)),1)
	VODSLFLAGS := -DVOXXXLOAD
    VODSLFLAGS += -I$(XCHANGE_DIR)/dslx/src/mta/inc/cfg
    VODSLFLAGS += -I$(XCHANGE_DIR)/dslx/apps/$(XCHANGE_BUILD_APP)/cfginc/mta
    VODSLFLAGS += -I${XCHANGE_DIR}/dslx_common/voice_res_gw/inc
    VODSLFLAGS += -I$(USERAPPS_DIR)/broadcom/vodsl/voip/inc
    VODSLFLAGS += $(BRCM_DSP_CODEC_DEFINES)
    VODSLFLAGS += $(BRCM_DSP_CHAN_DEFINES)
    VODSLFLAGS += -I$(TOPDIR)/../../huawei/voice/TAPI
    VODSLFLAGS += -I$(TOPDIR)/../../huawei/voice/include
  ifeq ($(strip $(MGCPLOAD)),1)
    VODSLFLAGS += -DIP_MGCP
  endif
  ifeq ($(strip $(SIPLOAD)),1)
    VODSLFLAGS += -DIP_SIP
  endif
  ifeq ($(strip $(VOAAL2LOAD)),1)
    VODSLFLAGS += -DVOAAL2
  endif

  ifeq ($(strip $(BRCM_VODSL_STUN_CLIENT)),1)
     VODSLFLAGS += -DSTUN_CLIENT
  endif
  ifeq ($(strip $(BRCM_VODSL_DUAL_3341)), y)
     VODSLFLAGS += -DBRCM_DUAL_3341
  endif

  ifneq ($(strip $(BUILD_SIPROXD)),)
     VODSLFLAGS += -DEXTERNAL_SIP_ALG
  endif

  ifeq ($(strip $(BRCM_6358_G729_FXO)),y)
    VODSLFLAGS += -DSUPPORT_PSTN
  endif
  
  ifeq ($(strip $(BRCM_6358_WIDEBAND_FXO)),y)
    VODSLFLAGS += -DSUPPORT_PSTN
  endif
  
else
	VODSLFLAGS :=
endif

ifeq ($(strip $(BRCM_PROFILER_TOOL)),1)
	PROFILERFLAGS += -DBRCM_PROFILER_ENABLED
endif

ifneq ($(strip $(BUILD_ATHEROSWLAN)),)
	WEBFLAGS += -DSUPPORT_ATHEROSWLAN
endif
ifneq ($(strip $(BUILD_SES)),)
	WEBFLAGS += -DSUPPORT_SES
endif

ifneq ($(strip $(BUILD_WLAN_PRNTCTL)),)
	WEBFLAGS += -DSUPPORT_WLAN_PRNTCTL
endif

ifneq ($(strip $(BUILD_WSC)),)
	WEBFLAGS += -DSUPPORT_WPS
	WEBFLAGS += -DSUPPORT_WSC
endif

ifneq ($(strip $(BUILD_NVRAM)),)
	WEBFLAGS += -DSUPPORT_NVRAM
endif

ifeq ($(strip $(RUN_NOISE)),1)
   DBGFLAGS := -DBRCM_DEBUG
   BUILTTIME := $(shell date '+%H%M_%m%d%y')-
else
   DBGFLAGS :=
   BUILTTIME :=
endif

ifeq ($(strip $(BRCM_PROFILER_TOOL)),1)
	PROFILERFLAGS += -DBRCM_PROFILER_ENABLED
endif

ifneq ($(strip $(BRCM_SNMP)),)
	DIR_BRCM_SNMP := ../snmp/
	OBJS_BRCM_SNMP := $(DIR_BRCM_SNMP)snmp.o
	WEBFLAGS   += -DSUPPORT_BRCM_SNMP
ifeq ($(strip $(BUILD_SNMP_CHINA_TELECOM_CPE_MIB)),y)
	WEBFLAGS += -DCPE_DSL_MIB
endif
ifeq ($(strip $(BUILD_SNMP_DEBUG)),y)
	WEBFLAGS += -DBUILD_SNMP_DEBUG
endif

else
	DIR_BRCM_SNMP :=
	OBJS_BRCM_SNMP :=
endif
	
ifeq ($(strip $(BUILD_4_LEVEL_QOS)),y)
	WEBFLAGS += -DFOUR_LEVEL_QOS
endif
	
ifneq ($(strip $(BUILD_CHINA_TELCOM_TEST_WARNING)),)
	WEBFLAGS += -DSUPPORT_CHINA_TELCOM_TEST_WARNING
endif

ifeq ($(strip $(BUILD_SINGAPORE_LOGIN)),y)
	WEBFLAGS += -DSINGAPORE_LOGIN
endif

ifeq ($(strip $(BUILD_DIF_UE_PORTS)),y)
	WEBFLAGS += -DSUPPORT_DIF_UE_PORTS
endif

ifneq ($(strip $(BUILD_MAC)),)
	WEBFLAGS += -DSUPPORT_MAC
endif

ifneq ($(strip $(BUILD_VDF_MACMATCHIP)),)
	WEBFLAGS += -DSUPPORT_MACMATCHIP
endif

#************************************************************
# liujianfeng
# 修改tr069权限问题，允许没有tr069目录访问权限的人也能编译
# tr069
# 修改方法：将tr069编译的目标文件tr69clib.o放在cfm/objs/tr69c
# 里面，如果没有tr69c目录访问权限，则使用这个目标文件来编译
# tr069,如果有tr69c目录访问权限，则还是使用tr69c目录的代码编译
# 以后如果cfm部分有其他的权限限制要求，也可以按照类似的方式来做
#************************************************************

ifneq ($(strip $(BUILD_TR69C)),)
	ifneq ($(strip $(shell ls ../tr69c|grep Makefile)),)
		DIR_TR69C := ../tr69c/
	else
		DIR_TR69C :=
	endif
	OBJ_SAVED_TR69C_EXIST := $(shell find ../tr69c -name "tr69clib_save" -print)
	ifeq ($(OBJ_SAVED_TR69C_EXIST),)
		TR69_CLIENT := tr69clib.a
	else
		TR69_CLIENT := tr69clib_save
	endif
	DIR_OBJS_TR69C := $(CFM_OBJS_DIR)/tr69c/
	OBJS_TR69C := $(DIR_OBJS_TR69C)$(TR69_CLIENT)
	WEBFLAGS += -DSUPPORT_TR69C
else
ifneq ($(strip $(BUILD_TR69C_SSL)),)
	ifneq ($(strip $(shell find ../tr69c -name "Makefile" -print)),)
		DIR_TR69C := ../tr69c/
	else
		DIR_TR69C :=
	endif
	OBJ_SAVED_TR69C_EXIST := $(shell find ../tr69c -name "tr69clib_save" -print)
	ifeq ($(OBJ_SAVED_TR69C_EXIST),)
		TR69_CLIENT := tr69clib.a
	else
		TR69_CLIENT := tr69clib_save
	endif
	DIR_OBJS_TR69C := $(CFM_OBJS_DIR)/tr69c/
	OBJS_TR69C := $(DIR_OBJS_TR69C)$(TR69_CLIENT)
	WEBFLAGS += -DSUPPORT_TR69C
else
	DIR_TR69C :=
	OBJS_TR69C :=
endif
endif

ifneq ($(strip $(BUILD_TR64)),)
	ifneq ($(strip $(shell ls ../tr064|grep Makefile)),)
		DIR_TR64 := ../tr064/
	else
		DIR_TR64 :=
	endif
	OBJ_SAVED_TR64_EXIST := $(shell find ../tr064 -name "tr64lib_save" -print)
	ifeq ($(OBJ_SAVED_TR64_EXIST),)
		TR64_CLIENT := tr64lib.a
	else
		TR64_CLIENT := tr64lib_save
	endif
	DIR_OBJS_TR64 := $(CFM_OBJS_DIR)/tr064/
	OBJS_TR64 := $(DIR_OBJS_TR64)$(TR64_CLIENT)
	WEBFLAGS += -DSUPPORT_TR64
else
ifneq ($(strip $(BUILD_TR64_SSL)),)
	ifneq ($(strip $(shell find ../tr064 -name "Makefile" -print)),)
		DIR_TR64 := ../tr064/
	else
		DIR_TR64 :=
	endif
	OBJ_SAVED_TR64_EXIST := $(shell find ../tr064 -name "tr64lib_save" -print)
	ifeq ($(OBJ_SAVED_TR64_EXIST),)
		TR64_CLIENT := tr64lib.a
	else
		TR64_CLIENT := tr64lib_save
	endif
	DIR_OBJS_TR64 := $(CFM_OBJS_DIR)/tr064/
	OBJS_TR64 := $(DIR_OBJS_TR64)$(TR64_CLIENT)
	WEBFLAGS += -DSUPPORT_TR64
else
	DIR_TR64 :=
	OBJS_TR64 :=
endif
endif

TR64_LDFLAGS	 	+=	-L$(TR64_DIR)/main	
TR64_LIBS		+=	 -ltr064sdkr -ltsp_xmlr


ifeq ($(strip $(BUILD_CUSTOMER_HUVDF)),y)
	WEBFLAGS += -DSUPPORT_CUSTOMER_HUVDF
endif

ifeq ($(strip $(BUILD_CUSTOMER_ITALY)),y)
	WEBFLAGS += -DSUPPORT_CUSTOMER_ITALY
endif 

ifeq ($(strip $(BUILD_BACKUPCFGFILE)),y)
	WEBFLAGS += -DSUPPORT_BACKUPCFGFILE
endif

ifeq ($(strip $(BUILD_VDF_WANINFO)),y)
	WEBFLAGS += -DSUPPORT_VDF_WANINFO
endif


ARFLAGS    = rcs
DEBUG_FLAGS	:= -g -DDBG=1 -DDEBUG -DBRCM_DEBUG
FLAGS := $(CLIFLAGS) $(SOAPCFLAGS) -D$(ADSL) $(WLFLAGS) $(VODSLFLAGS) $(WEBFLAGS) $(VCOPE_VDSL) \
				$(DBGFLAGS) $(PROFILERFLAGS) -DLINUX -Dmips -Wall -G 0 -Os \
				-DHGW_PRODUCTNAME=\"$(HGW_PRODUCTNAME)\" \
				-DHGW_SOFTVERSION=\"$(HGW_SOFTVERSION)\" \
				-fomit-frame-pointer -fno-strict-aliasing -fno-exceptions \
				-I. -I$(INC_APP_PATH) -I$(INC_API_PATH) -I$(INC_PSI_PATH) -I$(INC_WEB_PATH) \
				-I$(INC_ADSL_CFG_PATH) -I$(INC_ATM_CFG_PATH) -I$(INC_IFC_CFG_PATH) \
				-I$(INC_SEC_CFG_PATH) -I$(INC_VOICE_CFG_PATH) -I$(INC_BUSYBOX_PATH) \
				-I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
				-I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) \
				-I$(INC_BRCM_SNMP_PATH) -I$(INC_TRC69C_PATH) -I$(INC_ROUTE_CFG_PATH) \
				-I$(INC_VPN_CFG_PATH) -I$(INC_CERT_CFG_PATH) -I$(INC_TR64_PATH) -I$(INC_SMB_PATH) -I$(INC_TIMER_PATH)
CFLAGS   := -Wstrict-prototypes $(FLAGS)
CXXFLAGS := -fno-rtti $(FLAGS)

ifeq ($(strip $(BUILD_CFM)),static)
CFLAGS   += -DBUILD_STATIC
CXXFLAGS += -DBUILD_STATIC
endif

ifneq ($(strip $(BUILD_POLICY_ROUTE)),)
	CFLAGS += -DSUPPORT_POLICY_ROUTE
	CXXFLAGS += -DSUPPORT_POLICY_ROUTE
endif

ifneq ($(strip $(BUILD_DHCPOPTION)),)
	CFLAGS += -DSUPPORT_DHCPOPTIONS
	CXXFLAGS += -DSUPPORT_DHCPOPTIONS
endif

ifneq ($(strip $(BUILD_DHCPHOSTID)),)
	CFLAGS += -DSUPPORT_DHCPHOSTID
	CXXFLAGS += -DSUPPORT_DHCPHOSTID
endif

ifneq ($(strip $(BUILD_DHCPCLIENTID)),)
	CFLAGS += -DSUPPORT_DHCPCLIENTID
	CXXFLAGS += -DSUPPORT_DHCPCLIENTID
endif

ifneq ($(strip $(BUILD_IGMPCONFIG)),)
	CFLAGS += -DSUPPORT_IGMPCONFIG
	CXXFLAGS += -DSUPPORT_IGMPCONFIG
endif

ifneq ($(strip $(BUILD_PPPOUCONFIG)),)
	CFLAGS += -DSUPPORT_PPPOUCONFIG
	CXXFLAGS += -DSUPPORT_PPPOUCONFIG
endif

ifneq ($(strip $(BUILD_DHCP_FRAG)),)
	CFLAGS += -DSUPPORT_DHCP_FRAG
	CXXFLAGS += -DSUPPORT_DHCP_FRAG
endif

ifneq ($(strip $(BUILD_CHINATELECOM_DHCP)),)
	CFLAGS += -DSUPPORT_CHINATELECOM_DHCP
	CXXFLAGS += -DSUPPORT_CHINATELECOM_DHCP
endif

export ARCH TOPDIR CFLAGS CXXFLAGS CLIFLAGS SOAPCFLAGS ADSL WLFLAGS VODSLFLAGS WEBFLAGS \
	INC_APP_PATH INC_API_PATH INC_PSI_PATH INC_WEB_PATH INC_TRC69C_PATH INC_ROUTE_CFG_PATH \
	INC_ADSL_CFG_PATH INC_ATM_CFG_PATH INC_IFC_CFG_PATH EXTRA_LIB \
	INC_BRCMDRIVER_PUB_PATH INC_BRCMDRIVER_PRIV_PATH INC_BRCMSHARED_PUB_PATH INC_BRCMSHARED_PRIV_PATH BRCM_BOARD INC_BRCM_SNMP_PATH \
	INC_VPN_CFG_PATH INC_CERT_CFG_PATH CFM_OBJS_DIR -I$(INC_TR64_PATH) INC_SMB_PATH

# objects variables
#DIR_API := api/
#OBJS_API := $(DIR_API)api.o
DIR_UTIL := util/
OBJS_UTIL := $(DIR_UTIL)util.o
#DIR_MAIN := main/
#OBJS_MAIN := $(DIR_MAIN)main.o
#DIR_WEB := web/
#OBJS_WEB := $(DIR_WEB)web.o
#DIR_DNS := dns/
#OBJS_DNS := $(DIR_DNS)dnspr.o

#SUBDIRS := $(DIR_API) $(DIR_UTIL) $(DIR_MAIN) $(DIR_WEB) $(DIR_DNS) $(DIR_TELNETD) $(DIR_BFTPD) $(SSHD_DIR) \
#	 $(DIR_BRCM_SNMP) $(DIR_TR69C) $(DIR_TR64) 
#OBJ_SOAP_EXIST = $(shell find . -name $(SOAP_SERVER) -print)
#ifeq ($(OBJ_SOAP_EXIST),)
	#SUBDIRS += $(DIR_SOAP)
#endif
#OBJS := $(OBJS_API) $(OBJS_UTIL) $(OBJS_MAIN) $(OBJS_WEB) $(OBJS_DNS) $(OBJS_TELNETD) $(OBJS_SSHD) $(OBJS_BFTPD) \
	#$(OBJS_BRCM_SNMP) $(OBJS_TR69C) $(OBJS_TR64) $(CRYPTO_LIB) $(MATH_LIB)
	
#ifeq ($(strip $(BUILD_SOAP)),y)
	#OBJS += $(OBJS_SOAP)
#endif

# make default
EXECS = cfm

# additional libraries
EXTRA_LIB=-lcrypt -lutil

#yuyouqing42304 added: HG510v support tr069, so need pthead library
#ifeq ($(strip $(BUILD_VDSL)),)
#EXTRA_LIB += -lpthread
#endif
#ifeq ($(strip $(HRD_HG510V)),y) 
#EXTRA_LIB += -lpthread
#endif
#ifeq ($(strip $(HRD_HG510VI)),y) 
#EXTRA_LIB += -lpthread
#endif
#EXTRA_LIB +=$(TOPDIR)/timer/libvtop_linuxr.a
# add ssl lib for https protocol.
#ifneq ($(strip $(BUILD_TR69C_SSL)),)
#EXTRA_LIB+=-L$(OPENSOURCE_DIR)/openssl -lssl -lcrypto
#endif

#ifneq ($(strip $(BUILD_TR64_SSL)),)
#EXTRA_LIB+=-L$(OPENSOURCE_DIR)/openssl -lssl -lcrypto 
#EXTRA_LIB+=$(TR64_LDFLAGS) $(TR64_LIBS)
#endif

#ifneq ($(strip $(BUILD_NVRAM)),)
#	EXTRA_LIB += -lnvram
#ifeq ($(strip $(BUILD_WLCTL_SHLIB)),1)
 #  	EXTRA_LIB += -lwlctl
#endif
#endif

#All: $(EXECS)
All: dynamic

# create version.h
version:
	rm -f $(INC_APP_PATH)/version.h
	@echo "/* Full software version */" > $(INC_APP_PATH)/version.h
	@echo '#define SOFTWARE_VERSION' \"$(BUILTTIME)$(BRCM_RELEASETAG)\" >> $(INC_APP_PATH)/version.h
	@echo '#define RELEASE_VERSION' \"$(BRCM_RELEASETAG)\" >> $(INC_APP_PATH)/version.h
	@echo '#define PSI_VERSION' \"$(BRCM_PSI_VERSION)\" >> $(INC_APP_PATH)/version.h
	@echo "/* Software version used for query */" >> $(INC_APP_PATH)/version.h
	@echo '#define RELEASE_VERSION_VRB' \"$(BRCM_RELEASE_VERSION)\" >> $(INC_APP_PATH)/version.h

sharedlib:
	$(MAKE) -C $(INC_PSI_PATH)
	cp -f $(INC_PSI_PATH)/*.so $(INSTALL_DIR)/lib

install:
	install -m 755 cfm $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/cfm
ifneq ($(strip $(BUILD_WSC)),)
	install -m 755 wpsled $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/wpsled
endif	
	
	
$(EXECS): subdirs
	$(CC) -Wl,-allow-shlib-undefined $(CFLAGS) --verbose -o cfm $(OBJS) \
		-lpsi -lpsixml -ldl -lm $(EXTRA_LIB) -L$(INC_PSI_PATH) -L$(INC_BRCM_NVRAM_PATH) -L$(INC_BRCM_WLCTL_PATH)


ifneq ($(strip $(BUILD_WSC)),)
wpsled:$(INC_WLAN_PATH)/wlwpsled.c $(INC_APP_PATH)/board_api.h $(INC_APP_PATH)/wlapi.h
	$(CC) -Wl,-allow-shlib-undefined $(CFLAGS) --verbose -o wpsled $(INC_WLAN_PATH)/wlwpsled.c   \
	       -lpsi -lnvram -lwlctl  -L$(INC_PSI_PATH) -L$(INC_BRCM_NVRAM_PATH) -L$(INC_BRCM_WLCTL_PATH)
else
wpsled:
endif


lib: subdirs
	$(AR) $(ARFLAGS) $(EXECS).a $(OBJS)

dynamic:  ftpd untarsshd version sharedlib $(EXECS) wpsled install webuihtml mipsgenkey rmwl_html rmvoice_html rmipsec_html rmcert_html rm_redundant

ftpd:
	cd $(OPENSOURCE_DIR);	(tar xkfj ftpd.tar.bz2 2> /dev/null || true) 
	$(MAKE) -C $(OPENSOURCE_DIR)/ftpd 
#$(BUILD_FTPD)

ifeq ($(strip $(BUILD_CFM_SSHD)),y)
untarsshd:
	cd $(OPENSOURCE_DIR);   (tar xkfj sshd.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(CRYPTO_DIR)
	$(MAKE) -C $(MATH_DIR)
else
untarsshd:
endif

static: version lib webuihtml rmwl_html rmvoice_html rm_redundant

webuihtml:
	mkdir -p $(INSTALL_DIR)/webs
	mkdir -p $(INSTALL_DIR)/doc
ifneq ($(strip $(BUILD_SUPPORT_CD_WEB)),)
	mkdir -p $(INSTALL_DIR)/cd
	cp -rf html/cd/* $(INSTALL_DIR)/cd
endif
	#cp -f $(WEB_SOURCE_STATIC_DIR)/*.html $(WEB_SOURCE_STATIC_DIR)/*.js $(WEB_SOURCE_STATIC_DIR)/*.gif $(WEB_SOURCE_STATIC_DIR)/*.css $(WEB_SOURCE_STATIC_DIR)/*.ico $(WEB_SOURCE_STATIC_DIR)/*.png $(INSTALL_DIR)/webs
	#adapt to VOX <tanyin>
	cp -rf html/vox/* $(INSTALL_DIR)/webs
	#cp -f html/huawei/*.html html/huawei/*.js html/huawei/*.gif html/huawei/*.css $(INSTALL_DIR)/doc
	cp -rf html/huawei/* $(INSTALL_DIR)/doc
	rm -rf $(INSTALL_DIR)/doc/en_US/athwlan
	rm -rf $(INSTALL_DIR)/doc/es_ES/athwlan

ifneq ($(strip $(BUILD_ATHEROSWLAN)),)
	#cp -f html/huawei/athwlan/*.html $(INSTALL_DIR)/doc
	cp -f html/huawei/en_US/athwlan/*.html $(INSTALL_DIR)/doc/en_US
	cp -f html/huawei/es_ES/athwlan/*.html $(INSTALL_DIR)/doc/es_ES
endif
	#cp -f $(WEB_SOURCE_HSPA_DIR)/*.html $(WEB_SOURCE_HSPA_DIR)/*.js $(WEB_SOURCE_HSPA_DIR)/*.gif $(WEB_SOURCE_HSPA_DIR)/*.css $(WEB_SOURCE_HSPA_DIR)/*.ico $(WEB_SOURCE_HSPA_DIR)/*.url $(INSTALL_DIR)/doc
	#cp -f $(WEB_SOURCE_HSPA_DIR)/*.html $(WEB_SOURCE_HSPA_DIR)/*.url $(WEB_SOURCE_HSPA_DIR)/hspa_connecting.gif $(WEB_SOURCE_HSPA_DIR)/redrect.gif $(WEB_SOURCE_HSPA_DIR)/greenrect.gif $(INSTALL_DIR)/webs

ifeq ($(strip $(BUILD_WEBCOMPRESSED)),y)
	cp -f html/compressed/*.js $(INSTALL_DIR)/webs
#	cp -f html/compressed/*.html $(INSTALL_DIR)/webs
endif

ifneq ($(strip $(BUILD_SNTP_AUTOCONF)),)
	cp -f html/sntpautoconf/*.html $(INSTALL_DIR)/webs
endif
ifneq ($(strip $(BUILD_SYSLOG_AUTOCONF)),)
	cp -f html/logautoconf/*.html $(INSTALL_DIR)/webs
endif
ifneq ($(strip $(BUILD_ONE_USER)),)
	cp -f html/oneuser/*.html $(INSTALL_DIR)/webs
endif
ifeq ($(strip $(HRD_HG550II)),y)

ifeq ($(strip $(BRCM_APP_PHONE)),sip)
#	cp -f html/sip/*.html $(INSTALL_DIR)/webs
else
endif

ifeq ($(strip $(BUILD_CUSTOMER_ITALY)),y)
	rm -f $(INSTALL_DIR)/webs/wlsecurity_telmex.html	
	# 删除VDSL PTM模式下使用的页面:ptm_protocol(pp) ptm_vlan(pv) bridge mode(br)
	rm -f $(INSTALL_DIR)/webs/ntwkprtcl_pp.html
	rm -f $(INSTALL_DIR)/webs/ntwkprtcl_pv.html
	rm -f $(INSTALL_DIR)/webs/ntwkprtcl_br.html
endif

	rm -f $(INSTALL_DIR)/webs/autoscan.html
	rm -f $(INSTALL_DIR)/webs/autoscancancel.html
	rm -f $(INSTALL_DIR)/webs/autoscanerr.html
	rm -f $(INSTALL_DIR)/webs/certadd.html
	rm -f $(INSTALL_DIR)/webs/certcaimport.html
	rm -f $(INSTALL_DIR)/webs/certimport.html
	rm -f $(INSTALL_DIR)/webs/certloadsigned.html
	rm -f $(INSTALL_DIR)/webs/diagbr_ptm.html
	rm -f $(INSTALL_DIR)/webs/diaglan_ptm.html
	rm -f $(INSTALL_DIR)/webs/diagmer_ptm.html
	rm -f $(INSTALL_DIR)/webs/diagpppoe_ptm.html
	rm -f $(INSTALL_DIR)/webs/lancfg2_hg520v.html
	rm -f $(INSTALL_DIR)/webs/lancfg2_hg520v_chinatelecom.html
	rm -f $(INSTALL_DIR)/webs/lancfg2_hg520v_tde.html
	rm -f $(INSTALL_DIR)/webs/pppautherr.html
	rm -f $(INSTALL_DIR)/webs/pppauthinfo.html
	rm -f $(INSTALL_DIR)/webs/pvccfg.html
	rm -f $(INSTALL_DIR)/webs/pvccfg_old.html
	rm -f $(INSTALL_DIR)/webs/pvccfg_tde_atm.html
	rm -f $(INSTALL_DIR)/webs/pvccfg_tde_ptm.html
	rm -f $(INSTALL_DIR)/webs/qoscls_kpn.html
	rm -f $(INSTALL_DIR)/webs/qoscls_tde.html
	rm -f $(INSTALL_DIR)/webs/snmpconfig.html
	rm -f $(INSTALL_DIR)/webs/statsvdsl.html
	rm -f $(INSTALL_DIR)/webs/statsvdslreset.html
	rm -f $(INSTALL_DIR)/webs/tmodecfg.html
endif

ifneq ($(strip $(BUILD_SINGAPORE_LOGIN)),y)
	rm -f $(INSTALL_DIR)/webs/userlogin.html
	rm -f $(INSTALL_DIR)/webs/pppoeloginpage.html
	rm -f $(INSTALL_DIR)/webs/pppoestoppage.html
	rm -f $(INSTALL_DIR)/webs/pppoereboot.html
	rm -f $(INSTALL_DIR)/webs/lancfg2_singtel.html
else
	cp -f html/lancfg2_singtel.html $(INSTALL_DIR)/webs/lancfg2.html
	cp -f html/countrysupport/singtel/*.html $(INSTALL_DIR)/webs
endif
ifneq ($(strip $(BUILD_CUSTOMER_HUVDF)),y)
	rm -f $(INSTALL_DIR)/webs/pppoemodify.html
endif
ifeq ($(strip $(BUILD_CUSTOMER_TELMEX)),y)
	cp html/wlsecurity_telmex.html $(INSTALL_DIR)/webs/wlsecurity.html -f
endif

	rm -f $(INSTALL_DIR)/webs/rtdefaultcfgerr.html


ifneq ($(strip $(BUILD_ADSL)),)
	rm -f $(INSTALL_DIR)/webs/statsvdsl.html
	rm -f $(INSTALL_DIR)/webs/statsvdslreset.html
endif
ifneq ($(strip $(BUILD_VDSL)),)
	rm -f $(INSTALL_DIR)/webs/statsadsl.html
	rm -f $(INSTALL_DIR)/webs/statsadslerr.html
	rm -f $(INSTALL_DIR)/webs/statsadslreset.html
endif
	
rmwl_html:
ifneq ($(strip $(WIRELESS)),1)
	rm -f $(INSTALL_DIR)/webs/wl*.html
else
rmwl_html:
endif

rmvoice_html:
ifneq ($(strip $(VOXXXLOAD)),1)
	rm -f $(INSTALL_DIR)/webs/voice*.html
else
rmvoice_html:
endif

rmipsec_html:
ifneq ($(strip $(BUILD_IPSEC_TOOLS)),1)
#	rm -f $(INSTALL_DIR)/webs/ips*.html
else
rmipsec_html:
endif

rmcert_html:
ifneq ($(strip $(BUILD_CERT)),1)
#	rm -f $(INSTALL_DIR)/webs/cert*.html
else
rmcert_html:
endif

rm_redundant:
	rm -f $(INSTALL_DIR)/doc/en_US/hlpadslsync.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlpethconn.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppppoasess.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppppoeip.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlpwlconn.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlpatmetoe.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppngdns.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppppoeauth.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlptstdns.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlpatmseg.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppnggw.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlppppoeconn.html
	rm -f $(INSTALL_DIR)/doc/en_US/hlpusbconn.html
	rm -f $(INSTALL_DIR)/doc/en_US/banner-mid.gif
	rm -f $(INSTALL_DIR)/doc/en_US/huawei-logo.gif
	
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpadslsync.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpethconn.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppppoasess.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppppoeip.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpwlconn.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpatmetoe.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppngdns.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppppoeauth.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlptstdns.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpatmseg.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppnggw.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlppppoeconn.html
	rm -f $(INSTALL_DIR)/doc/es_ES/hlpusbconn.html
	rm -f $(INSTALL_DIR)/doc/es_ES/banner-mid.gif
	rm -f $(INSTALL_DIR)/doc/es_ES/huawei-logo.gif
	
	rm -f $(INSTALL_DIR)/webs/img/map/dma_on.png
	rm -f $(INSTALL_DIR)/webs/img/map/dma_off.png
	rm -f $(INSTALL_DIR)/webs/img/map/stb_off_1.png
	rm -f $(INSTALL_DIR)/webs/img/map/stb_on.png
	rm -f $(INSTALL_DIR)/webs/img/map/webcam_on.png
	rm -f $(INSTALL_DIR)/webs/img/map/webcam_off.png
	rm -f $(INSTALL_DIR)/webs/img/pictures/mode_expert.png
	find $(INSTALL_DIR)/webs/ -name "Thumbs.db" | xargs rm -f
	find $(INSTALL_DIR)/doc/ -name "Thumbs.db" | xargs rm -f

ifeq ($(strip $(BUILD_SSHD_MIPS_GENKEY)),y)
mipsgenkey:
	cd $(MIPS_GENKEY_DIR) && $(MAKE) clean && $(MAKE)
	install -m 755 $(MIPS_GENKEY) $(INSTALL_DIR)/bin
else
mipsgenkey:
endif

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))

$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) -C $(patsubst _dir_%, %, $@)

.PHONY: clean

export SSHD_DIR MIPS_GENKEY

clean:
	@find . \( ! -name "libhistory.a" ! -name "libvtop_linuxr.a" ! -name "libreadline.a" ! -name "libtermcap.a"  ! -name "tr69clib.a" ! -name "tr64lib.a" -name '*.[oasd]' -o -name core -o -name '.*.flags' -o -name '*.inc' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
	@rm -f $(EXECS)
	@rm -f wpsled
ifneq ($(strip $(SSHD_DIR)),)
	@find $(SSHD_DIR) -name "*.o" | xargs rm -f
	@find $(SSHD_DIR) -name ".*.flags" | xargs rm -f
	@find $(SSHD_DIR) -name ".depend" | xargs rm -f
	@find $(SSHD_DIR) -name "*.a" | xargs rm -f
endif
ifneq ($(strip $(DIR_BFTPD)),)
	@find $(DIR_BFTPD) -name "*.o" | xargs rm -f
	@find $(DIR_BFTPD) -name ".*.flags" | xargs rm -f
	@find $(DIR_BFTPD) -name ".depend" | xargs rm -f
	@find $(DIR_BFTPD) -name "*.a" | xargs rm -f
endif
ifneq ($(strip $(DIR_TR69C)),)
	@find $(DIR_TR69C) -name "*.o" | xargs rm -f
	@find $(DIR_TR69C) -name ".*.flags" | xargs rm -f
	@find $(DIR_TR69C) -name ".depend" | xargs rm -f
endif

ifneq ($(strip $(DIR_TR64)),)
	@find $(DIR_TR64) -name "*.o" | xargs rm -f
	@find $(DIR_TR64) -name ".*.flags" | xargs rm -f
	@find $(DIR_TR64) -name ".depend" | xargs rm -f
endif
