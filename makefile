ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set)
endif

include $(DEVKITPRO)/devkitARM/3ds_rules

TARGET := pokemon_world
BUILD := build
SOURCES := source

ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS := -g -Wall -O2 -mword-relocations \
          -ffunction-sections -fdata-sections \
          $(ARCH) -D__3DS__ \
          -I$(DEVKITPRO)/libctru/include \
          -I$(DEVKITPRO)/citro3d/include \
          -I$(DEVKITPRO)/citro2d/include

LDFLAGS := -specs=3dsx.specs -g $(ARCH) \
           -L$(DEVKITPRO)/libctru/lib \
           -L$(DEVKITPRO)/citro3d/lib \
           -L$(DEVKITPRO)/citro2d/lib

LIBS := -lcitro2d -lcitro3d -lctru -lm

.PHONY: all clean

all: $(TARGET).3dsx

$(TARGET).3dsx: $(SOURCES)/main.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) $(LIBS) -o $(TARGET).elf
	3dsxtool $(TARGET).elf $@

clean:
	rm -f $(TARGET).elf $(TARGET).3dsx
