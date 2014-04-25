# Makefile for the bcm963xx board drivers
#


obj-y := board.o bcm63xx_flash.o bcm63xx_led.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ifeq ($(strip $(WIRELESS)),1)
	EXTRA_CFLAGS += -DWIRELESS
endif
ifeq ($(strip $(HARDWARE_VERSION)),y)        
        EXTRA_CFLAGS += -DSUPPORT_HARDWARE_VERSION
endif

ifeq ($(strip $(WIRELESS)),1)
		EXTRA_CFLAGS += -DVDF_HG556
endif
ifeq ($(strip $(BUILD_ATHEROSWLAN)),1)      
        EXTRA_CFLAGS += -DSUPPORT_ATHEROSWLAN
endif
-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

