#
# Makefile for the linux cramfs routines.
#

obj-$(CONFIG_CRAMFS) += cramfs.o

#cramfs-objs := inode.o uncompress.o

# CONFIG_MIPS_BRCM Begin Broadcom changed code.

ifdef CONFIG_ZLIB_FS_INFLATE
cramfs-objs := inode.o uncompress.o
else
ifdef CONFIG_LZMA_FS_INFLATE
cramfs-objs := inode.o ../inflate_lzma/7zlzma.o ../inflate_lzma/LZMADecoder.o ../inflate_lzma/IInOutStreams.o
endif
endif

# CONFIG_MIPS_BRCM End Broadcom changed code.
