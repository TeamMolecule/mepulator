#include "mathdev.h"

#include "log.h"
#include "util.h"

#include <string.h>

#include "mbedtls/bignum.h"

uint32_t Math::Read32(uint32_t addr) {
	if (addr == 0x804) {
		return flags;
	} else if (addr >= 0x508 && addr < 0x608) {
		addr -= 0x500;
		return *(uint32_t*)&res[addr];
	}
	FATAL("Trying to read 0x%08X\n", addr);
}

void Math::Write32(uint32_t addr, uint32_t value) {
	if (addr == 0x800) {
		flags = 0x4000000;
		cmd = value;
		return;
	} else if (addr == 0x808) {
		exponent = value;
		PowMod();
	} else if (addr >= 0x100 && addr < 0x300) {
		addr -= 0x100;
		*(uint32_t*)&sig[addr] = value;
	} else if (addr >= 0x400 && addr < 0x500) {
		addr -= 0x400;
		*(uint32_t*)&modulus[addr] = value;
	} else {
		FATAL("Trying to write 0x%08X value=0x%08X - not implemented\n", addr, value);
	}
}

static void byteswap(uint8_t *arr, size_t sz) {
	for (size_t i = 0; i < sz / 2; ++i) {
		uint8_t tmp = arr[i];
		arr[i] = arr[sz - i - 1];
		arr[sz - i - 1] = tmp;
	}
}

static void print_big_num(const mbedtls_mpi *num) {
	char buf[4096];
	size_t olen = 0;
	mbedtls_mpi_write_string(num, 16, buf, sizeof(buf), &olen);
	printf("0x%s", buf);
}

void Math::PowMod() {
	TRACE("command=0x%08X\n", cmd);
	if ((cmd >> 27) != 0x12) {
		FATAL("unsupported command\n");
		// probably not completely correct check
	}

	uint32_t mod_size_words = (cmd & 0x3FC0000) >> 18;
	uint32_t exp_size_words = (cmd & 0x3FE00) >> 9;

	if (mod_size_words != 0x40 || exp_size_words != 0x1)
		FATAL("unsupported modulus/exponent size\n");

	uint8_t be_mod[0x100];
	uint8_t be_sig[0x100];
	uint8_t be_res[0x100];
	uint8_t be_exp[4];

	memcpy(be_mod, modulus, 0x100);
	memcpy(be_sig, sig + 8, 0x100);
	memcpy(be_exp, &exponent, 4);

	// Little=>Big endian
	byteswap(be_mod, 0x100);
	byteswap(be_sig, 0x100);
	byteswap(be_exp, 0x4);

	mbedtls_mpi n_mod, n_exp, n_sig, n_res;
	mbedtls_mpi_init(&n_mod);
	mbedtls_mpi_init(&n_exp);
	mbedtls_mpi_init(&n_sig);
	mbedtls_mpi_init(&n_res);

	mbedtls_mpi_read_binary(&n_mod, be_mod, sizeof(be_mod));
	mbedtls_mpi_read_binary(&n_sig, be_sig, sizeof(be_sig));
	mbedtls_mpi_read_binary(&n_exp, be_exp, sizeof(be_exp));

	printf("(");
	print_big_num(&n_sig);
	printf(" ** ");
	print_big_num(&n_exp);
	printf(") %% ");
	print_big_num(&n_mod);
	printf(" = ");

	mbedtls_mpi_exp_mod(&n_res, &n_sig, &n_exp, &n_mod, NULL);

	print_big_num(&n_res);
	printf("\n");

	mbedtls_mpi_write_binary(&n_res, be_res, sizeof(be_res));

	mbedtls_mpi_free(&n_mod);
	mbedtls_mpi_free(&n_exp);
	mbedtls_mpi_free(&n_sig);
	mbedtls_mpi_free(&n_res);

	// Big=>Little endian
	byteswap(be_res, 0x100);
	memcpy(res + 8, be_res, 0x100);

	// flags=size of output
	flags = 0x40 << 16;
}