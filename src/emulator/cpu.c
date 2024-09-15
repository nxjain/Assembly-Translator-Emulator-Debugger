/**
 * @file cpu.c
 * @brief Definitions for all enums, structs, constants and functions used in running the CPU.
 * @details This source file declares any function that is used for running the CPU.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "register.h"
#include "memory.h"
#include "cpu.h"
#include "../utils.h"
#include "../debugging.h"

#define DEBUGGING_MODE false

//Declare processor state variables:
processor_state pstate = {false, true, false, false};
/**
 * Initialize the CPU with register values and load instructions from a binary file into memory.
 *
 * @param input_file_path The path to the binary file containing instructions to load.
 *
 * @note If the file cannot be opened, an error message is printed to stderr, and the program exits.
 *
 * This function initializes the general-purpose registers and memory for the CPU.
 * It opens the specified binary file, loads the instructions into memory, and then closes the file.
 */
void init_cpu(const char* input_file_path) {
    //initialize general registers
    init_register();

    //initialize memory and load the instructions
    init_memory();

    //open file
    FILE *input_file = fopen(input_file_path, "rb");
    if (input_file == NULL) {
        fprintf(stderr, "Failed to open file %s", input_file_path);
        exit(EXIT_FAILURE);
    }

    load_instructions_to_memory(input_file);

    fclose(input_file);
}

/** Fetches an instruction from memory at the current program counter address. */
static Instruction fetch(void) {
    Instruction inst;
    inst.data = get_word(get_spec_register(PROGRAM_COUNTER));
    return inst;
}

// -----------------------------DP EXECUTE HELPER FUNCS:----------------------------
/**
 * @brief Apply an arithmetic operation (addition or subtraction) to two 32-bit unsigned integers and stores the result.
 *
 * @param src The first operand.
 * @param operand2 The second operand.
 * @param dest_reg_index The index of the destination register where the result will be stored.
 * @param opc_flag Flag to indicate if condition flags should be updated.
 * @param opc_op Operation code: 0 for addition, non-zero for subtraction.
 */
static void apply_arithmetic_32(uint32_t src, uint32_t operand2, uint32_t dest_reg_index, int opc_flag, int opc_op){
    int32_t temp_result;

    if (opc_op){ //subtraction
        temp_result = (int32_t)src - (int32_t)operand2;
    } else { //addition
        temp_result = (int32_t)src + (int32_t)operand2;
    }
    // Removes errors created by potentially
    uint32_t result = (uint32_t)temp_result;

    // Potentially update flags
    if (opc_flag){
        pstate.negative_flag = (result & MSB_32_BIT) != 0;
        pstate.zero_flag = result == 0;
        pstate.overflow_flag = (src > 0 && operand2 > 0 && result < 0);
        if (opc_op){ // subtraction
            pstate.carry_flag = (src >= operand2);
        } else{
            // If the result is less than either operand, an overflow (carry) occurred
            pstate.carry_flag = result < src || result < operand2;
        }
    }

    // Do not write if destination register is 11111 -> zero register
    if (dest_reg_index == NUM_REGISTERS){
        return;
    }

    //Store result in destination register
    set_reg_value(dest_reg_index, result); 
}

/**
 * @brief Apply an arithmetic operation (addition or subtraction) to two 64-bit unsigned integers and stores the result.
 *
 * @param src The first operand.
 * @param operand2 The second operand.
 * @param dest_reg_index The index of the destination register where the result will be stored.
 * @param opc_flag Flag to indicate if condition flags should be updated.
 * @param opc_op Operation code: 0 for addition, non-zero for subtraction.
 */
static void apply_arithmetic_64(uint64_t src, uint64_t operand2, uint32_t dest_reg_index, int opc_flag, int opc_op){
    int64_t temp_result;

    if (opc_op){ //subtraction
        temp_result = (int64_t)src - (int64_t)operand2;
    } else { //addition
        temp_result = (int64_t)src + (int64_t)operand2;
    }
    uint64_t result = (uint64_t)temp_result;

    // Potentially update flags
    if (opc_flag){
        pstate.negative_flag = ((result & MSB_64_BIT) != 0);
        pstate.zero_flag = (result == 0);
        pstate.overflow_flag = (src > 0 && operand2 > 0 && result < 0);
        if (opc_op){ // subtraction
            pstate.carry_flag = (src >= operand2);
        } else{
            // If the result is less than either operand, an overflow (carry) occurred
            pstate.carry_flag = result < src || result < operand2;
        }
    }

    // Do not write if destination register is 11111 -> zero register
    if (dest_reg_index == NUM_REGISTERS){
        return;
    }

    //Store result in destination register
    set_reg_value(dest_reg_index, result); 
}

/**
 * @brief Apply a shift operation to a 32-bit unsigned integer and returns the result.
 *
 * @param operand The value to be shifted.
 * @param shift The number of positions to shift.
 * @param shift_option The type of shift operation (ITP_LSL, ITP_LSR, ITP_ASR, ITP_ROR).
 * @return The result of the shift operation.
 *
 * The function applies the specified shift operation to the given operand:
 * - ITP_LSL: Logical shift left
 * - ITP_LSR: Logical shift right
 * - ITP_ASR: Arithmetic shift right
 * - ITP_ROR: Rotate right
 */
static uint32_t apply_shift_32(uint32_t operand, uint32_t shift, int shift_option){
    switch (shift_option) {
        case ITP_LSL:
            // Logic Shift Left:
            operand = operand << shift;
            break;
        case ITP_LSR:
            // Logic Shift Right:
            operand = operand >> shift;
            break;
        case ITP_ASR: {
            // Arithmetic Shift Right:
            // Convert to signed operand to preserve sign when shifting right
            int32_t signed_operand = (int32_t) operand;
            signed_operand = signed_operand >> shift;
            operand = (uint32_t)signed_operand;
            break;}
        case ITP_ROR:
            // Rotate Right Shift:
            operand = (operand >> shift) | (operand << (CONST_32 - shift));
            break;
    }
    return operand;
}

/**
 * @brief Apply a shift operation to a 64-bit unsigned integer and returns the result.
 *
 * @param operand The value to be shifted.
 * @param shift The number of positions to shift.
 * @param shift_option The type of shift operation (ITP_LSL, ITP_LSR, ITP_ASR, ITP_ROR).
 * @return The result of the shift operation.
 *
 * The function applies the specified shift operation to the given operand:
 * - ITP_LSL: Logical shift left
 * - ITP_LSR: Logical shift right
 * - ITP_ASR: Arithmetic shift right
 * - ITP_ROR: Rotate right
 */
static uint64_t apply_shift_64(uint64_t operand, uint32_t shift, int shift_option){
    switch (shift_option) {
        case ITP_LSL:
            // Logic Shift Left:
            operand = operand << shift;
            break;
        case ITP_LSR:
            // Logic Shift Right:
            operand = operand >> shift;
            break;
        case ITP_ASR: {
            // Arithmetic Shift Right:
            // Convert to signed operand to preserve sign when shifting right
            int64_t signed_operand = (int64_t) operand;
            signed_operand = signed_operand >> shift;
            operand = (uint64_t)signed_operand;
            break;}
        case ITP_ROR:
            // Rotate Right Shift:
            operand = (operand >> shift) | (operand << (CONST_64 - shift));
            break;
    }
    return operand;
}

/**
 * @brief Apply a logical operation to two 64-bit unsigned integers and return the result.
 *
 * @param src The first operand.
 * @param operand2 The second operand.
 * @param opc The operation code specifying the logical operation (ITP_AND, ITP_AND_W_FLAGS, ITP_OR, ITP_XOR).
 * @param N A flag indicating if a bitwise NOT should be applied to operand2.
 * @return The result of the logical operation.
 */
static uint64_t apply_logic(uint64_t src, uint64_t operand2, uint32_t opc, uint32_t N){
    uint64_t result;
    // Apply bitwise NOT on op2 if N == 1 
    if (N){
        operand2 = ~operand2;
    }

    switch (opc) {
        case ITP_AND:
        case ITP_AND_W_FLAGS: //flags will get dealt with later
            result = src & operand2;
            break;
        case ITP_OR:
            result = src | operand2;
            break;
        case ITP_XOR:
            result = src ^ operand2;
            break;
    }
    return result;
}

// -----------------------------EXECUTE FUNCS:----------------------------

/**
 * @brief Execute an immediate arithmetic instruction.
 * @param inst The segmented immediate arithmetic instruction
 */
static void exec_imm_arithmetic(const ImmArith inst) {
    debug_printf("EXECUTE: imm_arithmetic %d bit\n\n", inst.sf ? 64 : 32);
    uint32_t operand2 = inst.imm12;
    if (inst.sh){
        operand2 = operand2 << 12;
    }

    if (inst.sf){ // 64 bit mode
        uint64_t src = get_reg_value_64(inst.rn);
        apply_arithmetic_64(src, operand2, inst.rd, inst.opc_flag, inst.opc_op);

    } else { // 32 bit mode
        uint32_t src =  get_reg_value_32(inst.rn);
        apply_arithmetic_32(src, operand2, inst.rd, inst.opc_flag, inst.opc_op);
    }
}

/**
 * @brief Execute a wide move instruction.
 * @param inst The segmented wide move instruction.
 */
static void exec_wide_move(const ImmWide inst) {
    debug_printf("EXECUTE: imm_wide_move %d bit\n\n", inst.sf ? 64 : 32);
    if (inst.opc == ITP_MOVK){
        int position = 16 * inst.hw;
        uint64_t register_value = get_reg_value_64(inst.rd);
    
        // Clear the 16 bits at the specified position in x
        uint64_t mask = ~(0xFFFFULL << position);
        
        // Clear the bits at the specified position
        register_value &= mask;

        // Combine the modified x with the shifted y
        register_value = register_value | (uint64_t)inst.imm16 << position;
        if (!inst.sf) { // 32 bit
            register_value &= 0xFFFFFFFFULL; // Rd[63 : 32] set to zero.
        }

        //Does not need to distinguish between wn and xn
        set_reg_value(inst.rd, register_value);
    } else{
        uint64_t operand = ((uint64_t) inst.imm16) << (inst.hw * 16);
        if (inst.opc == ITP_MOVN){ // in MOVN: Rd := ~Op
            operand = ~operand;
        }
        if (!inst.sf) { // 32 bit
            operand &= 0xFFFFFFFFULL; // Rd[63 : 32] set to zero.
        }
        // Safe as specified by spec that operand will be max 2^32 - 1, so no information will be lost.
        set_reg_value(inst.rd, operand);
    }
}

/**
 * @brief Execute a register arithmetic instruction.
 * @param inst The segmented register arithmetic instruction.
 */
static void exec_reg_arithmetic(const RegArith inst) {
    debug_printf("EXECUTE: reg_arithmetic %d bit\n\n", inst.sf ? 64 : 32);
     if (inst.sf) {
        uint64_t operand2 = get_reg_value_64(inst.rm);

        operand2 = apply_shift_64(operand2, inst.operand, inst.shift);

        uint64_t src = get_reg_value_64(inst.rn);
        apply_arithmetic_64(src, operand2, inst.rd, inst.opc_flag, inst.opc_op);

    } else {
        uint32_t operand2 = get_reg_value_32(inst.rm); // Truncate operand2 to 32 bits

        operand2 = apply_shift_32(operand2, inst.operand, inst.shift);

        uint32_t src = get_reg_value_32(inst.rn);
        apply_arithmetic_32(src, operand2, inst.rd, inst.opc_flag, inst.opc_op);
    }
}

/**
 * @brief Execute a register logic instruction.
 * @param inst The segmented register logic instruction.
 */
static void exec_reg_logic(const RegLogic inst) {
    debug_printf("EXECUTE: reg_logic %d bit\n\n", inst.sf ? 64 : 32);
    if (inst.sf) {
        uint64_t operand2 = get_reg_value_64(inst.rm);

        operand2 = apply_shift_64(operand2, inst.operand, inst.shift);

        uint64_t src = get_reg_value_64(inst.rn);

        uint64_t result = apply_logic(src, operand2, inst.opc, inst.N);

        if (inst.opc == ITP_AND_W_FLAGS) {
            pstate.negative_flag = ((result & MSB_64_BIT) != 0);
            pstate.zero_flag = (result == 0);
            pstate.overflow_flag = false;
            pstate.carry_flag = false;
        }

        set_reg_value(inst.rd, result);  //Store result in destination register

    } else {
        uint32_t operand2 = get_reg_value_32(inst.rm); // Truncate operand2 to 32 bits

        operand2 = apply_shift_32(operand2, inst.operand, inst.shift);

        uint32_t src = get_reg_value_32(inst.rn);
        uint32_t result = (uint32_t) apply_logic(src, operand2, inst.opc, inst.N);

        if (inst.opc == ITP_AND_W_FLAGS) {
            pstate.negative_flag = ((result & MSB_32_BIT) != 0);
            pstate.zero_flag = (result == 0);
            pstate.overflow_flag = false;  // cannot be made true by logic operations
            pstate.carry_flag = false;     // cannot be made true by logic operations
        }

        set_reg_value(inst.rd, result);  //Store result in destination register)
    }
    //TODO
}

/**
 * @brief Execute a register multiplication instruction.
 * @param inst The segmented register multiplication instruction.
 */
static void exec_reg_multiply(const RegMultiply inst) {
    debug_printf("EXECUTE: reg_multiply %d bit\n\n", inst.sf ? 64 : 32);
    if (inst.sf) {
        uint64_t rn_val = get_reg_value_64(inst.rn);
        uint64_t rm_val = get_reg_value_64(inst.rm);
        uint64_t ra_val;
        if (inst.ra == 32) { //TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            ra_val = 0;
        } else {
            ra_val = get_reg_value_64(inst.ra);
        }

        __uint128_t temp_result;

        if (inst.x){
            temp_result = (uint64_t)ra_val - ((uint64_t)rn_val * (uint64_t)rm_val);
        } else {
            temp_result = (uint64_t)ra_val + ((uint64_t)rn_val * (uint64_t)rm_val);
        }

        uint64_t result = (uint64_t)temp_result;
        set_reg_value(inst.rd, result);

    } else {
        uint32_t rn_val = get_reg_value_32(inst.rn);
        uint32_t rm_val = get_reg_value_32(inst.rm);
        uint32_t ra_val;
        if (inst.ra == 32) {  //TODO!!!!!!!!!!!!!!!!
            ra_val = 0;
        } else {
            ra_val = get_reg_value_32(inst.ra);
        }

        uint64_t temp_result;

        if (inst.x){
            temp_result = (uint32_t)ra_val - ((uint32_t)rn_val * (uint32_t)rm_val);
        } else {
            temp_result = (uint32_t)ra_val + ((uint32_t)rn_val * (uint32_t)rm_val);
        }

        uint32_t result = (uint32_t)temp_result;
        set_reg_value(inst.rd, result);
    }
}

/**
 * @brief Execute a data transfer immediate offset instruction.
 * @param inst The segmented data transfer immediate offset instruction.
 */
static void exec_dt_imm_offset(const DTImmOffset inst) {
    debug_printf("EXECUTE: dt_imm_offset %d bit\n\n", inst.sf ? 64 : 32);

    uint32_t address;

    if (inst.sf) { //target register: 64 bit
        address = get_reg_value_64(inst.xn) + inst.imm12 * sizeof(double_word);
        if (inst.L) { //Load operation
            set_reg_value(inst.rt, get_double_word(address));
            return;
        }

        //Store operation
        set_double_word(address, get_reg_value_64(inst.rt));
        return;
    }

    //target register: 32 bit
    address = get_reg_value_64(inst.xn) + inst.imm12 * sizeof(word);
    
    if (inst.L) { //Load operation
        set_reg_value(inst.rt, get_word(address));
        return;
    }
    
    //Store operation
    set_word(address, get_reg_value_32(inst.rt));
}

/**
 * @brief Execute a data transfer register offset instruction.
 * @param inst The segmented data transfer register offset instruction.
 */
static void exec_dt_reg_offset(const DTRegOffset inst) {
    debug_printf("EXECUTE: dt_reg_offset %d bit\n\n", inst.sf ? 64 : 32);
    uint32_t address = get_reg_value_64(inst.xn) + get_reg_value_64(inst.xm);

    if (inst.sf) { //target register: 64 bit
        if (inst.L) { //Load operation
            set_reg_value(inst.rt, get_double_word(address));
            return;
        }
        //Store operation
        set_double_word(address, get_reg_value_64(inst.rt));
        return;
    }

    //target register: 32 bit    
    if (inst.L) { //Load operation
        set_reg_value(inst.rt, get_word(address));
        return;
    }
    
    //Store operation
    set_word(address, get_reg_value_32(inst.rt));
}

/**
 * @brief Execute a data transfer load literal instruction.
 * @param inst The segmented data transfer load literal instruction.
 */
static void exec_dt_load_literal(const DTLoadLiteral inst) {
    debug_printf("EXECUTE: dt_load_literal %d bit\n\n", inst.sf ? 64 : 32);
    // Compute memory address
    uint32_t address = get_spec_register(PROGRAM_COUNTER) + (sign_extend(inst.simm19, 19) * sizeof(word));
    if (inst.sf) { // target register: 64 bit
        set_reg_value(inst.rt, get_double_word(address));
        return;
    }
    // target register: 32 bit
    set_reg_value(inst.rt, get_word(address));
    return;
}

/**
 * @brief Execute a data transfer pre-index instruction.
 * @param inst The segmented data transfer pre-index instruction.
 */
static void exec_dt_pre_index(const DTPrePostIndex inst) {
    debug_printf("EXECUTE: dt_pre_index %d bit\n\n", inst.sf ? 64 : 32);
    uint32_t new_address;

    if (inst.sf) { // target register: 64 bit
        new_address = get_reg_value_64(inst.xn) + sign_extend(inst.simm9, 9);
        set_reg_value(inst.xn, new_address); // pre indexing
        if (inst.L) { // ldr
            set_reg_value(inst.rt, get_double_word(new_address));
            return;
        }
        // str
        set_double_word(new_address, get_reg_value_64(inst.rt));
        return;
    }
    // target register: 32 bit
    new_address = get_reg_value_64(inst.xn) + sign_extend(inst.simm9, 9);
    set_reg_value(inst.xn, new_address); // pre indexing
    
    if (inst.L) { // ldr
        set_reg_value(inst.rt, get_word(new_address));
        return;
    }    
    // str
    set_word(new_address, get_reg_value_32(inst.rt));
}

/**
 * @brief Execute a data transfer post-index instruction.
 * @param inst The segmented data transfer post-index instruction.
 */
static void exec_dt_post_index(const DTPrePostIndex inst) {
    debug_printf("EXECUTE: dt_post_index %d bit\n\n", inst.sf ? 64 : 32);
    uint32_t address = get_reg_value_64(inst.xn);
    if (inst.sf) { // target register: 64 bit
         // pre index
        if (inst.L) { // ldr
            set_reg_value(inst.rt, get_double_word(address));
        } else { // str
            set_double_word(address, get_reg_value_64(inst.rt));
        }
        // post indexing
        set_reg_value(inst.xn, address + sign_extend(inst.simm9, 9));
        return;
    }
    // target register: 32 bit  
    if (inst.L) { // ldr
        set_reg_value(inst.rt, get_word(address));
    } else { //str
        set_double_word(address, get_reg_value_64(inst.rt));
    }
    // post indexing
    set_reg_value(inst.xn, address+ sign_extend(inst.simm9, 9));
}

/**
 * @brief Execute an unconditional branch instruction.
 * @param inst The segmented unconditional branch instruction.
 */
static void exec_branch_uncond(const BranchUncond inst) {
    int64_t offset = sign_extend(inst.simm26, 26) * INSTR_SIZE;
    increase_pc(offset);
}

/**
 * @brief Execute a conditional branch instruction.
 * @param inst The segmented conditional branch instruction.
 */
static void exec_branch_cond(const BranchCond inst) {
    bool condition;
    int64_t offset = sign_extend(inst.simm19, 19) * INSTR_SIZE;
    switch(inst.cond){
        case ITP_EQ:
            condition = pstate.zero_flag;
            break;
        case ITP_NE:
            condition = !pstate.zero_flag;
            break;
        case ITP_GE:
            condition = pstate.negative_flag == pstate.overflow_flag;
            break;
        case ITP_LT:
            condition = pstate.negative_flag != pstate.overflow_flag;
            break;
        case ITP_GT:
            condition = (!pstate.zero_flag && pstate.negative_flag == pstate.overflow_flag);
            break;
        case ITP_LE:
            condition = !(!pstate.zero_flag && pstate.negative_flag == pstate.overflow_flag);
            break;
        case ITP_AL:
            condition = true;
            break;
        default:
            condition = false;
            break;
    }
    if (condition){
        increase_pc(offset);
        return;
    }
    increment_pc(); // if no branch then move on
}

/**
 * @brief Execute a branch register instruction.
 * @param inst The segmented branch register instruction.
 */
static void exec_branch_reg(const BranchReg inst) {
    set_spec_register(PROGRAM_COUNTER, get_reg_value_64(inst.xn));
}

/**
 * Decode and execute an instruction based on its type and operands.
 *
 * @param inst The instruction to decode and execute.
 *
 * This function decodes the given instruction and executes the appropriate operation
 * based on the instruction type and operands:
 * - Branch Instructions: Handles unconditional branch, conditional branch, and branch register instructions.
 * - Data Processing using Immediates: Executes immediate arithmetic and wide move operations.
 * - Data Processing using Registers: Executes register multiply, register arithmetic, and register logic operations.
 * - Data Transfers: Handles load literal, immediate offset, register offset, pre-index, and post-index data transfer operations.
 *
 * If the instruction type cannot be identified, it prints an error message and exits.
 */
static void decode_and_execute(const Instruction inst) {
    //Branch Instructions
    if(inst.gen_branch.op0 == ITP_BRANCH){
        if (inst.branch_unconditional.id == ITP_BRANCH_UNCOND) {
            exec_branch_uncond(inst.branch_unconditional);
            return;
        }

        if (inst.branch_conditional.id == ITP_BRANCH_COND) {
            exec_branch_cond(inst.branch_conditional);
            return;
        }

        exec_branch_reg(inst.branch_register);
        return;
    }
    //Data Processing using Immediates
    if (inst.gen_dp_imm.op0 == ITP_DP_IMM){
        if (inst.imm_arith.opi == ITP_IMM_ARITH) {
            exec_imm_arithmetic(inst.imm_arith);
            return;
        }

        if (inst.imm_wide.opi == ITP_WIDE_MOVE) {
            exec_wide_move(inst.imm_wide);
            return;
        }
    }
    
    //Data Processing using Registers
    if (inst.gen_dp_reg.op0 == ITP_DP_REG){
        if (inst.reg_multiply.M == ITP_REG_MULTIPLY) {
            exec_reg_multiply(inst.reg_multiply);
            return;
        }

        if (inst.reg_arith.id == ITP_REG_ARITH) {
            exec_reg_arithmetic(inst.reg_arith);
            return;
        }

        if (inst.reg_logic.id == ITP_REG_LOGIC) {
            exec_reg_logic(inst.reg_logic);
            return;
        }
    }
    
    //Data Transfers
    if (inst.gen_dt.op0_1 == ITP_DT_1 && inst.gen_dt.op0_2 == ITP_DT_2){
        if (inst.dt_load_literal.id == ITP_DT_LOAD_LITERAL) {
            exec_dt_load_literal(inst.dt_load_literal);
            return;
        }

        if (inst.dt_imm_offset.U == ITP_DT_IMM_OFFSET) {
            exec_dt_imm_offset(inst.dt_imm_offset);
            return;
        }

        if (inst.dt_reg_offset.id2 == ITP_DT_REGISTER_OFFSET) {
            exec_dt_reg_offset(inst.dt_reg_offset);
            return;
        }

        if (inst.dt_pre_post_index.I == ITP_DT_PRE_INDEX) {
            exec_dt_pre_index(inst.dt_pre_post_index);
            return;
        }

        exec_dt_post_index(inst.dt_pre_post_index);
        return;
    }

    fprintf(stderr, "ERROR: %x: Unknown instruction at address: %lu\n", inst.data, get_spec_register(PROGRAM_COUNTER));
    exit(EXIT_FAILURE);
}

// -----------------------------RUN FUNC:----------------------------
/**
 * @brief Runs the CPU by continuously fetching, decoding, and executing instructions until a halt instruction is encountered.
 *
 * This function simulates the execution of a CPU by repeatedly fetching an instruction,
 * decoding it, executing it, and then fetching the next instruction until a halt instruction
 * (HALT_INSTRUCTION) is encountered.
 *
 * The function prints debugging information for each fetched instruction before decoding and executing it,
 * including the instruction in hexadecimal format and its corresponding program counter (PC).
 * After executing each instruction (except branch instructions), the program counter is incremented.
 */
void run_cpu(void) {
    Instruction inst = fetch();

    while (inst.data != HALT_INSTRUCTION) {
        // Debuggings print statements to display all information every fde cycle:
        debug_printf("FETCH: 0x%x | PC: 0x%lx\nBinary: ", inst.data, get_spec_register(PROGRAM_COUNTER));
        print_bits(inst.data);

        decode_and_execute(inst);
        // Increment PC if instruction wasn't a branch instruction:
        if (inst.gen_branch.op0 != ITP_BRANCH) {
            increment_pc();
        }
        inst = fetch();
    }
}

// ----------------------------PRINT_CPU FUNC:---------------------------
/**
 * @brief Print CPU state information to a specified output file or to stdout if no file path is provided.
 *
 * @param output_file_path The path to the output file. If NULL, prints to stdout.
 *
 * This function prints the current state of the CPU including:
 * - Register values
 * - Processor state flags (negative, zero, carry, overflow)
 * - Memory contents
 *
 * If `output_file_path` is provided, the information is printed to the specified file;
 * otherwise, it is printed to stdout.
 */
void print_cpu(const char* output_file_path) {
    FILE* output_file;

    if (!output_file_path) {
        output_file = stdout;
    } else {
        output_file = fopen(output_file_path, "w");
    }

    print_registers(output_file);

    fprintf(output_file, "PSTATE : %s%s%s%s\n", \
    pstate.negative_flag ? "N" : "-", \
    pstate.zero_flag     ? "Z" : "-", \
    pstate.carry_flag    ? "C" : "-", \
    pstate.overflow_flag ? "V" : "-");
    
    print_memory(output_file);

    if (output_file_path) {
        fclose(output_file);
    }
}

// ----------------------------USED IN DEBUGGER:---------------------------

/**
 * @brief Executes a single instruction cycle.
 *
 * This function fetches the next instruction, decodes and executes it,
 * and handles the program counter (PC) increment if the instruction
 * is not a branch operation. It also checks if the fetched instruction
 * is a halt instruction and returns a boolean value indicating whether
 * the execution should continue.
 *
 * @return bool - Returns true if the fetched instruction is not the halt instruction,
 *                allowing execution to continue. Returns false if the instruction is
 *                the halt instruction, indicating that execution should stop.
 */
bool step_instruction(){
    Instruction inst = fetch();
    decode_and_execute(inst);
    if (inst.gen_branch.op0 != ITP_BRANCH) {
        increment_pc();
    }
    return inst.data != HALT_INSTRUCTION;
}

/* To retrieve the pstate in other files (e.g. debug_logic)*/
processor_state get_pstate(){
    return pstate;
}
