/*

Warning! This is a modified version of original sources!

To sum up:
- all include files and C sources were merged in a single file
- existing logs were removed (except error logs)
- main were removed and wrapper added


 * (c) Copyright 2012 by Einar Saukas. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of its author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define ZX7_MAX_OFFSET  2176  /* range 1..2176 */
#define ZX7_MAX_LEN    65536  /* range 2..65536 */

typedef struct match_t {
    size_t index;
    struct match_t *next;
} Match;

typedef struct optimal_t {
    size_t bits;
    int offset;
    int len;
} Optimal;

Optimal *zx7_optimize(unsigned char *input_data, int input_size);

unsigned char *ZX7_compress(Optimal *optimal, unsigned char *input_data, int input_size, int *output_size);

/*
 * (c) Copyright 2012 by Einar Saukas. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of its author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

int zx7_elias_gamma_bits(int value) {
    int bits;

    bits = 1;
    while (value > 1) {
        bits += 2;
        value >>= 1;
    }
    return bits;
}

int count_bits(int offset, int len) {
    return 1 + (offset > 128 ? 12 : 8) + zx7_elias_gamma_bits(len-1);
}

Optimal* zx7_optimize(unsigned char *input_data, int input_size) {
    size_t *min;
    size_t *max;
    Match *matches;
    Match *match_slots;
    Optimal *optimal;
    Match *match;
    int match_index;
    int offset;
    size_t len;
    size_t best_len;
    size_t bits;
    size_t i;

    /* allocate all data structures at once */
    min = (size_t *)calloc(ZX7_MAX_OFFSET+1, sizeof(size_t));
    max = (size_t *)calloc(ZX7_MAX_OFFSET+1, sizeof(size_t));
    matches = (Match *)calloc(256*256, sizeof(Match));
    match_slots = (Match *)calloc(input_size, sizeof(Match));
    optimal = (Optimal *)calloc(input_size, sizeof(Optimal));

    if (!min || !max || !matches || !match_slots || !optimal) {
         fprintf(stderr, "Error: Insufficient memory\n");
         exit(1);
    }

    /* first byte is always literal */
    optimal[0].bits = 8;

    /* process remaining bytes */
    for (i = 1; i < input_size; i++) {

        optimal[i].bits = optimal[i-1].bits + 9;
        match_index = input_data[i-1] << 8 | input_data[i];
        best_len = 1;
        for (match = &matches[match_index]; match->next != NULL && best_len < ZX7_MAX_LEN; match = match->next) {
            offset = i - match->next->index;
            if (offset > ZX7_MAX_OFFSET) {
                match->next = NULL;
                break;
            }

            for (len = 2; len <= ZX7_MAX_LEN; len++) {
                if (len > best_len) {
                    best_len = len;
                    bits = optimal[i-len].bits + count_bits(offset, len);
                    if (optimal[i].bits > bits) {
                        optimal[i].bits = bits;
                        optimal[i].offset = offset;
                        optimal[i].len = len;
                    }
                } else if (i+1 == max[offset]+len && max[offset] != 0) {
                    len = i-min[offset];
                    if (len > best_len) {
                        len = best_len;
                    }
                }
                if (i < offset+len || input_data[i-len] != input_data[i-len-offset]) {
                    break;
                }
            }
            min[offset] = i+1-len;
            max[offset] = i;
        }
        match_slots[i].index = i;
        match_slots[i].next = matches[match_index].next;
        matches[match_index].next = &match_slots[i];
    }

    /* save time by releasing the largest block only, the O.S. will clean everything else later */
    free(match_slots);

    return optimal;
}

/*
 * (c) Copyright 2012 by Einar Saukas. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of its author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//extern unsigned char* output_data;
//extern int output_index;
//extern int bit_index;
//extern int bit_mask;

// previously on ZX0-main
unsigned char* output_data;
int output_index;
int input_index;
int bit_index;
int bit_mask;
int diff;
int backtrack;


void zx7_write_byte(int value) {
    output_data[output_index++] = value;
}

void zx7_write_bit(int value) {
    if (bit_mask == 0) {
        bit_mask = 128;
        bit_index = output_index;
        zx7_write_byte(0);
    }
    if (value > 0) {
        output_data[bit_index] |= bit_mask;
    }
    bit_mask >>= 1;
}

void zx7_write_elias_gamma(int value) {
    int i;

    for (i = 2; i <= value; i <<= 1) {
        zx7_write_bit(0);
    }
    while ((i >>= 1) > 0) {
        zx7_write_bit(value & i);
    }
}

unsigned char *ZX7_compress(Optimal *optimal, unsigned char *input_data, int input_size, int *output_size) {
    size_t input_index;
    size_t input_prev;
    int offset1;
    int mask;
    int i;

    /* calculate and allocate output buffer */
    input_index = input_size-1;
    *output_size = (optimal[input_index].bits+18+7)/8;
    output_data = (unsigned char *)MemMalloc(*output_size);

    /* un-reverse optimal sequence */
    optimal[input_index].bits = 0;
    while (input_index > 0) {
        input_prev = input_index - (optimal[input_index].len > 0 ? optimal[input_index].len : 1);
        optimal[input_prev].bits = input_index;
        input_index = input_prev;
    }

    output_index = 0;
    bit_mask = 0;

    /* first byte is always literal */
    zx7_write_byte(input_data[0]);

    /* process remaining bytes */
    while ((input_index = optimal[input_index].bits) > 0) {
        if (optimal[input_index].len == 0) {

            /* literal indicator */
            zx7_write_bit(0);

            /* literal value */
            zx7_write_byte(input_data[input_index]);

        } else {

            /* sequence indicator */
            zx7_write_bit(1);

            /* sequence length */
            zx7_write_elias_gamma(optimal[input_index].len-1);

            /* sequence offset */
            offset1 = optimal[input_index].offset-1;
            if (offset1 < 128) {
                zx7_write_byte(offset1);
            } else {
                offset1 -= 128;
                zx7_write_byte((offset1 & 127) | 128);
                for (mask = 1024; mask > 127; mask >>= 1) {
                    zx7_write_bit(offset1 & mask);
                }
            }
        }
    }

    /* sequence indicator */
    zx7_write_bit(1);

    /* end marker > MAX_LEN */
    for (i = 0; i < 16; i++) {
        zx7_write_bit(0);
    }
    zx7_write_bit(1);

    return output_data;
}
