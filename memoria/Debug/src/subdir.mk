################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/memoria.c \
../src/utils.c 

OBJS += \
./src/memoria.o \
./src/utils.o 

C_DEPS += \
./src/memoria.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
<<<<<<< HEAD
	gcc -I/home/utnso/tp-2022-1c-Operativ-sticos/shared-utils/src -include/home/utnso/tp-2022-1c-Operativ-sticos/shared-utils/src/sharedUtils.h -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
=======
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
>>>>>>> aa470c9093478bb0900aa42e02e4a4f4442a1569
	@echo 'Finished building: $<'
	@echo ' '


