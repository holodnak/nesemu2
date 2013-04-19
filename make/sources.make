# sources.make - source files for building nesemu2

# path of source files
PATH_SOURCE=source

# misc source files
SOURCE_CONFIG = source/config/config.c
SOURCE_LOG = source/log/buffer.c source/log/log.c
SOURCE_EMU = source/emu/emu.c

# search mapper directory for source files
MAPPER_DIRS=$(shell find $(PATH_SOURCE)/mappers -type d)
SOURCE_MAPPERS=$(foreach dir,$(MAPPER_DIRS),$(wildcard $(dir)/*.c))

# input devices
SOURCE_INPUTDEV = source/inputdev/inputdev.c source/inputdev/null.c
SOURCE_INPUTDEV += source/inputdev/joypad0.c

# nes core
SOURCE_NES = source/nes/memory.c source/nes/nes.c source/nes/io.c
SOURCE_NES += source/nes/cart/cart.c source/nes/cart/ines.c source/nes/cart/ines20.c
SOURCE_NES += source/nes/cpu/cpu.c source/nes/cpu/disassemble.c
SOURCE_NES += source/nes/ppu/io.c source/nes/ppu/ppu.c source/nes/ppu/step.c
SOURCE_NES += source/nes/ppu/tilecache.c source/nes/ppu/attribcache.c

# palette
SOURCE_PALETTE = source/palette/generator.c source/palette/palette.c

# system files
SOURCE_SYSTEM_SDL = source/system/sdl/video.c source/system/sdl/input.c source/system/sdl/sound.c
SOURCE_SYSTEM_SDL += source/system/sdl/main.c source/system/sdl/system.c source/system/sdl/win32/SDL_win32_main.c
SOURCE_SYSTEM = $(SOURCE_SYSTEM_SDL)

# build list of source files
SOURCES = $(SOURCE_CONFIG) $(SOURCE_LOG) $(SOURCE_EMU) $(SOURCE_MAPPERS) $(SOURCE_INPUTDEV) $(SOURCE_NES) $(SOURCE_PALETTE) $(SOURCE_SYSTEM)

# extra files to remove
TRASHFILES = nesemu2.log stdout.txt stderr.txt
TRASHFILES += projects/codeblocks/nesemu2.depend projects/codeblocks/nesemu2.layout
TRASHFILES += projects/vc2003/nesemu2.ncb

# extra directories to remove
TRASHDIRS = projects/codeblocks/bin projects/codeblocks/obj
TRASHDIRS += projects/vc2003/Debug projects/vc2003/Release
TRASHDIRS += projects/vc2010/Debug projects/vc2010/Release projects/vc2010/ipch

# generate object files
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

# target executable filename
TARGET = nesemu2.exe
