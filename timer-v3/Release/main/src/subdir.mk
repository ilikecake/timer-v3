################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main/src/NHD28.c \
../main/src/cdc_desc.c \
../main/src/cdc_vcom.c \
../main/src/command.c \
../main/src/commands.c \
../main/src/cr_startup_lpc11xx.c \
../main/src/dfu.c \
../main/src/ds3232m.c \
../main/src/hardware.c \
../main/src/i2c.c \
../main/src/main.c \
../main/src/sysinit.c 

OBJS += \
./main/src/NHD28.o \
./main/src/cdc_desc.o \
./main/src/cdc_vcom.o \
./main/src/command.o \
./main/src/commands.o \
./main/src/cr_startup_lpc11xx.o \
./main/src/dfu.o \
./main/src/ds3232m.o \
./main/src/hardware.o \
./main/src/i2c.o \
./main/src/main.o \
./main/src/sysinit.o 

C_DEPS += \
./main/src/NHD28.d \
./main/src/cdc_desc.d \
./main/src/cdc_vcom.d \
./main/src/command.d \
./main/src/commands.d \
./main/src/cr_startup_lpc11xx.d \
./main/src/dfu.d \
./main/src/ds3232m.d \
./main/src/hardware.d \
./main/src/i2c.d \
./main/src/main.d \
./main/src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
main/src/%.o: ../main/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DNDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M0 -I"D:\Pat\Documents\GitHub\timer-v3\timer-v3\main\inc" -I"D:\Pat\Documents\GitHub\timer-v3\timer-v3\freertos\inc" -I"D:\Pat\Documents\GitHub\timer-v3\lpc_chip_11uxx_lib\inc" -I"D:\Pat\Documents\GitHub\timer-v3\lpc_chip_11uxx_lib\inc\usbd" -I"D:\Pat\Documents\GitHub\timer-v3\nxp_lpcxpresso_11u14_board_lib\inc" -Os -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


