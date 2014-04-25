#
# testbox Makefile
#
# Copyright 2005, Huawei Corporation
# All Rights Reserved.
# 
#
# $Id: Makefile,v 1.1 2008/10/05 10:32:04 l65130 Exp $
#
OBJS := diagd.o dump.o

CFLAGS += -I. 
#-I ../shared 

CFLAGS	+= -s -O2

#LDFLAGS += -L$(APP_COMMON_DIR)/syscomm -lbhalapi -L$(INSTALL_DIR)/lib/ -lnvram -L$(INSTALL_DIR)/lib/ -latshared


dynamic: all


clean:
	rm -f *.o *~ diagd


%:%.c
	$(CC) $(CFLAGS) -o $@ $^ #$(LDFLAGS)

all: diagd
	install -D diagd $(INSTALL_DIR)/bin/diagd
	$(STRIP) $(INSTALL_DIR)/bin/diagd

diagd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

	
