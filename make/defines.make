# defines for nesemu2

# detect os
ifeq ($(OSTARGET),UNKNOWN)
	ifeq ($(OS),Windows_NT)
		OSTARGET = WIN32
		USESDL ?= 1
		ifeq ($(USESDL),1)
			DEFINES = -DNO_STDIO_REDIRECT
		endif
	else
		UNAME = $(shell uname -s)
		ifeq ($(UNAME),Linux)
			USESDL = 1
			OSTARGET = LINUX
		endif
		ifeq ($(UNAME),Darwin)
			USESDL = 1
			OSTARGET = OSX
		endif
	endif
endif
ifeq ($(OSTARGET),UNKNOWN)
	OSTARGET = LINUX
endif

# setup defines
ifeq ($(USE_CPU_UNDOC),1)
	DEFINES += -DCPU_UNDOC
endif
ifeq ($(USE_QUICK_SPRITES),1)
	DEFINES += -DQUICK_SPRITES
endif
ifeq ($(USESDL),1)
	DEFINES += -DSDL
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
LIBS =

# compiler/linker flags
CPPFLAGS = $(CFLAGS_$(BUILD)) $(DEFINES) -I$(PATH_SOURCE) -D$(OSTARGET) -D$(BUILD)
LDFLAGS = $(LDFLAGS_$(BUILD))

# resource compiler (win32 only)
RC = windres
RCFLAGS = --input-format=rc --output-format=coff
