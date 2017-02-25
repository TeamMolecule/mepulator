#include "armcomm.h"

#include <stdio.h>
#include <mutex>

#include "cpu.h"
#include "log.h"

#define ARMCOMM_BASE 0xE0000000

ARMComm::ARMComm(Cpu *cpu_):
	cpu(cpu_)
{}

uint32_t ARMComm::Read32(uint32_t addr) {
	// printf("ARMComm::Read32 0x%x\n", addr);

	switch (addr) {
	case 0:
		return reg00;
	case 0x10:
		return reg10;
	default:
		FATAL("unknown addr 0x%x on read\n", ARMCOMM_BASE + addr);
	}
}

void ARMComm::Write32(uint32_t addr, uint32_t value) {
	if (addr == 0x10 && value == 0xFFFFFFFF)
		value = 0;

	switch (addr) {
	case 0:
		reg00 = value;
		break;
	case 0x10:
		reg10 = value;
		// we need to raise mep irq8
		if (value & 1)
			cpu->Irq(8);
		break;
	default:
		FATAL("unknown addr 0x%x on write\n", ARMCOMM_BASE + addr);
	}
}
