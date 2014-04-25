###################################################################################
#
#    Copyright 2000  Broadcom Corporation
#    All Rights Reserved
#    No portions of this material may be reproduced in any form without the
#    written permission of:
#             Broadcom Corporation
#             16215 Alton Parkway
#             P.O. Box 57013
#             Irvine, California 92619-7013
#    All information contained in this document is Broadcom Corporation
#    company private, proprietary, and trade secret.
#
###################################################################################

obj-m := haushost.o
haushost-objs := haushost-lib.a

HAUSLIB_SUBDIR   =  lib
HAUSLIB_NAME     =  libhausware

$(obj)/haushost-lib.a:
	echo $(src);
	echo $(HAUSLIB_SUBDIR);
	cp -f $(XCHANGE_DIR)/dslx/lib/LinuxKernel/$(XCHANGE_BUILD_APP)/$(HAUSLIB_NAME).a $(src)/$(HAUSLIB_SUBDIR)/$(HAUSLIB_NAME).lib;

	cd	$(src)/$(HAUSLIB_SUBDIR);									\
	$(AR) x $(HAUSLIB_NAME).lib;

	$(LD) -G 0 -r -s $(src)/$(HAUSLIB_SUBDIR)/*.o -o $(obj)/haushost-lib.a
	
