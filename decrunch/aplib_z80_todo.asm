;Z80 Version by Dan Weiss
;Call depack.
;hl = source
;de = dest

ap_bits: .db 0
ap_byte: .db 0
lwm:	.db 0
r0:	.dw 0

ap_getbit:
	push bc
		ld bc,(ap_bits)
		rrc c
		jr nc,ap_getbit_continue
		ld b,(hl)
		inc hl
ap_getbit_continue:
		ld a,c
		and b
		ld (ap_bits),bc
	pop bc
	ret

ap_getbitbc: ;doubles BC and adds the read bit
	sla c
	rl b
	call ap_getbit
	ret z
	inc bc
	ret

ap_getgamma:
	ld bc,1
ap_getgammaloop:
	call ap_getbitbc
	call ap_getbit
	jr nz,ap_getgammaloop
	ret


depack:
	;hl = source
	;de = dest
	ldi
	xor a
	ld (lwm),a
	inc a
	ld (ap_bits),a
	
aploop:
	call ap_getbit
	jp z, apbranch1
	call ap_getbit
	jr z, apbranch2
	call ap_getbit
	jr z, apbranch3
	;LWM = 0
	xor a
	ld (lwm),a
	;get an offset
	ld bc,0
	call ap_getbitbc
	call ap_getbitbc
	call ap_getbitbc
	call ap_getbitbc
	ld a,b
	or c
	jr nz,apbranch4
	xor a  ;write a 0
	ld (de),a
	inc de
	jr aploop
apbranch4:
	ex de,hl ;write a previous bit (1-15 away from dest)
	push hl
		sbc hl,bc
		ld a,(hl)
	pop hl
	ld (hl),a
	inc hl
	ex de,hl
	jr aploop
apbranch3:
	;use 7 bit offset, length = 2 or 3
	;if a zero is encountered here, it's EOF
	ld c,(hl)
	inc hl
	rr c
	ret z
	ld b,2
	jr nc,ap_dont_inc_b
	inc b
ap_dont_inc_b:
	;LWM = 1
	ld a,1
	ld (lwm),a
	
	push hl
		ld a,b
		ld b,0
		;R0 = c
		ld (r0),bc
		ld h,d
		ld l,e
		or a
		sbc hl,bc
		ld c,a
		ldir
	pop hl
	jr aploop
apbranch2:
	;use a gamma code * 256 for offset, another gamma code for length
	call ap_getgamma
	dec bc
	dec bc
	ld a,(lwm)
	or a
	jr nz,ap_not_lwm
	;bc = 2?
	ld a,b
	or c
	jr nz,ap_not_zero_gamma
	;if gamma code is 2, use old r0 offset, and a new gamma code for length
	call ap_getgamma
	push hl
		ld h,d
		ld l,e
		push bc
			ld bc,(r0)
			sbc hl,bc
		pop bc
		ldir
	pop hl
	jr ap_finishup
	
ap_not_zero_gamma:
	dec bc
ap_not_lwm:
	;do I even need this code?
	;bc=bc*256+(hl), lazy 16bit way
	ld b,c
	ld c,(hl)
	inc hl
	ld (r0),bc
	push bc
		call ap_getgamma
		ex (sp),hl
		;bc = len, hl=offs
		push de
			ex de,hl
			;some comparison junk for some reason
			ld hl,31999
			or a
			sbc hl,de
			jr nc,skip1
			inc bc
skip1:
			ld hl,1279
			or a
			sbc hl,de
			jr nc,skip2
			inc bc
skip2:
			ld hl,127
			or a
			sbc hl,de
			jr c,skip3
			inc bc
			inc bc
skip3:
			;bc = len, de = offs, hl=junk
		pop hl
		push hl
			or a
			sbc hl,de
		pop de
		;hl=dest-offs, bc=len, de = dest
		ldir
	pop hl
ap_finishup:
	ld a,1
	ld (lwm),a
	jp aploop

apbranch1:
	ldi
	xor a
	ld (lwm),a
	jp aploop
