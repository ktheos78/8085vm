#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "opcodes.h"
#include "cpu.h"

InstrFunc opcode_table[256];

void update_flags(uint16_t res, uint8_t op_type)
{
    // carry flag (CY)
    switch (op_type)
    {
        case OP_ARITHMETIC:
            if (res & 0x0100)
                flags |= FL_CY;
            else
                flags &= ~FL_CY;
            break;

        // logical ops always clear CY
        case OP_LOGICAL:
            flags &= ~FL_CY;
            break;

        // INR and DCR don't touch CY
        case OP_INRDCR:
            break;
    }

    // parity flag (P) - set if even number of 1s
    uint8_t count = 0;
    for (uint8_t i = 0; i < 8; ++i)
        if (res & (1 << i)) count++;

    if (count & 1)
        flags &= ~FL_P;
    else
        flags |= FL_P; 

    // zero flag (Z)
    if ((res & 0x00FF) == 0)
        flags |= FL_Z;
    else
        flags &= ~FL_Z;

    // sign flag (S)
    if (res & 0x0080)
        flags |= FL_S;
    else
        flags &= ~FL_S;

}

void op_mov(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t dst = (opcode >> 3) & 0x07;

    if (src == R_MEM)
        regs[dst] = memory[(regs[R_H] << 8) | regs[R_L]];
    else if (dst == R_MEM)
        memory[(regs[R_H] << 8) | regs[R_L]] = regs[src];
    else
        regs[dst] = regs[src];
}

void op_mvi(void)
{
    uint8_t dst = (opcode >> 3) & 0x07;
    
    if (dst == R_MEM)
        memory[(regs[R_H] << 8) | regs[R_L]] = memory[PC++];
    else
        regs[dst] = memory[PC++];
}

void op_add(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint16_t res;

    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];

    res = regs[R_A] + data;
    regs[R_A] = (uint8_t)res;      

    update_flags(res, OP_ARITHMETIC);
}

void op_adc(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint16_t res;

    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];

    res = regs[R_A] + data + ((flags & FL_CY) ? 1 : 0);
    regs[R_A] = (uint8_t)res;      

    update_flags(res, OP_ARITHMETIC);
}

void op_inr(void)
{
    uint8_t dst = (opcode >> 3) & 0x07;
    uint16_t res;

    if (dst == R_MEM)
    {
        uint16_t addr = (regs[R_H] << 8) | regs[R_L];
        res = memory[addr] + 1;
        memory[addr]++;
    }

    else
    {
        res = regs[dst] + 1;
        regs[dst]++;
    }

    update_flags(res, OP_INRDCR);
}

void op_dcr(void)
{
    uint8_t dst = (opcode >> 3) & 0x07;
    uint16_t res;

    if (dst == R_MEM)
    {
        uint16_t addr = (regs[R_H] << 8) | regs[R_L];
        res = memory[addr] - 1;
        memory[addr]--;
    }

    else
    {
        res = regs[dst] - 1;
        regs[dst]--;
    }

    update_flags(res, OP_INRDCR);
}

void op_ana(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint8_t res;
    
    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];

    res = regs[R_A] & data;
    regs[R_A] &= data;

    update_flags(res, OP_LOGICAL);
}

void op_xra(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint8_t res;
    
    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];

    res = regs[R_A] ^ data;
    regs[R_A] ^= data;

    update_flags(res, OP_LOGICAL);
}

void op_ora(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint8_t res;

    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];

    res = regs[R_A] | data;
    regs[R_A] |= data;

    update_flags(res, OP_LOGICAL);
}

void op_cmp(void)
{
    uint8_t src = opcode & 0x07;
    uint8_t data;
    uint16_t res;

    uint16_t addr = (regs[R_H] << 8) | regs[R_L];
    data = (src == R_MEM) ? memory[addr] : regs[src];
    res = (uint16_t)regs[R_A] - data;

    update_flags(res, OP_ARITHMETIC);
}

void op_lxi(void)
{
    uint8_t rp = (opcode >> 4) & 0x03;
    uint8_t data_low = memory[PC++];
    uint8_t data_high = memory[PC++];
    
    set_rp(rp, (data_high << 8) | data_low);
}

void op_ldax(void)
{
    uint8_t rp = (opcode >> 4) & 0x03;
    regs[R_A] = memory[get_rp(rp)];     // A <- (RP)
}

void op_stax(void)
{
    uint8_t rp = (opcode >> 4) & 0x03;
    memory[get_rp(rp)] = regs[R_A];     // (RP) <- A
}

void op_inx(void)
{
    uint16_t res;
    uint8_t rp = (opcode >> 4) & 0x03;

    res = get_rp(rp) + 1;
    set_rp(rp, res);
}

void op_dcx(void)
{
    uint16_t res;
    uint8_t rp = (opcode >> 4) & 0x03;

    res = get_rp(rp) - 1;
    set_rp(rp, res);
}

void op_nop(void)
{
    return;
}

void op_hlt(void)
{
    running = 0;
}

void op_push(void)
{
    uint8_t rp = (opcode >> 4) & 0x03;
    uint16_t val = get_rp(rp);

    memory[--SP] = (val >> 8) & 0xFF;
    memory[--SP] = val & 0xFF;
}

void op_pop(void)
{
    uint8_t rp = (opcode >> 4) & 0x03;
    uint8_t low = memory[SP++];
    uint8_t high = memory[SP++];

    set_rp(rp, (high << 8) | low);
}

void op_jmp(void)
{
    uint8_t cond = (opcode >> 3) & 0x07;
    uint8_t take_jump;
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];

    switch (cond)
    {
        case COND_ALWAYS:
            take_jump = 1;
            break;

        case COND_Z:
            take_jump = flags & FL_Z;
            break;

        case COND_NZ:
            take_jump = (flags & FL_Z) == 0;
            break;

        case COND_C:
            take_jump = flags & FL_CY;
            break;

        case COND_NC:
            take_jump = (flags & FL_CY) == 0;
            break;

        default:
            take_jump = 0;
            break;
    }

    if (take_jump)
        PC = (addr_high << 8) | addr_low;
}

void op_call(void)
{
    uint8_t cond = (opcode >> 3) & 0x07;
    uint8_t take_call;
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];

    switch (cond)
    {
        case COND_ALWAYS:
            take_call = 1;
            break;

        case COND_Z:
            take_call = flags & FL_Z;
            break;

        case COND_NZ:
            take_call = (flags & FL_Z) == 0;
            break;

        case COND_C:
            take_call = flags & FL_CY;
            break;

        case COND_NC:
            take_call = (flags & FL_CY) == 0;
            break;

        default:
            take_call = 0;
            break;
    }

    if (take_call)
    {
        // store next instruction PC in stack
        memory[--SP] = (PC >> 8) & 0xFF;
        memory[--SP] = PC & 0xFF;

        PC = (addr_high << 8) | addr_low;
    }
}

void op_ret(void)
{
    uint8_t cond = (opcode >> 3) & 0x07;
    uint8_t take_ret;

    switch (cond)
    {
        case COND_ALWAYS:
            take_ret = 1;
            break;

        case COND_Z:
            take_ret = flags & FL_Z;
            break;

        case COND_NZ:
            take_ret = (flags & FL_Z) == 0;
            break;

        case COND_C:
            take_ret = flags & FL_CY;
            break;

        case COND_NC:
            take_ret = (flags & FL_CY) == 0;
            break;

        default:
            take_ret = 0;
            break;
    }

    if (take_ret)
    {
        uint8_t low = memory[SP++];
        uint8_t high = memory[SP++];

        // restore PC
        PC = (high << 8) | low;
    }
}

void op_lda(void)
{
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];

    regs[R_A] = memory[(addr_high << 8) | addr_low];
}

void op_sta(void)
{
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];

    memory[(addr_high << 8) | addr_low] = regs[R_A];
}

void op_lhld(void)
{
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];
    
    regs[R_L] = memory[(addr_high << 8) | addr_low];
    regs[R_H] = memory[((addr_high << 8) | addr_low) + 1];
}

void op_shld(void)
{
    uint8_t addr_low = memory[PC++];
    uint8_t addr_high = memory[PC++];
    
    memory[(addr_high << 8) | addr_low] = regs[R_L];
    memory[((addr_high << 8) | addr_low) + 1] = regs[R_H];
}

void op_xchg(void)
{
    uint16_t temp = get_rp(RP_HL);
    set_rp(RP_HL, get_rp(RP_DE));
    set_rp(RP_DE, temp);
}

void op_adi(void)
{
    uint16_t res = (uint16_t)regs[R_A];
    uint8_t data = memory[PC++];

    res += data;
    regs[R_A] = (uint8_t)res;

    update_flags(res, OP_ARITHMETIC);
}

void op_aci(void)
{
    uint16_t res = (uint16_t)regs[R_A];
    uint8_t data = memory[PC++];

    res += data + ((flags & FL_CY) ? 1 : 0);
    regs[R_A] = (uint8_t)res;

    update_flags(res, OP_ARITHMETIC);
}

void op_sui(void)
{
    uint16_t res = (uint16_t)regs[R_A];
    uint8_t data = memory[PC++];

    res -= data;
    regs[R_A] = (uint8_t)res;

    update_flags(res, OP_ARITHMETIC);
}

void op_ani(void)
{
    uint8_t data = memory[PC++];
    uint16_t res = (uint16_t)regs[R_A] & data;
    regs[R_A] &= data;

    update_flags(res, OP_LOGICAL);
}

void op_xri(void)
{
    uint8_t data = memory[PC++];
    uint16_t res = (uint16_t)regs[R_A] ^ data;
    regs[R_A] ^= data;

    update_flags(res, OP_LOGICAL);
}

void op_ori(void)
{
    uint8_t data = memory[PC++];
    uint16_t res = (uint16_t)regs[R_A] | data;
    regs[R_A] |= data;

    update_flags(res, OP_LOGICAL);
}

void op_cpi(void)
{
    uint8_t data = memory[PC++];
    uint16_t res = (uint16_t)regs[R_A] - data;
    update_flags(res, OP_ARITHMETIC);
}

void op_rlc(void)
{
    uint8_t a7 = (regs[R_A] & 0x80) ? 1 : 0;
    regs[R_A] = (regs[R_A] << 1) | a7;
    flags = a7 ? (flags | FL_CY) : (flags & ~FL_CY);
}

void op_rrc(void)
{
    uint8_t a0 = (regs[R_A] & 0x01) ? 1 : 0;
    regs[R_A] = (regs[R_A] >> 1) | (a0 << 7);
    flags = a0 ? (flags | FL_CY) : (flags & ~FL_CY);
}

void op_ral(void)
{
    uint8_t a7 = (regs[R_A] & 0x80) ? 1 : 0;
    regs[R_A] = (regs[R_A] << 1) | (flags & FL_CY);
    flags = a7 ? (flags | FL_CY) : (flags & ~FL_CY);
}

void op_rar(void)
{
    uint8_t a0 = (regs[R_A] & 0x01) ? 1 : 0;
    regs[R_A] = (regs[R_A] >> 1) | ((flags & FL_CY) << 7);
    flags = a0 ? (flags | FL_CY) : (flags & ~FL_CY);
}

void init_opcodes(void)
{
    // initialize opcodes
    for (uint16_t i = 0; i <= 0xFF; ++i)
        opcode_table[i] = NULL;

    // MOV -> 01DDDSSS
    for (uint8_t i = 0x40; i <= 0x7F; ++i)
        opcode_table[i] = &op_mov;

    for (uint8_t i = 0; i <= 7; ++i)
    {
        opcode_table[0x06 | (i << 3)] = &op_mvi;    // MVI -> 00DDD110

        opcode_table[0x80 | i] = &op_add;           // ADD -> 10000SSS
        opcode_table[0x88 | i] = &op_adc;           // ADC -> 10001SSS

        opcode_table[0x04 | (i << 3)] = &op_inr;    // INR -> 00DDD100
        opcode_table[0x05 | (i << 3)] = &op_dcr;    // DCR -> 00DDD101

        opcode_table[0xA0 | i] = &op_ana;           // ANA -> 10100SSS
        opcode_table[0xA8 | i] = &op_xra;           // XRA -> 10101SSS
        opcode_table[0xB0 | i] = &op_ora;           // ORA -> 10110SSS
        opcode_table[0xB8 | i] = &op_cmp;           // CMP -> 10111SSS

        opcode_table[0xC2 | (i << 3)] = &op_jmp;    // JMP -> 11CCC010
        opcode_table[0xC4 | (i << 3)] = &op_call;   // CALL -> 11CCC100
        opcode_table[0xC0 | (i << 3)] = &op_ret;    // RET -> 11CCC000
    }
    
    for (uint8_t i = 0; i <= 3; ++i)
    {
        opcode_table[0x01 | (i << 4)] = &op_lxi;    // LXI -> 00RP0001
        opcode_table[0x0A | (i << 4)] = &op_ldax;   // LDAX -> 00RP1010
        opcode_table[0x02 | (i << 4)] = &op_stax;   // STAX -> 00RP0010

        opcode_table[0x03 | (i << 4)] = &op_inx;    // INX -> 00RP0011 
        opcode_table[0x0B | (i << 4)] = &op_dcx;    // DCX -> 00RP1011

        opcode_table[0xC5 | (i << 4)] = &op_push;          // PUSH -> 11RP0101
        opcode_table[0xC1 | (i << 4)] = &op_pop;           // POP -> 11RP0001
    }

    opcode_table[0x00] = &op_nop;   // NOP -> 0x00
    opcode_table[0x76] = &op_hlt;   // HLT -> 0x76

    opcode_table[0x3A] = &op_lda;   // LDA -> 0x3A
    opcode_table[0x32] = &op_sta;   // STA -> 0x32
    opcode_table[0x2A] = &op_lhld;  // LHLD -> 0x2A
    opcode_table[0x22] = &op_shld;  // SHLD -> 0x22
    opcode_table[0xEB] = &op_xchg;  // XCHG -> 0xEB

    opcode_table[0xC6] = &op_adi;   // ADI -> 0xC6
    opcode_table[0xCE] = &op_aci;   // ACI -> 0xCE
    opcode_table[0xD6] = &op_sui;   // SUI -> 0xD6

    opcode_table[0xE6] = &op_ani;   // ANI -> 0xE6
    opcode_table[0xEE] = &op_xri;   // XRI -> 0xEE
    opcode_table[0xF6] = &op_ori;   // ORI -> 0xF6
    opcode_table[0xFE] = &op_cpi;   // CPI -> 0xFE

    opcode_table[0x07] = &op_rlc;   // RLC -> 0x07
    opcode_table[0x0F] = &op_rrc;   // RRC -> 0x0F
    opcode_table[0x17] = &op_ral;   // RAL -> 0x17
    opcode_table[0x1F] = &op_rar;   // RAR -> 0x1F
}

uint16_t get_rp(int rp)
{
    switch (rp)
    {
        case RP_BC:
            return (regs[R_B] << 8) | regs[R_C];

        case RP_DE:
            return (regs[R_D] << 8) | regs[R_E];

        case RP_HL:
            return (regs[R_H] << 8) | regs[R_L];

        case RP_SP:
            return SP;

        case RP_PSW:
            return (regs[R_A] << 8) | flags;

        default:
            return -1;
    }
}

void set_rp(int rp, uint16_t val)
{
    switch (rp)
    {
        case RP_BC:
            regs[R_C] = val & 0xFF;
            regs[R_B] = (val >> 8) & 0xFF;
            return;

        case RP_DE:
            regs[R_E] = val & 0xFF;
            regs[R_D] = (val >> 8) & 0xFF;
            return;

        case RP_HL:
            regs[R_L] = val & 0xFF;
            regs[R_H] = (val >> 8) & 0xFF;
            return;

        case RP_SP:
            SP = val;
            return;

        case RP_PSW:
            flags = val & 0xFF;
            regs[R_A] = (val >> 8) & 0xFF;
            return;

        default:
            fprintf(stderr, "Error: invalid register pair\n");
            return;
    }
}