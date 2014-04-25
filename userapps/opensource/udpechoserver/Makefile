#
# build file 
#
#
CFLAGS = -s -Os -fomit-frame-pointer
LDFLAGS = -Wl,-allow-shlib-undefined
ifeq ($(strip $(BUILD_UDPECHOS)), static)
CFLAGS += -DBUILD_STATIC 
endif

CFLAGS+=-Wall

OBJS = udpechoserver.o

all: udpechoserver

install::
	install -m 755 udpechoserver $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/udpechoserver

dynamic: all install

static: udpechoserver.a

.c.o:
	$(CC) $(CFLAGS) -c $<

udpechoserver: udpechoserver.o
	$(CC) $(LDFLAGS) -o udpechoserver udpechoserver.o

udpechoserver.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	-rm -f udpechoserver *.o  dhcpr.a
