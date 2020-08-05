TOP = ../../..
include $(TOP)/mk/common.mk

OUTPUT = zforth

# hal sources
HAL_DIR = $(TOP)/src/soc/st/stm32f4/hal/src
SRC += $(HAL_DIR)/stm32f4xx_hal.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc_ex.c \
	$(HAL_DIR)/stm32f4xx_hal_cortex.c \
	$(HAL_DIR)/stm32f4xx_hal_gpio.c \
	$(TOP)/src/soc/st/stm32f4/system_stm32f4xx.c \

# soc library sources
LIB_DIR = $(TOP)/src/soc/st/stm32f4/lib
SRC += $(LIB_DIR)/gpio.c \
	$(LIB_DIR)/delay.c \
	$(LIB_DIR)/usart.c \

#	$(LIB_DIR)/i2c.c \
#	$(LIB_DIR)/i2s.c \
#	$(LIB_DIR)/dma.c \
#	$(LIB_DIR)/adc.c \
#	$(LIB_DIR)/rng.c \
#	$(LIB_DIR)/spi.c \

# target sources
TARGET_DIR = $(TOP)/src/target/mb997
SRC += $(TARGET_DIR)/main.c \

# common
COMMON_DIR = $(TOP)/src/common
SRC += $(COMMON_DIR)/logging.c \
	$(COMMON_DIR)/debounce.c \
	$(COMMON_DIR)/syscalls.c \
	$(COMMON_DIR)/rand.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT_printf.c \

# zforth
ZFORTH_DIR = $(TOP)/src/zforth
SRC += $(ZFORTH_DIR)/src/zforth/zforth.c \

OBJ = $(patsubst %.c, %.o, $(SRC))
OBJ += $(TARGET_DIR)/start.o

# include files

# include paths
INCLUDE += -I$(TOP)/src/soc/st/stm32f4/lib
INCLUDE += -I$(TOP)/src/soc/st/stm32f4/hal/inc
INCLUDE += -I$(TOP)/src/soc/st/stm32f4/cmsis
INCLUDE += -I$(TARGET_DIR)
INCLUDE += -I$(COMMON_DIR)
INCLUDE += -I$(COMMON_DIR)/rtt
INCLUDE += -I$(ZFORTH_DIR)/src/zforth

# defines
DEFINE = -DSTM32F407xx
DEFINE += -DSTDIO_SERIAL

# compiler flags
X_CFLAGS = -Werror -Wall -Wextra -Wstrict-prototypes
X_CFLAGS += --specs=$(X_SPECS_DIR)/nano.specs
X_CFLAGS += -O2
X_CFLAGS += -falign-functions -fomit-frame-pointer -fno-strict-aliasing
#X_CFLAGS += -mlittle-endian -mthumb-interwork
#X_CFLAGS += -mfpu=fpv4-sp-d16
X_CFLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=hard
X_CFLAGS += -std=c99

# linker flags
LDSCRIPT = stm32f407vg_flash.ld
X_LDFLAGS = -T$(LDSCRIPT)
X_LDFLAGS += -Wl,-Map,$(OUTPUT).map
X_LDFLAGS += -Wl,--gc-sections
X_LDFLAGS += -Wl,-u,_printf_float
X_LDFLAGS += -Wl,-u,_scanf_float

.S.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@
.c.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(OBJ)
	$(X_GCC) $(X_CFLAGS) $(X_LDFLAGS) $(OBJ) -o $(OUTPUT)
	$(X_OBJCOPY) -O binary $(OUTPUT) $(OUTPUT).bin

clean:
	-rm $(OBJ)	
	-rm $(OUTPUT)
	-rm $(OUTPUT).map	
	-rm $(OUTPUT).bin	
