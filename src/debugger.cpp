#include "debugger.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "cpu.h"
#include "log.h"
#include "util.h"

Debugger::Debugger(Cpu *cpu_):
	cpu(cpu_)
{}

void Debugger::Loop() {
	printf("Starting the debugger...\n");
	int s = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	sockaddr_in server = { 0 }, client = { 0 };

	if (s == -1) {
		perror("Failed to create the socket");
		FATAL("Failed to initialize the debugger");
	}
	printf("socket: %d\n", s);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(7778);

	if (bind(s,(sockaddr *)&server, sizeof(server)) == -1) {
		perror("Failed to bind the socket");
		FATAL("Failed to initialize the debugger\n");
	}

	if (listen(s, 3) == -1) {
		perror("Failed to listen the socket");
		FATAL("Failed to initialize the debugger\n");
	}

	while (true) {
		socklen_t client_len = sizeof(client);
		if ((client_fd = accept(s, (sockaddr *)&client, &client_len)) == -1) {
			perror("Failed to accept the client");
			continue;
		}

		if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &enable, sizeof(int)) == -1) {
			perror("Failed to put the socket into NODELAY mode");
		}

		while (true) {
			if (ReadPacket() < 0) {
				TRACE("Failure while reading a packet.\n");
				DisconnectClient();
				break;
			}

			ProcessPacket();
		}
	}
}

void Debugger::ProcessPacket() {
	// TRACE("Got packet: %s\n", packet.c_str());
	if (startswith(packet, "qSupported")) {
		SendPacket("PacketSize=40960");
	} else if (packet == "vMustReplyEmpty") {
		SendPacket("");
	} else if (packet == "g") {
		// Read general registers.
		SendRegs();
	} else if (packet[0] == 'p') {
		// 'p n': Read the value of register n; n is in hex.
		int regnum = strtol(packet.c_str() + 1, NULL, 16);
		SendReg(regnum);
	} else if (packet == "qC") {
		// Return the current thread ID.
		SendPacket("");
	} else if (packet[0] == 'H') {
		// Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.).
		SendPacket("OK");
	} else {
		// TRACE("Unsupported packet: %s\n", packet.c_str());
		SendPacket("E01");
	}
}

static std::string reg_to_hex(uint32_t value) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%02X%02X%02X%02X", value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF);
	return buf;
}

uint32_t Debugger::GetReg(int regnum) {
	if (regnum <= 15) {
		uint32_t *gpr = (uint32_t*)&cpu->gpr;
		return gpr[regnum];
	}
	uint32_t *ctr = (uint32_t*)&cpu->control;
	return ctr[regnum - 16];
}

void Debugger::SendReg(int regnum) {
	if (regnum <= 15) {
		SendPacket(reg_to_hex(GetReg(regnum)));
	}
}

void Debugger::SendRegs() {
	std::string regs = "";
	for (int i = 0; i < 48; ++i)
		regs += reg_to_hex(GetReg(i));
	SendPacket(regs);
}

#define RSP_START_TOKEN     '$'
#define RSP_END_TOKEN       '#'
#define RSP_ESCAPE_TOKEN    '}'
#define RSP_RUNLENGTH_TOKEN '*'

static int from_hex(int digit) {
	if (digit >= '0' && digit <= '9')
		return digit - '0';

	digit = tolower(digit);

	if (digit >= 'a' && digit <= 'f')
		return digit - 'a' + 0xA;

	return -1;
}

static uint32_t to_hex(int digit) {
	const char *hex_digits = "0123456789abcdef";
	return hex_digits[digit & 0xF];
}

int Debugger::ReadPacket() {
	packet = "";

	uint32_t sum = 0;
	int token = 0;

	// Skip to start of packet
	while (token != '$') {
		token = ReadChar();
		if (token < 0)
			return -1;
	}

	while (true) {
		token = ReadChar();
		if (token < 0)
			break;

		if (token == RSP_END_TOKEN) {
			// checksum
			int upper = ReadChar();
			int lower = ReadChar();
			if (upper < 0 || lower < 0)
				break;

			upper = from_hex(upper) << 4;
			lower = from_hex(lower);
			if (upper < 0 || lower < 0)
				break;

			// checksum is mod 256 of sum of all data
			if ((sum & 0xFF) != (upper | lower)) {
				TRACE("ReadPacket checksum error 0x%02X != 0x%02X\n", (sum & 0xFF), (upper | lower));
				TRACE("packet: %s\n", packet.c_str());
				break;
			}

			SendChar('+');
			return 0;
		}

		sum += token;
		if (token == RSP_ESCAPE_TOKEN) {
			token = ReadChar();
			if (token < 0)
				break;
			sum += token;
			token ^= 0x20;
		}
		packet += (char)token;
	}

	SendChar('-');
	return -1;
}

int Debugger::ReadChar() {
	uint8_t buf = 0;
	if (recv(client_fd, &buf, sizeof(buf), 0) == -1) {
		perror("Failed to recv");
		return -1;
	}
	return buf;
}

void Debugger::DisconnectClient() {
	close(client_fd);
	client_fd = -1;
}

static int is_token(int ch)
{
	return (ch == RSP_START_TOKEN
		|| ch == RSP_END_TOKEN
		|| ch == RSP_ESCAPE_TOKEN
		|| ch == RSP_RUNLENGTH_TOKEN);
}


int Debugger::SendPacket(const std::string &packet) {
	// TRACE("Sending packet %s\n", packet.c_str());

	uint32_t sum = 0;

	// write the start token
	SendChar(RSP_START_TOKEN);

	for (int i = 0; i < packet.size(); ++i) {
		int token = packet[i] & 0xFF;

		// check for any reserved tokens
		if (is_token(token))
		{
			SendChar(RSP_ESCAPE_TOKEN);
			SendChar(token ^ 0x20);

			sum += RSP_ESCAPE_TOKEN;
			sum += token ^ 0x20;
		}
		else
		{
			SendChar(token);
			sum += token;
		}
	}

	// done with data - now end + checksum
	SendChar(RSP_END_TOKEN);

	// checksum is mod 256
	sum &= 0xFF;
	SendChar(to_hex((sum >> 4) & 0xF));
	SendChar(to_hex(sum & 0xF));

	return 0;
}

int Debugger::SendChar(int ch) {
	uint8_t buf = ch;
	if (send(client_fd, &buf, sizeof(buf), 0) == -1) {
		perror("Failed to send");
		return -1;
	}
	return 0;
}
