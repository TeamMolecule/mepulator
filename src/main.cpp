#include <stdio.h>

#include "cpu.h"
#include "devices/bigmac.h"
#include "devices/e001device.h"
#include "devices/unknowndevice.h"

int main() {
	auto cpu = Cpu();

	cpu.memory.MapFile(0x800000, 0x200000, "1692_f00d.bin");

	cpu.memory.MapDevice(0xE0010000, 8, new e001device());
	cpu.memory.MapDevice(0xE0020000, 8, new UnknownDevice(0xE0020000));
	cpu.memory.MapDevice(0xE0050000, 0x80, new Bigmac);

	cpu.control.pc = 0x800100;

	cpu.Loop();

	return 0;
}
