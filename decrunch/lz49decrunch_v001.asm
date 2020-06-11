;
; LZ48 decrunch
; input
; hl  compressed data adress
; de  output adress of data
;
; output
; hl    last adress of compressed data read (you must inc once for LZ48 stream)
; de    last adress of decrunched data write +1
; bc    always 3
; a     always zero
; lx    undetermined
; flags (inc a -> 0)

org #8000

; CALL #8000,source,destination
di

; parameters
ld h,(ix+3)
ld l,(ix+2)
ld d,(ix+1)
ld e,(ix+0)

call LZ49_decrunch

ei
ret





LZ49_decrunch
ldi
ld b,0

nextsequence
ld a,(hl)
inc hl
ld lx,a
and #70
jr z,lzunpack ; no litteral bytes
rrca
rrca
rrca
rrca

ld c,a
cp 7 ; more bytes for length?
jr nz,copyliteral

getadditionallength
ld a,(hl)
inc hl
inc a
jr nz,lengthnext
inc b
dec bc
jr getadditionallength
lengthnext
dec a
add a,c
ld c,a
ld a,b
adc a,0
ld b,a ; bc=length

copyliteral
ldir

lzunpack
ld a,lx
and #F
add 3
ld c,a
cp 18 ; more bytes for length?
jr nz,readoffset

getadditionallengthbis
ld a,(hl)
inc hl
inc a
jr nz,lengthnextbis
inc b
dec bc
jr getadditionallengthbis
lengthnextbis
dec a
add a,c
ld c,a
ld a,b
adc a,0
ld b,a ; bc=length

readoffset
ld a,lx
add a
jr c,extendedoffset
; read encoded offset
ld a,(hl)
inc a
ret z ; LZ48 end with zero offset
inc hl
push hl
ld l,a
ld a,e
sub l
ld l,a
ld a,d
sbc a,0
ld h,a
; source=dest-copyoffset
ldir
pop hl
jr nextsequence

extendedoffset
ld a,(hl)
inc hl
push hl
inc a
ld l,a
ld a,e
sub l
ld l,a
ld a,d
sbc a,1
ld h,a
; source=dest-copyoffset
ldir
pop hl
jr nextsequence




