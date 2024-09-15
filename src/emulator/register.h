/**
 * @file register.h
 * @brief Declarations for register struct and getters and setters.
 * @details This header file contains the function declarations for register struct and
 *          its getters and setters. These are required to keep track of which register
 *          mode was last written to.
 */
#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>

#define NUM_REGISTERS 31

typedef enum {ZERO_REGISTER, PROGRAM_COUNTER, STACK_POINTER} SpecRegisterType;

// ---------------------------GETTERS AND SETTERS-------------------------

extern void init_register(void);

extern void set_reg_value(uint32_t reg_num, uint64_t value);
extern uint32_t get_reg_value_32(uint32_t reg_num);
extern uint64_t get_reg_value_64(uint32_t reg_num);

extern uint64_t get_spec_register(SpecRegisterType reg_type);
extern void set_spec_register(SpecRegisterType reg_type, uint64_t value);

extern void increase_pc(int64_t offset);
extern void increment_pc(void);

extern void print_registers(FILE* output_file);

#endif /* REGISTER_H */
