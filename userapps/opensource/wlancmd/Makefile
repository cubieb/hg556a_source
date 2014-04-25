#
# Makefile for wlancmd utility.
#
PROGRAM_NAME:=wlancmd
DRIVER_VENDOR_DIR:=&(BUILD_DIR)/driver/rt3052
INC_DRIVER_HUAWEI_DIR:=

OBJS = $(PROGRAM_NAME).o

OBJS += $(join $(SUB_DIRS), $(patsubst %, /%.o,$(notdir $(SUB_DIRS))))	

CFLAGS=-s -Os -fomit-frame-pointer -I$(INC_DRIVER_HUAWEI_DIR) -I$(INC_FLASHCONF_DIR) -I./include

CFLAGS += -DATPWPA2 -DDSLCPE -DCNXT_WLU_CONFIG -DSUPPORT_MIMO
ifeq ($(strip $(BUILD_WPS)),y)
	CFLAGS += -DSUPPORT_WPS
endif
ifeq ($(strip $(BUILD_MBSS)),y)
	CFLAGS += -DSUPPORT_MBSS
endif

ifeq ("$(PRODUCT_NAME)", "hg522c")
LIBS = -L$(WIRELESS_DIR)/lib/prismoids/main -loids
endif

SUB_DIRS = rt3062

ifeq ("$(CHIP_NAME)","bcm96358")
ifneq ($(strip $(BUILD_WPS)),)
LIBS += -L$(INSTALL_DIR)/lib -lnvram
endif
endif
ifeq ("$(CHIP_NAME)","bcm96368")
ifneq ($(strip $(BUILD_WPS)),)
LIBS += -L$(INSTALL_DIR)/lib -lnvram
endif
endif
ifeq ("$(CHIP_NAME)","rtl8672")
CFLAGS += -DSUPPORT_RTL8672
EXTRA_LIB = -L$(LIBA_TSP_DIR) -lbhalapi
endif
################ maybe we use more variety source code, then add rule below
%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_$@) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS_$@) -c -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $<

%.so : %.o
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

all_targets: $(O_TARGET) $(L_TARGET)

ifdef O_TARGET
ifeq ($(O_TARGET), $(OBJS))
$(O_TARGET):
else
$(O_TARGET): $(OBJS)
	rm -f $@
	$(LD) $(EXTRA_LDFLAGS) -r -o $@ $(filter $(OBJS), $^)
endif
endif

ifdef L_TARGET
$(L_TARGET): $(OBJS)
	$(RM) lib$@.a
	$(AR) $(EXTRA_ARFLAGS) rcs lib$@.a $@.o
endif


subdir-list = $(sort $(patsubst %,_subdir_%,$(SUB_DIRS)))
sub_dirs: $(subdir-list)

ifdef SUB_DIRS
$(subdir-list):
	$(MAKE) -C $(patsubst _subdir_%,%,$@)
endif

dynamic: $(PROGRAM_NAME) install

static: lib$(PROGRAM_NAME).a

$(PROGRAM_NAME): sub_dirs $(OBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $(PROGRAM_NAME) $(OBJS) $(LIBS) $(EXTRA_LIB)

lib$(PROGRAM_NAME).a: $(OBJS)
	$(AR) rcs $@ $^

shared: sub_dirs $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared $(OBJS) -o lib$(PROGRAM_NAME).so
	install -m 755 lib$(PROGRAM_NAME).so $(INSTALL_DIR)/lib


install:
	install -m 755 $(PROGRAM_NAME) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)


clean:
	$(RM) $(PROGRAM_NAME) *.o *.a *.so
	$(foreach dir,$(SUB_DIRS), if [ -d $(dir) ];then cd $(dir);make clean; cd $(CURR_DIR);fi;)