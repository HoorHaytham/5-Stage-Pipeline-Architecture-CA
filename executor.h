// executor.h
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "datapath.h"

extern int dropFlagCount;

typedef struct{
    int instruction; // stores output of fetch 
    decodedInstruction d1; // stores output of decode cycle 1
    decodedInstruction d2; // stores output of decode cycle 2
    int opcode; // stores output of execute cycle 1
    mem_reg_destinationInfo output; //stores output of execute cycle 2
    mem_reg_destinationInfo mem; // stores output of memory
    bool wb; // stores output of writeback
    int count; // stores count of execution cycle 
    bool droped; // true if the instruction has been dropped
} storeOutput;

const char* instrTypeToStr(InstrType type);

// Function to start pipeline execution
void pipelineExecution();

#endif // EXECUTOR_H
