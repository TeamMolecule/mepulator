#include "armcomm.h"

#include <stdio.h>
#include <mutex>

#include "log.h"

#define ARMCOMM_BASE 0xE0000000
#define LOCK std::lock_guard<std::mutex> guard(comm_mutex)

uint32_t ARMComm::Read32(uint32_t addr) {
	LOCK;

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
	LOCK;

	switch (addr) {
	case 0:
		reg00 = value;
		break;
	case 0x10:
		reg10 = value;
		break;
	default:
		FATAL("unknown addr 0x%x on write\n", ARMCOMM_BASE + addr);
	}
}
