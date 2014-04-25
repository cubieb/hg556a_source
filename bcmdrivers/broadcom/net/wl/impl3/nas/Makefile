all:
	cp nas_save nas
	
install:
	install -m 755 nas $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/nas
	ln -sf ../bin/nas $(INSTALL_DIR)/bin/nas4not

dynamic: all install

clean:
	rm -f nas

