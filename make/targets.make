# make targets for nesemu2

.PHONY: all clean distclean

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -f $(TRASHFILES)
	rm -rf $(TRASHDIRS)

source/system/win32/nesemu2-res.o: source/system/win32/nesemu2.rc
	$(RC) $(RCFLAGS) $^ $@

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

