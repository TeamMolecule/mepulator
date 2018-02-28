#pragma once

#include <inttypes.h>

class Device {
public:
	virtual uint8_t Read8(uint32_t addr);
	virtual uint16_t Read16(uint32_t addr);
	virtual uint32_t Read32(uint32_t addr);

	virtual void Write8(uint32_t addr, uint8_t value);
	virtual void Write16(uint32_t addr, uint16_t value);
	virtual void Write32(uint32_t addr, uint32_t value);
};
