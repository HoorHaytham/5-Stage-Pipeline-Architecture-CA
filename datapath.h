#ifndef DATAPATH_H
#define DATAPATH_H

#include <stdbool.h>
#include "parser.h"

extern bool flag;
extern bool overflow_flag;
extern bool carry_flag;
typedef struct {
    int shamt;   // Shift amount
    int r3;    // Destination or third register
    int r2;    // Second source register
    int r1;    // First source register
    int opcode;    // Operation code
} RFormat;

typedef struct {
    int immediate;  // Immediate value (can be signed)
    int r2;   // Second register or unused for some ops
    int r1;   // Destination register
    int opcode;   // Operation code
} IFormat;

typedef struct {
    int address;   // Jump target address
    int opcode;    // Operation code
} JFormat;

typedef struct {
    InstrType type;  // Tag to identify which format is valid
    bool isValid;

    union {
        RFormat r_format;
        IFormat i_format;
        JFormat j_format;
    } format;
} Instruction;

typedef struct {
    InstrType type;
    int opcode;
    int r1;
    int r2;
    int r3;
    int shamt;
    int immediate;
    int address;
    bool isValid;
} decodedInstruction;

typedef struct {
    char type; // store 'M' for memory, 'R' for register, or 'B' for both
    int index; // indicate the index of memory or reg to accessed
    int value; // value to be stored or loc used to read from memory.
    bool isValid; // to flag if an error occurs
    bool dropFlag; // to flag if executing/decoding instructions need to be dropped due to a jmp
} mem_reg_destinationInfo;

int sign_extend(int value, int bits);

int fetch();

decodedInstruction decode_cycle1(int instruction);
decodedInstruction decode_cycle2(decodedInstruction instData);

int execute_cycle1(decodedInstruction instr);
mem_reg_destinationInfo execute_cycle2(int opcode, decodedInstruction instr);

// ---------- Instruction Implementations ----------
mem_reg_destinationInfo add(int r1, int r2, int r3);
mem_reg_destinationInfo sub(int r1, int r2, int r3);
mem_reg_destinationInfo mul(int r1, int r2, int r3);
mem_reg_destinationInfo movi(int r1, int imm);
mem_reg_destinationInfo jeq(int r1, int r2, int offset);
mem_reg_destinationInfo and(int r1, int r2, int r3);
mem_reg_destinationInfo xori(int r1, int r2, int imm);
mem_reg_destinationInfo jmp(int address);
mem_reg_destinationInfo lsl(int r1, int r2, int shamt);
mem_reg_destinationInfo lsr(int r1, int r2, int shamt);
mem_reg_destinationInfo movr(int r1, int r2, int offset);
mem_reg_destinationInfo movm(int r1, int r2, int offset);

// ---------- Memory and Writeback ----------
mem_reg_destinationInfo MEM(mem_reg_destinationInfo dest);
bool writeBack(mem_reg_destinationInfo dest);

#endif // DATAPATH_H
