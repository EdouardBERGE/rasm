CC = cc
CFLAGS  = -O2
RM = rm
EXTENSION = .exe

all: zx0 dzx0

zx0: zx0.c optimize.c compress.c memory.c
	$(CC) $(CFLAGS) -o zx0$(EXTENSION) zx0.c optimize.c compress.c memory.c

dzx0: dzx0.c
	$(CC) $(CFLAGS) -o dzx0$(EXTENSION) dzx0.c

clean:
	$(RM) *.obj
