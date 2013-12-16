################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include main/src/subdir.mk
-include freertos/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: timer-v3.axf

# Tool invocations
timer-v3.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"D:\Pat\Documents\GitHub\timer-v3\nxp_lpcxpresso_11u14_board_lib\Debug" -L"D:\Pat\Documents\GitHub\timer-v3\lpc_chip_11uxx_lib\Debug" -L"D:\Pat\Documents\GitHub\timer-v3\lpc_chip_11uxx_lib\libs" -Xlinker -Map="timer-v3.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "timer-v3_Debug.ld" -o "timer-v3.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) timer-v3.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "timer-v3.axf";  arm-none-eabi-objcopy -O binary "timer-v3.axf" "timer-v3.bin" ; checksum -p LPC11U37_401 -d "timer-v3.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
