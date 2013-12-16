################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../example/src/NHD28.c \
../example/src/cdc_desc.c \
../example/src/cdc_vcom.c \
../example/src/command.c \
../example/src/commands.c \
../example/src/cr_startup_lpc11xx.c \
../example/src/dfu.c \
../example/src/ds3232m.c \
../example/src/hardware.c \
../example/src/i2c.c \
../example/src/main.c \
../example/src/sysinit.c 

OBJS += \
./example/src/NHD28.o \
./example/src/cdc_desc.o \
./example/src/cdc_vcom.o \
./example/src/command.o \
./example/src/commands.o \
./example/src/cr_startup_lpc11xx.o \
./example/src/dfu.o \
./example/src/ds3232m.o \
./example/src/hardware.o \
./example/src/i2c.o \
./example/src/main.o \
./example/src/sysinit.o 

C_DEPS += \
./example/src/NHD28.d \
./example/src/cdc_desc.d \
./example/src/cdc_vcom.d \
./example/src/command.d \
./example/src/commands.d \
./example/src/cr_startup_lpc11xx.d \
./example/src/dfu.d \
./example/src/ds3232m.d \
./example/src/hardware.d \
./example/src/i2c.d \
./example/src/main.d \
./example/src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
example/src/%.o: ../example/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M0 -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_freertos_blinky3\example\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_freertos_blinky3\freertos\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\lpc_chip_11uxx_lib\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\lpc_chip_11uxx_lib\inc\usbd" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_board_lib\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

example/src/cr_startup_lpc11xx.o: ../example/src/cr_startup_lpc11xx.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M0 -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_freertos_blinky3\example\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_freertos_blinky3\freertos\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\lpc_chip_11uxx_lib\inc" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\lpc_chip_11uxx_lib\inc\usbd" -I"D:\projects\timer\lpcopen_v2_00a_lpcxpresso_nxp_lpcxpresso_11u14\nxp_lpcxpresso_11u14_board_lib\inc" -Os -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"example/src/cr_startup_lpc11xx.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


