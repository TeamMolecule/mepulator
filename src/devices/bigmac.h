#pragma once

#include "device.h"

class Memory;

struct bigmac_regs {
	uint32_t src; // 0x0
	uint32_t dst; // 0x4
	uint32_t sz; // 0x8
	uint32_t func; // 0xC
	uint32_t keyslot; // 0x10
	uint32_t iv; // 0x14
	uint32_t unk_0x18; // 0x18
	uint32_t commit; // 0x1C
	uint32_t unk_0x20; // 0x20
	uint32_t busy; // 0x24
	uint32_t unk[(0x80 - 0x28) / 4];
};

struct bigmac_control {
	uint32_t unk0x0;
	uint32_t unk0x4;
};

static_assert (sizeof(bigmac_regs) == 0x80, "bigmac_regs size must be 0x80");

class Bigmac: public Device {
public:
	Bigmac(Memory *mem_);
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	void DoFunc(int c);

	Memory *mem;
	bigmac_regs channels[2] = {};
	uint32_t control[0x8/4] = { 0 };
	uint32_t control2[0x40/4] = { 0 };
};
