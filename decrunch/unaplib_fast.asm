;
;  Speed-optimized ApLib decompressor by spke (ver.04 spring 2020, 236 bytes)
;
;  The original Z80 decompressors for ApLib were written by Dan Weiss (Dwedit),
;  then tweaked by Francisco Javier Pena Pareja (utopian),
;  and optimized by Jaime Tejedor Gomez (Metalbrain) and Antonio Villena.
;
;  This is a new "implicit state" decompressor heavily optimized for speed by spke.
;  (It is 11 bytes shorter and 14% faster than the previously fastest
;  247b decompressor by Metalbrain and Antonio Villena.)
;
;  ver.00 by spke (21/08/2018-01/09/2018, 244 bytes, an edit of the existing 247b decompressor);
;  ver.01 by spke (12-13/11/2018, 234(-10) bytes, +3% speed using the state machine for LWM);
;  ver.02 by spke (06/08/2019, +1% speed);
;  ver.03 by spke (27/08/2019, 236(+2) bytes, +1% speed using partly expanded LDIR);
;  ver.04 by spke (spring 2020, added full revision history and support for long offsets)
;
;  The data must be compressed using any compressor for ApLib capable of generating raw data.
;  At present, two best available compressors are:
;
;  "APC" by Sven-Ake Dahl: https://github.com/svendahl/cap or
;  "apultra" by Emmanuel Marty: https://github.com/emmanuel-marty/apultra
;
;  The compression can done as follows:
;
;  apc.exe e <sourcefile> <outfile>
;  or
;  apultra.exe <sourcefile> <outfile>
;
;  A decent compressor was written by r57shell (although it is worse than compressors above):
;  http://gendev.spritesmind.net/forum/viewtopic.php?p=32548#p32548
;  The use of the official ApLib compressor by Joergen Ibsen is not recommended.
;
;  The decompression is done in the standard way:
;
;  ld hl,FirstByteOfCompressedData
;  ld de,FirstByteOfMemoryForDecompressedData
;  call DecompressApLib
;
;  The decompressor modifies AF, AF', BC, DE, HL, IXH, IY.
;  (However, note that the option "AllowSelfmodifyingCode" removes the dependency on IY.)
;
;  Of course, ApLib compression algorithms are (c) 1998-2014 Joergen Ibsen,
;  see http://www.ibsensoftware.com/ for more information
;
;  Drop me an email if you have any comments/ideas/suggestions: zxintrospec@gmail.com
;
;  This software is provided 'as-is', without any express or implied
;  warranty.  In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Permission is granted to anyone to use this software for any purpose,
;  including commercial applications, and to alter it and redistribute it
;  freely, subject to the following restrictions:
;
;  1. The origin of this software must not be misrepresented; you must not
;     claim that you wrote the original software. If you use this software
;     in a product, an acknowledgment in the product documentation would be
;     appreciated but is not required.
;  2. Altered source versions must be plainly marked as such, and must not be
;     misrepresented as being the original software.
;  3. This notice may not be removed or altered from any source distribution.

;	DEFINE SupportLongOffsets				; +4 bytes for long offset support. slows decompression down by 1%, but may be needed to decompress files >=32K

macro APunpack
		ld a,128 : jr @CASE0

;==================================================================================================================
;==================================================================================================================
;==================================================================================================================

@LWM0:			;LWM = 0 (LWM stands for "Last Was Match"; a flag that we did not have a match)

@ReloadByteC0		ld a,(hl) : inc hl : rla
			jr c,@Check2ndBit

;
;  case "0"+BYTE: copy a single literal

@CASE0:			ldi						; first byte is always copied as literal

;
;  main decompressor loop

@CASE0_MainLoop:		add a : jr z,@ReloadByteC0 : jr nc,@CASE0	; "0"+BYTE = copy literal
@Check2ndBit		add a : call z,ReloadByte : jr nc,@CASE10	; "10"+gamma(offset/256)+BYTE+gamma(length) = the main matching mechanism
			add a : call z,ReloadByte : jp c,@CASE111	; "110"+[oooooool] = matched 2-3 bytes with a small offset

;
;  branch "110"+[oooooool]: copy two or three bytes (bit "l") with the offset -1..-127 (bits "ooooooo"), or stop

@CASE110:		; "use 7 bit offset, length = 2 or 3"
			; "if a zero is found here, it's EOF"
			ld c,(hl) : rr c : ret z			; process EOF
			inc hl
			ld b,0

			ld iyl,c : ld iyh,b				; save offset for future LWMs

			push hl						; save src
			ld h,d : ld l,e					; HL = dest
			jr c,@LengthIs3

@LengthIs2		sbc hl,bc
			ldi : ldi
			jr @PreMainLoop

@LengthIs3		or a : sbc hl,bc
			ldi : ldi : ldi
			jr @PreMainLoop

;
;  branch "10"+gamma(offset/256)+BYTE+gamma(length): the main matching mechanism

@CASE10:		; "use a gamma code * 256 for offset, another gamma code for length"
			call @GetGammaCoded

			; the original decompressor contains
			;
			; if ((LWM == 0) && (offs == 2)) { ... }
			; else {
			;	if (LWM == 0) { offs -= 3; }
			;	else { offs -= 2; }
			; }
			;
			; so, the idea here is to use the fact that GetGammaCoded returns (offset/256)+2,
			; and to split the first condition by noticing that C-1 can never be zero
			dec c : dec c : jr z,@KickInLWM

@AfterLWM		dec c : ld b,c : ld c,(hl) : inc hl	; BC = offset

			ld iyl,c : ld iyh,b : push bc

			call @GetGammaCoded			; BC = len*

			ex (sp),hl

			; interpretation of length value is offset-dependent:
			; if (offs >= 32000) len++; if (offs >= 1280) len++; if (offs < 128) len+=2;
			; in other words,
			; (1 <= offs < 128) +=2
			; (128 <= offs < 1280) +=0
			; (1280 <= offs < 31999) +=1
			; NB offsets over 32000 need one more check, but other Z80 decompressors seem to ignore it. is it not needed?

			; interpretation of length value is offset-dependent
			exa : ld a,h
	IFDEF	SupportLongOffsets
			; NB offsets over 32000 require an additional check, which is skipped in most
			; Z80 decompressors (seemingly as a performance optimization)
			cp 32000/256 : jr nc,@Add2
	ENDIF
			cp 5 : jr nc,@Add1
			or a : jr nz,@Add0
			bit 7,l : jr nz,@Add0
@Add2			inc bc
@Add1			inc bc
@Add0			; for offs<128 : 4+4+7+7 + 4+7 + 8+7 + 6+6 = 60t
			; for offs>=1280 : 4+4+7+12 + 6 = 33t
			; for 128<=offs<1280 : 4+4+7+7 + 4+12 = 38t OR 4+4+7+7 + 4+7+8+12 = 53t
;			dec bc

@CopyMatch:		; this assumes that BC = len, DE = offset, HL = dest
			; and also that (SP) = src, while having NC
			ld a,e : sub l : ld l,a
			ld a,d : sbc h
@CopyMatchLDH		ld h,a : ldi : ldir : exa
@PreMainLoop		pop hl					; recover src

;==================================================================================================================
;==================================================================================================================
;==================================================================================================================

@LWM1:			; LWM = 1

;
;  main decompressor loop

@MainLoop:		add a : jr z,@ReloadByteC0 : jr nc,@CASE0		; "0"+BYTE = copy literal
@Check2ndBit		add a : call z,@ReloadByte : jr nc,@CASE10		; "10"+gamma(offset/256)+BYTE+gamma(length) = the main matching mechanism
			add a : call z,@ReloadByte : jr nc,@CASE110		; "110"+[oooooool] = matched 2-3 bytes with a small offset

;
;  case "111"+"oooo": copy a byte with offset -1..-15, or write zero to dest

@CASE111:		ld bc,%11100000
			DUP 4
			add a : call z,@ReloadByte : rl c		; read short offset (4 bits)
			EDUP
			ex de,hl : jr z,@WriteZero		; zero offset means "write zero" (NB: B is zero here)

			; "write a previous byte (1-15 away from dest)"
			push hl					; BC = offset, DE = src, HL = dest
			sbc hl,bc				; HL = dest-offset (SBC works because branching above ensured NC)
			ld b,(hl)
			pop hl

@WriteZero		ld (hl),b : ex de,hl
			inc de : jp @CASE0_MainLoop				; 10+4*(4+10+8)+4+7 + 11+15+7+10 + 7+4+6+10 = 179t

@ReloadByteC0		ld a,(hl) : inc hl : rla
			jp nc,@CASE0
			jr @Check2ndBit

;
;  branch "10"+gamma(offset/256)+BYTE+gamma(length): the main matching mechanism

@CASE10:		; "use a gamma code * 256 for offset, another gamma code for length"
			call @GetGammaCoded

			; the original decompressor contains
			;
			; if ((LWM == 0) && (offs == 2)) { ... }
			; else {
			;	if (LWM == 0) { offs -= 3; }
			;	else { offs -= 2; }
			; }
			;
			; so, the idea here is to use the fact that GetGammaCoded returns (offset/256)+2,
			; and to split the first condition by noticing that C-1 can never be zero
			dec c : jp @AfterLWM

;
;  the re-use of the previous offset (LWM magic)

@KickInLWM:		; "and a new gamma code for length"
			call @GetGammaCoded			; BC = len
			push hl
			exa : ld a,e : sub iyl : ld l,a
			ld a,d : sbc iyh
			jp @CopyMatchLDH

;==================================================================================================================
;==================================================================================================================
;==================================================================================================================

;
;  interlaced gamma code reader
;  x0 -> 1x
;  x1y0 -> 1xy
;  x1y1z0 -> 1xyz etc
;  (technically, this is a 2-based variation of Exp-Golomb-1)

@GetGammaCoded:		ld bc,1
@ReadGamma		add a : jr z,@ReloadByteRG1
			rl c : rl b
			add a : jr z,@ReloadByteRG2
			jr c,@ReadGamma : ret

@ReloadByteRG1		ld a,(hl) : inc hl : rla
			rl c : rl b
			add a : jr c,@ReadGamma : ret

@ReloadByteRG2		ld a,(hl) : inc hl : rla
			jr c,@ReadGamma : ret

;
;  pretty usual getbit for mixed datastreams

@ReloadByte:		ld a,(hl) : inc hl : rla : ret

mend

