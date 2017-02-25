#include <stdio.h>
#include <thread>

#include "arm.h"
#include "cpu.h"
#include "devices/armcomm.h"
#include "devices/bigmac.h"
#include "devices/e001device.h"
#include "devices/eeprom.h"
#include "devices/unknowndevice.h"

int main() {
	Cpu *cpu = new Cpu();
	ARMComm *comm = new ARMComm();
	ARM *arm = new ARM(comm);

	cpu->memory.MapFile(0x800000, 0x200000, "1692_f00d.bin");

	cpu->memory.MapDevice(0xE0000000, 0x20, comm);
	cpu->memory.MapDevice(0xE0010000, 8, new e001device());
	cpu->memory.MapDevice(0xE0020000, 8, new UnknownDevice(0xE0020000));
	cpu->memory.MapDevice(0xE0050000, 0x80, new Bigmac);

	EEPROM *eeprom = new EEPROM();
	EEPROMProgrammer *programmer = new EEPROMProgrammer(eeprom);
	eeprom->Load("1692_eeprom.bin");
	cpu->memory.MapDevice(0xE0058000, 0x10000, eeprom);
	cpu->memory.MapDevice(0xE0030000, 0x30, programmer);

	cpu->control.pc = 0x800100;

	std::thread arm_thread(&ARM::Loop, arm);

	while (1) {
		cpu->Step();
	}

	return 0;
}
