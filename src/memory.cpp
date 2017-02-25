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
	switch (bank->type) {
	case BankType::Memory:
		memcpy(dest, (char*)bank->buffer + pa - bank->pa, sz);
		break;
	case BankType::Device:
		if (sz != 4)
			FATAL("devices only support 32 bit ops\n");
		*(uint32_t*)dest = bank->device->Read32(pa - bank->pa);
		break;
	default:
		FATAL("tried to access undefined device\n");
	}
}

void Memory::Write(uint32_t pa, uint32_t sz, void *src) {
	MemoryBank *bank = GetBank(pa, sz);
	if (!bank)
		FATAL("failed to read 0x%08x size 0x%08x\n", pa, sz);
	switch (bank->type) {
	case BankType::Memory:
		memcpy((char*)bank->buffer + pa - bank->pa, src, sz);
		break;
	case BankType::Device:
		if (sz != 4)
			FATAL("devices only support 32 bit ops\n");
		bank->device->Write32(pa - bank->pa, *(uint32_t*)src);
		break;
	default:
		FATAL("tried to access undefined device\n");
	}
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
	bank.type = BankType::Memory;
	bank.pa = pa;
	bank.sz = sz;
	bank.buffer = addr;
	banks.push_back(bank);
}

void Memory::MapDevice(uint32_t pa, uint32_t sz, Device *device) {
	MemoryBank bank = { 0 };
	bank.type = BankType::Device;
	bank.pa = pa;
	bank.sz = sz;
	bank.device = device;
	banks.push_back(bank);
}
