# Makefile for hlefds

TARGET = hlefds.bin
SOURCE = hlefds.s
OBJECT = $(patsubst %.s,%.o,$(SOURCE))

AS = wla-6502
LD = wlalink

ASFLAGS =
LDFLAGS = -d linkfile

$(TARGET): $(OBJECT)
	$(LD) $(LDFLAGS) $@

all: $(TARGET)

clean:
	rm -f $(OBJECT) $(TARGET)

.SUFFIXES: .s

.s.o:
	$(AS) -$(ASFLAGS)o $< $@

.PHONY: all clean
