#pragma once

#include <inttypes.h>

class Cpu;
class ARMComm;
class Memory;

class ARM {
public:
	ARM(ARMComm *comm_, Memory *mem_);
	void Loop();
private:
	void SetSharedBuffer();
	void StartUp();
	void SetRvk();
	void SetTracebuf();

	ARMComm *comm;
	Memory *mem;
};
