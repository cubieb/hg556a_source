# $Id: Makefile,v 1.1 2008/08/26 01:34:07 l65130 Exp $
#
# Makefile for the linux kernel.
#

extra-y	:= vmlinux.lds.s

obj-y   := process.o traps.o irq.o ptrace.o setup.o \
	   time.o sys_cris.o semaphore.o

obj-$(CONFIG_MODULES)    += crisksyms.o
obj-$(CONFIG_MODULES)	 += module.o

clean:

