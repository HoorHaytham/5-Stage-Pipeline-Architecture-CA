#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "hardware.h"

InstructionInfo instructionSet[] = {
    {"ADD", ADD, R_TYPE},
    {"SUB", SUB, R_TYPE},
    {"MUL", MUL, R_TYPE},
    {"MOVI", MOVI, I_TYPE},
    {"JEQ", JEQ, I_TYPE},
    {"AND", AND, R_TYPE},
    {"XORI", XORI, I_TYPE},
    {"JMP", JMP, J_TYPE},
    {"LSL", LSL, R_TYPE},
    {"LSR", LSR, R_TYPE},
    {"MOVR", MOVR, I_TYPE},
    {"MOVM", MOVM, I_TYPE}
};

Opcode getOpcode(const char *mnemonic) {
    for (int i = 0; i < 12; i++) {
        if (strcmp(mnemonic, instructionSet[i].mnemonic) == 0)
            return instructionSet[i].opcode;
    }
    return -1;
}

InstrType getInstrType(const char *mnemonic) {
    for (int i = 0; i < 12; i++) {
        if (strcmp(mnemonic, instructionSet[i].mnemonic) == 0)
            return instructionSet[i].type;
    }
    return -1;
}

void intToBinary(int value, int bits, char *output) {
    output[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        output[i] = (value & 1) + '0';
        value >>= 1;
    }
}

int getRegisterNumber(const char *reg) {
    if (reg[0] == 'R')
        return atoi(reg + 1);
    return 0;
}

int binaryStringToInt(const char *binary) {
    int result = 0;
    while (*binary != '\0') {
        result = (result << 1) | (*binary - '0');
        binary++;
    }
    return result;
}

void encodeInstruction(const char *mnemonic, char *arg1, char *arg2, char *arg3) {
    char binary[33]; // 32 bits + null terminator
    Opcode opcode = getOpcode(mnemonic);
    InstrType type = getInstrType(mnemonic);

    char opBin[5], r1Bin[6], r2Bin[6], r3Bin[6], shamtBin[14], immBin[19], addrBin[29];

    intToBinary(opcode, 4, opBin);

    if (type == R_TYPE) {
        intToBinary(getRegisterNumber(arg1), 5, r1Bin);
        intToBinary(getRegisterNumber(arg2), 5, r2Bin);
        intToBinary(getRegisterNumber(arg3), 5, r3Bin);

        if (strcmp(mnemonic, "LSL") == 0 || strcmp(mnemonic, "LSR") == 0)
            intToBinary(atoi(arg3), 13, shamtBin);
        else
            intToBinary(0, 13, shamtBin);

        sprintf(binary, "%s%s%s%s%s", opBin, r1Bin, r2Bin, r3Bin, shamtBin);
    }

    else if (type == I_TYPE) {
        intToBinary(getRegisterNumber(arg1), 5, r1Bin);
        int imm;
        if (arg3 == NULL) {
            intToBinary(0, 5, r2Bin); // For MOVI
            imm = atoi(arg2);
        } else {
            intToBinary(getRegisterNumber(arg2), 5, r2Bin);
            imm = atoi(arg3);
        }

        if (imm < 0)
            imm = (1 << 18) + imm; // Two's complement for negative numbers

        intToBinary(imm, 18, immBin);
        sprintf(binary, "%s%s%s%s", opBin, r1Bin, r2Bin, immBin);
    }

    else if (type == J_TYPE) {
        int addr = atoi(arg1);
        if (addr < 0)
            addr = (1 << 28) + addr;

        intToBinary(addr, 28, addrBin);
        sprintf(binary, "%s%s", opBin, addrBin);
    }

    printf("Binary Encoding: %s\n", binary);
    
    memory[memoryInstructionCounter++] = binaryStringToInt(binary);
}

void parseInstruction(char *line) {
    char *mnemonic = strtok(line, " \n");
    char *arg1 = strtok(NULL, " \n");
    char *arg2 = strtok(NULL, " \n");
    char *arg3 = strtok(NULL, " \n");

    if (!mnemonic)
        return;

    printf("Parsed Instruction: %s %s %s %s\n", mnemonic,
           arg1 ? arg1 : "-", arg2 ? arg2 : "-", arg3 ? arg3 : "-");

    encodeInstruction(mnemonic, arg1, arg2, arg3);
}

void readFile() {
    FILE *file = fopen("dummy_instructions.txt", "r");
    if (!file) {
        perror("Could not open file");
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        parseInstruction(line);
    }

    fclose(file);
}