;
;  Speed-optimized LZSA2 decompressor by spke & uniabis (216 bytes)
;

	DEFINE	UNROLL_LONG_MATCHES						; uncomment for faster decompression of very compressible data (+38 bytes)
;	DEFINE	BACKWARD_DECOMPRESS						; uncomment for data compressed with option -b

	IFNDEF	BACKWARD_DECOMPRESS

		MACRO NEXT_HL
		inc hl
		ENDM

		MACRO ADD_OFFSET
		ex de,hl : add hl,de
		ENDM

		MACRO COPY1
		ldi
		ENDM

		MACRO COPYBC
		ldir
		ENDM

	ELSE

		MACRO NEXT_HL
		dec hl
		ENDM

		MACRO ADD_OFFSET
		ex de,hl : ld a,e : sub l : ld l,a
		ld a,d : sbc h : ld h,a						; 4*4+3*4 = 28t / 7 bytes
		ENDM

		MACRO COPY1
		ldd
		ENDM

		MACRO COPYBC
		lddr
		ENDM

	ENDIF


macro DecompressLZSA2
@lzsa2
		; A' stores next nibble as %1111.... or assumed to contain trash
		; B is assumed to be 0
		ld b,0 : scf : exa : jr .ReadToken

.ManyLiterals:	ld a,18 : add (hl) : NEXT_HL : jr nc,.CopyLiterals
		ld c,(hl) : NEXT_HL
		ld a,b : ld b,(hl)
		jr .NEXTHLuseBC


.MoreLiterals:	ld b,(hl) : NEXT_HL
		scf : exa : jr nc,.noUpdatemoar

			ld a,(hl) : or #F0 : exa
			ld a,(hl) : NEXT_HL : or #0F
			rrca : rrca : rrca : rrca

.noUpdatemoar	;sub #F0-3 : cp 15+3 : jr z,ManyLiterals
		inc a : jr z,.ManyLiterals : sub #F0-3+1

.CopyLiterals:	ld c,a : ld a,b : ld b,0
		COPYBC
		push de : or a : jp p,.CASE0xx ;: jr CASE1xx

		cp %11000000 : jr c,.CASE10x

.CASE11x		cp %11100000 : jr c,.CASE110

		; "111": repeated offset
.CASE111:	ld de,ix : jr .MatchLen


.Literals0011:	jr nz,.MoreLiterals

		; if "LL" of the byte token is equal to 0,
		; there are no literals to copy
.NoLiterals:	or (hl) : NEXT_HL
		push de : jp m,.CASE1xx

		; short (5 or 9 bit long) offsets
.CASE0xx		ld d,#FF : cp %01000000 : jr c,.CASE00x

		; "01x": the case of the 9-bit offset
.CASE01x:	cp %01100000 : rl d

.ReadOffsetE	ld e,(hl) : NEXT_HL

.SaveOffset:	LD ix,de

.MatchLen:	inc a : and %00000111 : jr z,.LongerMatch : inc a

.CopyMatch:	ld c,a
;.useC
		ex (sp),hl						; BC = len, DE = offset, HL = dest, SP ->[dest,src]
		ADD_OFFSET						; BC = len, DE = dest, HL = dest-offset, SP->[src]
		COPY1
		COPYBC
.popSrc		pop hl

		; compressed data stream contains records
		; each record begins with the byte token "XYZ|LL|MMM"
.ReadToken:	ld a,(hl) : and %00011000 : jp pe,.Literals0011		; process the cases 00 and 11 separately

		rrca : rrca : rrca

		ld c,a : ld a,(hl)					; token is re-read for further processing
.NEXTHLuseBC	NEXT_HL
		COPYBC

		; the token and literals are followed by the offset
		push de : or a : jp p,.CASE0xx

.CASE1xx		cp %11000000 : jr nc,.CASE11x

		; "10x": the case of the 13-bit offset
.CASE10x:	ld c,a : exa : jr nc,.noUpdatecase10x

			ld a,(hl) : or #F0 : exa
			ld a,(hl) : NEXT_HL : or #0F
			rrca : rrca : rrca : rrca

.noUpdatecase10x	ld d,a : ld a,c
		cp %10100000 : dec d : rl d : jr .ReadOffsetE


		
		; "110": 16-bit offset
.CASE110:	ld d,(hl) : NEXT_HL : jr .ReadOffsetE




		; "00x": the case of the 5-bit offset
.CASE00x:	ld c,a : exa : jr nc,.noUpdatecase00x

			ld a,(hl) : or #F0 : exa
			ld a,(hl) : NEXT_HL : or #0F
			rrca : rrca : rrca : rrca

.noUpdatecase00x	ld e,a : ld a,c
		cp %00100000 : rl e : jp .SaveOffset


.LongerMatch:	scf : exa : jr nc,.noUpdatelongermatch

			ld a,(hl) : or #F0 : exa
			ld a,(hl) : NEXT_HL : or #0F
			rrca : rrca : rrca : rrca

.noUpdatelongermatch	sub #F0-9 : cp 15+9 : jr c,.CopyMatch


.LongMatch:	add (hl) : NEXT_HL : jr c,.VeryLongMatch

		ld c,a
.useC		ex (sp),hl
		ADD_OFFSET
		COPY1

		; this is an unrolled equivalent of LDIR
		xor a : sub c
		and 32-1 : add a
		ld (.jrOffset),a : jr nz,$+2
.jrOffset	EQU $-1
.fastLDIR	repeat 32
		COPY1
		rend
		jp pe,.fastLDIR
		jp .popSrc

.VeryLongMatch:	ld c,(hl) : NEXT_HL
		ld b,(hl) : NEXT_HL : jr nz,.useC
		pop de : ret

mend





