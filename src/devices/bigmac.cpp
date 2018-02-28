#include "bigmac.h"

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "memory.h"
#include "util.h"

#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"

#define DEBUG_CRYPTO 1

Bigmac::Bigmac(Memory *mem_):
	mem(mem_)
{}

uint32_t Bigmac::Read32(uint32_t addr) {
	if (addr == 0x3C) {
		printf("access RNG\n");
		return 0x11223344;
	}
	if (addr == 0x24)
		return channels[0].busy;
	FATAL("unknown addr 0x%x on read\n", addr);
}

void Bigmac::Write32(uint32_t addr, uint32_t value) {
	if (addr >= 0x200) {
		addr -= 0x200;
		if (addr >= sizeof(control2) || addr & 0b11)
			FATAL("invalid control2 addr 0x200+0x%x\n", addr);
		control2[addr / 4] = value;
	} else if (addr >= 0x100) {
		addr -= 0x100;
		if (addr >= sizeof(control) || addr & 0b11)
			FATAL("invalid control addr 0x100+0x%x\n", addr);
		control[addr / 4] = value;
	} else {
		if (addr & 0b11)
			FATAL("unaligned address 0x%x\n", addr);
		int c = (addr >= sizeof(bigmac_regs)) ? 1 : 0;
		addr -= sizeof(bigmac_regs) * c;
		bigmac_regs *ch = &channels[c];
		*(uint32_t*)((char*)ch + addr) = value;
		if (addr == 0x1C) {
			printf("---- Bigmac commit: channel %d -------------------------------------------------------\n", c);
			printf("src: 0x%08x dst:  0x%08x sz:  0x%08x func: 0x%08x keyslot: 0x%08x\n", ch->src, ch->dst, ch->sz, ch->func, ch->keyslot);
			printf("iv:  0x%08x 0x18: 0x%08x cmt: 0x%08x 0x20: 0x%08x busy:    0x%08x\n", ch->iv, ch->unk_0x18, ch->commit, ch->unk_0x20, ch->busy);
			hex_dump(0x28, (uint8_t*)ch->unk, sizeof(ch->unk));
			printf("control:\n");
			hex_dump(0, (uint8_t*)control, sizeof(control));
			printf("control2:\n");
			hex_dump(0, (uint8_t*)control2, sizeof(control2));
			printf("-------------------------------------------------------------------------------------\n");

			if (value != 1)
				FATAL("commit: got unknown value 0x%x\n", value);

			ch->busy = 1;
			DoFunc(c);
			ch->busy = 0;
		}
	}
}

static void aes_decrypt(void *data, size_t data_sz, void *key, size_t key_sz, void *iv) {
	mbedtls_aes_context aes = {0};
	if (key_sz != 128 && key_sz != 192 && key_sz != 256)
		FATAL("bad key size: %d\n", key_sz);
	if (data_sz & 0xF)
		FATAL("bad data size: 0x%x\n", data_sz);
	if (mbedtls_aes_setkey_dec(&aes, (unsigned char*)key, key_sz))
		FATAL("mbedtls_aes_setkey_dec failed\n");

#if DEBUG_CRYPTO
	printf("-- aes-%d-cbc decrypt --\n", key_sz);

	printf("key:\n");
	hex_dump(0, (uint8_t*)key, key_sz/8);
	printf("iv:\n");
	hex_dump(0, (uint8_t*)iv, 16);
	printf("data:\n");
	hex_dump(0, (uint8_t*)data, data_sz);
#endif

	uint8_t *tmp = new uint8_t[data_sz];
	if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, data_sz, (unsigned char *)iv, (unsigned char *)data, (unsigned char *)tmp))
		FATAL("mbedtls_aes_crypt_cbc failed\n");

#if DEBUG_CRYPTO
	printf("decrypted:\n");
	hex_dump(0, (uint8_t*)tmp, data_sz);
#endif

	memcpy(data, tmp, data_sz);
	delete[] tmp;
}

static void hmac_sha256(uint8_t *data, size_t data_len, uint8_t *key, size_t key_len, uint8_t *output) {
	if (key_len != 0x20) {
		FATAL("key_len is not 0x20\n");
	}

	printf("-- hmac-sha256 --\n");
	printf("data:\n");
	hex_dump(0, data, data_len);
	printf("key:\n");
	hex_dump(0, key, key_len);

	uint8_t ipad[64] = { 0 };
	uint8_t opad[64] = { 0 };
	memcpy(ipad, key, key_len);
	memcpy(opad, key, key_len);

	for (int i = 0; i < 64; ++i) {
		ipad[i] ^= 0x36;
		opad[i] ^= 0x5c;
	}

	mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    uint8_t tmp[0x20] = { 0 };

    // Inner sha-256
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, ipad, 64);
    mbedtls_sha256_update(&ctx, data, data_len);
    mbedtls_sha256_finish(&ctx, tmp);

    // Outer sha-256
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, opad, 64);
    mbedtls_sha256_update(&ctx, tmp, 32);
    mbedtls_sha256_finish(&ctx, output);

    mbedtls_sha256_free(&ctx);

    printf("output:\n");
    hex_dump(0, output, 0x20);
}

void Bigmac::DoFunc(int channel) {
	bigmac_regs *ch = &channels[channel];
	switch (ch->func) {
	case 0x2080:
	case 0: { // memcpy
		uint8_t *tmp = new uint8_t [ch->sz];
		mem->Read(ch->src, ch->sz, tmp);
		mem->Write(ch->dst, ch->sz, tmp);
		delete[] tmp;
		break;
	}
	case 0x2093:  { // sha256
		uint8_t *data = new uint8_t [ch->sz];
		mem->Read(ch->src, ch->sz, data);
		uint8_t hash[256/8] = {0};
		mbedtls_sha256(data, ch->sz, hash, 0);
		mem->Write(ch->dst, sizeof(hash), hash);

#if DEBUG_CRYPTO
		printf("-- sha256 --\n");
		printf("input:\n");
		hex_dump(0, data, ch->sz);
		printf("hash:\n");
		hex_dump(0, hash, sizeof(hash));
#endif

		delete[] data;

		break;
	}
	case 0x218a:    // aes-128-cbc decrypt w/o keyslot
	case 0x238a:  { // aes-256-cbc decrypt w/o keyslot
		int keybits;
		if (ch->func == 0x218a)
			keybits = 128;
		else if (ch->func == 0x238a)
			keybits = 256;
		else
			FATAL("keybits\n");

		uint8_t *data = new uint8_t [ch->sz];
		uint8_t *iv = new uint8_t [16];

		mem->Read(ch->src, ch->sz, data);
		mem->Read(ch->iv, 16, iv);

		aes_decrypt(data, ch->sz, control2, keybits, iv);

		mem->Write(ch->iv, 16, iv);
		mem->Write(ch->dst, ch->sz, data);
		
		delete[] data;
		delete[] iv;
		break;
	}
	case 0x20b3: { // HMAC-SHA256
		uint8_t *data = new uint8_t [ch->sz];
		uint8_t key[0x20];
		uint8_t digest[0x20];

		memcpy(key, control2, 0x20);
		mem->Read(ch->src, ch->sz, data);

		hmac_sha256(data, ch->sz, key, 0x20, digest);

		mem->Write(ch->dst, 0x20, digest);

		delete[] data;
		break;
	}
	default:
		FATAL("unknown function 0x%x\n", ch->func);
	}
}
