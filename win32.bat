cl ZX0-main\src\compress.c      /Ox /c -I ZX0-main\src
cl ZX0-main\src\optimize.c      /Ox /c -I ZX0-main\src
cl ZX0-main\src\memory.c        /Ox /c -I ZX0-main\src

cl rasm.c /Ox /DNOAPULTRA=1 /DDOS_WIN=1 optimize.obj compress.obj memory.obj


