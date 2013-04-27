# make targets for nesemu2

.PHONY: all clean distclean

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -f $(TRASHFILES)
	rm -rf $(TRASHDIRS)

# the win32 resource file
source/system/win32/nesemu2-res.o: source/system/win32/nesemu2.rc
	$(RC) $(RCFLAGS) $^ $@

# osx objective c
source/system/osx/%.o: source/system/osx/%.m
	$(CC) $(CFLAGS) -c $< -o $@

# output executable
$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
