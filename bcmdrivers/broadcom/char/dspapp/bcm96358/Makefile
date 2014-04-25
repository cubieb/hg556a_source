# File: bcmdrivers/char/impl1/Makefile
#
# Makefile for the BCM6358 DSP application driver.
#

obj-$(CONFIG_BCM_BCMDSP) += dspdd.o
dspdd-objs := dspdrv.o dspApp$(BRCM_CHIP).o

$(obj)/dspApp$(BRCM_CHIP).o:
	cd $(XCHANGE_DIR)/dslx;                                  \
	$(XCHANGE_DIR)/dslx/build -a $(XCHANGE_BUILD_APP) -d $(XCHANGE_DSP_APP)$(XCHANGE_DSP_APP_EXTENSION) -t dspdd
	cp -f $(XCHANGE_DIR)/dslx/obj/LinuxKernel/$(XCHANGE_BUILD_APP)/dspApp$(BRCM_CHIP).o $(src)/dspApp$(BRCM_CHIP).o;
	$(OBJCOPY) --prefix-symbol TP0 $(src)/dspApp$(BRCM_CHIP).o $(src)/dspApp$(BRCM_CHIP)New.o	
	mv -f $(src)/dspApp$(BRCM_CHIP)New.o $(src)/dspApp$(BRCM_CHIP).o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -fno-exceptions

-include $(TOPDIR)/Rules.make

clean:
	
