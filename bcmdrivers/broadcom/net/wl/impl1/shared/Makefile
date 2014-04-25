#
# Makefile for the kernel objects for Broadcom wireless driver
#

obj-y +=

WLLX=1
WLCFGDIR	= $(obj)/../wl/config
WLCONFFILE	= wlconfig_lx_wl_dslcpe_binosl_pkt
include $(WLCFGDIR)/$(WLCONFFILE)

ifeq ($(BINOSL_PKT),1)
obj-y += linux_osl_pkt.o
EXTRA_CFLAGS	+= -DBINOSL_PKT
EXTRA_CFLAGS	+= -DDSLCPE
endif


EXTRA_CFLAGS	+= -I$(obj)/../include

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s *.bin
