# Makefile for AVR projects (C)
TARGET = main
MCU = atmega328p
F_CPU = 1000000
FORMAT = ihex

# Directories
SRCDIR = src
OBJDIR = build

# All source files in the src directory
SRC_FILES = $(wildcard $(SRCDIR)/*.c)
# Corresponding object files in the build directory
OBJ_FILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_FILES))

# Compiler/Linker Programs
CC = avr-gcc
HOST_CC = gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f

# Fuse bytes (internal 8MHz RC + CKDIV8 → 1MHz, matches F_CPU=1000000)
LFUSE = 0x62
HFUSE = 0xD9
EFUSE = 0xFF
REMOVEDIR = rm -rf

# Options
OPT = s
DEBUG = dwarf-2
CSTANDARD = -std=gnu99
CDEFS = -DF_CPU=$(F_CPU)UL

CFLAGS = -g$(DEBUG) $(CDEFS) -O$(OPT) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes $(CSTANDARD)
LDFLAGS = -Wl,-Map=$(OBJDIR)/$(TARGET).map,--cref -lm

# Build Flags
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS)

all: build size

build: $(OBJDIR)/$(TARGET).hex $(OBJDIR)/$(TARGET).elf $(OBJDIR)/$(TARGET).lss $(OBJDIR)/$(TARGET).sym

$(OBJDIR)/$(TARGET).hex: $(OBJDIR)/$(TARGET).elf
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock $< $@

$(OBJDIR)/$(TARGET).elf: $(OBJ_FILES)
	@mkdir -p $(OBJDIR)
	@echo Linking: $@
	@$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "Compiling C": $<
	@$(CC) -c $(ALL_CFLAGS) $< -o $@

$(OBJDIR)/$(TARGET).lss: $(OBJDIR)/$(TARGET).elf
	@$(OBJDUMP) -h -S -z $< > $@

$(OBJDIR)/$(TARGET).sym: $(OBJDIR)/$(TARGET).elf
	@$(NM) -n $< > $@

size: $(OBJDIR)/$(TARGET).elf
	@echo
	@$(SIZE) --mcu=$(MCU) --format=avr $(OBJDIR)/$(TARGET).elf

test:
	@mkdir -p build
	@$(HOST_CC) -Wall -Isrc src/counter_logic.c tests/test_counter.c -o build/test_runner
	@./build/test_runner

program: $(OBJDIR)/$(TARGET).hex
	$(AVRDUDE) -v -p $(MCU) -c usbasp -P usb -B 5.33 -U flash:w:$<

fuse:
	$(AVRDUDE) -p $(MCU) -c usbasp -P usb -B 5.33 \
		-U lfuse:w:$(LFUSE):m \
		-U hfuse:w:$(HFUSE):m \
		-U efuse:w:$(EFUSE):m

clean:
	$(REMOVEDIR) build build

.PHONY: all build size clean program fuse test
