#include "stdio.h"
#include "stdbool.h"
#include "datapath.h"
#include "hardware.h"
#include <stdint.h>  // for int64_t, INT32_MAX, INT32_MIN
#include <limits.h>  // for INT32_MAX, INT32_MIN



bool flag = false;
int dropFlagCount = 0;
bool carry_flag = false;
bool overflow_flag = false;

// remove dropped flag you dont need it 5alas


int sign_extend(int value, int bits) {
    int mask = 1 << (bits - 1);         // Gets the sign bit (e.g. bit 17 for 18-bit)
    return (value ^ mask) - mask;       // Applies two’s complement extension
}

int fetch(){
    int pc = reg_array[32];

    if(pc<0 || pc>1023 || pc >= memoryInstructionCounter){ //check that pc's value is valid
        return -1;
    }

    int instruction = memory[pc];

    printf("Fetched instruction at PC=%d: 0x%08X\n\n", pc, memory[pc]);

    pc++;
    reg_array[32] = pc;
    

    return instruction;
}

decodedInstruction decode_cycle1(int instruction){
    decodedInstruction res;

    if((flag && dropFlagCount < 2) || instruction == -1){
        res.isValid = false;
        return res;
    }

    res.opcode = (instruction >> 28) & 0xF;
    res.r1 = (instruction >> 23) & 0x1F; // R-type & I-type instructions
    res.r2 = (instruction >> 18) & 0x1F; // R-type & I-type instructions
    res.r3 = (instruction >> 13) & 0x1F; // R-type instructions
    res.shamt = instruction & 0x1FFF; // R-type instructions

    int tmp = instruction & 0b111111111111111111; //18 bits
    int signedVal = sign_extend(tmp, 18);
    
    res.immediate = signedVal; // I-type instructions
    res.address = instruction & 0b1111111111111111111111111111; // J-type instructions
    res.isValid = true; // no error occured and the instrucion hasn't been dropped

    return res;
}

decodedInstruction decode_cycle2(decodedInstruction instData){
    int opcode = instData.opcode;

    if((flag && dropFlagCount ==2) || instData.isValid == false){
        return instData;
    }

    switch(opcode){
        case 0b0000: //ADD
            instData.type = R_TYPE;
            break;
        case 0b0001: //SUB
            instData.type = R_TYPE;
            break;
        case 0b0010: //MUL
            instData.type = R_TYPE;
            break;
        case 0b0011: //MOVI
            instData.type = I_TYPE;
            instData.r2 = 0;
            break;
        case 0b0100: //JEQ
            instData.type = I_TYPE;
            break;
        case 0b0101: //AND
            instData.type = R_TYPE;
            break;
        case 0b0110: //XORI
            instData.type = I_TYPE;
            break;
        case 0b0111: //JMP
            
            instData.type = J_TYPE;
            break;
        case 0b1000: //LSL
            instData.type = R_TYPE;
            break;
        case 0b1001: //LSR
            instData.type = R_TYPE;
            break;
        case 0b1010: //MOVR
            instData.type = I_TYPE;
            break;
        case 0b1011: //MOVM
            instData.type = I_TYPE;
            break;
        default: // for invalid instructions
            printf("An error occurred in decode_cycle2!");
            instData.isValid = false;
    }

    return instData;
}

int execute_cycle1(decodedInstruction instr){
    if((flag && dropFlagCount < 2) || instr.isValid == false){
        return -1;
    }

    return instr.opcode;
}

mem_reg_destinationInfo execute_cycle2(int opcode, decodedInstruction instr){
    mem_reg_destinationInfo res;

    if((flag && dropFlagCount == 2) || opcode == -1){
        printf("entered null. flag = %s, opcode = %d \n", flag ? "true" : "false", opcode);
        res.isValid = false;
        res.dropFlag = false;
        return res;
    }


    if (opcode < 0 || opcode > 11) {
        printf("ERROR: Invalid opcode in execute_cycle2: %d\n", opcode);
        res.isValid = false;
        return res;
    }

    carry_flag = false;
    overflow_flag = false;
    
    switch (opcode) {
    case 0b0000: //ADD
            res = add(instr.r1,instr.r2,instr.r3);
            break;
    case 0b0001: //SUB
            res = sub(instr.r1,instr.r2,instr.r3);
            break;
    case 0b0010: //MUL
            res = mul(instr.r1,instr.r2,instr.r3);
            break;
    case 0b0011: //MOVI
            res = movi(instr.r1,instr.immediate);
            break;
    case 0b0100: //JEQ
            res = jeq(instr.r1,instr.r2,instr.immediate);
            break;
    case 0b0101: //AND
            res = and(instr.r1,instr.r2,instr.r3);
            break;
    case 0b0110: //XORI
            res = xori(instr.r1,instr.r2,instr.immediate);
            break;
    case 0b0111: //JMP
            res = jmp(instr.address);
            break;
    case 0b1000: //LSL
            res = lsl(instr.r1,instr.r2,instr.shamt);
            break;
    case 0b1001: //LSR
            res = lsr(instr.r1,instr.r2,instr.shamt);
            break;
    case 0b1010: //MOVR
            res = movr(instr.r1,instr.r2,instr.immediate);
            break;
    case 0b1011: //MOVM
            res = movm(instr.r1,instr.r2,instr.immediate);
            break;
    }

    return res;
}

mem_reg_destinationInfo add(int r1, int r2, int r3) {
    int32_t a = reg_array[r2];
    int32_t b = reg_array[r3];

    // Use 64-bit to capture overflow bit
    int64_t signed_result = (int64_t)a + (int64_t)b;
    uint64_t unsigned_result = (uint64_t)(uint32_t)a + (uint64_t)(uint32_t)b;

    // Carry: check bit 32 (for 32-bit unsigned)
    carry_flag = (unsigned_result >> 32) & 1;

    // Overflow: XOR of carry-in to MSB and carry-out of MSB
    int sign_a = (a >> 31) & 1;
    int sign_b = (b >> 31) & 1;
    int sign_res = (int32_t)signed_result >> 31 & 1;

    overflow_flag = (sign_a == sign_b && sign_a != sign_res);

    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = (int32_t) signed_result;  // Truncate to 32-bit
    res.isValid = true;
    res.dropFlag = false;

    printf("ADD: a=%d, b=%d, result=%d | carry=%d, overflow=%d\n", a, b, res.value, carry_flag, overflow_flag);

    return res;
}


mem_reg_destinationInfo sub(int r1, int r2, int r3) {
    int32_t a = reg_array[r2];
    int32_t b = reg_array[r3];

    int64_t signed_result = (int64_t)a - (int64_t)b;
    uint64_t unsigned_result = (uint64_t)(uint32_t)a - (uint64_t)(uint32_t)b;

    // Carry (unsigned borrow): if a < b
    carry_flag = (uint32_t)a < (uint32_t)b;

    // Overflow: a and b have opposite signs, and result sign is different from a
    int sign_a = (a >> 31) & 1;
    int sign_b = (b >> 31) & 1;
    int sign_res = (int32_t)signed_result >> 31 & 1;

    overflow_flag = (sign_a != sign_b && sign_a != sign_res);

    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = (int32_t) signed_result;
    res.isValid = true;
    res.dropFlag = false;

    printf("SUB: a=%d, b=%d, result=%d | carry=%d, overflow=%d\n", a, b, res.value, carry_flag, overflow_flag);

    return res;
}

mem_reg_destinationInfo mul(int r1, int r2, int r3) {
    int32_t a = reg_array[r2];
    int32_t b = reg_array[r3];

    int64_t wide_result = (int64_t)a * (int64_t)b;

    // Overflow: if result doesn't fit in 32-bit signed range
    overflow_flag = (wide_result > INT32_MAX || wide_result < INT32_MIN);
    carry_flag = false;  // Carry flag is undefined in signed MUL

    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = (int32_t) wide_result;  // Truncate
    res.isValid = true;
    res.dropFlag = false;

    printf("MUL: a=%d, b=%d, result=%d | overflow=%d\n", a, b, res.value, overflow_flag);

    return res;
}



mem_reg_destinationInfo movi(int r1, int imm) {
    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = imm;
    res.isValid = true;
    res.dropFlag = false;
    
    return res;

}

mem_reg_destinationInfo jeq(int r1, int r2, int offset) {
    mem_reg_destinationInfo res;

    if (reg_array[r1] == reg_array[r2]) {
        reg_array[32] = offset + reg_array[32];
        printf("    REG: PC --> %d\n", reg_array[32]);
        res.index = 32;
        res.isValid = true;
        flag = true;
    } else{
        res.isValid = false;
        res.dropFlag = false;
    }
    
    return res;

}

mem_reg_destinationInfo and(int r1, int r2, int r3) {
    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = reg_array[r2] & reg_array[r3];
    res.isValid = true;
    res.dropFlag = false;
    
    return res;
}

mem_reg_destinationInfo xori(int r1, int r2, int imm) {
    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = reg_array[r2] ^ imm;
    res.isValid = true;
    res.dropFlag = false;
    
    return res;
}

mem_reg_destinationInfo jmp(int address) {
    mem_reg_destinationInfo res;
    reg_array[32] = address;
    printf("    REG: PC --> %d\n", reg_array[32]);
    res.index = 32;
    res.isValid = true;
    flag = true;
    return res;
}

mem_reg_destinationInfo lsl(int r1, int r2, int shamt) {
    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = reg_array[r2] << shamt;
    res.isValid = true;
    res.dropFlag = false;
    
    return res;
}

mem_reg_destinationInfo lsr(int r1, int r2, int shamt) {
    mem_reg_destinationInfo res;
    res.type = 'R';
    res.index = r1;
    res.value = reg_array[r2] >> shamt;
    res.isValid = true;
    res.dropFlag = false;
    
    return res;
}

mem_reg_destinationInfo movr(int r1, int r2, int offset) {
    int addr = reg_array[r2] + offset;

    mem_reg_destinationInfo res;
    res.type = 'B';
    res.index = r1;
    res.value = addr; // overwrite the value for the address in the memory function to get memory[addr]
    res.isValid = true;
    res.dropFlag = false;
    
    return res;
}

mem_reg_destinationInfo movm(int r1, int r2, int offset) {
    int addr = reg_array[r2] + offset;

    mem_reg_destinationInfo res;
    res.type = 'M';
    res.index = addr;
    res.value = reg_array[r1]; 
    res.isValid = true;
    res.dropFlag = false;

    return res;
}

mem_reg_destinationInfo MEM(mem_reg_destinationInfo dest){
    mem_reg_destinationInfo res;

    if((flag && dropFlagCount < 2) || dest.isValid == false){
        res.isValid = false;
        res.dropFlag = false;
        return res;
    }

    if(dest.type == 'B'){
        res.type = 'R';
        res.index = dest.index;
        res.value = memory[dest.value+memoryDataCounter];
        printf("    Output: MEM: Read from memory[%d] = %d to be written in R%d\n", dest.value, res.value, res.index);
        res.isValid = true;
        res.dropFlag = false;
    } else if(dest.type == 'M'){
        memory[dest.index + memoryDataCounter] = dest.value;
        printf("    Output: MEM: Stored %d --> memory[%d]\n", dest.value, dest.index);
        res.type = dest.type;
        res.index = dest.index;
        res.value = dest.type;
        res.isValid = false;
        res.dropFlag = false;
    } else{
        printf("    Output: nothing read or written into memory\n", dest.value, dest.index);
        return dest;
    }
    
    return res;
}

bool writeBack(mem_reg_destinationInfo dest){
    if(dest.isValid == false || dest.type != 'R'){
        printf("    Output: REG: nothing was loaded into a register\n");
        return false;
    }
    if(dest.index != 0 && dest.index != 32 && dest.type == 'R'){
        reg_array[dest.index] = dest.value;
    } 

    printf("    Output: REG: Loaded R%d --> %d\n", dest.index, dest.value);

    if(dest.dropFlag == true){
        flag = true;
    }

    return true;
}