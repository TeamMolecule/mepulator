#include "arm.h"

#include <stdio.h>
#include <unistd.h>

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

	printf("arm: init shared buffer\n");
	comm->Write32(0x10, 0x40000000 | 1); // set shared buffer
	printf("arm: wait6\n");
	while (!(ret = comm->Read32(0)));
	comm->Write32(0, 0);
	printf("arm: got ret2 0x%x\n", ret);

	printf("arm: set_rvk\n");
	comm->Write32(0x10, 0x80A01);
	while ((ret = comm->Read32(0x10)));
	printf("arm: got ret3 0x%x\n", ret);
}
