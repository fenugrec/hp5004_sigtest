/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Sun Nov 29 22:33:13 2020
 * by pycrc v0.9.2, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x1281
 *  - XorIn         = 0x0000
 *  - ReflectIn     = False
 *  - XorOut        = 0x0000
 *  - ReflectOut    = False
 *  - Algorithm     = bit-by-bit
 */
#include "crc.h"     /* include the header file generated with pycrc */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    bool bit;
    unsigned char c;

    while (data_len--) {
        c = *d++;
        for (i = 0; i < 8; i++) {
            bit = crc & 0x8000;
            crc = (crc << 1) | ((c >> (7 - i)) & 0x01);
            if (bit) {
                crc ^= 0x1281;
            }
        }
        crc &= 0xffff;
    }
    return crc & 0xffff;
}


crc_t crc_finalize(crc_t crc)
{
    unsigned int i;
    bool bit;

    for (i = 0; i < 16; i++) {
        bit = crc & 0x8000;
        crc <<= 1;
        if (bit) {
            crc ^= 0x1281;
        }
    }
    return crc & 0xffff;
}
