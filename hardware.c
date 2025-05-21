#include "stdio.h"
#include "hardware.h"

int memory[MEMORY_SIZE] = {0}; // 0-1023 stores instructions. 
                         // 1024-2047 stores data.

int memoryInstructionCounter = 0; //counter for storing instructions in memory
int memoryDataCounter = 1024; //counter for storig data in memory

int reg_array[REGISTER_COUNT]; // Index 0: the zero register R0.
                               // Indices 1-31: general purpose registers R1-R31.
                               // Index 32: PC register.

int R0 = 0; //variable to store the values stored in R0 so we don't overwrite it.

