#pragma once

#include <inttypes.h>
#include <vector>

struct MemoryBank {
	uint32_t pa;
	uint32_t sz;
	void *buffer;
};

struct Memory {
	void Read(uint32_t pa, uint32_t sz, void *dest);
	void Write(uint32_t pa, uint32_t sz, void *src);
	void MapFile(uint32_t pa, uint32_t sz, const char *path);

private:
	MemoryBank *GetBank(uint32_t pa, uint32_t sz);
	std::vector<MemoryBank> banks;
};
