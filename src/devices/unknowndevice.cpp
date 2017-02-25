#include "unknowndevice.h"

#include <stdio.h>

UnknownDevice::UnknownDevice(uint32_t base):
	my_base(base)
{}

uint32_t UnknownDevice::Read32(uint32_t addr) {
	printf("UnknownDevice: Read [0x%08x]\n", addr + my_base);
	return 0;
}

void UnknownDevice::Write32(uint32_t addr, uint32_t value) {
	printf("UnknownDevice: Write [0x%08x] <= 0x%08x\n", addr + my_base, value);
}
