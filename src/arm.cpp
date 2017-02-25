#include "arm.h"

#include <stdio.h>

#include "devices/armcomm.h"

ARM::ARM(ARMComm *comm_):
	comm(comm_)
{}

void ARM::Loop() {
	uint32_t ret;

	printf("arm: starting up\n");
	while (comm->Read32(0) != 257);
	printf("arm: wait5 done\n");
	comm->Write32(0, 0);
	while (!(ret = comm->Read32(0)));
	printf("arm: got ret 0x%x\n", ret);

	printf("arm: init shared buffer\n");
	
}
