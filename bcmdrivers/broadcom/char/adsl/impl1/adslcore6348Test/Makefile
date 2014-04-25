# File: drivers/char/adsl/bcm963xx/adslcore/Makefile
#
# Makefile for the BCM63xx ADSL connection driver.
#

O_TARGET := adslselftest.o

EXTRA_CFLAGS += -I. -I$(HPATH)/asm/bcm963xx

ifeq "$(ADSL_PHY_MODE)" "file"
obj-y := ../adslnotestphy.o
../adslnotestphy.o:
	touch adslnotestphy.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ adslnotestphy.c
	rm -rf adslnotestphy.c
	cc $(EXTRA_CFLAGS) -I$(TOPDIR)/modulesrc/include/bcm963xx -o AdslTestPhyBld ../AdslTestPhyBld.c adsl_selftest_lmem.c adsl_selftest_sdram.c
	./AdslTestPhyBld ../adsl_test_phy.bin
	rm -rf AdslTestPhyBld
	cp $@ $(O_TARGET)
else
obj-y := adsl_selftest_lmem.o adsl_selftest_sdram.o
endif

export-objs	:= adsl_syms.o

obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make
