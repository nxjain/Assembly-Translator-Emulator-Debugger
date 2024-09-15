/**
 * @file instructions.h
 * @brief Declarations for structs and unions used in splitting instructions into segments.
 * @details This header file contains the function declarations for splitting an instruction
 *          as well as the all the struct definitions and the instruction union definition.
 *          Each struct uses bit fields to segment the instructions into the different fields
 *          required for executing the instruction, as specified by the specification.
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

// The meanings for all fields are explained by the specification if not explained here.
// All fields beginning with "NIL" are bits that will not be used

// --------------------------------STRUCTS:-------------------------------

typedef struct {
    unsigned int NIL_1 : 26;
    unsigned int op0 : 3;
    unsigned int NIL_2 : 3;
} GeneralDpImm;

typedef struct {
    unsigned int NIL_1 : 25;
    unsigned int op0 : 3;
    unsigned int NIL_2 : 4;
} GeneralDpReg;

typedef struct {
    unsigned int NIL_1 : 25;
    unsigned int op0_2 : 1;
    unsigned int NIL_2 : 1;
    unsigned int op0_1 : 1;
    unsigned int NIL_3 : 4;
} GeneralDT;

typedef struct {
    unsigned int NIL_1 : 26;
    unsigned int op0 : 3;
    unsigned int NIL_2 : 3;
} GeneralBranch;

typedef struct {
    unsigned int rd : 5;
    unsigned int rn : 5;
    unsigned int imm12: 12;
    unsigned int sh : 1;
    unsigned int opi : 3;
    unsigned int op0 : 3;
    unsigned int opc_flag : 1;  //  the opc into 2 because this bit determines whether flags are modified
    unsigned int opc_op : 1;    // and this bit determines which operation is whether it is subtraction or addition
    unsigned int sf : 1;
} ImmArith;

typedef struct {
    unsigned int rd : 5;
    unsigned int imm16: 16;
    unsigned int hw : 2;
    unsigned int opi : 3;
    unsigned int op0 : 3;
    unsigned int opc : 2;
    unsigned int sf : 1;
} ImmWide;

typedef struct {
    unsigned int rd : 5;
    unsigned int rn : 5;
    unsigned int operand: 6;
    unsigned int rm : 5;
    unsigned int N : 1;   
    unsigned int shift : 2;
    unsigned int id : 1;     // Determines whether it is a register arithmetic instruction or a register logic instruction
    unsigned int op0 : 3;
    unsigned int M : 1;
    unsigned int opc_flag : 1;  //  the opc into 2 because this bit determines whether flags are modified
    unsigned int opc_op : 1;    // and this bit determines which operation is whether it is subtraction or addition
    unsigned int sf : 1;
} RegArith;

typedef struct {
    unsigned int rd : 5;
    unsigned int rn : 5;
    unsigned int operand: 6;
    unsigned int rm : 5;
    unsigned int N : 1;   
    unsigned int shift : 2;
    unsigned int id : 1;     // Determines whether it is a register arithmetic instruction or a register logic instruction
    unsigned int op0 : 3;
    unsigned int M : 1;
    unsigned int opc : 2;
    unsigned int sf : 1;
} RegLogic;

typedef struct {
    unsigned int rd : 5;
    unsigned int rn : 5;
    unsigned int ra: 5;
    unsigned int x: 1;
    unsigned int rm : 5;
    unsigned int opr : 3;
    unsigned int id : 1;
    unsigned int op0 : 3;
    unsigned int M : 1;
    unsigned int opc : 2;
    unsigned int sf : 1;
} RegMultiply;

typedef struct {
    unsigned int rt : 5;
    unsigned int xn : 5;
    unsigned int imm12: 12;
    unsigned int L: 1;
    unsigned int NIL_1 : 1;
    unsigned int U : 1;
    unsigned int op0_2 : 1;
    unsigned int NIL_3 : 1;
    unsigned int op0_1 : 1;
    unsigned int NIL_4 : 1;
    unsigned int NIL_5 : 1;
    unsigned int sf : 1;
    unsigned int id : 1;    // Determines whether the instruction is a load literal instruction or not
} DTImmOffset;

typedef struct {
    unsigned int rt : 5;
    unsigned int xn : 5;
    unsigned int NIL_1 : 6;
    unsigned int xm : 5;
    unsigned int id2 : 1;   // Determines whether the instruction is a dt register offset instruction or not
    unsigned int L: 1;
    unsigned int NIL_2 : 1;
    unsigned int U : 1;
    unsigned int op0 : 4;
    unsigned int NIL_4 : 1;
    unsigned int sf : 1;
    unsigned int id : 1;    // Determines whether the instruction is a load literal instruction or not
} DTRegOffset;

typedef struct {
    unsigned int rt : 5;
    unsigned int simm19: 19;
    unsigned int NIL_1 : 1;
    unsigned int op0_2 : 1;
    unsigned int NIL_2 : 1;
    unsigned int op0_1 : 1;
    unsigned int NIL_3 : 2;
    unsigned int sf : 1;
    unsigned int id : 1;    // Determines whether the instruction is a load literal instruction or not
} DTLoadLiteral;

typedef struct {
    unsigned int rt : 5;
    unsigned int xn : 5;
    unsigned int NIL_1 : 1;
    unsigned int I : 1;
    unsigned int simm9 : 9;
    unsigned int NIL_2 : 1;
    unsigned int L: 1;
    unsigned int NIL_3 : 1;
    unsigned int U : 1;
    unsigned int op0 : 4;
    unsigned int NIL_4 : 1;
    unsigned int sf : 1;
    unsigned int id : 1;    // Determines whether the instruction is a load literal instruction or not
} DTPrePostIndex;

typedef struct {
    unsigned int simm26 : 26;
    unsigned int op0 : 3;
    unsigned int NIL_1 : 1;
    unsigned int id : 2;     // Determines which type of branch instruction it is
} BranchUncond;

typedef struct {
    unsigned int cond : 4;
    unsigned int NIL_1 : 1;
    unsigned int simm19 : 19;
    unsigned int NIL_2 : 2;
    unsigned int op0 : 3;
    unsigned int NIL_3 : 1;
    unsigned int id : 2;     // Determines which type of branch instruction it is
} BranchCond;

typedef struct {
    unsigned int NIL_1 : 5;
    unsigned int xn : 5;
    unsigned int NIL_2 : 6;
    unsigned int NIL_3 : 10;
    unsigned int op0 : 3;
    unsigned int NIL_4 : 1;
    unsigned int id : 2;     // Determines which type of branch instruction it is
} BranchReg;

// ------------------------------SPLIT_UNION:-----------------------------

typedef union {
    uint32_t data;
    GeneralDpImm gen_dp_imm;
    GeneralDpReg gen_dp_reg;
    GeneralDT gen_dt;
    GeneralBranch gen_branch;

    ImmArith imm_arith;
    ImmWide imm_wide;

    RegArith reg_arith;
    RegLogic reg_logic;
    RegMultiply reg_multiply;

    DTImmOffset dt_imm_offset;
    DTRegOffset dt_reg_offset;
    DTLoadLiteral dt_load_literal;
    DTPrePostIndex dt_pre_post_index;

    BranchUncond branch_unconditional;
    BranchCond branch_conditional;
    BranchReg branch_register;

} Instruction;

/*This enum stores the decoding patterns for each instruction type.
E.g. For branch functions, bits 27-25 should equal 101, so BRANCH = 5 so that when comparing bits 27-25 for an instruction
we can use the BRANCH value to see if the instruction is indeed a branch value.*/
enum InstTypePatterns {
    ITP_DP_IMM = 4,            // op0 = ?
    ITP_IMM_ARITH = 2,      // opi = ?
    ITP_WIDE_MOVE = 5,   // opi = ?

    ITP_DP_REG = 5,            // op0 = ?
    ITP_REG_MULTIPLY = 1,   // M = ?
    ITP_REG_ARITH = 1,   // id = ?
    ITP_REG_LOGIC = 0,   // id = ?

    ITP_DT_1 = 1,               // op0_1 = ?
    ITP_DT_2 = 0,               // op0_2 = ?
    ITP_DT_LOAD_LITERAL = 0,    // id = ?
    ITP_DT_IMM_OFFSET = 1,      // U = ?
    ITP_DT_REGISTER_OFFSET = 1, // id2 = ?
    ITP_DT_PRE_INDEX = 1,       // I = ?
    ITP_DT_POST_INDEX = 0,      // I = ?

    ITP_BRANCH = 5,        // op0 = ?
    ITP_BRANCH_UNCOND = 0, // id = ?
    ITP_BRANCH_COND = 1,   // id = ?
    ITP_BRANCH_REG = 3     // id = ?
};

enum BranchConditions {
    ITP_EQ = 0,
    ITP_NE = 1,
    ITP_GE = 10,
    ITP_LT = 11,
    ITP_GT = 12,
    ITP_LE = 13,
    ITP_AL = 14,
};

enum WideMoveOperations {
    ITP_MOVN = 0,
    ITP_MOVZ = 2,
    ITP_MOVK = 3,
};

enum LogicOperations {ITP_AND, ITP_OR, ITP_XOR, ITP_AND_W_FLAGS};

enum ShiftOperations {ITP_LSL, ITP_LSR, ITP_ASR, ITP_ROR};

#endif /* INSTRUCTIONS_H */
