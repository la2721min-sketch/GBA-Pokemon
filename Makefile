ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is not set. Install devkitPro with devkitARM and libgba.)
endif

include $(DEVKITPRO)/devkitARM/gba_rules

TARGET      := pokemon_world
BUILD       := build
SOURCES     := source
INCLUDES    :=

ARCH        := -mthumb -mthumb-interwork

CFLAGS      := -g -Wall -Wextra -O2 $(ARCH) \
               -ffunction-sections -fdata-sections \
               $(INCLUDES)

LDFLAGS     := -g $(ARCH) -Wl,--gc-sections
LIBS        := -lgba

.PHONY: all clean

all: $(TARGET).gba

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba $(TARGET).map
