#include "insn.h"

#include <stdio.h>

#include "cpu.h"

#define CRN(regname) (cpu->control.regname)
#define GRN(regname) (cpu->gpr.regname)
#define CR(num) (*((uint32_t*)&cpu->control + num))
#define GR(num) (*((uint32_t*)&cpu->gpr + num))

#define IMPL(name) static void name(Cpu *cpu, uint32_t x)

#define SignExt(value, from, to) \
	(value | ((value & (1 << (from - 1))) ? (((1 << (to - from)) - 1) << from) : (0)))

IMPL(i_jmp_target24) {
	uint32_t T = (x & 0b0000011111110000) >> 4;
	uint32_t t = x >> 16;
	uint32_t target24 = (t << 8) | (T << 1);
	CRN(pc) = (CRN(pc) & 0b1111000000000000) | target24;
}

IMPL(i_ldc) {
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

IMPL(i_movh) {
	uint32_t imm16 = (x & 0xFFFF0000) >> 16;
	uint32_t n = (x & 0b111100000000) >> 8;
	GR(n) = imm16 << 16;
}

IMPL(i_di) {
	printf("TODO: di\n");
}

IMPL(i_mov_imm8) {
	uint32_t imm8 = x & 0xFF;
	uint32_t n = (x >> 8) & 0xF;
	GR(n) = SignExt(imm8, 8, 32);
}

IMPL(i_movu_imm16) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t imm16 = (x >> 16);
	GR(n) = imm16;
}

IMPL(i_movu_imm24) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t I = x & 0xFF;
	uint32_t i = x >> 16;
	uint32_t imm24 = (i << 8) | I;
	GR(n) = imm24;
}

IMPL(i_sub) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t m = (x >> 4) & 0xF;
	GR(n) = GR(n) - GR(m);
}

IMPL(i_srl_imm5) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t imm5 = (x >> 3) & 0b11111;
	GR(n) = GR(n) >> imm5;
}

IMPL(i_beqz) {
	uint32_t n = (x >> 8) & 0xF;
	int8_t disp8 = x & 0xFF;
	if (GR(n) == 0)
		CRN(pc) = CRN(pc) + disp8 - 2; // TODO: check
}

IMPL(i_sw_rm) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t m = (x >> 4) & 0xF;
	uint32_t Rn = GR(n);
	uint32_t Rm = GR(m);
	// printf("[0x%08x] <= 0x%08x\n", Rm, Rn);
	cpu->memory.Write(Rm, 4, &Rn);
}

IMPL(i_sw_disp16_rm) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t m = (x >> 4) & 0xF;
	int16_t disp16 = (x >> 16);
	uint32_t Rn = GR(n);
	uint32_t Rm = GR(m);
	// printf("[0x%08x] <= 0x%08x\n", Rm + disp16, Rn);
	cpu->memory.Write(Rm + disp16, 4, &Rn);
}

IMPL(i_add) {
	uint32_t n = (x >> 8) & 0xF;
	uint32_t imm6 = (x >> 2) & 0b111111;
	GR(n) = GR(n) + SignExt(imm6, 6, 32);
}

IMPL(i_bra) {
	uint32_t disp12 = x & 0xFFF;
	CRN(pc) = CRN(pc) + SignExt(disp12, 12, 32) - 2;
}

insn_t insn_table[] = {
	{ 4, 0b1111100000001111, 0b1101100000001000, i_jmp_target24 },
	{ 2, 0b1111000000001110, 0b0111000000001010, i_ldc },
	{ 4, 0b1111000011111111, 0b1100000000100001, i_movh },
	{ 2, 0b1111111111111111, 0b0111000000000000, i_di },
	{ 2, 0b1111000000000000, 0b0101000000000000, i_mov_imm8 },
	{ 4, 0b1111000011111111, 0b1100000000010001, i_movu_imm16 },
	{ 4, 0b1111100000000000, 0b1101000000000000, i_movu_imm24 },
	{ 2, 0b1111000000001111, 0b0000000000000100, i_sub },
	// srl
	{ 2, 0b1111000000000111, 0b0110000000000010, i_srl_imm5 },
	// beqz
	{ 2, 0b1111000000000001, 0b1010000000000000, i_beqz },
	// sw
	{ 2, 0b1111000000001111, 0b0000000000001010, i_sw_rm },
	{ 4, 0b1111000000001111, 0b1100000000001010, i_sw_disp16_rm },
	// add
	{ 2, 0b1111000000000011, 0b0110000000000000, i_add },
	// bra
	{ 2, 0b1111000000000001, 0b1011000000000000, i_bra },
};

insn_t *insn_decode(uint32_t insn_i) {
	for (auto &insn : insn_table)
		if ((insn.mask & insn_i) == insn.match)
			return &insn;
	return nullptr;
}
