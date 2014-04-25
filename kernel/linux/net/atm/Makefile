#
# Makefile for the ATM Protocol Families.
#
ifneq ($(strip $(BUILD_BRCM_VLAN)),)
CFLAGS	+= -DSUPPORT_VLAN
endif

atm-y		:= addr.o pvc.o signaling.o svc.o ioctl.o common.o atm_misc.o raw.o resources.o
mpoa-objs	:= mpc.o mpoa_caches.o mpoa_proc.o

obj-$(CONFIG_ATM) += atm.o
obj-$(CONFIG_ATM_CLIP) += clip.o
atm-$(subst m,y,$(CONFIG_ATM_CLIP)) += ipcommon.o
obj-$(CONFIG_ATM_BR2684) += br2684.o
atm-$(subst m,y,$(CONFIG_ATM_BR2684)) += ipcommon.o
atm-$(subst m,y,$(CONFIG_NET_SCH_ATM)) += ipcommon.o
# CONFIG_MIPS_BRCM Begin Broadcom added code.
obj-$(CONFIG_ATM_RT2684) += rt2684.o
atm-$(subst m,y,$(CONFIG_ATM_RT2684)) += ipcommon.o
# CONFIG_MIPS_BRCM End Broadcom added code.
atm-$(CONFIG_PROC_FS) += proc.o

obj-$(CONFIG_ATM_LANE) += lec.o
obj-$(CONFIG_ATM_MPOA) += mpoa.o
obj-$(CONFIG_PPPOATM) += pppoatm.o
