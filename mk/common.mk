# host compilation tools
HOST_GCC = gcc

# set the tools path
XTOOLS_DIR = $(TOP)/xtools

# library paths
X_LIBGCC_DIR = $(XTOOLS_DIR)/lib/gcc/arm-none-eabi/9.2.1/thumb/v7e-m+fp/hard
X_LIBC_DIR = $(XTOOLS_DIR)/arm-none-eabi/lib/thumb/v7e-m+fp/hard
X_SPECS_DIR =$(XTOOLS_DIR)/arm-none-eabi/lib/thumb/v7e-m+fp/hard

# tools
X_GCC = $(XTOOLS_DIR)/bin/arm-none-eabi-gcc
X_OBJCOPY = $(XTOOLS_DIR)/bin/arm-none-eabi-objcopy
X_AR = $(XTOOLS_DIR)/bin/arm-none-eabi-ar
X_LD = $(XTOOLS_DIR)/bin/arm-none-eabi-ld
X_GDB = $(XTOOLS_DIR)/bin/arm-none-eabi-gdb
