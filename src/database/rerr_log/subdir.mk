################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/database/rerr_log/rerr_log.c 

OBJS += \
./src/database/rerr_log/rerr_log.o 

C_DEPS += \
./src/database/rerr_log/rerr_log.d 


# Each subdirectory must supply rules for building sources it contributes
src/database/rerr_log/%.o: ../src/database/rerr_log/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -DHASH_DEBUG=1 -Wall -c -fmessage-length=0 -DHASH_FUNCTION=HASH_BER -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


