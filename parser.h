#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#define MAX_LINE_LENGTH 100

// Opcode enumeration
typedef enum {
    ADD = 0,
    SUB,
    MUL,
    MOVI,
    JEQ,
    AND,
    XORI,
    JMP,
    LSL,
    LSR,
    MOVR,
    MOVM
} Opcode;

// Instruction type enumeration
typedef enum {
    R_TYPE,
    I_TYPE,
    J_TYPE
} InstrType;

// Instruction structure
typedef struct {
    char mnemonic[6];
    Opcode opcode;
    InstrType type;
} InstructionInfo;

// Function declarations
Opcode getOpcode(const char *mnemonic);
InstrType getInstrType(const char *mnemonic);
void intToBinary(int value, int bits, char *output);
int getRegisterNumber(const char *reg);
void encodeInstruction(const char *mnemonic, char *arg1, char *arg2, char *arg3);
void parseInstruction(char *line);
void readFile();

#endif // PARSER_H
