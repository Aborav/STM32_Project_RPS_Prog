################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Usr\ libs/STM32_Library_Flashprom/M_Flashprom.c 

OBJS += \
./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.o 

C_DEPS += \
./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.d 


# Each subdirectory must supply rules for building sources it contributes
Usr\ libs/STM32_Library_Flashprom/M_Flashprom.o: ../Usr\ libs/STM32_Library_Flashprom/M_Flashprom.c Usr\ libs/STM32_Library_Flashprom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"/media/aborav/Seagate Expansion Drive/1.Laptop/1.Programming/2.STM32/Projects/stm32g431cbu6_dac_tl494/Usr libs" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Usr libs/STM32_Library_Flashprom/M_Flashprom.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Usr-20-libs-2f-STM32_Library_Flashprom

clean-Usr-20-libs-2f-STM32_Library_Flashprom:
	-$(RM) ./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.cyclo ./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.d ./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.o ./Usr\ libs/STM32_Library_Flashprom/M_Flashprom.su

.PHONY: clean-Usr-20-libs-2f-STM32_Library_Flashprom

