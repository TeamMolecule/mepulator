#include "log.h"

#include "cpu.h"

extern Cpu *g_cpu;

void log_dump_state() {
	Cpu *cpu = g_cpu;

	printf("CPU state at the moment of error:\n");
	cpu->DumpRegs();
}