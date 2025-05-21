#ifndef HARDWARE_H
#define HARDWARE_H

// Memory size and register count constants
#define MEMORY_SIZE 2048
#define REGISTER_COUNT 33

// External memory and register declarations
extern int memory[MEMORY_SIZE];  // 0–1023: instructions, 1024–2047: data
extern int memoryInstructionCounter;  // starts at 0 till 1023: counter for storing instructions
extern int memoryDataCounter; //starts at 1024: counter for storing data 
extern int reg_array[REGISTER_COUNT];  // 0: R0, 1–31: R1–R31, 32: PC

#endif // HARDWARE_H
