# File: bcmdrivers/char/bcm963xx/adsl/Makefile
#
# Makefile for the BCM63xx ADSL connection driver.
#

obj-$(CONFIG_BCM_ADSL) += adsldd.o

hmi-objs := BcmXdslHmi.o BcmHmiMsgReply.o bcm_hmiMsgHdr.o bcm_hmiCoreMsg.o bcm_bigEndianByteSwap.o bcm_layout.o bcm_hmiLineMsg.o

softdsl-objs += softdsl/SoftDslFrame.o softdsl/HdlcFramer.o softdsl/HdlcByte.o softdsl/G997.o 
softdsl-objs += softdsl/Flatten.o softdsl/DslFramer.o softdsl/AdslMibObj.o softdsl/AdslMib.o softdsl/G992p3OvhMsg.o
softdsl-objs += softdsl/BlockShortMove.o softdsl/BlockLongMove.o softdsl/BlockByteMove.o softdsl/BlockByteFill.o softdsl/ASSERT_SIZE.o

adsldd-objs += adsldrv.o adsl.o AdslCore.o AdslCoreFrame.o BcmAdslCore.o BcmAdslDiagLinux.o BcmOs.o AdslFile.o
adsldd-objs += $(softdsl-objs)

ifeq "$(HMI_SUPPORT)" "y"
EXTRA_CFLAGS += -DHMI_QA_SUPPORT
adsldd-objs += $(hmi-objs)
endif

EXTRA_CFLAGS += -DVDF_SPAIN

ifeq "$(ADSL_PHY_MODE)" "file"
EXTRA_CFLAGS += -DADSL_PHY_FILE
endif

ifeq "$(BRCM_CHIP)" "6338"
BRCM_CHIP=6348
endif
ifeq "$(BRCM_CHIP)" "6358"
BRCM_CHIP=6348
endif

ifeq "$(ADSL)" "ANNEX_C"
adslphy := adslcore$(BRCM_CHIP)C
EXTRA_CFLAGS += -DADSL_ANNEXC
endif
ifeq "$(ADSL)" "ANNEX_B"
adslphy := adslcore$(BRCM_CHIP)B
EXTRA_CFLAGS += -DADSL_ANNEXB
endif
ifeq "$(ADSL)" "SADSL"
adslphy := adslcore$(BRCM_CHIP)SA
EXTRA_CFLAGS += -DADSL_SADSL
endif
ifeq "$(ADSL)" "ANNEX_A"
adslphy := adslcore$(BRCM_CHIP)
endif
ifeq "$(ADSL_SELF_TEST)" "y"
adslphy := adslcore$(BRCM_CHIP)Test
EXTRA_CFLAGS += -DADSL_SELF_TEST
endif

ifneq ($(strip $(BUILD_TR69_XBRCM)),)
EXTRA_CFLAGS += -DADSL_MIBOBJ_PLN
endif

ifneq ($(strip $(BUILD_SNMP_EOC)),)
EXTRA_CFLAGS += -DBUILD_SNMP_EOC
endif
ifneq ($(strip $(BUILD_SNMP_AUTO)),)
EXTRA_CFLAGS += -DBUILD_SNMP_AUTO
endif
ifeq "$(BUILD_SNMP_TRANSPORT_DEBUG)" "y"
EXTRA_CFLAGS += -DBUILD_SNMP_TRANSPORT_DEBUG
endif

EXTRA_CFLAGS += -DLINUX_FW_VERSION=$(BRCM_VERSION)$(BRCM_RELEASE)

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) ${BRCM_DRIVER_ATM_BONDING_DEFINES} 

ifeq "$(ADSL_PHY_MODE)" "file"

HOST_EXTRACFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

ifeq "$(ADSL_SELF_TEST)" "y"
PHYBLD_BIN := AdslTestPhyBld
PHYBLD_C := $(obj)/AdslTestPhyBld.c
else
PHYBLD_BIN := AdslPhyBld
PHYBLD_C := $(obj)/AdslPhyBld.c
endif

PHYBLD := $(obj)/$(adslphy)/$(PHYBLD_BIN)

clean-files := adsl_phy.bin adsl_test_phy.bin
clean-files += $(adslphy)/$(PHYBLD_BIN)

$(PHYBLD): $(obj)/$(adslphy)/*.[ch] $(PHYBLD_C)
ifeq "$(ADSL_SELF_TEST)" "y"
	cd $(obj)/$(adslphy); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslTestPhyBld.c $(obj)/$(adslphy)/adsl_selftest_lmem.c $(obj)/$(adslphy)/adsl_selftest_sdram.c; \
	./$(PHYBLD_BIN) ../adsl_test_phy.bin
else
	cd $(obj)/$(adslphy); \
	$(HOSTCC) $(HOST_EXTRACFLAGS) -o $@ $(obj)/AdslPhyBld.c $(obj)/$(adslphy)/adsl_lmem.c $(obj)/$(adslphy)/adsl_sdram.c; \
	./$(PHYBLD_BIN) ../adsl_phy.bin
endif	

$(obj)/adsldrv.o: $(PHYBLD)
targets += $(obj)/adsldrv.o


else

ifeq "$(ADSL_SELF_TEST)" "y"
adsldd-objs += $(adslphy)/adsl_selftest_lmem.o $(adslphy)/adsl_selftest_sdram.o
else
adsldd-objs += $(adslphy)/adsl_lmem.o $(adslphy)/adsl_sdram.o
endif
	
endif
	
-include $(TOPDIR)/Rules.make

