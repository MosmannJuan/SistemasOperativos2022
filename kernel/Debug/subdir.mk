################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../conexiones.c \
../instrucciones_handler.c \
../kernel.c \
../utils.c 

OBJS += \
./conexiones.o \
./instrucciones_handler.o \
./kernel.o \
./utils.o 

C_DEPS += \
./conexiones.d \
./instrucciones_handler.d \
./kernel.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


