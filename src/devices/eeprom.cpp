#include "eeprom.h"

#include <stdio.h>

#include "log.h"

uint32_t EEPROM::Read32(uint32_t addr) {
	if (addr < sizeof(memory) && (addr & 0b11) == 0) {
		return ((uint32_t*)memory)[addr / 4];
	}
	FATAL("unknown addr 0x%x on read\n", addr);
}

void EEPROM::Write32(uint32_t addr, uint32_t value) {
	FATAL("tried to write to eeprom directly: [0x%x] <= 0x%x\n", addr, value);
}

void EEPROM::Load(const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin)
		FATAL("failed to open %s for read\n", path);
	fread(memory, sizeof(memory), 1, fin);
	fclose(fin);
}

EEPROMProgrammer::EEPROMProgrammer(EEPROM *eeprom_):
	eeprom(eeprom_)
{}

uint32_t EEPROMProgrammer::Read32(uint32_t addr) {
	FATAL("attempt to read EEPROMProgrammer addr 0x%x\n", addr);
}

void EEPROMProgrammer::Write32(uint32_t addr, uint32_t value) {
	if (addr > sizeof(state) || (addr & 0b11))
		FATAL("invalid addr 0x%x\n", addr);
	if (addr == 0x20) {
		FATAL("programmer::write\n");
	}
	if (addr <= 0x1C)
		state[addr / 4] = value;
	else
		FATAL("unimpl\n");
}
