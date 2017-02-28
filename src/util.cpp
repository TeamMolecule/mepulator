#include "util.h"

#include <stdio.h>

void hex_dump(unsigned paddr, const uint8_t *addr, size_t size) {
	size_t lines = (size + 0xF) / 16;
	for (size_t line = 0; line < lines; ++line) {
		printf("0x%08X:", paddr);
		for (int i = 0; i < 16; ++i)
			if (size) {
				printf(" %02X", addr[0]);
				++addr;
				++paddr;
				--size;
			}
		printf("\n");
	}
}
