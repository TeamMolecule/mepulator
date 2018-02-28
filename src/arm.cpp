#include "arm.h"

#include <stdio.h>
#include <unistd.h>

#include "devices/armcomm.h"
#include "memory.h"
#include "log.h"

enum {
	RVK_BUF_SIZE = 0x1000,
	SHARED_BUFFER = 0x40000000,
	RVK_PA_LIST = 0x40001000,
	RVK_PA = 0x40002000,
};

typedef union {
  struct {
    uint32_t num_paddrs;
    uint32_t rvk_paddr_list;
  } rvk_init;
  struct {
    uint32_t paddr;
    uint32_t len;
  } buf_0x80_init;
  struct {
    uint32_t num_paddrs;
    uint32_t paddr_list;
    uint32_t buf_0x40; // field_30 in sm_handle
    uint32_t field_18;
    uint32_t ctx_0x4;
    uint32_t ctx_0x8;
    uint32_t ctx_0xC;
    uint32_t pad_unk;
    uint32_t field_60;
    uint32_t partition_id;
    uint64_t auth_id;
    uint64_t part_0xF00C;
    uint64_t part_0xFFFF;
    uint64_t part_unk;
    uint64_t part_unk2;
  } sm;
  struct {
    uint32_t num_paddrs;
    uint32_t paddr_list;
    uint32_t buf_0x40;
    uint32_t delayed_cmd;
  } sm2;
  unsigned char raw[0x100];
} shared_buffer_t;


ARM::ARM(ARMComm *comm_, Memory *mem_):
	comm(comm_),
	mem(mem_)
{}

void ARM::StartUp() {
	TRACE("starting up...\n");
	while (comm->Read32(0) != 257);
	TRACE("wait for 0x101 done\n");
}

void ARM::SetSharedBuffer() {
	uint32_t ret;

	TRACE("set shared_buffer to 0x%x\n", SHARED_BUFFER);
	comm->Write32(0x10, SHARED_BUFFER | 1);
	TRACE("wait...");
	while (!(ret = comm->Read32(0)));
	TRACE("wait done, 0x%x\n", ret);
}

void ARM::SetRvk() {
	uint32_t ret;

	char *rvk_buf = new char[RVK_BUF_SIZE];

	// prepare rvk list
	FILE *fin = fopen("prog_rvk.srvk", "rb");
	if (!fin)
		FATAL("cannot open prog_rvk.srvk\n");
	uint32_t rvk_sz = fread(rvk_buf, 1, RVK_BUF_SIZE, fin);
	fclose(fin);
	mem->Write(RVK_PA, rvk_sz, rvk_buf);
	delete[] rvk_buf;

	// prepare palist for rvk
	uint32_t rvk_palist[] = { RVK_PA, rvk_sz };
	mem->Write(RVK_PA_LIST, sizeof(rvk_palist), rvk_palist);

	// prepare shared_buffer
	shared_buffer_t shared = {0};
	shared.rvk_init.num_paddrs = 1;
	shared.rvk_init.rvk_paddr_list = RVK_PA_LIST;
	mem->Write(SHARED_BUFFER, sizeof(shared), &shared);

	TRACE("set_rvk\n");
	comm->Write32(0x10, 0x80A01);
	while ((ret = comm->Read32(0x10))); // commit
	while (!(ret = comm->Read32(0)));
	TRACE("set_rvk ret: 0x%x\n", ret);
}

void ARM::Loop() {
	StartUp();
	SetSharedBuffer();
	usleep(100 * 1000); // TODO: figure the race here? sometimes, setting rvk just fails
	SetRvk();
}
