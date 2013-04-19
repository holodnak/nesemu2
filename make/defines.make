# defines for nesemu2

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

# compiler/linker flags
CFLAGS = $(CFLAGS_$(BUILD)) $(DEFINES) -I$(PATH_SOURCE)
LDFLAGS = $(LDFLAGS_$(BUILD)) $(LIBS)
