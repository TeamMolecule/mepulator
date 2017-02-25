#pragma once

#include <inttypes.h>

struct Cpu;

typedef struct {
	uint32_t sz;
	uint32_t mask;
	uint32_t match;
	void (*execute)(Cpu *cpu, uint32_t x);
} insn_t;

insn_t *insn_decode(uint32_t insn_i);
