<img src="assets/logo.png" alt="Logo courtesy of Barjack" title="logo">

## Rasm Z80 assembler (is a COMMAND LINE tool)
    
Rasm was designed to be damn FAST and handle HUGE projects. On a modern machine, it can assemble millions of instructions in a blink of an eye (peak performance around 100 millions instructions per second). It's way faster than the best alternatives. Rasm also runs on almost any platform : Linux x64/ARM, MacOS Intel or ARM, Windows 32 bits or 64 bits, MS-DOS and even MorphOS

## This fork (vs upstream EdouardBERGE/rasm)

*File Handling*:
- New: path/filename lookups (`INCLUDE`, `SAVE`, ...) are now case-insensitive on case-sensitive filesystems (Linux/macOS), matching Windows/WinAPE behaviour.
- New: quoted filenames (`SAVE`, `BUILDCPR`, `INCLUDE`, ...) accept either `\` or `/` as path separator regardless of host OS; the wrong one is normalized instead of producing a literal backslash-named file on Linux/macOS.
- Changed: plain-file `SAVE` (no DSK/TAPE/AMSDOS/HOBETA) accepts offset/size beyond 64K when the bank is `EXTENDED`; hardware-bound SAVE variants stay capped at 64K.

*Relocation Tables*:
- New: `RELOCATE_START [HIGH]` / `RELOCATE_END` / `RELOCATE_TABLE [subtract_offset]`, WinAPE/SymbOS-compatible relocation table generator (distinct from the pre-existing `RELOCATE`/`ENDRELOCATE`). Labels, as well as '$'/'$$' inside the block are tracked as relocatable. `RELOCATE_TABLE` scans every 16bit slot assembled so far — `DEFW`/`DW`/struct fields as well as instruction operands (`LD reg,nn`, `JP`/`CALL nn`, `LD (nn),reg`, ...) — and records the position of every one whose value is exactly a single relocatable address plus/minus a compile-time constant. An expression combining two or more relocatable references is recorded only when it's provably delta-invariant, i.e. a plain `+`/`-` combination whose signs cancel out to a net zero (e.g. `labelA-labelB`, or `labelA-labelB+labelC-labelD`, even through nested parentheses like `-(labelA-labelB)`). Such a value needs no fixup at all. Anything else that combines multiple relocatable items like a sum (`labelA+labelB`), or one scaled by multiplication/division, cannot be expressed with the single per-entry fixup delta the table format supports. Those cases will throw an assembler error.
Without `HIGH`, each table entry stores the position of the low byte of each 16bit slot to fix, thus pointing to the whole LittleEndian word. 
With `HIGH`, the table entries point to the high byte of each 16bit slot to fix. This makes the file "page relocatable" by just adding an 8 bit delta to where the pointer points.   
The table is written inline, wherever `RELOCATE_TABLE` is placed. Its format is (word)RELOCATE_COUNT, (word)RELOCATE_SIZE, n*(word)SymbolAddress
- Changed: `INSTRUCTION_MAXLENGTH` upped by one to support `RELOCATE_START`/`RELOCATE_TABLE`, as they need 15 bytes with the zero-termination included.

*Assembler*:
- New: `$$` symbol, always the current physical output position (`outputadr`), usable in any expression — unlike `$`, whose meaning is context-dependent (`codeadr` normally, but
outputadr in ORG like ORG $+100). This behavior is preserved, so we don't break existing code. 
- New: `codeadr` overflow past `0xFFFF` now wraps to 0 with a warning instead of growing unbounded.
  Can be silenced by `-nowarning`.
- New: `LIMIT <value>,EXTENDED` opts a bank into output past 64K (`outputadr`); `codeadr` (Z80 PC) still must stay a valid 16-bit address.
- Fixed: tokenizer false positive ("empty parameter right after word") on an unrecognized mnemonic followed by `, ` (comma then space) before its next argument; only a genuinely empty slot (`,,`) is flagged now.
- Fixed: Maxam mode (`-m`) — `DIV`/`MOD` masked their result to unsigned 16-bit *after* dividing instead of masking the operands first, so e.g. `-3 mod 256` returned 65533 instead of the correct (floored-modulo, Maxam) 253.
- Fixed: `ld a,(<indexreg>-number)`, or any variant with a negative index offset, now also works in maxam mode. See rasm.c:10263 for details.
- Fixed: a lone `$`/`$$` immediately preceded by a unary minus (e.g. `-$`) failed to resolve ("keyword not found") in immediately-evaluated contexts such as `DS`'s size argument.
- Fixed: aN unary minus before a forward-referenced `EQU` constant or anything that is handled as an alias (e.g. `ld de,-foo` with `foo equ 14` defined later) resolved to the wrong (positive) value. The alias substitution mechanism overwrote the minus sign itself, and once preserved, needed the same "leading unary minus" handling used at expression start to correctly wrap it around a parenthesized alias expansion.
- Fixed: division/modulo by zero in an expression now reports an error instead of crashing rasm (SIGFPE).
- Fixed: `ORG` beyond 64K outside a crunched section segfaulted (missing `MakeError` format args) instead of reporting a clean error.
- Fixed: Removed stray direct formatting `[%s:%d]` at rasm.c:21025, 21716, 21729, 21739, 21752, 22545.
 
*General*:
- New: `.gitignore` covers selftest output files.
- New: `Makefile.linux` builds rasm without the `.exe` extension; `make -f Makefile.linux windows64` cross-builds for Windows-x86-64 using mingw.
- Changed: the `clean` target in `makefile`, `Makefile.linux`, and `makefile.MacOS` also removes selftest output files (not the binary itself).


## Documentation

There is a cool French and English documentation, you should read it ;)

- EN: http://rasm.wikidot.com/english-index:home
- FR: http://rasm.wikidot.com/

## Integrated crunched sections

- Rasm know how to load&crunch on the fly files when doing INCBIN (including binaries)
- Rasm is able to assemble and crunch portions of code, and relocate following instructions
- Rasm can assemble as much code as you want in each memory space and break the 64K barrier before crunch!
- Many crunchers are supported: LZ4, LZ48, LZ49, ZX0, ZX7, Exomizer 2, AP-Ultra, LZSA1, LZSA2

## Native format

- CPC regular and extended cartridges
- CPC snapshots up to 4M with RAM and ROM support
- CPC EDSK (as many as you want in update or generation mode)
- floppy HFE engine to manage ANY particular format
- CPC/ZX TAPE output
- CPC/ZX Binaries with or without AMSDOS/HOBETA header
- ZX snapshots
- you can initialize memory with a snapshot or a cartridge (usefull to patch memory)

## Debug output

- symbols import, or export in files, CPC snapshot
- breakpoint export in CPC snapshot or files
- dependencies export for makefile creation
- [ACE-DL](https://roudoudou.com/ACE-DL/) debug chunk in snapshot and cartridge for unprecedented debug experience

## Mostly unlimited design

- ORG checking, unlimited memory workspaces where labels are shared, bank management, structures
- conditionnal macro, unlimited & embeded loops with local labels, switch/case
- floating point engine, mathematical functions, ...
- crunched segments declaration may overflow the 64K limit as much as you want
- UTF8 characters mapper for any text remapping design

## Compatibility for archeology (with old versions of these assemblers)

- MAXAM
- AS80
- UZ80
- Pasmo

## Oldschool is beautiful

- Amsdos real & Microsoft basic 40bits float support

## Newschool is beautiful

- native FLOAT24 declaration support (which can be use with float24 library in RASM sources asset directory)

## Quality

Each build is tested every release, with 1915 automated tests. We use TDD, end-to-end and exploratory testing.

## History

The project was born a few years ago (2017) because i needed a really fast assembler in order to finish my demo before the deadline of the Alchemy 2017. The project was quite huge with approx 350.000 words to assemble but thanks to rasm, the 512K cartridge compiled in a blink of an eye even on my slow Atom CPU. Now Rasm is the reference for huge projects and ultra-fast assembling. You can expect tens of millions instructions per second on a modern CPU.

## Building RASM

As RASM does not rely on library or something, the compilation was always intended to be easy. I made simple makefile or scripts for this

### Linux / Raspberry PI

> make release

On Linux you can expect significant performance improvements with the mimalloc library

> make superfastmi

### MacOS

> make -f makefile.MacOS release

### Windows with any recent Visual Studio release

> compilProd.bat

### Windows XP with latest supported MSVC 15

> win32Prod.bat

## Links

You can play with RASM live on https://cpc-playground.iiivan.org/

Arkos Tracker is powered by RASM on https://www.julien-nevo.com/arkostracker/

RetroDev intégrated dev environnement with RASM by the Lord of the Bits! https://github.com/tlotb/retrodev

NEW! RASM on your CPC with developper studio! https://norecess464.weebly.com/news/pistudio-a-raspberry-pi-powered-ide-for-the-amstrad-cpc

![RASM on a CPC](https://github.com/EdouardBERGE/rasm/blob/master/Norecess.png)


BTW there is many other assemblers you could try:

- ORGAMS http://orgams.wikidot.com/ (definitely the best native Z80 assembler on any machine)
- BASM https://github.com/cpcsdk/rust.cpclib/releases (powerfull and Orgams compatible assembler)
- VASM http://sun.hasenbraten.de/vasm/
- SjASM http://xl2s.tk/
- SjASMPlus (fork) https://github.com/z00m128/sjasmplus
- SjASMPlus (fork) https://github.com/sjasmplus/sjasmplus
- UZ80 http://cngsoft.no-ip.org/uz80.htm
- PASMO https://pasmo.speccy.org/

