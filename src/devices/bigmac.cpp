#include "bigmac.h"

#include <stdio.h>

#include "log.h"
#include "memory.h"
#include "util.h"

Bigmac::Bigmac(Memory *mem_):
	mem(mem_)
{}

uint32_t Bigmac::Read32(uint32_t addr) {
	if (addr == 0x3C) {
		printf("access RNG\n");
		return 0x11223344;
	}
	if (addr == 0x24)
		return channels[0].busy;
	FATAL("unknown addr 0x%x on read\n", addr);
}

void Bigmac::Write32(uint32_t addr, uint32_t value) {
	if (addr >= 0x100) {
		addr -= 0x100;
		if (addr >= sizeof(control) || addr & 0b11)
			FATAL("invalid control addr 0x100+0x%x\n", addr);
		control[addr / 4] = value;
	} else {
		if (addr & 0b11)
			FATAL("unaligned address 0x%x\n", addr);
		int c = (addr >= sizeof(bigmac_regs)) ? 1 : 0;
		addr -= sizeof(bigmac_regs) * c;
		bigmac_regs *ch = &channels[c];
		*(uint32_t*)((char*)ch + addr) = value;
		if (addr == 0x1C) {
			printf("---- Bigmac commit: channel %d -------------------------------------------------------\n", c);
			printf("src: 0x%08x dst:  0x%08x sz:  0x%08x func: 0x%08x keyslot: 0x%08x\n", ch->src, ch->dst, ch->sz, ch->func, ch->keyslot);
			printf("iv:  0x%08x 0x18: 0x%08x cmt: 0x%08x 0x20: 0x%08x busy:    0x%08x\n", ch->iv, ch->unk_0x18, ch->commit, ch->unk_0x20, ch->busy);
			hex_dump(0x28, (char*)ch->unk, sizeof(ch->unk));
			printf("control:\n");
			hex_dump(0, (char*)control, sizeof(control));
			printf("-------------------------------------------------------------------------------------\n");

			if (value != 1)
				FATAL("commit: got unknown value 0x%x\n", value);

			ch->busy = 1;
			DoFunc(c);
			ch->busy = 0;
		}
	}
}

void Bigmac::DoFunc(int channel) {
	bigmac_regs *ch = &channels[channel];
	switch (ch->func) {
	case 0: { // memcpy
		char *tmp = new char [ch->sz];
		mem->Read(ch->src, ch->sz, tmp);
		mem->Write(ch->dst, ch->sz, tmp);
		delete[] tmp;
		break;
	}
	default:
		FATAL("unknown function 0x%x\n", ch->func);
	}
}
