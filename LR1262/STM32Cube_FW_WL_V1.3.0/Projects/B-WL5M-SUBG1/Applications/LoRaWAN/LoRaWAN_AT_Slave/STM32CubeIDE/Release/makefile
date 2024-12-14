################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Utilities/subdir.mk
-include Middlewares/SubGHz_Phy/subdir.mk
-include Middlewares/LoRaWAN/subdir.mk
-include Drivers/STM32WLxx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Application/User/Startup/subdir.mk
-include Application/User/LoRaWAN/Target/subdir.mk
-include Application/User/LoRaWAN/App/subdir.mk
-include Application/User/Core/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := LoRaWAN_AT_Slave
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
LoRaWAN_AT_Slave.elf \

MAP_FILES += \
LoRaWAN_AT_Slave.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
LoRaWAN_AT_Slave.list \


# All Target
all: main-build

# Main-build Target
main-build: LoRaWAN_AT_Slave.elf secondary-outputs

# Tool invocations
LoRaWAN_AT_Slave.elf LoRaWAN_AT_Slave.map: $(OBJS) $(USER_OBJS) C:\Users\User\OneDrive\Documents\buoy-stm32\LR1262\STM32Cube_FW_WL_V1.3.0\Projects\B-WL5M-SUBG1\Applications\LoRaWAN\LoRaWAN_AT_Slave\STM32CubeIDE\STM32WL5MOCHX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "LoRaWAN_AT_Slave.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\User\OneDrive\Documents\buoy-stm32\LR1262\STM32Cube_FW_WL_V1.3.0\Projects\B-WL5M-SUBG1\Applications\LoRaWAN\LoRaWAN_AT_Slave\STM32CubeIDE\STM32WL5MOCHX_FLASH.ld" --specs=nosys.specs -Wl,-Map="LoRaWAN_AT_Slave.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

LoRaWAN_AT_Slave.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "LoRaWAN_AT_Slave.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) LoRaWAN_AT_Slave.elf LoRaWAN_AT_Slave.list LoRaWAN_AT_Slave.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
