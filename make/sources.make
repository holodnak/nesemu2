# sources.make - source files for building nesemu2

# path of source files
PATH_SOURCE = source

# misc source files
SOURCE_MISC = source/misc/config.c source/misc/log.c source/misc/crc32.c
SOURCE_MISC += source/misc/memutil.c source/misc/vars.c source/misc/paths.c
SOURCE_MISC += source/misc/memfile.c source/misc/strutil.c source/misc/history.c
SOURCE_MISC += source/misc/slre/slre.c

# cartdb source files
SOURCE_CARTDB = source/cartdb/cartdb.c source/cartdb/parser.c
SOURCE_CARTDB += $(foreach dir,$(PATH_SOURCE)/cartdb/expat,$(wildcard $(dir)/*.c))

# emu source files
SOURCE_EMU = source/emu/emu.c source/emu/commands.c source/emu/events.c
SOURCE_EMU += source/emu/commands/general.c source/emu/commands/nes.c

# search mapper directory for source files
MAPPER_DIRS = $(shell find $(PATH_SOURCE)/mappers -type d)
SOURCE_MAPPERS = $(foreach dir,$(MAPPER_DIRS),$(wildcard $(dir)/*.c))

# search inputdev directory for source files
SOURCE_INPUTDEV = $(foreach dir,$(PATH_SOURCE)/inputdev,$(wildcard $(dir)/*.c))

# nes core
SOURCE_NES = source/nes/memory.c source/nes/nes.c source/nes/io.c source/nes/genie.c source/nes/region.c
SOURCE_NES += source/nes/cart/cart.c source/nes/cart/ines.c source/nes/cart/ines20.c
SOURCE_NES += source/nes/cart/unif.c source/nes/cart/fds.c source/nes/cart/nsf.c
SOURCE_NES += source/nes/cart/patch/patch.c source/nes/cart/patch/ips.c source/nes/cart/patch/ups.c
SOURCE_NES += source/nes/state/state.c source/nes/state/block.c
SOURCE_NES += source/nes/cpu/cpu.c source/nes/cpu/disassemble.c
SOURCE_NES += source/nes/ppu/io.c source/nes/ppu/ppu.c source/nes/ppu/step.c
SOURCE_NES += source/nes/ppu/tilecache.c
SOURCE_NES += source/nes/apu/apu.c source/nes/movie.c

# palette
SOURCE_PALETTE = source/palette/generator.c source/palette/palette.c

# sdl system files
SOURCE_SYSTEM_SDL = source/system/sdl/video.c source/system/sdl/input.c source/system/sdl/sound.c
SOURCE_SYSTEM_SDL += source/system/sdl/main.c source/system/sdl/system.c
SOURCE_SYSTEM_SDL += source/system/sdl/console/console.c source/system/sdl/console/font.c
SOURCE_SYSTEM_SDL += source/system/sdl/console/fontdata.c source/system/sdl/console/linebuffer.c

# win32 system files
SOURCE_SYSTEM_WIN32 = source/system/win32/video.c source/system/win32/input.c source/system/win32/sound.c
SOURCE_SYSTEM_WIN32 += source/system/win32/system.c source/system/win32/main.c source/system/win32/mainwnd.c
SOURCE_SYSTEM_WIN32 += $(foreach dir,$(PATH_SOURCE)/system/win32/dialogs,$(wildcard $(dir)/*.c))
SOURCE_SYSTEM_WIN32 += source/system/win32/nesemu2-res.o

# sdl/win32 system files
SOURCE_SYSTEM_SDL_WIN32 = source/system/sdl/win32/SDL_win32_main.c source/system/win32/nesemu2-res.o

# sdl/linux system files
SOURCE_SYSTEM_SDL_LINUX = source/system/linux/stricmp.c

# sdl/osx system files
SOURCE_SYSTEM_SDL_OSX = source/system/linux/stricmp.c source/system/sdl/osx/SDLMain.o

# raspberry pi system files
SOURCE_SYSTEM_PI = source/system/pi/main.c source/system/sdl/system.c source/system/sdl/input.c
SOURCE_SYSTEM_PI += source/system/pi/gpioinput.c
SOURCE_SYSTEM_PI += source/system/sdl/video.c source/system/sdl/sound.c
SOURCE_SYSTEM_PI += source/system/sdl/console/font.c source/system/sdl/console/fontdata.c

# common system files
SOURCE_SYSTEM_COMMON = source/system/common/filters.c
SOURCE_SYSTEM_COMMON += source/system/common/filters/draw/draw.c source/system/common/filters/interpolate/interpolate.c
SOURCE_SYSTEM_COMMON += source/system/common/filters/scale2x/scalebit.c source/system/common/filters/scale2x/scale2x.c
SOURCE_SYSTEM_COMMON += source/system/common/filters/scale2x/scale3x.c
SOURCE_SYSTEM_COMMON += source/system/common/filters/ntsc/ntsc.c

# build list of source files
SOURCES = $(SOURCE_MISC) $(SOURCE_EMU) $(SOURCE_MAPPERS) $(SOURCE_INPUTDEV) $(SOURCE_NES) $(SOURCE_PALETTE)
SOURCES += $(SOURCE_CARTDB)

# extra files to remove
TRASHFILES = nesemu2.log stdout.txt stderr.txt
TRASHFILES += projects/codeblocks/nesemu2.depend projects/codeblocks/nesemu2.layout
TRASHFILES += projects/vc2003/nesemu2.ncb projects/vc2010/nesemu2.sdf projects/vc2012/nesemu2.sdf

# extra directories to remove
TRASHDIRS = projects/codeblocks/bin projects/codeblocks/obj
TRASHDIRS += projects/vc2003/Debug projects/vc2003/Release
TRASHDIRS += projects/vc2010/Debug projects/vc2010/Release projects/vc2010/ipch
TRASHDIRS += projects/vc2012/Debug projects/vc2012/Release projects/vc2012/Profile projects/vc2012/ipch

# system stuff
ifeq ($(OSTARGET),WIN32)
	ifeq ($(USESDL),1)
		SOURCES += $(SOURCE_SYSTEM_SDL) $(SOURCE_SYSTEM_SDL_WIN32) $(SOURCE_SYSTEM_COMMON)
		LIBS += -lSDL
		TARGET = $(OUTPUT)-sdl.exe
	else
		SOURCES += $(SOURCE_SYSTEM_WIN32) $(SOURCE_SYSTEM_COMMON)
		LIBS += -lcomctl32 -lgdi32 -lcomdlg32 -lddraw -ldsound -ldxguid
		TARGET = $(OUTPUT)-win32.exe
	endif
endif

ifeq ($(OSTARGET),LINUX)
	SOURCES += $(SOURCE_SYSTEM_SDL) $(SOURCE_SYSTEM_SDL_LINUX) $(SOURCE_SYSTEM_COMMON)
	LIBS += -lSDL -lm
	TARGET = $(OUTPUT)
endif

ifeq ($(OSTARGET),OSX)
	SOURCES += $(SOURCE_SYSTEM_SDL) $(SOURCE_SYSTEM_SDL_OSX) $(SOURCE_SYSTEM_COMMON)
	CPPFLAGS += -I/usr/local/include
	LDFLAGS += -L/usr/local/lib
	LIBS += -lSDL -framework cocoa
	TARGET = $(OUTPUT)
endif

ifeq ($(OSTARGET),PI)
	SOURCES += $(SOURCE_SYSTEM_PI) $(SOURCE_SYSTEM_COMMON)
	LIBS += -lSDL -lm
	TARGET = $(OUTPUT)
endif

# generate object files
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

# output executable name
OUTPUT = nesemu2
