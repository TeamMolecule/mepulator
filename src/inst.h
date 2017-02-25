#pragma once

#include <inttypes.h>

typedef struct {
	uint32_t sz;
	uint32_t mask;
	uint32_t match;
	void (*execute)(cpu_t *cpu);
} inst_t;
