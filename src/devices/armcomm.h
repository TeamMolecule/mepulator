#pragma once

#include "device.h"

#include <mutex>

class Cpu;

class ARMComm: public Device {
public:
	ARMComm(Cpu *cpu_);
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	Cpu *cpu;
	uint32_t reg00 = 0; // 0x00
	uint32_t reg10 = 0; // 0x10
};
