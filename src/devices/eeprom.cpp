#include "eeprom.h"

#include <stdio.h>
#include <string.h>

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
	if (addr == 0x2c) {
		uint32_t line = state[0x28/4];
		printf("EEPROMProgrammer::GetProt(0x%08x)\n", line);
		// if (line == 0x500)
			return 0xDEAD;
	}
	FATAL("attempt to read EEPROMProgrammer addr 0x%x\n", addr);
}

void EEPROMProgrammer::Write32(uint32_t addr, uint32_t value) {
	if (addr > sizeof(state) || (addr & 0b11))
		FATAL("invalid addr 0x%x\n", addr);
	if (addr <= 0x1C || addr == 0x28) {
		state[addr / 4] = value;
	} else if (addr == 0x20) {
		state[addr / 4] = value;
		printf("EEPROMProgrammer::Write line = 0x%x\n", value);
		printf("data: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", state[0], state[1], state[2], state[3], state[4], state[5], state[6], state[7]);
		memcpy(eeprom->memory + 32 * value, state, 0x20);
	} else if (addr == 0x24) {
		printf("EEPROMProgrammer::SetProt(0x%08x)\n", value);
	} else {
		FATAL("unimpl 0x%x\n", addr);
	}
}
