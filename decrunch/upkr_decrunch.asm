;; https://github.com/exoticorn/upkr/blob/z80/c_unpacker/unpack.c - original C implementation
;; C source in comments ahead of asm - the C macros are removed to keep only bitstream variant
;; initial version by Peter "Ped" Helcmanovsky (C) 2022, licensed same as upkr project ("unlicensed")
;;
;; you can define UPKR_PROBS_ORIGIN to specific 256 byte aligned address for probs array (320 bytes),
;; otherwise it will be positioned after the unpacker code (256 aligned)
;;
;; public API:
;;
;;     upkr.unpack
;;         IN: IX = packed data, DE' (shadow DE) = destination
;;         OUT: IX = after packed data
;;         modifies: all registers except IY, requires 10 bytes of stack space
;;

;     DEFINE BACKWARDS_UNPACK         ; uncomment to build backwards depacker (write_ptr--, upkr_data_ptr--)
            ; initial IX points at last byte of compressed data
            ; initial DE' points at last byte of unpacked data

;     DEFINE UPKR_UNPACK_SPEED        ; uncomment to get larger but faster unpack routine

; code size hint: if you put probs array just ahead of BASIC entry point, you will get BC
; initialised to probs.e by BASIC `USR` command and you can remove it from unpack init (-3B)


NUMBER_BITS     EQU     16+15       ; context-bits per offset/length (16+15 for 16bit offsets/pointers)
    ; numbers (offsets/lengths) are encoded like: 1a1b1c1d1e0 = 0000'0000'001e'dbca

/*
u8* upkr_data_ptr;
u8 upkr_probs[1 + 255 + 1 + 2*32 + 2*32];
u16 upkr_state;
u8 upkr_current_byte;
int upkr_bits_left;

int upkr_unpack(void* destination, void* compressed_data) {
    upkr_data_ptr = (u8*)compressed_data;
    upkr_state = 0;
    upkr_bits_left = 0;
    for(int i = 0; i < sizeof(upkr_probs); ++i)
        upkr_probs[i] = 128;

    u8* write_ptr = (u8*)destination;

    int prev_was_match = 0;
    int offset = 0;
    for(;;) {
        if(upkr_decode_bit(0)) {
            if(prev_was_match || upkr_decode_bit(256)) {
                offset = upkr_decode_length(257) - 1;
                if(offset == 0) {
                    break;
                }
            }
            int length = upkr_decode_length(257 + 64);
            while(length--) {
                *write_ptr = write_ptr[-offset];
                ++write_ptr;
            }
            prev_was_match = 1;
        } else {
            int byte = 1;
            while(byte < 256) {
                int bit = upkr_decode_bit(byte);
                byte = (byte << 1) + bit;
            }
            *write_ptr++ = byte;
            prev_was_match = 0;
        }
    }

    return write_ptr - (u8*)destination;
}
*/
; IN: IX = compressed_data, DE' = destination
unpack:
  ; ** reset probs to 0x80, also reset HL (state) to zero, and set BC to probs+context 0
    ld      hl,probs_c>>1
    ld      bc,probs_e
    ld      a,$80
.reset_probs:
    dec     bc
    ld      (bc),a              ; will overwrite one extra byte after the array because of odd length
    dec     bc
    ld      (bc),a
    dec     l
    jr      nz,.reset_probs
    exa
    ; BC = probs (context_index 0), state HL = 0, A' = 0x80 (no source bits left in upkr_current_byte)

  ; ** main loop to decompress data
    ; D = prev_was_match = uninitialised, literal is expected first => will reset D to "false"
    ; values for false/true of prev_was_match are: false = high(probs), true = 1 + high(probs)
.decompress_data:
    ld      c,0
    call    decode_bit          ; if(upkr_decode_bit(0))
    jr      c,.copy_chunk

  ; * extract byte from compressed data (literal)
    inc     c                   ; C = byte = 1 (and also context_index)
.decode_byte:
    call    decode_bit          ; bit = upkr_decode_bit(byte);
    rl      c                   ; byte = (byte << 1) + bit;
    jr      nc,.decode_byte     ; while(byte < 256)
    ld      a,c
    exx
    ld      (de),a              ; *write_ptr++ = byte;
    IFNDEF BACKWARDS_UNPACK : inc de : ELSE : dec de : ENDIF
    exx
    ld      d,b                 ; prev_was_match = false
    jr      .decompress_data

  ; * copy chunk of already decompressed data (match)
.copy_chunk:
    ld      a,b
    inc     b                   ; context_index = 256
        ;             if(prev_was_match || upkr_decode_bit(256)) {
        ;                 offset = upkr_decode_length(257) - 1;
        ;                 if (0 == offset) break;
        ;             }
    cp      d                   ; CF = prev_was_match
    call    nc,decode_bit       ; if not prev_was_match, then upkr_decode_bit(256)
    jr      nc,.keep_offset     ; if neither, keep old offset
    call    decode_number       ; context_index is already 257-1 as needed by decode_number
    dec     de                  ; offset = upkr_decode_length(257) - 1;
    ld      a,d
    or      e
    ret     z                   ; if(offset == 0) break
    ld      (.offset+1),de
.keep_offset:
        ;             int length = upkr_decode_length(257 + 64);
        ;             while(length--) {
        ;                 *write_ptr = write_ptr[-offset];
        ;                 ++write_ptr;
        ;             }
        ;             prev_was_match = 1;
    ld      c,lo(257 + NUMBER_BITS - 1)    ; context_index to second "number" set for lengths decoding
    call    decode_number       ; length = upkr_decode_length(257 + 64);
    push    de
    exx
    IFNDEF BACKWARDS_UNPACK
        ; forward unpack (write_ptr++, upkr_data_ptr++)
        ld      h,d             ; DE = write_ptr
        ld      l,e
.offset:  ld  bc,0
        sbc     hl,bc           ; CF=0 from decode_number ; HL = write_ptr - offset
        pop     bc              ; BC = length
        ldir
    ELSE
        ; backward unpack (write_ptr--, upkr_data_ptr--)
.offset:  ld  hl,0
        add     hl,de           ; HL = write_ptr + offset
        pop     bc              ; BC = length
        lddr
    ENDIF
    exx
    ld      d,b                 ; prev_was_match = true
    djnz    .decompress_data    ; adjust context_index back to 0..255 range, go to main loop

/*
int upkr_decode_bit(int context_index) {
    while(upkr_state < 32768) {
        if(upkr_bits_left == 0) {
            upkr_current_byte = *upkr_data_ptr++;
            upkr_bits_left = 8;
        }
        upkr_state = (upkr_state << 1) + (upkr_current_byte >> 7);
        upkr_current_byte <<= 1;
        --upkr_bits_left;
    }

    int prob = upkr_probs[context_index];
    int bit = (upkr_state & 255) >= prob ? 1 : 0;

    int prob_offset = 16;
    int state_offset = 0;
    int state_scale = prob;
    if(bit) {
        state_offset = -prob;
        state_scale = 256 - prob;
        prob_offset = 0;
    }
    upkr_state = state_offset + state_scale * (upkr_state >> 8) + (upkr_state & 255);
    upkr_probs[context_index] = prob_offset + prob - ((prob + 8) >> 4);

    return bit;
}
*/
inc_c_decode_bit:
  ; ++low(context_index) before decode_bit (to get -1B by two calls in decode_number)
    inc     c
decode_bit:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; BC = probs+context_index
  ; A' = upkr_current_byte (!!! init to 0x80 at start, not 0x00)
  ; preserves DE
  ; ** while (state < 32768) - initial check
    push    de
    bit     7,h
    jr      nz,.state_b15_set
    exa
  ; ** while body
.state_b15_zero:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; A = upkr_current_byte (init to 0x80 at start, not 0x00)
    add     a,a                     ; upkr_current_byte <<= 1; // and testing if(upkr_bits_left == 0)
    jr      nz,.has_bit             ; CF=data, ZF=0 -> some bits + stop bit still available
  ; CF=1 (by stop bit)
    ld      a,(ix)
    IFNDEF BACKWARDS_UNPACK : inc ix : ELSE : dec ix : ENDIF    ; upkr_current_byte = *upkr_data_ptr++;
    adc     a,a                     ; CF=data, b0=1 as new stop bit
.has_bit:
    adc     hl,hl                   ; upkr_state = (upkr_state << 1) + (upkr_current_byte >> 7);
    jp      p,.state_b15_zero       ; while (state < 32768)
    exa
  ; ** set "bit"
.state_b15_set:
    ld      a,(bc)                  ; A = upkr_probs[context_index]
    dec     a                       ; prob is in ~7..249 range, never zero, safe to -1
    cp      l                       ; CF = bit = prob-1 < (upkr_state & 255) <=> prob <= (upkr_state & 255)
    inc     a
  ; ** adjust state
    push    bc
    ld      c,l                     ; C = (upkr_state & 255); (preserving the value)
    push    af
    jr      nc,.bit_is_0
    neg                             ; A = -prob == (256-prob), CF=1 preserved
.bit_is_0:
    ld      d,0
    ld      e,a                     ; DE = state_scale ; prob || (256-prob)
    ld      l,d                     ; H:L = (upkr_state>>8) : 0

  IFNDEF UPKR_UNPACK_SPEED

    ;; looped MUL for minimum unpack size
    ld      b,8                     ; counter
.mulLoop:
    add     hl,hl
    jr      nc,.mul0
    add     hl,de
.mul0:
    djnz    .mulLoop                ; until HL = state_scale * (upkr_state>>8), also BC becomes (upkr_state & 255)

  ELSE

    ;;; unrolled MUL for better performance, +25 bytes unpack size
    ld      b,d
    repeat     8
        add     hl,hl
        jr      nc,@mul0        ; rasm: label local a la boucle (sjasmplus: 0_f / 0:)
        add     hl,de
@mul0:
    rend

  ENDIF

    add     hl,bc                   ; HL = state_scale * (upkr_state >> 8) + (upkr_state & 255)
    pop     af                      ; restore prob and CF=bit
    jr      nc,.bit_is_0_2
    dec     d                       ; DE = -prob (also D = bit ? $FF : $00)
    add     hl,de                   ; HL += -prob
    ; ^ this always preserves CF=1, because (state>>8) >= 128, state_scale: 7..250, prob: 7..250,
    ; so 7*128 > 250 and thus edge case `ADD hl=(7*128+0),de=(-250)` => CF=1
.bit_is_0_2:
 ; *** adjust probs[context_index]
    rra                             ; + (bit<<4) ; part of -prob_offset, needs another -16
    and     $FC                     ; clear/keep correct bits to get desired (prob>>4) + extras, CF=0
    rra
    rra
    rra                             ; A = (bit<<4) + (prob>>4), CF=(prob & 8)
    adc     a,-16                   ; A = (bit<<4) - 16 + ((prob + 8)>>4) ; -prob_offset = (bit<<4) - 16
    ld      e,a
    pop     bc
    ld      a,(bc)                  ; A = prob (cheaper + shorter to re-read again from memory)
    sub     e                       ; A = 16 - (bit<<4) + prob - ((prob + 8)>>4) ; = prob_offset + prob - ((prob + 8)>>4)
    ld      (bc),a                  ; probs[context_index] = prob_offset + prob - ((prob + 8) >> 4);
    add     a,d                     ; restore CF = bit (D = bit ? $FF : $00 && A > 0)
    pop     de
    ret

/*
int upkr_decode_length(int context_index) {
    int length = 0;
    int bit_pos = 0;
    while(upkr_decode_bit(context_index)) {
        length |= upkr_decode_bit(context_index + 1) << bit_pos++;
        context_index += 2;
    }
    return length | (1 << bit_pos);
}
*/
decode_number:
  ; HL = upkr_state
  ; IX = upkr_data_ptr
  ; BC = probs+context_index-1
  ; A' = upkr_current_byte (!!! init to 0x80 at start, not 0x00)
  ; return length in DE, CF=0
    ld      de,$FFFF            ; length = 0 with positional-stop-bit
    or      a                   ; CF=0 to skip getting data bit and use only `rr d : rr e` to fix init DE
.loop:
    call    c,inc_c_decode_bit  ; get data bit, context_index + 1 / if CF=0 just add stop bit into DE init
    rr      d
    rr      e                   ; DE = length = (length >> 1) | (bit << 15);
    call    inc_c_decode_bit    ; context_index += 2
    jr      c,.loop
.fix_bit_pos:
    ccf                         ; NC will become this final `| (1 << bit_pos)` bit
    rr      d
    rr      e
    jr      c,.fix_bit_pos      ; until stop bit is reached (all bits did land to correct position)
    ret                         ; return with CF=0 (important for unpack routine)

    print "upkr.unpack total size: ",$-unpack

    ; reserve space for probs array without emitting any machine code (using only EQU)

    IFDEF UPKR_PROBS_ORIGIN     ; if specific address is defined by user, move probs array there
probs=((UPKR_PROBS_ORIGIN)+255)&0xFF00  ; probs array aligned to 256
    ELSE
probs=($+255)&0xFF00                    ; probs array aligned to 256, after unpacker code
    ENDIF
realc= 1 + 255 + 1 + 2*NUMBER_BITS
if realc&1 : realc+=1 : endif
probs_c=realc            ; padded SIZE of probs array (reference: probs.c), not an address
probs_e=probs+probs_c    ; end address of probs array (reference: probs.e)

    print "upkr.unpack probs array placed at: ",{hex}probs,",\tsize: ",{hex}probs_c

/*
 archived: negligibly faster but +6B longer decode_number variant using HL' and BC' to
 do `number|=(1<<bit_pos);` type of logic in single loop.
*/
; decode_number:
;     exx
;     ld      bc,1
;     ld      l,b
;     ld      h,b                 ; HL = 0
; .loop
;     exx
;     inc     c
;     call    decode_bit
;     jr      nc,.done
;     inc     c
;     call    decode_bit
;     exx
;     jr      nc,.b0
;     add     hl,bc
; .b0:
;     sla     c
;     rl      b
;     jr      .loop
; .done:
;     exx
;     add     hl,bc
;     push    hl
;     exx
;     pop     de
;     ret

/*
 archived: possible LUT variant of updating probs value, requires 512-aligned 512B table (not tested)
*/
; code is replacing decode_bit from "; *** adjust probs[context_index]", followed by `ld (bc),a : add a,d ...`
;     ld      c,a
;     ld      a,high(probs_update_table)/2    ; must be 512 aligned
;     rla
;     ld      b,a
;     ld      a,(bc)
;     pop     bc
; -------------------------------------------
; probs_update_table: EQU probs-512
; -------------------------------------------
; table generator is not obvious and probably not short either, 20+ bytes almost for sure, maybe even 30-40

