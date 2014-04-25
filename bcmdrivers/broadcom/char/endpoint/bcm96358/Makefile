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

obj-$(CONFIG_BCM_ENDPOINT) += endpointdd.o
endpointdd-objs := endptdrv.a gwcommon.a haushost.a
$(obj)/endptdrv.a: 
	make M=$(src)/endptdrv
	cp $(src)/endptdrv/endptdrv-lib.a $(src)/endptdrv.a
$(obj)/gwcommon.a: 
	make M=$(src)/gwcommon
	cp $(src)/gwcommon/gwcommon-lib.a $(src)/gwcommon.a
$(obj)/haushost.a: 
	make M=$(src)/haushost
	cp $(src)/haushost/haushost-lib.a $(src)/haushost.a

VOICE_CODE_ARCHIVE := dslx_common.tar.gz

XCHG_DSLX_COMMON_DIR=${XCHANGE_DIR}/dslx_common

DUMMY := $(shell	\
	if	[ -d $(XCHANGE_DIR) ]; then									\
		if	[ -d $(XCHG_DSLX_COMMON_DIR) ]; then					\
			echo "Voice-related source	code already unpacked";	\
		else																	\
			cd	$(XCHANGE_DIR);											\
			if	[ -e $(VOICE_CODE_ARCHIVE)	];	then					\
				echo "Untarring voice-related	source code...";	\
				tar xkfz	$(VOICE_CODE_ARCHIVE) 2>/dev/null;		\
			fi																	\
		fi																		\
	fi																			\
)


-include	$(TOPDIR)/Rules.make

clean:


