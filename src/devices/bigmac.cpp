#include "bigmac.h"

#include <stdio.h>

#include "log.h"

uint32_t Bigmac::Read32(uint32_t addr) {
	if (addr == 0x3C) {
		printf("access RNG\n");
		return 0x11223344;
	}
	FATAL("unknown addr 0x%x on read\n", addr);
}

void Bigmac::Write32(uint32_t addr, uint32_t value) {
	FATAL("unknown addr 0x%x on write\n", addr);
}
