# make variables

# objects variables
OBJS_PSI := psiapi.o psiapp.o psiifc.o psiobject.o psiutil.o psidl.o psishm.o \
		board_api.o dbapi.o dbvalid.o dbobject.o bget.o fwsyscall.o
OBJS_PSI_XML := psixml.o nanoxml.o

# make default
LIB_PSI     := libpsi.so
LIB_PSI_XML := libpsixml.so
#Atheros 11n     
CXXFLAGS += -DSUPPORT_ATHEROSWLAN


ALL: libpsi libpsixml

libpsi: $(OBJS_PSI)
	$(LD) -shared -soname $(LIB_PSI) -o $(LIB_PSI) $(OBJS_PSI)

libpsixml: $(OBJS_PSI_XML)
	$(LD) -shared -soname $(LIB_PSI_XML) -o $(LIB_PSI_XML) $(OBJS_PSI_XML)

%.o: %.c
	$(CC) -g -fpic -c -DPROTOTYPES $(CFLAGS) -o $@ $<
	@ ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CFLAGS))),$$(strip $$(subst $$(comma),:,$$(CFLAGS))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

%.o: %.cpp
	$(CXX) -g -fpic -c -DPROTOTYPES $(CXXFLAGS) -I$(OPENSOURCE_DIR)/ftpd -o $@ $<
	@ ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CXXFLAGS))),$$(strip $$(subst $$(comma),:,$$(CXXFLAGS))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

board_api.o: board_api.c
	$(CC) -g -fpic -c -DUSE_ALL $(CFLAGS) -o $@ $<
	@ ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CFLAGS))),$$(strip $$(subst $$(comma),:,$$(CFLAGS))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

fwsyscall.o: $(OPENSOURCE_DIR)/ftpd/fwsyscall.c
	$(CC) -g -fpic -c $(CFLAGS) -I$(OPENSOURCE_DIR)/ftpd -o $@ $<
	@ ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CFLAGS))),$$(strip $$(subst $$(comma),:,$$(CFLAGS))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

.PHONY: clean

clean:
	@find . \( -name '*.[oasd]' -o -name core -o -name '.*.flags' -o -name '*.inc' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
	@rm -f $(LIB_PSI) $(LIB_PSI_XML)

