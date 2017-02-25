#include "cpu.h"

inst_t *cpu_fetch(cpu_t *cpu) {

}

void cpu_loop(cpu_t *cpu) {
	while (1) {
		inst_t *inst = fetch(&cpu);
		inst->execute(&cpu);
	}
}