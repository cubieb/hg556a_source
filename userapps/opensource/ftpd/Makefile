O_TARGET := bftpd.o 
# Generated automatically from Makefile.in by configure.
VERSION=2.2.1
#CC=gcc
#INSTALL=/usr/bin/install -c
CFLAGS+=-g -O2 -DHAVE_CONFIG_H -Wall -I. -I$(INC_BRCMCFM_PATH) -DVERSION=\"$(VERSION)\" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
        -I$(INC_APP_PATH) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
LIBS= -lcrypt
prefix=/usr
mandir=${prefix}/man
sbindir=${exec_prefix}/sbin
DESTDIR=
DIRPAX=
PAX=

HEADERS=bftpdutmp.h commands.h commands_admin.h cwd.h dirlist.h list.h login.h logging.h main.h mystring.h options.h targzip.h mypaths.h md5.h md5_loc.h
OBJS=bftpdutmp.o commands.o commands_admin.o cwd.o dirlist.o list.o login.o logging.o main.o mystring.o options.o md5.o
SRCS=bftpdutmp.c commands.c commands_admin.c cwd.c dirlist.c list.c login.c logging.c main.c mystring.c options.c md5.c

OBJS2LINK=$(OBJS) $(PAX)
LDFLAGS=
all: dynamic

dynamic: $(OBJS)
	$(CC) $(CFLAGS) -c -o $(OBJS)
	$(LD) -allow-shlib-undefined -r -o bftpd.o $(OBJS)
obj-y := $(OBJS)\
		$(join $(subdir-y), $(patsubst %,/%.o,$(notdir $(subdir-y))))

#include $(TOPDIR)/Rules.make
include Rules.make