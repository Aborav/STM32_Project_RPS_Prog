################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/aborav/Seagate\ Expansion\ Drive/1.Laptop/1.Programming/2.STM32/Libraries_M/STM32_Library_Encoder/M_ENC.c 

OBJS += \
./Libraries_M/STM32_Library_Encoder/M_ENC.o 

C_DEPS += \
./Libraries_M/STM32_Library_Encoder/M_ENC.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries_M/STM32_Library_Encoder/M_ENC.o: /media/aborav/Seagate\ Expansion\ Drive/1.Laptop/1.Programming/2.STM32/Libraries_M/STM32_Library_Encoder/M_ENC.c Libraries_M/STM32_Library_Encoder/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"/media/aborav/Seagate Expansion Drive/1.Laptop/1.Programming/2.STM32/Libraries_M" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries_M-2f-STM32_Library_Encoder

clean-Libraries_M-2f-STM32_Library_Encoder:
	-$(RM) ./Libraries_M/STM32_Library_Encoder/M_ENC.cyclo ./Libraries_M/STM32_Library_Encoder/M_ENC.d ./Libraries_M/STM32_Library_Encoder/M_ENC.o ./Libraries_M/STM32_Library_Encoder/M_ENC.su

.PHONY: clean-Libraries_M-2f-STM32_Library_Encoder

