DESTDIR=
SBINDIR=$(INSTALL_DIR)/bin
CONFDIR=$(INSTALL_DIR)/etc
DOCDIR=

# Path to parent kernel include files directory
KERNEL_INCLUDE:=$(KERNEL_DIR)/include
LIBC_INCLUDE=$(KERNEL_DIR)/include/linux

# Path to db_185.h include
#DBM_INCLUDE:=/usr/include

#DEFINES= -DRESOLVE_HOSTNAMES

#options if you have a bind>=4.9.4 libresolv (or, maybe, glibc)
LDLIBS=
ADDLIB=

#options for decnet
#ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
#ADDLIB+=ipx_ntop.o ipx_pton.o

ifeq ($(LIBC_INCLUDE)/socketbits.h,$(wildcard $(LIBC_INCLUDE)/socketbits.h))
  ifeq ($(LIBC_INCLUDE)/net/if_packet.h,$(wildcard $(LIBC_INCLUDE)/net/if_packet.h))
    GLIBCFIX=-I../include-glibc -include ../include-glibc/glibc-bugs.h
  endif
endif
ifeq ($(LIBC_INCLUDE)/bits/socket.h,$(wildcard $(LIBC_INCLUDE)/bits/socket.h))
  GLIBCFIX=-I../include-glibc -I/usr/include/db3 -include ../include-glibc/glibc-bugs.h
endif

#CC = gcc
CCOPTS = -D_GNU_SOURCE -s -Os -Wstrict-prototypes -Wall -g
CFLAGS = $(CCOPTS) $(GLIBCFIX) -I$(KERNEL_INCLUDE) -I../include -I../glibc-include $(DEFINES)

LDLIBS += -L../lib -lnetlink -lipr2util -lm -ldl

SUBDIRS=lib ip tc misc

LIBNETLINK=../lib/libnetlink.a ../lib/libipr2util.a

all: Config
	@for i in $(SUBDIRS); \
	do $(MAKE) $(MFLAGS) -C $$i; done

Config:
	./configure $(KERNEL_INCLUDE)

install: all
	install -m 0755 tc/tc $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/tc
	install -m 0755 ip/ip $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/ip
	install -m 0755 misc/ss $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/ss
	install -m 0755 misc/nstat $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/nstat
	install -m 0755 misc/ifstat $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/ifstat
	install -m 0755 misc/rtacct $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/rtacct
	install -m 0755 misc/rtstat $(DESTDIR)$(SBINDIR)
	$(STRIP) $(INSTALL_DIR)/bin/rtstat

dynamic: install

clean:
	@for i in $(SUBDIRS) doc; \
	do if [ -f Config ]; then $(MAKE) $(MFLAGS) -C $$i clean; fi; done

clobber: clean
	rm -f Config


.EXPORT_ALL_VARIABLES:
