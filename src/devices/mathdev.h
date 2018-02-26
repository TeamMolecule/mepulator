#pragma once

#include "device.h"

class Math: public Device {
public:
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	void PowMod();

	uint32_t flags = 0;
	uint8_t sig[0x200] = { 0 };
	uint8_t modulus[0x100] = { 0 };
	uint8_t res[0x200] = { 0 };
	uint32_t exponent = 0;
	uint32_t cmd = 0;
};
