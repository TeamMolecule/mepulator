#pragma once

#include <inttypes.h>
#include <vector>

#include "device.h"

enum class BankType {
	Undefined,
	Memory,
	Device
};

struct MemoryBank {
	uint32_t pa;
	uint32_t sz;
	BankType type;
	void *buffer;
	Device *device;
};

struct Memory {
	void Read(uint32_t pa, uint32_t sz, void *dest);
	void Write(uint32_t pa, uint32_t sz, void *src);
	void MapFile(uint32_t pa, uint32_t sz, const char *path);
	void MapDevice(uint32_t pa, uint32_t sz, Device *device);
	void MapRam(uint32_t pa, uint32_t sz);

private:
	MemoryBank *GetBank(uint32_t pa, uint32_t sz);
	std::vector<MemoryBank> banks;
};
