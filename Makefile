CC = gcc
BASICFLAGS = -std=gnu11 -Wall -Wextra -Wpedantic
OPTFLAGS = -g
#extra flags for profiling
#EXFLAGS = -fprofile-arcs -ftest-coverage
CFLAGS = $(BASICFLAGS) $(OPTFLAGS) $(EXFLAGS)

TGTLIST = sigtest

all: $(TGTLIST)


sigtest:	sigtest.c crc.c

clean:
	rm -f *.o
	rm -f $(TGTLIST)
