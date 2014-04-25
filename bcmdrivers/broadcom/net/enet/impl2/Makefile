# File: bcmdrivers/broadcom/net/enet/bcm963xx/Makefile
#
# Makefile for the BCM63xx Enet/switch driver
#

obj-$(CONFIG_BCM_ENET) += bcm_enet.o vnet.o
bcm_enet-objs += bcmenet.o ../shared/bcmmii.o

EXTRA_CFLAGS	+= -I$(obj) -I$(obj)/../shared
EXTRA_CFLAGS	+= -DEXPORT_SYMTAB

#-DDUMP_DATA -- dump data
#-DUSE_BH -- use bottom half
#-DUSE_PROC -- display ethernet driver internal info
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -DUSE_PROC
EXTRA_CFLAGS += -DUSE_BH
EXTRA_CFLAGS += -DVPORTS

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s
