# make targets for nesemu2

.PHONY: all clean distclean

all: $(TARGET)

resources: hlefds nsfbios

hlefds:
	+$(MAKE) -C ./resources/bios/hlefds/

nsfbios:
	+$(MAKE) -C ./resources/bios/nsf/

install: all resources
	@echo Installing nesemu2 into $(INSTALLPATH)
	cp $(TARGET) $(INSTALLPATH)
	@echo Installing common data into $(DATAPATH)
	mkdir -p $(DATAPATH)/bios $(DATAPATH)/palette $(DATAPATH)/xml
	cp $(BIOSFILES) $(DATAPATH)/bios
	cp $(PALETTEFILES) $(DATAPATH)/palette
	cp $(XMLFILES) $(DATAPATH)/xml

uninstall:
	@echo Removing nesemu2 from $(INSTALLPATH) and nesemu2 data from $(DATAPATH)
	rm -f $(INSTALLPATH)/$(TARGET)
	rm -rf $(DATAPATH)

clean:
	rm -f $(OBJECTS) $(TARGET)
	+$(MAKE) -C ./resources/bios/hlefds/ clean
	+$(MAKE) -C ./resources/bios/nsf/ clean

distclean: clean
	rm -f $(TRASHFILES)
	rm -rf $(TRASHDIRS)

# the win32 resource file
source/system/win32/nesemu2-res.o: source/system/win32/nesemu2.rc
	$(RC) $(RCFLAGS) $^ $@

# osx objective c
source/system/sdl/osx/%.o: source/system/sdl/osx/%.m
	$(CC) $(CFLAGS) -c $< -o $@

# output executable
$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)
