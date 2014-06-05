#
#  Author Alejandro Santos
#  Copyright (c) 2013 extdigi, an APRS Digipeater
#  License: GNU GPL version 2 or later.
#
# Makefile based from:
#  https://github.com/gcds/AVR-devices-Makefile/blob/master/Makefile
#
##########################################################
# Project Settings
##########################################################
PROJECT_NAME = extdigi
OUTPUT_NAME = firmware
TARGET = atmega328p
TARGET2 = __AVR_ATmega328p__
PROGRAMMER_TARGET = m328
SYSTEM_CLOCK = 16000000L
FLASH_AFTER_COMPILE=FALSE
USE_BOOTLOADER=FALSE

##########################################################
# Compiler Options
##########################################################
OPTIMIZATION = s

##########################################################
# Programmer Options
##########################################################
PROGRAMMER = dragon_isp
PORGRMAMER_PORT = usb
PROGRAMMER_OPTIONS =

BOOTLOADER = avrisp
BOOTLOADER_PORT = /dev/tty.usbserial-
BOOTLOADER_OPTIONS = -b 115200

##########################################################
# Project Directories
##########################################################
BUILD_DIR = ./Build
# DRIVERS_DIR = ./Drivers
INCLUDES_DIR = ./include
INCLUDES2_DIR = ./arduino
INCLUDES3_DIR = ./bertos/cpu/avr
INCLUDES4_DIR = ./bertos
# LIBRARIES_DIR = ./Libraries
SOURCES_DIR = ./src

##########################################################
# Includes
##########################################################
# C Files
C_SRCS = $(wildcard *.c)
C_SRCS += $(wildcard $(SOURCES_DIR)/*.c)
C_SRCS += $(wildcard $(SOURCES_DIR)/*/*.c)
C_SRCS += $(wildcard $(SOURCES_DIR)/*/*/*.c)

# Header Files
INCLUDES = ./
INCLUDES += $(wildcard $(INCLUDES_DIR))
INCLUDES += $(wildcard $(INCLUDES2_DIR))
INCLUDES += $(wildcard $(INCLUDES3_DIR))
INCLUDES += $(wildcard $(INCLUDES4_DIR))
INCLUDES += $(wildcard $(SOURCES_DIR))

# Generate Object List
OBJECTS = $(C_SRCS:.c=.o)
# LD_OBJECTS = $(wildcard *.a) $(wildcard $(LIBRARIES_DIR)*.a) $(wildcard $(LIBRARIES_DIR)*/*.a) $(wildcard $(LIBRARIES_DIR)*/*/*.a) $(wildcard $(LIBRARIES_DIR)*.o) $(wildcard $(LIBRARIES_DIR)*/*.o) $(wildcard $(LIBRARIES_DIR)*/*/*.o)
LD_OBJECTS = 

##########################################################
# Tools Path
##########################################################
AVRDUDE = avrdude
AS = avr-gcc
GCC = avr-gcc
GPP = avr-g++
LD = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
REMOVE = rm -f

##########################################################
# Compiler FLAGS
##########################################################
GCC_FLAGS = -g -mmcu=$(TARGET) -O$(OPTIMIZATION) -D$(TARGET2) -DCPU_FREQ=$(SYSTEM_CLOCK) -DF_CPU=$(SYSTEM_CLOCK) -fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -Wall -Wstrict-prototypes -std=gnu99
GCC_FLAGS += $(patsubst %,-I%,$(INCLUDES)) -I.

AS_FLAGS = $(LISTING) -mmcu=$(TARGET) -D$(TARGET2) -DCPU_FREQ=$(SYSTEM_CLOCK) -DF_CPU=$(SYSTEM_CLOCK) -x assembler-with-cpp
AS_FLAGS += $(patsubst %,-I%,$(INCLUDES)) -I.

LD_FLAGS = -Wl,-Map,$(BUILD_DIR)/$(OUTPUT_NAME).map -mmcu=$(TARGET)
# Math Library
LD_FLAGS += -lm
# Minimalistic printf version
#LD_FLAGS += -Wl,-u,vfprintf -lprintf_min
# Floating point printf version (require math library above)
#LD_FLAGS += -Wl,-u,vfprintf -lprintf_flt

SIZE_FLAGS = -C --mcu=$(TARGET)

##########################################################
# Programmer FLAGS
##########################################################

PROGRAMMER_FLAGS = -p $(PROGRAMMER_TARGET) -c $(PROGRAMMER) -P $(PORGRMAMER_PORT) $(PROGRAMMER_OPTIONS)

BOOTLOADER_FLAGS = -p $(PROGRAMMER_TARGET) -c $(BOOTLOADER) -P $(BOOTLOADER_PORT) -F $(BOOTLOADER_OPTIONS)

ifeq ($(FLASH_AFTER_COMPILE),TRUE)
ifeq ($(USE_BOOTLOADER),TRUE)
all:: $(OUTPUT_NAME).hex flash_bootloader
else
all:: $(OUTPUT_NAME).hex flash_programmer
endif
else
all:: $(OUTPUT_NAME).hex
endif

$(OUTPUT_NAME).hex: $(OUTPUT_NAME).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(BUILD_DIR)/$(OUTPUT_NAME).elf $(BUILD_DIR)/$(OUTPUT_NAME).hex
	$(SIZE) $(SIZE_FLAGS) $(BUILD_DIR)/$(OUTPUT_NAME).elf

$(OUTPUT_NAME).elf: $(OBJECTS)
	$(LD) $(LD_FLAGS) $(LD_OBJECTS) $(OBJECTS) -o $(BUILD_DIR)/$(OUTPUT_NAME).elf

stats: $(OUTPUT_NAME).hex
	$(SIZE) $(SIZE_FLAGS) $(BUILD_DIR)/$(OUTPUT_NAME).elf

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(BUILD_DIR)/$(OUTPUT_NAME).hex
	$(REMOVE) $(BUILD_DIR)/$(OUTPUT_NAME).elf
	$(REMOVE) $(BUILD_DIR)/$(OUTPUT_NAME).map
	$(REMOVE) $(OBJECTS)

flash_programmer: $(OUTPUT_NAME).hex
	$(AVRDUDE) $(PROGRAMMER_FLAGS) -U flash:w:$(BUILD_DIR)/$(OUTPUT_NAME).hex

flash_bootloader: $(OUTPUT_NAME).hex
	$(AVRDUDE) $(BOOTLOADER_FLAGS) -U flash:w:$(BUILD_DIR)/$(OUTPUT_NAME).hex


##########################################################
# Default rules to compile .c and .cpp file to .o
# and assemble .S files to .o
##########################################################

.c.o :
	$(GCC) $(GCC_FLAGS) -c $< -o $(<:.c=.o)

.cpp.o :
	$(GPP) $(GPP_FLAGS) -c $< -o $(<:.cpp=.o)

.S.o :
	$(AS) $(AS_FLAGS) -c $< -o $(<:.S=.o)
