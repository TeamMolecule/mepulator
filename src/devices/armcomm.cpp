#include "armcomm.h"

#include <stdio.h>

#include "log.h"

#define ARMCOMM_BASE 0xE0000000

uint32_t ARMComm::Read32(uint32_t addr) {
	switch (addr) {
	case 0:
		return reg00;
	default:
		FATAL("unknown addr 0x%x on read\n", ARMCOMM_BASE + addr);
	}
}

void ARMComm::Write32(uint32_t addr, uint32_t value) {
	switch (addr) {
	case 0:
		reg00 = value;
		break;
	default:
		FATAL("unknown addr 0x%x on write\n", ARMCOMM_BASE + addr);
	}
}
