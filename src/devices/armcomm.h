#pragma once

#include "device.h"

#include <mutex>

class ARMComm: public Device {
public:
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	std::mutex comm_mutex;
	uint32_t reg00 = 0; // 0x00
	uint32_t reg10 = 0; // 0x10
};
