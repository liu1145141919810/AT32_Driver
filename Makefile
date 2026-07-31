PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc

FW_LIB = AT32F423_Firmware_Library_V2.1.0

BUILD_DIR = build
TARGET := $(BUILD_DIR)/rtthread.elf

INCLUDES = -I$(FW_LIB)/libraries/cmsis/cm4/core_support
INCLUDES += -I$(FW_LIB)/libraries/cmsis/cm4/device_support
INCLUDES += -I$(FW_LIB)/libraries/drivers/inc
INCLUDES += -I.
#INCLUDES += -I$(FW_LIB)/project/at32f423_board
INCLUDES += -I./MCU_code/inc
INCLUDES += -I$(FW_LIB)/middlewares/freertos/source/include                         #add2
INCLUDES += -I$(FW_LIB)/middlewares/freertos/source/portable/GCC/ARM_CM4F             #add3

ASM_SRCS = $(FW_LIB)/libraries/cmsis/cm4/device_support/startup/gcc/startup_at32f423.s
SYS_SRCS = $(FW_LIB)/libraries/cmsis/cm4/device_support/system_at32f423.c
#BSP_SRCS = $(FW_LIB)/project/at32f423_board/at32f423_board.c

OBJS = $(BUILD_DIR)/main.o
OBJS += $(BUILD_DIR)/system_at32f423.o
OBJS += $(BUILD_DIR)/startup_at32f423.o
#OBJS += $(BUILD_DIR)/at32f423_board.o
OBJS += $(BUILD_DIR)/at32f423_crm.o
OBJS += $(BUILD_DIR)/at32f423_gpio.o
OBJS += $(BUILD_DIR)/at32f423_usart.o
OBJS += $(BUILD_DIR)/at32f423_misc.o
OBJS += $(BUILD_DIR)/at32f423_ertc.o
OBJS += $(BUILD_DIR)/at32f423_pwc.o
OBJS += $(BUILD_DIR)/at32f423_dma.o
OBJS += $(BUILD_DIR)/at32f423_adc.o
OBJS += $(BUILD_DIR)/at32f423_tmr.o
OBJS += $(BUILD_DIR)/at32f423_can.o
OBJS += $(BUILD_DIR)/FSM.o
OBJS += $(BUILD_DIR)/init.o
OBJS += $(BUILD_DIR)/RTOS_Tasks.o
OBJS += $(BUILD_DIR)/utility.o

# FreeRTOS add3
FREERTOS_SRC = $(FW_LIB)/middlewares/freertos/source
FREERTOS_OBJS = $(BUILD_DIR)/tasks.o \
				$(BUILD_DIR)/queue.o \
				$(BUILD_DIR)/list.o \
				$(BUILD_DIR)/timers.o \
				$(BUILD_DIR)/event_groups.o \
				$(BUILD_DIR)/port.o \
				$(BUILD_DIR)/heap_4.o
OBJS += $(FREERTOS_OBJS)

CFLAGS  = -mcpu=cortex-m4 -mthumb
CFLAGS += -DAT32F423RCT7
CFLAGS += -DAT_START_F423_V1
CFLAGS += -g3 -Og
CFLAGS += -mfloat-abi=softfp -mfpu=fpv4-sp-d16 # add1
CFLAGS += $(INCLUDES)

LDFLAGS = -specs=nosys.specs
LDFLAGS += -specs=nano.specs -lc -lnosys
LDFLAGS += -T$(FW_LIB)/libraries/cmsis/cm4/device_support/startup/gcc/linker/AT32F423xC_FLASH.ld

all: $(TARGET)

$(BUILD_DIR)/%.o: MCU_code/src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/system_at32f423.o: $(SYS_SRCS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup_at32f423.o: $(ASM_SRCS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FW_LIB)/libraries/drivers/src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# FreeRTOS kernel sources  add4
$(BUILD_DIR)/tasks.o: $(FREERTOS_SRC)/tasks.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/queue.o: $(FREERTOS_SRC)/queue.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/list.o: $(FREERTOS_SRC)/list.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/timers.o: $(FREERTOS_SRC)/timers.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/event_groups.o: $(FREERTOS_SRC)/event_groups.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/stream_buffer.o: $(FREERTOS_SRC)/stream_buffer.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/croutine.o: $(FREERTOS_SRC)/croutine.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/port.o: $(FREERTOS_SRC)/portable/GCC/ARM_CM4F/port.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/heap_4.o: $(FREERTOS_SRC)/portable/memmang/heap_4.c
	mkdir -p $(BUILD_DIR) && $(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	arm-none-eabi-size $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

## 继续：需要对...纠错
#.elf文件：是编译后的完整可执行文件，包含烧录到芯片所需的一切信息。
