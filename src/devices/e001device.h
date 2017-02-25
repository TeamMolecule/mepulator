#pragma once

#include "device.h"

class e001device: public Device {
public:
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
};
