#include "inst.h"

void jmp_target24(cpu_t *cpu) {
	printf("jmp_target24\n");
}

inst_t inst_table[] = {
	{ 4, 0b1111111100001111, 0b0001000000001110, jmp_target24 }
};
