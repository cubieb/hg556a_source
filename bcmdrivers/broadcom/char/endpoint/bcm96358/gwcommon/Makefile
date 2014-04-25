###################################################################################
#
#    Copyright 2000  Broadcom Corporation
#    All Rights Reserved
#    No portions of this material may be reproduced in any form without the
#    written permission of:
#             Broadcom Corporation
#             16215 Alton Parkway
#             P.O. Box 57013
#             Irvine, California 92619-7013
#    All information contained in this document is Broadcom Corporation
#    company private, proprietary, and trade secret.
#
###################################################################################

obj-m := gwcommon.o
gwcommon-objs := gwcommon-lib.a

ENDPT_LIBS_SUBDIR := eptlib

XCHANGE_BUILD_EXTRA_CFLAGS = $(addprefix -D ,$(BRCM_VOICE_GLOBAL_CFLAGS))
ifeq ($(strip $(BRCM_PROFILER_TOOL)),1)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_PROFILER_ENABLED)
endif
ifeq ($(strip $(BRCM_CHIP)),6348)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6348)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , HAUS_OS_LINUXKERNEL)
endif
ifeq ($(strip $(BRCM_CHIP)),6358)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6358)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , HAUS_OS_LINUXKERNEL)
endif
ifeq ($(strip $(BRCM_CHIP)),6368)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6368)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , HAUS_OS_LINUXKERNEL)
endif
ifeq ($(strip $(BRCM_VODSL_DUAL_3341)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_DUAL_3341)
endif
ifeq ($(strip $(BRCM_DSP_PCM_G726)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_TDM_CONFERENCING_ENABLED)
endif
ifeq ($(strip $(BRCM_VOIP_TO_FXO)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_VOIP_TO_FXO)
endif

#
# Locale Selection
#
XCHANGE_BUILD_EXTRA_CFLAGS += $(LOCALEFLAGS_TYPE2)

# 
# xchg_common libraries
#
XCHG_LIBS :=           \
assert                 \
bos                    \
crt                    \
sme                    \
str

XCHG_LIBS += blog

# 
# voice_res_gw libraries
#
VRG_LIBS :=            \
bcm$(BRCM_CHIP)$(BRCM_DSP_HAL)$(BRCM_DSP_HAL_EXTENSION) \
casCtl                 \
classStm               \
codec                  \
endpt                  \
hdsp                   \
heartbeat              \
lhapi


# 
# xchg_drivers libraries
#
XDRV_LIBS :=           \
bcm63xx_intCtrl        \
casDriver              \
dbgCom


#
# Basic libraries for 6358 load
#
ifeq ($(strip $(BRCM_CHIP)),6358)
XDRV_LIBS +=  apmSim               \
              slicSim              \
              dlbStub              \
              bcm6358_mspi         \
              bcm6358_pcm          \
              bcm6358_led          \
              bcm6358_relay        \
              bcm63xx_resetCallout \
              debugCmtInterface    \
              exceptionHdlrCmtMips \
              exceptionHdlrMips    \
              flowCtrl             \
              ipcSharedMem         \
              itpcMips             \
              spinLockMips         \
              bcm6358_boot         \
              bcm6358_handshake    \
              cacheMips            \
              cp0Mips              \
              globalInterruptMips  
endif	      


#
# Basic libraries for 6368 load
#
ifeq ($(strip $(BRCM_CHIP)),6368)
XDRV_LIBS +=  apmSim               \
              slicSim              \
              dlbStub              \
              bcm6368_mspi         \
              bcm6368_pcm          \
              bcm6368_led          \
              bcm6368_relay        \
              bcm63xx_resetCallout \
              debugCmtInterface    \
              exceptionHdlrCmtMips \
              exceptionHdlrMips    \
              flowCtrl             \
              ipcSharedMem         \
              itpcMips             \
              spinLockMips         \
              bcm6368_boot         \
              bcm6368_handshake    \
              cacheMips            \
              cp0Mips              \
              globalInterruptMips  
endif	      

#
# Basic libraries for 6348 load
#
ifeq ($(strip $(BRCM_CHIP)),6348)

XDRV_LIBS += bcm3341_apm
XDRV_LIBS += bcm3341_assertHdlr
XDRV_LIBS += bcm3341_diag
XDRV_LIBS += bcm3341_intCtrl
XDRV_LIBS += bcm3341_ipc
XDRV_LIBS += bcm3341_mailbox
XDRV_LIBS += bcm3341_tdm
XDRV_LIBS += bcm3341_vpm
XDRV_LIBS += bcm$(BRCM_CHIP)_reset3341
XDRV_LIBS += flowCtrl
XDRV_LIBS += ledSim
#
# Additional driver libraries for LDX 2.1 based application
#
XDRV_LIBS += debugZspInterface
XDRV_LIBS += ipcSharedMem
XDRV_LIBS += globalInterruptMips
XDRV_LIBS += dlbStub

endif

ifeq ($(strip $(BRCM_6358_G729)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3210
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3215
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88221
endif

ifeq ($(strip $(BRCM_6358_2FXS_DECT_PONG)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88221
endif

ifeq ($(strip $(BRCM_6358_WIDEBAND)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3216 bcm6358_slicle88221
endif

ifeq ($(strip $(BRCM_6358_G729_FXO)),y) 
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3210 bcm6358_slicsi3215 bcm6358_slicle88221 bcm6358_daasi3050 bcm6358_daale88010
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
endif

ifeq ($(strip $(BRCM_6358_WIDEBAND_FXO)),y) 
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3210 bcm6358_slicsi3216 bcm6358_slicle88221 bcm6358_daasi3050 bcm6358_daale88010
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
endif

ifeq ($(strip $(BRCM_6368_G729_FXO)),y) 
XDRV_LIBS := $(XDRV_LIBS) bcm6368_slicsi3215 bcm6368_slicle88221 bcm6368_daasi3050 bcm6368_daale88010
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
endif

ifeq ($(strip $(BRCM_6358_G729_ISDN_FXO)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicsi3215 bcm6358_slicle88221 bcm6358_daasi3050 bcm6358_daale88010
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
endif

ifeq ($(strip $(BRCM_6358_G729_ISDN_FXO_LE88311)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88311 bcm6358_daale88311
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
endif

ifeq ($(strip $(BRCM_6358_G729_DECT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88111 bcm6358_dectA
VRG_LIBS  := $(VRG_LIBS) dectCtl
endif

ifeq ($(strip $(BRCM_6358_G729_WB_DECT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88111 bcm6358_dectA
VRG_LIBS  := $(VRG_LIBS) dectCtl
endif

ifeq ($(strip $(BRCM_6358_G729_1FXS)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm6358_slicle88111
endif

ifeq ($(strip $(BRCM_DSP_FXO)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_gpio bcm3341_mspi bcm3341_daaSi3050
VRG_LIBS  := $(VRG_LIBS) cmtdCtl pstnCtl
ifeq ($(strip $(BRCM_SLIC_LE9500)),y)
XDRV_LIBS += bcm3341_slicl9500
endif
ifeq ($(strip $(BRCM_SLIC_LE9502)),y)
XDRV_LIBS += bcm3341_slicLegerity
endif
endif

ifeq ($(strip $(BRCM_DSP_PCM)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_gpio bcm3341_mspi bcm3341_slacLE78D11
endif 

ifeq ($(strip $(BRCM_DSP_PCM_G726)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_gpio bcm3341_mspi bcm3341_slacLE78D11
endif 

ifeq ($(strip $(BRCM_DSP_PCM_T38_EXT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_gpio bcm3341_mspi bcm3341_slacLE78D11
endif 

ifeq ($(strip $(BRCM_DSP_HYBRID)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500 bcm3341_gpio bcm3341_mspi bcm3341_slacLE78D11
endif

ifeq ($(strip $(BRCM_DSP_HYBRID_EXT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500 bcm3341_gpio bcm3341_mspi bcm3341_slacLE78D11
export $(XCHANGE_DSP_APP_EXTENSION)
endif

#
# The following library is only needed on 6348 chip
#
ifeq ($(strip $(BRCM_CHIP)),6348)

ifeq ($(strip $(BRCM_DSP_CODEC_G711)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G723)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G726)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G729)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_G7xx)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_T38_EXT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

ifeq ($(strip $(BRCM_DSP_CODEC_T38_INT)),y)
XDRV_LIBS := $(XDRV_LIBS) bcm3341_slicl9500
endif

endif

ifeq ($(strip $(BRCM_CHIP)),6358)

headerFileDiffTest:
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6358/chipinc/6358_common.h $(XCHANGE_DIR)/../bcmdrivers/broadcom/include/bcm963xx/6358_common.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6358/chipinc/6358_map.h $(XCHANGE_DIR)/../bcmdrivers/broadcom/include/bcm963xx/6358_map.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6358/chipinc/6358_intr.h $(XCHANGE_DIR)/../bcmdrivers/opensource/include/bcm963xx/6358_intr.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6358/chipinc/bcmtypes.h $(XCHANGE_DIR)/../bcmdrivers/opensource/include/bcm963xx/bcmtypes.h

else

ifeq ($(strip $(BRCM_CHIP)),6368)

headerFileDiffTest:
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6368/chipinc/6368_common.h $(XCHANGE_DIR)/../bcmdrivers/broadcom/include/bcm963xx/6368_common.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6368/chipinc/6368_map.h $(XCHANGE_DIR)/../bcmdrivers/broadcom/include/bcm963xx/6368_map.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6368/chipinc/6368_intr.h $(XCHANGE_DIR)/../bcmdrivers/opensource/include/bcm963xx/6368_intr.h
#	diff -y --suppress-common-lines $(XCHANGE_DIR)/dslx_common/xchg_drivers/bcm6368/chipinc/bcmtypes.h $(XCHANGE_DIR)/../bcmdrivers/opensource/include/bcm963xx/bcmtypes.h

else

headerFileDiffTest:
endif
endif


$(obj)/gwcommon-lib.a: headerFileDiffTest
	cd $(XCHANGE_DIR)/dslx;                                  \
	$(XCHANGE_DIR)/dslx/build -a $(XCHANGE_BUILD_APP) -d $(XCHANGE_DSP_APP)$(XCHANGE_DSP_APP_EXTENSION) -t bosk $(XCHANGE_BUILD_EXTRA_CFLAGS) &&   \
	$(XCHANGE_DIR)/dslx/build -a $(XCHANGE_BUILD_APP) -d $(XCHANGE_DSP_APP)$(XCHANGE_DSP_APP_EXTENSION) -t xdrv $(XCHANGE_BUILD_EXTRA_CFLAGS) &&   \
	$(XCHANGE_DIR)/dslx/build -a $(XCHANGE_BUILD_APP) -d $(XCHANGE_DSP_APP)$(XCHANGE_DSP_APP_EXTENSION) -t vrg $(XCHANGE_BUILD_EXTRA_CFLAGS)

	mkdir -p $(CURDIR)/$(ENDPT_LIBS_SUBDIR)

	for i in $(XCHG_LIBS); do                                \
	cp -f $(XCHANGE_DIR)/dslx/lib/LinuxKernel/$(XCHANGE_BUILD_APP)/xchg/$$i.a $(CURDIR)/$(ENDPT_LIBS_SUBDIR)/$$i.lib;    \
	done;                                                    

	for i in $(VRG_LIBS); do                                 \
	cp -f $(XCHANGE_DIR)/dslx/lib/LinuxKernel/$(XCHANGE_BUILD_APP)/vrg/$$i.a $(CURDIR)/$(ENDPT_LIBS_SUBDIR)/$$i.lib;     \
	done;                                                    

	for i in $(XDRV_LIBS); do                                \
	cp -f $(XCHANGE_DIR)/dslx/lib/LinuxKernel/$(XCHANGE_BUILD_APP)/xdrv/$$i.a $(CURDIR)/$(ENDPT_LIBS_SUBDIR)/$$i.lib;     \
	done;

	cd $(CURDIR)/$(ENDPT_LIBS_SUBDIR);                       \
	for i in $(XCHG_LIBS); do                                \
	   $(AR) x $$i.lib;                                      \
	done;                                                    \
	for i in $(VRG_LIBS); do                                 \
	   $(AR) x $$i.lib;                                      \
	done;                                                    \
	for i in $(XDRV_LIBS); do                                \
	   $(AR) x $$i.lib;                                      \
	done;                                                    \

	$(LD)	-G	0 -r -s $(CURDIR)/$(ENDPT_LIBS_SUBDIR)/*.o -o $(obj)/gwcommon-lib.a
	
