
# host compilation tools
HOST_GCC = gcc

# set the tools path
XTOOLS_DIR = $(TOP)/xtools

# library paths
X_LIBGCC_DIR = $(XTOOLS_DIR)/lib/gcc/arm-none-eabi/9.2.1/arm/v5te/hard
X_LIBC_DIR = $(XTOOLS_DIR)/arm-none-eabi/lib/arm/v5te/hard

# tools
X_GCC = $(XTOOLS_DIR)/bin/arm-none-eabi-gcc
X_OBJCOPY = $(XTOOLS_DIR)/bin/arm-none-eabi-objcopy
X_AR = $(XTOOLS_DIR)/bin/arm-none-eabi-ar
X_LD = $(XTOOLS_DIR)/bin/arm-none-eabi-ld
X_GDB = $(XTOOLS_DIR)/bin/arm-none-eabi-gdb

# cross compile flags for cortex m4 build
X_CFLAGS = -Werror -Wall -Wextra -Wstrict-prototypes
X_CFLAGS += -O2
X_CFLAGS += -falign-functions -fomit-frame-pointer -fno-strict-aliasing
X_CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
X_CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
X_CFLAGS += -std=c99
