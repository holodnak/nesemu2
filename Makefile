# Makefile for nesemu2

SOURCE_CONFIG = source/config/config.c
SOURCE_LOG = source/log/buffer.c source/log/log.c
SOURCE_EMU = source/emu/emu.c
SOURCE_MAPPERS = source/mappers/ines.c source/mappers/ines20.c source/mappers/mappers.c source/mappers/unif.c
SOURCE_MAPPERS += source/mappers/boards/nintendo/nrom.c source/mappers/boards/nintendo/sxrom.c
SOURCE_MAPPERS += source/mappers/chips/c_mmc1.c
SOURCE_INPUTDEV = source/inputdev/inputdev.c source/inputdev/null.c
SOURCE_INPUTDEV += source/inputdev/joypad0.c
SOURCE_NES = source/nes/memory.c source/nes/nes.c source/nes/io.c
SOURCE_NES += source/nes/cart/cart.c source/nes/cart/ines.c source/nes/cart/ines20.c
SOURCE_NES += source/nes/cpu/cpu.c source/nes/cpu/disassemble.c
SOURCE_NES += source/nes/ppu/io.c source/nes/ppu/ppu.c source/nes/ppu/step.c
SOURCE_NES += source/nes/ppu/tilecache.c source/nes/ppu/attribcache.c
SOURCE_PALETTE = source/palette/generator.c source/palette/palette.c
SOURCE_SYSTEM_SDL = source/system/sdl/video.c source/system/sdl/input.c source/system/sdl/sound.c
SOURCE_SYSTEM_SDL += source/system/sdl/main.c source/system/sdl/system.c source/system/sdl/win32/SDL_win32_main.c

SOURCES = $(SOURCE_CONFIG) $(SOURCE_LOG) $(SOURCE_EMU) $(SOURCE_MAPPERS) $(SOURCE_INPUTDEV) $(SOURCE_NES) $(SOURCE_PALETTE) $(SOURCE_SYSTEM_SDL)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
TARGET = nesemu2.exe

TRASHFILES = nesemu2.log stdout.txt stderr.txt
TRASHFILES += projects/codeblocks/nesemu2.depend projects/codeblocks/nesemu2.layout
TRASHFILES += projects/vc2003/nesemu2.suo projects/vc2003/nesemu2.ncb
TRASHDIRS = projects/codeblocks/bin projects/codeblocks/obj
TRASHDIRS += projects/vc2003/Debug projects/vc2003/Release
TRASHDIRS += projects/vc2010/Debug projects/vc2010/Release

CC=gcc
LD=gcc

CFLAGS=-g -Isource
LDFLAGS=-lSDL

#SOURCE_PATH=source
#SOURCE_DIRS=$(shell find $(SOURCE_PATH) -type d)
#SOURCE_FILES=$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.c))
#OBJECTS=$(patsubst %.c,%.o,$(SOURCE_FILES))

.PHONY: clean distclean

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -f $(TRASHFILES)
	rm -rf $(TRASHDIRS)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
