CC=cc
EXEC=rasm.exe

CFLAGS=-lm -lrt -march=native -o $(EXEC)
CFLAGS_OPT = $(CFLAGS) -O2
CFLAGS_DBG = $(CFLAGS) -g -pthread -DRDD
CFLAGS_3RD = $(CFLAGS) -g -pthread -DNO_3RD_PARTIES

SRC_APUDIR=./apultra-master/src
SRC_LZSADIR=./lzsa-master/src

APU_FLAGS=-c -O3 -fomit-frame-pointer -I$(SRC_LZSADIR)/libdivsufsort/include -I$(SRC_APUDIR)

APU_OBJ =$(SRC_APUDIR)/expand.o
APU_OBJ+=$(SRC_APUDIR)/matchfinder.o
APU_OBJ+=$(SRC_APUDIR)/shrink.o
APU_OBJ+=$(SRC_APUDIR)/libdivsufsort/lib/divsufsort.o
APU_OBJ+=$(SRC_APUDIR)/libdivsufsort/lib/divsufsort_utils.o
APU_OBJ+=$(SRC_APUDIR)/libdivsufsort/lib/sssort.o
APU_OBJ+=$(SRC_APUDIR)/libdivsufsort/lib/trsort.o

LZSA_FLAGS=-c -O3 -fomit-frame-pointer -I$(SRC_LZSADIR)/libdivsufsort/include -I$(SRC_LZSADIR)

LZSA_OBJ =$(SRC_LZSADIR)/dictionary.o
LZSA_OBJ+=$(SRC_LZSADIR)/expand_block_v1.o
LZSA_OBJ+=$(SRC_LZSADIR)/expand_block_v2.o
LZSA_OBJ+=$(SRC_LZSADIR)/expand_context.o
LZSA_OBJ+=$(SRC_LZSADIR)/expand_inmem.o
LZSA_OBJ+=$(SRC_LZSADIR)/frame.o
LZSA_OBJ+=$(SRC_LZSADIR)/matchfinder.o
LZSA_OBJ+=$(SRC_LZSADIR)/shrink_block_v1.o
LZSA_OBJ+=$(SRC_LZSADIR)/shrink_block_v2.o
LZSA_OBJ+=$(SRC_LZSADIR)/shrink_context.o
LZSA_OBJ+=$(SRC_LZSADIR)/shrink_inmem.o
LZSA_OBJ+=$(SRC_LZSADIR)/stream.o

.PHONY: prod third debug clean

default: prod

third:
	$(CC) rasm.c $(CFLAGS_3RD)

debug:
	$(CC) $(SRC_APUDIR)/expand.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/expand.o
	$(CC) $(SRC_APUDIR)/matchfinder.c $(APU_FLAGS)                           -o $(SRC_APUDIR)/matchfinder.o
	$(CC) $(SRC_APUDIR)/shrink.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/shrink.o

	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort.c $(APU_FLAGS)         -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort_utils.c $(APU_FLAGS)   -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort_utils.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/sssort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/sssort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/trsort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/trsort.o

	$(CC) $(SRC_LZSADIR)/matchfinder.c $(LZSA_FLAGS)       -o $(SRC_LZSADIR)/matchfinder.o
	$(CC) $(SRC_LZSADIR)/dictionary.c $(LZSA_FLAGS)        -o $(SRC_LZSADIR)/dictionary.o
	$(CC) $(SRC_LZSADIR)/expand_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v1.o
	$(CC) $(SRC_LZSADIR)/expand_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v2.o
	$(CC) $(SRC_LZSADIR)/expand_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/expand_context.o
	$(CC) $(SRC_LZSADIR)/expand_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/expand_inmem.o
	$(CC) $(SRC_LZSADIR)/frame.c $(LZSA_FLAGS)             -o $(SRC_LZSADIR)/frame.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v1.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v2.o
	$(CC) $(SRC_LZSADIR)/shrink_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/shrink_context.o
	$(CC) $(SRC_LZSADIR)/shrink_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/shrink_inmem.o
	$(CC) $(SRC_LZSADIR)/stream.c $(LZSA_FLAGS)            -o $(SRC_LZSADIR)/stream.o

	$(CC) rasm.c $(CFLAGS_DBG)

prod:
	$(CC) $(SRC_APUDIR)/expand.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/expand.o
	$(CC) $(SRC_APUDIR)/matchfinder.c $(APU_FLAGS)                           -o $(SRC_APUDIR)/matchfinder.o
	$(CC) $(SRC_APUDIR)/shrink.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/shrink.o

	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort.c $(APU_FLAGS)         -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort_utils.c $(APU_FLAGS)   -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort_utils.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/sssort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/sssort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/trsort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/trsort.o

	$(CC) $(SRC_LZSADIR)/matchfinder.c $(LZSA_FLAGS)       -o $(SRC_LZSADIR)/matchfinder.o
	$(CC) $(SRC_LZSADIR)/dictionary.c $(LZSA_FLAGS)        -o $(SRC_LZSADIR)/dictionary.o
	$(CC) $(SRC_LZSADIR)/expand_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v1.o
	$(CC) $(SRC_LZSADIR)/expand_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v2.o
	$(CC) $(SRC_LZSADIR)/expand_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/expand_context.o
	$(CC) $(SRC_LZSADIR)/expand_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/expand_inmem.o
	$(CC) $(SRC_LZSADIR)/frame.c $(LZSA_FLAGS)             -o $(SRC_LZSADIR)/frame.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v1.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v2.o
	$(CC) $(SRC_LZSADIR)/shrink_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/shrink_context.o
	$(CC) $(SRC_LZSADIR)/shrink_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/shrink_inmem.o
	$(CC) $(SRC_LZSADIR)/stream.c $(LZSA_FLAGS)            -o $(SRC_LZSADIR)/stream.o

	$(CC) rasm.c $(CFLAGS_OPT) $(APU_OBJ) $(LZSA_OBJ)
	strip $(EXEC)

reload:
	$(CC) rasm.c $(CFLAGS_OPT) $(APU_OBJ) $(LZSA_OBJ)
	strip $(EXEC)

release:
	$(CC) $(SRC_APUDIR)/expand.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/expand.o
	$(CC) $(SRC_APUDIR)/matchfinder.c $(APU_FLAGS)                           -o $(SRC_APUDIR)/matchfinder.o
	$(CC) $(SRC_APUDIR)/shrink.c $(APU_FLAGS)                                -o $(SRC_APUDIR)/shrink.o

	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort.c $(APU_FLAGS)         -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/divsufsort_utils.c $(APU_FLAGS)   -o $(SRC_APUDIR)/libdivsufsort/lib/divsufsort_utils.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/sssort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/sssort.o
	$(CC) $(SRC_LZSADIR)/libdivsufsort/lib/trsort.c $(APU_FLAGS)             -o $(SRC_APUDIR)/libdivsufsort/lib/trsort.o

	$(CC) $(SRC_LZSADIR)/matchfinder.c $(LZSA_FLAGS)       -o $(SRC_LZSADIR)/matchfinder.o
	$(CC) $(SRC_LZSADIR)/dictionary.c $(LZSA_FLAGS)        -o $(SRC_LZSADIR)/dictionary.o
	$(CC) $(SRC_LZSADIR)/expand_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v1.o
	$(CC) $(SRC_LZSADIR)/expand_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/expand_block_v2.o
	$(CC) $(SRC_LZSADIR)/expand_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/expand_context.o
	$(CC) $(SRC_LZSADIR)/expand_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/expand_inmem.o
	$(CC) $(SRC_LZSADIR)/frame.c $(LZSA_FLAGS)             -o $(SRC_LZSADIR)/frame.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v1.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v1.o
	$(CC) $(SRC_LZSADIR)/shrink_block_v2.c $(LZSA_FLAGS)   -o $(SRC_LZSADIR)/shrink_block_v2.o
	$(CC) $(SRC_LZSADIR)/shrink_context.c $(LZSA_FLAGS)    -o $(SRC_LZSADIR)/shrink_context.o
	$(CC) $(SRC_LZSADIR)/shrink_inmem.c $(LZSA_FLAGS)      -o $(SRC_LZSADIR)/shrink_inmem.o
	$(CC) $(SRC_LZSADIR)/stream.c $(LZSA_FLAGS)            -o $(SRC_LZSADIR)/stream.o
	$(CC) rasm.c $(CFLAGS_OPT) $(APU_OBJ) $(LZSA_OBJ)
	strip $(EXEC)
	./upx --brute rasm.exe

clean:
	rm -rf *.o

