# Makefile.in for kernel module

SHELL = /bin/sh
INSTALL = /usr/bin/install -c
mkdir_p = mkdir -p
VERSION = 2.7.3

DISTFILES = Makefile.in configure.ac configure config.h.in ../install-sh \
	dev.c dir.c file.c inode.c fuse_i.h fuse_kernel.h control.c

fusemoduledir = /lib/modules/2.6.18-1.2798.fc6/kernel/fs/fuse

fusemodule := fuse.ko

all: all-n
install: install-n
uninstall: uninstall-n

all-n:
install-n:
uninstall-n:

all-y: all-spec

install-y: all
	$(mkdir_p) $(DESTDIR)$(fusemoduledir)
	$(INSTALL) -m 644 $(fusemodule) $(DESTDIR)$(fusemoduledir)/$(fusemodule)
	-/sbin/depmod -a

uninstall-y:
	rm -f $(DESTDIR)$(fusemoduledir)/$(fusemodule)
	-/sbin/depmod -a

clean:
	-rm -f $(fusemodule) *.o .*.cmd *.mod.c *.ko *.s */*.o

distclean: clean
	rm -f Makefile
	rm -f config.h config.log config.status config.cache
	rm -rf .tmp_versions

maintainer-clean: distclean

distdir: $(DISTFILES)
	cp -p $(DISTFILES) $(distdir)

EXTRA_CFLAGS += -DFUSE_VERSION=\"$(VERSION)\"

obj-m := fuse.o
fuse-objs := dev.o dir.o file.o inode.o control.o

all-spec:
	$(MAKE) -C /usr/src/kernels/2.6.18-1.2798.fc6-i586 SUBDIRS=`pwd`  modules
