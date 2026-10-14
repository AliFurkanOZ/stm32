################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/bms_state.c \
../App/Src/modbus_crc.c \
../App/Src/modbus_handlers.c \
../App/Src/modbus_rtu.c \
../App/Src/register_map.c 

OBJS += \
./App/Src/bms_state.o \
./App/Src/modbus_crc.o \
./App/Src/modbus_handlers.o \
./App/Src/modbus_rtu.o \
./App/Src/register_map.o 

C_DEPS += \
./App/Src/bms_state.d \
./App/Src/modbus_crc.d \
./App/Src/modbus_handlers.d \
./App/Src/modbus_rtu.d \
./App/Src/register_map.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su App/Src/%.cyclo: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB55xx -c -I../Core/Inc -I"C:/Users/onure/OneDrive/Desktop/STM project/modbus123/stm32/App/Inc" -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/bms_state.cyclo ./App/Src/bms_state.d ./App/Src/bms_state.o ./App/Src/bms_state.su ./App/Src/modbus_crc.cyclo ./App/Src/modbus_crc.d ./App/Src/modbus_crc.o ./App/Src/modbus_crc.su ./App/Src/modbus_handlers.cyclo ./App/Src/modbus_handlers.d ./App/Src/modbus_handlers.o ./App/Src/modbus_handlers.su ./App/Src/modbus_rtu.cyclo ./App/Src/modbus_rtu.d ./App/Src/modbus_rtu.o ./App/Src/modbus_rtu.su ./App/Src/register_map.cyclo ./App/Src/register_map.d ./App/Src/register_map.o ./App/Src/register_map.su

.PHONY: clean-App-2f-Src

