
ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set)
endif

include $(DEVKITPRO)/gba_rules

TARGET := pokemon_world
BUILD := build
SOURCES := source
DATA := data
GRAPHICS := graphics

ARCH := -mthumb -mthumb-interwork

CFLAGS := -g -Wall -O2 $(ARCH) \
          -ffunction-sections -fdata-sections

LDFLAGS := -g $(ARCH) \
           -Wl,--gc-sections \
           -Wl,-Map,$(TARGET).map

LIBS := -lgba

.PHONY: all clean

all: $(TARGET).gba

clean:
	rm -rf $(BUILD) $(TARGET).gba $(TARGET).elf $(TARGET).map
