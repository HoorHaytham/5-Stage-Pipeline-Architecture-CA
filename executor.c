#include "stdio.h"
#include "stdbool.h"
#include "executor.h"
#include "datapath.h"
#include "hardware.h"

const char* instrTypeToStr(InstrType type) {
    switch (type) {
        case R_TYPE: return "R_TYPE";
        case I_TYPE: return "I_TYPE";
        case J_TYPE: return "J_TYPE";
        default: return "UNKNOWN";
    }
}

void pipelineExecution(){
    int clk = 1;
    flag = false;
    readFile();

    storeOutput instArray[1024];
    int instArrayPointer = 0;
    int stopCount = 0;

    do {
        printf("\n");
        printf("------------------- clk cycle = %d ---------------\n", clk);
        printf("\n");

        if(clk % 2 != 0){
            int fetchOutput = fetch();
            
            if(fetchOutput != -1){
                storeOutput tmp = {0};
                tmp.instruction = fetchOutput;
                tmp.count = 1;
                instArray[instArrayPointer++] = tmp;
            }

            if(flag){
                for(int j=0; j<instArrayPointer; j++){
                    storeOutput tmp2 = instArray[j];
                    if(flag && tmp2.count != 6 && tmp2.instruction != fetchOutput && tmp2.count != 7 && tmp2.droped == false){
                        printf("Instruction 0x%08X has been droped\n", tmp2.instruction);
                        stopCount++;
                        tmp2.droped = true;
                    }
                    instArray[j] = tmp2;
                }
        }

            for(int i=0; i<instArrayPointer; i++){
                storeOutput tmp2 = instArray[i];
                if(tmp2.count != 7 && tmp2.droped == false){
                    if(tmp2.count == 2){
                        printf("Instruction 0x%08X going through decode cycle 2\n", tmp2.instruction);
                        printf("    Decode cycle 2 input variable:\n    opcode = %d, r1 = %d, r2 = %d, r3 = %d, shamt = %d, immediate = %d, address = %d\n", tmp2.d1.opcode, tmp2.d1.r1, tmp2.d1.r2, tmp2.d1.r3, tmp2.d1.shamt, tmp2.d1.immediate, tmp2.d1.address);
                        tmp2.d2 = decode_cycle2(tmp2.d1);
                        printf("    Output: type = %s\n", instrTypeToStr(tmp2.d2.type));
                        tmp2.count++;
                    } else if(tmp2.count == 4){
                        printf("Instruction 0x%08X going through execute cycle 2\n", tmp2.instruction);
                        printf("    Execute cycle 2 input variable:\n    opcode = %d, r1 = %d, r2 = %d, r3 = %d, shamt = %d, immediate = %d, address = %d, type = %s\n", tmp2.d2.opcode, tmp2.d2.r1, tmp2.d2.r2, tmp2.d2.r3, tmp2.d2.shamt, tmp2.d2.immediate, tmp2.d2.address, instrTypeToStr(tmp2.d2.type));
                        tmp2.output = execute_cycle2(tmp2.opcode,tmp2.d2);
                        if(tmp2.output.type == 'R'){
                            printf("    Output: value %d will be written in register %d\n", tmp2.output.value, tmp2.output.index);
                        } else if(tmp2.output.type == 'M'){
                            printf("    Output: value %d will be written in memory at location %d\n", tmp2.output.value, tmp2.output.index);
                        } else{
                            printf("    Output: value to be written in register %d will be read from memory location %d\n", tmp2.output.index, tmp2.output.value);
                        }
                        if(flag){
                            dropFlagCount++;
                        }
                        tmp2.count++;
                    } else if(tmp2.count == 6){
                        printf("Instruction 0x%08X going through write back\n", tmp2.instruction);
                        if(tmp2.mem.type = 'R'){
                            printf("    Write back input: value %d will be written into register %d\n", tmp2.mem.value, tmp2.mem.index);
                        } else{
                            printf("    Write back input: nothing will be written into a register\n");
                        }
                        tmp2.wb = writeBack(tmp2.mem);
                        printf("    Instruction 0x%08X finished execution\n");
                        tmp2.count++;
                        stopCount++;
                    }
                    instArray[i] = tmp2;
                } 
            }

        } else{
            if(flag && dropFlagCount == 2){
                flag = false;
                dropFlagCount = 0;
            } else if(dropFlagCount == 1){
                dropFlagCount++;
            }
            for(int i=0; i<instArrayPointer; i++){
                storeOutput tmp2 = instArray[i];

                if(tmp2.count != 7 && tmp2.droped == false){
                    if(tmp2.count == 1){
                        printf("Instruction 0x%08X going through decode cycle 1\n", tmp2.instruction);
                        printf("    Decode cycle 1 input variable:  \n    instruction = 0x%08x\n", tmp2.instruction);
                        tmp2.d1 = decode_cycle1(tmp2.instruction);
                        tmp2.count = tmp2.count + 1;
                        printf("    Output variables: opcode = %d, r1 = %d, r2 = %d, r3 = %d, shamt = %d, immediate = %d, address = %d \n", tmp2.d1.opcode, tmp2.d1.r1, tmp2.d1.r2, tmp2.d1.r3, tmp2.d1.shamt, tmp2.d1.immediate, tmp2.d1.address);
                    } else if(tmp2.count == 3){
                        printf("Instruction 0x%08X going through execute cycle 1\n", tmp2.instruction);
                        printf("    Execute cycle 1 input variables: \n    type = %s\n", instrTypeToStr(tmp2.d2.type));
                        tmp2.opcode = execute_cycle1(tmp2.d2);
                        printf("    Output: opcode = %d\n", tmp2.opcode);
                        tmp2.count++;
                    } else if(tmp2.count == 5){
                        printf("Instruction 0x%08X going through memory read or write\n", tmp2.instruction);
                        if(tmp2.output.type == 'M'){
                            printf("    Memory read or write input:\n    value %d to be stored in memory at location %d\n", tmp2.output.value, tmp2.output.index);
                        } else if(tmp2.output.type == 'B'){
                            printf("    Memory read or write input:\n    value to be stored in register %d to be read from memory at location %d\n", tmp2.output.index , tmp2.output.value);
                        } else{
                            printf("    Memory read or write input: nothing will be read or written in memory\n");
                        }

                        tmp2.mem = MEM(tmp2.output);
                        tmp2.count++;
                    }
                } 

                instArray[i] = tmp2;
            }
        }
        
        clk++;
    } while (stopCount < instArrayPointer);
    
    printf("\n");
    printf("Pipeline finished execution at clk cycle %d\n", clk-1);
} 