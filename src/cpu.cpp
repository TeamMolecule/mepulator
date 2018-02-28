#include "cpu.h"

#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "util.h"

#define CRN(regname) (control.regname)
#define GRN(regname) (gpr.regname)

Cpu::Cpu():
	pending_irq(0xFFFFFFFF)
{}

void Cpu::DumpRegs() {
	printf("r0:  0x%08X r1:  0x%08X r2:  0x%08X r3:  0x%08X ",  GRN(r0),  GRN(r1), GRN(r2),  GRN(r3));
	printf("r4:  0x%08X r5:  0x%08X r6:  0x%08X r7:  0x%08X\n", GRN(r4),  GRN(r5), GRN(r6),  GRN(r7));
	printf("r8:  0x%08X r9:  0x%08X r10: 0x%08X r11: 0x%08X ",  GRN(r8),  GRN(r9), GRN(r10), GRN(r11));
	printf("r12: 0x%08X tp:  0x%08X gp:  0x%08X sp:  0x%08X\n", GRN(r12), GRN(tp), GRN(gp),  GRN(sp));
	printf("pc:  0x%08X lp:  0x%08X rpb: 0x%08X rpe: 0x%08X\n", CRN(pc), CRN(lp), CRN(rpb), CRN(rpe));
	printf("cfg: 0x%08X psw: 0x%08X\n", CRN(cfg), CRN(psw));
	printf("-----------------------------------------------------------------------------------------------------------------------------\n");
}

void Cpu::Step() {
	// The debugger has stopped the cpu
	if (state == CpuState::Stopped)
		return;

	if (CRN(psw) & 1) {
		uint32_t prev_irq = pending_irq.exchange(0xFFFFFFFF);
		if (prev_irq != 0xFFFFFFFF) {
			// 3.6.5.  Hardware interrupt (INT)

			// A jump is executed to the exception vector address of the hardware interrupt.
			// Interrupt vector addresses become as shown below by the CFG.IVM, EVM, EVA and IVA bits.
			// TODO: check status flags to figure which irq handler to execute, instead of hardcoding shit
			CRN(pc) = 0x800030 + 4 * prev_irq;

			// The upper 31 bits of the PC value of currently executed instruction is stored in the EPC field of the EPC register.
			CRN(epc) = CRN(pc) & 0xFFFFFFFE;

			// The EXC field value of the EXC register is set to 0.
			// TODO

			// The PSW.IEC is saved in the PSW.IEP. The PSW.IEC is set to 0 and interrupts are disabled.
			CRN(psw) = SetBit(CRN(psw), 1, GetBit(CRN(psw), 0)); // PSW.IEP <- PSW.IEC
			CRN(psw) = SetBit(CRN(psw), 0, 0); // PSW.IEC = 0

			// The PSW.UMC is saved in the PSW.UMP. The PSW.UMC is set to 0 and the mode changes to the kernel
			CRN(psw) = SetBit(CRN(psw), 3, GetBit(CRN(psw), 2)); // PSW.UMP <- PSW.UMC
			CRN(psw) = SetBit(CRN(psw), 2, 0); // PSW.UMC = 0

			if (state == CpuState::Sleep)
				state = CpuState::Running;
		}
	}

	if (state == CpuState::Sleep)
		return;

	// Stop the cpu if we hit a breakpoint
	if (breakpoints.find(control.pc) != breakpoints.end()) {
		state = CpuState::Stopped;
		return;
	}

	// DumpRegs();

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
		if (control.rpe & 1) {
			// erepeat
			rpb_in = -1;
			control.pc = control.rpb;
		} else {
			// repeat
			rpb_in = -1;
			if (control.rpc != 0) {
				control.pc = control.rpb;
				control.rpc--;
			}
		}
	}

	if (state == CpuState::SingleStep) {
		state = CpuState::Stopped;
	}
}


void Cpu::Irq(uint32_t num) {
	// TODO: check if interrupts enabled?
	printf("set pending_irq %d\n", num);
	pending_irq = num;
}
