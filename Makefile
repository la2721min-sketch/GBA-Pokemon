
ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set)
endif

include $(DEVKITPRO)/devkitARM/gba_rules

TARGET := pokemon_world
BUILD := build
SOURCES := source
INCLUDES := $(DEVKITPRO)/libgba/include

ARCH := -march=armv4t -mtune=arm7tdmi -mthumb

CFLAGS := -g -Wall -Wextra -O2 $(ARCH) \
          -ffunction-sections -fdata-sections \
          $(foreach dir,$(INCLUDES),-I$(dir))

LDFLAGS := -g $(ARCH) -Wl,--gc-sections
LIBS := -lgba
LIBPATHS := -L$(DEVKITPRO)/libgba/lib

# Use ARM compiler for linking, not Linux system ld
export LD := $(CC)

.PHONY: all clean

all: $(TARGET).gba

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba $(TARGET).map
