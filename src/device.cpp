#include "device.h"
#include "log.h"

uint8_t Device::Read8(uint32_t addr) {
	FATAL("this device does not support Read8\n");
}

uint16_t Device::Read16(uint32_t addr) {
	FATAL("this device does not support Read16\n");
}

uint32_t Device::Read32(uint32_t addr) {
	FATAL("this device does not support Read32\n");
}

void Device::Write8(uint32_t addr, uint8_t value) {
	FATAL("this device does not support Write8\n");
}

void Device::Write16(uint32_t addr, uint16_t value) {
	FATAL("this device does not support Write16\n");
}

void Device::Write32(uint32_t addr, uint32_t value) {
	FATAL("this device does not support Write32\n");
}
