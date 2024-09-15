/**
 * @file decode.c
 * @brief Functions for decoding assembly instructions into 32-bit machine code.
 *
 * This file contains functions for initializing decoding, decoding assembly instructions,
 * converting opcode aliases, assembling instructions, and managing memory for decoded instructions.
 * It interacts with symbol tables, dynamic arrays, and various utility functions for
 * reading operands, converting instructions, and handling errors.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "decode.h"
#include "decode_helper.h"
#include "../instructions.h"
#include "../utils.h"
#include "../debugging.h"
#include "../ADTs/darray.h"

#define DEBUGGING_MODE false

#define INSTR_SIZE 4

static DArray *instructions;
static uint32_t current_address;

// ----------------------------------------ASSEMBLE FUNCS:---------------------------------------

/**
 * @brief Initialize the decoding process.
 *
 * This function initializes the symbol table, sets up an array for instructions
 * with a destructor for freeing memory, and initializes the current_address counter to zero.
 */
void decode_init(void) {
    symbol_table_init();
    instructions = darray_init(free);
    current_address = 0;
}

/* Assembles madd and msub instructions. Any aliases are converted beforehand. */
static uint32_t assemble_multiply(char *opcode, char **operands){
    //PRECONDITION: At least 4 operands:
    assert_num_opcodes(operands, MIN_MUL_OPERANDS);

    //Initialise instruction
    Instruction inst;
    inst.data = 0;
    
    // Set bits that indicate it is a multiply instruction:
    inst.reg_multiply.op0 = ITP_DP_REG;
    set_bit(inst.reg_multiply.M);
    set_bit(inst.reg_multiply.id);
    
    // Set bits that are variable due to operands:
    inst.reg_multiply.sf = is_bit_mode_64(operands[OPERAND_1]);
    inst.reg_multiply.rd = read_reg_value(operands[OPERAND_1]);
    inst.reg_multiply.rn = read_reg_value(operands[OPERAND_2]);
    inst.reg_multiply.rm = read_reg_value(operands[OPERAND_3]);
    inst.reg_multiply.ra = read_reg_value(operands[OPERAND_4]);
    inst.reg_multiply.x = (strcmp(opcode, opcode_names[OP_M_SUB]) == 0) || (strcmp(opcode, opcode_names[OP_M_NEG]) == 0);
    
    return inst.data;
}

/* Assembles add, adds, sub, subs instructions. Any aliases are converted beforehand. */
static uint32_t assemble_add_sub(char *opcode, char **operands){
    //PRECONDITION: At least 3 operands:
    assert_num_opcodes(operands, MIN_ADD_SUB_OPERANDS);

    // Initialise instruction
    Instruction inst;
    inst.data = 0;
    
    // Following properties are for both immediate and register arithmetic 
    if (is_zero_register(operands[OPERAND_1])) { // If op1 is zero register - ours does not specify register modes
        inst.imm_arith.sf = is_bit_mode_64(operands[OPERAND_2]);
    } else{ // If op1 not is zero register (hence it mentions register mode)
        inst.imm_arith.sf = is_bit_mode_64(operands[OPERAND_1]);
    }
    inst.imm_arith.opc_op = strncmp(opcode, opcode_names[OP_SUB], 3) == 0 ;
    inst.imm_arith.opc_flag = is_set_flags(opcode);
    inst.imm_arith.rn = read_reg_value(operands[OPERAND_2]);
    inst.imm_arith.rd = read_reg_value(operands[OPERAND_1]);
    
    //Following properties are for only immediate arithmetic 
    if (is_immediate(operands[OPERAND_3])){
        inst.imm_arith.op0 = ITP_DP_IMM;
        inst.imm_arith.opi = ITP_IMM_ARITH;
        
        //Check if there is a shift
        if (operands[OPERAND_5] != NULL){
            inst.imm_arith.sh = read_imm_value(operands[OPERAND_5]) != 0;
        }
        inst.imm_arith.imm12 = read_imm_value(operands[OPERAND_3]);

    } else{ //Following properties are for only register arithmetic 
        inst.reg_arith.op0 = ITP_DP_REG;
        inst.reg_arith.id = ITP_REG_ARITH;
        inst.reg_arith.rm = read_reg_value(operands[OPERAND_3]);
        
        //Check if there is a shift
        if (operands[OPERAND_4] != NULL){
            inst.reg_arith.shift = read_shift_type(operands[OPERAND_4]);
            inst.reg_arith.operand = read_imm_value(operands[OPERAND_5]);
        }
    }
    return inst.data;
}

/* Assembles movn, movz, movk instructions. Any aliases are converted beforehand. */
static uint32_t assemble_wide_move(char *opcode, char **operands){
    //PRECONDITION: At least 2 operands: (I THINK? - change if needed - remove this bracket if confirmed)
    assert_num_opcodes(operands, MIN_WIDE_MOVE_OPERANDS);
    
    // Initialise instruction
    Instruction inst;
    inst.data = 0;

    // Set bits that indicate it is a wide move instruction:
    inst.imm_wide.opi = ITP_WIDE_MOVE;
    inst.imm_wide.op0 = ITP_DP_IMM;
    
    // Set bits that are variable due to operands:
    if (is_zero_register(operands[OPERAND_1])) { // If op1 is zero register - ours does not specify register modes
        inst.imm_wide.sf = is_bit_mode_64(operands[OPERAND_2]);
    } else{ // If op1 not is zero register (hence it mentions register mode)
        inst.imm_wide.sf = is_bit_mode_64(operands[OPERAND_1]);
    }

    if (strcmp(opcode, opcode_names[OP_MOVN]) == 0){
        inst.imm_wide.opc = ITP_MOVN;
    } else if (strcmp(opcode, opcode_names[OP_MOVK]) == 0){
        inst.imm_wide.opc = ITP_MOVK;
    } else if (strcmp(opcode, opcode_names[OP_MOVZ]) == 0) {
        inst.imm_wide.opc = ITP_MOVZ;
    }

    inst.imm_wide.rd = read_reg_value(operands[OPERAND_1]);
    inst.imm_wide.imm16 = read_imm_value(operands[OPERAND_2]);

    if (operands[OPERAND_3] != NULL){
        inst.imm_wide.hw = read_imm_value(operands[OPERAND_4]) / DIV_VAL_HW;
    }

    return inst.data;
}

/* Assembles and, ands, bic, bics, orr, orn, eor, eon instructions. Any aliases are converted beforehand. */
static uint32_t assemble_logic(char *opcode, char **operands){
    //PRECONDITION: At least 3 operands
    assert_num_opcodes(operands, MIN_LOGIC_OPERANDS);
    
    // Initialise instruction
    Instruction inst;
    inst.data = 0;

    // Set bits that indicate it is a logic instruction:
    inst.reg_logic.op0 = ITP_DP_REG;

    // Set bits that are variable due to operands:
    if (is_zero_register(operands[OPERAND_1])) { // If op1 is zero register - ours does not specify register modes
        inst.reg_logic.sf = is_bit_mode_64(operands[OPERAND_2]);
    } else{ // If op1 not is zero register (hence it mentions register mode)
        inst.reg_logic.sf = is_bit_mode_64(operands[OPERAND_1]);
    }

    if (strcmp(opcode, opcode_names[OP_AND]) ==0 || strcmp(opcode, opcode_names[OP_BIC])==0){
        inst.reg_logic.opc = ITP_AND;
    } else if (strcmp(opcode, opcode_names[OP_ORR]) == 0 || strcmp(opcode, opcode_names[OP_ORN])==0) {
        inst.reg_logic.opc = ITP_OR;
    } else if (strcmp(opcode, opcode_names[OP_EOR]) == 0 || strcmp(opcode, opcode_names[OP_EON])==0) {
        inst.reg_logic.opc = ITP_XOR;
    } else if (strcmp(opcode, opcode_names[OP_ANDS]) == 0 || strcmp(opcode, opcode_names[OP_BICS])==0) {
        inst.reg_logic.opc = ITP_AND_W_FLAGS;
    }
    
    inst.reg_logic.N = is_opcode(opcode, (Opcode[]) {OP_BIC, OP_ORN, OP_EON, OP_BICS}, NUM_LOGIC_N_INSTS);

    //Check if there is a shift
    if (operands[OPERAND_4] != NULL){
        inst.reg_logic.shift = read_shift_type(operands[OPERAND_4]);
        inst.reg_logic.operand = read_imm_value(operands[OPERAND_5]);
    }

    inst.reg_logic.rd = read_reg_value(operands[OPERAND_1]);
    inst.reg_logic.rn = read_reg_value(operands[OPERAND_2]);
    inst.reg_logic.rm = read_reg_value(operands[OPERAND_3]);

    return inst.data;

}

/* Assembles ldr and str instructions. Any aliases are converted beforehand. */
static uint32_t assemble_load_store(char *opcode, char **operands){
    //PRECONDITION: At least 2 operands
    assert_num_opcodes(operands, MIN_LOAD_STORE_OPERANDS);

    // Initialise instruction
    Instruction inst;
    inst.data = 0;

    // All instructions share these
    inst.dt_imm_offset.rt = read_reg_value(operands[OPERAND_1]);
    inst.dt_imm_offset.sf = is_bit_mode_64(operands[OPERAND_1]); // POTENTIALLY SOURCE OF ERROR IF THIS IS "RZR" ZERO REGISTER.

    if (operands[OPERAND_3] == NULL && operands[OPERAND_2][FST_CHAR_INDEX] != OPEN_SQUARE_BRACKET) {
        //Load Literal:
        //Sets the bits that are always set for these instructions
        set_bit(inst.dt_load_literal.op0_1);
        set_bit(inst.dt_load_literal.NIL_3);

        if (is_label_literal(operands[OPERAND_2])) {
            inst.dt_load_literal.simm19 = symbol_table_get_address(current_address, operands[OPERAND_2]);
        } else if (is_immediate(operands[OPERAND_2])) {
            inst.dt_load_literal.simm19 = read_imm_value(operands[OPERAND_2]) / INSTR_SIZE;
        } else {
            fprintf(stderr, "Unknown operand: %s\n", operands[OPERAND_2]);
            exit(EXIT_FAILURE);
        }

        return inst.data;
    } 
    //Not load literal:
    //Sets the bits that are always set for these instructions
    set_bit(inst.dt_imm_offset.id);
    set_bit(inst.dt_imm_offset.NIL_5);
    set_bit(inst.dt_imm_offset.NIL_4);
    set_bit(inst.dt_imm_offset.op0_1);

    inst.dt_imm_offset.L = strcmp(opcode, opcode_names[OP_LDR]) == 0;
    inst.dt_imm_offset.xn = read_reg_value(operands[OPERAND_2]+1); // +1 to ignore the "[" at the front.

    // Zero offset
    if (operands[OPERAND_3] == NULL){
        set_bit(inst.dt_imm_offset.U);
        return inst.data;
    }

    int len = strlen(operands[OPERAND_3]);
    // Pre Index - e.g. OPERAND 3: #0x1]!
    if (is_pre_index(operands[OPERAND_3])){
        set_bit(inst.dt_pre_post_index.I);
        set_bit(inst.dt_pre_post_index.NIL_1);
        operands[OPERAND_3][len - 2] = TERMINATION_CHARACTER; // Remove the square bracket and exclamation mark.
        inst.dt_pre_post_index.simm9 = read_imm_value(operands[OPERAND_3]);
        return inst.data;
    }

    // Post Index - e.g. OPERAND 3: #226
    if (is_immediate(operands[OPERAND_3]) && operands[OPERAND_3][len - 1] != CLOSED_SQUARE_BRACKET){
        set_bit(inst.dt_pre_post_index.NIL_1);
        inst.dt_pre_post_index.simm9 = read_imm_value(operands[OPERAND_3]);
        return inst.data;
    }

    // Unsigned Immediate Offset - e.g. OPERAND 2: #0x8]
    if (is_immediate(operands[OPERAND_3])){
        set_bit(inst.dt_imm_offset.U);
        unsigned int immediate = read_imm_value(operands[OPERAND_3]);
        if (inst.dt_imm_offset.sf){
            inst.dt_imm_offset.imm12 = immediate / 8;
        } else{
            inst.dt_imm_offset.imm12 = immediate / 4;
        }
        return inst.data;
    }

    // Register Offset Index - e.g OPERAND 3: x15]
    if (operands[OPERAND_3][len - 1] == CLOSED_SQUARE_BRACKET){
        set_bit(inst.dt_reg_offset.id2);
        inst.dt_reg_offset.NIL_1 = DT_REG_PATTERN;
        operands[OPERAND_3][len - 1] = TERMINATION_CHARACTER; // Remove the square bracket.
        inst.dt_reg_offset.xm = read_reg_value(operands[OPERAND_3]);
        return inst.data;
    }

    fprintf(stderr, "ERROR: Unknown load/store type received: %s\n", opcode);
    exit(EXIT_FAILURE);
}

/* Assembles b, br, b.cond instructions. Any aliases are converted beforehand. */
static uint32_t assemble_branch(char *opcode, char **operands){
    //PRECONDITION: At least 1 operand:
    
    assert_num_opcodes(operands, MIN_BRANCH_OPERANDS);
    
    // Initialise instruction
    Instruction inst;
    inst.data = 0;

    inst.branch_conditional.op0 = ITP_BRANCH;

    if (strcmp(opcode, opcode_names[OP_B]) == 0){
        inst.branch_unconditional.id = ITP_BRANCH_UNCOND;

        assert_msg(is_label_literal(operands[OPERAND_1]), "First operand: %s is not a label\n", operands[OPERAND_1]);
        inst.branch_unconditional.simm26 = symbol_table_get_address(current_address, operands[OPERAND_1]);

        return inst.data;
    }

    if (strncmp(opcode, opcode_names[OP_B_COND], 2) == 0){
        inst.branch_conditional.id = ITP_BRANCH_COND;
        inst.branch_conditional.cond = read_branch_cond_type(opcode);
        
        assert_msg(is_label_literal(operands[OPERAND_1]), "First operand: %s is not a label\n", operands[OPERAND_1]);
        inst.branch_conditional.simm19 = symbol_table_get_address(current_address, operands[OPERAND_1]);

        return inst.data;
    }

     if (strcmp(opcode, opcode_names[OP_BR]) == 0){
        inst.branch_register.id = ITP_BRANCH_REG;
        inst.branch_register.NIL_3 = BR_REG_PATTERN;
        inst.branch_register.xn = read_reg_value(operands[OPERAND_1]);
        return inst.data;
    }

    fprintf(stderr, "ERROR: Unknown branch instruction type received: %s\n", opcode);
    exit(EXIT_FAILURE);
}

// ----------------------------------------MAIN FUNCS:---------------------------------------
/**
 * @brief Converts alias instructions to their assembleable counterparts.
 *
 * This function takes an opcode and its operands and converts the any alias instructions
 * to counterpart which is then able to be assembled as defined by the specification. It modifies
 * the opcode and operands in-place to reflect the conversion.
 *
 * @param opcode The opcode to convert.
 * @param operands An array of operands associated with the opcode.
 */
static void convert_aliases(char *opcode, char **operands){
    // All of these could potentially have a shift so this check is made so only one "shift" check is made:
    if (is_opcode(opcode, (Opcode[]) {OP_NEG, OP_NEGS, OP_CMP, OP_CMN, OP_TST}, NUM_SHIFT_ALIAS_INSTS)){
        //Check for shift
        if (operands[OPERAND_3] != NULL){
            // Move shift operands across to make space for new operand
            operands[OPERAND_5] = operands[OPERAND_4];
            operands[OPERAND_4] = operands[OPERAND_3];
        }
        
        // neg rd, <op2> == sub rd, rzr, <op2>
        if (strcmp(opcode, opcode_names[OP_NEG]) == 0){
            strcpy(opcode, opcode_names[OP_SUB]);
            operands[OPERAND_3] = operands[OPERAND_2];
            operands[OPERAND_2] = ZERO_REGISTER;
            return;
        }
        
        // negs rd, <op2> == subs rd, rzr, <op2>
        if (strcmp(opcode, opcode_names[OP_NEGS]) == 0){
            strcpy(opcode, opcode_names[OP_SUBS]);
            operands[OPERAND_3] = operands[OPERAND_2];
            operands[OPERAND_2] = ZERO_REGISTER;
            return;
        }
        
        // cmn rn, <op2> == adds rzr, rn, <op2>
        if (strcmp(opcode, opcode_names[OP_CMN]) == 0){
            strcpy(opcode, opcode_names[OP_ADDS]);
            operands[OPERAND_3] = operands[OPERAND_2];
            operands[OPERAND_2] = operands[OPERAND_1];
            operands[OPERAND_1] = ZERO_REGISTER;
            return;
        }
        
        // cmp rn, <op2> == subs rzr, rn, <op2>
        if (strcmp(opcode, opcode_names[OP_CMP]) == 0){
            strcpy(opcode, opcode_names[OP_SUBS]);
            operands[OPERAND_3] = operands[OPERAND_2];
            operands[OPERAND_2] = operands[OPERAND_1];
            operands[OPERAND_1] = ZERO_REGISTER;
            return;
        }
        
    }
    
    // mul rd, rn, rm == madd rd, rn, rm, rzr
    if (strcmp(opcode, opcode_names[OP_MUL]) == 0){
        strcpy(opcode, opcode_names[OP_M_ADD]);
        operands[OPERAND_4] = ZERO_REGISTER;
        return;
    }
    
    // mneg rd, rn, rm == msub rd, rn, rm, rzr
    if (strcmp(opcode, opcode_names[OP_M_NEG]) == 0){
        strcpy(opcode, opcode_names[OP_M_SUB]);
        operands[OPERAND_4] = ZERO_REGISTER;
        return;
    }
    
    // tst rn, <op2> == ands rzr, rn, <op2>
    if (strcmp(opcode, opcode_names[OP_TST]) == 0){
        strcpy(opcode, opcode_names[OP_ANDS]);
        operands[OPERAND_3] = operands[OPERAND_2];
        operands[OPERAND_2] = operands[OPERAND_1];
        operands[OPERAND_1] = ZERO_REGISTER;
        return;
    }
    
    // mvn rn, <op2> == orn rd, rzr, <op2>
    if (strcmp(opcode, opcode_names[OP_MVN]) == 0){
        strcpy(opcode, opcode_names[OP_ORN]);
        operands[OPERAND_3] = operands[OPERAND_2];
        operands[OPERAND_2] = ZERO_REGISTER;
        return;
    }
    
    // mov rn, rm == orr rd, rzr, rm
    if (strcmp(opcode, opcode_names[OP_MOV]) == 0){
        strcpy(opcode, opcode_names[OP_ORR]);
        operands[OPERAND_3] = operands[OPERAND_2];
        operands[OPERAND_2] = ZERO_REGISTER;
        return;
    }
}

/**
 * Determines the opcode type based on the given opcode string and assembles
 * the corresponding instruction using the provided operands.
 *
 * @param opcode The opcode string to determine and assemble.
 * @param operands Array of operand strings associated with the opcode.
 * @return The assembled 32-bit instruction.
 * @remarks This function determines the opcode type and delegates assembly
 *          to specific functions based on the opcode type. It handles various
 *          instruction types such as add/sub, multiplication, logic operations,
 *          wide moves, load/store, branches, and reports errors for unsupported
 *          opcode types.
 */
static uint32_t determine_and_assemble(char *opcode, char **operands){
    if (is_directive(opcode)) {
        assert_msg(is_int_directive(opcode), "Unknown directive\n");
        return read_imm_value(operands[OPERAND_1]);
    }

    if (is_opcode(opcode, (Opcode[]) {OP_ADD, OP_ADDS, OP_SUB, OP_SUBS}, NUM_ADD_SUB_INSTS)){
        return assemble_add_sub(opcode, operands);
    }
    if (is_opcode(opcode, (Opcode[]) {OP_M_ADD, OP_M_SUB}, NUM_MUL_INSTS)){
        return assemble_multiply(opcode, operands);
    }
    if (is_opcode(opcode, (Opcode[]) {OP_AND, OP_ANDS, OP_BIC, OP_BICS, OP_ORN, OP_ORR, OP_EOR, OP_EON}, NUM_LOGIC_INSTS)){
        return assemble_logic(opcode, operands);
    }
    if (is_opcode(opcode, (Opcode[]) {OP_MOVN, OP_MOVZ, OP_MOVK}, NUM_WIDE_MOVE_INSTS)){
        return assemble_wide_move(opcode, operands);
    }

    if (is_opcode(opcode, (Opcode[]) {OP_LDR, OP_STR}, NUM_LOAD_STORE_INSTS)){
        return assemble_load_store(opcode, operands);
    }

    if (is_opcode(opcode, (Opcode[]) {OP_B, OP_BR}, NUM_BRANCH_INSTS) || strncmp(opcode, opcode_names[OP_B_COND], 2) == 0){
        return assemble_branch(opcode, operands);
    }
    fprintf(stderr, "ERROR: Haven't implemented this function type: %s\n", opcode);
    exit(EXIT_FAILURE);
}

/**
 * Decodes an assembly line input, extracts opcode and operands,
 * converts opcode aliases, assembles the instruction, and returns
 * the assembled 32-bit instruction.
 *
 * @param assembly_line_input The assembly line input string to decode.
 * @return The assembled 32-bit instruction.
 * @remarks This function processes the assembly line input to determine
 *          the opcode and its associated operands. It handles special cases
 *          such as HALT instructions and labels, prints errors for unhandled
 *          labels, and converts opcode aliases before assembling the final  } else if (is_immediate(operands[OPERAND_2])) {
            inst.dt_load_literal.simm19 = read_imm_value(operands[OPERAND_2]) / INSTR_SIZE;
        } else {
            fprintf(stderr, "Unknown operand: %s\n", operands[OPERAND_2]);
            exit(EXIT_FAILURE);
        }

       
 *          instruction using the `determine_and_assemble` function.
 */
void decode(char *assembly_line_input){
    strtok(assembly_line_input, "/");

    // Initialise operands
    char opcode[OPCODE_SIZE];
    char *operands[MAX_NUM_OPERANDS];
    for (int i = 0; i < MAX_NUM_OPERANDS; ++i) {
        operands[i] = NULL;
    }

    // Extract the opcode/branch name segment
    char *segment = strtok(assembly_line_input, ", ");

    if (is_label(segment)) {
        segment[strlen(segment) - 1] = '\0'; //remove the :
        symbol_table_add_label(instructions, current_address, segment); //add to the symbol tabel with current_address one instruction below the label
        return;
    }

    strcpy(opcode, segment);

    int num_ops = 0;
    
    // Loop through the string to extract all other segments
    while((segment = strtok(NULL, ", ")) != NULL) {
        operands[num_ops] = segment;
        num_ops++;
    }
 
    convert_aliases(opcode, operands);

    // Print the segmented opcode and operands for debugging reasons:
    debug_printf("OPCODE:    %s\n", opcode);
    for (int i = 0; i < num_ops; i++) {
        debug_printf("OPERAND %d: %s\n", i+1, operands[i]);
    }

    // Make space in instructions buffer for new instruction
    uint32_t *inst = malloc(sizeof(uint32_t));
    assert_msg(inst != NULL, "Memory allocation failed\n");

    // Assemble instruction
    *inst = determine_and_assemble(opcode, operands);

    // Adds assembled instruction to instructions buffer
    darray_add(instructions, inst);

    // Increase current current_address
    current_address += INSTR_SIZE;
}

void decode_debug(char *assembly_line_input, HashMap* address_to_line, uint32_t line_num){
    //Check if line is an empty line:
    if (strcmp(assembly_line_input, "") == 0){
        return;
    }

    int address_before = current_address;
    // Make a copy for debugger so "assembly lines" does not get affected (and only show opcodes)
    char *asm_line_copy = strdup(assembly_line_input);
    assert_msg(asm_line_copy != NULL, "MEMORY ERROR: strdup failed\n");

    // Strips in line comments after code. 
    strtok(asm_line_copy, "/");

    decode(asm_line_copy);
    //Check if input is label, if so do nothing. Current address will not change via label due to an early return.
    if (address_before == current_address){
        return;
    }
    
    // Convert address to string and line_num to a pointer
    char *str_address = int_to_string(address_before);
    int32_t* line_num_ptr = malloc_assert_num(line_num);

    hashmap_set(address_to_line, str_address, line_num_ptr);
}

/**
 * @brief Retrieve the array of decoded instructions.
 * @return A pointer to the array of decoded instructions.
 */
DArray *decode_get_instructions(void) {
    return instructions;
}

/**
 * The `decode_free` function frees memory used by a symbol table and an array of instructions.
 */
void decode_free(void) {
    symbol_table_free();
    darray_free(instructions);
}