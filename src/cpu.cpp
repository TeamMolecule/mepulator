#include "cpu.h"

#include <stdio.h>
#include <unistd.h>

#include "log.h"

#define CRN(regname) (control.regname)
#define GRN(regname) (gpr.regname)

void Cpu::DumpRegs() {
	printf("r0:  0x%08x r1: 0x%08x r2:  0x%08x r3:  0x%08x ",  GRN(r0),  GRN(r1), GRN(r2),  GRN(r3));
	printf("r4:  0x%08x r5: 0x%08x r6:  0x%08x r7:  0x%08x\n", GRN(r4),  GRN(r5), GRN(r6),  GRN(r7));
	printf("r8:  0x%08x r9: 0x%08x r10: 0x%08x r11: 0x%08x ",  GRN(r8),  GRN(r9), GRN(r10), GRN(r11));
	printf("r12: 0x%08x tp: 0x%08x gp:  0x%08x sp:  0x%08x\n", GRN(r12), GRN(tp), GRN(gp),  GRN(sp));
	printf("pc:  0x%08x lp: 0x%08x rpb: 0x%08x rpe: 0x%08x\n", CRN(pc), CRN(lp), CRN(rpb), CRN(rpe));
	printf("-----------------------------------------------------------------------------------------------------------------------------\n");
}

void Cpu::Loop() {
	while (1) {
		if (state == CpuState::Sleep) {
			usleep(1000);
			continue;
		}

		DumpRegs();

		// fetch
		uint32_t insn_i = 0;
		memory.Read(control.pc, 4, &insn_i);

		// When the Program Counter value at the decode stage is equal to the contents of the RPE register, the instruction 
		// at the decode stage (D stage) and an instruction following it are executed, after which the RPB value is placed
		// into PC, causing a jump to the first instruction of the repeat block.
		if (control.pc == (control.rpe & ~1))
			rpb_in = 2;

		// decode
		insn_t *insn = insn_decode(insn_i);

		if (!insn)
			FATAL("failed to decode at 0x%x bytes 0x%x\n", control.pc, insn_i);

		control.pc += insn->sz;

		// execute
		insn->execute(this, insn_i);
		if (rpb_in > 0)
			--rpb_in;
		if (rpb_in == 0) {
			control.pc = control.rpb;
			rpb_in = -1;
		}
	}
}
