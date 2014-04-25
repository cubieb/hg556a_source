CFLAGS = -Wall -s -Os -fomit-frame-pointer


##support POTS_PINMNGR
CFLAGS+= -DLY_DEBUG

OBJS = web-tool.o

dynamic: web-tool install

web-tool: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o web-tool
	
install:
	install -m 755 web-tool $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/web-tool	

	
clean:
	$(RM) *.o web-tool
