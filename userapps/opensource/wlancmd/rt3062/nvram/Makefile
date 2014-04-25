PROGRAM_NAME:= wpsd

CFLAGS= -Wall -s -Os
LDFLAGS=-Os -Wl,-allow-shlib-undefined

CGLAGS_EXTRA =-fomit-frame-pointer

INC_LINUX=$(HOST_INCLUDE_DIR)

CFLAGS  += -Wall -I$(INC_DRIVER_VENDOR_DIR) -I../include -I$(INC_WIRELESS_DRIVER_PATH) -I$(INC_WIRELESS_DRIVER_PATH)/bcmcrypto -I$(INC_WIRELESS_DRIVER_PATH)/proto -I. -I$(INC_LINUX) -DDSLCPE

OBJS = gpio.o 

	
all:  wpsd

install: $(PROGRAM_NAME)
	install -m 755 $(PROGRAM_NAME) $(INSTALL_DIR)/bin	
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)

wpsd: $(OBJS)
	$(CC) -o wpsd $(CFLAGS) $(OBJS)

clean: 
	@rm -rf *.o;
	@rm -rf wpsd;	
	
