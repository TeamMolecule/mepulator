#pragma once

#include <inttypes.h>

#include "insn.h"
#include "memory.h"

struct __attribute__ ((__packed__)) Gpr {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t tp;
	uint32_t gp;
	uint32_t sp;
};

struct __attribute__ ((__packed__)) Control {
	uint32_t pc;
	uint32_t lp;
	uint32_t sar;
	uint32_t r3;
	uint32_t rpb;
	uint32_t rpe;
	uint32_t rpc;
	uint32_t hi;
	uint32_t lo;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t mb0;
	uint32_t me0;
	uint32_t mb1;
	uint32_t me1;
	uint32_t psw;
	uint32_t id;
	uint32_t tmp;
	uint32_t epc;
	uint32_t exc;
	uint32_t cfg;
	uint32_t res22;
	uint32_t npc;
	uint32_t dbg;
	uint32_t depc;
	uint32_t opt;
	uint32_t rcfg;
	uint32_t ccfg;
	uint32_t r29;
	uint32_t r30;
	uint32_t r31;
	uint32_t r32;
};

struct Cpu {
	Memory memory;
	Gpr gpr;
	Control control;
	int rpb_in = -1;

	insn_t* Fetch();
	void Loop();
	void DumpRegs();
};
