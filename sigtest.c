#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "crc.h"

const char symbols[]="0123456789ACFHPU";

uint16_t revbits(uint16_t v) {
	unsigned i;
	unsigned out = 0;
	for (i = 0; i < 16; i++) {
		out = (out << 1) | (v & 1);
		v >>= 1;
	}
	return (uint16_t) out;
}

void print_sig(uint16_t sig) {
	int i;
	for (i=3; i >= 0; i--) {
		printf("%c", symbols[(sig >> (4 * i)) & 0x0F]);
	}
	return;
}

unsigned bitcount(unsigned v) {
	unsigned int c; // c accumulates the total bits set in v
	for (c = 0; v; c++)
	{
	  v &= v - 1; // clear the least significant bit set
	}
	return c;
}
	
uint16_t crc1281_singlebit(int newbit, uint16_t crc) {
	bool bit;

/*
	                incoming = (bin(shiftreg & 0x0291).count('1') + data) & 1
                shiftreg = (incoming << 15) | (shiftreg >> 1)
*/
#if 1
	bit = (bitcount(crc & 0x0291) + newbit) & 1;
	crc = (bit << 15) | (crc >> 1);
#endif

#if 0
	bit = crc & 0x8000;
	crc = (crc << 1) | newbit;
	if (bit) {
		crc ^= 0x1281;
	}
#endif
#if 0
	bit = crc & 0x8000;
	crc = (crc << 1) | newbit;
	if (bit) {
		crc ^= 0x0291;
	}
#endif
#if 0
	bit = crc & 0x8000;
	crc = (newbit << 15) | (crc >> 1);
	if (bit) {
		crc ^= 0x1281;
	}
#endif
	return crc;

}

/** note : initval sans effet */
struct testvec {
	unsigned testlen;
	unsigned initval;
	unsigned bitval;
};

const struct testvec vectors[] = {
	{2, 0, 1},
	{3, 0, 1},
	{4, 0, 1},
	{5, 0, 1},
	{1U << 11, 0, 0},
	{1U << 11, 0, 1},
//	{1U << 11, 1, 0},
//	{1U << 11, 1, 1},
	{1U << 12, 0, 0},
	{1U << 12, 0, 1},
//	{1U << 12, 1, 0},
//	{1U << 12, 1, 1},
	{1U << 15, 0, 0},
	{1U << 15, 0, 1},
//	{1U << 15, 1, 0},
//	{1U << 15, 1, 1},
	{1U << 16, 0, 0},
	{1U << 16, 0, 1},
//	{1U << 16, 1, 0},
//	{1U << 16, 1, 1},
	{0, 0, 0}
};
	

void test_algo(void) {
	unsigned i;

	printf("printsig: ");
	uint16_t dummy_sig = 0x0123;
	
	for (i=0; i <= 0x03; i++) {
		print_sig(dummy_sig);
		dummy_sig += 0x4444U;
	}
	printf("\n");

	for (i=0; vectors[i].testlen; i++) {
		unsigned bitcnt;
		uint16_t crc;

		crc = vectors[i].initval;
		for (bitcnt = 0; bitcnt < vectors[i].testlen; bitcnt++) {
			crc = crc1281_singlebit(vectors[i].bitval, crc);
		}
		crc = revbits(crc);
		printf("testlen %05X, init %u, bitval %u, sig: ",
				vectors[i].testlen, vectors[i].initval, vectors[i].bitval);
		print_sig(crc);
		printf("\n");
	}

}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;
	
	test_algo();	


	return 0;
}