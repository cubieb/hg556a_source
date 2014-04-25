O_TARGET = rt3062.o

OBJS = wlancmdadapter.o
CUR_DIR = $(shell pwd)
CFLAGS=-s -Os -fomit-frame-pointer -I$(HOST_INCLUDE_DIR) -I$(WIRELESS_DIR)/lib/prismoids/main -I$(CUR_DIR)/include 

CFLAGS += -DATPWPA2 -DDSLCPE -DCNXT_WLU_CONFIG -DSUPPORT_MIMO 
   
ifneq ($(strip $(BUILD_NVRAM)),)
CFLAGS += -DSUPPORT_NVRAM
endif      
ifeq ($(strip $(BUILD_WPS)),y)
	CFLAGS += -DSUPPORT_WPS
endif
ifeq ($(strip $(BUILD_MBSS)),y)
	CFLAGS += -DSUPPORT_MBSS
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


ifeq ($(O_TARGET), $(OBJS))
$(O_TARGET):
else
$(O_TARGET): $(OBJS)
	rm -f $@
	$(LD) $(EXTRA_LDFLAGS) -r -o $@ $(filter $(OBJS), $^)
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
  
