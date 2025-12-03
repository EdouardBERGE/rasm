cl z80-master\z80.c -I z80-master /O2 /Ob3 /c /Foz80.obj
cl emuZ80.c /O2 z80.obj -DOS_WIN


