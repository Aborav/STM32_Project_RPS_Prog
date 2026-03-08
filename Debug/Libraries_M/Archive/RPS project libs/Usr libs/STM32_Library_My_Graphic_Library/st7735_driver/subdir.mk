################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/media/aborav/Seagate\ Expansion\ Drive/1.Laptop/1.Programming/2.STM32/Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.c 

OBJS += \
./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.o 

C_DEPS += \
./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.o: /media/aborav/Seagate\ Expansion\ Drive/1.Laptop/1.Programming/2.STM32/Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.c Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"/media/aborav/Seagate Expansion Drive/1.Laptop/1.Programming/2.STM32/Libraries_M" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Libraries_M/Archive/RPS project libs/Usr libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries_M-2f-Archive-2f-RPS-20-project-20-libs-2f-Usr-20-libs-2f-STM32_Library_My_Graphic_Library-2f-st7735_driver

clean-Libraries_M-2f-Archive-2f-RPS-20-project-20-libs-2f-Usr-20-libs-2f-STM32_Library_My_Graphic_Library-2f-st7735_driver:
	-$(RM) ./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.cyclo ./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.d ./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.o ./Libraries_M/Archive/RPS\ project\ libs/Usr\ libs/STM32_Library_My_Graphic_Library/st7735_driver/st7735.su

.PHONY: clean-Libraries_M-2f-Archive-2f-RPS-20-project-20-libs-2f-Usr-20-libs-2f-STM32_Library_My_Graphic_Library-2f-st7735_driver

