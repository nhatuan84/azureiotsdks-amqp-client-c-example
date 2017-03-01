################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../iothub_client_sample_amqp.c 

OBJS += \
./iothub_client_sample_amqp.o 

C_DEPS += \
./iothub_client_sample_amqp.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/tuan/workspace/iot-client/azure_c_shared_utility" -I"/home/tuan/workspace/iot-client/azure_uamqp_c" -I"/home/tuan/workspace/iot-client/inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


