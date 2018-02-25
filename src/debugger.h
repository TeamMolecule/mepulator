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
	int SendChar(int ch);

	void ProcessPacket();
	void DisconnectClient();

	Cpu *cpu;
	int client_fd;
	std::string packet;
};
