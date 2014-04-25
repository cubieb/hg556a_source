#****************************************************************************
#
#  Copyright (c) 2001, 2002, 2003, 2004  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16251 Laguna Canyon Road
#          Irvine, California 92618
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#****************************************************************************

# Top-level Makefile for all commengine xDSL platforms

BRCM_RELEASETAG=$(BRCM_VERSION)$(BRCM_RELEASE)$(BRCM_EXTRAVERSION)
BRCM_RELEASE_VERSION=$(BRCM_EXTRAVERSION)

###########################################
#
# Define Basic Variables
#
###########################################
BUILD_DIR = $(shell pwd)
KERNEL_VER = 2.6
ifeq ($(strip $(KERNEL_VER)),2.6)
INC_KERNEL_BASE = $(BUILD_DIR)/kernel
ORIGINAL_KERNEL = linuxmips.tar.bz2
endif
KERNEL_DIR = $(INC_KERNEL_BASE)/linux
BRCMDRIVERS_DIR = $(BUILD_DIR)/bcmdrivers
USERAPPS_DIR = $(BUILD_DIR)/userapps
LINUXDIR = $(INC_KERNEL_BASE)/linux
HOSTTOOLS_DIR = $(BUILD_DIR)/hostTools
IMAGES_DIR = $(BUILD_DIR)/images
TARGETS_DIR = $(BUILD_DIR)/targets
DEFAULTCFG_DIR = $(TARGETS_DIR)/defaultcfg
XCHANGE_DIR = $(BUILD_DIR)/xChange
FSSRC_DIR = $(TARGETS_DIR)/fs.src


SHARED_DIR = $(BUILD_DIR)/shared
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)
GENDEFCONFIG_CMD = $(HOSTTOOLS_DIR)/scripts/gendefconfig
RUN_NOISE=0
6510_REF_CODE=$(BUILD_DIR)/userapps/broadcom/6510refCode

###########################################
#
# atheros wireless version 
#
###########################################
ifeq ("$(RC3)", "y")
ATHR_WIRELESS_DIR = $(BUILD_DIR)/athdrivers/atheros/wireless/atheros_11n.71
else
ATHR_WIRELESS_DIR = $(BUILD_DIR)/athdrivers/atheros/wireless/atheros_11n.110
endif

export ATHR_WIRELESS_DIR

###########################################

###########################################
#
# Import Build Profiles
#
###########################################
BRCM_BOARD=bcm963xx
LAST_PROFILE=$(shell find targets -name vmlinux | sed -e "s?targets/??" -e "s?/.*??" -e "q")
ifeq ($(strip $(PROFILE)),)
PROFILE=$(LAST_PROFILE)
export PROFILE
endif

ifeq ($(strip $(OPT)),)
    OPT=MA11
endif

############################################
#
# Choose profile for different hardware
#
############################################
ifeq ($(strip $(PROFILE)),96358MW)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    # 0~9 for HG520 hardware
    ifeq ($(strip $(HARDWARE)),0)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).broadcom
        include version.make
    endif
    ifeq ($(strip $(HARDWARE)),1)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).hwfast
        include version.make
    endif
    ifeq ($(strip $(HARDWARE)),2)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.huawei
        include version.make
    endif
    ifeq ($(strip $(HARDWARE)),3)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.TDE.huawei
        include version.make
    endif
    ifeq ($(strip $(HARDWARE)),4)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.KPN.huawei
        include version.make
    endif    
    
    
    #A~H for HG510v hardware
    ifeq ($(strip $(HARDWARE)),A)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.huawei
        include version_hg510v.make
    endif
    ifeq ($(strip $(HARDWARE)),B)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.TDE.huawei
        include version_hg510v.make
    endif
    ifeq ($(strip $(HARDWARE)),C)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.KPN.huawei
        include version_hg510v.make
    endif    
    
    #I~P for HG510vi hardware    
    ifeq ($(strip $(HARDWARE)),I)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.huawei
        include version_hg510vi.make
    endif
    ifeq ($(strip $(HARDWARE)),J)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.TDE.huawei
        include version_hg510vi.make
    endif
    ifeq ($(strip $(HARDWARE)),K)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.KPN.huawei
        include version_hg510vi.make
    endif
endif

ifeq ($(strip $(PROFILE)),96348MW-CPE)
	HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
	ifeq ($(strip $(HARDWARE)),0)
    	include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).ADSL.huawei
    	include version_hg520.make
    else
		include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE)
		include version.make
	endif
endif

ifeq ($(strip $(PROFILE)),96358GWV)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    ifeq ($(strip $(HARDWARE)),0)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550i.SIP
        include version_hg550i.make
    endif
    ifeq ($(strip $(HARDWARE)),1)
        include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
        include version_hg550.make
    endif

    ifeq ($(strip $(HARDWARE)),2)
        #include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).VDSL2.SIP.POST
        #include version.make
    endif
   
    ifeq ($(strip $(HARDWARE)),A)
        #include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).ADSL.SIP.ISDN
        #include version.make
    endif    
    
    ifeq ($(strip $(HARDWARE)),B)
        #include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).VDSL2.SIP.ISDN
        #include version.make
    endif
endif

ifeq ($(strip $(PROFILE)),96358GWV_Singtel)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    include version_hg550_singtel.make
endif

ifeq ($(strip $(PROFILE)),96358GWV_Telmex)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
    include version_hg553_telmex.make
endif

ifeq ($(strip $(PROFILE)),96358GWV_HuVDF)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550i.SIP
    include version_hg550i_HuVDF.make
endif

ifeq ($(strip $(PROFILE)),96358GWV_Italy)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    ifeq ($(strip $(HARDWARE)),1)
    	include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    	include version_hg550_italy.make
    endif
    ifeq ($(strip $(HARDWARE)),2)
    	include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
    	include version_hg553_italy.make
    endif
    
endif

ifeq ($(strip $(PROFILE)),96358GWV_Spain)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    ifeq ($(strip $(HARDWARE)),1)
    	include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    	include version_hg550_spain.make
    endif
    ifeq ($(strip $(HARDWARE)),2)
    	include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
    	include version_hg553_spain.make
    endif
    
endif

ifeq ($(strip $(PROFILE)),96358GWV_Global)
    HARDWARE := $(shell echo $(OPT) | awk '{print substr("$(OPT)",3,1)}')
    include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG556.SIP
    	include version_hg556_global.make
endif



ifndef HARDWARE
ifneq ($(strip $(PROFILE)),)
    include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE)
    include version.make
else
exit1:
	@echo "Please input correct PROFILE"
	@exit 1;
endif
endif

###########################################
#
# Default configure file
#
###########################################
TELECOM := $(shell echo $(OPT) | awk '{print substr("$(OPT)",4,1)}')
ifeq ($(strip $(PROFILE)),96358MW)
    # 0 for stardard version, 1 for tde version; 2 for kpn version
    ifeq ($(strip $(TELECOM)),0)
	BRCM_DEFAULTCFG="default_96358MW_$(BRCM_RELEASE).cfg"
    endif            
    ifeq ($(strip $(TELECOM)),1)
	BRCM_DEFAULTCFG="default_96358MW_$(BRCM_RELEASE)_tde.cfg"
    endif    
    ifeq ($(strip $(TELECOM)),2)
	BRCM_DEFAULTCFG="default_96358MW_$(BRCM_RELEASE)_kpn.cfg"
    endif
endif

ifeq ($(strip $(PROFILE)),96348GW)
    BRCM_DEFAULTCFG="default_96348GW.cfg"
endif

ifeq ($(strip $(PROFILE)),96348MW-CPE)
    BRCM_DEFAULTCFG="default_96348MW-CPE_ADSL.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV)
    ifeq ($(strip $(TELECOM)),0)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE).cfg"
    endif 
endif

ifeq ($(strip $(PROFILE)),96358GWV_Singtel)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_singtel.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV_HuVDF)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_HuVDF.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV_Italy)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_Italy.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV_Spain)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_Spain.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV_Global)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_Global.cfg"
endif

ifeq ($(strip $(PROFILE)),96358GWV_Telmex)
    BRCM_DEFAULTCFG="default_96358GWV_$(BRCM_RELEASE)_telmex.cfg"
endif

#ifeq ($(strip $(BRCM_DEFAULTCFG)),"")
#    BRCM_DEFAULTCFG=default.cfg
#endif


ifneq ($(strip $(PROFILE)),)
#include $(TARGETS_DIR)/$(PROFILE)/$(PROFILE)
export BRCM_CHIP
export BRCM_FLASHPSI_SIZE
export BRCM_DRIVER_WIRELESS_PCMCIA_DATASWAP BRCM_DRIVER_WIRELESS_EBI_DMA
export BRCM_DRIVER_USB BRCM_DRIVER_ETHERNET_CONFIG
export BRCM_DEFAULTCFG
export BRCM_KERNEL_NF_FIREWALL BRCM_KERNEL_NF_MANGLE BRCM_KERNEL_NF_NAT
endif

###########################################
#
# Define Toolchain
#
###########################################
ifeq ($(strip $(BRCM_UCLIBC)),y)
NTC=1
ifeq ($(strip $(NTC)),1)
TOOLCHAIN=/opt/toolchains/uclibc-crosstools_gcc-3.4.2_uclibc-20050502
CROSS_COMPILE = $(TOOLCHAIN)/bin/mips-linux-uclibc-
else
TOOLCHAIN=/opt/toolchains/uclibc
CROSS_COMPILE = $(TOOLCHAIN)/bin/mips-uclibc-
endif
else
 TOOLCHAIN=/usr/crossdev/mips
CROSS_COMPILE = $(TOOLCHAIN)/bin/mips-linux-
endif

AR              = $(CROSS_COMPILE)ar
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CXX             = $(CROSS_COMPILE)g++
CPP             = $(CROSS_COMPILE)cpp
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
SSTRIP          = $(CROSS_COMPILE)sstrip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
RANLIB          = $(CROSS_COMPILE)ranlib

LIB_PATH        = $(TOOLCHAIN)/mips-linux/lib
LIBDIR          = $(TOOLCHAIN)/mips-linux/lib
LIBCDIR         = $(TOOLCHAIN)/mips-linux

###########################################
#
# Application-specific settings
#
###########################################
INSTALL_DIR = $(TARGETS_DIR)/fs.src
TARGET_FS = $(TARGETS_DIR)/$(PROFILE)/fs
PROFILE_DIR = $(TARGETS_DIR)/$(PROFILE)
###########################################
# Set profile path for different hardware
###########################################
#PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE)
ifeq ($(strip $(PROFILE)),96358MW)
    # 0~9 for HG520 hardware
    ifeq ($(strip $(HARDWARE)),0)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).broadcom
    endif
    ifeq ($(strip $(HARDWARE)),1)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).hwfast
    endif
    ifeq ($(strip $(HARDWARE)),2)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.huawei
    endif    
    ifeq ($(strip $(HARDWARE)),3)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.TDE.huawei
    endif
    ifeq ($(strip $(HARDWARE)),4)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG520v.KPN.huawei
    endif    
     
    #A~H for HG510v hardware.
    ifeq ($(strip $(HARDWARE)),A)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.huawei
    endif
    ifeq ($(strip $(HARDWARE)),B)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.TDE.huawei
    endif
    ifeq ($(strip $(HARDWARE)),C)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510v.KPN.huawei
    endif         
    
    #I~P for HG510vi hardware
    ifeq ($(strip $(HARDWARE)),I)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.huawei
    endif
    ifeq ($(strip $(HARDWARE)),J)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.TDE.huawei
    endif
    ifeq ($(strip $(HARDWARE)),K)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG510vi.KPN.huawei
    endif         
endif           

ifeq ($(strip $(PROFILE)),96348MW-CPE)
	ifeq ($(strip $(HARDWARE)),0)
    	PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).ADSL.huawei   	
	endif
endif

ifeq ($(strip $(PROFILE)),96358GWV)
    ifeq ($(strip $(HARDWARE)),0)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550i.SIP
    endif

    ifeq ($(strip $(HARDWARE)),1)
        PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    endif
   
    ifeq ($(strip $(HARDWARE)),A)
        #PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).ADSL.SIP.ISDN
    endif    
    
    ifeq ($(strip $(HARDWARE)),B)
        #PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).VDSL2.SIP.ISDN
    endif
endif

ifeq ($(strip $(PROFILE)),96358GWV_Singtel)
    PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
endif

ifeq ($(strip $(PROFILE)),96358GWV_HuVDF)
    PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550i.SIP
endif

ifeq ($(strip $(PROFILE)),96358GWV_Italy)
    ifeq ($(strip $(HARDWARE)),1)
    	PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    endif
    ifeq ($(strip $(HARDWARE)),2)
    	PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
    endif
    
endif

ifeq ($(strip $(PROFILE)),96358GWV_Spain)
    ifeq ($(strip $(HARDWARE)),1)
    	PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG550.SIP
    endif
    ifeq ($(strip $(HARDWARE)),2)
    	PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
    endif
    
endif

ifeq ($(strip $(PROFILE)),96358GWV_Global)
    PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG556.SIP
endif


ifeq ($(strip $(PROFILE)),96358GWV_Telmex)
    PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE).HG553.SIP
endif

ifndef PROFILE_PATH
    PROFILE_PATH = $(TARGETS_DIR)/$(PROFILE)/$(PROFILE)
endif

ifeq ($(strip $(BRCM_BOARD_ID)), "HW556")
CFE_FILE = $(TARGETS_DIR)/cfe/cfe$(BRCM_CHIP).bin
endif

ifeq ($(strip $(BRCM_BOARD_ID)), "HW553")
CFE_FILE = $(TARGETS_DIR)/cfe/cfe$(BRCM_CHIP)_IS.bin
endif



VENDOR_NAME = bcm
FS_KERNEL_IMAGE_NAME = $(HGW_PRODUCTTYPE)$(BRCM_EXTRAVERSION)_fs_kernel
CFE_FS_KERNEL_IMAGE_NAME = $(HGW_PRODUCTTYPE)$(BRCM_EXTRAVERSION)_cfe_fs_kernel
FLASH_IMAGE_NAME = $(HGW_PRODUCTTYPE)$(BRCM_EXTRAVERSION)_flash_image
INC_BRCMDRIVER_PUB_PATH=$(BRCMDRIVERS_DIR)/opensource/include
INC_BRCMDRIVER_PRIV_PATH=$(BRCMDRIVERS_DIR)/broadcom/include
INC_ENDPOINT_PATH=$(BRCMDRIVERS_DIR)/broadcom/char/endpoint/bcm9$(BRCM_CHIP)/inc
INC_ADSLDRV_PATH=$(BRCMDRIVERS_DIR)/broadcom/char/adsl/impl1
BROADCOM_TR69C_DIR=$(BROADCOM_DIR)/tr69c
BROADCOM_CFM_DIR=$(BROADCOM_DIR)/cfm
INC_BRCMCFM_PATH=$(BROADCOM_CFM_DIR)/inc
INC_BRCMSHARED_PUB_PATH=$(SHARED_DIR)/opensource/include
INC_BRCMSHARED_PRIV_PATH=$(SHARED_DIR)/broadcom/include
INC_BRCMBOARDPARMS_PATH=$(SHARED_DIR)/opensource/boardparms
INC_FLASH_PATH=$(SHARED_DIR)/opensource/flash
WLAN_ART_DIR = $(TARGETS_DIR)/wlan.art
HGW_PRODUCTTYPE=$(BRCM_RELEASE)
HGW_PRODUCTNAME=$(BRCM_VERSION)$(BRCM_RELEASE)
HGW_SOFTVERSION=$(BRCM_EXTRAVERSION)

ifeq ($(strip $(BRCM_APP_PHONE)),sip)
export VOXXXLOAD=1
export VOIPLOAD=1
export SIPLOAD=1
export BUILD_WT104=y

ifeq ($(strip $(BRCM_VODSL_CONFIG_MANAGER)),y)
	export BRCM_VODSL_CFGMGR=1
endif

BRCM_RELEASETAG := $(BRCM_RELEASETAG).sip
endif

ifeq ($(strip $(BRCM_APP_PHONE)),mgcp)
export VOXXXLOAD=1
export VOIPLOAD=1
export MGCPLOAD=1
export BRCM_VODSL_CFGMGR=0
BRCM_RELEASETAG := $(BRCM_RELEASETAG).mgcp
endif

ifeq ($(strip $(BRCM_PROFILER_ENABLED)),y)
export BRCM_PROFILER_TOOL=1
else
export BRCM_PROFILER_TOOL=0
endif

ifneq ($(strip $(BUILD_VODSL)),)
export VOXXXLOAD=1
endif

ifeq ($(strip $(BRCM_VODSL_STUNC)),y)
	export BRCM_VODSL_STUN_CLIENT=1
endif

ifeq ($(strip $(BRCM_VODSL_RANDOMP)),y)
	export BRCM_VODSL_RANDOM_PORT=1
endif

BRCM_DSP_HAL := gw
BRCM_DSP_HAL_EXTENSION :=
XCHANGE_DSP_APP_EXTENSION :=
export XCHANGE_DSP_APP=$(BRCM_DSP_CODEC)

ifeq ($(strip $(BRCM_DSP_PCM)),y)
XCHANGE_DSP_APP := dspApp3341_tdm
BRCM_DSP_HAL_EXTENSION := _pcm
endif

ifeq ($(strip $(BRCM_DSP_PCM_G726)),y)
XCHANGE_DSP_APP := dspApp3341_tdm_g726
BRCM_DSP_HAL_EXTENSION := _pcm
endif

ifeq ($(strip $(BRCM_DSP_PCM_T38_EXT)),y)
XCHANGE_DSP_APP := dspApp3341_tdm_t38
BRCM_DSP_HAL_EXTENSION := _pcm
endif

ifeq ($(strip $(BRCM_DSP_APM_FXO_EXT)),y)
XCHANGE_DSP_APP := dspApp3341
BRCM_DSP_HAL_EXTENSION := _hybrid
XCHANGE_DSP_APP_EXTENSION := _fxo_ext
endif

ifeq ($(strip $(BUILD_VDSL)),y)
export BUILD_VDSL=y
VBOOT_BIN="boot.bin"
ifneq ($(strip $(CPE_ANNEX_B)),)
  CPE_MODEM="cpe_annex_b.bin"
else
  CPE_MODEM="cpe_annex_a.bin"
endif
ifeq ($(strip $(VCOPE_TYPE)),CO)
  VMODEM_BIN="co_modem.bin"
endif
ifeq ($(strip $(VCOPE_TYPE)),CPE)
  VMODEM_BIN=$(CPE_MODEM)
endif

endif

ifeq ($(strip $(BUILD_KPN_QOS)),y)
export BUILD_KPN_QOS=y
endif

ifeq ($(strip $(BUILD_TDE_QOS)),y)
export BUILD_TDE_QOS=y
endif

ifeq ($(strip $(BUILD_VDF_QOS)),y)
export BUILD_VDF_QOS=y
endif

ifeq ($(strip $(BUILD_SUPPORT_POLICY)),y)
export BUILD_SUPPORT_POLICY=y
endif

ifeq ($(strip $(BUILD_SUPPORT_GLB_PRO)),y)
export BUILD_SUPPORT_GLB_PRO=y
endif

#ifeq ($(strip $(BUILD_SCOUT)),y)
#export BUILD_SUPPORT_SCOUT=y
#endif

ifeq ($(strip $(BUILD_VDF_SEC)),y)
export BUILD_VDF_SEC=y
endif

ifeq ($(strip $(BUILD_SUPPORT_VDF_DHCP)),y)
export BUILD_SUPPORT_VDF_DHCP=y
endif

ifeq ($(strip $(BUILD_SUPPORT_CD_WEB)),y)
export BUILD_SUPPORT_CD_WEB=y
endif

ifeq ($(strip $(BUILD_SUPPORT_VDF_DIAGNOSE)),y)
export BUILD_SUPPORT_VDF_DIAGNOSE=y
endif

ifeq ($(strip $(BUILD_SUPPORT_VDF_DMZ)),y)
export BUILD_SUPPORT_VDF_DMZ=y
endif

ifeq ($(strip $(BUILD_SUPPORT_SNTP_AUTOCONF)),y)
export BUILD_SUPPORT_SNTP_AUTOCONF=y
endif
#
#  Warning here, we do re-assign some of the variables defined earlier:
#  BRCM_DSP_HAL and BRCM_DSP_HAL_EXTENSION for example, in order to pickup
#  the correct board HAL application.
#
ifeq ($(strip $(BRCM_DSP_FXO)),y)
ifeq ($(strip $(BRCM_SLIC_LE9502)),y)
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)_Le9502FXO
BRCM_RELEASETAG := $(BRCM_RELEASETAG)._LE9502
BRCM_DSP_HAL := _Le9502FXO
BRCM_DSP_HAL_EXTENSION :=
export BRCM_SLIC_LE9502
else
ifeq ($(strip $(BRCM_SLIC_LE9500)),y)
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)_Le9500FXO
BRCM_RELEASETAG := $(BRCM_RELEASETAG)._LE9500
BRCM_DSP_HAL := _Le9500FXO
BRCM_DSP_HAL_EXTENSION :=
export BRCM_SLIC_LE9500
endif
endif
endif


#
# DSP codec flags definition.  To be used throughout the application (for configuration and vodsl)
#

BRCM_DSP_CODEC_DEFINES := -DXCFG_G711_SUPPORT=1

ifeq ($(strip $(BRCM_DSP_CODEC_G723)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G7231_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G726)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G729)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G7xx)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G7231_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_PCM)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_PCM_G726)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_PCM_T38_EXT)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_CODEC_T38_EXT)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_APM_FXO_EXT)),y)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
endif

ifeq ($(strip $(BRCM_DSP_CODEC)),all)
BRCM_DSP_CODEC_DEFINES += -DXCFG_G7231_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_BV16_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_ILBC_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
endif


#
#  Definition of the number of voice channels supported based on the specific
#  application being created.
#

ifeq ($(strip $(BRCM_DSP_APM_FXO_EXT)), y)
BRCM_DSP_CHAN_DEFINES = -DNUM_APM_VOICE_CHANNELS=2 -DNUM_TDM_VOICE_CHANNELS=0 -DNUM_FXO_CHANNELS=1
else
BRCM_DSP_CHAN_DEFINES = -DNUM_APM_VOICE_CHANNELS=0 -DNUM_TDM_VOICE_CHANNELS=0 -DNUM_FXO_CHANNELS=0
endif

export BRCM_DSP_CODEC_DEFINES
export BRCM_DSP_CHAN_DEFINES
export BRCM_DSP_FXO
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)$(BRCM_DSP_HAL)$(BRCM_DSP_HAL_EXTENSION)
export XCHANGE_DSP_APP_EXTENSION
export BRCM_DSP_HAL_EXTENSION

# If no codec is selected, build G.711 load.
# Any XCHANGE_BUILD_APP directory would be OK
# because G.711 is included with all the voice DSP images.
ifneq ($(strip $(BUILD_VODSL)),)
ifeq ($(strip $(XCHANGE_DSP_APP)),)
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)gw
export XCHANGE_DSP_APP=g711
endif
endif
BRCM_RELEASETAG := $(BRCM_RELEASETAG).$(XCHANGE_DSP_APP)

ifeq ($(strip $(BRCM_CHIP)),6358)
ifeq ($(strip $(BRCM_6358_G729_FXO)),y)
BRCM_DSP_HAL := vw_fxo
BRCM_DSP_CHAN_DEFINES = -DNUM_FXO_CHANNELS=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_GSMAMR_SUPPORT=1
export BRCM_6358_G729_FXO
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_fxo
export BRCM_DSP_CODEC_DEFINES
else
ifeq ($(strip $(BRCM_6358_WIDEBAND_FXO)),y)
BRCM_DSP_HAL := vw_2fxs_wb_fxo
BRCM_DSP_CHAN_DEFINES = -DNUM_FXO_CHANNELS=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G7231_SUPPORT=0
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_BV16_SUPPORT=0
BRCM_DSP_CODEC_DEFINES += -DXCFG_BV32_SUPPORT=0
BRCM_DSP_CODEC_DEFINES += -DXCFG_LPCMWB_SUPPORT=0
BRCM_DSP_CODEC_DEFINES += -DXCFG_G722_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_ILBC_SUPPORT=0
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_AMRWB_SUPPORT=1
export BRCM_6358_WIDEBAND_FXO
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_2fxs_wb_fxo
export BRCM_DSP_CODEC_DEFINES
else
ifeq ($(strip $(BRCM_6358_G729_4FXS)),y)
BRCM_DSP_HAL := vw_4fxs
export BRCM_6358_G729_4FXS
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_4fxs
else
ifeq ($(strip $(BRCM_6358_G729_2FXS_LEGERITY)),y)
BRCM_DSP_HAL := vw_legerity
export BRCM_6358_G729_2FXS_LEGERITY
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_legerity
else
ifeq ($(strip $(BRCM_6358_G729_FXO_LEGERITY)),y)
BRCM_DSP_HAL := vw_fxo_legerity
BRCM_DSP_CHAN_DEFINES = -DNUM_FXO_CHANNELS=1
export BRCM_6358_G729_FXO_LEGERITY
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_fxo_legerity
else
ifeq ($(strip $(BRCM_6358_G729_ISDN_FXO)),y)
BRCM_DSP_HAL := vw_isdn_fxo
BRCM_DSP_CHAN_DEFINES   = -DNUM_FXO_CHANNELS=1
BRCM_DSP_CODEC_DEFINES := -DXCFG_G711_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
export BRCM_6358_G729_ISDN_FXO
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_isdn_fxo
export BRCM_DSP_CODEC_DEFINES
else
ifeq ($(strip $(BRCM_6358_WIDEBAND)),y)
BRCM_DSP_HAL := vw_wb
BRCM_DSP_CODEC_DEFINES += -DXCFG_G7231_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G726_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G729_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_BV16_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_BV32_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_LPCMWB_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_G722_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_ILBC_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_FAX_SUPPORT=1
BRCM_DSP_CODEC_DEFINES += -DXCFG_AMRWB_SUPPORT=1
export BRCM_6358_WIDEBAND
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw_wb
export BRCM_DSP_CODEC_DEFINES
else
BRCM_DSP_HAL := vw
export BRCM_6358_G729
export XCHANGE_BUILD_APP=Bcm$(BRCM_CHIP)vw
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(strip $(BRCM_VODSL_DUAL_3341)),y)
BRCM_RELEASETAG := $(BRCM_RELEASETAG).dual3341
endif

#Set up ADSL standard
export ADSL=$(BRCM_ADSL_STANDARD)

#Set up ADSL_PHY_MODE  {file | obj}
ifneq ($(strip $(BRCM_ADSL_STANDARD)),ADSL_NONE)
export ADSL_PHY_MODE=file
endif

#Set up ADSL_SELF_TEST
export ADSL_SELF_TEST=$(BRCM_ADSL_SELF_TEST)

###########################################
#
# Complete list of applications
#
###########################################
export OPENSOURCE_DIR=$(USERAPPS_DIR)/opensource
SUBDIRS_OPENSOURCE = $(OPENSOURCE_DIR)/atm2684/pvc2684ctl \
        $(OPENSOURCE_DIR)/openssl \
        $(OPENSOURCE_DIR)/ipsec-tools \
        $(OPENSOURCE_DIR)/bridge-utils \
        $(OPENSOURCE_DIR)/ppp/pppoe \
        $(OPENSOURCE_DIR)/udhcp \
        $(OPENSOURCE_DIR)/iptables \
        $(OPENSOURCE_DIR)/ebtables \
        $(OPENSOURCE_DIR)/reaim  \
        $(OPENSOURCE_DIR)/iproute2  \
        $(OPENSOURCE_DIR)/libosip2 \
        $(OPENSOURCE_DIR)/siproxd \
        $(OPENSOURCE_DIR)/zebra  \
        $(OPENSOURCE_DIR)/net-snmp  \
		$(OPENSOURCE_DIR)/wlan-diag \
		$(OPENSOURCE_DIR)/web-tool \
        $(OPENSOURCE_DIR)/ftpd \
	    $(OPENSOURCE_DIR)/usbmount \
		$(OPENSOURCE_DIR)/ttyUSB \
		$(OPENSOURCE_DIR)/ntfs-3g \
        $(OPENSOURCE_DIR)/libcreduction \
        $(OPENSOURCE_DIR)/busybox \
        $(OPENSOURCE_DIR)/dnsmasq  \
        $(OPENSOURCE_DIR)/oprofile \
        $(OPENSOURCE_DIR)/samba    \
        $(OPENSOURCE_DIR)/scout  \
        $(OPENSOURCE_DIR)/wlancmd  \
        $(OPENSOURCE_DIR)/wireless_tools \
        $(OPENSOURCE_DIR)/ate  \
        $(OPENSOURCE_DIR)/udpechoserver

        

#In future, we need to add soap when it
#is decoupled from cli

export BROADCOM_DIR=$(USERAPPS_DIR)/broadcom
SUBDIRS_BROADCOM = $(BROADCOM_DIR)/nvram \
	$(BROADCOM_DIR)/cfm \
        $(BROADCOM_DIR)/upnp/router/upnp \
        $(BROADCOM_DIR)/nas \
        $(BROADCOM_DIR)/wlctl \
        $(BROADCOM_DIR)/vodsl \
        $(BROADCOM_DIR)/atmctl \
        $(BROADCOM_DIR)/adslctl \
        $(BROADCOM_DIR)/netctl \
        $(BROADCOM_DIR)/dnsprobe \
	$(BROADCOM_DIR)/wifipoll \
        $(BROADCOM_DIR)/igmp \
        $(BROADCOM_DIR)/dhcpr \
        $(BROADCOM_DIR)/fwlog \
        $(BROADCOM_DIR)/hwlog \
	$(BROADCOM_DIR)/sntp \
	$(BROADCOM_DIR)/ippd \
	$(BROADCOM_DIR)/ddnsd \
	$(BROADCOM_DIR)/ethctl \
        $(BROADCOM_DIR)/hotplug \
        $(BROADCOM_DIR)/epittcp \
        $(BROADCOM_DIR)/snmp \
#        $(BROADCOM_DIR)/tr69c \
        $(BROADCOM_DIR)/ses \
        $(BROADCOM_DIR)/relayCtl \
        $(BROADCOM_DIR)/vdslctl \
        $(BROADCOM_DIR)/hostCode

export HUAWEI_DIR=$(USERAPPS_DIR)/huawei
SUBDIR_HUAWEI = $(HUAWEI_DIR)/voice/DMM \
        $(HUAWEI_DIR)/voice/TAPI \
        $(HUAWEI_DIR)/voice/SIP \
        $(HUAWEI_DIR)/voice/ISDN \
        $(HUAWEI_DIR)/voice/Common
 
SUBDIRS_APP = $(SUBDIRS_BROADCOM) $(SUBDIRS_OPENSOURCE) $(SUBDIR_HUAWEI)
SUBDIRS = $(foreach dir, $(SUBDIRS_APP), $(shell if [ -d "$(dir)" ]; then echo $(dir); fi))

OPENSOURCE_APPS = ipsec-tools pvc2684ctl pvc2684d brctl pppd udhcp iptables ebtables samba scout\
                  reaim tc libosip2 siproxd snmp zebra bftpd busybox dnsmasq ttyUSB usbmount wlan-diag web-tool diagd \
                  wireless_tools wlancmd ate udpechoserver ntfs-3g

BROADCOM_APPS = nvram bcmcrypto bcmshared nas wlctl cfm upnp vodsl atmctl adslctl netctl dnsprobe \
                igmp dhcpr hwlog fwlog diagapp sntp ddnsd ilmi ippd hotplug ethctl epittcp snmp ses \
                hmi2proxy relayctl vdslctl lld2d wsccmd wifipoll

                
ifeq ($(strip $(HRD_HG550II)),y)
ifeq ($(strip $(PROFILE)),96358GWV_Singtel)
HUAWEI_APPS = voice ifdns
else
HUAWEI_APPS = voice ifdns
endif
else
HUAWEI_APPS =        
endif
                
LIBC_OPTIMIZATION = libcreduction

ifneq ($(strip $(BUILD_GDBSERVER)),)
TOOLCHAIN_UTIL_APPS = gdbserver
endif

BUSYBOX_DIR = $(OPENSOURCE_DIR)/busybox

BRCMAPPS = openssl $(BROADCOM_APPS) $(HUAWEI_APPS) $(OPENSOURCE_APPS) $(TOOLCHAIN_UTIL_APPS) $(LIBC_OPTIMIZATION)
#$(HUAWEI_APPS) remove voip by zwp

all: kernel opensource
opensource: dnsmasq iptables scout web-tool wlancmd diagd ebtables libosip2 samba siproxd udpechoserver wireless_tools zebra iproute2 bridge-utils atm2684 pppd udhcp \
        ttyUSB busybox ftpd
sanity_check:
	@if [ "$(PROFILE)" = "" ]; then \
          echo You need to specify build profile name from $(TARGETS_DIR) using 'make PROFILE=<profile name>...'; exit 1; \
	fi

profile_check:
	@if [ "$(LAST_PROFILE)" != "" ] && [ "$(LAST_PROFILE)" != "$(PROFILE)" ]; then \
		echo "The specified profile, $(PROFILE), differs from the last one built, $(LAST_PROFILE)."; \
		echo "The entire image must be rebuilt."; \
		read -p "Press ENTER to rebuild the entire image or CTRL-C to abort. " ; \
		$(MAKE) PROFILE=$(LAST_PROFILE) clean; \
		$(MAKE) PROFILE=$(PROFILE); \
		echo "Ignore the make exit error, Error 1"; \
		exit 1; \
	fi

$(KERNEL_DIR)/vmlinux:
	$(GENDEFCONFIG_CMD) $(PROFILE_PATH)
	cd $(KERNEL_DIR); \
	cp -f $(KERNEL_DIR)/arch/mips/defconfig $(KERNEL_DIR)/.config; \
	$(MAKE) oldconfig; $(MAKE); $(MAKE) modules_install

kernelbuild:
ifeq ($(wildcard $(KERNEL_DIR)/vmlinux),)
	@cd $(INC_KERNEL_BASE); \
	if [ ! -e linux/CREDITS ]; then \
	  echo Untarring original Linux kernel source...; \
	  (tar xkfj $(ORIGINAL_KERNEL) 2> /dev/null || true); \
	fi
	$(GENDEFCONFIG_CMD) $(PROFILE_PATH)
	cd $(KERNEL_DIR); \
	cp -f $(KERNEL_DIR)/arch/mips/defconfig $(KERNEL_DIR)/.config; \
	$(MAKE) oldconfig; $(MAKE)
else
	cd $(KERNEL_DIR); $(MAKE)
endif

kernel: profile_check kernelbuild
	
web-tool:
	$(MAKE) -C $(OPENSOURCE_DIR)/web-tool dynamic
web-tool_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/web-tool clean

diagd:
	$(MAKE) -C $(OPENSOURCE_DIR)/diagd
diagd_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/diagd clean

ifneq ($(strip $(BUILD_PVC2684CTL)),)
export BUILD_PVC2684D=$(BUILD_PVC2684CTL)
atm2684 :
	$(MAKE) -C $(OPENSOURCE_DIR)/atm2684/pvc2684ctl $(BUILD_PVC2684CTL)
atm2684_clean :
	$(MAKE) -C $(OPENSOURCE_DIR)/atm2684/pvc2684ctl clean
else
atm2684 :
atm2684_clean :
endif

ifneq ($(strip $(BUILD_BRCTL)),)
bridge-utils :
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj bridge-utils.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/bridge-utils $(BUILD_BRCTL)
bridge-utils_clean :
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj bridge-utils.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/bridge-utils clean
else
bridge-utils:
bridge-utils_clean:
endif

ifneq ($(strip $(BUILD_VCONFIG)),)
export BUILD_VCONFIG=y
endif

# iptables is dependent on kernel netfilter modules
ifneq ($(strip $(BRCM_KERNEL_NETFILTER)),)
ifneq ($(strip $(BUILD_IPTABLES)),)
iptables:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj iptables.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/iptables $(BUILD_IPTABLES)
iptables-build:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj iptables.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/iptables static
iptables_clean:
	cd $(OPENSOURCE_DIR);   
	$(MAKE) -C $(OPENSOURCE_DIR)/iptables clean
else
iptables:
endif
else
iptables:
	@echo Warning: You need to enable netfilter in the kernel !!!!!
endif

ifneq ($(strip $(BUILD_EBTABLES)),)
ebtables:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj ebtables.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/ebtables $(BUILD_EBTABLES)
ebtables_clean:
	cd $(OPENSOURCE_DIR);   
	$(MAKE) -C $(OPENSOURCE_DIR)/ebtables clean
else
ebtables:
ebtables_clean:
endif

ifneq ($(strip $(BUILD_PPPD)),)
pppd:
	$(MAKE) -C $(OPENSOURCE_DIR)/ppp/pppoe $(BUILD_PPPD)
pppd_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/ppp/pppoe clean
else
pppd:
endif

ifneq ($(strip $(BRCM_KERNEL_NETQOS)),)
iproute2:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj iproute2.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/iproute2 dynamic
iproute2_clean:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj iproute2.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/iproute2 clean
else
iproute2:
iproute2_clean:
endif

ifneq ($(strip $(BUILD_UDHCP)),)
udhcp:
	$(MAKE) -C $(OPENSOURCE_DIR)/udhcp $(BUILD_UDHCP)
udhcp_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/udhcp clean
else
udhcp:
udhcp_clean:
endif

ifneq ($(strip $(BUILD_SIPROXD)),)
siproxd:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj siproxd.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/siproxd $(BUILD_SIPROXD)
libosip2:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj libosip2.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/libosip2
siproxd_clean:
	cd $(OPENSOURCE_DIR);   
	$(MAKE) -C $(OPENSOURCE_DIR)/siproxd clean
libosip2_clean:
	cd $(OPENSOURCE_DIR);   
	$(MAKE) -C $(OPENSOURCE_DIR)/libosip2 clean
else
siproxd:

libosip2:

siproxd_clean:

libosip2_clean:

endif

ifneq ($(strip $(BUILD_SNMP)),)

ifneq ($(strip $(BUILD_SNMP_SET)),)
export BUILD_SNMP_SET=1
else
export BUILD_SNMP_SET=0
endif

ifneq ($(strip $(BUILD_SNMP_ADSL_MIB)),)
export BUILD_SNMP_ADSL_MIB=1
else
export BUILD_SNMP_ADSL_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_ATM_MIB)),)
export BUILD_SNMP_ATM_MIB=1
else
export BUILD_SNMP_ATM_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_AT_MIB)),)
export BUILD_SNMP_AT_MIB=1
else
export BUILD_SNMP_AT_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_SYSOR_MIB)),)
export BUILD_SNMP_SYSOR_MIB=1
else
export BUILD_SNMP_SYSOR_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_TCP_MIB)),)
export BUILD_SNMP_TCP_MIB=1
else
export BUILD_SNMP_TCP_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_UDP_MIB)),)
export BUILD_SNMP_UDP_MIB=1
else
export BUILD_SNMP_UDP_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_IP_MIB)),)
export BUILD_SNMP_IP_MIB=1
else
export BUILD_SNMP_IP_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_ICMP_MIB)),)
export BUILD_SNMP_ICMP_MIB=1
else
export BUILD_SNMP_ICMP_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_SNMP_MIB)),)
export BUILD_SNMP_SNMP_MIB=1
else
export BUILD_SNMP_SNMP_MIB=0
endif

ifneq ($(strip $(BUILD_SNMP_ATMFORUM_MIB)),)
export BUILD_SNMP_ATMFORUM_MIB=1
else
export BUILD_SNMP_ATMFORUM_MIB=0
endif

ifneq ($(strip $(BRCM_SNMP)),)

ifneq ($(strip $(BUILD_SNMP_CHINA_TELECOM_CPE_MIB)),)
export BUILD_SNMP_CHINA_TELECOM_CPE_MIB=y
export BUILD_SNMP_MIB2=y
endif

ifneq ($(strip $(BUILD_SNMP_UDP)),)
export BUILD_SNMP_UDP=y
endif

ifneq ($(strip $(BUILD_SNMP_EOC)),)
export BUILD_SNMP_EOC=y
endif

ifneq ($(strip $(BUILD_SNMP_AAL5)),)
export BUILD_SNMP_AAL5=y
endif

ifneq ($(strip $(BUILD_SNMP_AUTO)),)
export BUILD_SNMP_AUTO=y
endif

ifneq ($(strip $(BUILD_SNMP_DEBUG)),)
export BUILD_SNMP_DEBUG=y
endif

ifneq ($(strip $(BUILD_SNMP_TRANSPORT_DEBUG)),)
export BUILD_SNMP_TRANSPORT_DEBUG=y
endif

ifneq ($(strip $(BUILD_SNMP_LAYER_DEBUG)),)
export BUILD_SNMP_LAYER_DEBUG=y
endif
endif

snmp:
ifneq ($(strip $(BRCM_SNMP)),)
##	$(MAKE) -C $(BROADCOM_DIR)/snmp $(BUILD_SNMP)
else
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj net-snmp.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/net-snmp $(BUILD_SNMP)
endif
else
snmp:
endif

ifneq ($(strip &(BUILD_DNSMASQ)),)
dnsmasq:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/dnsmasq install 
dnsmasq_clean:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/dnsmasq clean
else
dnsmasq:
dnsmasq_clean:
endif

ftpd:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/ftpd install 
ftpd_clean:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/ftpd clean

ifneq ($(strip $(BUILD_4_LEVEL_QOS)),)
export BUILD_4_LEVEL_QOS=y
endif

ifneq ($(strip $(BUILD_ZEBRA)),)
zebra:
	cd $(OPENSOURCE_DIR);   
#	(tar xkfj zebra.tar.bz2 2> /dev/null || true)
	$(MAKE) -C $(OPENSOURCE_DIR)/zebra $(BUILD_ZEBRA)
zebra_clean:
	cd $(OPENSOURCE_DIR);   
	$(MAKE) -C $(OPENSOURCE_DIR)/zebra clean
else
zebra:
zebra_clean:
endif

ifneq ($(strip $(BUILD_BUSYBOX)),)
busybox:
	cd $(OPENSOURCE_DIR); 
#	(tar xkfj busybox.tar.bz2 2> /dev/null || true)
#	$(MAKE) -C $(OPENSOURCE_DIR)/busybox $(BUILD_BUSYBOX)
	cd $(OPENSOURCE_DIR)/busybox; cp -f brcm.config .config
	$(MAKE) -C $(OPENSOURCE_DIR)/busybox install
busybox_clean:
	cd $(OPENSOURCE_DIR); 
	$(MAKE) -C $(OPENSOURCE_DIR)/busybox clean
else
busybox:
endif

ttyUSB:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/ttyUSB dynamic
ttyUSB_clean:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/ttyUSB clean

wlancmd:	
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/wlancmd dynamic
wlancmd_clean:
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/wlancmd clean

wireless_tools:	
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/wireless_tools all	
wireless_tools_clean:	
	cd $(OPENSOURCE_DIR);
	$(MAKE) -C $(OPENSOURCE_DIR)/wireless_tools clean	
	
ifneq ($(strip $(BUILD_SAMBA)),)
samba:
	$(MAKE) -C $(OPENSOURCE_DIR)/samba dynamic
samba_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/samba clean
else
samba:
samba_clean:
endif

ifneq ($(strip $(BUILD_SCOUT)),)
scout:
	$(MAKE) -C $(OPENSOURCE_DIR)/scout dynamic
scout_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/scout clean
else
scout:
scout_clean:
endif

ifneq ($(strip $(BUILD_UDPECHOS)),)
udpechoserver:
	$(MAKE) -C $(OPENSOURCE_DIR)/udpechoserver dynamic
udpechoserver_clean:
	$(MAKE) -C $(OPENSOURCE_DIR)/udpechoserver clean
else
udpechoserver:
udpechoserver_clean:
endif

clean: kernel_clean opensource_clean fssrc_clean

kernel_clean: sanity_check
	$(MAKE) -C $(KERNEL_DIR) mrproper
	rm -f $(KERNEL_DIR)/arch/mips/defconfig
	rm -f $(HOSTTOOLS_DIR)/lzma/decompress/*.o

opensource_clean: dnsmasq_clean iptables_clean scout_clean web-tool_clean wlancmd_clean diagd_clean ebtables_clean libosip2_clean samba_clean siproxd_clean udpechoserver_clean \
              wireless_tools_clean zebra_clean iproute2_clean bridge-utils_clean atm2684_clean pppd_clean udhcp_clean ttyUSB_clean busybox_clean ftpd_clean

fssrc_clean:
	rm -fr $(INSTALL_DIR)/sbin
	rm -fr $(INSTALL_DIR)/lib
	rm -fr $(INSTALL_DIR)/upnp
	rm -fr $(INSTALL_DIR)/docs
	rm -fr $(INSTALL_DIR)/webs
	rm -fr $(INSTALL_DIR)/usr
	rm -fr $(INSTALL_DIR)/linuxrc

###########################################
#
# System-wide exported variables
# (in alphabetical order)
#
###########################################

export \
AR                         \
AS                         \
BRCM_APP_PHONE             \
BRCMAPPS                   \
BRCM_BOARD                 \
BRCM_DRIVER_PCI            \
BRCMDRIVERS_DIR            \
BRCM_DSP_APM_FXO           \
BRCM_DSP_APM_FXO_EXT       \
BRCM_DSP_CODEC_G711        \
BRCM_DSP_CODEC_G723        \
BRCM_DSP_CODEC_G726        \
BRCM_DSP_CODEC_G729        \
BRCM_DSP_CODEC_G7xx        \
BRCM_DSP_CODEC_T38_EXT     \
BRCM_DSP_CODEC_T38_INT     \
BRCM_DSP_HAL               \
BRCM_DSP_HAL_EXTENSION     \
BRCM_DSP_PCM               \
BRCM_DSP_PCM_G726          \
BRCM_DSP_PCM_T38_EXT       \
BRCM_EXTRAVERSION          \
BRCM_KERNEL_NETQOS         \
BRCM_KERNEL_ROOTFS         \
BRCM_LDX_APP               \
BRCM_MIPS_ONLY_BUILD       \
BRCM_BOARD_ID       	   \
USB_PORT_NUM			   \
BRCM_PSI_VERSION           \
BRCM_PTHREADS              \
BRCM_RELEASE               \
BRCM_RELEASETAG            \
BRCM_SNMP                  \
BRCM_UCLIBC                \
BRCM_VERSION               \
BRCM_VODSL_DUAL_3341       \
BRCM_VOICE_COUNTRY_JAPAN   \
BRCM_VOICE_GLOBAL_CFLAGS   \
BROADCOM_CFM_DIR           \
BROADCOM_TR69C_DIR           \
BUILD_ADSLCTL              \
BUILD_ATMCTL               \
BUILD_BR2684CTL            \
BUILD_BRCM_VLAN            \
BUILD_BRCTL                \
BUILD_FWLOG                \
BUILD_HWLOG                \
BUILD_BUSYBOX              \
BUILD_CERT                 \
BUILD_CFM                  \
BUILD_CFM_CLI              \
BUILD_CFM_TELNETD          \
BUILD_DDNSD                \
BUILD_DHCPR                \
BUILD_DIAGAPP              \
BUILD_DIR                  \
BUILD_DNSPROBE             \
BUILD_EBTABLES             \
BUILD_EPITTCP              \
BUILD_ETHWAN               \
BUILD_FTPD                 \
BUILD_TFTP                 \
BUILD_TFTPD                \
BUILD_GDBSERVER            \
BUILD_IGMP                 \
BUILD_IPPD                 \
BUILD_IPSEC_TOOLS          \
BUILD_IPTABLES             \
BUILD_WPS_BTN              \
BUILD_LLTD                 \
BUILD_WSC                   \
BUILD_BCMCRYPTO \
BUILD_BCMSHARED \
BUILD_NAS                  \
BUILD_NETCTL               \
BUILD_NVRAM                \
BUILD_PORT_MIRRORING	   \
BUILD_PPPD                 \
BUILD_PVC2684CTL           \
BUILD_REAIM                \
BUILD_RT2684D              \
BUILD_SAMBA                \
BUILD_SCOUT                \
BUILD_SES                  \
BUILD_WLAN_PRNTCTL         \
BUILD_SIPROXD              \
BUILD_SLACTEST             \
BUILD_SNMP                 \
BUILD_SNTP                 \
BUILD_SOAP                 \
BUILD_SOAP_VER             \
BUILD_SSHD_MIPS_GENKEY     \
BUILD_TOD                  \
BUILD_MAC                  \
BUILD_TR69C                \
BUILD_TR69C_SSL            \
BUILD_TR64                \
BUILD_TR64_SSL            \
BUILD_UDHCP                \
BUILD_UPNP                 \
BUILD_VCONFIG              \
BUILD_VCONFIG              \
BUILD_VODSL                \
BUILD_WLCTL                \
BUILD_ZEBRA                \
BUSYBOX_DIR                \
BUILD_POLICY_ROUTE         \
CC                         \
CROSS_COMPILE              \
CXX                        \
DEFAULTCFG_DIR             \
FSSRC_DIR                  \
HOSTTOOLS_DIR              \
HGW_PRODUCTTYPE            \
HGW_PRODUCTNAME            \
HGW_SOFTVERSION            \
INC_ADSLDRV_PATH           \
INC_BRCMBOARDPARMS_PATH    \
INC_BRCMCFM_PATH           \
INC_BRCMDRIVER_PRIV_PATH   \
INC_BRCMDRIVER_PUB_PATH    \
INC_BRCMSHARED_PRIV_PATH   \
INC_BRCMSHARED_PUB_PATH    \
INC_FLASH_PATH             \
INC_ENDPOINT_PATH          \
INC_KERNEL_BASE            \
INSTALL_DIR                \
JTAG_KERNEL_DEBUG          \
KERNEL_DIR                 \
KPN_QOS                    \
LD                         \
LIBCDIR                    \
LIBDIR                     \
LIB_PATH                   \
LINUXDIR                   \
NM                         \
OBJCOPY                    \
OBJDUMP                    \
PROFILE_DIR                \
RANLIB                     \
RUN_NOISE                  \
SSTRIP                     \
STRIP                      \
TARGETS_DIR                \
TOOLCHAIN                  \
USERAPPS_DIR               \
WEB_POPUP                  \
XCHANGE_DIR                \
XCHANGE_DSP_APP_EXTENSION  \
VCOPE_TYPE                 \
VCOPE_BOARD                \
VCOPE_LINE_NUMBER          \
HARDWARE_VERSION           \
BUILD_RADIUS		       \
BUILD_SNTP_AUTOCONF        \
BUILD_SYSLOG_AUTOCONF      \
BUILD_KPN_SYSLOG	       \
HRD_HG520V                 \
HRD_HG510V                 \
HRD_HG510VI                \
HRD_HG550II                \
BUILD_KPN_VLAN             \
BUILD_VLAN_CONFIG          \
BUILD_DHCPOPTION	       \
BUILD_ONE_USER             \
BUILD_WLAN_REVERSE_PRESS_RESET_KEY \
BUILD_PASSWORD_UNENCRYPTED         \
BUILD_CHINA_TELCOM_TEST_WARNING	   \
BUILD_DHCP_FRAG                    \
BUILD_CHINATELECOM_DHCP            \
BUILD_SLIC_3210                    \
BUILD_SLIC_3215            \
BUILD_SLIC_LE88221            \
BUILD_SINGAPORE_LOGIN      \
BUILD_WL_MAC_AS_SSID       \
BUILD_DIF_UE_PORTS         \
BUILD_FXO_DAA_CIRCUIT      \
BUILD_FXO_DAA_CHIP         \
BUILD_CUSTOMER_TELMEX      \
BUILD_CUSTOMER_SINGTEL    \
BUILD_CUSTOMER_HUVDF      \
BUILD_CUSTOMER_ITALY      \
BUILD_CUSTOMER_SPAIN      \
BUILD_CUSTOMER_GLOBAL     \
BUILD_OPTION61            \
BUILD_PORTMAPING          \
BUILD_WEBCOMPRESSED       \
BUILD_HSPA_DATA			\
BUILD_DNSMASQ           \
BUILD_VDF_DHCP			\
BUILD_DHCPHOSTID         \
BUILD_DHCPCLIENTID       \
BUILD_IGMPCONFIG         \
BUILD_PPPOUCONFIG        \
BUILD_SUPPORT_VDF_DHCP  \
BUILD_VDF_CD            \
BUILD_BACKUPCFGFILE \
BUILD_SUPPORT_VDF_DIAGNOSE \
BUILD_VDF_WANINFO\
BUILD_SUPPORT_VDF_DMZ\
BUILD_VDF_MACMATCHIP\
BRCM_RELEASE_VERSION \
BUILD_SUPPORT_WEB_AUTH \
BUILD_FXO_REINJECTION \
BUILD_FXO_FUNCTION \
BUILD_SUPPORT_CD_WEB \
BUILD_SUPPORT_WEB_LANG \
BUILD_RT30XX_WLAN \
BUILD_MBSS     \
BUILD_WPS      \
BUILD_TR143\
BUILD_UDPECHOS
