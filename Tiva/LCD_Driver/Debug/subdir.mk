################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lcdbpv2.c \
../pic.c 

OBJS += \
./lcdbpv2.o \
./pic.o 

C_DEPS += \
./lcdbpv2.d \
./pic.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -std=c99 -Dgcc -DARM_MATH_CM -DCLASS_IS_TM4C123 -DPART_TM4C123GH6PM -DTARGET_IS_TM4C123_RA1 -UPART_LM4F120H5QR -I/home/paul/Documents/Project/tivaware -I/home/paul/Documents/Project/tivaware/examples/boards/dk-tm4c123g/ -Os -g3 -Wall -c -fmessage-length=0 -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


