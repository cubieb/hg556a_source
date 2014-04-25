#
# Makefile for USB Host Controller Driver
# framework and drivers
#

obj-$(CONFIG_USB_EHCI_HCD)	+= ehci-hcd.o
obj-$(CONFIG_USB_OHCI_HCD)	+= ohci-hcd.o
obj-$(CONFIG_USB_UHCI_HCD)	+= uhci-hcd.o

obj-$(CONFIG_USB_SL811HS)	+= hc_sl811.o

ifeq ($(CONFIG_BCM96348),y)
EXTRA_CFLAGS	+= -DCONFIG_SWAP_IO_SPACE -D__MIPSEB__
endif