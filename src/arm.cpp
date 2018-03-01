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
	TRACEBUF_PA = 0x41000000,
	TRACEBUF_SIZE = 0x1000,

	SM_PA_LIST = 0x50000000, // size=8
	SM_0x40_BUFFER = 0x50001000, // size=0x40
	SM_PA = 0x51000000,
	SM_SIZE = 0x40000,
};

typedef union {
  struct {
    uint32_t num_paddrs;
    uint32_t rvk_paddr_list;
  } rvk_init;
  struct {
    uint32_t paddr;
    uint32_t len;
  } tracebuf;
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

void ARM::SetTracebuf() {
	uint32_t ret = 0;

	shared_buffer_t shared = {0};
	shared.tracebuf.paddr = TRACEBUF_PA;
	shared.tracebuf.len = TRACEBUF_SIZE;
	mem->Write(SHARED_BUFFER, sizeof(shared), &shared);

	TRACE("wait 1\n");
	comm->Write32(0x10, 0x80901);
	while (comm->Read32(0x10));
	TRACE("wait 1 done\n");

	do {
		ret = comm->Read32(0);
	} while (!(uint16_t)ret);
	comm->Write32(0, ret);
	TRACE("wait 2 done, ret=0x%08X\n", ret);
	if (ret & 0x8000)
		TRACE("wait 2 error: 0x%x\n", 0x800F0300 | (ret & 0xFF));
}

void ARM::LoadSm() {
	FILE *fin = fopen("kprx_auth_sm.self", "rb");
	if (!fin) {
		FATAL("failed to open sm self file\n");
	}

	uint8_t *sm_buf = new uint8_t[SM_SIZE];
	uint32_t sm_sz = fread(sm_buf, 1, SM_SIZE, fin);
	mem->Write(SM_PA, sm_sz, sm_buf);
	delete[] sm_buf;

	shared_buffer_t shared = {0};

	uint32_t sm_palist[] = { SM_PA, sm_sz };
	mem->Write(SM_PA_LIST, sizeof(sm_palist), sm_palist);

	shared.sm.num_paddrs = 1;
	shared.sm.paddr_list = SM_PA_LIST;
	shared.sm.buf_0x40 = SM_0x40_BUFFER;
	// shared.sm.field_60 = t->field_60; // change = error SCE_SBL_ERROR_DRV_ESYSEXVER 0x800f0338
	// shared.sm.partition_id = t->partition_id; // change = error SCE_SBL_ERROR_DRV_ENOTINITIALIZED 0x800f0332
	// shared.sm.auth_id = t->auth_id;         // change = no effect
	// shared.sm.part_0xF00C = t->part_0xF00C; // change = no effect
	// shared.sm.part_0xFFFF = t->part_0xFFFF; // change = no effect
	mem->Write(SHARED_BUFFER, sizeof(shared), &shared);

	TRACE("wait 1\n");
	comm->Write32(0x10, 0x500201);
	int32_t ret = 0;
	do {
		ret = comm->Read32(0x10);
		if (ret < 0)
			FATAL("ret: 0x%08X\n", ret);
		printf("Ret: 0x%08X\n", ret);
	} while (ret);

	uint32_t other = comm->Read32(0x0);
	printf("Other: 0x%08X\n", other); // sometimes we get 0x800F032A / 0x802A

	TRACE("wait 1 done\n");
}

void ARM::Loop() {
	StartUp();
	SetSharedBuffer();
	usleep(100 * 1000); // TODO: figure the race here? sometimes, setting rvk just fails
	SetRvk();
	SetTracebuf();
	LoadSm();
}
