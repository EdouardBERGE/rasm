;
; LZ48 decrunch
;

; In	; HL=compressed data address
; 	; DE=output data address
; Out	; HL    last address of compressed data read (you must inc once for LZ48 stream)
;	; DE    last address of decrunched data write +1
;	; BC    always 3
;	; A     always zero
;	; IXL   undetermined
;	; flags (inc a -> 0)
; Modif	; AF, BC, DE, HL, IXL
LZ48_decrunch
	ldi
	ld b,0

nextsequence
	ld a,(hl)
	inc hl
	cp #10
	jr c,lzunpack ; no literal bytes
	ld ixl,a
	and #f0
	rrca
	rrca
	rrca
	rrca

	cp 15 ; more bytes for literal length?
	jr nz,copyliteral
getadditionallength
	ld c,(hl) ; get additional literal length byte
	inc hl
	add a,c ; compute literal length total
	jr nc,lengthNC
	inc b
lengthNC
	inc c
	jr z,getadditionallength ; if last literal length byte was 255, we have more bytes to process
copyliteral
	ld c,a
	ldir
	ld a,ixl
	and #F
lzunpack
	add 3
	cp 18 ; more bytes for match length?
	jr nz,readoffset
getadditionallengthbis
	ld c,(hl) ; get additional match length byte
	inc hl
	add a,c ; compute match length size total
	jr nc,lengthNCbis
	inc b
lengthNCbis
	inc c
	jr z,getadditionallengthbis ; if last match length byte was 255, we have more bytes to process

readoffset
	ld c,a
; read encoded offset
	ld a,(hl)
	inc a
	ret z ; LZ48 end with zero offset
	inc hl
	push hl
; source=dest-copyoffset
	; A != 0 here
	neg
	ld l,a
	ld h,#ff
	add hl,de
copykey
	ldir

	pop hl
	jr nextsequence
