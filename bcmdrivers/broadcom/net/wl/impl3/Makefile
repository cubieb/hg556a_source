export WLSRCBASE= $(obj)
export WLCFGDIR = $(obj)/wl/config

export WLBASE	= wl
export SHBASE	= shared
export COBASE	= bcmcrypto

#mark WLAN configuration
WLLX=1

include $(WLCFGDIR)/wlconfig_lx_wl_dslcpe
include $(WLCFGDIR)/wl.mk

wl_open_objs := $(WLBASE)/sys/wl_linux_dslcpe.o
wl_open_objs += $(SHBASE)/bcmsrom.o $(SHBASE)/linux_osl.o $(SHBASE)/bcmutils.o

######################################################
ifneq ($(strip $(WLDEVBUILD)), 1)
obj-$(CONFIG_BCM_WLAN) += wl.o
wl-objs += wl_dep.o  $(wl_open_objs)
$(obj)/wl_dep.o: build_srom_map
	cp $(obj)/wl$(BRCM_CHIP)partial.o_save $(obj)/wl_dep.o
endif
######################################################

include $(WLSRCBASE)/Makefile.srom

EXTRA_CFLAGS	+= -I$(WLSRCBASE)/include
EXTRA_CFLAGS 	+= -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS	+= -DIL_BIGENDIAN
EXTRA_CFLAGS	+= -DBCMDRIVER
EXTRA_CFLAGS	+= $(WLFLAGS)
EXTRA_CFLAGS	+= -DDSLCPE
EXTRA_CFLAGS	+= -DDSLCPE_DGASP
EXTRA_CFLAGS	+= -DWLTEST
EXTRA_CFLAGS	+= -DDSLCPE_DELAY
#EXTRA_CFLAGS	+= -DBCMDBG -DBCMDBG_ERR
EXTRA_CFLAGS	+= -I$(WLSRCBASE)/wl/sys
EXTRA_CFLAGS	+= -DDSLCPE_WOMBO

clean:
	rm -f core $(obj)/wl/sys/*.o $(obj)/wl/sys/*.a $(obj)/wl/sys/*.s $(obj)/wl/sys/*.bin
	rm -f core $(obj)/shared/*.o $(obj)/shared/*.a $(obj)/shared/*.s $(obj)/shared/*.bin
	rm -f core $(obj)/bcmcrypto/*.o $(obj)/bcmcrypto/*.a $(obj)/bcmcrypto/*.s $(obj)/bcmcrypto/*.bin
		
-include $(TOPDIR)/Rules.make

