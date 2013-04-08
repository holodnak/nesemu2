# Makefile for nesemu2

TARGET = nesemu2
OBJECTS = source/config/config.o source/config/defaults.o
OBJECTS += source/emu/emu.o
OBJECTS += source/log/buffer.o source/log/log.o
OBJECTS += source/mappers/ines.o source/mappers/ines20.o source/mappers/mappers.o source/mappers/unif.o
OBJECTS += source/mappers/boards/nintendo/nrom.o source/mappers/boards/nintendo/sxrom.o
OBJECTS += source/mappers/chips/c_mmc1.o
OBJECTS += source/nes/memory.o source/nes/nes.o
OBJECTS += source/nes/cart/cart.o source/nes/cart/ines.o source/nes/cart/ines20.o
OBJECTS += source/nes/cpu/cpu.o source/nes/cpu/disassemble.o
OBJECTS += source/nes/ppu/io.o source/nes/ppu/ppu.o source/nes/ppu/step.o
OBJECTS += source/palette/generator.o source/palette/palette.o
OBJECTS += source/system/sdl/video.o source/system/sdl/input.o source/system/sdl/sound.o
OBJECTS += source/system/sdl/main.o source/system/sdl/system.o source/system/sdl/win32/SDL_win32_main.o

SOURCE_PATH=source

CC=gcc
LD=gcc

CFLAGS=-g -I$(SOURCE_PATH)
LDFLAGS=-s -lSDL

#SOURCE_DIRS=$(shell find $(SOURCE_PATH) -type d)
#SOURCE_FILES=$(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.c))
#OBJECTS=$(patsubst %.c,%.o,$(SOURCE_FILES))

.PHONY: clean

all: $(TARGET)

clean:
	rm $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
