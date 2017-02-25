#include "e001device.h"

#include <stdio.h>

#include "log.h"

uint32_t e001device::Read32(uint32_t addr) {
	if (addr == 4)
		return 0x80000005;
	FATAL("unknown addr 0x%x on read\n", addr);
}

void e001device::Write32(uint32_t addr, uint32_t value) {
	FATAL("unknown addr 0x%x on write\n", addr);
}
