PROGRAM_NAME:= lld2d

all:
	cp $(PROGRAM_NAME)_save $(PROGRAM_NAME)

install:
	install -m 755 $(PROGRAM_NAME) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)
	install -m 444 ./wrt54g.large.ico $(INSTALL_DIR)/etc
	install -m 444 ./wrt54g.small.ico $(INSTALL_DIR)/etc

dynamic: all install

static: install

clean:
	rm -f $(PROGRAM_NAME)	
