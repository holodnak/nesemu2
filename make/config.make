# build configuration for nesemu2

# use attribute cache
USE_CACHE_ATTRIB ?= 0

# use assembly rendering code
USE_ASM_RENDER ?= 0

# use undocumented cpu opcodes
USE_CPU_UNDOC ?= 0

# use quick sprite code
USE_QUICK_SPRITES ?= 1

# use accurate sprite 0 hit (use with quick sprites only)
USE_ACCURATE_SPRITE0 ?= 1

# build type
BUILD ?= RELEASE

# os target
OSTARGET ?= WIN32
