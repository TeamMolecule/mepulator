#pragma once

#include <inttypes.h>

typedef struct {
	void (*init)(memory_t *memory);
	void (*read)(memory_t *memory, uint32_t pa, uint32_t sz, void *dest);
	void (*write)(memory_t *memory, uint32_t pa, uint32_t sz, void *dest);
	void (*map_file)(memory_t *memory, uint32_t pa, const char *path);
} memory_t;
