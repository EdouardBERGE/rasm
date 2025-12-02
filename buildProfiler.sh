cc -c -O2 z80-master/z80.c -o z80-master/z80.o
cc emuZ80.c z80-master/z80.o -o emuZ80.exe
strip emuZ80.exe

