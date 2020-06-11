/*
 * apultra.c - command line compression utility for the apultra library
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 ********************************************************************************************
 ********************************************************************************************
 ********************************************************************************************

 T H I S   I S   A N   A L T E R E D   V E R S I O N   O F   A P U L T R A   S O U R C E S

 ********************************************************************************************
 ********************************************************************************************
 ********************************************************************************************
 *
 *
 *
 *
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif






/*
 * divsufsort_private.h for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DIVSUFSORT_PRIVATE_H
#define _DIVSUFSORT_PRIVATE_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STDINT_H 1
#define INLINE inline

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#endif /* _MSC_VER */



#include <assert.h>
#include <stdio.h>
#if HAVE_STRING_H
# include <string.h>
#endif
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#if HAVE_MEMORY_H
# include <memory.h>
#endif
#if HAVE_STDDEF_H
# include <stddef.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif






/*
 * divsufsort.h for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DIVSUFSORT_H
#define _DIVSUFSORT_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DIVSUFSORT_API

/*- Datatypes -*/
#ifndef SAUCHAR_T
#define SAUCHAR_T
typedef unsigned char sauchar_t;
#endif /* SAUCHAR_T */
#ifndef SAINT_T
#define SAINT_T
typedef int saint_t;
#endif /* SAINT_T */
#ifndef SAIDX_T
#define SAIDX_T
typedef int saidx_t;
#endif /* SAIDX_T */
#ifndef PRIdSAIDX_T
#define PRIdSAIDX_T "d"
#endif

/*- divsufsort context */
typedef struct _divsufsort_ctx_t {
   saidx_t *bucket_A;
   saidx_t *bucket_B;
} divsufsort_ctx_t;

/*- Prototypes -*/

/**
 * Initialize suffix array context
 *
 * @return 0 for success, or non-zero in case of an error
 */
int divsufsort_init(divsufsort_ctx_t *ctx);

/**
 * Destroy suffix array context
 *
 * @param ctx suffix array context to destroy
 */
void divsufsort_destroy(divsufsort_ctx_t *ctx);

/**
 * Constructs the suffix array of a given string.
 * @param ctx suffix array context
 * @param T[0..n-1] The input string.
 * @param SA[0..n-1] The output array of suffixes.
 * @param n The length of the given string.
 * @return 0 if no error occurred, -1 or -2 otherwise.
 */
DIVSUFSORT_API
saint_t divsufsort_build_array(divsufsort_ctx_t *ctx, const sauchar_t *T, saidx_t *SA, saidx_t n);

#if 0
/**
 * Constructs the burrows-wheeler transformed string of a given string.
 * @param T[0..n-1] The input string.
 * @param U[0..n-1] The output string. (can be T)
 * @param A[0..n-1] The temporary array. (can be NULL)
 * @param n The length of the given string.
 * @return The primary index if no error occurred, -1 or -2 otherwise.
 */
DIVSUFSORT_API
saidx_t
divbwt(const sauchar_t *T, sauchar_t *U, saidx_t *A, saidx_t n);

/**
 * Returns the version of the divsufsort library.
 * @return The version number string.
 */
DIVSUFSORT_API
const char *
divsufsort_version(void);


/**
 * Constructs the burrows-wheeler transformed string of a given string and suffix array.
 * @param T[0..n-1] The input string.
 * @param U[0..n-1] The output string. (can be T)
 * @param SA[0..n-1] The suffix array. (can be NULL)
 * @param n The length of the given string.
 * @param idx The output primary index.
 * @return 0 if no error occurred, -1 or -2 otherwise.
 */
DIVSUFSORT_API
saint_t
bw_transform(const sauchar_t *T, sauchar_t *U,
             saidx_t *SA /* can NULL */,
             saidx_t n, saidx_t *idx);

/**
 * Inverse BW-transforms a given BWTed string.
 * @param T[0..n-1] The input string.
 * @param U[0..n-1] The output string. (can be T)
 * @param A[0..n-1] The temporary array. (can be NULL)
 * @param n The length of the given string.
 * @param idx The primary index.
 * @return 0 if no error occurred, -1 or -2 otherwise.
 */
DIVSUFSORT_API
saint_t
inverse_bw_transform(const sauchar_t *T, sauchar_t *U,
                     saidx_t *A /* can NULL */,
                     saidx_t n, saidx_t idx);

/**
 * Checks the correctness of a given suffix array.
 * @param T[0..n-1] The input string.
 * @param SA[0..n-1] The input suffix array.
 * @param n The length of the given string.
 * @param verbose The verbose mode.
 * @return 0 if no error occurred.
 */
DIVSUFSORT_API
saint_t
sufcheck(const sauchar_t *T, const saidx_t *SA, saidx_t n, saint_t verbose);

/**
 * Search for the pattern P in the string T.
 * @param T[0..Tsize-1] The input string.
 * @param Tsize The length of the given string.
 * @param P[0..Psize-1] The input pattern string.
 * @param Psize The length of the given pattern string.
 * @param SA[0..SAsize-1] The input suffix array.
 * @param SAsize The length of the given suffix array.
 * @param idx The output index.
 * @return The count of matches if no error occurred, -1 otherwise.
 */
DIVSUFSORT_API
saidx_t
sa_search(const sauchar_t *T, saidx_t Tsize,
          const sauchar_t *P, saidx_t Psize,
          const saidx_t *SA, saidx_t SAsize,
          saidx_t *left);

/**
 * Search for the character c in the string T.
 * @param T[0..Tsize-1] The input string.
 * @param Tsize The length of the given string.
 * @param SA[0..SAsize-1] The input suffix array.
 * @param SAsize The length of the given suffix array.
 * @param c The input character.
 * @param idx The output index.
 * @return The count of matches if no error occurred, -1 otherwise.
 */
DIVSUFSORT_API
saidx_t
sa_simplesearch(const sauchar_t *T, saidx_t Tsize,
                const saidx_t *SA, saidx_t SAsize,
                saint_t c, saidx_t *left);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _DIVSUFSORT_H */



















/*- Constants -*/
#if !defined(UINT8_MAX)
# define UINT8_MAX (255)
#endif /* UINT8_MAX */
#if defined(ALPHABET_SIZE) && (ALPHABET_SIZE < 1)
# undef ALPHABET_SIZE
#endif
#if !defined(ALPHABET_SIZE)
# define ALPHABET_SIZE (UINT8_MAX + 1)
#endif
/* for divsufsort.c */
#define BUCKET_A_SIZE (ALPHABET_SIZE)
#define BUCKET_B_SIZE (ALPHABET_SIZE * ALPHABET_SIZE)
/* for sssort.c */
#if defined(SS_INSERTIONSORT_THRESHOLD)
# if SS_INSERTIONSORT_THRESHOLD < 1
#  undef SS_INSERTIONSORT_THRESHOLD
#  define SS_INSERTIONSORT_THRESHOLD (1)
# endif
#else
# define SS_INSERTIONSORT_THRESHOLD (8)
#endif
#if defined(SS_BLOCKSIZE)
# if SS_BLOCKSIZE < 0
#  undef SS_BLOCKSIZE
#  define SS_BLOCKSIZE (0)
# elif 32768 <= SS_BLOCKSIZE
#  undef SS_BLOCKSIZE
#  define SS_BLOCKSIZE (32767)
# endif
#else
# define SS_BLOCKSIZE (1024)
#endif
/* minstacksize = log(SS_BLOCKSIZE) / log(3) * 2 */
#if SS_BLOCKSIZE == 0
# if defined(BUILD_DIVSUFSORT64)
#  define SS_MISORT_STACKSIZE (96)
# else
#  define SS_MISORT_STACKSIZE (64)
# endif
#elif SS_BLOCKSIZE <= 4096
# define SS_MISORT_STACKSIZE (16)
#else
# define SS_MISORT_STACKSIZE (24)
#endif
#if defined(BUILD_DIVSUFSORT64)
# define SS_SMERGE_STACKSIZE (64)
#else
# define SS_SMERGE_STACKSIZE (32)
#endif
/* for trsort.c */
#define TR_INSERTIONSORT_THRESHOLD (8)
#if defined(BUILD_DIVSUFSORT64)
# define TR_STACKSIZE (96)
#else
# define TR_STACKSIZE (64)
#endif


/*- Macros -*/
#ifndef SWAP
# define SWAP(_a, _b) do { t = (_a); (_a) = (_b); (_b) = t; } while(0)
#endif /* SWAP */
#ifndef MIN
# define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
#endif /* MIN */
#ifndef MAX
# define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
#endif /* MAX */
#define STACK_PUSH(_a, _b, _c, _d)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize++].d = (_d);\
  } while(0)
#define STACK_PUSH5(_a, _b, _c, _d, _e)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize].d = (_d), stack[ssize++].e = (_e);\
  } while(0)
#define STACK_POP(_a, _b, _c, _d)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d;\
  } while(0)
#define STACK_POP5(_a, _b, _c, _d, _e)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d, (_e) = stack[ssize].e;\
  } while(0)
/* for divsufsort.c */
#define BUCKET_A(_c0) bucket_A[(_c0)]
#if ALPHABET_SIZE == 256
#define BUCKET_B(_c0, _c1) (bucket_B[((_c1) << 8) | (_c0)])
#define BUCKET_BSTAR(_c0, _c1) (bucket_B[((_c0) << 8) | (_c1)])
#else
#define BUCKET_B(_c0, _c1) (bucket_B[(_c1) * ALPHABET_SIZE + (_c0)])
#define BUCKET_BSTAR(_c0, _c1) (bucket_B[(_c0) * ALPHABET_SIZE + (_c1)])
#endif


/*- Private Prototypes -*/
/* sssort.c */
void
sssort(const sauchar_t *Td, const saidx_t *PA,
       saidx_t *first, saidx_t *last,
       saidx_t *buf, saidx_t bufsize,
       saidx_t depth, saidx_t n, saint_t lastsuffix);
/* trsort.c */
void
trsort(saidx_t *ISA, saidx_t *SA, saidx_t n, saidx_t depth);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _DIVSUFSORT_PRIVATE_H */



/*
 * matchfinder.h - LZ match finder definitions
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#ifndef _MATCHFINDER_H
#define _MATCHFINDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct _apultra_match apultra_match;
typedef struct _apultra_compressor apultra_compressor;

/**
 * Parse input data, build suffix array and overlaid data structures to speed up match finding
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nInWindowSize total input size in bytes (previously compressed bytes + bytes to compress)
 *
 * @return 0 for success, non-zero for failure
 */
int apultra_build_suffix_array(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nInWindowSize);

/**
 * Find matches at the specified offset in the input window
 *
 * @param pCompressor compression context
 * @param nOffset offset to find matches at, in the input window
 * @param pMatches pointer to returned matches
 * @param pMatchDepth pointer to returned match depths
 * @param pMatch1 pointer to 1-byte length, 4 bit offset match
 * @param nMaxMatches maximum number of matches to return (0 for none)
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 *
 * @return number of matches
 */
int apultra_find_matches_at(apultra_compressor *pCompressor, const int nOffset, apultra_match *pMatches, unsigned short *pMatchDepth, unsigned char *pMatch1, const int nMaxMatches, const int nBlockFlags);

/**
 * Skip previously compressed bytes
 *
 * @param pCompressor compression context
 * @param nStartOffset current offset in input window (typically 0)
 * @param nEndOffset offset to skip to in input window (typically the number of previously compressed bytes)
 */
void apultra_skip_matches(apultra_compressor *pCompressor, const int nStartOffset, const int nEndOffset);

/**
 * Find all matches for the data to be compressed
 *
 * @param pCompressor compression context
 * @param nMatchesPerOffset maximum number of matches to store for each offset
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 */
void apultra_find_all_matches(apultra_compressor *pCompressor, const int nMatchesPerOffset, const int nStartOffset, const int nEndOffset, const int nBlockFlags);

#ifdef __cplusplus
}
#endif

#endif /* _MATCHFINDER_H */
/*
 * format.h - byte stream format definitions
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#ifndef _FORMAT_H
#define _FORMAT_H

#define MIN_OFFSET 1
#define MAX_OFFSET 0x1fffff

#define MAX_VARLEN 0x1fffff

#define BLOCK_SIZE 0x100000

#define MIN_MATCH_SIZE 1
#define MINMATCH3_OFFSET 1280
#define MINMATCH4_OFFSET 32000

#endif /* _FORMAT_H */
/*
 * shrink.h - compressor definitions
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#ifndef _SHRINK_H
#define _SHRINK_H


#ifdef __cplusplus
extern "C" {
#endif

#define LCP_BITS 15
#define TAG_BITS 4
#define LCP_MAX ((1U<<(LCP_BITS - TAG_BITS)) - 1)
#define LCP_AND_TAG_MAX ((1U<<LCP_BITS) - 1)
#define LCP_SHIFT (63-LCP_BITS)
#define LCP_MASK (((1ULL<<LCP_BITS) - 1) << LCP_SHIFT)
#define POS_MASK ((1ULL<<LCP_SHIFT) - 1)
#define VISITED_FLAG 0x8000000000000000ULL
#define EXCL_VISITED_MASK  0x7fffffffffffffffULL

#define NMATCHES_PER_ARRIVAL 32
#define NMATCHES_PER_ARRIVAL_SMALL 9

#define NMATCHES_PER_INDEX 64
#define MATCHES_PER_INDEX_SHIFT 6

#define LEAVE_ALONE_MATCH_SIZE 120

/** One match option */
typedef struct _apultra_match {
   unsigned int length:11;
   unsigned int offset:21;
} apultra_match;

/** One finalized match */
typedef struct _apultra_final_match {
   int length;
   int offset;
} apultra_final_match;

/** Forward arrival slot */
typedef struct {
   int cost;

   unsigned int from_pos:21;
   int from_slot:7;
   unsigned int follows_literal:1;

   unsigned int rep_offset:21;
   unsigned int short_offset:4;
   unsigned int rep_pos:21;
   unsigned int match_len:11;

   int score;
} apultra_arrival;

/** Compression statistics */
typedef struct _apultra_stats {
   int num_literals;
   int num_4bit_matches;
   int num_7bit_matches;
   int num_variable_matches;
   int num_rep_matches;

   int safe_dist;

   int min_offset;
   int max_offset;
   long long total_offsets;

   int min_match_len;
   int max_match_len;
   int total_match_lens;

   int min_rle1_len;
   int max_rle1_len;
   int total_rle1_lens;

   int min_rle2_len;
   int max_rle2_len;
   int total_rle2_lens;

   int commands_divisor;
   int match_divisor;
   int rle1_divisor;
   int rle2_divisor;
} apultra_stats;

/** Compression context */
typedef struct _apultra_compressor {
   divsufsort_ctx_t divsufsort_context;
   unsigned long long *intervals;
   unsigned long long *pos_data;
   unsigned long long *open_intervals;
   apultra_match *match;
   unsigned short *match_depth;
   unsigned char *match1;
   apultra_final_match *best_match;
   apultra_arrival *arrival;
   int *first_offset_for_byte;
   int *next_offset_for_pos;
   int flags;
   int block_size;
   apultra_stats stats;
} apultra_compressor;

/**
 * Get maximum compressed size of input(source) data
 *
 * @param nInputSize input(source) size in bytes
 *
 * @return maximum compressed size
 */
size_t apultra_get_max_compressed_size(size_t nInputSize);

/**
 * Compress memory
 *
 * @param pInputData pointer to input(source) data to compress
 * @param pOutBuffer buffer for compressed data
 * @param nInputSize input(source) size in bytes
 * @param nMaxOutBufferSize maximum capacity of compression buffer
 * @param nFlags compression flags (set to 0)
 * @param progress progress function, called after compressing each block, or NULL for none
 * @param pStats pointer to compression stats that are filled if this function is successful, or NULL
 * @param nMaxWindowSize maximum window size to use (0 for default)
 *
 * @return actual compressed size, or -1 for error
 */
size_t apultra_compress(const unsigned char *pInputData, unsigned char *pOutBuffer, size_t nInputSize, size_t nMaxOutBufferSize,
   const unsigned int nFlags, size_t nMaxWindowSize, void(*progress)(long long nOriginalSize, long long nCompressedSize), apultra_stats *pStats);

#ifdef __cplusplus
}
#endif

#endif /* _SHRINK_H */
/*
 * expand.h - decompressor definitions
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#ifndef _EXPAND_H
#define _EXPAND_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get maximum decompressed size of compressed data
 *
 * @param pInputData compressed data
 * @param nInputSize compressed size in bytes
 * @param nFlags compression flags (set to 0)
 *
 * @return maximum decompressed size
 */
size_t apultra_get_max_decompressed_size(const unsigned char *pInputData, size_t nInputSize, const unsigned int nFlags);

/**
 * Decompress data in memory
 *
 * @param pInputData compressed data
 * @param pOutBuffer buffer for decompressed data
 * @param nInputSize compressed size in bytes
 * @param nMaxOutBufferSize maximum capacity of decompression buffer
 * @param nFlags compression flags (set to 0)
 *
 * @return actual decompressed size, or -1 for error
 */
size_t apultra_decompress(const unsigned char *pInputData, unsigned char *pOutBuffer, size_t nInputSize, size_t nMaxOutBufferSize, const unsigned int nFlags);

#ifdef __cplusplus
}
#endif

#endif /* _EXPAND_H */
/*
 * apultra.c - command line compression utility for the apultra library
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 ********************************************************************************************
 ********************************************************************************************
 ********************************************************************************************

 T H I S   I S   A N   A L T E R E D   V E R S I O N   O F   A P U L T R A   S O U R C E S

 ********************************************************************************************
 ********************************************************************************************
 ********************************************************************************************
 *
 *
 *
 *
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif


















#define OPT_VERBOSE        1
#define OPT_STATS          2

#define TOOL_VERSION "1.2.0"

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
LARGE_INTEGER hpc_frequency;
BOOL hpc_available = FALSE;
#endif


/*---------------------------------------------------------------------------*/

static void compression_progress(long long nOriginalSize, long long nCompressedSize) {
   if (nOriginalSize >= 512 * 1024) {
      fprintf(stdout, "\r%lld => %lld (%g %%)     \b\b\b\b\b", nOriginalSize, nCompressedSize, (double)(nCompressedSize * 100.0 / nOriginalSize));
      fflush(stdout);
   }
}

static int do_apultra(unsigned char *datain, int lenin, unsigned char **dataout, int *lenout) {
   long long nStartTime = 0LL, nEndTime = 0LL;
   size_t nOriginalSize = 0L, nCompressedSize = 0L, nMaxCompressedSize;
   int nFlags = 0;
   apultra_stats stats;
   unsigned char *pCompressedData;

   /* Allocate max compressed size */

   nMaxCompressedSize = apultra_get_max_compressed_size(lenin);
   pCompressedData = (unsigned char*)MemMalloc(nMaxCompressedSize);
   memset(pCompressedData, 0, nMaxCompressedSize);

   nCompressedSize = apultra_compress(datain, pCompressedData, lenin, nMaxCompressedSize, nFlags, 65536, compression_progress, &stats);

   if (nCompressedSize == -1) {
      fprintf(stderr, "APULTRA compression error\n");
      *lenout=0;
      *dataout=NULL;
      return 100;
   }

   *lenout=nCompressedSize;
   *dataout=pCompressedData;

   return 0;
}

/*---------------------------------------------------------------------------*/

static int do_decompress(const char *pszInFilename, const char *pszOutFilename, const char *pszDictionaryFilename, const unsigned int nOptions) {
   long long nStartTime = 0LL, nEndTime = 0LL;
   size_t nCompressedSize, nMaxDecompressedSize, nOriginalSize;
   unsigned char *pCompressedData;
   unsigned char *pDecompressedData;
   int nFlags = 0;

   /* Read the whole compressed file in memory */

   FILE *f_in = fopen(pszInFilename, "rb");
   if (!f_in) {
      fprintf(stderr, "error opening '%s' for reading\n", pszInFilename);
      return 100;
   }

   fseek(f_in, 0, SEEK_END);
   nCompressedSize = (size_t)ftell(f_in);
   fseek(f_in, 0, SEEK_SET);

   pCompressedData = (unsigned char*)malloc(nCompressedSize);
   if (!pCompressedData) {
      fclose(f_in);
      fprintf(stderr, "out of memory for reading '%s', %zd bytes needed\n", pszInFilename, nCompressedSize);
      return 100;
   }

   if (fread(pCompressedData, 1, nCompressedSize, f_in) != nCompressedSize) {
      free(pCompressedData);
      fclose(f_in);
      fprintf(stderr, "I/O error while reading '%s'\n", pszInFilename);
      return 100;
   }

   fclose(f_in);

   /* Allocate max decompressed size */

   nMaxDecompressedSize = apultra_get_max_decompressed_size(pCompressedData, nCompressedSize, nFlags);
   if (nMaxDecompressedSize == -1) {
      free(pCompressedData);
      fprintf(stderr, "invalid compressed format for file '%s'\n", pszInFilename);
      return 100;
   }

   pDecompressedData = (unsigned char*)malloc(nMaxDecompressedSize);
   if (!pDecompressedData) {
      free(pCompressedData);
      fprintf(stderr, "out of memory for decompressing '%s', %zd bytes needed\n", pszInFilename, nMaxDecompressedSize);
      return 100;
   }

   memset(pDecompressedData, 0, nMaxDecompressedSize);


   nOriginalSize = apultra_decompress(pCompressedData, pDecompressedData, nCompressedSize, nMaxDecompressedSize, nFlags);
   if (nOriginalSize == -1) {
      free(pDecompressedData);
      free(pCompressedData);

      fprintf(stderr, "decompression error for '%s'\n", pszInFilename);
      return 100;
   }

   if (pszOutFilename) {
      FILE *f_out;

      /* Write whole decompressed file out */

      f_out = fopen(pszOutFilename, "wb");
      if (f_out) {
         fwrite(pDecompressedData, 1, nOriginalSize, f_out);
         fclose(f_out);
      }
   }

   free(pDecompressedData);
   free(pCompressedData);

   return 0;
}


/*---------------------------------------------------------------------------*/

static void generate_compressible_data(unsigned char *pBuffer, size_t nBufferSize, unsigned int nSeed, int nNumLiteralValues, float fMatchProbability) {
   size_t nIndex = 0;
   int nMatchProbability = (int)(fMatchProbability * 1023.0f);

   srand(nSeed);
   
   if (nIndex >= nBufferSize) return;
   pBuffer[nIndex++] = rand() % nNumLiteralValues;

   while (nIndex < nBufferSize) {
      if ((rand() & 1023) >= nMatchProbability) {
         size_t nLiteralCount = rand() & 127;
         if (nLiteralCount > (nBufferSize - nIndex))
            nLiteralCount = nBufferSize - nIndex;

         while (nLiteralCount--)
            pBuffer[nIndex++] = rand() % nNumLiteralValues;
      }
      else {
         size_t nMatchLength = MIN_MATCH_SIZE + (rand() & 1023);
         size_t nMatchOffset;

         if (nMatchLength > (nBufferSize - nIndex))
            nMatchLength = nBufferSize - nIndex;
         if (nMatchLength > nIndex)
            nMatchLength = nIndex;

         if (nMatchLength < nIndex)
            nMatchOffset = rand() % (nIndex - nMatchLength);
         else
            nMatchOffset = 0;

         while (nMatchLength--) {
            pBuffer[nIndex] = pBuffer[nIndex - nMatchOffset];
            nIndex++;
         }
      }
   }
}

static void xor_data(unsigned char *pBuffer, size_t nBufferSize, unsigned int nSeed, float fXorProbability) {
   size_t nIndex = 0;
   int nXorProbability = (int)(fXorProbability * 1023.0f);

   srand(nSeed);

   if (nIndex >= nBufferSize) return;

   while (nIndex < nBufferSize) {
      if ((rand() & 1023) < nXorProbability) {
         pBuffer[nIndex] ^= 0xff;
      }
      nIndex++;
   }
}

int APULTRA_crunch(unsigned char *data, int len, unsigned char **dataout, int *lenout) {
   int i;
   const char *pszInFilename = NULL;
   const char *pszOutFilename = NULL;
   const char *pszDictionaryFilename = NULL;
   bool bArgsError = false;
   bool bCommandDefined = false;
   bool bVerifyCompression = false;
   bool bFormatVersionDefined = false;
   unsigned int nOptions = 0;
   unsigned int nMaxWindowSize = 0;

   return do_apultra(data, len, dataout, lenout);
}
/*
 * expand.c - decompressor implementation
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#include <stdlib.h>
#include <string.h>


 FORCE_INLINE int apultra_read_bit(const unsigned char **ppInBlock, const unsigned char *pDataEnd, int *nCurBitMask, unsigned char *bits) {
   const unsigned char *pInBlock = *ppInBlock;
   int nBit;

   if ((*nCurBitMask) == 0) {
      if (pInBlock >= pDataEnd) return -1;
      (*bits) = *pInBlock++;
      (*nCurBitMask) = 128;
   }

   nBit = ((*bits) & 128) ? 1 : 0;

   (*bits) <<= 1;
   (*nCurBitMask) >>= 1;

   *ppInBlock = pInBlock;
   return nBit;
}

 FORCE_INLINE int apultra_read_gamma2(const unsigned char **ppInBlock, const unsigned char *pDataEnd, int *nCurBitMask, unsigned char *bits) {
   int bit;
   unsigned int v = 1;

   do {
      v = (v << 1) + apultra_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
      bit = apultra_read_bit(ppInBlock, pDataEnd, nCurBitMask, bits);
      if (bit < 0) return bit;
   } while (bit);

   return v;
}

/**
 * Get maximum decompressed size of compressed data
 *
 * @param pInputData compressed data
 * @param nInputSize compressed size in bytes
 * @param nFlags compression flags (set to 0)
 *
 * @return maximum decompressed size
 */
size_t apultra_get_max_decompressed_size(const unsigned char *pInputData, size_t nInputSize, const unsigned int nFlags) {
   const unsigned char *pInputDataEnd = pInputData + nInputSize;
   int nCurBitMask = 0;
   unsigned char bits = 0;
   int nMatchOffset = 1;
   int nFollowsLiteral = 2;
   size_t nDecompressedSize = 0;

   if (pInputData >= pInputDataEnd)
      return -1;
   pInputData++;
   nDecompressedSize++;

   while (1) {
      unsigned int nResult;

      nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
      if (nResult < 0) return -1;

      if (!nResult) {
         /* '0': literal */
         if (pInputData < pInputDataEnd) {
            pInputData++;
            nDecompressedSize++;
            nFollowsLiteral = 2;
         }
         else {
            return -1;
         }
      }
      else {
         nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nResult < 0) return -1;

         if (nResult == 0) {
            unsigned int nMatchLen;

            /* '10': 8+n bits offset */
            unsigned int nMatchOffsetHi = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            nMatchOffsetHi -= nFollowsLiteral;
            if (nMatchOffsetHi) {
               nMatchOffset = (nMatchOffsetHi - 1) << 8;
               nMatchOffset |= (unsigned int)(*pInputData++);

               nMatchLen = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);

               if (nMatchOffset < 128 || nMatchOffset >= MINMATCH4_OFFSET)
                  nMatchLen += 2;
               else if (nMatchOffset >= MINMATCH3_OFFSET)
                  nMatchLen++;
            }
            else {
               /* else rep-match */
               nMatchLen = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            }

            nFollowsLiteral = 1;

            nDecompressedSize += nMatchLen;
         }
         else {
            nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            if (nResult < 0) return -1;

            if (nResult == 0) {
               unsigned int nCommand;
               unsigned int nMatchLen;

               /* '110': 7 bits offset + 1 bit length */
               nCommand = (unsigned int)(*pInputData++);
               if (nCommand == 0x00) {
                  /* EOD. No match len follows. */
                  break;
               }

               /* Bits 7-1: offset; bit 0: length */
               nMatchOffset = (nCommand >> 1);
               nMatchLen = (nCommand & 1) + 2;

               nFollowsLiteral = 1;
               nDecompressedSize += nMatchLen;
            }
            else {
               unsigned int nShortMatchOffset;

               /* '111': 4 bit offset */
               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset = nResult << 3;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 2;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 1;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 0;

               nFollowsLiteral = 2;
               nDecompressedSize++;
            }
         }
      }
   }

   return nDecompressedSize;
}

/**
 * Decompress data in memory
 *
 * @param pInputData compressed data
 * @param pOutBuffer buffer for decompressed data
 * @param nInputSize compressed size in bytes
 * @param nMaxOutBufferSize maximum capacity of decompression buffer
 * @param nFlags compression flags (set to 0)
 *
 * @return actual decompressed size, or -1 for error
 */
size_t apultra_decompress(const unsigned char *pInputData, unsigned char *pOutData, size_t nInputSize, size_t nMaxOutBufferSize, const unsigned int nFlags) {
   const unsigned char *pInputDataEnd = pInputData + nInputSize;
   unsigned char *pCurOutData = pOutData;
   const unsigned char *pOutDataEnd = pCurOutData + nMaxOutBufferSize;
   const unsigned char *pOutDataFastEnd = pOutDataEnd - 20;
   int nCurBitMask = 0;
   unsigned char bits = 0;
   int nMatchOffset = 1;
   int nFollowsLiteral = 2;

   if (pInputData >= pInputDataEnd && pCurOutData < pOutDataEnd)
      return -1;
   *pCurOutData++ = *pInputData++;

   while (1) {
      unsigned int nResult;

      nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
      if (nResult < 0) return -1;

      if (!nResult) {
         /* '0': literal */
         if (pInputData < pInputDataEnd && pCurOutData < pOutDataEnd) {
            *pCurOutData++ = *pInputData++;
            nFollowsLiteral = 2;
         }
         else {
            return -1;
         }
      }
      else {
         nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
         if (nResult < 0) return -1;

         if (nResult == 0) {
            const unsigned char *pSrc = pCurOutData - nMatchOffset;
            unsigned int nMatchLen;

            /* '10': 8+n bits offset */
            unsigned int nMatchOffsetHi = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            nMatchOffsetHi -= nFollowsLiteral;
            if (nMatchOffsetHi) {
               nMatchOffset = (nMatchOffsetHi - 1) << 8;
               nMatchOffset |= (unsigned int)(*pInputData++);

               nMatchLen = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);

               if (nMatchOffset < 128 || nMatchOffset >= MINMATCH4_OFFSET)
                  nMatchLen += 2;
               else if (nMatchOffset >= MINMATCH3_OFFSET)
                  nMatchLen++;
            }
            else {
               /* else rep-match */
               nMatchLen = apultra_read_gamma2(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            }

            nFollowsLiteral = 1;
            if (pSrc >= pOutData) {
               if (nMatchLen < 11 && nMatchOffset >= 8 && pCurOutData < pOutDataFastEnd) {
                  memcpy(pCurOutData, pSrc, 8);
                  memcpy(pCurOutData + 8, pSrc + 8, 2);
                  pCurOutData += nMatchLen;
               }
               else {
                  if ((pCurOutData + nMatchLen) <= pOutDataEnd && (pSrc + nMatchLen) <= pOutDataEnd) {
                     /* Do a deterministic, left to right byte copy instead of memcpy() so as to handle overlaps */

                     if (nMatchOffset >= 16 && (pCurOutData + nMatchLen) < (pOutDataFastEnd - 15)) {
                        const unsigned char *pCopySrc = pSrc;
                        unsigned char *pCopyDst = pCurOutData;
                        const unsigned char *pCopyEndDst = pCurOutData + nMatchLen;

                        do {
                           memcpy(pCopyDst, pCopySrc, 16);
                           pCopySrc += 16;
                           pCopyDst += 16;
                        } while (pCopyDst < pCopyEndDst);

                        pCurOutData += nMatchLen;
                     }
                     else {
                        while (nMatchLen) {
                           *pCurOutData++ = *pSrc++;
                           nMatchLen--;
                        }
                     }
                  }
                  else {
                     return -1;
                  }
               }
            }
            else {
               return -1;
            }
         }
         else {
            nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
            if (nResult < 0) return -1;

            if (nResult == 0) {
               unsigned int nCommand;
               unsigned int nMatchLen;
               const unsigned char *pSrc = pCurOutData - nMatchOffset;

               /* '110': 7 bits offset + 1 bit length */
               nCommand = (unsigned int)(*pInputData++);
               if (nCommand == 0x00) {
                  /* EOD. No match len follows. */
                  break;
               }

               /* Bits 7-1: offset; bit 0: length */
               nMatchOffset = (nCommand >> 1);
               nMatchLen = (nCommand & 1) + 2;

               nFollowsLiteral = 1;
               if (pSrc >= pOutData && (pSrc + nMatchLen) <= pOutDataEnd) {
                  if (nMatchOffset >= 8 && pCurOutData < pOutDataFastEnd) {
                     memcpy(pCurOutData, pSrc, 8);
                     memcpy(pCurOutData + 8, pSrc + 8, 2);
                     pCurOutData += nMatchLen;
                  }
                  else {
                     if ((pCurOutData + nMatchLen) <= pOutDataEnd) {
                        while (nMatchLen) {
                           *pCurOutData++ = *pSrc++;
                           nMatchLen--;
                        }
                     }
                     else {
                        return -1;
                     }
                  }
               }
               else {
                  return -1;
               }
            }
            else {
               unsigned int nShortMatchOffset;

               /* '111': 4 bit offset */
               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset = nResult << 3;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 2;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 1;

               nResult = apultra_read_bit(&pInputData, pInputDataEnd, &nCurBitMask, &bits);
               if (nResult < 0) return -1;
               nShortMatchOffset |= nResult << 0;

               nFollowsLiteral = 2;
               if (nShortMatchOffset) {
                  /* Short offset, 1-15 */
                  const unsigned char *pSrc = pCurOutData - nShortMatchOffset;
                  if (pSrc >= pOutData && (pCurOutData + 1) <= pOutDataEnd && (pSrc + 1) <= pOutDataEnd) {
                     *pCurOutData++ = *pSrc++;
                  }
                  else {
                     return -1;
                  }
               }
               else {
                  /* Write zero */
                  if ((pCurOutData + 1) <= pOutDataEnd) {
                     *pCurOutData++ = 0;
                  }
                  else {
                     return -1;
                  }
               }
            }
         }
      }
   }

   return (size_t)(pCurOutData - pOutData);
}
/*
 * matchfinder.c - LZ match finder implementation
 *
 * The following copying information applies to this specific source code file:
 *
 * Written in 2019 by Emmanuel Marty <marty.emmanuel@gmail.com>
 * Portions written in 2014-2015 by Eric Biggers <ebiggers3@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide via the Creative Commons Zero 1.0 Universal Public Domain
 * Dedication (the "CC0").
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the CC0 for more details.
 *
 * You should have received a copy of the CC0 along with this software; if not
 * see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#include <stdlib.h>
#include <string.h>

/**
 * Hash index into TAG_BITS
 *
 * @param nIndex index value
 *
 * @return hash
 */
static inline int apultra_get_index_tag(unsigned int nIndex) {
   return (int)(((unsigned long long)nIndex * 11400714819323198485ULL) >> (64ULL - TAG_BITS));
}

/**
 * Parse input data, build suffix array and overlaid data structures to speed up match finding
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nInWindowSize total input size in bytes (previously compressed bytes + bytes to compress)
 *
 * @return 0 for success, non-zero for failure
 */
int apultra_build_suffix_array(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nInWindowSize) {
   unsigned long long *intervals = pCompressor->intervals;

   /* Build suffix array from input data */
   saidx_t *suffixArray = (saidx_t*)intervals;
   if (divsufsort_build_array(&pCompressor->divsufsort_context, pInWindow, suffixArray, nInWindowSize) != 0) {
      return 100;
   }

   int i, r;

   for (i = nInWindowSize - 1; i >= 0; i--) {
      intervals[i] = suffixArray[i];
   }

   int *PLCP = (int*)pCompressor->pos_data;  /* Use temporarily */
   int *Phi = PLCP;
   int nCurLen = 0;

   /* Compute the permuted LCP first (Kärkkäinen method) */
   Phi[intervals[0]] = -1;
   for (i = 1; i < nInWindowSize; i++)
      Phi[intervals[i]] = (unsigned int)intervals[i - 1];
   for (i = 0; i < nInWindowSize; i++) {
      if (Phi[i] == -1) {
         PLCP[i] = 0;
         continue;
      }
      int nMaxLen = (i > Phi[i]) ? (nInWindowSize - i) : (nInWindowSize - Phi[i]);
      while (nCurLen < nMaxLen && pInWindow[i + nCurLen] == pInWindow[Phi[i] + nCurLen]) nCurLen++;
      PLCP[i] = nCurLen;
      if (nCurLen > 0)
         nCurLen--;
   }

   /* Rotate permuted LCP into the LCP. This has better cache locality than the direct Kasai LCP method. This also
    * saves us from having to build the inverse suffix array index, as the LCP is calculated without it using this method,
    * and the interval builder below doesn't need it either. */
   intervals[0] &= POS_MASK;

   for (i = 1; i < nInWindowSize; i++) {
      int nIndex = (int)(intervals[i] & POS_MASK);
      int nLen = PLCP[nIndex];
      if (nLen < MIN_MATCH_SIZE)
         nLen = 0;
      if (nLen > LCP_MAX)
         nLen = LCP_MAX;
      int nTaggedLen = 0;
      if (nLen)
         nTaggedLen = (nLen << TAG_BITS) | (apultra_get_index_tag((unsigned int)nIndex) & ((1 << TAG_BITS) - 1));
      intervals[i] = ((unsigned long long)nIndex) | (((unsigned long long)nTaggedLen) << LCP_SHIFT);
   }

   /**
    * Build intervals for finding matches
    *
    * Methodology and code fragment taken from wimlib (CC0 license):
    * https://wimlib.net/git/?p=wimlib;a=blob_plain;f=src/lcpit_matchfinder.c;h=a2d6a1e0cd95200d1f3a5464d8359d5736b14cbe;hb=HEAD
    */
   unsigned long long * const SA_and_LCP = intervals;
   unsigned long long *pos_data = pCompressor->pos_data;
   unsigned long long next_interval_idx;
   unsigned long long *top = pCompressor->open_intervals;
   unsigned long long prev_pos = SA_and_LCP[0] & POS_MASK;

   *top = 0;
   intervals[0] = 0;
   next_interval_idx = 1;

   for (r = 1; r < nInWindowSize; r++) {
      const unsigned long long next_pos = SA_and_LCP[r] & POS_MASK;
      const unsigned long long next_lcp = SA_and_LCP[r] & LCP_MASK;
      const unsigned long long top_lcp = *top & LCP_MASK;

      if (next_lcp == top_lcp) {
         /* Continuing the deepest open interval  */
         pos_data[prev_pos] = *top;
      }
      else if (next_lcp > top_lcp) {
         /* Opening a new interval  */
         *++top = next_lcp | next_interval_idx++;
         pos_data[prev_pos] = *top;
      }
      else {
         /* Closing the deepest open interval  */
         pos_data[prev_pos] = *top;
         for (;;) {
            const unsigned long long closed_interval_idx = *top-- & POS_MASK;
            const unsigned long long superinterval_lcp = *top & LCP_MASK;

            if (next_lcp == superinterval_lcp) {
               /* Continuing the superinterval */
               intervals[closed_interval_idx] = *top;
               break;
            }
            else if (next_lcp > superinterval_lcp) {
               /* Creating a new interval that is a
                * superinterval of the one being
                * closed, but still a subinterval of
                * its superinterval  */
               *++top = next_lcp | next_interval_idx++;
               intervals[closed_interval_idx] = *top;
               break;
            }
            else {
               /* Also closing the superinterval  */
               intervals[closed_interval_idx] = *top;
            }
         }
      }
      prev_pos = next_pos;
   }

   /* Close any still-open intervals.  */
   pos_data[prev_pos] = *top;
   for (; top > pCompressor->open_intervals; top--)
      intervals[*top & POS_MASK] = *(top - 1);

   /* Success */
   return 0;
}

/**
 * Find matches at the specified offset in the input window
 *
 * @param pCompressor compression context
 * @param nOffset offset to find matches at, in the input window
 * @param pMatches pointer to returned matches
 * @param pMatchDepth pointer to returned match depths
 * @param pMatch1 pointer to 1-byte length, 4 bit offset match
 * @param nMaxMatches maximum number of matches to return (0 for none)
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 *
 * @return number of matches
 */
int apultra_find_matches_at(apultra_compressor *pCompressor, const int nOffset, apultra_match *pMatches, unsigned short *pMatchDepth, unsigned char *pMatch1, const int nMaxMatches, const int nBlockFlags) {
   unsigned long long *intervals = pCompressor->intervals;
   unsigned long long *pos_data = pCompressor->pos_data;
   unsigned long long ref;
   unsigned long long super_ref;
   unsigned long long match_pos;
   apultra_match *matchptr;
   unsigned short *depthptr;

   *pMatch1 = 0;

   /**
    * Find matches using intervals
    *
    * Taken from wimlib (CC0 license):
    * https://wimlib.net/git/?p=wimlib;a=blob_plain;f=src/lcpit_matchfinder.c;h=a2d6a1e0cd95200d1f3a5464d8359d5736b14cbe;hb=HEAD
    */

    /* Get the deepest lcp-interval containing the current suffix. */
   ref = pos_data[nOffset];

   pos_data[nOffset] = 0;

   /* Ascend until we reach a visited interval, the root, or a child of the
    * root.  Link unvisited intervals to the current suffix as we go.  */
   while ((super_ref = intervals[ref & POS_MASK]) & LCP_MASK) {
      intervals[ref & POS_MASK] = nOffset | VISITED_FLAG;
      ref = super_ref;
   }

   if (super_ref == 0) {
      /* In this case, the current interval may be any of:
       * (1) the root;
       * (2) an unvisited child of the root */

      if (ref != 0)  /* Not the root?  */
         intervals[ref & POS_MASK] = nOffset | VISITED_FLAG;
      return 0;
   }

   /* Ascend indirectly via pos_data[] links.  */
   match_pos = super_ref & EXCL_VISITED_MASK;
   matchptr = pMatches;
   depthptr = pMatchDepth;
   int nPrevOffset = 0;
   int nPrevLen = 0;
   int nCurDepth = 0;
   unsigned short *cur_depth = NULL;
   
   if (nOffset >= match_pos && (nBlockFlags & 3) == 3) {
      int nMatchOffset = (int)(nOffset - match_pos);
      int nMatchLen = (int)(ref >> (LCP_SHIFT + TAG_BITS));

      if ((matchptr - pMatches) < nMaxMatches) {
         if (nMatchOffset <= MAX_OFFSET) {
            if (nPrevOffset && nPrevLen > 2 && nMatchOffset == (nPrevOffset - 1) && nMatchLen == (nPrevLen - 1) && cur_depth && nCurDepth < LCP_MAX) {
               nCurDepth++;
               *cur_depth = nCurDepth;
            }
            else {
               nCurDepth = 0;

               cur_depth = depthptr;
               matchptr->length = nMatchLen;
               matchptr->offset = nMatchOffset;
               *depthptr = 0;
               matchptr++;
               depthptr++;
            }

            nPrevLen = nMatchLen;
            nPrevOffset = nMatchOffset;
         }
      }
   }

   for (;;) {
      if ((super_ref = pos_data[match_pos]) > ref) {
         match_pos = intervals[super_ref & POS_MASK] & EXCL_VISITED_MASK;

         if (nOffset >= match_pos && (nBlockFlags & 3) == 3) {
            int nMatchOffset = (int)(nOffset - match_pos);
            int nMatchLen = (int)(ref >> (LCP_SHIFT + TAG_BITS));

            if ((matchptr - pMatches) < nMaxMatches) {
               if (nMatchOffset <= MAX_OFFSET && abs(nMatchOffset - nPrevOffset) >= 288) {
                  if (nPrevOffset && nPrevLen > 2 && nMatchOffset == (nPrevOffset - 1) && nMatchLen == (nPrevLen - 1) && cur_depth && nCurDepth < LCP_MAX) {
                     nCurDepth++;
                     *cur_depth = nCurDepth | 0x8000;
                  }
                  else {
                     nCurDepth = 0;

                     cur_depth = depthptr;
                     matchptr->length = nMatchLen;
                     matchptr->offset = nMatchOffset;
                     *depthptr = 0x8000;
                     matchptr++;
                     depthptr++;
                  }

                  nPrevLen = nMatchLen;
                  nPrevOffset = nMatchOffset;
               }
            }
         }
      }

      while ((super_ref = pos_data[match_pos]) > ref) {
         match_pos = intervals[super_ref & POS_MASK] & EXCL_VISITED_MASK;

         if (nOffset > match_pos && (nBlockFlags & 3) == 3) {
            int nMatchOffset = (int)(nOffset - match_pos);
            int nMatchLen = (int)(ref >> (LCP_SHIFT + TAG_BITS));

            if ((matchptr - pMatches) < nMaxMatches) {
               if (nMatchOffset <= MAX_OFFSET && (nMatchLen >= 3 || (nMatchLen >= 2 && (matchptr - pMatches) < (nMaxMatches - 1))) && nMatchLen < 1280 && abs(nMatchOffset - nPrevOffset) >= 288) {
                  if (nPrevOffset && nPrevLen > 2 && nMatchOffset == (nPrevOffset - 1) && nMatchLen == (nPrevLen - 1) && cur_depth && nCurDepth < LCP_MAX) {
                     nCurDepth++;
                     *cur_depth = nCurDepth | 0x8000;
                  }
                  else {
                     nCurDepth = 0;

                     cur_depth = depthptr;
                     matchptr->length = nMatchLen;
                     matchptr->offset = nMatchOffset;
                     *depthptr = 0x8000;
                     matchptr++;
                     depthptr++;
                  }

                  nPrevLen = nMatchLen;
                  nPrevOffset = nMatchOffset;
               }
            }
         }
      }

      intervals[ref & POS_MASK] = nOffset | VISITED_FLAG;
      pos_data[match_pos] = (unsigned long long)ref;

      int nMatchOffset = (int)(nOffset - match_pos);
      int nMatchLen = (int)(ref >> (LCP_SHIFT + TAG_BITS));

      if ((matchptr - pMatches) < nMaxMatches) {
         if (nMatchOffset <= MAX_OFFSET && nMatchOffset != nPrevOffset) {
            if (nPrevOffset && nPrevLen > 2 && nMatchOffset == (nPrevOffset - 1) && nMatchLen == (nPrevLen - 1) && cur_depth && nCurDepth < LCP_MAX) {
               nCurDepth++;
               *cur_depth = nCurDepth;
            }
            else {
               nCurDepth = 0;

               cur_depth = depthptr;
               matchptr->length = nMatchLen;
               matchptr->offset = nMatchOffset;
               *depthptr = 0;
               matchptr++;
               depthptr++;
            }

            nPrevLen = nMatchLen;
            nPrevOffset = nMatchOffset;
         }
      }

      if (nMatchOffset && nMatchOffset < 16 && nMatchLen)
         *pMatch1 = nMatchOffset;

      if (super_ref == 0)
         break;
      ref = super_ref;
      match_pos = intervals[ref & POS_MASK] & EXCL_VISITED_MASK;

      if (nOffset > match_pos && (nBlockFlags & 3) == 3) {
         int nMatchOffset = (int)(nOffset - match_pos);
         int nMatchLen = (int)(ref >> (LCP_SHIFT + TAG_BITS));

         if ((matchptr - pMatches) < nMaxMatches) {
            if (nMatchOffset <= MAX_OFFSET && nMatchLen >= 2 && abs(nMatchOffset - nPrevOffset) >= 288) {
               if (nPrevOffset && nPrevLen > 2 && nMatchOffset == (nPrevOffset - 1) && nMatchLen == (nPrevLen - 1) && cur_depth && nCurDepth < LCP_MAX) {
                  nCurDepth++;
                  *cur_depth = nCurDepth | 0x8000;
               }
               else {
                  nCurDepth = 0;

                  cur_depth = depthptr;
                  matchptr->length = nMatchLen;
                  matchptr->offset = nMatchOffset;
                  *depthptr = 0x8000;
                  matchptr++;
                  depthptr++;
               }

               nPrevLen = nMatchLen;
               nPrevOffset = nMatchOffset;
            }
         }
      }
   }

   return (int)(matchptr - pMatches);
}

/**
 * Skip previously compressed bytes
 *
 * @param pCompressor compression context
 * @param nStartOffset current offset in input window (typically 0)
 * @param nEndOffset offset to skip to in input window (typically the number of previously compressed bytes)
 */
void apultra_skip_matches(apultra_compressor *pCompressor, const int nStartOffset, const int nEndOffset) {
   apultra_match match;
   unsigned short depth;
   unsigned char match1;
   int i;

   /* Skipping still requires scanning for matches, as this also performs a lazy update of the intervals. However,
    * we don't store the matches. */
   for (i = nStartOffset; i < nEndOffset; i++) {
      apultra_find_matches_at(pCompressor, i, &match, &depth, &match1, 0, 0);
   }
}

/**
 * Find all matches for the data to be compressed
 *
 * @param pCompressor compression context
 * @param nMatchesPerOffset maximum number of matches to store for each offset
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 */
void apultra_find_all_matches(apultra_compressor *pCompressor, const int nMatchesPerOffset, const int nStartOffset, const int nEndOffset, const int nBlockFlags) {
   apultra_match *pMatch = pCompressor->match;
   unsigned short *pMatchDepth = pCompressor->match_depth;
   unsigned char *pMatch1 = pCompressor->match1;
   int i;

   for (i = nStartOffset; i < nEndOffset; i++) {
      int nMatches = apultra_find_matches_at(pCompressor, i, pMatch, pMatchDepth, pMatch1, nMatchesPerOffset, nBlockFlags);

      while (nMatches < nMatchesPerOffset) {
         pMatch[nMatches].length = 0;
         pMatch[nMatches].offset = 0;
         pMatchDepth[nMatches] = 0;
         nMatches++;
      }

      pMatch += nMatchesPerOffset;
      pMatchDepth += nMatchesPerOffset;
      pMatch1++;
   }
}
/*
 * shrink.c - compressor implementation
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Uses the libdivsufsort library Copyright (c) 2003-2008 Yuta Mori
 *
 * Inspired by cap by Sven-Åke Dahl. https://github.com/svendahl/cap
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 * With ideas from LZ4 by Yann Collet. https://github.com/lz4/lz4
 * With help and support from spke <zxintrospec@gmail.com>
 *
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TOKEN_CODE_LARGE_MATCH   2 /* 10 */
#define TOKEN_SIZE_LARGE_MATCH   2

#define TOKEN_CODE_7BIT_MATCH    6 /* 110 */
#define TOKEN_SIZE_7BIT_MATCH    3

#define TOKEN_CODE_4BIT_MATCH    7 /* 111 */
#define TOKEN_SIZE_4BIT_MATCH    3

#define CountShift(N,bits)  if ((N)>>(bits)) { (N)>>=(bits); (n) += (bits); }

/** Code values for variable 8+gamma2 bits offset + gamma2 len match; 7 bits offset + 1 bit len match; and 4 bits offset + fixed 1 byte len match */
static const int _token_code[3] = { TOKEN_CODE_LARGE_MATCH, TOKEN_CODE_7BIT_MATCH, TOKEN_CODE_4BIT_MATCH };

/** Code sizes for variable 8+gamma2 bits offset + gamma2 len match; 7 bits offset + 1 bit len match; and 4 bits offset + fixed 1 byte len match */
static const int _token_size[3] = { TOKEN_SIZE_LARGE_MATCH, TOKEN_SIZE_7BIT_MATCH, TOKEN_SIZE_4BIT_MATCH };

/** Gamma2 bit counts for common values, up to 255 */
static char _gamma2_size[256] = {
   0, 0, 2, 2, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
};

/**
 * Write bit to output (compressed) buffer
 *
 * @param pOutData pointer to output buffer
 * @param nOutOffset current write index into output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nBit value of bit to write (0 or 1)
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 *
 * @return updated write index into output buffer, or -1 in case of an error
 */
static int apultra_write_bit(unsigned char *pOutData, int nOutOffset, const int nMaxOutDataSize, const int nBit, int *nCurBitsOffset, int *nCurBitMask) {
   if (nOutOffset < 0) return -1;

   if ((*nCurBitsOffset) == INT_MIN) {
      /* Allocate a new byte in the stream to pack bits in */
      if (nOutOffset >= nMaxOutDataSize) return -1;
      (*nCurBitsOffset) = nOutOffset;
      (*nCurBitMask) = 1 << 7;
      pOutData[nOutOffset++] = 0;
   }

   if (nBit) {
      pOutData[(*nCurBitsOffset)] |= (*nCurBitMask);
   }

   (*nCurBitMask) >>= 1;
   if ((*nCurBitMask) == 0) {
      /* Current byte is full */
      (*nCurBitsOffset) = INT_MIN;
   }

   return nOutOffset;
}

/**
 * Get size of gamma2 encoded value
 *
 * @param nValue value of evaluate (2..n)
 *
 * @return number of bits required
 */
static int apultra_get_gamma2_size(int nValue) {
   if (nValue >= 0 && nValue < 256)
      return _gamma2_size[nValue];
   else {
      unsigned int n = 0;
      CountShift(nValue, 16);
      CountShift(nValue, 8);
      CountShift(nValue, 4);
      CountShift(nValue, 2);
      CountShift(nValue, 1);

      return n << 1;
   }
}

/**
 * Write gamma2 encoded value to output (compressed) buffer
 *
 * @param pOutData pointer to output buffer
 * @param nOutOffset current write index into output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nValue value of write (2..n)
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 *
 * @return updated write index into output buffer, or -1 in case of an error
 */
static int apultra_write_gamma2_value(unsigned char *pOutData, int nOutOffset, const int nMaxOutDataSize, int nValue, int *nCurBitsOffset, int *nCurBitMask) {
   int msb = 30;
   while ((nValue >> msb--) == 0);

   while (msb >= 0) {
      int bit = (nValue >> msb) & 1;
   
      nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, bit ? 1 : 0, nCurBitsOffset, nCurBitMask);
      msb--;
      nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, msb >= 0 ? 1 : 0, nCurBitsOffset, nCurBitMask);
   }

   return nOutOffset;
}

/**
 * Get the number of extra bits required to represent a match offset
 *
 * @param nLength match length
 * @param nMatchOffset match offset
 * @param nFollowsLiteral non-zero if the match follows a literal, zero if it immediately follows another match
 *
 * @return number of extra bits required
 */
static inline int apultra_get_offset_varlen_size(const int nLength, const int nMatchOffset, const int nFollowsLiteral) {
   if (nLength == 1 && nMatchOffset < 16)
      return 4 + TOKEN_SIZE_4BIT_MATCH;
   else {
      if (nLength <= 3 && nMatchOffset < 128)
         return 8 + TOKEN_SIZE_7BIT_MATCH;
      else {
         if (nFollowsLiteral)
            return 8 + TOKEN_SIZE_LARGE_MATCH + apultra_get_gamma2_size((nMatchOffset >> 8) + 3);
         else
            return 8 + TOKEN_SIZE_LARGE_MATCH + apultra_get_gamma2_size((nMatchOffset >> 8) + 2);
      }
   }
}

/**
 * Get the number of extra bits required to represent a rep-match
 *
 * @return number of extra bits required
 */
static inline int apultra_get_rep_offset_varlen_size(void) {
   return TOKEN_SIZE_LARGE_MATCH + 2 /* apultra_get_gamma2_size(2) */;
}

/**
 * Get the number of extra bits required to represent a match length
 *
 * @param nLength match length
 * @param nMatchOffset match offset
 * @param nIsRepMatch non-zero if this is a rep-match, zero if it is a regular match
 *
 * @return number of extra bits required
 */
static inline int apultra_get_match_varlen_size(int nLength, const int nMatchOffset, const int nIsRepMatch) {
   if (nLength == 1 && nMatchOffset < 16)
      return 0;
   else {
      if (nLength <= 3 && nMatchOffset < 128 && !nIsRepMatch)
         return 0;
      else {
         if (nMatchOffset < 128 && !nIsRepMatch)
            nLength -= 2;
         if (nMatchOffset < MINMATCH3_OFFSET || nIsRepMatch)
            return apultra_get_gamma2_size(nLength);
         else if (nMatchOffset < MINMATCH4_OFFSET)
            return apultra_get_gamma2_size(nLength - 1);
         else
            return apultra_get_gamma2_size(nLength - 2);
      }
   }
}

/**
 * Write extra encoded match length bytes to output (compressed) buffer. The caller must first check that there is enough
 * room to write the bytes.
 *
 * @param pOutData pointer to output buffer
 * @param nOutOffset current write index into output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 * @param nLength match length
 * @param nMatchOffset match offset
 * @param nIsRepMatch non-zero if this is a rep-match, zero if it is a regular match
 *
 * @return updated write index into output buffer, or -1 in case of an error
 */
static inline int apultra_write_match_varlen(unsigned char *pOutData, int nOutOffset, const int nMaxOutDataSize, int *nCurBitsOffset, int *nCurBitMask, int nLength, const int nMatchOffset, const int nIsRepMatch) {
   if (nLength < 2)
      return -1;

   if (nMatchOffset < 128 && !nIsRepMatch)
      nLength -= 2;
   if (nMatchOffset < MINMATCH3_OFFSET || nIsRepMatch)
      return apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, nLength, nCurBitsOffset, nCurBitMask);
   else if (nMatchOffset < MINMATCH4_OFFSET)
      return apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, nLength - 1, nCurBitsOffset, nCurBitMask);
   else
      return apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, nLength - 2, nCurBitsOffset, nCurBitMask);
}

/**
 * Insert forward rep candidate
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param i input data window position whose matches are being considered
 * @param nMatchOffset match offset to use as rep candidate
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param nDepth current insertion depth
 */
static void apultra_insert_forward_match(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int i, const int nMatchOffset, const int nStartOffset, const int nEndOffset, const int nMatchesPerArrival, int nDepth) {
   apultra_arrival *arrival = pCompressor->arrival + ((i - nStartOffset) * nMatchesPerArrival);
   int j;

   if (nDepth >= 10) return;

   for (j = 0; j < nMatchesPerArrival && arrival[j].from_slot; j++) {
      int nRepOffset = arrival[j].rep_offset;

      if (nMatchOffset != nRepOffset && nRepOffset) {
         int nRepPos = arrival[j].rep_pos;

         if (arrival[j].follows_literal &&
            nRepPos &&
            nRepPos >= nMatchOffset &&
            nRepPos < nEndOffset) {
            int nCurRepLen = 0;

            int nMaxRepLen = nEndOffset - nRepPos;
            if (nMaxRepLen > LCP_MAX)
               nMaxRepLen = LCP_MAX;
            while ((nCurRepLen + 8) < nMaxRepLen && !memcmp(pInWindow + nRepPos + nCurRepLen, pInWindow + nRepPos - nMatchOffset + nCurRepLen, 8))
               nCurRepLen += 8;
            while ((nCurRepLen + 4) < nMaxRepLen && !memcmp(pInWindow + nRepPos + nCurRepLen, pInWindow + nRepPos - nMatchOffset + nCurRepLen, 4))
               nCurRepLen += 4;
            while (nCurRepLen < nMaxRepLen && pInWindow[nRepPos + nCurRepLen] == pInWindow[nRepPos - nMatchOffset + nCurRepLen])
               nCurRepLen++;

            if (nCurRepLen >= 2) {
               apultra_match *fwd_match = pCompressor->match + ((nRepPos - nStartOffset) << MATCHES_PER_INDEX_SHIFT);
               unsigned short *fwd_depth = pCompressor->match_depth + ((nRepPos - nStartOffset) << MATCHES_PER_INDEX_SHIFT);
               int exists = 0;
               int r;

               for (r = 0; r < NMATCHES_PER_INDEX && fwd_match[r].length >= MIN_MATCH_SIZE; r++) {
                  if (fwd_match[r].offset == nMatchOffset && (fwd_depth[r] & 0x7fff) == 0) {
                     exists = 1;

                     if ((int)fwd_match[r].length < nCurRepLen) {
                        fwd_match[r].length = nCurRepLen;
                        fwd_depth[r] = 0;
                        apultra_insert_forward_match(pCompressor, pInWindow, nRepPos, nMatchOffset, nStartOffset, nEndOffset, nMatchesPerArrival, nDepth + 1);
                     }
                     break;
                  }
               }

               if (!exists && r < NMATCHES_PER_INDEX) {
                  fwd_match[r].offset = nMatchOffset;
                  fwd_match[r].length = nCurRepLen;
                  fwd_depth[r] = 0;

                  apultra_insert_forward_match(pCompressor, pInWindow, nRepPos, nMatchOffset, nStartOffset, nEndOffset, nMatchesPerArrival, nDepth + 1);
               }
            }
         }
      }
   }
}

/**
 * Attempt to pick optimal matches, so as to produce the smallest possible output that decompresses to the same input
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param nInsertForwardReps non-zero to insert forward repmatch candidates, zero to use the previously inserted candidates
 * @param nCurRepMatchOffset starting rep offset for this block
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 */
static void apultra_optimize_forward(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nStartOffset, const int nEndOffset, const int nInsertForwardReps, const int *nCurRepMatchOffset, const int nBlockFlags, const int nMatchesPerArrival) {
   apultra_arrival *arrival = pCompressor->arrival - (nStartOffset * nMatchesPerArrival);
   int i, j, n;

   if ((nEndOffset - nStartOffset) > pCompressor->block_size) return;

   memset(arrival + (nStartOffset * nMatchesPerArrival), 0, sizeof(apultra_arrival) * ((nEndOffset - nStartOffset + 1) * nMatchesPerArrival));

   arrival[nStartOffset * nMatchesPerArrival].from_slot = -1;
   arrival[nStartOffset * nMatchesPerArrival].rep_offset = *nCurRepMatchOffset;

   for (i = (nStartOffset * nMatchesPerArrival); i != ((nEndOffset+1) * nMatchesPerArrival); i++) {
      arrival[i].cost = 0x40000000;
   }

   for (i = nStartOffset; i != nEndOffset; i++) {
      apultra_arrival *cur_arrival = &arrival[i * nMatchesPerArrival];
      int m;
      
      unsigned char *match1 = pCompressor->match1 + (i - nStartOffset);
      int nShortOffset;
      int nMatchLen;
      int nLiteralCost;
      unsigned short *match_depth = pCompressor->match_depth + ((i - nStartOffset) << MATCHES_PER_INDEX_SHIFT);
      const int nRepMatchOffsetCost = apultra_get_rep_offset_varlen_size();
      int nMinRepLen[NMATCHES_PER_ARRIVAL];

      if ((pInWindow[i] != 0 && (*match1) == 0) || (i == nStartOffset && (nBlockFlags & 1))) {
         nShortOffset = 0;
         nMatchLen = 0;
         nLiteralCost = 9 /* literal bit + literal byte */;
      }
      else {
         nShortOffset = (pInWindow[i] == 0) ? 0 : (*match1);
         nMatchLen = 1;
         nLiteralCost = 4 + TOKEN_SIZE_4BIT_MATCH /* command and offset cost; no length cost */;
      }

      for (j = 0; j < nMatchesPerArrival && cur_arrival[j].from_slot; j++) {
         int nPrevCost = cur_arrival[j].cost & 0x3fffffff;
         int nCodingChoiceCost = nPrevCost + nLiteralCost;

         apultra_arrival *pDestSlots = &cur_arrival[1 * nMatchesPerArrival];
         if (nCodingChoiceCost <= pDestSlots[nMatchesPerArrival - 1].cost) {
            int nScore = cur_arrival[j].score + (nShortOffset ? 3 : 1);
            int exists = 0;

            for (n = 0;
               n < nMatchesPerArrival && pDestSlots[n].cost < nCodingChoiceCost;
               n++) {
               if (pDestSlots[n].rep_offset == cur_arrival[j].rep_offset) {
                  exists = 1;
                  break;
               }
            }

            if (!exists) {
               int nn;

               for (nn = n;
                  nn < nMatchesPerArrival && pDestSlots[nn].cost == nCodingChoiceCost;
                  nn++) {
                  if (pDestSlots[nn].rep_offset == cur_arrival[j].rep_offset) {
                     exists = 1;
                     break;
                  }
               }

               if (!exists) {
                  for (; n < nMatchesPerArrival; n++) {
                     apultra_arrival* pDestArrival = &pDestSlots[n];
                     if (nCodingChoiceCost < pDestArrival->cost ||
                        (nCodingChoiceCost == pDestArrival->cost && nScore < pDestArrival->score)) {
                        int z;

                        for (z = n; z < nMatchesPerArrival - 1; z++) {
                           if (pDestSlots[z].rep_offset == cur_arrival[j].rep_offset)
                              break;
                        }

                        memmove(&pDestSlots[n + 1],
                           &pDestSlots[n],
                           sizeof(apultra_arrival) * (z - n));

                        pDestArrival->cost = nCodingChoiceCost;
                        pDestArrival->from_pos = i;
                        pDestArrival->from_slot = j + 1;
                        pDestArrival->follows_literal = 1;
                        pDestArrival->short_offset = nShortOffset;
                        pDestArrival->match_len = nMatchLen;
                        pDestArrival->score = nScore;
                        pDestArrival->rep_offset = cur_arrival[j].rep_offset;
                        pDestArrival->rep_pos = cur_arrival[j].rep_pos;
                        break;
                     }
                  }
               }
            }
         }
      }

      if (i == nStartOffset && (nBlockFlags & 1)) continue;

      apultra_match *match = pCompressor->match + ((i - nStartOffset) << MATCHES_PER_INDEX_SHIFT);
      memset(nMinRepLen, 0, NMATCHES_PER_ARRIVAL * sizeof(int));

      for (m = 0; m < NMATCHES_PER_INDEX && match[m].length; m++) {
         const int nOrigMatchLen = match[m].length;
         const int nOrigMatchOffset = match[m].offset;
         const unsigned int nOrigMatchDepth = match_depth[m] & 0x7fff;
         const int nScorePenalty = 3 + ((match_depth[m] & 0x8000) >> 15);

         for (unsigned int d = 0; d <= nOrigMatchDepth; d += (nOrigMatchDepth ? nOrigMatchDepth : 1)) {
            int nStartingMatchLen, k;
            int nMaxRepLen[NMATCHES_PER_ARRIVAL];
            int nMinMatchLen[NMATCHES_PER_ARRIVAL];

            const int nMatchOffset = nOrigMatchOffset - d;
            int nMatchLen = nOrigMatchLen - d;

            if ((i + nMatchLen) > nEndOffset)
               nMatchLen = nEndOffset - i;

            for (j = 0; j < nMatchesPerArrival && cur_arrival[j].from_slot; j++) {
               int nRepOffset = cur_arrival[j].rep_offset;
               int nCurMaxLen = 0;

               if (cur_arrival[j].follows_literal &&
                  nRepOffset) {

                  if (nMatchOffset == nRepOffset)
                     nCurMaxLen = nMatchLen;
                  else {
                     if (i >= nRepOffset &&
                        (i - nRepOffset + nMatchLen) <= nEndOffset) {
                        nCurMaxLen = nMinRepLen[j];
                        while ((nCurMaxLen + 8) < nMatchLen && !memcmp(pInWindow + i - nRepOffset + nCurMaxLen, pInWindow + i - nMatchOffset + nCurMaxLen, 8))
                           nCurMaxLen += 8;
                        while ((nCurMaxLen + 4) < nMatchLen && !memcmp(pInWindow + i - nRepOffset + nCurMaxLen, pInWindow + i - nMatchOffset + nCurMaxLen, 4))
                           nCurMaxLen += 4;
                        while (nCurMaxLen < nMatchLen && pInWindow[i - nRepOffset + nCurMaxLen] == pInWindow[i - nMatchOffset + nCurMaxLen])
                           nCurMaxLen++;
                        nMinRepLen[j] = nCurMaxLen;
                     }
                  }
               }

               nMaxRepLen[j] = nCurMaxLen;

               int nIsRepMatch = (nMatchOffset == nRepOffset &&
                  cur_arrival[j].follows_literal) ? 1 : 0;

               if (nIsRepMatch == 0) {
                  if (nMatchOffset < MINMATCH3_OFFSET)
                     nMinMatchLen[j] = 2;
                  else {
                     if (nMatchOffset < MINMATCH4_OFFSET)
                        nMinMatchLen[j] = 3;
                     else
                        nMinMatchLen[j] = 4;
                  }
               }
               else {
                  nMinMatchLen[j] = nMatchLen + 1;
               }
            }
            while (j < NMATCHES_PER_ARRIVAL) {
               nMaxRepLen[j] = 0;
               nMinMatchLen[j++] = 0;
            }

            if (nInsertForwardReps)
               apultra_insert_forward_match(pCompressor, pInWindow, i, nMatchOffset, nStartOffset, nEndOffset, nMatchesPerArrival, 0);

            if (nMatchLen >= LEAVE_ALONE_MATCH_SIZE && i >= nMatchLen)
               nStartingMatchLen = nMatchLen;
            else
               nStartingMatchLen = 2;

            if (nStartingMatchLen <= nMatchLen) {
               int nNoRepMatchOffsetCostForLit[2];

               if (nStartingMatchLen <= 3) {
                  nNoRepMatchOffsetCostForLit[0] = apultra_get_offset_varlen_size(2, nMatchOffset, 0);
                  nNoRepMatchOffsetCostForLit[1] = apultra_get_offset_varlen_size(2, nMatchOffset, 1);
               }
               else {
                  nNoRepMatchOffsetCostForLit[0] = apultra_get_offset_varlen_size(4, nMatchOffset, 0);
                  nNoRepMatchOffsetCostForLit[1] = apultra_get_offset_varlen_size(4, nMatchOffset, 1);
               }

               for (k = nStartingMatchLen; k <= nMatchLen; k++) {
                  int nNoRepMatchMatchLenCost;
                  int nRepMatchMatchLenCost = apultra_get_gamma2_size(k);

                  if (k <= 3 && nMatchOffset < 128)
                     nNoRepMatchMatchLenCost = 0;
                  else {
                     if (nMatchOffset < 128 || nMatchOffset >= MINMATCH4_OFFSET)
                        nNoRepMatchMatchLenCost = apultra_get_gamma2_size(k - 2);
                     else if (nMatchOffset < MINMATCH3_OFFSET)
                        nNoRepMatchMatchLenCost = nRepMatchMatchLenCost;
                     else
                        nNoRepMatchMatchLenCost = apultra_get_gamma2_size(k - 1);
                  }

                  int nRepMatchCmdCost = nRepMatchOffsetCost + nRepMatchMatchLenCost;
                  apultra_arrival *pDestSlots = &cur_arrival[k * nMatchesPerArrival];
                  int nInsertedNonRepOffset = 0;
 
                  for (j = 0; j < nMatchesPerArrival && cur_arrival[j].from_slot; j++) {
                     int nPrevCost = cur_arrival[j].cost & 0x3fffffff;

                     int nRepCodingChoiceCost = nPrevCost /* the actual cost of the literals themselves accumulates up the chain */ + nRepMatchCmdCost;

                     if (nRepCodingChoiceCost <= pDestSlots[nMatchesPerArrival - 1].cost) {
                        if (k >= nMinMatchLen[j] && !nInsertedNonRepOffset) {
                           int nMatchCmdCost = nNoRepMatchMatchLenCost + nNoRepMatchOffsetCostForLit[cur_arrival[j].follows_literal];
                           int nCodingChoiceCost = nPrevCost /* the actual cost of the literals themselves accumulates up the chain */ + nMatchCmdCost;

                           if (nCodingChoiceCost <= pDestSlots[nMatchesPerArrival - 1].cost) {
                              int exists = 0;

                              for (n = 0;
                                 n < nMatchesPerArrival && pDestSlots[n].cost < nCodingChoiceCost;
                                 n++) {
                                 if (pDestSlots[n].rep_offset == nMatchOffset) {
                                    exists = 1;
                                    if ((nCodingChoiceCost - pDestSlots[n].cost) > 8)
                                       nInsertedNonRepOffset = 1;
                                    break;
                                 }
                              }

                              if (!exists) {
                                 int nn;

                                 for (nn = n;
                                    nn < nMatchesPerArrival && pDestSlots[nn].cost == nCodingChoiceCost;
                                    nn++) {
                                    if (pDestSlots[nn].rep_offset == nMatchOffset) {
                                       exists = 1;
                                       break;
                                    }
                                 }

                                 if (!exists) {
                                    int nScore = cur_arrival[j].score + nScorePenalty;

                                    if (nMatchLen >= LCP_MAX)
                                       nCodingChoiceCost -= 1;

                                    for (; n < nMatchesPerArrival - 1; n++) {
                                       apultra_arrival* pDestArrival = &pDestSlots[n];

                                       if (nCodingChoiceCost < pDestArrival->cost ||
                                          (nCodingChoiceCost == pDestArrival->cost && nScore < pDestArrival->score)) {
                                          int z;

                                          for (z = n; z < nMatchesPerArrival - 1; z++) {
                                             if (pDestSlots[z].rep_offset == nMatchOffset)
                                                break;
                                          }

                                          if (z == (nMatchesPerArrival - 1) && pDestSlots[z].from_slot && pDestSlots[z].match_len < 2)
                                             z--;

                                          memmove(&pDestSlots[n + 1],
                                             &pDestSlots[n],
                                             sizeof(apultra_arrival) * (z - n));

                                          pDestArrival->cost = nCodingChoiceCost;
                                          pDestArrival->from_pos = i;
                                          pDestArrival->from_slot = j + 1;
                                          pDestArrival->short_offset = 0;
                                          pDestArrival->match_len = k;
                                          pDestArrival->follows_literal = 0;
                                          pDestArrival->score = nScore;
                                          pDestArrival->rep_offset = nMatchOffset;
                                          pDestArrival->rep_pos = i;
                                          nMinMatchLen[j] = k + 1;
                                          break;
                                       }
                                    }
                                 }
                              }
                           }
                        }

                        /* If this coding choice doesn't rep-match, see if we still get a match by using the current repmatch offset for this arrival. This can occur (and not have the
                         * matchfinder offer the offset in the first place, or have too many choices with the same cost to retain the repmatchable offset) when compressing regions
                         * of identical bytes, for instance. Checking for this provides a big compression win on some files. */

                        if (nMaxRepLen[j] >= k) {
                           int nRepOffset = cur_arrival[j].rep_offset;

                           /* A match is possible at the rep offset; insert the extra coding choice. */

                           int exists = 0;

                           for (n = 0;
                              n < nMatchesPerArrival && pDestSlots[n].cost < nRepCodingChoiceCost;
                              n++) {
                              if (pDestSlots[n].rep_offset == nRepOffset) {
                                 exists = 1;
                                 break;
                              }
                           }

                           if (!exists) {
                              int nn;

                              for (nn = n;
                                 nn < nMatchesPerArrival && pDestSlots[nn].cost == nRepCodingChoiceCost;
                                 nn++) {
                                 if (pDestSlots[nn].rep_offset == nRepOffset) {
                                    exists = 1;
                                    break;
                                 }
                              }

                              if (!exists) {
                                 int nScore = cur_arrival[j].score + 2;

                                 for (; n < nMatchesPerArrival; n++) {
                                    apultra_arrival* pDestArrival = &pDestSlots[n];

                                    if (nRepCodingChoiceCost < pDestArrival->cost ||
                                       (nRepCodingChoiceCost == pDestArrival->cost && nScore < pDestArrival->score)) {
                                       int z;

                                       for (z = n; z < nMatchesPerArrival - 1; z++) {
                                          if (pDestSlots[z].rep_offset == nRepOffset)
                                             break;
                                       }

                                       memmove(&pDestSlots[n + 1],
                                          &pDestSlots[n],
                                          sizeof(apultra_arrival) * (z - n));

                                       pDestArrival->cost = nRepCodingChoiceCost;
                                       pDestArrival->from_pos = i;
                                       pDestArrival->from_slot = j + 1;
                                       pDestArrival->short_offset = 0;
                                       pDestArrival->match_len = k;
                                       pDestArrival->follows_literal = 0;
                                       pDestArrival->score = nScore;
                                       pDestArrival->rep_offset = nRepOffset;
                                       pDestArrival->rep_pos = i;
                                       break;
                                    }
                                 }
                              }
                           }
                        }
                     }
                     else {
                        break;
                     }
                  }

                  if (k == 3) {
                     nNoRepMatchOffsetCostForLit[0] = apultra_get_offset_varlen_size(4, nMatchOffset, 0);
                     nNoRepMatchOffsetCostForLit[1] = apultra_get_offset_varlen_size(4, nMatchOffset, 1);
                  }
               }
            }

            if (nOrigMatchLen >= 512)
               break;
         }
      }
   }
   
   apultra_arrival *end_arrival = &arrival[(i * nMatchesPerArrival) + 0];
   apultra_final_match *pBestMatch = pCompressor->best_match - nStartOffset;
      
   while (end_arrival->from_slot > 0 && end_arrival->from_pos >= 0 && (int)end_arrival->from_pos < nEndOffset) {
      pBestMatch[end_arrival->from_pos].length = end_arrival->match_len;
      if (end_arrival->match_len >= 2)
         pBestMatch[end_arrival->from_pos].offset = end_arrival->rep_offset;
      else
         pBestMatch[end_arrival->from_pos].offset = end_arrival->short_offset;

      end_arrival = &arrival[(end_arrival->from_pos * nMatchesPerArrival) + (end_arrival->from_slot-1)];
   }
}

/**
 * Attempt to replace matches by literals when it makes the final bitstream smaller, and merge large matches
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param pBestMatch optimal matches to evaluate and update
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param nCurRepMatchOffset starting rep offset for this block
 *
 * @return non-zero if the number of tokens was reduced, 0 if it wasn't
 */
static int apultra_reduce_commands(apultra_compressor *pCompressor, const unsigned char *pInWindow, apultra_final_match *pBestMatch, const int nStartOffset, const int nEndOffset, const int *nCurRepMatchOffset) {
   int i;
   int nNumLiterals = 0;
   int nRepMatchOffset = *nCurRepMatchOffset;
   int nFollowsLiteral = 0;
   int nDidReduce = 0;
   int nLastMatchLen = 0;
   const unsigned char *match1 = pCompressor->match1 - nStartOffset;

   for (i = nStartOffset; i < nEndOffset; ) {
      apultra_final_match *pMatch = pBestMatch + i;

      if (pMatch->length <= 1 &&
         (i + 1) < nEndOffset &&
         pBestMatch[i + 1].length >= 2 &&
         pBestMatch[i + 1].length < MAX_VARLEN &&
         pBestMatch[i + 1].offset &&
         i >= pBestMatch[i + 1].offset &&
         (i + pBestMatch[i + 1].length + 1) <= nEndOffset &&
         !memcmp(pInWindow + i - (pBestMatch[i + 1].offset), pInWindow + i, pBestMatch[i + 1].length + 1)) {
         if ((pBestMatch[i + 1].offset < MINMATCH3_OFFSET || (pBestMatch[i + 1].length + 1) >= 3 || (pBestMatch[i + 1].offset == nRepMatchOffset && nFollowsLiteral)) &&
            (pBestMatch[i + 1].offset < MINMATCH4_OFFSET || (pBestMatch[i + 1].length + 1) >= 4 || (pBestMatch[i + 1].offset == nRepMatchOffset && nFollowsLiteral))) {

            int nCurPartialCommandSize = (pMatch->length == 1) ? (TOKEN_SIZE_4BIT_MATCH + 4) : (1 /* literal bit */ + 8 /* literal size */);
            if (pBestMatch[i + 1].offset == nRepMatchOffset /* always follows a literal, the one at the current position */) {
               nCurPartialCommandSize += apultra_get_rep_offset_varlen_size() + apultra_get_match_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, 1);
            }
            else {
               nCurPartialCommandSize += apultra_get_offset_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, 1) + apultra_get_match_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, 0);
            }

            int nReducedPartialCommandSize;
            if (pBestMatch[i + 1].offset == nRepMatchOffset && nFollowsLiteral) {
               nReducedPartialCommandSize = apultra_get_rep_offset_varlen_size() + apultra_get_match_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, 1);
            }
            else {
               nReducedPartialCommandSize = apultra_get_offset_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, nFollowsLiteral) + apultra_get_match_varlen_size(pBestMatch[i + 1].length, pBestMatch[i + 1].offset, 0);
            }

            if (nReducedPartialCommandSize < nCurPartialCommandSize || (nFollowsLiteral == 0 && nLastMatchLen >= LCP_MAX)) {
               /* Merge */
               pBestMatch[i].length = pBestMatch[i + 1].length + 1;
               pBestMatch[i].offset = pBestMatch[i + 1].offset;
               pBestMatch[i + 1].length = 0;
               pBestMatch[i + 1].offset = 0;
               nDidReduce = 1;
               continue;
            }
         }
      }

      if (pMatch->length >= MIN_MATCH_SIZE) {
         if (pMatch->length < 32 && /* Don't waste time considering large matches, they will always win over literals */
             (i + pMatch->length) < nEndOffset /* Don't consider the last match in the block, we can only reduce a match inbetween other tokens */) {
            int nNextIndex = i + pMatch->length;
            int nNextLiterals = 0;
            int nNextFollowsLiteral = (pMatch->length >= 2) ? 0 : 1;
            int nCannotEncode = 0;

            while (nNextIndex < nEndOffset && pBestMatch[nNextIndex].length < 2) {
               nNextLiterals++;
               nNextIndex++;
               nNextFollowsLiteral = 1;
            }

            if (nNextIndex < nEndOffset && pBestMatch[nNextIndex].length >= 2) {
               if (pMatch->length >= 2 && nRepMatchOffset && nRepMatchOffset != pMatch->offset && pBestMatch[nNextIndex].offset && pMatch->offset != pBestMatch[nNextIndex].offset &&
                  nNextFollowsLiteral) {
                  /* Try to gain a match forward */
                  if (i >= pBestMatch[nNextIndex].offset && (i - pBestMatch[nNextIndex].offset + pMatch->length) <= nEndOffset) {
                     if ((pBestMatch[nNextIndex].offset < MINMATCH3_OFFSET || pMatch->length >= 3) &&
                        (pBestMatch[nNextIndex].offset < MINMATCH4_OFFSET || pMatch->length >= 4)) {
                        int nMaxLen = 0;
                        while (nMaxLen < pMatch->length && pInWindow[i - pBestMatch[nNextIndex].offset + nMaxLen] == pInWindow[i - pMatch->offset + nMaxLen])
                           nMaxLen++;

                        if (nMaxLen >= pMatch->length) {
                           /* Replace */
                           pMatch->offset = pBestMatch[nNextIndex].offset;
                           nDidReduce = 1;
                        }
                        else if (nMaxLen >= 2) {
                           if ((nFollowsLiteral && nRepMatchOffset == pBestMatch[nNextIndex].offset) ||
                              ((pBestMatch[nNextIndex].offset < MINMATCH3_OFFSET || nMaxLen >= 3) &&
                               (pBestMatch[nNextIndex].offset < MINMATCH4_OFFSET || nMaxLen >= 4))) {

                              int nPartialSizeBefore, nPartialSizeAfter, j;

                              nPartialSizeBefore = apultra_get_offset_varlen_size(pMatch->length, pMatch->offset, nFollowsLiteral);
                              nPartialSizeBefore += apultra_get_match_varlen_size(pMatch->length, pMatch->offset, 0);

                              nPartialSizeBefore += apultra_get_offset_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 1);
                              nPartialSizeBefore += apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 0);

                              nPartialSizeAfter = apultra_get_offset_varlen_size(nMaxLen, pBestMatch[nNextIndex].offset, nFollowsLiteral);
                              nPartialSizeAfter += apultra_get_match_varlen_size(nMaxLen, pBestMatch[nNextIndex].offset, (nFollowsLiteral && nRepMatchOffset == pBestMatch[nNextIndex].offset) ? 1 : 0);

                              nPartialSizeAfter += apultra_get_rep_offset_varlen_size();
                              nPartialSizeAfter += apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 1);

                              for (j = nMaxLen; j < pMatch->length; j++) {
                                 if (pInWindow[i + j] == 0 || match1[i + j])
                                    nPartialSizeAfter += TOKEN_SIZE_4BIT_MATCH + 4;
                                 else
                                    nPartialSizeAfter += 1 /* literal bit */ + 8 /* literal byte */;
                              }

                              if (nPartialSizeAfter < nPartialSizeBefore) {
                                 /* We gain a repmatch that is shorter than the original match as this is the best we can do, so it is followed by extra literals, but
                                  * we have calculated that this is shorter */

                                 int nOrigLen = pMatch->length;
                                 int j;

                                 pMatch->offset = pBestMatch[nNextIndex].offset;
                                 pMatch->length = nMaxLen;

                                 for (j = nMaxLen; j < nOrigLen; j++) {
                                    pBestMatch[i + j].offset = match1[i + j];
                                    pBestMatch[i + j].length = (pInWindow[i + j] && match1[i+j] == 0) ? 0 : 1;
                                 }

                                 nDidReduce = 1;
                                 continue;
                              }
                           }
                        }
                     }
                  }
               }

               /* This command is a match, is followed by 'nNextLiterals' literals and then by another match. Calculate this command's current cost (excluding 'nNumLiterals' bytes) */

               int nCurCommandSize = nNumLiterals /* literal flag bits */;
               if (pMatch->offset == nRepMatchOffset && nFollowsLiteral && pMatch->length >= 2) {
                  nCurCommandSize += apultra_get_rep_offset_varlen_size() + apultra_get_match_varlen_size(pMatch->length, pMatch->offset, 1);
               }
               else {
                  nCurCommandSize += apultra_get_offset_varlen_size(pMatch->length, pMatch->offset, nFollowsLiteral) + apultra_get_match_varlen_size(pMatch->length, pMatch->offset, 0);
               }

               /* Calculate the next command's current cost */
               int nNextCommandSize = nNextLiterals /* literal flag bits */ + (nNextLiterals << 3) /* literal bytes */;
               int nCurRepOffset = (pMatch->length >= 2) ? pMatch->offset : nRepMatchOffset;
               if (pBestMatch[nNextIndex].offset == nCurRepOffset && nNextFollowsLiteral && pBestMatch[nNextIndex].length >= 2) {
                  nNextCommandSize += apultra_get_rep_offset_varlen_size() + apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 1);
               }
               else {
                  nNextCommandSize += apultra_get_offset_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, nNextFollowsLiteral) + apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 0);
               }

               int nOriginalCombinedCommandSize = nCurCommandSize + nNextCommandSize;

               /* Calculate the cost of replacing this match command by literals + the next command with the cost of encoding these literals (excluding 'nNumLiterals' bytes) */
               int nReducedFollowsLiteral = (nNumLiterals + pMatch->length) ? 1 : 0;
               int nReducedCommandSize = nNumLiterals + nNextLiterals /* literal flag bits */ + (nNextLiterals << 3) /* literal bytes */;
               int j;

               for (j = 0; j < pMatch->length; j++) {
                  if (pInWindow[i + j] == 0 || match1[i + j])
                     nReducedCommandSize += TOKEN_SIZE_4BIT_MATCH + 4;
                  else
                     nReducedCommandSize += 1 /* literal bit */ + 8;
               }

               if (pBestMatch[nNextIndex].offset == nRepMatchOffset && nReducedFollowsLiteral && pBestMatch[nNextIndex].length >= 2) {
                  nReducedCommandSize += apultra_get_rep_offset_varlen_size() + apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 1);
               }
               else {
                  if ((pBestMatch[nNextIndex].length < 3 && pBestMatch[nNextIndex].offset >= MINMATCH3_OFFSET) ||
                     (pBestMatch[nNextIndex].length < 4 && pBestMatch[nNextIndex].offset >= MINMATCH4_OFFSET)) {
                     /* This match length can only be encoded with a rep-match */
                     nCannotEncode = 1;
                  }
                  else {
                     nReducedCommandSize += apultra_get_offset_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, nReducedFollowsLiteral) + apultra_get_match_varlen_size(pBestMatch[nNextIndex].length, pBestMatch[nNextIndex].offset, 0);
                  }
               }

               if (!nCannotEncode && nOriginalCombinedCommandSize > nReducedCommandSize) {
                  /* Reduce */
                  int nMatchLen = pMatch->length;
                  int j;

                  for (j = 0; j < nMatchLen; j++) {
                     pBestMatch[i + j].offset = match1[i + j];
                     pBestMatch[i + j].length = (pInWindow[i + j] && match1[i + j] == 0) ? 0 : 1;
                  }

                  nDidReduce = 1;
                  continue;
               }
            }
         }

         if ((i + pMatch->length) < nEndOffset && pMatch->offset > 0 && pMatch->length >= 2 &&
            pBestMatch[i + pMatch->length].offset > 0 &&
            pBestMatch[i + pMatch->length].length >= 2 &&
            (pMatch->length + pBestMatch[i + pMatch->length].length) >= LEAVE_ALONE_MATCH_SIZE &&
            (pMatch->length + pBestMatch[i + pMatch->length].length) <= MAX_VARLEN &&
            (i + pMatch->length) >= pMatch->offset &&
            (i + pMatch->length) >= pBestMatch[i + pMatch->length].offset &&
            (i + pMatch->length + pBestMatch[i + pMatch->length].length) <= nEndOffset &&
            !memcmp(pInWindow + i + pMatch->length - pMatch->offset,
               pInWindow + i + pMatch->length - pBestMatch[i + pMatch->length].offset,
               pBestMatch[i + pMatch->length].length)) {
            int nMatchLen = pMatch->length;

            /* Join large matches */

            int nNextIndex = i + pMatch->length + pBestMatch[i + pMatch->length].length;
            int nNextFollowsLiteral = 0;
            int nCannotEncode = 0;

            while (nNextIndex < nEndOffset && pBestMatch[nNextIndex].length < 2) {
               nNextIndex++;
               nNextFollowsLiteral = 1;
            }

            if (nNextIndex < nEndOffset && nNextFollowsLiteral && pBestMatch[nNextIndex].length >= 2 &&
               pBestMatch[nNextIndex].offset == pBestMatch[i + pMatch->length].offset) {
               if ((pBestMatch[nNextIndex].offset >= MINMATCH3_OFFSET && pBestMatch[nNextIndex].length < 3) ||
                  (pBestMatch[nNextIndex].offset >= MINMATCH4_OFFSET && pBestMatch[nNextIndex].length < 4)) {
                  nCannotEncode = 1;
               }
            }

            if (!nCannotEncode) {
               pMatch->length += pBestMatch[i + nMatchLen].length;
               pBestMatch[i + nMatchLen].offset = 0;
               pBestMatch[i + nMatchLen].length = -1;
               nDidReduce = 1;
               continue;
            }
         }

         if (pMatch->offset == nRepMatchOffset && nFollowsLiteral && pMatch->length >= 2) {
            /* Rep-match */
            nRepMatchOffset = pMatch->offset;
            nFollowsLiteral = 0;
            nLastMatchLen = pMatch->length;
         }
         else {
            if (pMatch->length == 1 && pMatch->offset < 16) {
               /* 4 bits offset */
               nFollowsLiteral = 1;
               nLastMatchLen = 0;
            }
            else if (pMatch->length <= 3 && pMatch->offset < 128) {
               /* 7 bits offset + 1 bit length */
               nRepMatchOffset = pMatch->offset;
               nFollowsLiteral = 0;
               nLastMatchLen = pMatch->length;
            }
            else {
               /* 8+n bits offset */
               nRepMatchOffset = pMatch->offset;
               nFollowsLiteral = 0;
               nLastMatchLen = pMatch->length;
            }
         }

         i += pMatch->length;
         nNumLiterals = 0;
      }
      else {
         nNumLiterals++;
         i++;
         nFollowsLiteral = 1;
         nLastMatchLen = 0;
      }
   }

   return nDidReduce;
}

/**
 * Emit a block of compressed data
 *
 * @param pCompressor compression context
 * @param pBestMatch optimal matches to emit
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param pOutData pointer to output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 * @param nFollowsLiteral non-zero if the next command to be issued follows a literal, 0 if not
 * @param nCurRepMatchOffset starting rep offset for this block, updated after the block is compressed successfully
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 *
 * @return size of compressed data in output buffer, or -1 if the data is uncompressible
 */
static int apultra_write_block(apultra_compressor *pCompressor, apultra_final_match *pBestMatch, const unsigned char *pInWindow, const int nStartOffset, const int nEndOffset, unsigned char *pOutData, int nOutOffset, const int nMaxOutDataSize, int *nCurBitsOffset, int *nCurBitMask, int *nFollowsLiteral, int *nCurRepMatchOffset, const int nBlockFlags) {
   int i, j;
   int nRepMatchOffset = *nCurRepMatchOffset;

   if (nBlockFlags & 1) {
      if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
         return -1;
      pOutData[nOutOffset++] = pInWindow[nStartOffset];
      *nFollowsLiteral = 1;
   }

   for (i = nStartOffset + ((nBlockFlags & 1) ? 1 : 0); i < nEndOffset; ) {
      const apultra_final_match *pMatch = pBestMatch + i;

      if (pMatch->length >= MIN_MATCH_SIZE) {
         int nMatchOffset = pMatch->offset;
         int nMatchLen = pMatch->length;
         int nTokenOffsetMode;
         int nOffsetSize;

         if (nMatchOffset == nRepMatchOffset && *nFollowsLiteral && nMatchLen >= 2) {
            /* Rep-match */
            nTokenOffsetMode = 3;
            nOffsetSize = TOKEN_SIZE_LARGE_MATCH + apultra_get_gamma2_size(2);
         }
         else {
            if (nMatchLen == 1 && nMatchOffset < 16) {
               /* 4 bits offset */
               nTokenOffsetMode = 2;
               nOffsetSize = 4 + TOKEN_SIZE_4BIT_MATCH;
            }
            else if (nMatchLen <= 3 && nMatchOffset < 128) {
               /* 7 bits offset + 1 bit length */
               nTokenOffsetMode = 1;
               nOffsetSize = 8 + TOKEN_SIZE_7BIT_MATCH;
            }
            else {
               /* 8+n bits offset */
               nTokenOffsetMode = 0;
               if (*nFollowsLiteral)
                  nOffsetSize = 8 + TOKEN_SIZE_LARGE_MATCH + apultra_get_gamma2_size((nMatchOffset >> 8) + 3);
               else
                  nOffsetSize = 8 + TOKEN_SIZE_LARGE_MATCH + apultra_get_gamma2_size((nMatchOffset >> 8) + 2);
            }
         }

         int nCommandSize = nOffsetSize /* match offset */ + apultra_get_match_varlen_size(nMatchLen, nMatchOffset, (nTokenOffsetMode == 3) ? 1 : 0);

         if ((nOutOffset + ((nCommandSize + 7) >> 3)) > nMaxOutDataSize)
            return -1;
         if (nMatchOffset < ((nMatchLen == 1) ? 0 : MIN_OFFSET) || nMatchOffset > MAX_OFFSET)
            return -1;

         int nActualTokenOffsetMode = nTokenOffsetMode;
         if (nActualTokenOffsetMode == 3)
            nActualTokenOffsetMode = 0;
         for (j = _token_size[nActualTokenOffsetMode] - 1; j >= 0; j--)
            nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (_token_code[nActualTokenOffsetMode] & (1 << j)) ? 1 : 0, nCurBitsOffset, nCurBitMask);

         int nEmitMatchLength = 0;

         if (nTokenOffsetMode == 0) {
            /* 8+n bits offset */

            if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
               return -1;
            if (*nFollowsLiteral)
               nOutOffset = apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset >> 8) + 3, nCurBitsOffset, nCurBitMask);
            else
               nOutOffset = apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset >> 8) + 2, nCurBitsOffset, nCurBitMask);
            pOutData[nOutOffset++] = nMatchOffset & 0xff;

            if (nMatchOffset < 128 && nMatchLen <= 3) {
               /* A shorter match must have been encoded as a 4 bits offset or a 7 bits offset + 1 bit match length command */
               return -1;
            }

            nEmitMatchLength = 1;
            *nFollowsLiteral = 0;
            nRepMatchOffset = nMatchOffset;

            pCompressor->stats.num_variable_matches++;
         }
         else if (nTokenOffsetMode == 1) {
            /* 7 bits offset + 1 bit length */

            if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
               return -1;
            pOutData[nOutOffset++] = ((nMatchOffset) & 0x7f) << 1 | (nMatchLen - 2);

            *nFollowsLiteral = 0;
            nRepMatchOffset = nMatchOffset;

            pCompressor->stats.num_7bit_matches++;
         }
         else if (nTokenOffsetMode == 2) {
            /* 4 bits offset */

            nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset & 0x08) ? 1 : 0, nCurBitsOffset, nCurBitMask);
            nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset & 0x04) ? 1 : 0, nCurBitsOffset, nCurBitMask);
            nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset & 0x02) ? 1 : 0, nCurBitsOffset, nCurBitMask);
            nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (nMatchOffset & 0x01) ? 1 : 0, nCurBitsOffset, nCurBitMask);
            if (nOutOffset < 0) return -1;

            *nFollowsLiteral = 1;

            pCompressor->stats.num_4bit_matches++;
         }
         else {
            /* rep match */
            nOutOffset = apultra_write_gamma2_value(pOutData, nOutOffset, nMaxOutDataSize, 2, nCurBitsOffset, nCurBitMask);

            nEmitMatchLength = 1;
            *nFollowsLiteral = 0;

            pCompressor->stats.num_rep_matches++;
         }

         if (nEmitMatchLength) {
            /* The match length isn't encoded in the command, emit elias gamma value */
            nOutOffset = apultra_write_match_varlen(pOutData, nOutOffset, nMaxOutDataSize, nCurBitsOffset, nCurBitMask, nMatchLen, nMatchOffset, (nTokenOffsetMode == 3) ? 1 : 0);
            if (nOutOffset < 0) return -1;
         }

         if (nMatchOffset < pCompressor->stats.min_offset || pCompressor->stats.min_offset == -1)
            pCompressor->stats.min_offset = nMatchOffset;
         if (nMatchOffset > pCompressor->stats.max_offset)
            pCompressor->stats.max_offset = nMatchOffset;
         pCompressor->stats.total_offsets += (long long)nMatchOffset;

         if (nMatchLen < pCompressor->stats.min_match_len || pCompressor->stats.min_match_len == -1)
            pCompressor->stats.min_match_len = nMatchLen;
         if (nMatchLen > pCompressor->stats.max_match_len)
            pCompressor->stats.max_match_len = nMatchLen;
         pCompressor->stats.total_match_lens += nMatchLen;
         pCompressor->stats.match_divisor++;

         if (nMatchOffset == 1) {
            if (nMatchLen < pCompressor->stats.min_rle1_len || pCompressor->stats.min_rle1_len == -1)
               pCompressor->stats.min_rle1_len = nMatchLen;
            if (nMatchLen > pCompressor->stats.max_rle1_len)
               pCompressor->stats.max_rle1_len = nMatchLen;
            pCompressor->stats.total_rle1_lens += nMatchLen;
            pCompressor->stats.rle1_divisor++;
         }
         else if (nMatchOffset == 2) {
            if (nMatchLen < pCompressor->stats.min_rle2_len || pCompressor->stats.min_rle2_len == -1)
               pCompressor->stats.min_rle2_len = nMatchLen;
            if (nMatchLen > pCompressor->stats.max_rle2_len)
               pCompressor->stats.max_rle2_len = nMatchLen;
            pCompressor->stats.total_rle2_lens += nMatchLen;
            pCompressor->stats.rle2_divisor++;
         }

         i += nMatchLen;

         pCompressor->stats.commands_divisor++;
      }
      else {
         nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, 0 /* literal */, nCurBitsOffset, nCurBitMask);

         if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
            return -1;
         pOutData[nOutOffset++] = pInWindow[i];

         pCompressor->stats.num_literals++;
         pCompressor->stats.commands_divisor++;
         i++;
         *nFollowsLiteral = 1;
      }

      int nCurSafeDist = (i - nStartOffset) - nOutOffset;
      if (nCurSafeDist >= 0 && pCompressor->stats.safe_dist < nCurSafeDist)
         pCompressor->stats.safe_dist = nCurSafeDist;
   }

   if (nBlockFlags & 2) {
      /* 8 bits offset */

      for (j = TOKEN_SIZE_7BIT_MATCH - 1; j >= 0; j--)
         nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (_token_code[1] & (1 << j)) ? 1 : 0, nCurBitsOffset, nCurBitMask);

      if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
         return -1;
      pOutData[nOutOffset++] = 0x00;   /* Offset: EOD */
      pCompressor->stats.commands_divisor++;

      int nCurSafeDist = (i - nStartOffset) - nOutOffset;
      if (nCurSafeDist >= 0 && pCompressor->stats.safe_dist < nCurSafeDist)
         pCompressor->stats.safe_dist = nCurSafeDist;
   }

   *nCurRepMatchOffset = nRepMatchOffset;
   return nOutOffset;
}

/**
 * Emit raw block of uncompressible data
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nStartOffset current offset in input window (typically the number of previously compressed bytes)
 * @param nEndOffset offset to end finding matches at (typically the size of the total input window in bytes
 * @param pOutData pointer to output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 * @param nFollowsLiteral non-zero if the next command to be issued follows a literal, 0 if not
 *
 * @return size of compressed data in output buffer, or -1 if the data is uncompressible
 */
static int apultra_write_raw_uncompressed_block_v3(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nStartOffset, const int nEndOffset, unsigned char *pOutData, int nOutOffset, const int nMaxOutDataSize, const int nBlockFlags, int *nCurBitsOffset, int *nCurBitMask, int *nFollowsLiteral) {
   int nNumLiterals = nEndOffset - nStartOffset;
   int j;
   int nInOffset = nStartOffset;

   int nCommandSize = nNumLiterals /* literal flag bits */ + (nNumLiterals << 3) /* literal bytes */ + TOKEN_SIZE_7BIT_MATCH /* token */ + 8 /* match offset */;
   if ((nOutOffset + ((nCommandSize + 7) >> 3)) > nMaxOutDataSize)
      return -1;

   pCompressor->stats.commands_divisor = 0;
   *nFollowsLiteral = 1;

   for (j = 0; j < nNumLiterals; j++) {
      nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, 0 /* literal */, nCurBitsOffset, nCurBitMask);
      pOutData[nOutOffset++] = pInWindow[nInOffset + j];
   }

   nNumLiterals = 0;

   /* 8 bits offset */
   for (j = TOKEN_SIZE_7BIT_MATCH - 1; j >= 0; j--)
      nOutOffset = apultra_write_bit(pOutData, nOutOffset, nMaxOutDataSize, (_token_code[1] & (1 << j)) ? 1 : 0, nCurBitsOffset, nCurBitMask);

   if (nOutOffset < 0 || nOutOffset >= nMaxOutDataSize)
      return -1;
   pOutData[nOutOffset++] = 0x00;   /* Offset: EOD */

   pCompressor->stats.commands_divisor++;

   return nOutOffset;
}

/**
 * Select the most optimal matches, reduce the token count if possible, and then emit a block of compressed data
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nPreviousBlockSize number of previously compressed bytes (or 0 for none)
 * @param nInDataSize number of input bytes to compress
 * @param pOutData pointer to output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 * @param nCurFollowsLiteral non-zero if the next command to be issued follows a literal, 0 if not
 * @param nCurRepMatchOffset starting rep offset for this block, updated after the block is compressed successfully
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 *
 * @return size of compressed data in output buffer, or -1 if the data is uncompressible
 */
static int apultra_optimize_and_write_block(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nPreviousBlockSize, const int nInDataSize, unsigned char *pOutData, const int nMaxOutDataSize, int *nCurBitsOffset, int *nCurBitMask, int *nCurFollowsLiteral, int *nCurRepMatchOffset, const int nBlockFlags) {
   int nResult;
   int nOutOffset = 0;
   const int nMatchesPerArrival = ((nBlockFlags & 3) == 3) ? NMATCHES_PER_ARRIVAL : NMATCHES_PER_ARRIVAL_SMALL;
   int *first_offset_for_byte = pCompressor->first_offset_for_byte;
   int *next_offset_for_pos = pCompressor->next_offset_for_pos;
   int nPosition;

   memset(pCompressor->best_match, 0, pCompressor->block_size * sizeof(apultra_final_match));

   if ((nBlockFlags & 3) == 3) {
      /* Supplement 2-byte matches */

      memset(first_offset_for_byte, 0xff, sizeof(int) * 256);
      memset(next_offset_for_pos, 0xff, sizeof(int) * nInDataSize);

      for (nPosition = nPreviousBlockSize; nPosition < nPreviousBlockSize + nInDataSize; nPosition++) {
         next_offset_for_pos[nPosition - nPreviousBlockSize] = first_offset_for_byte[pInWindow[nPosition]];
         first_offset_for_byte[pInWindow[nPosition]] = nPosition;
      }

      for (nPosition = nPreviousBlockSize + 2; nPosition < (nPreviousBlockSize + nInDataSize - 1); nPosition++) {
         apultra_match *match = pCompressor->match + ((nPosition - nPreviousBlockSize) << MATCHES_PER_INDEX_SHIFT);
         unsigned short *match_depth = pCompressor->match_depth + ((nPosition - nPreviousBlockSize) << MATCHES_PER_INDEX_SHIFT);
         int m = 0;
         int nMatchPos;

         while (m < NMATCHES_PER_INDEX && match[m].length)
            m++;

         for (nMatchPos = next_offset_for_pos[nPosition - nPreviousBlockSize]; m < 8 && nMatchPos >= 0; nMatchPos = next_offset_for_pos[nMatchPos - nPreviousBlockSize]) {
            if (pInWindow[nMatchPos + 1] == pInWindow[nPosition + 1]) {
               int nMatchOffset = nPosition - nMatchPos;
               int nExistingMatchIdx;
               int nAlreadyExists = 0;

               for (nExistingMatchIdx = 0; nExistingMatchIdx < m; nExistingMatchIdx++) {
                  if (match[nExistingMatchIdx].offset == nMatchOffset) {
                     nAlreadyExists = 1;
                     break;
                  }
               }

               if (!nAlreadyExists) {
                  match[m].length = 2;
                  match[m].offset = nMatchOffset;
                  match_depth[m] = 0;
                  break;
               }
            }
         }
      }
   }

   apultra_optimize_forward(pCompressor, pInWindow, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, 1 /* nInsertForwardReps */, nCurRepMatchOffset, nBlockFlags, nMatchesPerArrival);

   /* Pick optimal matches */
   apultra_optimize_forward(pCompressor, pInWindow, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, 0 /* nInsertForwardReps */, nCurRepMatchOffset, nBlockFlags, nMatchesPerArrival);

   /* Apply reduction and merge pass */
   int nDidReduce;
   int nPasses = 0;
   do {
      nDidReduce = apultra_reduce_commands(pCompressor, pInWindow, pCompressor->best_match - nPreviousBlockSize, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, nCurRepMatchOffset);
      nPasses++;
   } while (nDidReduce && nPasses < 20);

   /* Write compressed block */

   nResult = apultra_write_block(pCompressor, pCompressor->best_match - nPreviousBlockSize, pInWindow, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, pOutData, nOutOffset, nMaxOutDataSize, nCurBitsOffset, nCurBitMask, nCurFollowsLiteral, nCurRepMatchOffset, nBlockFlags);
   if (nResult < 0) {
      /* Try to write block as all literals */
      *nCurRepMatchOffset = 0;
      nResult = apultra_write_raw_uncompressed_block_v3(pCompressor, pInWindow, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, pOutData, nOutOffset, nMaxOutDataSize, nBlockFlags, nCurBitsOffset, nCurBitMask, nCurFollowsLiteral);
   }

   return nResult;
}

/* Forward declaration */
static void apultra_compressor_destroy(apultra_compressor *pCompressor);

/**
 * Initialize compression context
 *
 * @param pCompressor compression context to initialize
 * @param nBlockSize maximum size of input data (bytes to compress only)
 * @param nMaxWindowSize maximum size of input data window (previously compressed bytes + bytes to compress)
 * @param nFlags compression flags
 *
 * @return 0 for success, non-zero for failure
 */
static int apultra_compressor_init(apultra_compressor *pCompressor, const int nBlockSize, const int nMaxWindowSize, const int nFlags) {
   int nResult;

   nResult = divsufsort_init(&pCompressor->divsufsort_context);
   pCompressor->intervals = NULL;
   pCompressor->pos_data = NULL;
   pCompressor->open_intervals = NULL;
   pCompressor->match = NULL;
   pCompressor->match_depth = NULL;
   pCompressor->match1 = NULL;
   pCompressor->best_match = NULL;
   pCompressor->arrival = NULL;
   pCompressor->first_offset_for_byte = NULL;
   pCompressor->next_offset_for_pos = NULL;
   pCompressor->flags = nFlags;
   pCompressor->block_size = nBlockSize;

   memset(&pCompressor->stats, 0, sizeof(pCompressor->stats));
   pCompressor->stats.min_match_len = -1;
   pCompressor->stats.min_offset = -1;
   pCompressor->stats.min_rle1_len = -1;
   pCompressor->stats.min_rle2_len = -1;

   if (!nResult) {
      pCompressor->intervals = (unsigned long long *)malloc(nMaxWindowSize * sizeof(unsigned long long));

      if (pCompressor->intervals) {
         pCompressor->pos_data = (unsigned long long *)malloc(nMaxWindowSize * sizeof(unsigned long long));

         if (pCompressor->pos_data) {
            pCompressor->open_intervals = (unsigned long long *)malloc((LCP_AND_TAG_MAX + 1) * sizeof(unsigned long long));

            if (pCompressor->open_intervals) {
               pCompressor->arrival = (apultra_arrival *)malloc((nBlockSize + 1) * NMATCHES_PER_ARRIVAL * sizeof(apultra_arrival));

               if (pCompressor->arrival) {
                  pCompressor->best_match = (apultra_final_match *)malloc(nBlockSize * sizeof(apultra_final_match));

                  if (pCompressor->best_match) {
                     pCompressor->match = (apultra_match *)malloc(nBlockSize * NMATCHES_PER_INDEX * sizeof(apultra_match));
                     if (pCompressor->match) {
                        pCompressor->match_depth = (unsigned short *)malloc(nBlockSize * NMATCHES_PER_INDEX * sizeof(unsigned short));
                        if (pCompressor->match_depth) {
                           pCompressor->match1 = (unsigned char *)malloc(nBlockSize * sizeof(unsigned char));
                           if (pCompressor->match1) {
                              pCompressor->first_offset_for_byte = (int*)malloc(256 * sizeof(int));
                              if (pCompressor->first_offset_for_byte) {
                                 pCompressor->next_offset_for_pos = (int*)malloc(nBlockSize * sizeof(int));
                                 if (pCompressor->next_offset_for_pos) {
                                    return 0;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   apultra_compressor_destroy(pCompressor);
   return 100;
}

/**
 * Clean up compression context and free up any associated resources
 *
 * @param pCompressor compression context to clean up
 */
static void apultra_compressor_destroy(apultra_compressor *pCompressor) {
   divsufsort_destroy(&pCompressor->divsufsort_context);

   if (pCompressor->next_offset_for_pos) {
      free(pCompressor->next_offset_for_pos);
      pCompressor->next_offset_for_pos = NULL;
   }

   if (pCompressor->first_offset_for_byte) {
      free(pCompressor->first_offset_for_byte);
      pCompressor->first_offset_for_byte = NULL;
   }

   if (pCompressor->match1) {
      free(pCompressor->match1);
      pCompressor->match1 = NULL;
   }

   if (pCompressor->match_depth) {
      free(pCompressor->match_depth);
      pCompressor->match_depth = NULL;
   }

   if (pCompressor->match) {
      free(pCompressor->match);
      pCompressor->match = NULL;
   }

   if (pCompressor->arrival) {
      free(pCompressor->arrival);
      pCompressor->arrival = NULL;
   }

   if (pCompressor->best_match) {
      free(pCompressor->best_match);
      pCompressor->best_match = NULL;
   }

   if (pCompressor->open_intervals) {
      free(pCompressor->open_intervals);
      pCompressor->open_intervals = NULL;
   }

   if (pCompressor->pos_data) {
      free(pCompressor->pos_data);
      pCompressor->pos_data = NULL;
   }

   if (pCompressor->intervals) {
      free(pCompressor->intervals);
      pCompressor->intervals = NULL;
   }
}

/**
 * Compress one block of data
 *
 * @param pCompressor compression context
 * @param pInWindow pointer to input data window (previously compressed bytes + bytes to compress)
 * @param nPreviousBlockSize number of previously compressed bytes (or 0 for none)
 * @param nInDataSize number of input bytes to compress
 * @param pOutData pointer to output buffer
 * @param nMaxOutDataSize maximum size of output buffer, in bytes
 * @param nCurBitsOffset write index into output buffer, of current byte being filled with bits
 * @param nCurBitMask bit shifter
 * @param nCurFollowsLiteral non-zero if the next command to be issued follows a literal, 0 if not
 * @param nCurRepMatchOffset starting rep offset for this block, updated after the block is compressed successfully
 * @param nBlockFlags bit 0: 1 for first block, 0 otherwise; bit 1: 1 for last block, 0 otherwise
 *
 * @return size of compressed data in output buffer, or -1 if the data is uncompressible
 */
static int apultra_compressor_shrink_block(apultra_compressor *pCompressor, const unsigned char *pInWindow, const int nPreviousBlockSize, const int nInDataSize, unsigned char *pOutData, const int nMaxOutDataSize, int *nCurBitsOffset, int *nCurBitMask, int *nCurFollowsLiteral, int *nCurRepMatchOffset, const int nBlockFlags) {
   int nCompressedSize;

   if (apultra_build_suffix_array(pCompressor, pInWindow, nPreviousBlockSize + nInDataSize))
      nCompressedSize = -1;
   else {
      if (nPreviousBlockSize) {
         apultra_skip_matches(pCompressor, 0, nPreviousBlockSize);
      }
      apultra_find_all_matches(pCompressor, NMATCHES_PER_INDEX, nPreviousBlockSize, nPreviousBlockSize + nInDataSize, nBlockFlags);

      nCompressedSize = apultra_optimize_and_write_block(pCompressor, pInWindow, nPreviousBlockSize, nInDataSize, pOutData, nMaxOutDataSize, nCurBitsOffset, nCurBitMask, nCurFollowsLiteral, nCurRepMatchOffset, nBlockFlags);
   }

   return nCompressedSize;
}

/**
 * Get maximum compressed size of input(source) data
 *
 * @param nInputSize input(source) size in bytes
 *
 * @return maximum compressed size
 */
size_t apultra_get_max_compressed_size(size_t nInputSize) {
   return ((nInputSize * 9 /* literals + literal bits */ + 1 /* match bit */ + 2 /* 7+1 command bits */ + 8 /* EOD offset bits */) + 7) >> 3;
}

/**
 * Compress memory
 *
 * @param pInputData pointer to input(source) data to compress
 * @param pOutBuffer buffer for compressed data
 * @param nInputSize input(source) size in bytes
 * @param nMaxOutBufferSize maximum capacity of compression buffer
 * @param nFlags compression flags (set to 0)
 * @param progress progress function, called after compressing each block, or NULL for none
 * @param pStats pointer to compression stats that are filled if this function is successful, or NULL
 * @param nMaxWindowSize maximum window size to use (0 for default)
 *
 * @return actual compressed size, or -1 for error
 */
size_t apultra_compress(const unsigned char *pInputData, unsigned char *pOutBuffer, size_t nInputSize, size_t nMaxOutBufferSize,
      const unsigned int nFlags, size_t nMaxWindowSize, void(*progress)(long long nOriginalSize, long long nCompressedSize), apultra_stats *pStats) {
   apultra_compressor compressor;
   size_t nOriginalSize = 0;
   size_t nCompressedSize = 0L;
   int nResult;
   int nError = 0;
   const int nDefaultBlockSize = (nInputSize < BLOCK_SIZE) ? ((nInputSize < 1024) ? 1024 : (int)nInputSize) : BLOCK_SIZE;
   const int nBlockSize = nMaxWindowSize ? ((nDefaultBlockSize < nMaxWindowSize / 2) ? nDefaultBlockSize : (int)nMaxWindowSize / 2) : nDefaultBlockSize;
   const int nMaxOutBlockSize = (int)apultra_get_max_compressed_size(nBlockSize);

   nResult = apultra_compressor_init(&compressor, nBlockSize, nBlockSize * 2, nFlags);
   if (nResult != 0) {
      return -1;
   }

   int nPreviousBlockSize = 0;
   int nNumBlocks = 0;
   int nCurBitsOffset = INT_MIN, nCurBitMask = 0, nCurFollowsLiteral = 0;
   int nBlockFlags = 1;
   int nCurRepMatchOffset = 0;

   while (nOriginalSize < nInputSize && !nError) {
      int nInDataSize;

      nInDataSize = (int)(nInputSize - nOriginalSize);
      if (nInDataSize > nBlockSize)
         nInDataSize = nBlockSize;

      if (nInDataSize > 0) {
         int nOutDataSize;
         int nOutDataEnd = (int)(nMaxOutBufferSize - nCompressedSize);

         if (nOutDataEnd > nMaxOutBlockSize)
            nOutDataEnd = nMaxOutBlockSize;

         if ((nOriginalSize + nInDataSize) >= nInputSize)
            nBlockFlags |= 2;
         nOutDataSize = apultra_compressor_shrink_block(&compressor, pInputData + nOriginalSize - nPreviousBlockSize, nPreviousBlockSize, nInDataSize, pOutBuffer + nCompressedSize, nOutDataEnd,
            &nCurBitsOffset, &nCurBitMask, &nCurFollowsLiteral, &nCurRepMatchOffset, nBlockFlags);
         nBlockFlags &= (~1);

         if (nOutDataSize >= 0) {
            /* Write compressed block */

            if (!nError) {
               nOriginalSize += nInDataSize;
               nCompressedSize += nOutDataSize;
               if (nCurBitsOffset != INT_MIN)
                  nCurBitsOffset -= nOutDataSize;
            }
         }
         else {
            nError = -1;
         }

         nPreviousBlockSize = nInDataSize;
         nNumBlocks++;
      }

      if (!nError && nOriginalSize < nInputSize) {
         if (progress)
            progress(nOriginalSize, nCompressedSize);
      }
   }

   if (progress)
      progress(nOriginalSize, nCompressedSize);
   if (pStats)
      *pStats = compressor.stats;

   apultra_compressor_destroy(&compressor);

   if (nError) {
      return -1;
   }
   else {
      return nCompressedSize;
   }
}
/*
 * divsufsort.c for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef _OPENMP
# include <omp.h>
#endif


/*- Private Functions -*/

/* Sorts suffixes of type B*. */
static
saidx_t
sort_typeBstar(const sauchar_t *T, saidx_t *SA,
               saidx_t *bucket_A, saidx_t *bucket_B,
               saidx_t n) {
  saidx_t *PAb, *ISAb, *buf;
#ifdef _OPENMP
  saidx_t *curbuf;
  saidx_t l;
#endif
  saidx_t i, j, k, t, m, bufsize;
  saint_t c0, c1;
#ifdef _OPENMP
  saint_t d0, d1;
  int tmp;
#endif

  /* Initialize bucket arrays. */
  for(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  for(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  /* Count the number of occurrences of the first one or two characters of each
     type A, B and B* suffix. Moreover, store the beginning position of all
     type B* suffixes into the array SA. */
  for(i = n - 1, m = n, c0 = T[n - 1]; 0 <= i;) {
    /* type A suffix. */
    do { ++BUCKET_A(c1 = c0); } while((0 <= --i) && ((c0 = T[i]) >= c1));
    if(0 <= i) {
      /* type B* suffix. */
      ++BUCKET_BSTAR(c0, c1);
      SA[--m] = i;
      /* type B suffix. */
      for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) {
        ++BUCKET_B(c0, c1);
      }
    }
  }
  m = n - m;
/*
note:
  A type B* suffix is lexicographically smaller than a type B suffix that
  begins with the same first two characters.
*/

  /* Calculate the index of start/end point of each bucket. */
  for(c0 = 0, i = 0, j = 0; c0 < ALPHABET_SIZE; ++c0) {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; /* start point */
    i = t + BUCKET_B(c0, c0);
    for(c1 = c0 + 1; c1 < ALPHABET_SIZE; ++c1) {
      j += BUCKET_BSTAR(c0, c1);
      BUCKET_BSTAR(c0, c1) = j; /* end point */
      i += BUCKET_B(c0, c1);
    }
  }

  if(0 < m) {
    /* Sort the type B* suffixes by their first two characters. */
    PAb = SA + n - m; ISAb = SA + m;
    for(i = m - 2; 0 <= i; --i) {
      t = PAb[i], c0 = T[t], c1 = T[t + 1];
      SA[--BUCKET_BSTAR(c0, c1)] = i;
    }
    t = PAb[m - 1], c0 = T[t], c1 = T[t + 1];
    SA[--BUCKET_BSTAR(c0, c1)] = m - 1;

    /* Sort the type B* substrings using sssort. */
#ifdef _OPENMP
    tmp = omp_get_max_threads();
    buf = SA + m, bufsize = (n - (2 * m)) / tmp;
    c0 = ALPHABET_SIZE - 2, c1 = ALPHABET_SIZE - 1, j = m;
#pragma omp parallel default(shared) private(curbuf, k, l, d0, d1, tmp)
    {
      tmp = omp_get_thread_num();
      curbuf = buf + tmp * bufsize;
      k = 0;
      for(;;) {
        #pragma omp critical(sssort_lock)
        {
          if(0 < (l = j)) {
            d0 = c0, d1 = c1;
            do {
              k = BUCKET_BSTAR(d0, d1);
              if(--d1 <= d0) {
                d1 = ALPHABET_SIZE - 1;
                if(--d0 < 0) { break; }
              }
            } while(((l - k) <= 1) && (0 < (l = k)));
            c0 = d0, c1 = d1, j = k;
          }
        }
        if(l == 0) { break; }
        sssort(T, PAb, SA + k, SA + l,
               curbuf, bufsize, 2, n, *(SA + k) == (m - 1));
      }
    }
#else
    buf = SA + m, bufsize = n - (2 * m);
    for(c0 = ALPHABET_SIZE - 2, j = m; 0 < j; --c0) {
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; j = i, --c1) {
        i = BUCKET_BSTAR(c0, c1);
        if(1 < (j - i)) {
          sssort(T, PAb, SA + i, SA + j,
                 buf, bufsize, 2, n, *(SA + i) == (m - 1));
        }
      }
    }
#endif

    /* Compute ranks of type B* substrings. */
    for(i = m - 1; 0 <= i; --i) {
      if(0 <= SA[i]) {
        j = i;
        do { ISAb[SA[i]] = i; } while((0 <= --i) && (0 <= SA[i]));
        SA[i + 1] = i - j;
        if(i <= 0) { break; }
      }
      j = i;
      do { ISAb[SA[i] = ~SA[i]] = j; } while(SA[--i] < 0);
      ISAb[SA[i]] = j;
    }

    /* Construct the inverse suffix array of type B* suffixes using trsort. */
    trsort(ISAb, SA, m, 1);

    /* Set the sorted order of tyoe B* suffixes. */
    for(i = n - 1, j = m, c0 = T[n - 1]; 0 <= i;) {
      for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) >= c1); --i, c1 = c0) { }
      if(0 <= i) {
        t = i;
        for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) { }
        SA[ISAb[--j]] = ((t == 0) || (1 < (t - i))) ? t : ~t;
      }
    }

    /* Calculate the index of start/end point of each bucket. */
    BUCKET_B(ALPHABET_SIZE - 1, ALPHABET_SIZE - 1) = n; /* end point */
    for(c0 = ALPHABET_SIZE - 2, k = m - 1; 0 <= c0; --c0) {
      i = BUCKET_A(c0 + 1) - 1;
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i; /* end point */

        /* Move all type B* suffixes to the correct position. */
        for(i = t, j = BUCKET_BSTAR(c0, c1);
            j <= k;
            --i, --k) { SA[i] = SA[k]; }
      }
      BUCKET_BSTAR(c0, c0 + 1) = i - BUCKET_B(c0, c0) + 1; /* start point */
      BUCKET_B(c0, c0) = i; /* end point */
    }
  }

  return m;
}

/* Constructs the suffix array by using the sorted order of type B* suffixes. */
static
void
construct_SA(const sauchar_t *T, saidx_t *SA,
             saidx_t *bucket_A, saidx_t *bucket_B,
             saidx_t n, saidx_t m) {
  saidx_t *i, *j, *k;
  saidx_t s;
  saint_t c0, c1, c2;

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 < (s = *j)) {
          assert(T[s] == c1);
          assert(((s + 1) < n) && (T[s] <= T[s + 1]));
          assert(T[s - 1] <= T[s]);
          *j = ~s;
          c0 = T[--s];
          if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          if(c0 != c2) {
            if(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
          assert(k < j);
          *k-- = s;
        } else {
          assert(((s == 0) && (T[s] == c1)) || (s < 0));
          *j = ~s;
        }
      }
    }
  }

  /* Construct the suffix array by using
     the sorted order of type B suffixes. */
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~(n - 1) : (n - 1);
  /* Scan the suffix array from left to right. */
  for(i = SA, j = SA + n; i < j; ++i) {
    if(0 < (s = *i)) {
      assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      if((s == 0) || (T[s - 1] < c0)) { s = ~s; }
      if(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
      assert(i < k);
      *k++ = s;
    } else {
      assert(s < 0);
      *i = ~s;
    }
  }
}

#if 0
/* Constructs the burrows-wheeler transformed string directly
   by using the sorted order of type B* suffixes. */
static
saidx_t
construct_BWT(const sauchar_t *T, saidx_t *SA,
              saidx_t *bucket_A, saidx_t *bucket_B,
              saidx_t n, saidx_t m) {
  saidx_t *i, *j, *k, *orig;
  saidx_t s;
  saint_t c0, c1, c2;

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 < (s = *j)) {
          assert(T[s] == c1);
          assert(((s + 1) < n) && (T[s] <= T[s + 1]));
          assert(T[s - 1] <= T[s]);
          c0 = T[--s];
          *j = ~((saidx_t)c0);
          if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          if(c0 != c2) {
            if(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
          assert(k < j);
          *k-- = s;
        } else if(s != 0) {
          *j = ~s;
#ifndef NDEBUG
        } else {
          assert(T[s] == c1);
#endif
        }
      }
    }
  }

  /* Construct the BWTed string by using
     the sorted order of type B suffixes. */
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~((saidx_t)T[n - 2]) : (n - 1);
  /* Scan the suffix array from left to right. */
  for(i = SA, j = SA + n, orig = SA; i < j; ++i) {
    if(0 < (s = *i)) {
      assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      *i = c0;
      if((0 < s) && (T[s - 1] < c0)) { s = ~((saidx_t)T[s - 1]); }
      if(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
      assert(i < k);
      *k++ = s;
    } else if(s != 0) {
      *i = ~s;
    } else {
      orig = i;
    }
  }

  return orig - SA;
}
#endif

/*---------------------------------------------------------------------------*/

/**
 * Initialize suffix array context
 *
 * @return 0 for success, or non-zero in case of an error
 */
int divsufsort_init(divsufsort_ctx_t *ctx) {
   ctx->bucket_A = (saidx_t *)malloc(BUCKET_A_SIZE * sizeof(saidx_t));
   ctx->bucket_B = NULL;

   if (ctx->bucket_A) {
      ctx->bucket_B = (saidx_t *)malloc(BUCKET_B_SIZE * sizeof(saidx_t));

      if (ctx->bucket_B)
         return 0;
   }

   divsufsort_destroy(ctx);
   return -1;
}

/**
 * Destroy suffix array context
 *
 * @param ctx suffix array context to destroy
 */
void divsufsort_destroy(divsufsort_ctx_t *ctx) {
   if (ctx->bucket_B) {
      free(ctx->bucket_B);
      ctx->bucket_B = NULL;
   }

   if (ctx->bucket_A) {
      free(ctx->bucket_A);
      ctx->bucket_A = NULL;
   }
}

/*- Function -*/

saint_t
divsufsort_build_array(divsufsort_ctx_t *ctx, const sauchar_t *T, saidx_t *SA, saidx_t n) {
  saidx_t m;
  saint_t err = 0;

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  else if(n == 0) { return 0; }
  else if(n == 1) { SA[0] = 0; return 0; }
  else if(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; return 0; }

  /* Suffixsort. */
  if((ctx->bucket_A != NULL) && (ctx->bucket_B != NULL)) {
    m = sort_typeBstar(T, SA, ctx->bucket_A, ctx->bucket_B, n);
    construct_SA(T, SA, ctx->bucket_A, ctx->bucket_B, n, m);
  } else {
    err = -2;
  }

  return err;
}

#if 0
saidx_t
divbwt(const sauchar_t *T, sauchar_t *U, saidx_t *A, saidx_t n) {
  saidx_t *B;
  saidx_t *bucket_A, *bucket_B;
  saidx_t m, pidx, i;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0)) { return -1; }
  else if(n <= 1) { if(n == 1) { U[0] = T[0]; } return n; }

  if((B = A) == NULL) { B = (saidx_t *)malloc((size_t)(n + 1) * sizeof(saidx_t)); }
  bucket_A = (saidx_t *)malloc(BUCKET_A_SIZE * sizeof(saidx_t));
  bucket_B = (saidx_t *)malloc(BUCKET_B_SIZE * sizeof(saidx_t));

  /* Burrows-Wheeler Transform. */
  if((B != NULL) && (bucket_A != NULL) && (bucket_B != NULL)) {
    m = sort_typeBstar(T, B, bucket_A, bucket_B, n);
    pidx = construct_BWT(T, B, bucket_A, bucket_B, n, m);

    /* Copy to output string. */
    U[0] = T[n - 1];
    for(i = 0; i < pidx; ++i) { U[i + 1] = (sauchar_t)B[i]; }
    for(i += 1; i < n; ++i) { U[i] = (sauchar_t)B[i]; }
    pidx += 1;
  } else {
    pidx = -2;
  }

  free(bucket_B);
  free(bucket_A);
  if(A == NULL) { free(B); }

  return pidx;
}

const char *
divsufsort_version(void) {
  return PROJECT_VERSION_FULL;
}
#endif
/*
 * utils.c for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */



/*- Private Function -*/

#if 0
/* Binary search for inverse bwt. */
static
saidx_t
binarysearch_lower(const saidx_t *A, saidx_t size, saidx_t value) {
  saidx_t half, i;
  for(i = 0, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    if(A[i + half] < value) {
      i += half + 1;
      half -= (size & 1) ^ 1;
    }
  }
  return i;
}


/*- Functions -*/

/* Burrows-Wheeler transform. */
saint_t
bw_transform(const sauchar_t *T, sauchar_t *U, saidx_t *SA,
             saidx_t n, saidx_t *idx) {
  saidx_t *A, i, j, p, t;
  saint_t c;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0) || (idx == NULL)) { return -1; }
  if(n <= 1) {
    if(n == 1) { U[0] = T[0]; }
    *idx = n;
    return 0;
  }

  if((A = SA) == NULL) {
    i = divbwt(T, U, NULL, n);
    if(0 <= i) { *idx = i; i = 0; }
    return (saint_t)i;
  }

  /* BW transform. */
  if(T == U) {
    t = n;
    for(i = 0, j = 0; i < n; ++i) {
      p = t - 1;
      t = A[i];
      if(0 <= p) {
        c = T[j];
        U[j] = (j <= p) ? T[p] : (sauchar_t)A[p];
        A[j] = c;
        j++;
      } else {
        *idx = i;
      }
    }
    p = t - 1;
    if(0 <= p) {
      c = T[j];
      U[j] = (j <= p) ? T[p] : (sauchar_t)A[p];
      A[j] = c;
    } else {
      *idx = i;
    }
  } else {
    U[0] = T[n - 1];
    for(i = 0; A[i] != 0; ++i) { U[i + 1] = T[A[i] - 1]; }
    *idx = i + 1;
    for(++i; i < n; ++i) { U[i] = T[A[i] - 1]; }
  }

  if(SA == NULL) {
    /* Deallocate memory. */
    free(A);
  }

  return 0;
}

/* Inverse Burrows-Wheeler transform. */
saint_t
inverse_bw_transform(const sauchar_t *T, sauchar_t *U, saidx_t *A,
                     saidx_t n, saidx_t idx) {
  saidx_t C[ALPHABET_SIZE];
  sauchar_t D[ALPHABET_SIZE];
  saidx_t *B;
  saidx_t i, p;
  saint_t c, d;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0) || (idx < 0) ||
     (n < idx) || ((0 < n) && (idx == 0))) {
    return -1;
  }
  if(n <= 1) { return 0; }

  if((B = A) == NULL) {
    /* Allocate n*sizeof(saidx_t) bytes of memory. */
    if((B = (saidx_t *)malloc((size_t)n * sizeof(saidx_t))) == NULL) { return -2; }
  }

  /* Inverse BW transform. */
  for(c = 0; c < ALPHABET_SIZE; ++c) { C[c] = 0; }
  for(i = 0; i < n; ++i) { ++C[T[i]]; }
  for(c = 0, d = 0, i = 0; c < ALPHABET_SIZE; ++c) {
    p = C[c];
    if(0 < p) {
      C[c] = i;
      D[d++] = (sauchar_t)c;
      i += p;
    }
  }
  for(i = 0; i < idx; ++i) { B[C[T[i]]++] = i; }
  for( ; i < n; ++i)       { B[C[T[i]]++] = i + 1; }
  for(c = 0; c < d; ++c) { C[c] = C[D[c]]; }
  for(i = 0, p = idx; i < n; ++i) {
    U[i] = D[binarysearch_lower(C, d, p)];
    p = B[p - 1];
  }

  if(A == NULL) {
    /* Deallocate memory. */
    free(B);
  }

  return 0;
}

/* Checks the suffix array SA of the string T. */
saint_t
sufcheck(const sauchar_t *T, const saidx_t *SA,
         saidx_t n, saint_t verbose) {
  saidx_t C[ALPHABET_SIZE];
  saidx_t i, p, q, t;
  saint_t c;

  if(verbose) { fprintf(stderr, "sufcheck: "); }

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) {
    if(verbose) { fprintf(stderr, "Invalid arguments.\n"); }
    return -1;
  }
  if(n == 0) {
    if(verbose) { fprintf(stderr, "Done.\n"); }
    return 0;
  }

  /* check range: [0..n-1] */
  for(i = 0; i < n; ++i) {
    if((SA[i] < 0) || (n <= SA[i])) {
      if(verbose) {
        fprintf(stderr, "Out of the range [0,%" PRIdSAIDX_T "].\n"
                        "  SA[%" PRIdSAIDX_T "]=%" PRIdSAIDX_T "\n",
                        n - 1, i, SA[i]);
      }
      return -2;
    }
  }

  /* check first characters. */
  for(i = 1; i < n; ++i) {
    if(T[SA[i - 1]] > T[SA[i]]) {
      if(verbose) {
        fprintf(stderr, "Suffixes in wrong order.\n"
                        "  T[SA[%" PRIdSAIDX_T "]=%" PRIdSAIDX_T "]=%d"
                        " > T[SA[%" PRIdSAIDX_T "]=%" PRIdSAIDX_T "]=%d\n",
                        i - 1, SA[i - 1], T[SA[i - 1]], i, SA[i], T[SA[i]]);
      }
      return -3;
    }
  }

  /* check suffixes. */
  for(i = 0; i < ALPHABET_SIZE; ++i) { C[i] = 0; }
  for(i = 0; i < n; ++i) { ++C[T[i]]; }
  for(i = 0, p = 0; i < ALPHABET_SIZE; ++i) {
    t = C[i];
    C[i] = p;
    p += t;
  }

  q = C[T[n - 1]];
  C[T[n - 1]] += 1;
  for(i = 0; i < n; ++i) {
    p = SA[i];
    if(0 < p) {
      c = T[--p];
      t = C[c];
    } else {
      c = T[p = n - 1];
      t = q;
    }
    if((t < 0) || (p != SA[t])) {
      if(verbose) {
        fprintf(stderr, "Suffix in wrong position.\n"
                        "  SA[%" PRIdSAIDX_T "]=%" PRIdSAIDX_T " or\n"
                        "  SA[%" PRIdSAIDX_T "]=%" PRIdSAIDX_T "\n",
                        t, (0 <= t) ? SA[t] : -1, i, SA[i]);
      }
      return -4;
    }
    if(t != q) {
      ++C[c];
      if((n <= C[c]) || (T[SA[C[c]]] != c)) { C[c] = -1; }
    }
  }

  if(1 <= verbose) { fprintf(stderr, "Done.\n"); }
  return 0;
}


static
int
_compare(const sauchar_t *T, saidx_t Tsize,
         const sauchar_t *P, saidx_t Psize,
         saidx_t suf, saidx_t *match) {
  saidx_t i, j;
  saint_t r;
  for(i = suf + *match, j = *match, r = 0;
      (i < Tsize) && (j < Psize) && ((r = T[i] - P[j]) == 0); ++i, ++j) { }
  *match = j;
  return (r == 0) ? -(j != Psize) : r;
}

/* Search for the pattern P in the string T. */
saidx_t
sa_search(const sauchar_t *T, saidx_t Tsize,
          const sauchar_t *P, saidx_t Psize,
          const saidx_t *SA, saidx_t SAsize,
          saidx_t *idx) {
  saidx_t size, lsize, rsize, half;
  saidx_t match, lmatch, rmatch;
  saidx_t llmatch, lrmatch, rlmatch, rrmatch;
  saidx_t i, j, k;
  saint_t r;

  if(idx != NULL) { *idx = -1; }
  if((T == NULL) || (P == NULL) || (SA == NULL) ||
     (Tsize < 0) || (Psize < 0) || (SAsize < 0)) { return -1; }
  if((Tsize == 0) || (SAsize == 0)) { return 0; }
  if(Psize == 0) { if(idx != NULL) { *idx = 0; } return SAsize; }

  for(i = j = k = 0, lmatch = rmatch = 0, size = SAsize, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    match = MIN(lmatch, rmatch);
    r = _compare(T, Tsize, P, Psize, SA[i + half], &match);
    if(r < 0) {
      i += half + 1;
      half -= (size & 1) ^ 1;
      lmatch = match;
    } else if(r > 0) {
      rmatch = match;
    } else {
      lsize = half, j = i, rsize = size - half - 1, k = i + half + 1;

      /* left part */
      for(llmatch = lmatch, lrmatch = match, half = lsize >> 1;
          0 < lsize;
          lsize = half, half >>= 1) {
        lmatch = MIN(llmatch, lrmatch);
        r = _compare(T, Tsize, P, Psize, SA[j + half], &lmatch);
        if(r < 0) {
          j += half + 1;
          half -= (lsize & 1) ^ 1;
          llmatch = lmatch;
        } else {
          lrmatch = lmatch;
        }
      }

      /* right part */
      for(rlmatch = match, rrmatch = rmatch, half = rsize >> 1;
          0 < rsize;
          rsize = half, half >>= 1) {
        rmatch = MIN(rlmatch, rrmatch);
        r = _compare(T, Tsize, P, Psize, SA[k + half], &rmatch);
        if(r <= 0) {
          k += half + 1;
          half -= (rsize & 1) ^ 1;
          rlmatch = rmatch;
        } else {
          rrmatch = rmatch;
        }
      }

      break;
    }
  }

  if(idx != NULL) { *idx = (0 < (k - j)) ? j : i; }
  return k - j;
}

/* Search for the character c in the string T. */
saidx_t
sa_simplesearch(const sauchar_t *T, saidx_t Tsize,
                const saidx_t *SA, saidx_t SAsize,
                saint_t c, saidx_t *idx) {
  saidx_t size, lsize, rsize, half;
  saidx_t i, j, k, p;
  saint_t r;

  if(idx != NULL) { *idx = -1; }
  if((T == NULL) || (SA == NULL) || (Tsize < 0) || (SAsize < 0)) { return -1; }
  if((Tsize == 0) || (SAsize == 0)) { return 0; }

  for(i = j = k = 0, size = SAsize, half = size >> 1;
      0 < size;
      size = half, half >>= 1) {
    p = SA[i + half];
    r = (p < Tsize) ? T[p] - c : -1;
    if(r < 0) {
      i += half + 1;
      half -= (size & 1) ^ 1;
    } else if(r == 0) {
      lsize = half, j = i, rsize = size - half - 1, k = i + half + 1;

      /* left part */
      for(half = lsize >> 1;
          0 < lsize;
          lsize = half, half >>= 1) {
        p = SA[j + half];
        r = (p < Tsize) ? T[p] - c : -1;
        if(r < 0) {
          j += half + 1;
          half -= (lsize & 1) ^ 1;
        }
      }

      /* right part */
      for(half = rsize >> 1;
          0 < rsize;
          rsize = half, half >>= 1) {
        p = SA[k + half];
        r = (p < Tsize) ? T[p] - c : -1;
        if(r <= 0) {
          k += half + 1;
          half -= (rsize & 1) ^ 1;
        }
      }

      break;
    }
  }

  if(idx != NULL) { *idx = (0 < (k - j)) ? j : i; }
  return k - j;
}
#endif
/*
 * sssort.c for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */



/*- Private Functions -*/

static const saint_t lg_table[256]= {
 -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

#if (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE)

static INLINE
saint_t
ss_ilg(saidx_t n) {
#if SS_BLOCKSIZE == 0
# if defined(BUILD_DIVSUFSORT64)
  return (n >> 32) ?
          ((n >> 48) ?
            ((n >> 56) ?
              56 + lg_table[(n >> 56) & 0xff] :
              48 + lg_table[(n >> 48) & 0xff]) :
            ((n >> 40) ?
              40 + lg_table[(n >> 40) & 0xff] :
              32 + lg_table[(n >> 32) & 0xff])) :
          ((n & 0xffff0000) ?
            ((n & 0xff000000) ?
              24 + lg_table[(n >> 24) & 0xff] :
              16 + lg_table[(n >> 16) & 0xff]) :
            ((n & 0x0000ff00) ?
               8 + lg_table[(n >>  8) & 0xff] :
               0 + lg_table[(n >>  0) & 0xff]));
# else
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
# endif
#elif SS_BLOCKSIZE < 256
  return lg_table[n];
#else
  return (n & 0xff00) ?
          8 + lg_table[(n >> 8) & 0xff] :
          0 + lg_table[(n >> 0) & 0xff];
#endif
}

#endif /* (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE) */

#if SS_BLOCKSIZE != 0

static const saint_t sqq_table[256] = {
  0,  16,  22,  27,  32,  35,  39,  42,  45,  48,  50,  53,  55,  57,  59,  61,
 64,  65,  67,  69,  71,  73,  75,  76,  78,  80,  81,  83,  84,  86,  87,  89,
 90,  91,  93,  94,  96,  97,  98,  99, 101, 102, 103, 104, 106, 107, 108, 109,
110, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
128, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
143, 144, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155,
156, 157, 158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179, 180,
181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190, 191,
192, 192, 193, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 201,
202, 203, 203, 204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211,
212, 212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 221,
221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230,
230, 231, 231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247,
247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255
};

static INLINE
saidx_t
ss_isqrt(saidx_t x) {
  saidx_t y, e;

  if(x >= (SS_BLOCKSIZE * SS_BLOCKSIZE)) { return SS_BLOCKSIZE; }
  e = (x & 0xffff0000) ?
        ((x & 0xff000000) ?
          24 + lg_table[(x >> 24) & 0xff] :
          16 + lg_table[(x >> 16) & 0xff]) :
        ((x & 0x0000ff00) ?
           8 + lg_table[(x >>  8) & 0xff] :
           0 + lg_table[(x >>  0) & 0xff]);

  if(e >= 16) {
    y = sqq_table[x >> ((e - 6) - (e & 1))] << ((e >> 1) - 7);
    if(e >= 24) { y = (y + 1 + x / y) >> 1; }
    y = (y + 1 + x / y) >> 1;
  } else if(e >= 8) {
    y = (sqq_table[x >> ((e - 6) - (e & 1))] >> (7 - (e >> 1))) + 1;
  } else {
    return sqq_table[x] >> 4;
  }

  return (x < (y * y)) ? y - 1 : y;
}

#endif /* SS_BLOCKSIZE != 0 */


/*---------------------------------------------------------------------------*/

/* Compares two suffixes. */
static INLINE
saint_t
ss_compare(const sauchar_t *T,
           const saidx_t *p1, const saidx_t *p2,
           saidx_t depth) {
  const sauchar_t *U1, *U2, *U1n, *U2n;

  for(U1 = T + depth + *p1,
      U2 = T + depth + *p2,
      U1n = T + *(p1 + 1) + 2,
      U2n = T + *(p2 + 1) + 2;
      (U1 < U1n) && (U2 < U2n) && (*U1 == *U2);
      ++U1, ++U2) {
  }

  return U1 < U1n ?
        (U2 < U2n ? *U1 - *U2 : 1) :
        (U2 < U2n ? -1 : 0);
}


/*---------------------------------------------------------------------------*/

#if (SS_BLOCKSIZE != 1) && (SS_INSERTIONSORT_THRESHOLD != 1)

/* Insertionsort for small size groups */
static
void
ss_insertionsort(const sauchar_t *T, const saidx_t *PA,
                 saidx_t *first, saidx_t *last, saidx_t depth) {
  saidx_t *i, *j;
  saidx_t t;
  saint_t r;

  for(i = last - 2; first <= i; --i) {
    for(t = *i, j = i + 1; 0 < (r = ss_compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < last) && (*j < 0));
      if(last <= j) { break; }
    }
    if(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}

#endif /* (SS_BLOCKSIZE != 1) && (SS_INSERTIONSORT_THRESHOLD != 1) */


/*---------------------------------------------------------------------------*/

#if (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE)

static INLINE
void
ss_fixdown(const sauchar_t *Td, const saidx_t *PA,
           saidx_t *SA, saidx_t i, saidx_t size) {
  saidx_t j, k;
  saidx_t v;
  saint_t c, d, e;

  for(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    if(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
ss_heapsort(const sauchar_t *Td, const saidx_t *PA, saidx_t *SA, saidx_t size) {
  saidx_t i, m;
  saidx_t t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(Td[PA[SA[m / 2]]] < Td[PA[SA[m]]]) { SWAP(SA[m], SA[m / 2]); }
  }

  for(i = m / 2 - 1; 0 <= i; --i) { ss_fixdown(Td, PA, SA, i, m); }
  if((size % 2) == 0) { SWAP(SA[0], SA[m]); ss_fixdown(Td, PA, SA, 0, m); }
  for(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    ss_fixdown(Td, PA, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static INLINE
saidx_t *
ss_median3(const sauchar_t *Td, const saidx_t *PA,
           saidx_t *v1, saidx_t *v2, saidx_t *v3) {
  saidx_t *t;
  if(Td[PA[*v1]] > Td[PA[*v2]]) { SWAP(v1, v2); }
  if(Td[PA[*v2]] > Td[PA[*v3]]) {
    if(Td[PA[*v1]] > Td[PA[*v3]]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static INLINE
saidx_t *
ss_median5(const sauchar_t *Td, const saidx_t *PA,
           saidx_t *v1, saidx_t *v2, saidx_t *v3, saidx_t *v4, saidx_t *v5) {
  saidx_t *t;
  if(Td[PA[*v2]] > Td[PA[*v3]]) { SWAP(v2, v3); }
  if(Td[PA[*v4]] > Td[PA[*v5]]) { SWAP(v4, v5); }
  if(Td[PA[*v2]] > Td[PA[*v4]]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(Td[PA[*v1]] > Td[PA[*v3]]) { SWAP(v1, v3); }
  if(Td[PA[*v1]] > Td[PA[*v4]]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(Td[PA[*v3]] > Td[PA[*v4]]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static INLINE
saidx_t *
ss_pivot(const sauchar_t *Td, const saidx_t *PA, saidx_t *first, saidx_t *last) {
  saidx_t *middle;
  saidx_t t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return ss_median3(Td, PA, first, middle, last - 1);
    } else {
      t >>= 2;
      return ss_median5(Td, PA, first, first + t, middle, last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  first  = ss_median3(Td, PA, first, first + t, first + (t << 1));
  middle = ss_median3(Td, PA, middle - t, middle, middle + t);
  last   = ss_median3(Td, PA, last - 1 - (t << 1), last - 1 - t, last - 1);
  return ss_median3(Td, PA, first, middle, last);
}


/*---------------------------------------------------------------------------*/

/* Binary partition for substrings. */
static INLINE
saidx_t *
ss_partition(const saidx_t *PA,
                    saidx_t *first, saidx_t *last, saidx_t depth) {
  saidx_t *a, *b;
  saidx_t t;
  for(a = first - 1, b = last;;) {
    for(; (++a < b) && ((PA[*a] + depth) >= (PA[*a + 1] + 1));) { *a = ~*a; }
    for(; (a < --b) && ((PA[*b] + depth) <  (PA[*b + 1] + 1));) { }
    if(b <= a) { break; }
    t = ~*b;
    *b = *a;
    *a = t;
  }
  if(first < a) { *first = ~*first; }
  return a;
}

/* Multikey introsort for medium size groups. */
static
void
ss_mintrosort(const sauchar_t *T, const saidx_t *PA,
              saidx_t *first, saidx_t *last,
              saidx_t depth) {
#define STACK_SIZE SS_MISORT_STACKSIZE
  struct { saidx_t *a, *b, c; saint_t d; } stack[STACK_SIZE];
  const sauchar_t *Td;
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t s, t;
  saint_t ssize;
  saint_t limit;
  saint_t v, x = 0;

  for(ssize = 0, limit = ss_ilg(last - first);;) {

    if((last - first) <= SS_INSERTIONSORT_THRESHOLD) {
#if 1 < SS_INSERTIONSORT_THRESHOLD
      if(1 < (last - first)) { ss_insertionsort(T, PA, first, last, depth); }
#endif
      STACK_POP(first, last, depth, limit);
      continue;
    }

    Td = T + depth;
    if(limit-- == 0) { ss_heapsort(Td, PA, first, last - first); }
    if(limit < 0) {
      for(a = first + 1, v = Td[PA[*first]]; a < last; ++a) {
        if((x = Td[PA[*a]]) != v) {
          if(1 < (a - first)) { break; }
          v = x;
          first = a;
        }
      }
      if(Td[PA[*first] - 1] < v) {
        first = ss_partition(PA, first, a, depth);
      }
      if((a - first) <= (last - a)) {
        if(1 < (a - first)) {
          STACK_PUSH(a, last, depth, -1);
          last = a, depth += 1, limit = ss_ilg(a - first);
        } else {
          first = a, limit = -1;
        }
      } else {
        if(1 < (last - a)) {
          STACK_PUSH(first, a, depth + 1, ss_ilg(a - first));
          first = a, limit = -1;
        } else {
          last = a, depth += 1, limit = ss_ilg(a - first);
        }
      }
      continue;
    }

    /* choose pivot */
    a = ss_pivot(Td, PA, first, last);
    v = Td[PA[*a]];
    SWAP(*first, *a);

    /* partition */
    for(b = first; (++b < last) && ((x = Td[PA[*b]]) == v);) { }
    if(((a = b) < last) && (x < v)) {
      for(; (++b < last) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    for(c = last; (b < --c) && ((x = Td[PA[*c]]) == v);) { }
    if((b < (d = c)) && (x > v)) {
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }
    for(; b < c;) {
      SWAP(*b, *c);
      for(; (++b < c) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }

    if(a <= d) {
      c = b - 1;

      if((s = a - first) > (t = b - a)) { s = t; }
      for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      if((s = d - c) > (t = last - d - 1)) { s = t; }
      for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = first + (b - a), c = last - (d - c);
      b = (v <= Td[PA[*a] - 1]) ? a : ss_partition(PA, a, c, depth);

      if((a - first) <= (last - c)) {
        if((last - c) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          STACK_PUSH(c, last, depth, limit);
          last = a;
        } else if((a - first) <= (c - b)) {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          last = a;
        } else {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(first, a, depth, limit);
          first = b, last = c, depth += 1, limit = ss_ilg(c - b);
        }
      } else {
        if((a - first) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          STACK_PUSH(first, a, depth, limit);
          first = c;
        } else if((last - c) <= (c - b)) {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          first = c;
        } else {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(c, last, depth, limit);
          first = b, last = c, depth += 1, limit = ss_ilg(c - b);
        }
      }
    } else {
      limit += 1;
      if(Td[PA[*first] - 1] < v) {
        first = ss_partition(PA, first, last, depth);
        limit = ss_ilg(last - first);
      }
      depth += 1;
    }
  }
#undef STACK_SIZE
}

#endif /* (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE) */


/*---------------------------------------------------------------------------*/

#if SS_BLOCKSIZE != 0

static INLINE
void
ss_blockswap(saidx_t *a, saidx_t *b, saidx_t n) {
  saidx_t t;
  for(; 0 < n; --n, ++a, ++b) {
    t = *a, *a = *b, *b = t;
  }
}

static INLINE
void
ss_rotate(saidx_t *first, saidx_t *middle, saidx_t *last) {
  saidx_t *a, *b, t;
  saidx_t l, r;
  l = middle - first, r = last - middle;
  for(; (0 < l) && (0 < r);) {
    if(l == r) { ss_blockswap(first, middle, l); break; }
    if(l < r) {
      a = last - 1, b = middle - 1;
      t = *a;
      do {
        *a-- = *b, *b-- = *a;
        if(b < first) {
          *a = t;
          last = a;
          if((r -= l + 1) <= l) { break; }
          a -= 1, b = middle - 1;
          t = *a;
        }
      } while(1);
    } else {
      a = first, b = middle;
      t = *a;
      do {
        *a++ = *b, *b++ = *a;
        if(last <= b) {
          *a = t;
          first = a + 1;
          if((l -= r + 1) <= r) { break; }
          a += 1, b = middle;
          t = *a;
        }
      } while(1);
    }
  }
}


/*---------------------------------------------------------------------------*/

static
void
ss_inplacemerge(const sauchar_t *T, const saidx_t *PA,
                saidx_t *first, saidx_t *middle, saidx_t *last,
                saidx_t depth) {
  const saidx_t *p;
  saidx_t *a, *b;
  saidx_t len, half;
  saint_t q, r;
  saint_t x;

  for(;;) {
    if(*(last - 1) < 0) { x = 1; p = PA + ~*(last - 1); }
    else                { x = 0; p = PA +  *(last - 1); }
    for(a = first, len = middle - first, half = len >> 1, r = -1;
        0 < len;
        len = half, half >>= 1) {
      b = a + half;
      q = ss_compare(T, PA + ((0 <= *b) ? *b : ~*b), p, depth);
      if(q < 0) {
        a = b + 1;
        half -= (len & 1) ^ 1;
      } else {
        r = q;
      }
    }
    if(a < middle) {
      if(r == 0) { *a = ~*a; }
      ss_rotate(a, middle, last);
      last -= middle - a;
      middle = a;
      if(first == middle) { break; }
    }
    --last;
    if(x != 0) { while(*--last < 0) { } }
    if(middle == last) { break; }
  }
}


/*---------------------------------------------------------------------------*/

/* Merge-forward with internal buffer. */
static
void
ss_mergeforward(const sauchar_t *T, const saidx_t *PA,
                saidx_t *first, saidx_t *middle, saidx_t *last,
                saidx_t *buf, saidx_t depth) {
  saidx_t *a, *b, *c, *bufend;
  saidx_t t;
  saint_t r;

  bufend = buf + (middle - first) - 1;
  ss_blockswap(buf, first, middle - first);

  for(t = *(a = first), b = buf, c = middle;;) {
    r = ss_compare(T, PA + *b, PA + *c, depth);
    if(r < 0) {
      do {
        *a++ = *b;
        if(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);
    } else if(r > 0) {
      do {
        *a++ = *c, *c++ = *a;
        if(last <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    } else {
      *c = ~*c;
      do {
        *a++ = *b;
        if(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);

      do {
        *a++ = *c, *c++ = *a;
        if(last <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    }
  }
}

/* Merge-backward with internal buffer. */
static
void
ss_mergebackward(const sauchar_t *T, const saidx_t *PA,
                 saidx_t *first, saidx_t *middle, saidx_t *last,
                 saidx_t *buf, saidx_t depth) {
  const saidx_t *p1, *p2;
  saidx_t *a, *b, *c, *bufend;
  saidx_t t;
  saint_t r;
  saint_t x;

  bufend = buf + (last - middle) - 1;
  ss_blockswap(buf, middle, last - middle);

  x = 0;
  if(*bufend < 0)       { p1 = PA + ~*bufend; x |= 1; }
  else                  { p1 = PA +  *bufend; }
  if(*(middle - 1) < 0) { p2 = PA + ~*(middle - 1); x |= 2; }
  else                  { p2 = PA +  *(middle - 1); }
  for(t = *(a = last - 1), b = bufend, c = middle - 1;;) {
    r = ss_compare(T, p1, p2, depth);
    if(0 < r) {
      if(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = *b;
      if(b <= buf) { *buf = t; break; }
      *b-- = *a;
      if(*b < 0) { p1 = PA + ~*b; x |= 1; }
      else       { p1 = PA +  *b; }
    } else if(r < 0) {
      if(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      if(c < first) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      if(*c < 0) { p2 = PA + ~*c; x |= 2; }
      else       { p2 = PA +  *c; }
    } else {
      if(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = ~*b;
      if(b <= buf) { *buf = t; break; }
      *b-- = *a;
      if(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      if(c < first) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      if(*b < 0) { p1 = PA + ~*b; x |= 1; }
      else       { p1 = PA +  *b; }
      if(*c < 0) { p2 = PA + ~*c; x |= 2; }
      else       { p2 = PA +  *c; }
    }
  }
}

/* D&C based merge. */
static
void
ss_swapmerge(const sauchar_t *T, const saidx_t *PA,
             saidx_t *first, saidx_t *middle, saidx_t *last,
             saidx_t *buf, saidx_t bufsize, saidx_t depth) {
#define STACK_SIZE SS_SMERGE_STACKSIZE
#define GETIDX(a) ((0 <= (a)) ? (a) : (~(a)))
#define MERGE_CHECK(a, b, c)\
  do {\
    if(((c) & 1) ||\
       (((c) & 2) && (ss_compare(T, PA + GETIDX(*((a) - 1)), PA + *(a), depth) == 0))) {\
      *(a) = ~*(a);\
    }\
    if(((c) & 4) && ((ss_compare(T, PA + GETIDX(*((b) - 1)), PA + *(b), depth) == 0))) {\
      *(b) = ~*(b);\
    }\
  } while(0)
  struct { saidx_t *a, *b, *c; saint_t d; } stack[STACK_SIZE];
  saidx_t *l, *r, *lm, *rm;
  saidx_t m, len, half;
  saint_t ssize;
  saint_t check, next;

  for(check = 0, ssize = 0;;) {
    if((last - middle) <= bufsize) {
      if((first < middle) && (middle < last)) {
        ss_mergebackward(T, PA, first, middle, last, buf, depth);
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
      continue;
    }

    if((middle - first) <= bufsize) {
      if(first < middle) {
        ss_mergeforward(T, PA, first, middle, last, buf, depth);
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
      continue;
    }

    for(m = 0, len = MIN(middle - first, last - middle), half = len >> 1;
        0 < len;
        len = half, half >>= 1) {
      if(ss_compare(T, PA + GETIDX(*(middle + m + half)),
                       PA + GETIDX(*(middle - m - half - 1)), depth) < 0) {
        m += half + 1;
        half -= (len & 1) ^ 1;
      }
    }

    if(0 < m) {
      lm = middle - m, rm = middle + m;
      ss_blockswap(lm, middle, m);
      l = r = middle, next = 0;
      if(rm < last) {
        if(*rm < 0) {
          *rm = ~*rm;
          if(first < lm) { for(; *--l < 0;) { } next |= 4; }
          next |= 1;
        } else if(first < lm) {
          for(; *r < 0; ++r) { }
          next |= 2;
        }
      }

      if((l - first) <= (last - r)) {
        STACK_PUSH(r, rm, last, (next & 3) | (check & 4));
        middle = lm, last = l, check = (check & 3) | (next & 4);
      } else {
        if((next & 2) && (r == middle)) { next ^= 6; }
        STACK_PUSH(first, lm, l, (check & 3) | (next & 4));
        first = r, middle = rm, check = (next & 3) | (check & 4);
      }
    } else {
      if(ss_compare(T, PA + GETIDX(*(middle - 1)), PA + *middle, depth) == 0) {
        *middle = ~*middle;
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
    }
  }
#undef STACK_SIZE
}

#endif /* SS_BLOCKSIZE != 0 */


/*---------------------------------------------------------------------------*/

/*- Function -*/

/* Substring sort */
void
sssort(const sauchar_t *T, const saidx_t *PA,
       saidx_t *first, saidx_t *last,
       saidx_t *buf, saidx_t bufsize,
       saidx_t depth, saidx_t n, saint_t lastsuffix) {
  saidx_t *a;
#if SS_BLOCKSIZE != 0
  saidx_t *b, *middle, *curbuf;
  saidx_t j, k, curbufsize, limit;
#endif
  saidx_t i;

  if(lastsuffix != 0) { ++first; }

#if SS_BLOCKSIZE == 0
  ss_mintrosort(T, PA, first, last, depth);
#else
  if((bufsize < SS_BLOCKSIZE) &&
      (bufsize < (last - first)) &&
      (bufsize < (limit = ss_isqrt(last - first)))) {
    if(SS_BLOCKSIZE < limit) { limit = SS_BLOCKSIZE; }
    buf = middle = last - limit, bufsize = limit;
  } else {
    middle = last, limit = 0;
  }
  for(a = first, i = 0; SS_BLOCKSIZE < (middle - a); a += SS_BLOCKSIZE, ++i) {
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
    ss_mintrosort(T, PA, a, a + SS_BLOCKSIZE, depth);
#elif 1 < SS_BLOCKSIZE
    ss_insertionsort(T, PA, a, a + SS_BLOCKSIZE, depth);
#endif
    curbufsize = last - (a + SS_BLOCKSIZE);
    curbuf = a + SS_BLOCKSIZE;
    if(curbufsize <= bufsize) { curbufsize = bufsize, curbuf = buf; }
    for(b = a, k = SS_BLOCKSIZE, j = i; j & 1; b -= k, k <<= 1, j >>= 1) {
      ss_swapmerge(T, PA, b - k, b, b + k, curbuf, curbufsize, depth);
    }
  }
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
  ss_mintrosort(T, PA, a, middle, depth);
#elif 1 < SS_BLOCKSIZE
  ss_insertionsort(T, PA, a, middle, depth);
#endif
  for(k = SS_BLOCKSIZE; i != 0; k <<= 1, i >>= 1) {
    if(i & 1) {
      ss_swapmerge(T, PA, a - k, a, middle, buf, bufsize, depth);
      a -= k;
    }
  }
  if(limit != 0) {
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
    ss_mintrosort(T, PA, middle, last, depth);
#elif 1 < SS_BLOCKSIZE
    ss_insertionsort(T, PA, middle, last, depth);
#endif
    ss_inplacemerge(T, PA, first, middle, last, depth);
  }
#endif

  if(lastsuffix != 0) {
    /* Insert last type B* suffix. */
    saidx_t PAi[2]; PAi[0] = PA[*(first - 1)], PAi[1] = n - 2;
    for(a = first, i = *(first - 1);
        (a < last) && ((*a < 0) || (0 < ss_compare(T, &(PAi[0]), PA + *a, depth)));
        ++a) {
      *(a - 1) = *a;
    }
    *(a - 1) = i;
  }
}
/*
 * trsort.c for libdivsufsort
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */



/*- Private Functions -*/


static INLINE
saint_t
tr_ilg(saidx_t n) {
#if defined(BUILD_DIVSUFSORT64)
  return (n >> 32) ?
          ((n >> 48) ?
            ((n >> 56) ?
              56 + lg_table[(n >> 56) & 0xff] :
              48 + lg_table[(n >> 48) & 0xff]) :
            ((n >> 40) ?
              40 + lg_table[(n >> 40) & 0xff] :
              32 + lg_table[(n >> 32) & 0xff])) :
          ((n & 0xffff0000) ?
            ((n & 0xff000000) ?
              24 + lg_table[(n >> 24) & 0xff] :
              16 + lg_table[(n >> 16) & 0xff]) :
            ((n & 0x0000ff00) ?
               8 + lg_table[(n >>  8) & 0xff] :
               0 + lg_table[(n >>  0) & 0xff]));
#else
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
#endif
}


/*---------------------------------------------------------------------------*/

/* Simple insertionsort for small size groups. */
static
void
tr_insertionsort(const saidx_t *ISAd, saidx_t *first, saidx_t *last) {
  saidx_t *a, *b;
  saidx_t t, r;

  for(a = first + 1; a < last; ++a) {
    for(t = *a, b = a - 1; 0 > (r = ISAd[t] - ISAd[*b]);) {
      do { *(b + 1) = *b; } while((first <= --b) && (*b < 0));
      if(b < first) { break; }
    }
    if(r == 0) { *b = ~*b; }
    *(b + 1) = t;
  }
}


/*---------------------------------------------------------------------------*/

static INLINE
void
tr_fixdown(const saidx_t *ISAd, saidx_t *SA, saidx_t i, saidx_t size) {
  saidx_t j, k;
  saidx_t v;
  saidx_t c, d, e;

  for(v = SA[i], c = ISAd[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = ISAd[SA[k = j++]];
    if(d < (e = ISAd[SA[j]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
tr_heapsort(const saidx_t *ISAd, saidx_t *SA, saidx_t size) {
  saidx_t i, m;
  saidx_t t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(ISAd[SA[m / 2]] < ISAd[SA[m]]) { SWAP(SA[m], SA[m / 2]); }
  }

  for(i = m / 2 - 1; 0 <= i; --i) { tr_fixdown(ISAd, SA, i, m); }
  if((size % 2) == 0) { SWAP(SA[0], SA[m]); tr_fixdown(ISAd, SA, 0, m); }
  for(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    tr_fixdown(ISAd, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static INLINE
saidx_t *
tr_median3(const saidx_t *ISAd, saidx_t *v1, saidx_t *v2, saidx_t *v3) {
  saidx_t *t;
  if(ISAd[*v1] > ISAd[*v2]) { SWAP(v1, v2); }
  if(ISAd[*v2] > ISAd[*v3]) {
    if(ISAd[*v1] > ISAd[*v3]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static INLINE
saidx_t *
tr_median5(const saidx_t *ISAd,
           saidx_t *v1, saidx_t *v2, saidx_t *v3, saidx_t *v4, saidx_t *v5) {
  saidx_t *t;
  if(ISAd[*v2] > ISAd[*v3]) { SWAP(v2, v3); }
  if(ISAd[*v4] > ISAd[*v5]) { SWAP(v4, v5); }
  if(ISAd[*v2] > ISAd[*v4]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(ISAd[*v1] > ISAd[*v3]) { SWAP(v1, v3); }
  if(ISAd[*v1] > ISAd[*v4]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(ISAd[*v3] > ISAd[*v4]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static INLINE
saidx_t *
tr_pivot(const saidx_t *ISAd, saidx_t *first, saidx_t *last) {
  saidx_t *middle;
  saidx_t t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return tr_median3(ISAd, first, middle, last - 1);
    } else {
      t >>= 2;
      return tr_median5(ISAd, first, first + t, middle, last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  first  = tr_median3(ISAd, first, first + t, first + (t << 1));
  middle = tr_median3(ISAd, middle - t, middle, middle + t);
  last   = tr_median3(ISAd, last - 1 - (t << 1), last - 1 - t, last - 1);
  return tr_median3(ISAd, first, middle, last);
}


/*---------------------------------------------------------------------------*/

typedef struct _trbudget_t trbudget_t;
struct _trbudget_t {
  saidx_t chance;
  saidx_t remain;
  saidx_t incval;
  saidx_t count;
};

static INLINE
void
trbudget_init(trbudget_t *budget, saidx_t chance, saidx_t incval) {
  budget->chance = chance;
  budget->remain = budget->incval = incval;
}

static INLINE
saint_t
trbudget_check(trbudget_t *budget, saidx_t size) {
  if(size <= budget->remain) { budget->remain -= size; return 1; }
  if(budget->chance == 0) { budget->count += size; return 0; }
  budget->remain += budget->incval - size;
  budget->chance -= 1;
  return 1;
}


/*---------------------------------------------------------------------------*/

static INLINE
void
tr_partition(const saidx_t *ISAd,
             saidx_t *first, saidx_t *middle, saidx_t *last,
             saidx_t **pa, saidx_t **pb, saidx_t v) {
  saidx_t *a, *b, *c, *d, *e, *f;
  saidx_t t, s;
  saidx_t x = 0;

  for(b = middle - 1; (++b < last) && ((x = ISAd[*b]) == v);) { }
  if(((a = b) < last) && (x < v)) {
    for(; (++b < last) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
  }
  for(c = last; (b < --c) && ((x = ISAd[*c]) == v);) { }
  if((b < (d = c)) && (x > v)) {
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }
  for(; b < c;) {
    SWAP(*b, *c);
    for(; (++b < c) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }

  if(a <= d) {
    c = b - 1;
    if((s = a - first) > (t = b - a)) { s = t; }
    for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    if((s = d - c) > (t = last - d - 1)) { s = t; }
    for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    first += (b - a), last -= (d - c);
  }
  *pa = first, *pb = last;
}

static
void
tr_copy(saidx_t *ISA, const saidx_t *SA,
        saidx_t *first, saidx_t *a, saidx_t *b, saidx_t *last,
        saidx_t depth) {
  /* sort suffixes of middle partition
     by using sorted order of suffixes of left and right partition. */
  saidx_t *c, *d, *e;
  saidx_t s, v;

  v = b - SA - 1;
  for(c = first, d = a - 1; c <= d; ++c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      ISA[s] = d - SA;
    }
  }
  for(c = last - 1, e = d + 1, d = b; e < d; --c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      ISA[s] = d - SA;
    }
  }
}

static
void
tr_partialcopy(saidx_t *ISA, const saidx_t *SA,
               saidx_t *first, saidx_t *a, saidx_t *b, saidx_t *last,
               saidx_t depth) {
  saidx_t *c, *d, *e;
  saidx_t s, v;
  saidx_t rank, lastrank, newrank = -1;

  v = b - SA - 1;
  lastrank = -1;
  for(c = first, d = a - 1; c <= d; ++c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      rank = ISA[s + depth];
      if(lastrank != rank) { lastrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }

  lastrank = -1;
  for(e = d; first <= e; --e) {
    rank = ISA[*e];
    if(lastrank != rank) { lastrank = rank; newrank = e - SA; }
    if(newrank != rank) { ISA[*e] = newrank; }
  }

  lastrank = -1;
  for(c = last - 1, e = d + 1, d = b; e < d; --c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      rank = ISA[s + depth];
      if(lastrank != rank) { lastrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }
}

static
void
tr_introsort(saidx_t *ISA, const saidx_t *ISAd,
             saidx_t *SA, saidx_t *first, saidx_t *last,
             trbudget_t *budget) {
#define STACK_SIZE TR_STACKSIZE
  struct { const saidx_t *a; saidx_t *b, *c; saint_t d, e; }stack[STACK_SIZE];
  saidx_t *a, *b, *c;
  saidx_t t;
  saidx_t v, x = 0;
  saidx_t incr = ISAd - ISA;
  saint_t limit, next;
  saint_t ssize, trlink = -1;

  for(ssize = 0, limit = tr_ilg(last - first);;) {

    if(limit < 0) {
      if(limit == -1) {
        /* tandem repeat partition */
        tr_partition(ISAd - incr, first, first, last, &a, &b, last - SA - 1);

        /* update ranks */
        if(a < last) {
          for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
        }
        if(b < last) {
          for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; }
        }

        /* push */
        if(1 < (b - a)) {
          STACK_PUSH5(NULL, a, b, 0, 0);
          STACK_PUSH5(ISAd - incr, first, last, -2, trlink);
          trlink = ssize - 2;
        }
        if((a - first) <= (last - b)) {
          if(1 < (a - first)) {
            STACK_PUSH5(ISAd, b, last, tr_ilg(last - b), trlink);
            last = a, limit = tr_ilg(a - first);
          } else if(1 < (last - b)) {
            first = b, limit = tr_ilg(last - b);
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        } else {
          if(1 < (last - b)) {
            STACK_PUSH5(ISAd, first, a, tr_ilg(a - first), trlink);
            first = b, limit = tr_ilg(last - b);
          } else if(1 < (a - first)) {
            last = a, limit = tr_ilg(a - first);
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        }
      } else if(limit == -2) {
        /* tandem repeat copy */
        a = stack[--ssize].b, b = stack[ssize].c;
        if(stack[ssize].d == 0) {
          tr_copy(ISA, SA, first, a, b, last, ISAd - ISA);
        } else {
          if(0 <= trlink) { stack[trlink].d = -1; }
          tr_partialcopy(ISA, SA, first, a, b, last, ISAd - ISA);
        }
        STACK_POP5(ISAd, first, last, limit, trlink);
      } else {
        /* sorted partition */
        if(0 <= *first) {
          a = first;
          do { ISA[*a] = a - SA; } while((++a < last) && (0 <= *a));
          first = a;
        }
        if(first < last) {
          a = first; do { *a = ~*a; } while(*++a < 0);
          next = (ISA[*a] != ISAd[*a]) ? tr_ilg(a - first + 1) : -1;
          if(++a < last) { for(b = first, v = a - SA - 1; b < a; ++b) { ISA[*b] = v; } }

          /* push */
          if(trbudget_check(budget, a - first)) {
            if((a - first) <= (last - a)) {
              STACK_PUSH5(ISAd, a, last, -3, trlink);
              ISAd += incr, last = a, limit = next;
            } else {
              if(1 < (last - a)) {
                STACK_PUSH5(ISAd + incr, first, a, next, trlink);
                first = a, limit = -3;
              } else {
                ISAd += incr, last = a, limit = next;
              }
            }
          } else {
            if(0 <= trlink) { stack[trlink].d = -1; }
            if(1 < (last - a)) {
              first = a, limit = -3;
            } else {
              STACK_POP5(ISAd, first, last, limit, trlink);
            }
          }
        } else {
          STACK_POP5(ISAd, first, last, limit, trlink);
        }
      }
      continue;
    }

    if((last - first) <= TR_INSERTIONSORT_THRESHOLD) {
      tr_insertionsort(ISAd, first, last);
      limit = -3;
      continue;
    }

    if(limit-- == 0) {
      tr_heapsort(ISAd, first, last - first);
      for(a = last - 1; first < a; a = b) {
        for(x = ISAd[*a], b = a - 1; (first <= b) && (ISAd[*b] == x); --b) { *b = ~*b; }
      }
      limit = -3;
      continue;
    }

    /* choose pivot */
    a = tr_pivot(ISAd, first, last);
    SWAP(*first, *a);
    v = ISAd[*first];

    /* partition */
    tr_partition(ISAd, first, first + 1, last, &a, &b, v);
    if((last - first) != (b - a)) {
      next = (ISA[*a] != v) ? tr_ilg(b - a) : -1;

      /* update ranks */
      for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      if(b < last) { for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }

      /* push */
      if((1 < (b - a)) && (trbudget_check(budget, b - a))) {
        if((a - first) <= (last - b)) {
          if((last - b) <= (b - a)) {
            if(1 < (a - first)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              last = a;
            } else if(1 < (last - b)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              first = b;
            } else {
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else if((a - first) <= (b - a)) {
            if(1 < (a - first)) {
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              last = a;
            } else {
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else {
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            ISAd += incr, first = a, last = b, limit = next;
          }
        } else {
          if((a - first) <= (b - a)) {
            if(1 < (last - b)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              first = b;
            } else if(1 < (a - first)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              last = a;
            } else {
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else if((last - b) <= (b - a)) {
            if(1 < (last - b)) {
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              first = b;
            } else {
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else {
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            ISAd += incr, first = a, last = b, limit = next;
          }
        }
      } else {
        if((1 < (b - a)) && (0 <= trlink)) { stack[trlink].d = -1; }
        if((a - first) <= (last - b)) {
          if(1 < (a - first)) {
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            last = a;
          } else if(1 < (last - b)) {
            first = b;
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        } else {
          if(1 < (last - b)) {
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            first = b;
          } else if(1 < (a - first)) {
            last = a;
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        }
      }
    } else {
      if(trbudget_check(budget, last - first)) {
        limit = tr_ilg(last - first), ISAd += incr;
      } else {
        if(0 <= trlink) { stack[trlink].d = -1; }
        STACK_POP5(ISAd, first, last, limit, trlink);
      }
    }
  }
#undef STACK_SIZE
}



/*---------------------------------------------------------------------------*/

/*- Function -*/

/* Tandem repeat sort */
void
trsort(saidx_t *ISA, saidx_t *SA, saidx_t n, saidx_t depth) {
  saidx_t *ISAd;
  saidx_t *first, *last;
  trbudget_t budget;
  saidx_t t, skip, unsorted;

  trbudget_init(&budget, tr_ilg(n) * 2 / 3, n);
/*  trbudget_init(&budget, tr_ilg(n) * 3 / 4, n); */
  for(ISAd = ISA + depth; -n < *SA; ISAd += ISAd - ISA) {
    first = SA;
    skip = 0;
    unsorted = 0;
    do {
      if((t = *first) < 0) { first -= t; skip += t; }
      else {
        if(skip != 0) { *(first + skip) = skip; skip = 0; }
        last = SA + ISA[t] + 1;
        if(1 < (last - first)) {
          budget.count = 0;
          tr_introsort(ISA, ISAd, SA, first, last, &budget);
          if(budget.count != 0) { unsorted += budget.count; }
          else { skip = first - last; }
        } else if((last - first) == 1) {
          skip = -1;
        }
        first = last;
      }
    } while(first < (SA + n));
    if(skip != 0) { *(first + skip) = skip; }
    if(unsorted == 0) { break; }
  }
}
