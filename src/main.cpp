#include <stdio.h>

#include "cpu.h"
#include "devices/bigmac.h"
#include "devices/e001device.h"
#include "devices/eeprom.h"
#include "devices/unknowndevice.h"

int main() {
	auto cpu = Cpu();

	cpu.memory.MapFile(0x800000, 0x200000, "1692_f00d.bin");

	cpu.memory.MapDevice(0xE0000000, 8, new UnknownDevice(0xE0000000));
	cpu.memory.MapDevice(0xE0010000, 8, new e001device());
	cpu.memory.MapDevice(0xE0020000, 8, new UnknownDevice(0xE0020000));
	cpu.memory.MapDevice(0xE0050000, 0x80, new Bigmac);

	EEPROM *eeprom = new EEPROM();
	EEPROMProgrammer *programmer = new EEPROMProgrammer(eeprom);
	eeprom->Load("1692_eeprom.bin");
	cpu.memory.MapDevice(0xE0058000, 0x10000, eeprom);
	cpu.memory.MapDevice(0xE0030000, 0x30, programmer);

	cpu.control.pc = 0x800100;

	cpu.Loop();

	return 0;
}
