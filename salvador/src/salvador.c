/*
 * salvador.c - command line compression utility for the salvador library
 *
 * Copyright (C) 2021 Emmanuel Marty
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
 * Implements the ZX0 encoding designed by Einar Saukas. https://github.com/einar-saukas/ZX0
 * Also inspired by Charles Bloom's compression blog. http://cbloomrants.blogspot.com/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "libsalvador.h"

#define OPT_VERBOSE        1
#define OPT_STATS          2
#define OPT_BACKWARD       4
#define OPT_CLASSIC        8

#define TOOL_VERSION "1.4.2"

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
LARGE_INTEGER hpc_frequency;
BOOL hpc_available = FALSE;
#endif

static void do_init_time() {
#ifdef _WIN32
   hpc_frequency.QuadPart = 0;
   hpc_available = QueryPerformanceFrequency(&hpc_frequency);
#endif
}

static long long do_get_time() {
   long long nTime;

#ifdef _WIN32
   if (hpc_available) {
      LARGE_INTEGER nCurTime;

      /* Use HPC hardware for best precision */
      QueryPerformanceCounter(&nCurTime);
      nTime = (long long)(nCurTime.QuadPart * 1000000LL / hpc_frequency.QuadPart);
   }
   else {
      struct _timeb tb;
      _ftime(&tb);

      nTime = ((long long)tb.time * 1000LL + (long long)tb.millitm) * 1000LL;
   }
#else
   struct timeval tm;
   gettimeofday(&tm, NULL);

   nTime = (long long)tm.tv_sec * 1000000LL + (long long)tm.tv_usec;
#endif
   return nTime;
}

static void do_reverse_buffer(unsigned char *pBuffer, size_t nBufferSize) {
   size_t nMidPoint = nBufferSize / 2;
   size_t i, j;

   for (i = 0, j = nBufferSize - 1; i < nMidPoint; i++, j--) {
      unsigned char c = pBuffer[i];
      pBuffer[i] = pBuffer[j];
      pBuffer[j] = c;
   }
}

/*---------------------------------------------------------------------------*/

static void compression_progress(long long nOriginalSize, long long nCompressedSize) {
   if (nOriginalSize >= 512 * 1024) {
      fprintf(stdout, "\r%lld => %lld (%g %%)     \b\b\b\b\b", nOriginalSize, nCompressedSize, (double)(nCompressedSize * 100.0 / nOriginalSize));
      fflush(stdout);
   }
}

static int zx0_compress(const char *pszInFilename, const char *pszOutFilename, const char *pszDictionaryFilename, const unsigned int nOptions, const unsigned int nMaxWindowSize) {
   long long nStartTime = 0LL, nEndTime = 0LL;
   size_t nOriginalSize = 0L, nCompressedSize = 0L, nMaxCompressedSize;
   int nFlags = (nOptions & OPT_CLASSIC) ? 0 : FLG_IS_INVERTED;
   salvador_stats stats;
   unsigned char *pDecompressedData;
   unsigned char *pCompressedData;

   if (nOptions & OPT_BACKWARD)
      nFlags |= FLG_IS_BACKWARD;

   if (nOptions & OPT_VERBOSE) {
      nStartTime = do_get_time();
   }

   FILE* f_dict = NULL;
   size_t nDictionarySize = 0;
   if (pszDictionaryFilename) {
      /* Open the dictionary */
      f_dict = fopen(pszDictionaryFilename, "rb");
      if (!f_dict) {
         fprintf(stderr, "error opening dictionary '%s' for reading\n", pszDictionaryFilename);
         return 100;
      }

      /* Get dictionary size */
      fseek(f_dict, 0, SEEK_END);
      nDictionarySize = (size_t)ftell(f_dict);
      fseek(f_dict, 0, SEEK_SET);

      if (nDictionarySize > BLOCK_SIZE) nDictionarySize = BLOCK_SIZE;
   }

   /* Read the whole original file in memory */

   FILE *f_in = fopen(pszInFilename, "rb");
   if (!f_in) {
      if (f_dict) fclose(f_dict);
      fprintf(stderr, "error opening '%s' for reading\n", pszInFilename);
      return 100;
   }

   fseek(f_in, 0, SEEK_END);
   nOriginalSize = (size_t)ftell(f_in);
   fseek(f_in, 0, SEEK_SET);

   pDecompressedData = (unsigned char*)malloc(nDictionarySize + nOriginalSize);
   if (!pDecompressedData) {
      fclose(f_in);
      if (f_dict) fclose(f_dict);
      fprintf(stderr, "out of memory for reading '%s', %zu bytes needed\n", pszInFilename, nOriginalSize);
      return 100;
   }

   if (f_dict) {
      /* Read dictionary data */
      if (fread(pDecompressedData + ((nOptions & OPT_BACKWARD) ? nOriginalSize : 0), 1, nDictionarySize, f_dict) != nDictionarySize) {
         free(pDecompressedData);
         fclose(f_in);
         fclose(f_dict);
         fprintf(stderr, "I/O error while reading dictionary '%s'\n", pszDictionaryFilename);
         return 100;
      }

      fclose(f_dict);
      f_dict = NULL;
   }

   /* Read input file data */
   if (fread(pDecompressedData + ((nOptions & OPT_BACKWARD) ? 0 : nDictionarySize), 1, nOriginalSize, f_in) != nOriginalSize) {
      free(pDecompressedData);
      fclose(f_in);
      fprintf(stderr, "I/O error while reading '%s'\n", pszInFilename);
      return 100;
   }

   fclose(f_in);
   f_in = NULL;

   if (nOptions & OPT_BACKWARD)
      do_reverse_buffer(pDecompressedData, nDictionarySize + nOriginalSize);

   /* Allocate max compressed size */

   nMaxCompressedSize = salvador_get_max_compressed_size(nDictionarySize + nOriginalSize);

   pCompressedData = (unsigned char*)malloc(nMaxCompressedSize);
   if (!pCompressedData) {
      free(pDecompressedData);
      fprintf(stderr, "out of memory for compressing '%s', %zu bytes needed\n", pszInFilename, nMaxCompressedSize);
      return 100;
   }

   memset(pCompressedData, 0, nMaxCompressedSize);

   nCompressedSize = salvador_compress(pDecompressedData, pCompressedData, nDictionarySize + nOriginalSize, nMaxCompressedSize, nFlags, nMaxWindowSize, nDictionarySize, compression_progress, &stats);

   if (nOptions & OPT_VERBOSE) {
      nEndTime = do_get_time();
   }

   if (nCompressedSize == (size_t)-1) {
      free(pCompressedData);
      free(pDecompressedData);
      fprintf(stderr, "compression error for '%s'\n", pszInFilename);
      return 100;
   }

   if (nOptions & OPT_BACKWARD)
      do_reverse_buffer(pCompressedData, nCompressedSize);

   /* Write whole compressed file out */

   FILE *f_out = fopen(pszOutFilename, "wb");
   if (!f_out) {
      free(pCompressedData);
      free(pDecompressedData);
      fprintf(stderr, "error opening '%s' for writing\n", pszOutFilename);
      return 100;
   }
   
   fwrite(pCompressedData, 1, nCompressedSize, f_out);
   fclose(f_out);

   free(pCompressedData);
   free(pDecompressedData);

   if (nOptions & OPT_VERBOSE) {
      double fDelta = ((double)(nEndTime - nStartTime)) / 1000000.0;
      double fSpeed = ((double)nOriginalSize / 1048576.0) / fDelta;
      fprintf(stdout, "\rCompressed '%s' in %g seconds, %.02g Mb/s, %d tokens (%g bytes/token), %zu into %zu bytes ==> %g %%\n",
         pszInFilename, fDelta, fSpeed, stats.commands_divisor, (double)nOriginalSize / (double)stats.commands_divisor,
         nOriginalSize, nCompressedSize, (double)(nCompressedSize * 100.0 / nOriginalSize));
   }

   if (nOptions & OPT_STATS) {
      if (stats.literals_divisor > 0)
         fprintf(stdout, "Literals: min: %d avg: %d max: %d count: %d\n", stats.min_literals, stats.total_literals / stats.literals_divisor, stats.max_literals, stats.literals_divisor);
      else
         fprintf(stdout, "Literals: none\n");

      fprintf(stdout, "Normal matches: %d rep matches: %d EOD: %d\n",
         stats.num_normal_matches, stats.num_rep_matches, stats.num_eod);

      if (stats.match_divisor > 0) {
         fprintf(stdout, "Offsets: min: %d avg: %d max: %d count: %d\n", stats.min_offset, (int)(stats.total_offsets / (long long)stats.match_divisor), stats.max_offset, stats.match_divisor);
         fprintf(stdout, "Match lens: min: %d avg: %d max: %d count: %d\n", stats.min_match_len, stats.total_match_lens / stats.match_divisor, stats.max_match_len, stats.match_divisor);
      }
      else {
         fprintf(stdout, "Offsets: none\n");
         fprintf(stdout, "Match lens: none\n");
      }
      if (stats.rle1_divisor > 0) {
         fprintf(stdout, "RLE1 lens: min: %d avg: %d max: %d count: %d\n", stats.min_rle1_len, stats.total_rle1_lens / stats.rle1_divisor, stats.max_rle1_len, stats.rle1_divisor);
      }
      else {
         fprintf(stdout, "RLE1 lens: none\n");
      }
      if (stats.rle2_divisor > 0) {
         fprintf(stdout, "RLE2 lens: min: %d avg: %d max: %d count: %d\n", stats.min_rle2_len, stats.total_rle2_lens / stats.rle2_divisor, stats.max_rle2_len, stats.rle2_divisor);
      }
      else {
         fprintf(stdout, "RLE2 lens: none\n");
      }
      fprintf(stdout, "Safe distance: %d (0x%X)\n", stats.safe_dist, stats.safe_dist);
   }
   return 0;
}

/*---------------------------------------------------------------------------*/

static void generate_compressible_data(unsigned char *pBuffer, size_t nBufferSize, unsigned int nSeed, int nNumLiteralValues, float fMatchProbability) {
   size_t nIndex = 0;
   int nMatchProbability = (int)(fMatchProbability * 1023.0f);

   srand(nSeed);
   
   if (nBufferSize == 0) return;
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

   while (nIndex < nBufferSize) {
      if ((rand() & 1023) < nXorProbability) {
         pBuffer[nIndex] ^= 0xff;
      }
      nIndex++;
   }
}


