#pragma once

#include "device.h"

class Bigmac: public Device {
public:
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	uint32_t state[0x80/4] = { 0 };
};
