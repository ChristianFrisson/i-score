################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/basic_c_strings.c \
../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/commonsyms.c \
../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_byteorder.c \
../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_database.c \
../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/mouse.c 

OBJS += \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/basic_c_strings.o \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/commonsyms.o \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_byteorder.o \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_database.o \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/mouse.o 

C_DEPS += \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/basic_c_strings.d \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/commonsyms.d \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_byteorder.d \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/ext_database.d \
./Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/mouse.d 


# Each subdirectory must supply rules for building sources it contributes
Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/%.o: ../Acousmosynth/ACOUSMOSYNTH2.app/Contents/Frameworks/MaxAPI.framework/Versions/Current/Headers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


