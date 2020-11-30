#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stypes.h"

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

/** test vectors for counter test */
const unsigned count_vectors[] = {
	0x800,	//example : hp3478A. signatures match
	0x1000,
	0x8000,	//example: boonton 4210, signatures match
	0x10000,
	0};	//delimit


//get crc for bit <bitno> of a counter from 0 to (counts - 1)
u16 test_counter(unsigned counts, unsigned bitno) {
	unsigned cnt;
	u16 crc = 0;
	for (cnt = 0; cnt < counts; cnt++) {
		unsigned newbit = (cnt >> bitno) & 1;
		crc = crc1281_singlebit(newbit, crc);
	}
	return revbits(crc);
}

// get crc of constant bit stream
u16 calc_vector(unsigned clocks, unsigned bitval) {
	unsigned bitcnt;
	u16 crc = 0;
	for (bitcnt = 0; bitcnt < clocks; bitcnt++) {
			crc = crc1281_singlebit(bitval, crc);
	}
	crc = revbits(crc);
	return crc;
}

void test_algo(void) {
	unsigned i;

	/************ test sig charset */
	printf("printsig: ");
	uint16_t dummy_sig = 0x0123;
	
	for (i=0; i <= 0x03; i++) {
		print_sig(dummy_sig);
		dummy_sig += 0x4444U;
	}
	printf("\n");

	/************ test simple vectors */
	for (i=0; vectors[i].testlen; i++) {
		uint16_t crc;

		crc = calc_vector(vectors[i].testlen, vectors[i].bitval);
		
		printf("testlen %05X, bitval %u, sig: ",
				vectors[i].testlen, vectors[i].bitval);
		print_sig(crc);
		printf("\n");
	}

	/************ test counter bits */
	for (i=0; count_vectors[i]; i++) {
		// for each count : repeat for every bit position
		unsigned count = count_vectors[i];
		unsigned curbit;
		for (curbit = 0; (count -1) >> curbit; curbit++) {
			u16 crc;
			crc = test_counter(count, curbit);
			printf("counter : %X (%u) clocks, bit %u: ", count, count, curbit);
			print_sig(crc);
			printf("\n");
		}
	}
}


// hax, get file length but restore position
u32 flen(FILE *hf) {
	long siz;
	long orig;

	if (!hf) return 0;
	orig = ftell(hf);
	if (orig < 0) return 0;

	if (fseek(hf, 0, SEEK_END)) return 0;

	siz = ftell(hf);
	if (siz < 0) siz=0;
		//the rest of the code just won't work if siz = UINT32_MAX
	#if (LONG_MAX >= UINT32_MAX)
		if ((long long) siz == (long long) UINT32_MAX) siz = 0;
	#endif

	if (fseek(hf, orig, SEEK_SET)) return 0;
	return (u32) siz;
}


#define FILE_MAXSIZE 64*1024UL
void rom_printsigs(FILE *i_file) {
	u32 file_len;

	rewind(i_file);
	file_len = flen(i_file);
	if ((!file_len) || (file_len > FILE_MAXSIZE)) {
		printf("huge file (length %lu)\n", (unsigned long) file_len);
		return;
	}

	u8 *src = malloc(file_len);
	if (!src) {
		printf("malloc choke\n");
		return;
	}

	/* load whole ROM */
	if (fread(src,1,file_len,i_file) != file_len) {
		printf("trouble reading\n");
		free(src);
		return;
	}

	printf("clocks: 0x%05X\n", file_len);
	printf("constant 1: ");
	print_sig(calc_vector(file_len, 1));
	printf("\n");
	unsigned databit;
	for (databit = 0; databit <= 7; databit++) {
		//for every bit position : generate signature as if probing signal
		unsigned cnt;
		u16 crc = 0;
		for (cnt = 0; cnt < file_len; cnt++) {
			unsigned newbit = (src[cnt] >> databit) & 1;
			crc = crc1281_singlebit(newbit, crc);
		}
		crc = revbits(crc);
		printf("D%u : ", databit);
		print_sig(crc);
		printf("\n");
	}
	return;
}

int main(int argc, char **argv) {
	FILE *romfile;

	if (argc != 2) {
		printf("usage : %s <romfile>\n", argv[0]);
		test_algo();
		return 0;
	}

	romfile = fopen(argv[1], "rb");
	if (!romfile) {
		printf("fopen() failed: %s\n", strerror(errno));
		return -1;
	}
	rom_printsigs(romfile);
	fclose(romfile);
	return 0;
}