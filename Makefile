STM32F4_LIB_DIR = /home/tyrotoxism/dev/embedded_dev/STM32CubeF4
CMSIS_DIR = $(STM32F4_LIB_DIR)/Drivers/CMSIS
STARTUP_FILE = $(HOME)/dev/embedded_dev/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s 
SYS_INIT_FILE = $(HOME)/dev/embedded_dev/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
BIN_FILE = $(OBJ_DIR)/ir_decoder.bin


# Compiler
CC = arm-none-eabi-gcc

CFLAGS = -DSTM32F446xx -mcpu=cortex-m4 -mthumb -DPRINTF_INCLUDE_CONFIG_H

LDFLAGS = -T $(STM32F4_LIB_DIR)/Projects/STM32446E-Nucleo/Templates/STM32CubeIDE/STM32F446RETX_FLASH.ld --specs=nosys.specs 

# Directories
OBJ_DIR = build

# Source files
SRCS = src/main.c src/pulse_measure.c lib/UART_driver/src/uart.c \
       lib/printf/printf.c $(SYS_INIT_FILE)

# Object files (manually listed)
OBJS = $(OBJ_DIR)/startup.o $(OBJ_DIR)/system_init.o $(OBJ_DIR)/main.o $(OBJ_DIR)/pulse_measure.o $(OBJ_DIR)/uart.o $(OBJ_DIR)/printf.o

# Include directories
INCLUDES = -Iinclude -Ilib/UART_driver/src -Ilib/printf \
           -I$(HOME)/dev/embedded_dev/STM32CubeF4/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
	   -I$(HOME)/dev/embedded_dev/STM32CubeF4/Drivers/CMSIS/Core/Include

# Main target
all: $(OBJ_DIR) ir_decoder

# Create build directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/startup.o: $(STARTUP_FILE)
	$(CC) -c $(STARTUP_FILE) -o $(OBJ_DIR)/startup.o $(CFLAGS) $(INCLUDES)

$(OBJ_DIR)/system_init.o: $(SYS_INIT_FILE)
	$(CC) -c $(SYS_INIT_FILE) -o $(OBJ_DIR)/system_init.o $(CFLAGS) $(INCLUDES)

# Compile each source file separately
$(OBJ_DIR)/main.o: src/main.c
	$(CC) -c src/main.c -o $(OBJ_DIR)/main.o $(CFLAGS) $(INCLUDES)

$(OBJ_DIR)/pulse_measure.o: src/pulse_measure.c
	$(CC) -c src/pulse_measure.c -o $(OBJ_DIR)/pulse_measure.o $(CFLAGS) $(INCLUDES)

$(OBJ_DIR)/uart.o: lib/UART_driver/src/uart.c
	$(CC) -c lib/UART_driver/src/uart.c -o $(OBJ_DIR)/uart.o $(CFLAGS) $(INCLUDES)

$(OBJ_DIR)/printf.o: lib/printf/printf.c
	$(CC) -c lib/printf/printf.c -o $(OBJ_DIR)/printf.o $(CFLAGS) $(INCLUDES)

# Link object files into final executable
ir_decoder: $(OBJS)
	$(CC) -o $(OBJ_DIR)/ir_decoder $(OBJS) $(LDFLAGS)

$(BIN_FILE): $(OBJ_DIR)/ir_decoder
	arm-none-eabi-objcopy -O binary $(OBJ_DIR)/ir_decoder $(BIN_FILE)

flash: $(BIN_FILE)
	st-flash write $(BIN_FILE) 0x8000000

# Clean up build files
clean:
	rm -rf $(OBJ_DIR)

