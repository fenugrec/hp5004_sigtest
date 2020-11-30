Simulate the HP 5004A signature analyzer algorithm to check & compare ROMs.

This tool serves one main purpose : to generate "signatures" for a certain ROM dump, to compare against service manual listings.

For example : an SM could list signatures for a few different PROM options or versions. These are normally obtained by clocking on a certain address bit, and probing each bit of the data bus. So the SM lists a signature for each data bit.
This software does the same by emulating the "CRC" algorithm of the 5004A.

Based on sigrok's protocol decoder ( https://sigrok.org/wiki/Protocol_decoder:Signature )
and https://www.hpl.hp.com/hpjournal/pdfs/IssuePDFs/1977-05.pdf .


# status

Seems to mostly work :
- feeding constant '1' bit for N clocks  work
- N-bit binary counter (e.g. address counter) produces correct sigs for all bits
- hp3478A signatures, as tested with ROM dump "dc118.bin" (available on ko4bb.com), match 'Revision C' of the SM signatures almost perfectly except 3 data lines. Typos in the SM , or slightly different revision ?

# usage
`./sigtest` without arguments : runs built-in tests

`./sigtest rom.bin` : generates signatures for `rom.bin`

# example
comparing the last 2kB chunk of HP3478A ROM :
```
$ ./sigtest hpchunk.bin 
clocks: 0x00800
constant 1: 7A70
D0 : 32FC
D1 : 192A
D2 : 87U7
D3 : 6508
D4 : 643H
D5 : 5866
D6 : 67H3
D7 : 0229
```
Service manual, figure 7-D-2 "flowchart A"

```
			revC	(revB)
U502 pin 9: 32FC (P5FP)
U502 pin 10: 192A (P5FP)
U502 p1n 11: 87U7 (F91F)
U502 pin 13: 65C8 (H6F3)
U502 pin 14: 643H (6967)
U502 pin 15: 5866 (PAF5)
U502 pin 16: 67H3 (89FH)
U502 pin 17: 0229 (PA7C)
```
in this case, the signatures for D3 (U502 pin 13) differ by one character, but all the others match perfectly. This is suspicious as one would expect this kind of CRC algorithm to produce very different values given even minor changes in the input stream. Also 'C' looks like '0' so this could very well be a copy error.
