CC=cc
EXEC=rasm.exe

CFLAGS=-lm -lrt -march=native -o $(EXEC)
CFLAGS_DBG = $(CFLAGS) -g -pthread -DRDD

.PHONY: prod debug clean

debug:
	$(CC) rasm.c $(CFLAGS_DBG)

prod:
	$(CC) rasm.c $(CFLAGS)


clean:
	rm -rf *.o

