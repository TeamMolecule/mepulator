#pragma once

#include <stddef.h>

#define SignExt(value, from, to) \
	(value | ((value & (1 << (from - 1))) ? (((1 << (to - from)) - 1) << from) : (0)))

#define GetBits(x, off, len) \
	((x >> off) & ((1 << len) - 1))

#define GetBit(x, off) \
	GetBits(x, off, 1)

#define SetBit(x, off, value) \
	((x & ~(1 << off)) | (value << off))

void hex_dump(unsigned paddr, const char *addr, size_t size);
