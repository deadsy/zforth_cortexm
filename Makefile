TOP = .

DL_DIR = $(TOP)/dl
SRC_DIR = $(TOP)/src
XTOOLS_DIR = $(TOP)/xtools

TARGET ?= mb997

TARGET_DIR = $(SRC_DIR)/target/$(TARGET)
BIN_FILE = $(TARGET_DIR)/zforth.bin

# cross compilation tools for ARM cortex-m
GCC_RELEASE = 9-2019q4
GCC_VERSION = 9-2019-q4-major
GCC_FILE = gcc-arm-none-eabi-$(GCC_VERSION)-x86_64-linux.tar.bz2
GCC_TBZ = $(DL_DIR)/$(GCC_FILE)
GCC_URL = https://developer.arm.com/-/media/Files/downloads/gnu-rm/$(GCC_RELEASE)/$(GCC_FILE)

PATCHFILES := $(sort $(wildcard patches/*.patch ))

PATCH_CMD = \
  for f in $(PATCHFILES); do\
      echo $$f ":"; \
      patch -b -p1 < $$f || exit 1; \
  done

COPY_CMD = tar cf - -C files . | tar xf - -C $(SRC_DIR)

.PHONY: all
all: .stamp_xtools .stamp_src
	make -C $(TARGET_DIR) $@

.PHONY: clean
clean:
	make -C $(TARGET_DIR) $@

.PHONY: clobber
clobber: clean
	-rm -rf $(XTOOLS_DIR) .stamp_xtools

.PHONY: program
program: 
	st-flash write $(BIN_FILE) 0x08000000

.PHONY: format
format: 
	./tools/cfmt.py

$(GCC_TBZ):
	mkdir -p $(DL_DIR)
	wget $(GCC_URL) -O $(GCC_TBZ)

.stamp_xtools: $(GCC_TBZ)
	-rm -rf $(XTOOLS_DIR)
	mkdir -p $(XTOOLS_DIR)
	tar -C $(XTOOLS_DIR) -jxf $(GCC_TBZ) --strip-components 1
	touch $@

.stamp_src: $(ZFORTH_TGZ)
	#$(COPY_CMD)
	#$(PATCH_CMD)
	touch $@
