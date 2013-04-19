# make targets for nesemu2

.PHONY: all clean distclean

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -f $(TRASHFILES)
	rm -rf $(TRASHDIRS)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
