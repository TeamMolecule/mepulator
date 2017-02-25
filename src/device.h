#pragma once

#include <inttypes.h>

class Device {
public:
	virtual uint32_t Read32(uint32_t addr) = 0;
	virtual void Write32(uint32_t addr, uint32_t value) = 0;
};
