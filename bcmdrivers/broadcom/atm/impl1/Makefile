# File: bcmdrivers/atm/bcm963xx/Makefile
#
# Makefile for the BCM63xx Broadcom Linux ATM API Device Driver.
#

obj-$(CONFIG_BCM_ATMAPI) += blaa_dd.o

blaa_dd-objs += blaadd.o bcmBlaaMirrorLinux.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s
