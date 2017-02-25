#include "insn.h"

#include <stdio.h>
#include <string>

#include "cpu.h"
#include "log.h"

#define CRN(regname) (cpu->control.regname)
#define GRN(regname) (cpu->gpr.regname)
#define CR(num) (*((uint32_t*)&cpu->control + num))
#define GR(num) (*((uint32_t*)&cpu->gpr + num))

#define INSN(name) static void name(Cpu *cpu, uint32_t insn)

#define SignExt(value, from, to) \
	(value | ((value & (1 << (from - 1))) ? (((1 << (to - from)) - 1) << from) : (0)))

#define GetBits(x, off, len) \
	((x >> off) & ((1 << len) - 1))


#define Load1(addr) \
	({uint32_t tmp = 0; \
	 cpu->memory.Read(addr, 1, &tmp); \
	 tmp;})

#define Load2(addr) \
	({uint32_t tmp = 0; \
	 cpu->memory.Read(addr, 2, &tmp); \
	 tmp;})

#define Load4(addr) \
	({uint32_t tmp = 0; \
	 cpu->memory.Read(addr, 4, &tmp); \
	 tmp;})


#define Store1(value, addr) \
	do { \
		uint32_t tmp = value; \
		cpu->memory.Write(addr, 1, &tmp); \
	} while(0);

#define Store2(value, addr) \
	do { \
		uint32_t tmp = value; \
		cpu->memory.Write(addr, 2, &tmp); \
	} while(0);

#define Store4(value, addr) \
	do { \
		uint32_t tmp = value; \
		cpu->memory.Write(addr, 4, &tmp); \
	} while(0);


#define BRA(value) \
	do_branch(cpu, value);

void do_branch(Cpu *cpu, uint32_t newpc) {
	if (cpu->rpb_in != -1) {
		// exit repeat/erepeat block
		if (newpc < cpu->control.rpb || newpc > cpu->control.rpe) {
			printf("repeat/erepeat out via branch\n");
			cpu->rpb_in = -1;
		}
	}
	cpu->control.pc = newpc;
}

#include "insn_gen.h" // See src/insn.in


insn_t *insn_decode(uint32_t insn_i) {
	for (auto &insn : insn_table)
		if ((insn.mask & insn_i) == insn.match)
			return &insn;
	return nullptr;
}
