#include <stdio.h>
#include <stdbool.h>
#include "datapath.h"
#include "hardware.h"
#include "executor.h"
#include <stdint.h>  // For fixed-width types like int32_t
#include <limits.h>  // ✅ This is what you need for INT32_MAX, INT32_MIN, and UINT32_MAX


void print_registers() {
    printf("=== Registers ===\n");
    for (int i = 0; i < 32; i++) {
        printf("R[%d] = %d\n", i, reg_array[i]);
    }
    printf("PC = %d\n", reg_array[32]);
}

// void print_memory() {
//     printf("\n=== Memory [1024+] ===\n");
//     for (int i = 1024; i < memoryDataCounter; i++) {
//         printf("mem[%d] = %d\n", i, memory[i]);
//     }
// }

void print_memory() {
    printf("\n=== Memory [1024+] ===\n");
    for (int i = 1024; i < memoryDataCounter; i++) {
        printf("mem[%d]=%d  ", i, memory[i]);
        if ((i - 1024 + 1) % 5 == 0)  // Print 5 memory cells per line
            printf("\n");
    }
   printf("\n");
}

// const char* instrTypeToStr(InstrType type) {
//     switch (type) {
//         case R_TYPE: return "R_TYPE";
//         case I_TYPE: return "I_TYPE";
//         case J_TYPE: return "J_TYPE";
//         default: return "UNKNOWN";
//     }
// }
/*
void test_add_overflow_carry() {
    printf("\n=== ADD OVERFLOW & CARRY TEST ===\n");

    // Case 1: Signed overflow: INT_MAX + 1
    reg_array[2] = INT32_MAX;
    reg_array[3] = 1;

    Instruction instr1 = {
        .isValid = true,
        .type = R_TYPE,
        .format.r_format = {
            .opcode = 0b0000,  // ADD
            .r1 = 1,
            .r2 = 2,
            .r3 = 3
        }
    };

    int opcode = execute_cycle1(instr1);
    mem_reg_destinationInfo result = execute_cycle2(true, opcode, instr1);
    writeBack(result);

    printf("R1 (Result) = %d\n", reg_array[1]);
    printf("Expected: Signed overflow, no carry\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("----------------------------------------\n");

    // Case 2: Unsigned carry: UINT_MAX + 1
    reg_array[2] = UINT32_MAX;
    reg_array[3] = 1;

    // reinterpret as int because registers are signed
    reg_array[2] = (int)UINT32_MAX;

    opcode = execute_cycle1(instr1);
    result = execute_cycle2(true, opcode, instr1);
    writeBack(result);

    printf("R1 (Result) = %d\n", reg_array[1]);
    printf("Expected: Carry (unsigned overflow), no signed overflow\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("========================================\n");
}

void test_sub_overflow_carry() {
    printf("\n=== SUB OVERFLOW & CARRY TEST ===\n");

    // Case 1: Signed overflow: INT_MIN - 1
    reg_array[2] = INT32_MIN;
    reg_array[3] = 1;

    Instruction instr2 = {
        .isValid = true,
        .type = R_TYPE,
        .format.r_format = {
            .opcode = 0b0001,  // SUB
            .r1 = 1,
            .r2 = 2,
            .r3 = 3
        }
    };

    int opcode = execute_cycle1(instr2);
    mem_reg_destinationInfo result = execute_cycle2(true, opcode, instr2);
    writeBack(result);

    printf("R1 (Result) = %d\n", reg_array[1]);
    printf("Expected: Signed overflow\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("----------------------------------------\n");

    // Case 2: Unsigned borrow: 0 - 1
    reg_array[2] = 0;
    reg_array[3] = 1;

    opcode = execute_cycle1(instr2);
    result = execute_cycle2(true, opcode, instr2);
    writeBack(result);

    printf("R1 (Result) = %d\n", reg_array[1]);
    printf("Expected: Unsigned borrow --> carry = 1\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("========================================\n");
}


void test_unsigned_mul_overflow() {
    printf("=== UNSIGNED MUL OVERFLOW TEST ===\n");

    // Set registers for overflow test: 70000 * 70000 = 4.9B (too large for uint32)
    reg_array[2] = 70000;
    reg_array[3] = 70000;

    Instruction overflow_instr;
    overflow_instr.isValid = true;
    overflow_instr.type = R_TYPE;
    overflow_instr.format.r_format.opcode = 0b0010;  // MUL
    overflow_instr.format.r_format.r1 = 1;  // destination: R1
    overflow_instr.format.r_format.r2 = 2;
    overflow_instr.format.r_format.r3 = 3;

    int opcode = execute_cycle1(overflow_instr);
    mem_reg_destinationInfo result = execute_cycle2(true, opcode, overflow_instr);
    writeBack(result);

    printf("R1 (Result) = %u\n", reg_array[1]);
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("----------------------------------------\n");

    // Set registers for safe test: 1000 * 1000 = 1M (safe)
    reg_array[2] = 1000;
    reg_array[3] = 1000;

    Instruction safe_instr;
    safe_instr.isValid = true;
    safe_instr.type = R_TYPE;
    safe_instr.format.r_format.opcode = 0b0010;  // MUL
    safe_instr.format.r_format.r1 = 1;  // destination: R1
    safe_instr.format.r_format.r2 = 2;
    safe_instr.format.r_format.r3 = 3;

    opcode = execute_cycle1(safe_instr);
    result = execute_cycle2(true, opcode, safe_instr);
    writeBack(result);

    printf("R1 (Result) = %u\n", reg_array[1]);
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("========================================\n");
}

void test_signed_mul_overflow() {
    printf("\n=== SIGNED MUL OVERFLOW TEST ===\n");

    // Case 1: INT32_MAX * 2 → overflow
    reg_array[2] = INT32_MAX;
    reg_array[3] = 2;

    Instruction instr = {
        .isValid = true,
        .type = R_TYPE,
        .format.r_format = {
            .opcode = 0b0010,  // MUL opcode
            .r1 = 1,
            .r2 = 2,
            .r3 = 3
        }
    };

    int opcode = execute_cycle1(instr);
    mem_reg_destinationInfo result = execute_cycle2(true, opcode, instr);
    writeBack(result);

    printf("R1 = %d\n", reg_array[1]);
    printf("Expected: Overflow\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("----------------------------------------\n");

    // Case 2: -100000 * 50 → no overflow
    reg_array[2] = -100000;
    reg_array[3] = 50;

    opcode = execute_cycle1(instr);
    result = execute_cycle2(true, opcode, instr);
    writeBack(result);

    printf("R1 = %d\n", reg_array[1]);
    printf("Expected: No overflow\n");
    printf("Overflow flag = %d | Carry flag = %d\n", overflow_flag, carry_flag);
    printf("========================================\n");
}*/

/*void test_all_arithmetic_flags() {
    printf("\n=============================\n");
    printf("⚙️  TESTING ADD / SUB / MUL FLAGS\n");
    printf("=============================\n");

    decodedInstruction instr;
    instr.isValid = true;
    instr.type = R_TYPE;
    instr.opcode = 0; // Placeholder, will change
    instr.r1 = 1;
    instr.r2 = 2;
    instr.r3 = 3;

    // ====== ADD: Signed Overflow (INT32_MAX + 1)
    printf("\n🔹 ADD (Signed Overflow)\n");
    reg_array[2] = INT32_MAX;
    reg_array[3] = 1;
    instr.opcode = 0b0000;  // ADD
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    // ====== ADD: Unsigned Carry (UINT32_MAX + 1)
    printf("\n🔹 ADD (Unsigned Carry)\n");
    reg_array[2] = (int)UINT32_MAX;
    reg_array[3] = 1;
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    // ====== SUB: Signed Overflow (INT32_MIN - 1)
    printf("\n🔹 SUB (Signed Overflow)\n");
    reg_array[2] = INT32_MIN;
    reg_array[3] = 1;
    instr.opcode = 0b0001;  // SUB
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    // ====== SUB: Unsigned Borrow (0 - 1)
    printf("\n🔹 SUB (Unsigned Borrow)\n");
    reg_array[2] = 0;
    reg_array[3] = 1;
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    // ====== MUL: Signed Overflow (INT32_MAX * 2)
    printf("\n🔹 MUL (Signed Overflow)\n");
    reg_array[2] = INT32_MAX;
    reg_array[3] = 2;
    instr.opcode = 0b0010;  // MUL
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    // ====== MUL: No Overflow (-100000 * 50)
    printf("\n🔹 MUL (No Overflow)\n");
    reg_array[2] = -100000;
    reg_array[3] = 50;
    writeBack(execute_cycle2(true, execute_cycle1(instr), instr));
    printf("R1 = %d | overflow = %d | carry = %d\n", reg_array[1], overflow_flag, carry_flag);

    printf("\n✅ Flag testing complete.\n");
}*/



/*int main() { // main for testing the datapath functions 
    // Reset
    for (int i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
    for (int i = 0; i < REGISTER_COUNT; i++) reg_array[i] = 0;
    reg_array[32] = 0;  // PC

    readFile();

    while (reg_array[32] < memoryInstructionCounter && !flag) {
        printf("pc = %d\n", reg_array[32]);
        int instr = fetch();
    
        decodedInstruction d1 = decode_cycle1(instr);

        decodedInstruction d2 = decode_cycle2(d1);

        printf("reg or mem: %s\n", instrTypeToStr(d2.type));

        int opcode = execute_cycle1(d2);
        printf("opcode = %d\n", opcode);
        mem_reg_destinationInfo exec_result = execute_cycle2(opcode, d2);
        printf("execres %s \n", exec_result.isValid ? "true" : "false");
        mem_reg_destinationInfo mem_result = MEM(exec_result);

        writeBack(mem_result);
        printf("pc = %d\n", reg_array[32]);
    }

    // Results
    printf("\n=== Registers ===\n");
    for (int i = 0; i < 32; i++)
        printf("R[%d] = %d\n", i, reg_array[i]);
    printf("PC = %d\n", reg_array[32]);

    printf("\n=== Memory [1024+] ===\n");
    for (int i = 1024; i < 1100; i++)
        printf("mem[%d] = %d\n", i, memory[i]);

    return 0;

}*/

/*int main() { // main to test ovrflow and carry
    // Reset
    for (int i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
    for (int i = 0; i < REGISTER_COUNT; i++) reg_array[i] = 0;
    reg_array[32] = 0;  // PC

    test_all_arithmetic_flags();

    // 🔹 Call your MUL overflow test
    
    test_unsigned_mul_overflow();
    test_add_overflow_carry();
    test_sub_overflow_carry();
    test_signed_mul_overflow();
    
    return 0;
}*/

int main(){

    pipelineExecution();

    // Results
    printf("\n=== Registers ===\n");
    for (int i = 0; i < 32; i++)
        printf("R[%d] = %d\n", i, reg_array[i]);
    printf("PC = %d\n", reg_array[32]);

    printf("\n=== Memory [1024+] ===\n");
    for (int i = 1024; i < 1100; i++)
        printf("mem[%d] = %d\n", i, memory[i]);


    return 0;
}


