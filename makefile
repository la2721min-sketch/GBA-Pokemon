
ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set)
endif

include $(DEVKITPRO)/devkitARM/base_rules

TARGET := pokemon_world
BUILD := build
SOURCES := source

ARCH := -mthumb -mthumb-interwork

CFLAGS := -g -Wall -O2 $(ARCH) \
          -ffunction-sections -fdata-sections \
          -I$(DEVKITPRO)/libgba/include

LDFLAGS := -specs=gba.specs $(ARCH) \
           -Wl,--gc-sections \
           -L$(DEVKITPRO)/libgba/lib

LIBS := -lgba

.PHONY: all clean

all: $(TARGET).gba

$(TARGET).elf: $(SOURCES)/main.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBS) -o $@

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba
