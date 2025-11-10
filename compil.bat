cl z80-master\z80.c -I z80-master /O2 /Ob3 /c /Foz80.obj

cl salvador\src\matchfinder.c       /O2 /Qpar /Ob3 /c -I salvador\src -I lzsa-master\src\libdivsufsort\include /Fomatch_salvador.obj
cl salvador\src\expand.c            /O2 /Qpar /Ob3 /c -I salvador\src -I lzsa-master\src\libdivsufsort\include /Foexpand_salvador.obj
cl salvador\src\shrink.c            /O2 /Qpar /Ob3 /c -I salvador\src -I lzsa-master\src\libdivsufsort\include /Foshrink_salvador.obj

cl apultra-master\src\expand.c      /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I apultra-master\src
cl apultra-master\src\matchfinder.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I apultra-master\src
cl apultra-master\src\shrink.c      /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I apultra-master\src

cl lzsa-master\src\dictionary.c      /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\expand_block_v1.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\expand_block_v2.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\expand_context.c  /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\expand_inmem.c    /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\frame.c           /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\matchfinder.c     /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src /Fomatchfinder_lzsa.obj
cl lzsa-master\src\shrink_block_v1.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\shrink_block_v2.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\shrink_context.c  /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src
cl lzsa-master\src\shrink_inmem.c    /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include -I lzsa-master\src

cl lzsa-master\src\libdivsufsort\lib\divsufsort.c       /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include
cl lzsa-master\src\libdivsufsort\lib\divsufsort_utils.c /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include
cl lzsa-master\src\libdivsufsort\lib\sssort.c           /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include
cl lzsa-master\src\libdivsufsort\lib\trsort.c           /O2 /Qpar /Ob3 /c -I lzsa-master\src\libdivsufsort\include

cl rasm.c /O2 expand.obj matchfinder.obj shrink.obj dictionary.obj expand_block_v1.obj expand_block_v2.obj expand_context.obj expand_inmem.obj frame.obj matchfinder_lzsa.obj shrink_block_v1.obj shrink_block_v2.obj shrink_context.obj shrink_inmem.obj divsufsort.obj divsufsort_utils.obj sssort.obj trsort.obj match_salvador.obj expand_salvador.obj shrink_salvador.obj z80.obj


