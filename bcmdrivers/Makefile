# File: bcmdrivers/Makefile
#
# Makefile for the Linux kernel modules.
#

LN_NAME=bcm9$(BRCM_CHIP)
LN_DRIVER_DIRS =

-include $(KERNEL_DIR)/.config

ifneq ($(CONFIG_BCM_BOARD),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BOARD_IMPL) opensource/char/board/$(BRCM_BOARD)/$(LN_NAME);
obj-$(CONFIG_BCM_BOARD) += opensource/char/board/$(BRCM_BOARD)/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_SERIAL),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_SERIAL_IMPL) opensource/char/serial/$(LN_NAME);
obj-$(CONFIG_BCM_SERIAL) += opensource/char/serial/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ATMAPI),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BLAA_IMPL) broadcom/atm/$(LN_NAME);
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ATMAPI_IMPL) broadcom/char/atmapi/$(LN_NAME);
  obj-$(CONFIG_BCM_ATMAPI) += broadcom/atm/$(LN_NAME)/
  obj-$(CONFIG_BCM_ATMAPI) += broadcom/char/atmapi/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ADSL),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ADSL_IMPL) broadcom/char/adsl/$(LN_NAME);
  obj-$(CONFIG_BCM_ADSL) += broadcom/char/adsl/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_PROCFS),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_PROCFS_IMPL) broadcom/char/bcmprocfs/$(LN_NAME);
  obj-$(CONFIG_BCM_PROCFS) += broadcom/char/bcmprocfs/$(LN_NAME)/
endif



ifneq ($(CONFIG_BCM_BCMPROF),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_BCMPROF_IMPL) broadcom/char/profiler/$(LN_NAME);
  obj-$(CONFIG_BCM_BCMPROF) += broadcom/char/profiler/$(LN_NAME)/
endif


ifneq ($(CONFIG_BCM_SECURITY),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_SECURITY_IMPL) broadcom/char/smdrv/$(LN_NAME);
  obj-$(CONFIG_BCM_SECURITY) += broadcom/char/smdrv/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ATMTEST),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ATMTEST_IMPL) broadcom/char/test/atm/$(LN_NAME);
  obj-$(CONFIG_BCM_ATMTEST) += broadcom/char/test/atm/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_ENET),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_ENET_IMPL) broadcom/net/enet/$(LN_NAME);
  obj-$(CONFIG_BCM_ENET) += broadcom/net/enet/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_USB),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_USB_IMPL) broadcom/net/usb/$(LN_NAME);
  obj-$(CONFIG_BCM_USB) += broadcom/net/usb/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_WLAN),)
   CONFIG_BCM_WLAN_IMPL=3
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_WLAN_IMPL) broadcom/net/wl/$(LN_NAME);
  obj-$(CONFIG_BCM_WLAN) += broadcom/net/wl/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_HPNA),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_HPNA_IMPL) broadcom/net/hpna/$(LN_NAME);
  obj-$(CONFIG_BCM_HPNA) += broadcom/net/hpna/$(LN_NAME)/
endif

ifneq ($(CONFIG_BCM_NETDEVTEST),)
  LN_DRIVER_DIRS +=ln -sn impl$(CONFIG_BCM_NETDEVTEST_IMPL) broadcom/net/test/$(LN_NAME);
  obj-$(CONFIG_BCM_NETDEVTEST) += broadcom/net/test/$(LN_NAME)/
endif

symlinks:
	find . -lname "*" -name "$(LN_NAME)" -print -exec rm -f "{}" ";"
	$(CONFIG_SHELL) -c "$(LN_DRIVER_DIRS)"

