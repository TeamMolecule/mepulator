#pragma once

#include <stddef.h>
#include <inttypes.h>
#include <string>

#define SignExt(value, from, to) \
	(value | ((value & (1 << (from - 1))) ? (((1 << (to - from)) - 1) << from) : (0)))

#define GetBits(x, off, len) \
	((x >> off) & ((1 << len) - 1))

#define GetBit(x, off) \
	GetBits(x, off, 1)

#define SetBit(x, off, value) \
	((x & ~(1 << off)) | (value << off))

void hex_dump(unsigned paddr, const uint8_t *addr, size_t size);
bool startswith(const std::string &haystack, const std::string &needle);
