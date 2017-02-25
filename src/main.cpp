#include <stdio.h>

#include "cpu.h"

int main() {
	auto cpu = Cpu();

	cpu.memory.MapFile(0x800000, 0x200000, "1692_f00d.bin");
	cpu.control.pc = 0x800100;
	cpu.Loop();

	return 0;
}
