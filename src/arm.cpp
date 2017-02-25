#include "arm.h"

#include <stdio.h>

#include "devices/armcomm.h"

ARM::ARM(ARMComm *comm_):
	comm(comm_)
{}

void ARM::Loop() {
	printf("arm: starting up\n");
	while (comm->Read32(0) != 257);
	printf("arm: wait5 done\n");
}
