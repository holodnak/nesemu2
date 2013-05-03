# defines for nesemu2

# detect os
OSTARGET = UNKNOWN
ifeq ($(OS),Windows_NT)
	OSTARGET = WIN32
	DEFINES = -DNO_STDIO_REDIRECT
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME),Linux)
		OSTARGET = LINUX
	endif
	ifeq ($(UNAME),Darwin)
		OSTARGET = OSX
	endif
endif
ifeq ($(OSTARGET),UNKNOWN)
	OSTARGET = LINUX
endif

# setup defines
ifeq ($(USE_CPU_UNDOC),1)
	DEFINES += -DCPU_UNDOC
endif
ifeq ($(USE_ASM_RENDER),1)
	DEFINES += -DASM_RENDER
endif
ifeq ($(USE_CACHE_ATTRIB),1)
	DEFINES += -DCACHE_ATTRIB
endif
ifeq ($(USE_QUICK_SPRITES),1)
	DEFINES += -DQUICK_SPRITES
endif
ifeq ($(USE_ACCURATE_SPRITE0),1)
	DEFINES += -DACCURATE_SPRITE0
endif

# compiler/linker programs
CC = gcc
LD = gcc

# compiler/linker flags for debug
CFLAGS_DEBUG = -g
LDFLAGS_DEBUG = 

# compiler/linker flags for release
CFLAGS_RELEASE = -O3 -funroll-loops -fomit-frame-pointer
LDFLAGS_RELEASE = -s

# libraries for linking
LIBS = -lSDL

# compiler/linker flags
CFLAGS = $(CFLAGS_$(BUILD)) $(DEFINES) -I$(PATH_SOURCE) -D$(OSTARGET) -D$(BUILD)
LDFLAGS = $(LDFLAGS_$(BUILD)) $(LIBS)

# resource compiler (win32 only)
RC = windres
RCFLAGS = --input-format=rc --output-format=coff
