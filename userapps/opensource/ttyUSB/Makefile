CFLAGS = -Wall -s -Os -fomit-frame-pointer
CFLAGS += -I$(BROADCOM_CFM_DIR)/inc
LIBS+=-L$(BROADCOM_CFM_DIR)/util/psi -lpsi
CFLAGS+=-I$(BROADCOM_CFM_DIR)/inc
CFLAGS+=-I$(BROADCOM_TR69C_DIR)/inc

##support POTS_PINMNGR
CFLAGS+= -DPOTS_PINMNGR

ifeq ($(strip $(BRCM_BOARD_ID)), "HW556")
CFLAGS+= -DVDF_HG556
endif


OBJS = ttyUSB.o

dynamic: ttyUSB install

ttyUSB: $(OBJS)
	$(CC) -lpthread $(CFLAGS)  -Wl,--allow-shlib-undefined $(OBJS) -o ttyUSB $(LIBS)
	
install:
	install -m 755 ttyUSB $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/ttyUSB	

	
clean:
	$(RM) *.o ttyUSB
