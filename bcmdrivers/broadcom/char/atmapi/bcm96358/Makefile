# File: bcmdrivers/char/atmapi/bcm963xx/Makefile
#
# Makefile for the BCM63xx ATM API driver.
#

obj-$(CONFIG_BCM_ATMAPI) += atmapi.o

atmapi-objs := atmapidrv.o atmappl.o atmappltable.o atminterface.o atmnotify.o atmoslinux.o atmprocessor.o atmqueue.o atmsoftsar.o atmvccinfotable.o atmwrapperdriver.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_ADSLDRV_PATH) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -fno-exceptions -fno-rtti $(VCOPE_VDSL)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

