#include "memory.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "log.h"

void Memory::Read(uint32_t pa, uint32_t sz, void *dest) {
	MemoryBank *bank = GetBank(pa, sz);
	if (!bank)
		FATAL("failed to read 0x%08x size 0x%08x\n", pa, sz);
	memcpy(dest, (char*)bank->buffer + pa - bank->pa, sz);
}

void Memory::Write(uint32_t pa, uint32_t sz, void *src) {
	MemoryBank *bank = GetBank(pa, sz);
	if (!bank)
		FATAL("failed to read 0x%08x size 0x%08x\n", pa, sz);
	memcpy((char*)bank->buffer + pa - bank->pa, src, sz);
}

MemoryBank* Memory::GetBank(uint32_t pa, uint32_t sz) {
	for (auto &bank : banks)
		if (bank.pa <= pa && bank.pa + bank.sz >= pa + sz)
			return &bank;
	return nullptr;
}

void Memory::MapFile(uint32_t pa, uint32_t sz, const char *path) {
	int fd = open(path, O_RDONLY);
	void *addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	MemoryBank bank = { 0 };
	bank.pa = pa;
	bank.sz = sz;
	bank.buffer = addr;
	banks.push_back(bank);
}
