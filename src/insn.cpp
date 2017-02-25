#include "insn.h"

#include <stdio.h>

#include "cpu.h"

#define CRN(regname) (cpu->control.regname)
#define GRN(regname) (cpu->gpr.regname)
#define CR(num) (*((uint32_t*)&cpu->control + num))
#define GR(num) (*((uint32_t*)&cpu->gpr + num))

static void i_jmp_target24(Cpu *cpu, uint32_t x) {
	uint32_t T = (x & 0b0000011111110000) >> 4;
	uint32_t t = x >> 16;
	uint32_t target24 = (t << 8) | (T << 1);
	CRN(pc) = (CRN(pc) & 0b1111000000000000) | target24;
}

static void i_ldc(Cpu *cpu, uint32_t x) {
	uint32_t n = (x >> 8) & 0b1111;
	uint32_t i = (x >> 4) & 0b1111;
	uint32_t I = x & 0b1;
	uint32_t imm5 = (I << 4) | i;
	GR(n) = CR(imm5);
	// TODO
// 	In case that the PC value is read by ldc $0,$pc, the PC+2 of the ldc instruction is read.
// ?  REPEAT/EREPEAT instruction: When the PC value is read by the ldc in the last instruction of a repeat
// block, the value is undefined.
// ?  DBG and DEPC registers: Access to the DBG and DEPC registers in the normal mode is prohibited.
}

static void i_movh(Cpu *cpu, uint32_t x) {
	uint32_t imm16 = (x & 0xFFFF0000) >> 16;
	uint32_t n = (x & 0b111100000000) >> 8;
	GR(n) = imm16 << 16;
}

static void i_di(Cpu *cpu, uint32_t x) {
	printf("TODO: di\n");
}

static void i_mov_imm8(Cpu *cpu, uint32_t x) {
	uint32_t imm8 = x & 0xFF;
	uint32_t n = (x >> 8) & 0xF;
	uint32_t s = (n & 0x80) ? 0xffffff00 : 0;
	GR(n) = s | imm8;
}

static void i_movu_imm16(Cpu *cpu, uint32_t x) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t imm16 = (x >> 16);
	GR(n) = imm16;
}

static void i_movu_imm24(Cpu *cpu, uint32_t x) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t imm24 = (x >> 8);
	GR(n) = imm24;
}

insn_t insn_table[] = {
	{ 4, 0b1111100000001111, 0b1101100000001000, i_jmp_target24 },
	{ 2, 0b1111000000001110, 0b0111000000001010, i_ldc },
	{ 4, 0b1111000011111111, 0b1100000000100001, i_movh },
	{ 2, 0b1111111111111111, 0b0111000000000000, i_di },
	{ 2, 0b1111000000000000, 0b0101000000000000, i_mov_imm8 },
	{ 4, 0b1111000011111111, 0b1100000000010001, i_movu_imm16 },
	{ 4, 0b1111100000000000, 0b1101000000000000, i_movu_imm24 },
};

insn_t *insn_decode(uint32_t insn_i) {
	for (auto &insn : insn_table)
		if ((insn.mask & insn_i) == insn.match)
			return &insn;
	return nullptr;
}
