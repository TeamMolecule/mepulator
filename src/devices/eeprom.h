#pragma once

#include "device.h"

class EEPROMProgrammer;

class EEPROM: public Device {
public:
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
	void Load(const char *path);
private:
	uint8_t memory[0x10000] = { 0 };

	friend EEPROMProgrammer;
};

class EEPROMProgrammer: public Device {
public:
	EEPROMProgrammer(EEPROM *eeprom_);
	uint32_t Read32(uint32_t addr);
	void Write32(uint32_t addr, uint32_t value);
private:
	EEPROM *eeprom;
	uint32_t state[0x30 / 4] = { 0 };
};
