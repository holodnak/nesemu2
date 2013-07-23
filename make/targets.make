# make targets for nesemu2

.PHONY: all clean distclean

all: $(TARGET) hlefds nsfbios

hlefds:
	+$(MAKE) -C ./resources/bios/hlefds/

nsfbios:
	+$(MAKE) -C ./resources/bios/nsf/

install: all
	echo Installing for user into $(INSTALLPATH)
	mkdir -p $(INSTALLPATH) $(INSTALLPATH)/bios $(INSTALLPATH)/palette
	cp $(BIOSFILES) $(INSTALLPATH)/bios
	cp $(PALETTEFILES) $(INSTALLPATH)/palette
	cp $(XMLFILES) $(INSTALLPATH)

clean:
	rm -f $(OBJECTS) $(TARGET)

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
