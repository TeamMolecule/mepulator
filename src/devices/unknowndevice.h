#pragma once

#include "device.h"

class UnknownDevice: public Device {
public:
	UnknownDevice(uint32_t base);
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	uint32_t my_base;
};
