CFLAGS = -Wall -s -Os -fomit-frame-pointer

OBJS = scout.o

dynamic: scout install

scout: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o scout
	
install:
	install -m 755 scout $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/scout	

clean:
	$(RM) *.o scout
