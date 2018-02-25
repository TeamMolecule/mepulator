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
	TRACE("Got packet: %s\n", packet.c_str());
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

int Debugger::SendChar(int ch) {
	uint8_t buf = ch;
	if (send(client_fd, &buf, sizeof(buf), 0) == -1) {
		perror("Failed to send");
		return -1;
	}
	return 0;
}
