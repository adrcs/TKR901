################################################################################
# Automatically-generated file. Do not edit!
################################################################################

RM := rm -rf

CC = gcc

C_SRCS = \
./src/frqtbl.c \
./src/main.c \
./src/doTable.c 

OBJS = \
./obj/frqtbl.o \
./obj/main.o \
./obj/doTable.o 

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: mb1501

# Tool invocations
mb1501: $(OBJS) include/mb1501.h makefile
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	$(CC) -o "mb1501" $(OBJS) 
	@echo 'Finished building target: $@'
	@echo ' '


# Each subdirectory must supply rules for building sources it contributes
obj/%.o: src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -I"./include" -O0 -g3 -Wall -c -fmessage-length=0 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Other Targets

clean:
	-$(RM) mb1501
	-$(RM) obj/*.o
	-@echo ' '
