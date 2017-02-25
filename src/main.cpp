#include <stdio.h>

#include "cpu.h"

int main() {
	auto cpu = Cpu();

	cpu.Loop();

	return 0;
}
