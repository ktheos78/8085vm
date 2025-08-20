#ifndef OPCODES_H_
#define OPCODES_H_

#include <stdint.h>
#include "cpu.h"

#define OP_ARITHMETIC 0
#define OP_LOGICAL 1
#define OP_INRDCR 2 

#define COND_ALWAYS 0b000
#define COND_Z 0b001
#define COND_NZ 0b010
#define COND_C 0b011
#define COND_NC 0b100

typedef void (*InstrFunc) (void);

extern InstrFunc opcode_table[256];
extern uint8_t memory[MEMORY_MAX];
extern uint8_t regs[R_COUNT];
extern uint8_t flags;
extern uint8_t opcode;
extern uint16_t PC;
extern uint16_t SP;
extern uint8_t running;

void init_opcodes(void);
uint16_t get_rp(int rp);
void set_rp(int rp, uint16_t val);
void push_helper(uint16_t val);

void op_mov(void);
void op_mvi(void);
void op_add(void);
void op_adc(void);
void op_inr(void);
void op_dcr(void);
void op_ana(void);
void op_xra(void);
void op_ora(void);
void op_cmp(void);
void op_lxi(void);
void op_ldax(void);
void op_stax(void);
void op_inx(void);
void op_dcx(void);
void op_nop(void);
void op_hlt(void);
void op_call(void);
void op_ret(void);
void op_jmp(void);
void op_jnz(void);
void op_jz(void);
void op_jnc(void);
void op_jc(void);
void op_lda(void);
void op_sta(void);
void op_lhld(void);
void op_shld(void);
void op_xchg(void);
void op_adi(void);
void op_aci(void);
void op_sui(void);
void op_ani(void);
void op_xri(void);
void op_ori(void);
void op_cpi(void);
void op_rlc(void);
void op_rrc(void);
void op_ral(void);
void op_rar(void);

#endif /* OPCODES_H_ */