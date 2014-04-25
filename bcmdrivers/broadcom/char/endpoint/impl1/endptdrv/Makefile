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

obj-m := endptdrv.o
endptdrv-objs := endptdrv-lib.a

ENDPTDRV_SUBDIR   =  lib
ENDPTDRV_LIBS     =  endptdrv
XCHANGE_BUILD_EXTRA_CFLAGS = $(addprefix -D ,$(BRCM_VOICE_GLOBAL_CFLAGS))
ifeq ($(strip $(BRCM_MIPS_ONLY_BUILD)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , ENDPOINT_DRIVER_MIPS_ONLY)
endif
ifeq ($(strip $(BRCM_PROFILER_TOOL)),1)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_PROFILER_ENABLED)
endif
ifeq ($(strip $(BRCM_CHIP)),6348)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6348)
endif
ifeq ($(strip $(BRCM_CHIP)),6358)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6358)
endif
ifeq ($(strip $(BRCM_CHIP)),6368)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_6368)
endif
ifeq ($(strip $(BRCM_VODSL_DUAL_3341)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_DUAL_3341)
endif
ifeq ($(strip $(BRCM_DSP_PCM_G726)),y)
XCHANGE_BUILD_EXTRA_CFLAGS += $(addprefix -D , BRCM_TDM_CONFERENCING_ENABLED)
endif

$(obj)/endptdrv-lib.a:
	echo $(src);
	echo $(ENDPTDRV_SUBDIR);
	cd $(XCHANGE_DIR)/dslx;                                  \
	$(XCHANGE_DIR)/dslx/build -a $(XCHANGE_BUILD_APP) -d $(XCHANGE_DSP_APP)$(XCHANGE_DSP_APP_EXTENSION) -t mta $(XCHANGE_BUILD_EXTRA_CFLAGS)

	for i	in	$(ENDPTDRV_LIBS);	do								\
		cp -f $(XCHANGE_DIR)/dslx/lib/LinuxKernel/$(XCHANGE_BUILD_APP)/mta/$$i.a $(src)/$(ENDPTDRV_SUBDIR)/$$i.lib;	\
	done;																\

	cd	$(src)/$(ENDPTDRV_SUBDIR);					\
	for i	in	$(ENDPTDRV_LIBS);	do								\
		$(AR)	x $$i.lib;											\
	done;																

	$(LD) -G 0 -r -s $(src)/$(ENDPTDRV_SUBDIR)/*.o -o $(obj)/endptdrv-lib.a
	rm -f $(src)/$(ENDPTDRV_SUBDIR)/*.o
