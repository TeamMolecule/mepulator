#pragma once

#include "device.h"

#include <atomic>

struct Cpu;

class ARMComm: public Device {
public:
	ARMComm(Cpu *cpu_);
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	Cpu *cpu;
	std::atomic<uint32_t> reg00; // 0x00
	std::atomic<uint32_t> reg10; // 0x10
};
