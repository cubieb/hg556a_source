# udhcp makefile

#CROSS_COMPILE=mips-linux-
LDFLAGS = -Wl,--allow-shlib-undefined

prefix=/usr
SBINDIR=/sbin
USRSBINDIR=${prefix}/sbin
USRBINDIR=${prefix}/bin
USRSHAREDIR=${prefix}/share

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
COMBINED_BINARY=1

# Uncomment this for extra output and to compile with debugging symbols
#DEBUG=1

# Uncomment this to output messages to syslog, otherwise, messages go to stdout
CFLAGS += -DSYSLOG

ifeq ($(strip $(BUILD_VDF_DHCP)),y)
CFLAGS += -DVDF_FORCERENEW
CFLAGS += -DVDF_RESERVED
CFLAGS += -DVDF_OPTION
endif

ifeq ($(strip $(BUILD_SUPPORT_VDF_DHCP)),y)
CFLAGS += -DSUPPORT_VDF_DHCP
endif

ifneq ($(strip $(BUILD_SYSLOG_AUTOCONF)),)
CFLAGS += -DSUPPORT_SYSLOG_AUTOCONF
endif

ifeq ($(strip $(BUILD_SINGAPORE_LOGIN)),y)
CFLAGS += -DSINGAPORE_LOGIN
endif


ifneq ($(strip $(BUILD_DHCPOPTION)),)
CFLAGS += -DSUPPORT_DHCPOPTIONS
endif

ifneq ($(strip $(BUILD_DHCPHOSTID)),)
	CFLAGS += -DSUPPORT_DHCPHOSTID
	CXXFLAGS += -DSUPPORT_DHCPHOSTID
endif

ifneq ($(strip $(BUILD_DHCPCLIENTID)),)
	CFLAGS += -DSUPPORT_DHCPCLIENTID
	CXXFLAGS += -DSUPPORT_DHCPCLIENTID
endif

ifneq ($(strip $(BUILD_SNTP_AUTOCONF)),)
CFLAGS += -DSUPPORT_SNTP_AUTOCONF
endif

ifneq ($(strip $(BUILD_DHCP_FRAG)),)
CFLAGS += -DSUPPORT_DHCP_FRAG	
endif

ifneq ($(strip $(BUILD_CHINATELECOM_DHCP)),)
CFLAGS += -DSUPPORT_CHINATELECOM_DHCP
endif

ifneq ($(strip $(BUILD_TR69C)),)
CFLAGS += -DSUPPORT_TR69C	
endif
ifneq ($(strip $(BUILD_PORTMAPING)),)
CFLAGS += -DSUPPORT_PORTMAPING	
endif

ifneq ($(strip $(BUILD_OPTION61)),)
CFLAGS += -DSUPPORT_OPTION61
endif

ifneq ($(strip $(BUILD_VDF_MACMATCHIP)),)
CFLAGS += -DSUPPORT_MACMATCHIP
endif

ifneq ($(strip $(BUILD_SUPPORT_SNTP_AUTOCONF)),)
CFLAGS += -DSUPPORT_SNTP_AUTOCONF
endif

INSTALL = install

VER := 0.9.6


OBJS_SHARED = options.o socket.o packet.o pidfile.o
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o
DHCPC_OBJS = dhcpc.o clientpacket.o script.o syscall.o board_api.o

ifdef COMBINED_BINARY
# BRCM begin
AR1 = udhcpd.a
# BRCM end
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
CFLAGS += -DCOMBINED_BINARY
else
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)

EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
endif

EXEC3 = dumpleases
OBJS3 = dumpleases.o

BOOT_PROGRAMS = udhcpc
DAEMONS = udhcpd
COMMANDS = dumpleases

ifdef SYSLOG
CFLAGS += -DSYSLOG
endif

CFLAGS += -s -W -Wall -Wstrict-prototypes -DVERSION='"$(VER)"' -I$(INC_BRCMCFM_PATH) -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx -g -v

ifdef DEBUG
CFLAGS += -g -DDEBUG
else
CFLAGS += -Os -fomit-frame-pointer
#STRIP=-s
endif

ifeq ($(strip $(BUILD_UDHCP)), static)
CFLAGS += -DBUILD_STATIC
endif

# BRCM begin
#all: $(AR1)
all: $(EXEC1) $(EXEC2) $(EXEC3)
# BRCM end

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

.c.o:
	$(CC) -c $(CFLAGS) $<
	
CFLAGS_EXTRA += -I$(INC_BRCMCFM_PATH) -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx -I$(BRCMDRIVERS_DIR)/broadcom/include/bcm963xx
syscall.o: $(BROADCOM_CFM_DIR)/util/system/syscall.c
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -c $(BROADCOM_CFM_DIR)/util/system/syscall.c
board_api.o: $(BROADCOM_CFM_DIR)/util/psi/board_api.c
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -c $(BROADCOM_CFM_DIR)/util/psi/board_api.c

# BRCM begin
$(AR1): $(OBJS1)
	$(AR) cvr $(AR1) $(OBJS1)
# BRCM end

$(EXEC1): $(OBJS1)
	$(CC) $(LDFLAGS) $(OBJS1) -lpthread -o $(EXEC1)

$(EXEC2): $(OBJS2)
	$(CC) $(LDFLAGS) $(OBJS2) -lpthread -o $(EXEC2)

$(EXEC3): $(OBJS3)
	$(CC) $(LDFLAGS) $(OBJS3) -o $(EXEC3)


install: 
	install -m 755 udhcpd $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/udhcpd
	ln -sf udhcpd $(INSTALL_DIR)/bin/dhcpc
	ln -sf udhcpd $(INSTALL_DIR)/bin/dhcpd

dynamic: all install

static: $(AR1)

#	$(INSTALL) $(STRIP) $(DAEMONS) $(USRSBINDIR)
#	$(INSTALL) $(STRIP) $(COMMANDS) $(USRBINDIR)
#ifdef COMBINED_BINARY
#	ln -sf $(USRSBINDIR)/$(DAEMONS) $(SBINDIR)/$(BOOT_PROGRAMS)
#else
#	$(INSTALL) $(STRIP) $(BOOT_PROGRAMS) $(SBINDIR)
#endif
#	mkdir -p $(USRSHAREDIR)/udhcpc
#	for name in bound deconfig renew script ; do \
#		$(INSTALL) samples/sample.$$name \
#			$(USRSHAREDIR)/udhcpc/default.$$name ; \
#	done
#	mkdir -p $(USRSHAREDIR)/man/man1
#	$(INSTALL) dumpleases.1 $(USRSHAREDIR)/man/man1
#	mkdir -p $(USRSHAREDIR)/man/man5
#	$(INSTALL) udhcpd.conf.5 $(USRSHAREDIR)/man/man5
#	mkdir -p $(USRSHAREDIR)/man/man8
#	$(INSTALL) udhcpc.8 udhcpd.8 $(USRSHAREDIR)/man/man8

clean:
	-rm -f udhcpd udhcpc dumpleases *.o core

