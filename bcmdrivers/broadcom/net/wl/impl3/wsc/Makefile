PROGRAM_NAME:= wsccmd

all:
	cp $(PROGRAM_NAME)_save $(PROGRAM_NAME)

install:
	install -m 755 $(PROGRAM_NAME) $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/$(PROGRAM_NAME)
	install -m 444 wsc_config_1a_ap.txt  $(INSTALL_DIR)/etc

dynamic: all install

static: install

clean:
	rm -f $(PROGRAM_NAME)	
