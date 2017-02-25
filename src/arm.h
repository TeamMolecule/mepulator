#pragma once

class Cpu;
class ARMComm;

class ARM {
public:
	ARM(ARMComm *comm_);
	void Loop();
private:
	ARMComm *comm;
};
