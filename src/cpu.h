#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

#define MEMORY_MAX (1 << 16)
#define STACK_SEGMENT_START 0xE000  // 8KB stack segment

// registers 
enum
{
    R_B = 0,
    R_C,
    R_D,
    R_E,
    R_H,
    R_L,
    R_MEM,
    R_A,
    R_COUNT
};

// register pairs
enum
{
    RP_BC = 0,
    RP_DE,
    RP_HL,
    RP_SP,
    RP_PSW
};

// flags
enum
{
    FL_CY = 1 << 0,
    FL_P = 1 << 2,
    FL_AC = 1 << 4,
    FL_Z = 1 << 6,
    FL_S = 1 << 7
};

extern uint8_t memory[MEMORY_MAX];
extern uint8_t regs[R_COUNT];
extern uint8_t flags;

extern uint16_t PC;
extern uint16_t SP;

#endif /* CPU_H_ */