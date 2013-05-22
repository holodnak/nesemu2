# build configuration for nesemu2

# use undocumented cpu opcodes
USE_CPU_UNDOC ?= 1

# use quick sprite code
USE_QUICK_SPRITES ?= 1

# use accurate sprite 0 hit (use with quick sprites only)
USE_ACCURATE_SPRITE0 ?= 1

# build type
BUILD ?= RELEASE

# os target default to autodetect
OSTARGET ?= UNKNOWN
