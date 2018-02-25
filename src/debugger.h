#pragma once

#include <string>

struct Cpu;

class Debugger {
public:
	Debugger(Cpu *cpu_);
	void Loop();
private:
	int ReadPacket();
	int ReadChar();
	int SendPacket(const std::string &packet);
	int SendChar(int ch);

	void ProcessPacket();
	void DisconnectClient();

	uint32_t GetReg(int regnum);
	void SendRegs();
	void SendReg(int regnum);

	Cpu *cpu;
	int client_fd;
	std::string packet;
};
