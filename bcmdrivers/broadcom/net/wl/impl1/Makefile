obj-$(CONFIG_BCM_WLAN) += wl.o

wl-objs += wl_dep.o
ifneq ($(CONFIG_BCM_MIMO),)
$(obj)/wl_dep.o: $(obj)/shared/bcm4306_map.o $(obj)/shared/bcm4318_map.o $(obj)/shared/bcm4321_map.o
else
$(obj)/wl_dep.o: $(obj)/shared/bcm4306_map.o $(obj)/shared/bcm4318_map.o 
endif
	cp $(obj)/wl$(BRCM_CHIP).o_save $(obj)/wl_dep.o

include $(BRCMDRIVERS_DIR)/broadcom/net/wl/impl1/Makefile.srom

-include $(TOPDIR)/Rules.make

