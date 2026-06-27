# Makefile для raster_console_viewer
SILENT ?= 1
ifeq (1,$(SILENT))
.SILENT:
endif

TARGET = application
DEBUG = 0
OPT = -Og
OPT += -g3

# Build paths
BUILD_DIR = build
BUILD_ASM_DIR = $(BUILD_DIR)/asm
BUILD_APP_DIR = $(BUILD_DIR)/app
BUILD_CC_DIR  = $(BUILD_DIR)/ccc
BUILD_CPP_DIR = $(BUILD_DIR)/cpp

# Source directories
SRC_DIRS  = raster_viewer
INC_DIRS  = raster_viewer

# Find source files
C_SOURCES   = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.c'))
CPP_SOURCES = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.cpp'))
ASM_SOURCES = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.s'))
C_INC       = $(shell find $(INC_DIRS) -type f \( -name '*.h' -o -name '*.hpp' \) -exec dirname {} \; | sort -u)

C_INCLUDES = $(addprefix -I,$(C_INC))

# Toolchain
PREFIX =
CC  = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS  = $(PREFIX)gcc -x assembler-with-cpp
CP  = $(PREFIX)objcopy
SZ  = $(PREFIX)size

CPU = -m64
MCU = $(CPU)

# Compile flags
WARNINGS := -Wall -Wextra -Wshadow -Wundef
GCCFLAGS += -O0 -g $(WARNINGS)
CFLAGS_STD = -c -Os -w -std=gnu17 $(GCCFLAGS)
CXXFLAGS_STD = -c -Os -w -std=gnu++17 $(GCCFLAGS)
CFLAGS = $(MCU) $(C_INCLUDES) $(OPT) $(CFLAGS_STD)
CPPFLAGS = $(MCU) $(C_INCLUDES) $(OPT) $(CXXFLAGS_STD)

# Libraries
LIBS  = -lc -lm
LDFLAGS += $(LIBS)
LDFLAGS += -Wl,--start-group
LDFLAGS += -lgcc
LDFLAGS += -lstdc++
LDFLAGS += -Wl,--end-group

all: $(BUILD_APP_DIR)/$(TARGET).elf

green=\033[0;32m
YELLOW=\033[1;33m
NC=\033[0m

OBJECTS = $(addprefix $(BUILD_CC_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_CPP_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
OBJECTS += $(addprefix $(BUILD_ASM_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_CC_DIR)/%.o: %.c Makefile | $(BUILD_CC_DIR)
	@echo " ${green}[compile:]${YELLOW} $< ${NC}"
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CC_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_CPP_DIR)/%.o: %.cpp Makefile | $(BUILD_CPP_DIR)
	@echo " ${green}[compile:]${YELLOW} $< ${NC}"
	$(CXX) -c $(CPPFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CPP_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_ASM_DIR)/%.o: %.s Makefile | $(BUILD_ASM_DIR)
	@echo " ${green}[compile:]${YELLOW} $< ${NC}"
	$(AS) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_ASM_DIR)/$(notdir $(<:.s=.lst)) $< -o $@

$(BUILD_APP_DIR)/$(TARGET).elf: $(OBJECTS) Makefile | $(BUILD_APP_DIR)
	@echo " ${green}[linking:]${YELLOW} $@ ${NC}"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@ --format=Berkeley
	cp $(BUILD_APP_DIR)/$(TARGET).elf ./
	./$(BUILD_APP_DIR)/$(TARGET).elf

$(BUILD_CC_DIR):
	mkdir -p $@
$(BUILD_CPP_DIR):
	mkdir -p $@
$(BUILD_APP_DIR):
	mkdir -p $@
$(BUILD_ASM_DIR):
	mkdir -p $@

clean:
	-rm -fR $(BUILD_DIR)
	-rm -f $(TARGET).elf

-include $(wildcard $(BUILD_DIR)/*.d)
