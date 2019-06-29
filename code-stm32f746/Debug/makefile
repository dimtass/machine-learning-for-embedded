################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Drivers/STM32F7xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
code-stm32f746.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
code-stm32f746.list \


# All Target
all: code-stm32f746.elf secondary-outputs

# Tool invocations
code-stm32f746.elf: $(OBJS) $(USER_OBJS) /home/dimtass/STM32CubeIDE/workspace_1.0.1/code-stm32f746/STM32F746NGHX_FLASH.ld
	arm-none-eabi-gcc -o "code-stm32f746.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"/home/dimtass/STM32CubeIDE/workspace_1.0.1/code-stm32f746/STM32F746NGHX_FLASH.ld" --specs=nosys.specs -Wl,-Map="code-stm32f746.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

code-stm32f746.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "code-stm32f746.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
