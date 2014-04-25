# File: bcmdrivers/broadcom/net/usb/Makefile
#
# Makefile for the BCM63xx USB driver
#

obj-$(CONFIG_BCM_USB) += bcm_usb.o
bcm_usb-objs += bcmusb.o bcmrndis.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -DUSE_BH

-include $(TOPDIR)/Rules.make
