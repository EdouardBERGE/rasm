/*

Warning! This is a modified version of original sources!

To sum up:
- all include files and C sources were merged in a single file
- existing logs were removed (except error logs)
- main were removed and wrapper added


 *  LZ4 - Fast LZ compression algorithm
 *  Header File
 *  Copyright (C) 2011-2017, Yann Collet.

   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
    - LZ4 homepage : http://www.lz4.org
    - LZ4 source repository : https://github.com/lz4/lz4
*/
#if defined (__cplusplus)
extern "C" {
#endif

#ifndef LZ4_H_2983827168210
#define LZ4_H_2983827168210

/* --- Dependency --- */


/**
  Introduction

  LZ4 is lossless compression algorithm, providing compression speed at 400 MB/s per core,
  scalable with multi-cores CPU. It features an extremely fast decoder, with speed in
  multiple GB/s per core, typically reaching RAM speed limits on multi-core systems.

  The LZ4 compression library provides in-memory compression and decompression functions.
  Compression can be done in:
    - a single step (described as Simple Functions)
    - a single step, reusing a context (described in Advanced Functions)
    - unbounded multiple steps (described as Streaming compression)

  lz4.h provides block compression functions. It gives full buffer control to user.
  Decompressing an lz4-compressed block also requires metadata (such as compressed size).
  Each application is free to encode such metadata in whichever way it wants.

  An additional format, called LZ4 frame specification (doc/lz4_Frame_format.md),
  take care of encoding standard metadata alongside LZ4-compressed blocks.
  If your application requires interoperability, it's recommended to use it.
  A library is provided to take care of it, see lz4frame.h.
*/

/*^***************************************************************
*  Export parameters
*****************************************************************/
/*
*  LZ4_DLL_EXPORT :
*  Enable exporting of functions when building a Windows DLL
*  LZ4LIB_API :
*  Control library symbols visibility.
*/
#if defined(LZ4_DLL_EXPORT) && (LZ4_DLL_EXPORT==1)
#  define LZ4LIB_API __declspec(dllexport)
#elif defined(LZ4_DLL_IMPORT) && (LZ4_DLL_IMPORT==1)
#  define LZ4LIB_API __declspec(dllimport) /* It isn't required but allows to generate better code, saving a function pointer load from the IAT and an indirect jump.*/
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#  define LZ4LIB_API __attribute__ ((__visibility__ ("default")))
#else
#  define LZ4LIB_API
#endif


/*------   Version   ------*/
#define LZ4_VERSION_MAJOR    1    /* for breaking interface changes  */
#define LZ4_VERSION_MINOR    7    /* for new (non-breaking) interface capabilities */
#define LZ4_VERSION_RELEASE  6    /* for tweaks, bug-fixes, or development */

#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)

#define LZ4_LIB_VERSION LZ4_VERSION_MAJOR.LZ4_VERSION_MINOR.LZ4_VERSION_RELEASE
#define LZ4_QUOTE(str) #str
#define LZ4_EXPAND_AND_QUOTE(str) LZ4_QUOTE(str)
#define LZ4_VERSION_STRING LZ4_EXPAND_AND_QUOTE(LZ4_LIB_VERSION)

LZ4LIB_API int LZ4_versionNumber (void);  /**< library version number; to be used when checking dll version */
LZ4LIB_API const char* LZ4_versionString (void);   /**< library version string; to be used when checking dll version */


/*-************************************
*  Tuning parameter
**************************************/
/*!
 * LZ4_MEMORY_USAGE :
 * Memory usage formula : N->2^N Bytes (examples : 10 -> 1KB; 12 -> 4KB ; 16 -> 64KB; 20 -> 1MB; etc.)
 * Increasing memory usage improves compression ratio
 * Reduced memory usage can improve speed, due to cache effect
 * Default value is 14, for 16KB, which nicely fits into Intel x86 L1 cache
 */
#ifndef LZ4_MEMORY_USAGE
# define LZ4_MEMORY_USAGE 14
#endif

/*-************************************
*  Simple Functions
**************************************/
/*! LZ4_compress_default() :
    Compresses 'sourceSize' bytes from buffer 'source'
    into already allocated 'dest' buffer of size 'maxDestSize'.
    Compression is guaranteed to succeed if 'maxDestSize' >= LZ4_compressBound(sourceSize).
    It also runs faster, so it's a recommended setting.
    If the function cannot compress 'source' into a more limited 'dest' budget,
    compression stops *immediately*, and the function result is zero.
    As a consequence, 'dest' content is not valid.
    This function never writes outside 'dest' buffer, nor read outside 'source' buffer.
        sourceSize  : Max supported value is LZ4_MAX_INPUT_VALUE
        maxDestSize : full or partial size of buffer 'dest' (which must be already allocated)
        return : the number of bytes written into buffer 'dest' (necessarily <= maxOutputSize)
              or 0 if compression fails */
LZ4LIB_API int LZ4_compress_default(const char* source, char* dest, int sourceSize, int maxDestSize);

/*! LZ4_decompress_safe() :
    compressedSize : is the precise full size of the compressed block.
    maxDecompressedSize : is the size of destination buffer, which must be already allocated.
    return : the number of bytes decompressed into destination buffer (necessarily <= maxDecompressedSize)
             If destination buffer is not large enough, decoding will stop and output an error code (<0).
             If the source stream is detected malformed, the function will stop decoding and return a negative result.
             This function is protected against buffer overflow exploits, including malicious data packets.
             It never writes outside output buffer, nor reads outside input buffer.
*/
LZ4LIB_API int LZ4_decompress_safe (const char* source, char* dest, int compressedSize, int maxDecompressedSize);


/*-************************************
*  Advanced Functions
**************************************/
#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)

/*!
LZ4_compressBound() :
    Provides the maximum size that LZ4 compression may output in a "worst case" scenario (input data not compressible)
    This function is primarily useful for memory allocation purposes (destination buffer size).
    Macro LZ4_COMPRESSBOUND() is also provided for compilation-time evaluation (stack memory allocation for example).
    Note that LZ4_compress_default() compress faster when dest buffer size is >= LZ4_compressBound(srcSize)
        inputSize  : max supported value is LZ4_MAX_INPUT_SIZE
        return : maximum output size in a "worst case" scenario
              or 0, if input size is too large ( > LZ4_MAX_INPUT_SIZE)
*/
LZ4LIB_API int LZ4_compressBound(int inputSize);

/*!
LZ4_compress_fast() :
    Same as LZ4_compress_default(), but allows to select an "acceleration" factor.
    The larger the acceleration value, the faster the algorithm, but also the lesser the compression.
    It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
    An acceleration value of "1" is the same as regular LZ4_compress_default()
    Values <= 0 will be replaced by ACCELERATION_DEFAULT (see lz4.c), which is 1.
*/
LZ4LIB_API int LZ4_compress_fast (const char* source, char* dest, int sourceSize, int maxDestSize, int acceleration);


/*!
LZ4_compress_fast_extState() :
    Same compression function, just using an externally allocated memory space to store compression state.
    Use LZ4_sizeofState() to know how much memory must be allocated,
    and allocate it on 8-bytes boundaries (using malloc() typically).
    Then, provide it as 'void* state' to compression function.
*/
LZ4LIB_API int LZ4_sizeofState(void);
LZ4LIB_API int LZ4_compress_fast_extState (void* state, const char* source, char* dest, int inputSize, int maxDestSize, int acceleration);


/*!
LZ4_compress_destSize() :
    Reverse the logic, by compressing as much data as possible from 'source' buffer
    into already allocated buffer 'dest' of size 'targetDestSize'.
    This function either compresses the entire 'source' content into 'dest' if it's large enough,
    or fill 'dest' buffer completely with as much data as possible from 'source'.
        *sourceSizePtr : will be modified to indicate how many bytes where read from 'source' to fill 'dest'.
                         New value is necessarily <= old value.
        return : Nb bytes written into 'dest' (necessarily <= targetDestSize)
              or 0 if compression fails
*/
LZ4LIB_API int LZ4_compress_destSize (const char* source, char* dest, int* sourceSizePtr, int targetDestSize);


/*!
LZ4_decompress_fast() :
    originalSize : is the original and therefore uncompressed size
    return : the number of bytes read from the source buffer (in other words, the compressed size)
             If the source stream is detected malformed, the function will stop decoding and return a negative result.
             Destination buffer must be already allocated. Its size must be a minimum of 'originalSize' bytes.
    note : This function fully respect memory boundaries for properly formed compressed data.
           It is a bit faster than LZ4_decompress_safe().
           However, it does not provide any protection against intentionally modified data stream (malicious input).
           Use this function in trusted environment only (data to decode comes from a trusted source).
*/
LZ4LIB_API int LZ4_decompress_fast (const char* source, char* dest, int originalSize);

/*!
LZ4_decompress_safe_partial() :
    This function decompress a compressed block of size 'compressedSize' at position 'source'
    into destination buffer 'dest' of size 'maxDecompressedSize'.
    The function tries to stop decompressing operation as soon as 'targetOutputSize' has been reached,
    reducing decompression time.
    return : the number of bytes decoded in the destination buffer (necessarily <= maxDecompressedSize)
       Note : this number can be < 'targetOutputSize' should the compressed block to decode be smaller.
             Always control how many bytes were decoded.
             If the source stream is detected malformed, the function will stop decoding and return a negative result.
             This function never writes outside of output buffer, and never reads outside of input buffer. It is therefore protected against malicious data packets
*/
LZ4LIB_API int LZ4_decompress_safe_partial (const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize);


/*-*********************************************
*  Streaming Compression Functions
***********************************************/
typedef union LZ4_stream_u LZ4_stream_t;   /* incomplete type (defined later) */

/*! LZ4_createStream() and LZ4_freeStream() :
 *  LZ4_createStream() will allocate and initialize an `LZ4_stream_t` structure.
 *  LZ4_freeStream() releases its memory.
 */
LZ4LIB_API LZ4_stream_t* LZ4_createStream(void);
LZ4LIB_API int           LZ4_freeStream (LZ4_stream_t* streamPtr);

/*! LZ4_resetStream() :
 *  An LZ4_stream_t structure can be allocated once and re-used multiple times.
 *  Use this function to init an allocated `LZ4_stream_t` structure and start a new compression.
 */
LZ4LIB_API void LZ4_resetStream (LZ4_stream_t* streamPtr);

/*! LZ4_loadDict() :
 *  Use this function to load a static dictionary into LZ4_stream.
 *  Any previous data will be forgotten, only 'dictionary' will remain in memory.
 *  Loading a size of 0 is allowed.
 *  Return : dictionary size, in bytes (necessarily <= 64 KB)
 */
LZ4LIB_API int LZ4_loadDict (LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);

/*! LZ4_compress_fast_continue() :
 *  Compress buffer content 'src', using data from previously compressed blocks as dictionary to improve compression ratio.
 *  Important : Previous data blocks are assumed to remain present and unmodified !
 *  'dst' buffer must be already allocated.
 *  If dstCapacity >= LZ4_compressBound(srcSize), compression is guaranteed to succeed, and runs faster.
 *  If not, and if compressed data cannot fit into 'dst' buffer size, compression stops, and function returns a zero.
 *  After an error, the stream status is invalid, and it can only be reset or freed.
 */
LZ4LIB_API int LZ4_compress_fast_continue (LZ4_stream_t* streamPtr, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_saveDict() :
 *  If previously compressed data block is not guaranteed to remain available at its current memory location,
 *  save it into a safer place (char* safeBuffer).
 *  Note : it's not necessary to call LZ4_loadDict() after LZ4_saveDict(), dictionary is immediately usable.
 *  @return : saved dictionary size in bytes (necessarily <= dictSize), or 0 if error.
 */
LZ4LIB_API int LZ4_saveDict (LZ4_stream_t* streamPtr, char* safeBuffer, int dictSize);


/*-**********************************************
*  Streaming Decompression Functions
*  Bufferless synchronous API
************************************************/
typedef union LZ4_streamDecode_u LZ4_streamDecode_t;   /* incomplete type (defined later) */

/*! LZ4_createStreamDecode() and LZ4_freeStreamDecode() :
 *  creation / destruction of streaming decompression tracking structure */
LZ4LIB_API LZ4_streamDecode_t* LZ4_createStreamDecode(void);
LZ4LIB_API int                 LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream);

/*! LZ4_setStreamDecode() :
 *  Use this function to instruct where to find the dictionary.
 *  Setting a size of 0 is allowed (same effect as reset).
 *  @return : 1 if OK, 0 if error
 */
LZ4LIB_API int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize);

/*!
LZ4_decompress_*_continue() :
    These decoding functions allow decompression of multiple blocks in "streaming" mode.
    Previously decoded blocks *must* remain available at the memory position where they were decoded (up to 64 KB)
    In the case of a ring buffers, decoding buffer must be either :
    - Exactly same size as encoding buffer, with same update rule (block boundaries at same positions)
      In which case, the decoding & encoding ring buffer can have any size, including very small ones ( < 64 KB).
    - Larger than encoding buffer, by a minimum of maxBlockSize more bytes.
      maxBlockSize is implementation dependent. It's the maximum size you intend to compress into a single block.
      In which case, encoding and decoding buffers do not need to be synchronized,
      and encoding ring buffer can have any size, including small ones ( < 64 KB).
    - _At least_ 64 KB + 8 bytes + maxBlockSize.
      In which case, encoding and decoding buffers do not need to be synchronized,
      and encoding ring buffer can have any size, including larger than decoding buffer.
    Whenever these conditions are not possible, save the last 64KB of decoded data into a safe buffer,
    and indicate where it is saved using LZ4_setStreamDecode()
*/
LZ4LIB_API int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxDecompressedSize);
LZ4LIB_API int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize);


/*! LZ4_decompress_*_usingDict() :
 *  These decoding functions work the same as
 *  a combination of LZ4_setStreamDecode() followed by LZ4_decompress_*_continue()
 *  They are stand-alone, and don't need an LZ4_streamDecode_t structure.
 */
LZ4LIB_API int LZ4_decompress_safe_usingDict (const char* source, char* dest, int compressedSize, int maxDecompressedSize, const char* dictStart, int dictSize);
LZ4LIB_API int LZ4_decompress_fast_usingDict (const char* source, char* dest, int originalSize, const char* dictStart, int dictSize);


/*^**********************************************
 * !!!!!!   STATIC LINKING ONLY   !!!!!!
 ***********************************************/
/*-************************************
 *  Private definitions
 **************************************
 * Do not use these definitions.
 * They are exposed to allow static allocation of `LZ4_stream_t` and `LZ4_streamDecode_t`.
 * Using these definitions will expose code to API and/or ABI break in future versions of the library.
 **************************************/
#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
#define LZ4_HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define LZ4_HASH_SIZE_U32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)

typedef struct {
    uint32_t hashTable[LZ4_HASH_SIZE_U32];
    uint32_t currentOffset;
    uint32_t initCheck;
    const uint8_t* dictionary;
    uint8_t* bufferStart;   /* obsolete, used for slideInputBuffer */
    uint32_t dictSize;
} LZ4_stream_t_internal;

typedef struct {
    const uint8_t* externalDict;
    size_t extDictSize;
    const uint8_t* prefixEnd;
    size_t prefixSize;
} LZ4_streamDecode_t_internal;

#else

typedef struct {
    unsigned int hashTable[LZ4_HASH_SIZE_U32];
    unsigned int currentOffset;
    unsigned int initCheck;
    const unsigned char* dictionary;
    unsigned char* bufferStart;   /* obsolete, used for slideInputBuffer */
    unsigned int dictSize;
} LZ4_stream_t_internal;

typedef struct {
    const unsigned char* externalDict;
    size_t extDictSize;
    const unsigned char* prefixEnd;
    size_t prefixSize;
} LZ4_streamDecode_t_internal;

#endif

/*!
 * LZ4_stream_t :
 * information structure to track an LZ4 stream.
 * init this structure before first use.
 * note : only use in association with static linking !
 *        this definition is not API/ABI safe,
 *        and may change in a future version !
 */
#define LZ4_STREAMSIZE_U64 ((1 << (LZ4_MEMORY_USAGE-3)) + 4)
#define LZ4_STREAMSIZE     (LZ4_STREAMSIZE_U64 * sizeof(unsigned long long))
union LZ4_stream_u {
    unsigned long long table[LZ4_STREAMSIZE_U64];
    LZ4_stream_t_internal internal_donotuse;
} ;  /* previously typedef'd to LZ4_stream_t */


/*!
 * LZ4_streamDecode_t :
 * information structure to track an LZ4 stream during decompression.
 * init this structure  using LZ4_setStreamDecode (or memset()) before first use
 * note : only use in association with static linking !
 *        this definition is not API/ABI safe,
 *        and may change in a future version !
 */
#define LZ4_STREAMDECODESIZE_U64  4
#define LZ4_STREAMDECODESIZE     (LZ4_STREAMDECODESIZE_U64 * sizeof(unsigned long long))
union LZ4_streamDecode_u {
    unsigned long long table[LZ4_STREAMDECODESIZE_U64];
    LZ4_streamDecode_t_internal internal_donotuse;
} ;   /* previously typedef'd to LZ4_streamDecode_t */


/*-************************************
*  Obsolete Functions
**************************************/

/*! Deprecation warnings
   Should deprecation warnings be a problem,
   it is generally possible to disable them,
   typically with -Wno-deprecated-declarations for gcc
   or _CRT_SECURE_NO_WARNINGS in Visual.
   Otherwise, it's also possible to define LZ4_DISABLE_DEPRECATE_WARNINGS */
#ifdef LZ4_DISABLE_DEPRECATE_WARNINGS
#  define LZ4_DEPRECATED(message)   /* disable deprecation warnings */
#else
#  define LZ4_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#  if defined (__cplusplus) && (__cplusplus >= 201402) /* C++14 or greater */
#    define LZ4_DEPRECATED(message) [[deprecated(message)]]
#  elif (LZ4_GCC_VERSION >= 405) || defined(__clang__)
#    define LZ4_DEPRECATED(message) __attribute__((deprecated(message)))
#  elif (LZ4_GCC_VERSION >= 301)
#    define LZ4_DEPRECATED(message) __attribute__((deprecated))
#  elif defined(_MSC_VER)
#    define LZ4_DEPRECATED(message) __declspec(deprecated(message))
#  else
#    pragma message("WARNING: You need to implement LZ4_DEPRECATED for this compiler")
#    define LZ4_DEPRECATED(message)
#  endif
#endif /* LZ4_DISABLE_DEPRECATE_WARNINGS */

/* Obsolete compression functions */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_default() instead") int LZ4_compress               (const char* source, char* dest, int sourceSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_default() instead") int LZ4_compress_limitedOutput (const char* source, char* dest, int sourceSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") int LZ4_compress_withState               (void* state, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_extState() instead") int LZ4_compress_limitedOutput_withState (void* state, const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") int LZ4_compress_continue                (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_fast_continue() instead") int LZ4_compress_limitedOutput_continue  (LZ4_stream_t* LZ4_streamPtr, const char* source, char* dest, int inputSize, int maxOutputSize);

/* Obsolete decompression functions */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_fast() instead") int LZ4_uncompress (const char* source, char* dest, int outputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_safe() instead") int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize);

/* Obsolete streaming functions; use new streaming interface whenever possible */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStream() instead") void* LZ4_create (char* inputBuffer);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStream() instead") int   LZ4_sizeofStreamState(void);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_resetStream() instead")  int   LZ4_resetStreamState(void* state, char* inputBuffer);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_saveDict() instead")     char* LZ4_slideInputBuffer (void* state);

/* Obsolete streaming decoding functions */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_safe_usingDict() instead") int LZ4_decompress_safe_withPrefix64k (const char* src, char* dst, int compressedSize, int maxDstSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_decompress_fast_usingDict() instead") int LZ4_decompress_fast_withPrefix64k (const char* src, char* dst, int originalSize);

#endif /* LZ4_H_2983827168210 */

#if defined (__cplusplus)
}
#endif
/*
   LZ4 - Fast LZ compression algorithm
   Copyright (C) 2011-2017, Yann Collet.

   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
    - LZ4 homepage : http://www.lz4.org
    - LZ4 source repository : https://github.com/lz4/lz4
*/


/*-************************************
*  Tuning parameters
**************************************/
/*
 * HEAPMODE :
 * Select how default compression functions will allocate memory for their hash table,
 * in memory stack (0:default, fastest), or in memory heap (1:requires malloc()).
 */
#ifndef HEAPMODE
#  define HEAPMODE 0
#endif

/*
 * ACCELERATION_DEFAULT :
 * Select "acceleration" for LZ4_compress_fast() when parameter value <= 0
 */
#define ACCELERATION_DEFAULT 1


/*-************************************
*  CPU Feature Detection
**************************************/
/* LZ4_FORCE_MEMORY_ACCESS
 * By default, access to unaligned memory is controlled by `memcpy()`, which is safe and portable.
 * Unfortunately, on some target/compiler combinations, the generated assembly is sub-optimal.
 * The below switch allow to select different access method for improved performance.
 * Method 0 (default) : use `memcpy()`. Safe and portable.
 * Method 1 : `__packed` statement. It depends on compiler extension (ie, not portable).
 *            This method is safe if your compiler supports it, and *generally* as fast or faster than `memcpy`.
 * Method 2 : direct access. This method is portable but violate C standard.
 *            It can generate buggy code on targets which assembly generation depends on alignment.
 *            But in some circumstances, it's the only known way to get the most performance (ie GCC + ARMv6)
 * See https://fastcompression.blogspot.fr/2015/08/accessing-unaligned-memory.html for details.
 * Prefer these methods in priority order (0 > 1 > 2)
 */
#ifndef LZ4_FORCE_MEMORY_ACCESS   /* can be defined externally */
#  if defined(__GNUC__) && ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) )
#    define LZ4_FORCE_MEMORY_ACCESS 2
#  elif defined(__INTEL_COMPILER) || defined(__GNUC__)
#    define LZ4_FORCE_MEMORY_ACCESS 1
#  endif
#endif

/*
 * LZ4_FORCE_SW_BITCOUNT
 * Define this parameter if your target system or compiler does not support hardware bit count
 */
#if defined(_MSC_VER) && defined(_WIN32_WCE)   /* Visual Studio for Windows CE does not support Hardware bit count */
#  define LZ4_FORCE_SW_BITCOUNT
#endif


/*-************************************
*  Dependency
**************************************/
/* see also "memory routines" below */


/*-************************************
*  Compiler Options
**************************************/
#ifdef _MSC_VER    /* Visual Studio */
#  define FORCE_INLINE static __forceinline
#  pragma warning(disable : 4127)        /* disable: C4127: conditional expression is constant */
#  pragma warning(disable : 4293)        /* disable: C4293: too large shift (32-bits) */
#else
#  if defined(__GNUC__) || defined(__clang__)
#    define FORCE_INLINE static inline __attribute__((always_inline))
#  elif defined(__cplusplus) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
#    define FORCE_INLINE static inline
#  else
#    define FORCE_INLINE static
#  endif
#endif  /* _MSC_VER */

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 800)) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#define likely(expr)     expect((expr) != 0, 1)
#define unlikely(expr)   expect((expr) != 0, 0)


/*-************************************
*  Memory routines
**************************************/
#define ALLOCATOR(n,s) calloc(n,s)
#define FREEMEM        free
#define MEM_INIT       memset


/*-************************************
*  Basic Types
**************************************/
#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
  typedef  uint8_t U8;
  typedef uint16_t U16;
  typedef uint32_t U32;
  typedef  int32_t S32;
  typedef uint64_t U64;
  typedef uintptr_t uptrval;
#else
  typedef unsigned char       U8;
  typedef unsigned short      U16;
  typedef unsigned int        U32;
  typedef   signed int        S32;
  typedef unsigned long long  U64;
  typedef size_t              uptrval;   /* generally true, except OpenVMS-64 */
#endif

#if defined(__x86_64__)
  typedef U64    reg_t;   /* 64-bits in x32 mode */
#else
  typedef size_t reg_t;   /* 32-bits in x32 mode */
#endif

/*-************************************
*  Reading and writing into memory
**************************************/
static unsigned LZ4_isLittleEndian(void)
{
    const union { U32 u; U8 c[4]; } one = { 1 };   /* don't use static : performance detrimental */
    return one.c[0];
}


#if defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==2)
/* lie to the compiler about data alignment; use with caution */

static U16 LZ4_read16(const void* memPtr) { return *(const U16*) memPtr; }
static U32 LZ4_read32(const void* memPtr) { return *(const U32*) memPtr; }
static reg_t LZ4_read_ARCH(const void* memPtr) { return *(const reg_t*) memPtr; }

static void LZ4_write16(void* memPtr, U16 value) { *(U16*)memPtr = value; }
static void LZ4_write32(void* memPtr, U32 value) { *(U32*)memPtr = value; }

#elif defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==1)

/* __pack instructions are safer, but compiler specific, hence potentially problematic for some compilers */
/* currently only defined for gcc and icc */
typedef union { U16 u16; U32 u32; reg_t uArch; } __attribute__((packed)) unalign;

static U16 LZ4_read16(const void* ptr) { return ((const unalign*)ptr)->u16; }
static U32 LZ4_read32(const void* ptr) { return ((const unalign*)ptr)->u32; }
static reg_t LZ4_read_ARCH(const void* ptr) { return ((const unalign*)ptr)->uArch; }

static void LZ4_write16(void* memPtr, U16 value) { ((unalign*)memPtr)->u16 = value; }
static void LZ4_write32(void* memPtr, U32 value) { ((unalign*)memPtr)->u32 = value; }

#else  /* safe and portable access through memcpy() */

static U16 LZ4_read16(const void* memPtr)
{
    U16 val; memcpy(&val, memPtr, sizeof(val)); return val;
}

static U32 LZ4_read32(const void* memPtr)
{
    U32 val; memcpy(&val, memPtr, sizeof(val)); return val;
}

static reg_t LZ4_read_ARCH(const void* memPtr)
{
    reg_t val; memcpy(&val, memPtr, sizeof(val)); return val;
}

static void LZ4_write16(void* memPtr, U16 value)
{
    memcpy(memPtr, &value, sizeof(value));
}

static void LZ4_write32(void* memPtr, U32 value)
{
    memcpy(memPtr, &value, sizeof(value));
}

#endif /* LZ4_FORCE_MEMORY_ACCESS */


static U16 LZ4_readLE16(const void* memPtr)
{
    if (LZ4_isLittleEndian()) {
        return LZ4_read16(memPtr);
    } else {
        const U8* p = (const U8*)memPtr;
        return (U16)((U16)p[0] + (p[1]<<8));
    }
}

static void LZ4_writeLE16(void* memPtr, U16 value)
{
    if (LZ4_isLittleEndian()) {
        LZ4_write16(memPtr, value);
    } else {
        U8* p = (U8*)memPtr;
        p[0] = (U8) value;
        p[1] = (U8)(value>>8);
    }
}

static void LZ4_copy8(void* dst, const void* src)
{
    memcpy(dst,src,8);
}

/* customized variant of memcpy, which can overwrite up to 8 bytes beyond dstEnd */
static void LZ4_wildCopy(void* dstPtr, const void* srcPtr, void* dstEnd)
{
    U8* d = (U8*)dstPtr;
    const U8* s = (const U8*)srcPtr;
    U8* const e = (U8*)dstEnd;

    do { LZ4_copy8(d,s); d+=8; s+=8; } while (d<e);
}


/*-************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (WILDCOPYLENGTH+MINMATCH)
static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define MAXD_LOG 16
#define MAX_DISTANCE ((1 << MAXD_LOG) - 1)

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)


/*-************************************
*  Common Utils
**************************************/
#define LZ4_STATIC_ASSERT(c)    { enum { LZ4_static_assert = 1/(int)(!!(c)) }; }   /* use only *after* variable declarations */


/*-************************************
*  Common functions
**************************************/
static unsigned LZ4_NbCommonBytes (register reg_t val)
{
    if (LZ4_isLittleEndian()) {
        if (sizeof(val)==8) {
#       if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r = 0;
            _BitScanForward64( &r, (U64)val );
            return (int)(r>>3);
#       elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (__builtin_ctzll((U64)val) >> 3);
#       else
            static const int DeBruijnBytePos[64] = { 0, 0, 0, 0, 0, 1, 1, 2, 0, 3, 1, 3, 1, 4, 2, 7, 0, 2, 3, 6, 1, 5, 3, 5, 1, 3, 4, 4, 2, 5, 6, 7, 7, 0, 1, 2, 3, 3, 4, 6, 2, 6, 5, 5, 3, 4, 5, 6, 7, 1, 2, 4, 6, 4, 4, 5, 7, 2, 6, 5, 7, 6, 7, 7 };
            return DeBruijnBytePos[((U64)((val & -(long long)val) * 0x0218A392CDABBD3FULL)) >> 58];
#       endif
        } else /* 32 bits */ {
#       if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r;
            _BitScanForward( &r, (U32)val );
            return (int)(r>>3);
#       elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (__builtin_ctz((U32)val) >> 3);
#       else
            static const int DeBruijnBytePos[32] = { 0, 0, 3, 0, 3, 1, 3, 0, 3, 2, 2, 1, 3, 2, 0, 1, 3, 3, 1, 2, 2, 2, 2, 0, 3, 1, 2, 0, 1, 0, 1, 1 };
            return DeBruijnBytePos[((U32)((val & -(S32)val) * 0x077CB531U)) >> 27];
#       endif
        }
    } else   /* Big Endian CPU */ {
        if (sizeof(val)==8) {
#       if defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r = 0;
            _BitScanReverse64( &r, val );
            return (unsigned)(r>>3);
#       elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (__builtin_clzll((U64)val) >> 3);
#       else
            unsigned r;
            if (!(val>>32)) { r=4; } else { r=0; val>>=32; }
            if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
            r += (!val);
            return r;
#       endif
        } else /* 32 bits */ {
#       if defined(_MSC_VER) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r = 0;
            _BitScanReverse( &r, (unsigned long)val );
            return (unsigned)(r>>3);
#       elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3))) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (__builtin_clz((U32)val) >> 3);
#       else
            unsigned r;
            if (!(val>>16)) { r=2; val>>=8; } else { r=0; val>>=24; }
            r += (!val);
            return r;
#       endif
        }
    }
}

#define STEPSIZE sizeof(reg_t)
static unsigned LZ4_count(const U8* pIn, const U8* pMatch, const U8* pInLimit)
{
    const U8* const pStart = pIn;

    while (likely(pIn<pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) { pIn+=STEPSIZE; pMatch+=STEPSIZE; continue; }
        pIn += LZ4_NbCommonBytes(diff);
        return (unsigned)(pIn - pStart);
    }

    if ((STEPSIZE==8) && (pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn))) { pIn+=4; pMatch+=4; }
    if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn))) { pIn+=2; pMatch+=2; }
    if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
    return (unsigned)(pIn - pStart);
}


#ifndef LZ4_COMMONDEFS_ONLY
/*-************************************
*  Local Constants
**************************************/
static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
static const U32 LZ4_skipTrigger = 6;  /* Increase this value ==> compression run slower on incompressible data */


/*-************************************
*  Local Structures and types
**************************************/
typedef enum {
    noLimit = 0,notLimited = 0,
    limitedOutput = 1,
    limitedDestSize = 2,
} limitedOutput_directive;
typedef enum { byPtr, byU32, byU16 } tableType_t;

typedef enum { noDict = 0, withPrefix64k, usingExtDict } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { full = 0, partial = 1 } earlyEnd_directive;


/*-************************************
*  Local Utils
**************************************/
int LZ4_versionNumber (void) { return LZ4_VERSION_NUMBER; }
const char* LZ4_versionString(void) { return LZ4_VERSION_STRING; }
int LZ4_compressBound(int isize)  { return LZ4_COMPRESSBOUND(isize); }
int LZ4_sizeofState() { return LZ4_STREAMSIZE; }


/*-******************************
*  Compression functions
********************************/
static U32 LZ4_hash4(U32 sequence, tableType_t const tableType)
{
    if (tableType == byU16)
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-(LZ4_HASHLOG+1)));
    else
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-LZ4_HASHLOG));
}

static U32 LZ4_hash5(U64 sequence, tableType_t const tableType)
{
    static const U64 prime5bytes = 889523592379ULL;
    static const U64 prime8bytes = 11400714785074694791ULL;
    const U32 hashLog = (tableType == byU16) ? LZ4_HASHLOG+1 : LZ4_HASHLOG;
    if (LZ4_isLittleEndian())
        return (U32)(((sequence << 24) * prime5bytes) >> (64 - hashLog));
    else
        return (U32)(((sequence >> 24) * prime8bytes) >> (64 - hashLog));
}

FORCE_INLINE U32 LZ4_hashPosition(const void* const p, tableType_t const tableType)
{
    if ((sizeof(reg_t)==8) && (tableType != byU16)) return LZ4_hash5(LZ4_read_ARCH(p), tableType);
    return LZ4_hash4(LZ4_read32(p), tableType);
}

static void LZ4_putPositionOnHash(const U8* p, U32 h, void* tableBase, tableType_t const tableType, const U8* srcBase)
{
    switch (tableType)
    {
    case byPtr: { const U8** hashTable = (const U8**)tableBase; hashTable[h] = p; return; }
    case byU32: { U32* hashTable = (U32*) tableBase; hashTable[h] = (U32)(p-srcBase); return; }
    case byU16: { U16* hashTable = (U16*) tableBase; hashTable[h] = (U16)(p-srcBase); return; }
    }
}

FORCE_INLINE void LZ4_putPosition(const U8* p, void* tableBase, tableType_t tableType, const U8* srcBase)
{
    U32 const h = LZ4_hashPosition(p, tableType);
    LZ4_putPositionOnHash(p, h, tableBase, tableType, srcBase);
}

static const U8* LZ4_getPositionOnHash(U32 h, void* tableBase, tableType_t tableType, const U8* srcBase)
{
    if (tableType == byPtr) { const U8** hashTable = (const U8**) tableBase; return hashTable[h]; }
    if (tableType == byU32) { const U32* const hashTable = (U32*) tableBase; return hashTable[h] + srcBase; }
    { const U16* const hashTable = (U16*) tableBase; return hashTable[h] + srcBase; }   /* default, to ensure a return */
}

FORCE_INLINE const U8* LZ4_getPosition(const U8* p, void* tableBase, tableType_t tableType, const U8* srcBase)
{
    U32 const h = LZ4_hashPosition(p, tableType);
    return LZ4_getPositionOnHash(h, tableBase, tableType, srcBase);
}


/** LZ4_compress_generic() :
    inlined, to ensure branches are decided at compilation time */
FORCE_INLINE int LZ4_compress_generic(
                 LZ4_stream_t_internal* const cctx,
                 const char* const source,
                 char* const dest,
                 const int inputSize,
                 const int maxOutputSize,
                 const limitedOutput_directive outputLimited,
                 const tableType_t tableType,
                 const dict_directive dict,
                 const dictIssue_directive dictIssue,
                 const U32 acceleration)
{
    const U8* ip = (const U8*) source;
    const U8* base;
    const U8* lowLimit;
    const U8* const lowRefLimit = ip - cctx->dictSize;
    const U8* const dictionary = cctx->dictionary;
    const U8* const dictEnd = dictionary + cctx->dictSize;
    const ptrdiff_t dictDelta = dictEnd - (const U8*)source;
    const U8* anchor = (const U8*) source;
    const U8* const iend = ip + inputSize;
    const U8* const mflimit = iend - MFLIMIT;
    const U8* const matchlimit = iend - LASTLITERALS;

    U8* op = (U8*) dest;
    U8* const olimit = op + maxOutputSize;

    U32 forwardH;

    /* Init conditions */
    if ((U32)inputSize > (U32)LZ4_MAX_INPUT_SIZE) return 0;   /* Unsupported inputSize, too large (or negative) */
    switch(dict)
    {
    case noDict:
    default:
        base = (const U8*)source;
        lowLimit = (const U8*)source;
        break;
    case withPrefix64k:
        base = (const U8*)source - cctx->currentOffset;
        lowLimit = (const U8*)source - cctx->dictSize;
        break;
    case usingExtDict:
        base = (const U8*)source - cctx->currentOffset;
        lowLimit = (const U8*)source;
        break;
    }
    if ((tableType == byU16) && (inputSize>=LZ4_64Klimit)) return 0;   /* Size too large (not within 64K limit) */
    if (inputSize<LZ4_minLength) goto _last_literals;                  /* Input too small, no compression (all literals) */

    /* First Byte */
    LZ4_putPosition(ip, cctx->hashTable, tableType, base);
    ip++; forwardH = LZ4_hashPosition(ip, tableType);

    /* Main Loop */
    for ( ; ; ) {
        ptrdiff_t refDelta = 0;
        const U8* match;
        U8* token;

        /* Find a match */
        {   const U8* forwardIp = ip;
            unsigned step = 1;
            unsigned searchMatchNb = acceleration << LZ4_skipTrigger;
            do {
                U32 const h = forwardH;
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimit)) goto _last_literals;

                match = LZ4_getPositionOnHash(h, cctx->hashTable, tableType, base);
                if (dict==usingExtDict) {
                    if (match < (const U8*)source) {
                        refDelta = dictDelta;
                        lowLimit = dictionary;
                    } else {
                        refDelta = 0;
                        lowLimit = (const U8*)source;
                }   }
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putPositionOnHash(ip, h, cctx->hashTable, tableType, base);

            } while ( ((dictIssue==dictSmall) ? (match < lowRefLimit) : 0)
                || ((tableType==byU16) ? 0 : (match + MAX_DISTANCE < ip))
                || (LZ4_read32(match+refDelta) != LZ4_read32(ip)) );
        }

        /* Catch up */
        while (((ip>anchor) & (match+refDelta > lowLimit)) && (unlikely(ip[-1]==match[refDelta-1]))) { ip--; match--; }

        /* Encode Literals */
        {   unsigned const litLength = (unsigned)(ip - anchor);
            token = op++;
            if ((outputLimited) &&  /* Check output buffer overflow */
                (unlikely(op + litLength + (2 + 1 + LASTLITERALS) + (litLength/255) > olimit)))
                return 0;
            if (litLength >= RUN_MASK) {
                int len = (int)litLength-RUN_MASK;
                *token = (RUN_MASK<<ML_BITS);
                for(; len >= 255 ; len-=255) *op++ = 255;
                *op++ = (U8)len;
            }
            else *token = (U8)(litLength<<ML_BITS);

            /* Copy Literals */
            LZ4_wildCopy(op, anchor, op+litLength);
            op+=litLength;
        }

_next_match:
        /* Encode Offset */
        LZ4_writeLE16(op, (U16)(ip-match)); op+=2;

        /* Encode MatchLength */
        {   unsigned matchCode;

            if ((dict==usingExtDict) && (lowLimit==dictionary)) {
                const U8* limit;
                match += refDelta;
                limit = ip + (dictEnd-match);
                if (limit > matchlimit) limit = matchlimit;
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, limit);
                ip += MINMATCH + matchCode;
                if (ip==limit) {
                    unsigned const more = LZ4_count(ip, (const U8*)source, matchlimit);
                    matchCode += more;
                    ip += more;
                }
            } else {
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);
                ip += MINMATCH + matchCode;
            }

            if ( outputLimited &&    /* Check output buffer overflow */
                (unlikely(op + (1 + LASTLITERALS) + (matchCode>>8) > olimit)) )
                return 0;
            if (matchCode >= ML_MASK) {
                *token += ML_MASK;
                matchCode -= ML_MASK;
                LZ4_write32(op, 0xFFFFFFFF);
                while (matchCode >= 4*255) op+=4, LZ4_write32(op, 0xFFFFFFFF), matchCode -= 4*255;
                op += matchCode / 255;
                *op++ = (U8)(matchCode % 255);
            } else
                *token += (U8)(matchCode);
        }

        anchor = ip;

        /* Test end of chunk */
        if (ip > mflimit) break;

        /* Fill table */
        LZ4_putPosition(ip-2, cctx->hashTable, tableType, base);

        /* Test next position */
        match = LZ4_getPosition(ip, cctx->hashTable, tableType, base);
        if (dict==usingExtDict) {
            if (match < (const U8*)source) {
                refDelta = dictDelta;
                lowLimit = dictionary;
            } else {
                refDelta = 0;
                lowLimit = (const U8*)source;
        }   }
        LZ4_putPosition(ip, cctx->hashTable, tableType, base);
        if ( ((dictIssue==dictSmall) ? (match>=lowRefLimit) : 1)
            && (match+MAX_DISTANCE>=ip)
            && (LZ4_read32(match+refDelta)==LZ4_read32(ip)) )
        { token=op++; *token=0; goto _next_match; }

        /* Prepare next loop */
        forwardH = LZ4_hashPosition(++ip, tableType);
    }

_last_literals:
    /* Encode Last Literals */
    {   size_t const lastRun = (size_t)(iend - anchor);
        if ( (outputLimited) &&  /* Check output buffer overflow */
            ((op - (U8*)dest) + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > (U32)maxOutputSize) )
            return 0;
        if (lastRun >= RUN_MASK) {
            size_t accumulator = lastRun - RUN_MASK;
            *op++ = RUN_MASK << ML_BITS;
            for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
            *op++ = (U8) accumulator;
        } else {
            *op++ = (U8)(lastRun<<ML_BITS);
        }
        memcpy(op, anchor, lastRun);
        op += lastRun;
    }

    /* End */
    return (int) (((char*)op)-dest);
}


int LZ4_compress_fast_extState(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    LZ4_stream_t_internal* ctx = &((LZ4_stream_t*)state)->internal_donotuse;
    LZ4_resetStream((LZ4_stream_t*)state);
    if (acceleration < 1) acceleration = ACCELERATION_DEFAULT;

    if (maxOutputSize >= LZ4_compressBound(inputSize)) {
        if (inputSize < LZ4_64Klimit)
            return LZ4_compress_generic(ctx, source, dest, inputSize,             0,    notLimited,                        byU16, noDict, noDictIssue, acceleration);
        else
            return LZ4_compress_generic(ctx, source, dest, inputSize,             0,    notLimited, (sizeof(void*)==8) ? byU32 : byPtr, noDict, noDictIssue, acceleration);
    } else {
        if (inputSize < LZ4_64Klimit)
            return LZ4_compress_generic(ctx, source, dest, inputSize, maxOutputSize, limitedOutput,                        byU16, noDict, noDictIssue, acceleration);
        else
            return LZ4_compress_generic(ctx, source, dest, inputSize, maxOutputSize, limitedOutput, (sizeof(void*)==8) ? byU32 : byPtr, noDict, noDictIssue, acceleration);
    }
}


int LZ4_compress_fast(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
#if (HEAPMODE)
    void* ctxPtr = ALLOCATOR(1, sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
#else
    LZ4_stream_t ctx;
    void* const ctxPtr = &ctx;
#endif

    int const result = LZ4_compress_fast_extState(ctxPtr, source, dest, inputSize, maxOutputSize, acceleration);

#if (HEAPMODE)
    FREEMEM(ctxPtr);
#endif
    return result;
}


int LZ4_compress_default(const char* source, char* dest, int inputSize, int maxOutputSize)
{
    return LZ4_compress_fast(source, dest, inputSize, maxOutputSize, 1);
}


/* hidden debug function */
/* strangely enough, gcc generates faster code when this function is uncommented, even if unused */
int LZ4_compress_fast_force(const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    LZ4_stream_t ctx;
    LZ4_resetStream(&ctx);

    if (inputSize < LZ4_64Klimit)
        return LZ4_compress_generic(&ctx.internal_donotuse, source, dest, inputSize, maxOutputSize, limitedOutput, byU16,                        noDict, noDictIssue, acceleration);
    else
        return LZ4_compress_generic(&ctx.internal_donotuse, source, dest, inputSize, maxOutputSize, limitedOutput, sizeof(void*)==8 ? byU32 : byPtr, noDict, noDictIssue, acceleration);
}


/*-******************************
*  *_destSize() variant
********************************/

static int LZ4_compress_destSize_generic(
                       LZ4_stream_t_internal* const ctx,
                 const char* const src,
                       char* const dst,
                       int*  const srcSizePtr,
                 const int targetDstSize,
                 const tableType_t tableType)
{
    const U8* ip = (const U8*) src;
    const U8* base = (const U8*) src;
    const U8* lowLimit = (const U8*) src;
    const U8* anchor = ip;
    const U8* const iend = ip + *srcSizePtr;
    const U8* const mflimit = iend - MFLIMIT;
    const U8* const matchlimit = iend - LASTLITERALS;

    U8* op = (U8*) dst;
    U8* const oend = op + targetDstSize;
    U8* const oMaxLit = op + targetDstSize - 2 /* offset */ - 8 /* because 8+MINMATCH==MFLIMIT */ - 1 /* token */;
    U8* const oMaxMatch = op + targetDstSize - (LASTLITERALS + 1 /* token */);
    U8* const oMaxSeq = oMaxLit - 1 /* token */;

    U32 forwardH;


    /* Init conditions */
    if (targetDstSize < 1) return 0;                                     /* Impossible to store anything */
    if ((U32)*srcSizePtr > (U32)LZ4_MAX_INPUT_SIZE) return 0;            /* Unsupported input size, too large (or negative) */
    if ((tableType == byU16) && (*srcSizePtr>=LZ4_64Klimit)) return 0;   /* Size too large (not within 64K limit) */
    if (*srcSizePtr<LZ4_minLength) goto _last_literals;                  /* Input too small, no compression (all literals) */

    /* First Byte */
    *srcSizePtr = 0;
    LZ4_putPosition(ip, ctx->hashTable, tableType, base);
    ip++; forwardH = LZ4_hashPosition(ip, tableType);

    /* Main Loop */
    for ( ; ; ) {
        const U8* match;
        U8* token;

        /* Find a match */
        {   const U8* forwardIp = ip;
            unsigned step = 1;
            unsigned searchMatchNb = 1 << LZ4_skipTrigger;

            do {
                U32 h = forwardH;
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimit)) goto _last_literals;

                match = LZ4_getPositionOnHash(h, ctx->hashTable, tableType, base);
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putPositionOnHash(ip, h, ctx->hashTable, tableType, base);

            } while ( ((tableType==byU16) ? 0 : (match + MAX_DISTANCE < ip))
                || (LZ4_read32(match) != LZ4_read32(ip)) );
        }

        /* Catch up */
        while ((ip>anchor) && (match > lowLimit) && (unlikely(ip[-1]==match[-1]))) { ip--; match--; }

        /* Encode Literal length */
        {   unsigned litLength = (unsigned)(ip - anchor);
            token = op++;
            if (op + ((litLength+240)/255) + litLength > oMaxLit) {
                /* Not enough space for a last match */
                op--;
                goto _last_literals;
            }
            if (litLength>=RUN_MASK) {
                unsigned len = litLength - RUN_MASK;
                *token=(RUN_MASK<<ML_BITS);
                for(; len >= 255 ; len-=255) *op++ = 255;
                *op++ = (U8)len;
            }
            else *token = (U8)(litLength<<ML_BITS);

            /* Copy Literals */
            LZ4_wildCopy(op, anchor, op+litLength);
            op += litLength;
        }

_next_match:
        /* Encode Offset */
        LZ4_writeLE16(op, (U16)(ip-match)); op+=2;

        /* Encode MatchLength */
        {   size_t matchLength = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);

            if (op + ((matchLength+240)/255) > oMaxMatch) {
                /* Match description too long : reduce it */
                matchLength = (15-1) + (oMaxMatch-op) * 255;
            }
            ip += MINMATCH + matchLength;

            if (matchLength>=ML_MASK) {
                *token += ML_MASK;
                matchLength -= ML_MASK;
                while (matchLength >= 255) { matchLength-=255; *op++ = 255; }
                *op++ = (U8)matchLength;
            }
            else *token += (U8)(matchLength);
        }

        anchor = ip;

        /* Test end of block */
        if (ip > mflimit) break;
        if (op > oMaxSeq) break;

        /* Fill table */
        LZ4_putPosition(ip-2, ctx->hashTable, tableType, base);

        /* Test next position */
        match = LZ4_getPosition(ip, ctx->hashTable, tableType, base);
        LZ4_putPosition(ip, ctx->hashTable, tableType, base);
        if ( (match+MAX_DISTANCE>=ip)
            && (LZ4_read32(match)==LZ4_read32(ip)) )
        { token=op++; *token=0; goto _next_match; }

        /* Prepare next loop */
        forwardH = LZ4_hashPosition(++ip, tableType);
    }

_last_literals:
    /* Encode Last Literals */
    {   size_t lastRunSize = (size_t)(iend - anchor);
        if (op + 1 /* token */ + ((lastRunSize+240)/255) /* litLength */ + lastRunSize /* literals */ > oend) {
            /* adapt lastRunSize to fill 'dst' */
            lastRunSize  = (oend-op) - 1;
            lastRunSize -= (lastRunSize+240)/255;
        }
        ip = anchor + lastRunSize;

        if (lastRunSize >= RUN_MASK) {
            size_t accumulator = lastRunSize - RUN_MASK;
            *op++ = RUN_MASK << ML_BITS;
            for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
            *op++ = (U8) accumulator;
        } else {
            *op++ = (U8)(lastRunSize<<ML_BITS);
        }
        memcpy(op, anchor, lastRunSize);
        op += lastRunSize;
    }

    /* End */
    *srcSizePtr = (int) (((const char*)ip)-src);
    return (int) (((char*)op)-dst);
}


static int LZ4_compress_destSize_extState (LZ4_stream_t* state, const char* src, char* dst, int* srcSizePtr, int targetDstSize)
{
    LZ4_resetStream(state);

    if (targetDstSize >= LZ4_compressBound(*srcSizePtr)) {  /* compression success is guaranteed */
        return LZ4_compress_fast_extState(state, src, dst, *srcSizePtr, targetDstSize, 1);
    } else {
        if (*srcSizePtr < LZ4_64Klimit)
            return LZ4_compress_destSize_generic(&state->internal_donotuse, src, dst, srcSizePtr, targetDstSize, byU16);
        else
            return LZ4_compress_destSize_generic(&state->internal_donotuse, src, dst, srcSizePtr, targetDstSize, sizeof(void*)==8 ? byU32 : byPtr);
    }
}


int LZ4_compress_destSize(const char* src, char* dst, int* srcSizePtr, int targetDstSize)
{
#if (HEAPMODE)
    LZ4_stream_t* ctx = (LZ4_stream_t*)ALLOCATOR(1, sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
#else
    LZ4_stream_t ctxBody;
    LZ4_stream_t* ctx = &ctxBody;
#endif

    int result = LZ4_compress_destSize_extState(ctx, src, dst, srcSizePtr, targetDstSize);

#if (HEAPMODE)
    FREEMEM(ctx);
#endif
    return result;
}



/*-******************************
*  Streaming functions
********************************/

LZ4_stream_t* LZ4_createStream(void)
{
    LZ4_stream_t* lz4s = (LZ4_stream_t*)ALLOCATOR(8, LZ4_STREAMSIZE_U64);
    LZ4_STATIC_ASSERT(LZ4_STREAMSIZE >= sizeof(LZ4_stream_t_internal));    /* A compilation error here means LZ4_STREAMSIZE is not large enough */
    LZ4_resetStream(lz4s);
    return lz4s;
}

void LZ4_resetStream (LZ4_stream_t* LZ4_stream)
{
    MEM_INIT(LZ4_stream, 0, sizeof(LZ4_stream_t));
}

int LZ4_freeStream (LZ4_stream_t* LZ4_stream)
{
    FREEMEM(LZ4_stream);
    return (0);
}


#define HASH_UNIT sizeof(reg_t)
int LZ4_loadDict (LZ4_stream_t* LZ4_dict, const char* dictionary, int dictSize)
{
    LZ4_stream_t_internal* dict = &LZ4_dict->internal_donotuse;
    const U8* p = (const U8*)dictionary;
    const U8* const dictEnd = p + dictSize;
    const U8* base;

    if ((dict->initCheck) || (dict->currentOffset > 1 GB))  /* Uninitialized structure, or reuse overflow */
        LZ4_resetStream(LZ4_dict);

    if (dictSize < (int)HASH_UNIT) {
        dict->dictionary = NULL;
        dict->dictSize = 0;
        return 0;
    }

    if ((dictEnd - p) > 64 KB) p = dictEnd - 64 KB;
    dict->currentOffset += 64 KB;
    base = p - dict->currentOffset;
    dict->dictionary = p;
    dict->dictSize = (U32)(dictEnd - p);
    dict->currentOffset += dict->dictSize;

    while (p <= dictEnd-HASH_UNIT) {
        LZ4_putPosition(p, dict->hashTable, byU32, base);
        p+=3;
    }

    return dict->dictSize;
}


static void LZ4_renormDictT(LZ4_stream_t_internal* LZ4_dict, const U8* src)
{
    if ((LZ4_dict->currentOffset > 0x80000000) ||
        ((uptrval)LZ4_dict->currentOffset > (uptrval)src)) {   /* address space overflow */
        /* rescale hash table */
        U32 const delta = LZ4_dict->currentOffset - 64 KB;
        const U8* dictEnd = LZ4_dict->dictionary + LZ4_dict->dictSize;
        int i;
        for (i=0; i<LZ4_HASH_SIZE_U32; i++) {
            if (LZ4_dict->hashTable[i] < delta) LZ4_dict->hashTable[i]=0;
            else LZ4_dict->hashTable[i] -= delta;
        }
        LZ4_dict->currentOffset = 64 KB;
        if (LZ4_dict->dictSize > 64 KB) LZ4_dict->dictSize = 64 KB;
        LZ4_dict->dictionary = dictEnd - LZ4_dict->dictSize;
    }
}


int LZ4_compress_fast_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    LZ4_stream_t_internal* streamPtr = &LZ4_stream->internal_donotuse;
    const U8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;

    const U8* smallest = (const U8*) source;
    if (streamPtr->initCheck) return 0;   /* Uninitialized structure detected */
    if ((streamPtr->dictSize>0) && (smallest>dictEnd)) smallest = dictEnd;
    LZ4_renormDictT(streamPtr, smallest);
    if (acceleration < 1) acceleration = ACCELERATION_DEFAULT;

    /* Check overlapping input/dictionary space */
    {   const U8* sourceEnd = (const U8*) source + inputSize;
        if ((sourceEnd > streamPtr->dictionary) && (sourceEnd < dictEnd)) {
            streamPtr->dictSize = (U32)(dictEnd - sourceEnd);
            if (streamPtr->dictSize > 64 KB) streamPtr->dictSize = 64 KB;
            if (streamPtr->dictSize < 4) streamPtr->dictSize = 0;
            streamPtr->dictionary = dictEnd - streamPtr->dictSize;
        }
    }

    /* prefix mode : source data follows dictionary */
    if (dictEnd == (const U8*)source) {
        int result;
        if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
            result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byU32, withPrefix64k, dictSmall, acceleration);
        else
            result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byU32, withPrefix64k, noDictIssue, acceleration);
        streamPtr->dictSize += (U32)inputSize;
        streamPtr->currentOffset += (U32)inputSize;
        return result;
    }

    /* external dictionary mode */
    {   int result;
        if ((streamPtr->dictSize < 64 KB) && (streamPtr->dictSize < streamPtr->currentOffset))
            result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byU32, usingExtDict, dictSmall, acceleration);
        else
            result = LZ4_compress_generic(streamPtr, source, dest, inputSize, maxOutputSize, limitedOutput, byU32, usingExtDict, noDictIssue, acceleration);
        streamPtr->dictionary = (const U8*)source;
        streamPtr->dictSize = (U32)inputSize;
        streamPtr->currentOffset += (U32)inputSize;
        return result;
    }
}


/* Hidden debug function, to force external dictionary mode */
int LZ4_compress_forceExtDict (LZ4_stream_t* LZ4_dict, const char* source, char* dest, int inputSize)
{
    LZ4_stream_t_internal* streamPtr = &LZ4_dict->internal_donotuse;
    int result;
    const U8* const dictEnd = streamPtr->dictionary + streamPtr->dictSize;

    const U8* smallest = dictEnd;
    if (smallest > (const U8*) source) smallest = (const U8*) source;
    LZ4_renormDictT(streamPtr, smallest);

    result = LZ4_compress_generic(streamPtr, source, dest, inputSize, 0, notLimited, byU32, usingExtDict, noDictIssue, 1);

    streamPtr->dictionary = (const U8*)source;
    streamPtr->dictSize = (U32)inputSize;
    streamPtr->currentOffset += (U32)inputSize;

    return result;
}


/*! LZ4_saveDict() :
 *  If previously compressed data block is not guaranteed to remain available at its memory location,
 *  save it into a safer place (char* safeBuffer).
 *  Note : you don't need to call LZ4_loadDict() afterwards,
 *         dictionary is immediately usable, you can therefore call LZ4_compress_fast_continue().
 *  Return : saved dictionary size in bytes (necessarily <= dictSize), or 0 if error.
 */
int LZ4_saveDict (LZ4_stream_t* LZ4_dict, char* safeBuffer, int dictSize)
{
    LZ4_stream_t_internal* const dict = &LZ4_dict->internal_donotuse;
    const U8* const previousDictEnd = dict->dictionary + dict->dictSize;

    if ((U32)dictSize > 64 KB) dictSize = 64 KB;   /* useless to define a dictionary > 64 KB */
    if ((U32)dictSize > dict->dictSize) dictSize = dict->dictSize;

    memmove(safeBuffer, previousDictEnd - dictSize, dictSize);

    dict->dictionary = (const U8*)safeBuffer;
    dict->dictSize = (U32)dictSize;

    return dictSize;
}



/*-*****************************
*  Decompression functions
*******************************/
/*! LZ4_decompress_generic() :
 *  This generic decompression function cover all use cases.
 *  It shall be instantiated several times, using different sets of directives
 *  Note that it is important this generic function is really inlined,
 *  in order to remove useless branches during compilation optimization.
 */
FORCE_INLINE int LZ4_decompress_generic(
                 const char* const source,
                 char* const dest,
                 int inputSize,
                 int outputSize,         /* If endOnInput==endOnInputSize, this value is the max size of Output Buffer. */

                 int endOnInput,         /* endOnOutputSize, endOnInputSize */
                 int partialDecoding,    /* full, partial */
                 int targetOutputSize,   /* only used if partialDecoding==partial */
                 int dict,               /* noDict, withPrefix64k, usingExtDict */
                 const U8* const lowPrefix,  /* == dest when no prefix */
                 const U8* const dictStart,  /* only if dict==usingExtDict */
                 const size_t dictSize         /* note : = 0 if noDict */
                 )
{
    /* Local Variables */
    const U8* ip = (const U8*) source;
    const U8* const iend = ip + inputSize;

    U8* op = (U8*) dest;
    U8* const oend = op + outputSize;
    U8* cpy;
    U8* oexit = op + targetOutputSize;
    const U8* const lowLimit = lowPrefix - dictSize;

    const U8* const dictEnd = (const U8*)dictStart + dictSize;
    const unsigned dec32table[] = {0, 1, 2, 1, 4, 4, 4, 4};
    const int dec64table[] = {0, 0, 0, -1, 0, 1, 2, 3};

    const int safeDecode = (endOnInput==endOnInputSize);
    const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));


    /* Special cases */
    if ((partialDecoding) && (oexit > oend-MFLIMIT)) oexit = oend-MFLIMIT;                        /* targetOutputSize too high => decode everything */
    if ((endOnInput) && (unlikely(outputSize==0))) return ((inputSize==1) && (*ip==0)) ? 0 : -1;  /* Empty output buffer */
    if ((!endOnInput) && (unlikely(outputSize==0))) return (*ip==0?1:-1);

    /* Main Loop : decode sequences */
    while (1) {
        size_t length;
        const U8* match;
        size_t offset;

        /* get literal length */
        unsigned const token = *ip++;
        if ((length=(token>>ML_BITS)) == RUN_MASK) {
            unsigned s;
            do {
                s = *ip++;
                length += s;
            } while ( likely(endOnInput ? ip<iend-RUN_MASK : 1) & (s==255) );
            if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)(op))) goto _output_error;   /* overflow detection */
            if ((safeDecode) && unlikely((uptrval)(ip)+length<(uptrval)(ip))) goto _output_error;   /* overflow detection */
        }

        /* copy literals */
        cpy = op+length;
        if ( ((endOnInput) && ((cpy>(partialDecoding?oexit:oend-MFLIMIT)) || (ip+length>iend-(2+1+LASTLITERALS))) )
            || ((!endOnInput) && (cpy>oend-WILDCOPYLENGTH)) )
        {
            if (partialDecoding) {
                if (cpy > oend) goto _output_error;                           /* Error : write attempt beyond end of output buffer */
                if ((endOnInput) && (ip+length > iend)) goto _output_error;   /* Error : read attempt beyond end of input buffer */
            } else {
                if ((!endOnInput) && (cpy != oend)) goto _output_error;       /* Error : block decoding must stop exactly there */
                if ((endOnInput) && ((ip+length != iend) || (cpy > oend))) goto _output_error;   /* Error : input must be consumed */
            }
            memcpy(op, ip, length);
            ip += length;
            op += length;
            break;     /* Necessarily EOF, due to parsing restrictions */
        }
        LZ4_wildCopy(op, ip, cpy);
        ip += length; op = cpy;

        /* get offset */
        offset = LZ4_readLE16(ip); ip+=2;
        match = op - offset;
        if ((checkOffset) && (unlikely(match < lowLimit))) goto _output_error;   /* Error : offset outside buffers */
        LZ4_write32(op, (U32)offset);   /* costs ~1%; silence an msan warning when offset==0 */

        /* get matchlength */
        length = token & ML_MASK;
        if (length == ML_MASK) {
            unsigned s;
            do {
                s = *ip++;
                if ((endOnInput) && (ip > iend-LASTLITERALS)) goto _output_error;
                length += s;
            } while (s==255);
            if ((safeDecode) && unlikely((uptrval)(op)+length<(uptrval)op)) goto _output_error;   /* overflow detection */
        }
        length += MINMATCH;

        /* check external dictionary */
        if ((dict==usingExtDict) && (match < lowPrefix)) {
            if (unlikely(op+length > oend-LASTLITERALS)) goto _output_error;   /* doesn't respect parsing restriction */

            if (length <= (size_t)(lowPrefix-match)) {
                /* match can be copied as a single segment from external dictionary */
                memmove(op, dictEnd - (lowPrefix-match), length);
                op += length;
            } else {
                /* match encompass external dictionary and current block */
                size_t const copySize = (size_t)(lowPrefix-match);
                size_t const restSize = length - copySize;
                memcpy(op, dictEnd - copySize, copySize);
                op += copySize;
                if (restSize > (size_t)(op-lowPrefix)) {  /* overlap copy */
                    U8* const endOfMatch = op + restSize;
                    const U8* copyFrom = lowPrefix;
                    while (op < endOfMatch) *op++ = *copyFrom++;
                } else {
                    memcpy(op, lowPrefix, restSize);
                    op += restSize;
            }   }
            continue;
        }

        /* copy match within block */
        cpy = op + length;
        if (unlikely(offset<8)) {
            const int dec64 = dec64table[offset];
            op[0] = match[0];
            op[1] = match[1];
            op[2] = match[2];
            op[3] = match[3];
            match += dec32table[offset];
            memcpy(op+4, match, 4);
            match -= dec64;
        } else { LZ4_copy8(op, match); match+=8; }
        op += 8;

        if (unlikely(cpy>oend-12)) {
            U8* const oCopyLimit = oend-(WILDCOPYLENGTH-1);
            if (cpy > oend-LASTLITERALS) goto _output_error;    /* Error : last LASTLITERALS bytes must be literals (uncompressed) */
            if (op < oCopyLimit) {
                LZ4_wildCopy(op, match, oCopyLimit);
                match += oCopyLimit - op;
                op = oCopyLimit;
            }
            while (op<cpy) *op++ = *match++;
        } else {
            LZ4_copy8(op, match);
            if (length>16) LZ4_wildCopy(op+8, match+8, cpy);
        }
        op=cpy;   /* correction */
    }

    /* end of decoding */
    if (endOnInput)
       return (int) (((char*)op)-dest);     /* Nb of output bytes decoded */
    else
       return (int) (((const char*)ip)-source);   /* Nb of input bytes read */

    /* Overflow error detected */
_output_error:
    return (int) (-(((const char*)ip)-source))-1;
}


int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, full, 0, noDict, (U8*)dest, NULL, 0);
}

int LZ4_decompress_safe_partial(const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, partial, targetOutputSize, noDict, (U8*)dest, NULL, 0);
}

int LZ4_decompress_fast(const char* source, char* dest, int originalSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (U8*)(dest - 64 KB), NULL, 64 KB);
}


/*===== streaming decompression functions =====*/

/*
 * If you prefer dynamic allocation methods,
 * LZ4_createStreamDecode()
 * provides a pointer (void*) towards an initialized LZ4_streamDecode_t structure.
 */
LZ4_streamDecode_t* LZ4_createStreamDecode(void)
{
    LZ4_streamDecode_t* lz4s = (LZ4_streamDecode_t*) ALLOCATOR(1, sizeof(LZ4_streamDecode_t));
    return lz4s;
}

int LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream)
{
    FREEMEM(LZ4_stream);
    return 0;
}

/*!
 * LZ4_setStreamDecode() :
 * Use this function to instruct where to find the dictionary.
 * This function is not necessary if previous data is still available where it was decoded.
 * Loading a size of 0 is allowed (same effect as no dictionary).
 * Return : 1 if OK, 0 if error
 */
int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    lz4sd->prefixSize = (size_t) dictSize;
    lz4sd->prefixEnd = (const U8*) dictionary + dictSize;
    lz4sd->externalDict = NULL;
    lz4sd->extDictSize  = 0;
    return 1;
}

/*
*_continue() :
    These decoding functions allow decompression of multiple blocks in "streaming" mode.
    Previously decoded blocks must still be available at the memory position where they were decoded.
    If it's not possible, save the relevant part of decoded data into a safe buffer,
    and indicate where it stands using LZ4_setStreamDecode()
*/
int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxOutputSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    int result;

    if (lz4sd->prefixEnd == (U8*)dest) {
        result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                        endOnInputSize, full, 0,
                                        usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += result;
        lz4sd->prefixEnd  += result;
    } else {
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
        result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                        endOnInputSize, full, 0,
                                        usingExtDict, (U8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = result;
        lz4sd->prefixEnd  = (U8*)dest + result;
    }

    return result;
}

int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    int result;

    if (lz4sd->prefixEnd == (U8*)dest) {
        result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                        endOnOutputSize, full, 0,
                                        usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += originalSize;
        lz4sd->prefixEnd  += originalSize;
    } else {
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
        result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                        endOnOutputSize, full, 0,
                                        usingExtDict, (U8*)dest, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = originalSize;
        lz4sd->prefixEnd  = (U8*)dest + originalSize;
    }

    return result;
}


/*
Advanced decoding functions :
*_usingDict() :
    These decoding functions work the same as "_continue" ones,
    the dictionary must be explicitly provided within parameters
*/

FORCE_INLINE int LZ4_decompress_usingDict_generic(const char* source, char* dest, int compressedSize, int maxOutputSize, int safe, const char* dictStart, int dictSize)
{
    if (dictSize==0)
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (U8*)dest, NULL, 0);
    if (dictStart+dictSize == dest) {
        if (dictSize >= (int)(64 KB - 1))
            return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, withPrefix64k, (U8*)dest-64 KB, NULL, 0);
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (U8*)dest-dictSize, NULL, 0);
    }
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, usingExtDict, (U8*)dest, (const U8*)dictStart, dictSize);
}

int LZ4_decompress_safe_usingDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_usingDict_generic(source, dest, compressedSize, maxOutputSize, 1, dictStart, dictSize);
}

int LZ4_decompress_fast_usingDict(const char* source, char* dest, int originalSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_usingDict_generic(source, dest, 0, originalSize, 0, dictStart, dictSize);
}

/* debug function */
int LZ4_decompress_safe_forceExtDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, usingExtDict, (U8*)dest, (const U8*)dictStart, dictSize);
}


/*=*************************************************
*  Obsolete Functions
***************************************************/
/* obsolete compression functions */
int LZ4_compress_limitedOutput(const char* source, char* dest, int inputSize, int maxOutputSize) { return LZ4_compress_default(source, dest, inputSize, maxOutputSize); }
int LZ4_compress(const char* source, char* dest, int inputSize) { return LZ4_compress_default(source, dest, inputSize, LZ4_compressBound(inputSize)); }
int LZ4_compress_limitedOutput_withState (void* state, const char* src, char* dst, int srcSize, int dstSize) { return LZ4_compress_fast_extState(state, src, dst, srcSize, dstSize, 1); }
int LZ4_compress_withState (void* state, const char* src, char* dst, int srcSize) { return LZ4_compress_fast_extState(state, src, dst, srcSize, LZ4_compressBound(srcSize), 1); }
int LZ4_compress_limitedOutput_continue (LZ4_stream_t* LZ4_stream, const char* src, char* dst, int srcSize, int maxDstSize) { return LZ4_compress_fast_continue(LZ4_stream, src, dst, srcSize, maxDstSize, 1); }
int LZ4_compress_continue (LZ4_stream_t* LZ4_stream, const char* source, char* dest, int inputSize) { return LZ4_compress_fast_continue(LZ4_stream, source, dest, inputSize, LZ4_compressBound(inputSize), 1); }

/*
These function names are deprecated and should no longer be used.
They are only provided here for compatibility with older user programs.
- LZ4_uncompress is totally equivalent to LZ4_decompress_fast
- LZ4_uncompress_unknownOutputSize is totally equivalent to LZ4_decompress_safe
*/
int LZ4_uncompress (const char* source, char* dest, int outputSize) { return LZ4_decompress_fast(source, dest, outputSize); }
int LZ4_uncompress_unknownOutputSize (const char* source, char* dest, int isize, int maxOutputSize) { return LZ4_decompress_safe(source, dest, isize, maxOutputSize); }


/* Obsolete Streaming functions */

int LZ4_sizeofStreamState() { return LZ4_STREAMSIZE; }

static void LZ4_init(LZ4_stream_t* lz4ds, U8* base)
{
    MEM_INIT(lz4ds, 0, sizeof(LZ4_stream_t));
    lz4ds->internal_donotuse.bufferStart = base;
}

int LZ4_resetStreamState(void* state, char* inputBuffer)
{
    if ((((uptrval)state) & 3) != 0) return 1;   /* Error : pointer is not aligned on 4-bytes boundary */
    LZ4_init((LZ4_stream_t*)state, (U8*)inputBuffer);
    return 0;
}

void* LZ4_create (char* inputBuffer)
{
    LZ4_stream_t* lz4ds = (LZ4_stream_t*)ALLOCATOR(8, sizeof(LZ4_stream_t));
    LZ4_init (lz4ds, (U8*)inputBuffer);
    return lz4ds;
}

char* LZ4_slideInputBuffer (void* LZ4_Data)
{
    LZ4_stream_t_internal* ctx = &((LZ4_stream_t*)LZ4_Data)->internal_donotuse;
    int dictSize = LZ4_saveDict((LZ4_stream_t*)LZ4_Data, (char*)ctx->bufferStart, 64 KB);
    return (char*)(ctx->bufferStart + dictSize);
}

/* Obsolete streaming decompression functions */

int LZ4_decompress_safe_withPrefix64k(const char* source, char* dest, int compressedSize, int maxOutputSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, withPrefix64k, (U8*)dest - 64 KB, NULL, 64 KB);
}

int LZ4_decompress_fast_withPrefix64k(const char* source, char* dest, int originalSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (U8*)dest - 64 KB, NULL, 64 KB);
}

#endif   /* LZ4_COMMONDEFS_ONLY */
/*
   LZ4 HC - High Compression Mode of LZ4
   Header File
   Copyright (C) 2011-2017, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 source repository : https://github.com/lz4/lz4
   - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/
#ifndef LZ4_HC_H_19834876238432
#define LZ4_HC_H_19834876238432

#if defined (__cplusplus)
extern "C" {
#endif

/* --- Dependency --- */
/* note : lz4hc is not an independent module, it requires lz4.h/lz4.c for proper compilation */


/* --- Useful constants --- */
#define LZ4HC_CLEVEL_MIN         3
#define LZ4HC_CLEVEL_DEFAULT     9
#define LZ4HC_CLEVEL_OPT_MIN    11
#define LZ4HC_CLEVEL_MAX        12


/*-************************************
 *  Block Compression
 **************************************/
/*! LZ4_compress_HC() :
 * Compress data from `src` into `dst`, using the more powerful but slower "HC" algorithm.
 * `dst` must be already allocated.
 * Compression is guaranteed to succeed if `dstCapacity >= LZ4_compressBound(srcSize)` (see "lz4.h")
 * Max supported `srcSize` value is LZ4_MAX_INPUT_SIZE (see "lz4.h")
 * `compressionLevel` : Recommended values are between 4 and 9, although any value between 1 and LZ4HC_MAX_CLEVEL will work.
 *                      Values >LZ4HC_MAX_CLEVEL behave the same as LZ4HC_MAX_CLEVEL.
 * @return : the number of bytes written into 'dst'
 *           or 0 if compression fails.
 */
LZ4LIB_API int LZ4_compress_HC (const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel);


/* Note :
 *   Decompression functions are provided within "lz4.h" (BSD license)
 */


/*! LZ4_compress_HC_extStateHC() :
 * Same as LZ4_compress_HC(), but using an externally allocated memory segment for `state`.
 * `state` size is provided by LZ4_sizeofStateHC().
 * Memory segment must be aligned on 8-bytes boundaries (which a normal malloc() will do properly).
 */
LZ4LIB_API int LZ4_compress_HC_extStateHC(void* state, const char* src, char* dst, int srcSize, int maxDstSize, int compressionLevel);
LZ4LIB_API int LZ4_sizeofStateHC(void);


/*-************************************
 *  Streaming Compression
 *  Bufferless synchronous API
 **************************************/
 typedef union LZ4_streamHC_u LZ4_streamHC_t;   /* incomplete type (defined later) */

/*! LZ4_createStreamHC() and LZ4_freeStreamHC() :
 * These functions create and release memory for LZ4 HC streaming state.
 * Newly created states are automatically initialized.
 * Existing states can be re-used several times, using LZ4_resetStreamHC().
 * These methods are API and ABI stable, they can be used in combination with a DLL.
 */
LZ4LIB_API LZ4_streamHC_t* LZ4_createStreamHC(void);
LZ4LIB_API int             LZ4_freeStreamHC (LZ4_streamHC_t* streamHCPtr);

LZ4LIB_API void LZ4_resetStreamHC (LZ4_streamHC_t* streamHCPtr, int compressionLevel);
LZ4LIB_API int  LZ4_loadDictHC (LZ4_streamHC_t* streamHCPtr, const char* dictionary, int dictSize);

LZ4LIB_API int LZ4_compress_HC_continue (LZ4_streamHC_t* streamHCPtr, const char* src, char* dst, int srcSize, int maxDstSize);

LZ4LIB_API int LZ4_saveDictHC (LZ4_streamHC_t* streamHCPtr, char* safeBuffer, int maxDictSize);

/*
  These functions compress data in successive blocks of any size, using previous blocks as dictionary.
  One key assumption is that previous blocks (up to 64 KB) remain read-accessible while compressing next blocks.
  There is an exception for ring buffers, which can be smaller than 64 KB.
  Ring buffers scenario is automatically detected and handled by LZ4_compress_HC_continue().

  Before starting compression, state must be properly initialized, using LZ4_resetStreamHC().
  A first "fictional block" can then be designated as initial dictionary, using LZ4_loadDictHC() (Optional).

  Then, use LZ4_compress_HC_continue() to compress each successive block.
  Previous memory blocks (including initial dictionary when present) must remain accessible and unmodified during compression.
  'dst' buffer should be sized to handle worst case scenarios (see LZ4_compressBound()), to ensure operation success.
  Because in case of failure, the API does not guarantee context recovery, and context will have to be reset.
  If `dst` buffer budget cannot be >= LZ4_compressBound(), consider using LZ4_compress_HC_continue_destSize() instead.

  If, for any reason, previous data block can't be preserved unmodified in memory for next compression block,
  you can save it to a more stable memory space, using LZ4_saveDictHC().
  Return value of LZ4_saveDictHC() is the size of dictionary effectively saved into 'safeBuffer'.
*/


 /*-*************************************
 * PRIVATE DEFINITIONS :
 * Do not use these definitions.
 * They are exposed to allow static allocation of `LZ4_streamHC_t`.
 * Using these definitions makes the code vulnerable to potential API break when upgrading LZ4
 **************************************/
#define LZ4HC_DICTIONARY_LOGSIZE 17   /* because of btopt, hc would only need 16 */
#define LZ4HC_MAXD (1<<LZ4HC_DICTIONARY_LOGSIZE)
#define LZ4HC_MAXD_MASK (LZ4HC_MAXD - 1)

#define LZ4HC_HASH_LOG 15
#define LZ4HC_HASHTABLESIZE (1 << LZ4HC_HASH_LOG)
#define LZ4HC_HASH_MASK (LZ4HC_HASHTABLESIZE - 1)


#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)

typedef struct
{
    uint32_t   hashTable[LZ4HC_HASHTABLESIZE];
    uint16_t   chainTable[LZ4HC_MAXD];
    const uint8_t* end;         /* next block here to continue on current prefix */
    const uint8_t* base;        /* All index relative to this position */
    const uint8_t* dictBase;    /* alternate base for extDict */
    uint8_t* inputBuffer;       /* deprecated */
    uint32_t   dictLimit;       /* below that point, need extDict */
    uint32_t   lowLimit;        /* below that point, no more dict */
    uint32_t   nextToUpdate;    /* index from which to continue dictionary update */
    uint32_t   searchNum;       /* only for optimal parser */
    uint32_t   compressionLevel;
} LZ4HC_CCtx_internal;

#else

typedef struct
{
    unsigned int   hashTable[LZ4HC_HASHTABLESIZE];
    unsigned short   chainTable[LZ4HC_MAXD];
    const unsigned char* end;        /* next block here to continue on current prefix */
    const unsigned char* base;       /* All index relative to this position */
    const unsigned char* dictBase;   /* alternate base for extDict */
    unsigned char* inputBuffer;      /* deprecated */
    unsigned int   dictLimit;        /* below that point, need extDict */
    unsigned int   lowLimit;         /* below that point, no more dict */
    unsigned int   nextToUpdate;     /* index from which to continue dictionary update */
    unsigned int   searchNum;        /* only for optimal parser */
    int            compressionLevel;
} LZ4HC_CCtx_internal;

#endif

#define LZ4_STREAMHCSIZE       (4*LZ4HC_HASHTABLESIZE + 2*LZ4HC_MAXD + 56) /* 393268 */
#define LZ4_STREAMHCSIZE_SIZET (LZ4_STREAMHCSIZE / sizeof(size_t))
union LZ4_streamHC_u {
    size_t table[LZ4_STREAMHCSIZE_SIZET];
    LZ4HC_CCtx_internal internal_donotuse;
};   /* previously typedef'd to LZ4_streamHC_t */
/*
  LZ4_streamHC_t :
  This structure allows static allocation of LZ4 HC streaming state.
  State must be initialized using LZ4_resetStreamHC() before first use.

  Static allocation shall only be used in combination with static linking.
  When invoking LZ4 from a DLL, use create/free functions instead, which are API and ABI stable.
*/


/*-************************************
*  Deprecated Functions
**************************************/
/* see lz4.h LZ4_DISABLE_DEPRECATE_WARNINGS to turn off deprecation warnings */

/* deprecated compression functions */
/* these functions will trigger warning messages in future releases */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC() instead") int LZ4_compressHC               (const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC() instead") int LZ4_compressHC_limitedOutput (const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC() instead") int LZ4_compressHC2 (const char* source, char* dest, int inputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC() instead") int LZ4_compressHC2_limitedOutput (const char* source, char* dest, int inputSize, int maxOutputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_extStateHC() instead") int LZ4_compressHC_withStateHC               (void* state, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_extStateHC() instead") int LZ4_compressHC_limitedOutput_withStateHC (void* state, const char* source, char* dest, int inputSize, int maxOutputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_extStateHC() instead") int LZ4_compressHC2_withStateHC (void* state, const char* source, char* dest, int inputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_extStateHC() instead") int LZ4_compressHC2_limitedOutput_withStateHC(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_continue() instead") int LZ4_compressHC_continue               (LZ4_streamHC_t* LZ4_streamHCPtr, const char* source, char* dest, int inputSize);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_continue() instead") int LZ4_compressHC_limitedOutput_continue (LZ4_streamHC_t* LZ4_streamHCPtr, const char* source, char* dest, int inputSize, int maxOutputSize);

/* Deprecated Streaming functions using older model; should no longer be used */
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStreamHC() instead") void* LZ4_createHC (char* inputBuffer);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_saveDictHC() instead")     char* LZ4_slideInputBufferHC (void* LZ4HC_Data);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_freeStreamHC() instead")   int   LZ4_freeHC (void* LZ4HC_Data);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_continue() instead") int LZ4_compressHC2_continue (void* LZ4HC_Data, const char* source, char* dest, int inputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_compress_HC_continue() instead") int LZ4_compressHC2_limitedOutput_continue (void* LZ4HC_Data, const char* source, char* dest, int inputSize, int maxOutputSize, int compressionLevel);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_createStreamHC() instead") int   LZ4_sizeofStreamStateHC(void);
LZ4LIB_API LZ4_DEPRECATED("use LZ4_resetStreamHC() instead")  int   LZ4_resetStreamStateHC(void* state, char* inputBuffer);


#if defined (__cplusplus)
}
#endif

#endif /* LZ4_HC_H_19834876238432 */

/*-************************************************
 * !!!!!     STATIC LINKING ONLY     !!!!!
 * Following definitions are considered experimental.
 * They should not be linked from DLL,
 * as there is no guarantee of API stability yet.
 * Prototypes will be promoted to "stable" status
 * after successfull usage in real-life scenarios.
 *************************************************/
#ifdef LZ4_HC_STATIC_LINKING_ONLY   /* protection macro */
#ifndef LZ4_HC_SLO_098092834
#define LZ4_HC_SLO_098092834

/*! LZ4_compress_HC_destSize() :
 *  Will try to compress as much data from `src` as possible
 *  that can fit in `targetDstSize` budget.
 *  Result is provided in 2 parts :
 * @return : the number of bytes written into 'dst'
 *           or 0 if compression fails.
 * `srcSizePtr` : value will be updated to indicate how much bytes were read from `src`
 */
LZ4LIB_API int LZ4_compress_HC_destSize(void* LZ4HC_Data,
                            const char* src, char* dst,
                            int* srcSizePtr, int targetDstSize,
                            int compressionLevel);

/*! LZ4_compress_HC_continue_destSize() :
 *  Similar as LZ4_compress_HC_continue(),
 *  but will read a variable nb of bytes from `src`
 *  to fit into `targetDstSize` budget.
 *  Result is provided in 2 parts :
 * @return : the number of bytes written into 'dst'
 *           or 0 if compression fails.
 * `srcSizePtr` : value will be updated to indicate how much bytes were read from `src`
 * Important : due to limitations, this prototype only works well up to cLevel < LZ4HC_CLEVEL_OPT_MIN
 *             beyond that level, compression performance will be much reduced due to internal incompatibilities
 */
LZ4LIB_API int LZ4_compress_HC_continue_destSize(LZ4_streamHC_t* LZ4_streamHCPtr,
                            const char* src, char* dst,
                            int* srcSizePtr, int targetDstSize);

#endif   /* LZ4_HC_SLO_098092834 */
#endif   /* LZ4_HC_STATIC_LINKING_ONLY */
/*
    lz4opt.h - Optimal Mode of LZ4
    Copyright (C) 2015-2017, Przemyslaw Skibinski <inikep@gmail.com>

    BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    You can contact the author at :
       - LZ4 source repository : https://github.com/lz4/lz4
       - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/

#define LZ4_OPT_NUM   (1<<12)


typedef struct {
    int off;
    int len;
} LZ4HC_match_t;

typedef struct {
    int price;
    int off;
    int mlen;
    int litlen;
} LZ4HC_optimal_t;


/* price in bits */
FORCE_INLINE size_t LZ4HC_literalsPrice(size_t litlen)
{
    size_t price = litlen;
    if (litlen >= (size_t)RUN_MASK) price += 1 + (litlen-RUN_MASK)/255;
    return price;
}


/* requires mlen >= MINMATCH */
FORCE_INLINE size_t LZ4HC_sequencePrice(size_t litlen, size_t mlen)
{
    size_t price = 2 + 1; /* 16-bit offset + token */

    price += LZ4HC_literalsPrice(litlen);

    if (mlen >= (size_t)(ML_MASK+MINMATCH))
        price+= 1+(mlen-(ML_MASK+MINMATCH))/255;

    return price;
}


/*===   Common LZ4 definitions   ===*/
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wunused-function"
#endif
#if defined (__clang__)
#  pragma clang diagnostic ignored "-Wunused-function"
#endif

//#define LZ4_COMMONDEFS_ONLY


/*===   Constants   ===*/
#define OPTIMAL_ML (int)((ML_MASK-1)+MINMATCH)


/*===   Macros   ===*/
#define HASH_FUNCTION(i)       (((i) * 2654435761U) >> ((MINMATCH*8)-LZ4HC_HASH_LOG))
#define DELTANEXTMAXD(p)       chainTable[(p) & LZ4HC_MAXD_MASK]    /* flexible, LZ4HC_MAXD dependent */
#define DELTANEXTU16(p)        chainTable[(U16)(p)]   /* faster */
static U32 LZ4HC_hashPtr(const void* ptr) { return HASH_FUNCTION(LZ4_read32(ptr)); }
/*-*************************************
*  Binary Tree search
***************************************/
FORCE_INLINE int LZ4HC_BinTree_InsertAndGetAllMatches (
    LZ4HC_CCtx_internal* ctx,
    const U8* const ip,
    const U8* const iHighLimit,
    size_t best_mlen,
    LZ4HC_match_t* matches,
    int* matchNum)
{
    U16* const chainTable = ctx->chainTable;
    U32* const HashTable = ctx->hashTable;
    const U8* const base = ctx->base;
    const U32 dictLimit = ctx->dictLimit;
    const U32 current = (U32)(ip - base);
    const U32 lowLimit = (ctx->lowLimit + MAX_DISTANCE > current) ? ctx->lowLimit : current - (MAX_DISTANCE - 1);
    const U8* const dictBase = ctx->dictBase;
    const U8* match;
    int nbAttempts = ctx->searchNum;
    int mnum = 0;
    U16 *ptr0, *ptr1, delta0, delta1;
    U32 matchIndex;
    size_t matchLength = 0;
    U32* HashPos;

    if (ip + MINMATCH > iHighLimit) return 1;

    /* HC4 match finder */
    HashPos = &HashTable[LZ4HC_hashPtr(ip)];
    matchIndex = *HashPos;
    *HashPos = current;

    ptr0 = &DELTANEXTMAXD(current*2+1);
    ptr1 = &DELTANEXTMAXD(current*2);
    delta0 = delta1 = (U16)(current - matchIndex);

    while ((matchIndex < current) && (matchIndex>=lowLimit) && (nbAttempts)) {
        nbAttempts--;
        if (matchIndex >= dictLimit) {
            match = base + matchIndex;
            matchLength = LZ4_count(ip, match, iHighLimit);
        } else {
            const U8* vLimit = ip + (dictLimit - matchIndex);
            match = dictBase + matchIndex;
            if (vLimit > iHighLimit) vLimit = iHighLimit;
            matchLength = LZ4_count(ip, match, vLimit);
            if ((ip+matchLength == vLimit) && (vLimit < iHighLimit))
                matchLength += LZ4_count(ip+matchLength, base+dictLimit, iHighLimit);
        }

        if (matchLength > best_mlen) {
            best_mlen = matchLength;
            if (matches) {
                if (matchIndex >= dictLimit)
                    matches[mnum].off = (int)(ip - match);
                else
                    matches[mnum].off = (int)(ip - (base + matchIndex)); /* virtual matchpos */
                matches[mnum].len = (int)matchLength;
                mnum++;
            }
            if (best_mlen > LZ4_OPT_NUM) break;
        }

        if (ip+matchLength >= iHighLimit)   /* equal : no way to know if inf or sup */
            break;   /* drop , to guarantee consistency ; miss a bit of compression, but other solutions can corrupt the tree */

        if (*(ip+matchLength) < *(match+matchLength)) {
            *ptr0 = delta0;
            ptr0 = &DELTANEXTMAXD(matchIndex*2);
            if (*ptr0 == (U16)-1) break;
            delta0 = *ptr0;
            delta1 += delta0;
            matchIndex -= delta0;
        } else {
            *ptr1 = delta1;
            ptr1 = &DELTANEXTMAXD(matchIndex*2+1);
            if (*ptr1 == (U16)-1) break;
            delta1 = *ptr1;
            delta0 += delta1;
            matchIndex -= delta1;
        }
    }

    *ptr0 = (U16)-1;
    *ptr1 = (U16)-1;
    if (matchNum) *matchNum = mnum;
  /*  if (best_mlen > 8) return best_mlen-8; */
    if (!matchNum) return 1;
    return 1;
}


FORCE_INLINE void LZ4HC_updateBinTree(LZ4HC_CCtx_internal* ctx, const U8* const ip, const U8* const iHighLimit)
{
    const U8* const base = ctx->base;
    const U32 target = (U32)(ip - base);
    U32 idx = ctx->nextToUpdate;
    while(idx < target)
        idx += LZ4HC_BinTree_InsertAndGetAllMatches(ctx, base+idx, iHighLimit, 8, NULL, NULL);
}


/** Tree updater, providing best match */
FORCE_INLINE int LZ4HC_BinTree_GetAllMatches (
                        LZ4HC_CCtx_internal* ctx,
                        const U8* const ip, const U8* const iHighLimit,
                        size_t best_mlen, LZ4HC_match_t* matches, const int fullUpdate)
{
    int mnum = 0;
    if (ip < ctx->base + ctx->nextToUpdate) return 0;   /* skipped area */
    if (fullUpdate) LZ4HC_updateBinTree(ctx, ip, iHighLimit);
    best_mlen = LZ4HC_BinTree_InsertAndGetAllMatches(ctx, ip, iHighLimit, best_mlen, matches, &mnum);
    ctx->nextToUpdate = (U32)(ip - ctx->base + best_mlen);
    return mnum;
}


#define SET_PRICE(pos, ml, offset, ll, cost)           \
{                                                      \
    while (last_pos < pos)  { opt[last_pos+1].price = 1<<30; last_pos++; } \
    opt[pos].mlen = (int)ml;                           \
    opt[pos].off = (int)offset;                        \
    opt[pos].litlen = (int)ll;                         \
    opt[pos].price = (int)cost;                        \
}

FORCE_INLINE int LZ4HC_encodeSequence ( const U8** ip, U8** op, const U8** anchor, int matchLength, const U8* const match, limitedOutput_directive limit, U8* oend);

static int LZ4HC_compress_optimal (
    LZ4HC_CCtx_internal* ctx,
    const char* const source,
    char* dest,
    int inputSize,
    int maxOutputSize,
    limitedOutput_directive limit,
    size_t sufficient_len,
    const int fullUpdate
    )
{
    LZ4HC_optimal_t opt[LZ4_OPT_NUM + 1];   /* this uses a bit too much stack memory to my taste ... */
    LZ4HC_match_t matches[LZ4_OPT_NUM + 1];

    const U8* ip = (const U8*) source;
    const U8* anchor = ip;
    const U8* const iend = ip + inputSize;
    const U8* const mflimit = iend - MFLIMIT;
    const U8* const matchlimit = (iend - LASTLITERALS);
    U8* op = (U8*) dest;
    U8* const oend = op + maxOutputSize;

    /* init */
    if (sufficient_len >= LZ4_OPT_NUM) sufficient_len = LZ4_OPT_NUM-1;
    ctx->end += inputSize;
    ip++;

    /* Main Loop */
    while (ip < mflimit) {
        size_t const llen = ip - anchor;
        size_t last_pos = 0;
        size_t match_num, cur, best_mlen, best_off;
        memset(opt, 0, sizeof(LZ4HC_optimal_t));

        match_num = LZ4HC_BinTree_GetAllMatches(ctx, ip, matchlimit, MINMATCH-1, matches, fullUpdate);
        if (!match_num) { ip++; continue; }

        if ((size_t)matches[match_num-1].len > sufficient_len) {
            /* good enough solution : immediate encoding */
            best_mlen = matches[match_num-1].len;
            best_off = matches[match_num-1].off;
            cur = 0;
            last_pos = 1;
            goto encode;
        }

        /* set prices using matches at position = 0 */
        {   size_t matchNb;
            for (matchNb = 0; matchNb < match_num; matchNb++) {
                size_t mlen = (matchNb>0) ? (size_t)matches[matchNb-1].len+1 : MINMATCH;
                best_mlen = matches[matchNb].len;   /* necessarily < sufficient_len < LZ4_OPT_NUM */
                for ( ; mlen <= best_mlen ; mlen++) {
                    size_t const cost = LZ4HC_sequencePrice(llen, mlen) - LZ4HC_literalsPrice(llen);
                    SET_PRICE(mlen, mlen, matches[matchNb].off, 0, cost);   /* updates last_pos and opt[pos] */
        }   }   }

        if (last_pos < MINMATCH) { ip++; continue; }  /* note : on clang at least, this test improves performance */

        /* check further positions */
        opt[0].mlen = opt[1].mlen = 1;
        for (cur = 1; cur <= last_pos; cur++) {
            const U8* const curPtr = ip + cur;

            /* establish baseline price if cur is literal */
            {   size_t price, litlen;
                if (opt[cur-1].mlen == 1) {
                    /* no match at previous position */
                    litlen = opt[cur-1].litlen + 1;
                    if (cur > litlen) {
                        price = opt[cur - litlen].price + LZ4HC_literalsPrice(litlen);
                    } else {
                        price = LZ4HC_literalsPrice(llen + litlen) - LZ4HC_literalsPrice(llen);
                    }
                } else {
                    litlen = 1;
                    price = opt[cur - 1].price + LZ4HC_literalsPrice(1);
                }

                if (price < (size_t)opt[cur].price)
                    SET_PRICE(cur, 1, 0, litlen, price);   /* note : increases last_pos */
            }

            if (cur == last_pos || curPtr >= mflimit) break;

            match_num = LZ4HC_BinTree_GetAllMatches(ctx, curPtr, matchlimit, MINMATCH-1, matches, fullUpdate);
            if ((match_num > 0) && (size_t)matches[match_num-1].len > sufficient_len) {
                /* immediate encoding */
                best_mlen = matches[match_num-1].len;
                best_off = matches[match_num-1].off;
                last_pos = cur + 1;
                goto encode;
            }

            /* set prices using matches at position = cur */
            {   size_t matchNb;
                for (matchNb = 0; matchNb < match_num; matchNb++) {
                    size_t ml = (matchNb>0) ? (size_t)matches[matchNb-1].len+1 : MINMATCH;
                    best_mlen = (cur + matches[matchNb].len < LZ4_OPT_NUM) ?
                                (size_t)matches[matchNb].len : LZ4_OPT_NUM - cur;

                    for ( ; ml <= best_mlen ; ml++) {
                        size_t ll, price;
                        if (opt[cur].mlen == 1) {
                            ll = opt[cur].litlen;
                            if (cur > ll)
                                price = opt[cur - ll].price + LZ4HC_sequencePrice(ll, ml);
                            else
                                price = LZ4HC_sequencePrice(llen + ll, ml) - LZ4HC_literalsPrice(llen);
                        } else {
                            ll = 0;
                            price = opt[cur].price + LZ4HC_sequencePrice(0, ml);
                        }

                        if (cur + ml > last_pos || price < (size_t)opt[cur + ml].price) {
                            SET_PRICE(cur + ml, ml, matches[matchNb].off, ll, price);
            }   }   }   }
        } /* for (cur = 1; cur <= last_pos; cur++) */

        best_mlen = opt[last_pos].mlen;
        best_off = opt[last_pos].off;
        cur = last_pos - best_mlen;

encode: /* cur, last_pos, best_mlen, best_off must be set */
        opt[0].mlen = 1;
        while (1) {  /* from end to beginning */
            size_t const ml = opt[cur].mlen;
            int const offset = opt[cur].off;
            opt[cur].mlen = (int)best_mlen;
            opt[cur].off = (int)best_off;
            best_mlen = ml;
            best_off = offset;
            if (ml > cur) break;
            cur -= ml;
        }

        /* encode all recorded sequences */
        cur = 0;
        while (cur < last_pos) {
            int const ml = opt[cur].mlen;
            int const offset = opt[cur].off;
            if (ml == 1) { ip++; cur++; continue; }
            cur += ml;
            if ( LZ4HC_encodeSequence(&ip, &op, &anchor, ml, ip - offset, limit, oend) ) return 0;
        }
    }  /* while (ip < mflimit) */

    /* Encode Last Literals */
    {   int lastRun = (int)(iend - anchor);
        if ((limit) && (((char*)op - dest) + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > (U32)maxOutputSize)) return 0;  /* Check output limit */
        if (lastRun>=(int)RUN_MASK) { *op++=(RUN_MASK<<ML_BITS); lastRun-=RUN_MASK; for(; lastRun > 254 ; lastRun-=255) *op++ = 255; *op++ = (U8) lastRun; }
        else *op++ = (U8)(lastRun<<ML_BITS);
        memcpy(op, anchor, iend - anchor);
        op += iend-anchor;
    }

    /* End */
    return (int) ((char*)op-dest);
}
/*
    LZ4 HC - High Compression Mode of LZ4
    Copyright (C) 2011-2017, Yann Collet.

    BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    You can contact the author at :
       - LZ4 source repository : https://github.com/lz4/lz4
       - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/
/* note : lz4hc is not an independent module, it requires lz4.h/lz4.c for proper compilation */


/* *************************************
*  Tuning Parameter
***************************************/

/*! HEAPMODE :
 *  Select how default compression function will allocate workplace memory,
 *  in stack (0:fastest), or in heap (1:requires malloc()).
 *  Since workplace is rather large, heap mode is recommended.
 */
#ifndef LZ4HC_HEAPMODE
#  define LZ4HC_HEAPMODE 1
#endif


/*===    Dependency    ===*/






/**************************************
*  HC Compression
**************************************/
static void LZ4HC_init (LZ4HC_CCtx_internal* hc4, const U8* start)
{
    MEM_INIT((void*)hc4->hashTable, 0, sizeof(hc4->hashTable));
    MEM_INIT(hc4->chainTable, 0xFF, sizeof(hc4->chainTable));
    hc4->nextToUpdate = 64 KB;
    hc4->base = start - 64 KB;
    hc4->end = start;
    hc4->dictBase = start - 64 KB;
    hc4->dictLimit = 64 KB;
    hc4->lowLimit = 64 KB;
}


/* Update chains up to ip (excluded) */
FORCE_INLINE void LZ4HC_Insert (LZ4HC_CCtx_internal* hc4, const U8* ip)
{
    U16* const chainTable = hc4->chainTable;
    U32* const hashTable  = hc4->hashTable;
    const U8* const base = hc4->base;
    U32 const target = (U32)(ip - base);
    U32 idx = hc4->nextToUpdate;

    while (idx < target) {
        U32 const h = LZ4HC_hashPtr(base+idx);
        size_t delta = idx - hashTable[h];
        if (delta>MAX_DISTANCE) delta = MAX_DISTANCE;
        DELTANEXTU16(idx) = (U16)delta;
        hashTable[h] = idx;
        idx++;
    }

    hc4->nextToUpdate = target;
}


FORCE_INLINE int LZ4HC_InsertAndFindBestMatch (LZ4HC_CCtx_internal* hc4,   /* Index table will be updated */
                                               const U8* ip, const U8* const iLimit,
                                               const U8** matchpos,
                                               const int maxNbAttempts)
{
    U16* const chainTable = hc4->chainTable;
    U32* const HashTable = hc4->hashTable;
    const U8* const base = hc4->base;
    const U8* const dictBase = hc4->dictBase;
    const U32 dictLimit = hc4->dictLimit;
    const U32 lowLimit = (hc4->lowLimit + 64 KB > (U32)(ip-base)) ? hc4->lowLimit : (U32)(ip - base) - (64 KB - 1);
    U32 matchIndex;
    int nbAttempts = maxNbAttempts;
    size_t ml = 0;

    /* HC4 match finder */
    LZ4HC_Insert(hc4, ip);
    matchIndex = HashTable[LZ4HC_hashPtr(ip)];

    while ((matchIndex>=lowLimit) && (nbAttempts)) {
        nbAttempts--;
        if (matchIndex >= dictLimit) {
            const U8* const match = base + matchIndex;
            if (*(match+ml) == *(ip+ml)
                && (LZ4_read32(match) == LZ4_read32(ip)))
            {
                size_t const mlt = LZ4_count(ip+MINMATCH, match+MINMATCH, iLimit) + MINMATCH;
                if (mlt > ml) { ml = mlt; *matchpos = match; }
            }
        } else {
            const U8* const match = dictBase + matchIndex;
            if (LZ4_read32(match) == LZ4_read32(ip)) {
                size_t mlt;
                const U8* vLimit = ip + (dictLimit - matchIndex);
                if (vLimit > iLimit) vLimit = iLimit;
                mlt = LZ4_count(ip+MINMATCH, match+MINMATCH, vLimit) + MINMATCH;
                if ((ip+mlt == vLimit) && (vLimit < iLimit))
                    mlt += LZ4_count(ip+mlt, base+dictLimit, iLimit);
                if (mlt > ml) { ml = mlt; *matchpos = base + matchIndex; }   /* virtual matchpos */
            }
        }
        matchIndex -= DELTANEXTU16(matchIndex);
    }

    return (int)ml;
}


FORCE_INLINE int LZ4HC_InsertAndGetWiderMatch (
    LZ4HC_CCtx_internal* hc4,
    const U8* const ip,
    const U8* const iLowLimit,
    const U8* const iHighLimit,
    int longest,
    const U8** matchpos,
    const U8** startpos,
    const int maxNbAttempts)
{
    U16* const chainTable = hc4->chainTable;
    U32* const HashTable = hc4->hashTable;
    const U8* const base = hc4->base;
    const U32 dictLimit = hc4->dictLimit;
    const U8* const lowPrefixPtr = base + dictLimit;
    const U32 lowLimit = (hc4->lowLimit + 64 KB > (U32)(ip-base)) ? hc4->lowLimit : (U32)(ip - base) - (64 KB - 1);
    const U8* const dictBase = hc4->dictBase;
    U32   matchIndex;
    int nbAttempts = maxNbAttempts;
    int delta = (int)(ip-iLowLimit);


    /* First Match */
    LZ4HC_Insert(hc4, ip);
    matchIndex = HashTable[LZ4HC_hashPtr(ip)];

    while ((matchIndex>=lowLimit) && (nbAttempts)) {
        nbAttempts--;
        if (matchIndex >= dictLimit) {
            const U8* matchPtr = base + matchIndex;
            if (*(iLowLimit + longest) == *(matchPtr - delta + longest)) {
                if (LZ4_read32(matchPtr) == LZ4_read32(ip)) {
                    int mlt = MINMATCH + LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, iHighLimit);
                    int back = 0;

                    while ((ip+back > iLowLimit)
                           && (matchPtr+back > lowPrefixPtr)
                           && (ip[back-1] == matchPtr[back-1]))
                            back--;

                    mlt -= back;

                    if (mlt > longest) {
                        longest = (int)mlt;
                        *matchpos = matchPtr+back;
                        *startpos = ip+back;
            }   }   }
        } else {
            const U8* const matchPtr = dictBase + matchIndex;
            if (LZ4_read32(matchPtr) == LZ4_read32(ip)) {
                size_t mlt;
                int back=0;
                const U8* vLimit = ip + (dictLimit - matchIndex);
                if (vLimit > iHighLimit) vLimit = iHighLimit;
                mlt = LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, vLimit) + MINMATCH;
                if ((ip+mlt == vLimit) && (vLimit < iHighLimit))
                    mlt += LZ4_count(ip+mlt, base+dictLimit, iHighLimit);
                while ((ip+back > iLowLimit) && (matchIndex+back > lowLimit) && (ip[back-1] == matchPtr[back-1])) back--;
                mlt -= back;
                if ((int)mlt > longest) { longest = (int)mlt; *matchpos = base + matchIndex + back; *startpos = ip+back; }
            }
        }
        matchIndex -= DELTANEXTU16(matchIndex);
    }

    return longest;
}



#define LZ4HC_DEBUG 0
#if LZ4HC_DEBUG
static unsigned debug = 0;
#endif


/* LZ4HC_encodeSequence() :
 * @return : 0 if ok,
 *           1 if buffer issue detected */
FORCE_INLINE int LZ4HC_encodeSequence ( const U8** ip, U8** op, const U8** anchor, int matchLength, const U8* const match, limitedOutput_directive limit, U8* oend)
{
    size_t length;
    U8* token;

#if LZ4HC_DEBUG
    if (debug) printf("literal : %u  --  match : %u  --  offset : %u\n", (U32)(*ip - *anchor), (U32)matchLength, (U32)(*ip-match));
#endif

    /* Encode Literal length */
    length = (size_t)(*ip - *anchor);
    token = (*op)++;
    if ((limit) && ((*op + (length >> 8) + length + (2 + 1 + LASTLITERALS)) > oend)) return 1;   /* Check output limit */
    if (length >= RUN_MASK) {
        size_t len = length - RUN_MASK;
        *token = (RUN_MASK << ML_BITS);
        for(; len >= 255 ; len -= 255) *(*op)++ = 255;
        *(*op)++ = (U8)len;
    } else {
        *token = (U8)(length << ML_BITS);
    }

    /* Copy Literals */
    LZ4_wildCopy(*op, *anchor, (*op) + length);
    *op += length;

    /* Encode Offset */
    LZ4_writeLE16(*op, (U16)(*ip-match)); *op += 2;

    /* Encode MatchLength */
    length = (size_t)(matchLength - MINMATCH);
    if ((limit) && (*op + (length >> 8) + (1 + LASTLITERALS) > oend)) return 1;   /* Check output limit */
    if (length >= ML_MASK) {
        *token += ML_MASK;
        length -= ML_MASK;
        for(; length >= 510 ; length -= 510) { *(*op)++ = 255; *(*op)++ = 255; }
        if (length >= 255) { length -= 255; *(*op)++ = 255; }
        *(*op)++ = (U8)length;
    } else {
        *token += (U8)(length);
    }

    /* Prepare next loop */
    *ip += matchLength;
    *anchor = *ip;

    return 0;
}

/* btopt */


static int LZ4HC_compress_hashChain (
    LZ4HC_CCtx_internal* const ctx,
    const char* const source,
    char* const dest,
    int* srcSizePtr,
    int const maxOutputSize,
    unsigned maxNbAttempts,
    limitedOutput_directive limit
    )
{
    const int inputSize = *srcSizePtr;

    const U8* ip = (const U8*) source;
    const U8* anchor = ip;
    const U8* const iend = ip + inputSize;
    const U8* const mflimit = iend - MFLIMIT;
    const U8* const matchlimit = (iend - LASTLITERALS);

    U8* optr = (U8*) dest;
    U8* op = (U8*) dest;
    U8* oend = op + maxOutputSize;

    int   ml, ml2, ml3, ml0;
    const U8* ref = NULL;
    const U8* start2 = NULL;
    const U8* ref2 = NULL;
    const U8* start3 = NULL;
    const U8* ref3 = NULL;
    const U8* start0;
    const U8* ref0;

    /* init */
    *srcSizePtr = 0;
    if (limit == limitedDestSize && maxOutputSize < 1) return 0;         /* Impossible to store anything */
    if ((U32)inputSize > (U32)LZ4_MAX_INPUT_SIZE) return 0;              /* Unsupported input size, too large (or negative) */

    ctx->end += inputSize;
    if (limit == limitedDestSize) oend -= LASTLITERALS;                  /* Hack for support limitations LZ4 decompressor */
    if (inputSize < LZ4_minLength) goto _last_literals;                  /* Input too small, no compression (all literals) */

    ip++;

    /* Main Loop */
    while (ip < mflimit) {
        ml = LZ4HC_InsertAndFindBestMatch (ctx, ip, matchlimit, (&ref), maxNbAttempts);
        if (!ml) { ip++; continue; }

        /* saved, in case we would skip too much */
        start0 = ip;
        ref0 = ref;
        ml0 = ml;

_Search2:
        if (ip+ml < mflimit)
            ml2 = LZ4HC_InsertAndGetWiderMatch(ctx, ip + ml - 2, ip + 0, matchlimit, ml, &ref2, &start2, maxNbAttempts);
        else
            ml2 = ml;

        if (ml2 == ml) { /* No better match */
            optr = op;
            if (LZ4HC_encodeSequence(&ip, &op, &anchor, ml, ref, limit, oend)) goto _dest_overflow;
            continue;
        }

        if (start0 < ip) {
            if (start2 < ip + ml0) {  /* empirical */
                ip = start0;
                ref = ref0;
                ml = ml0;
            }
        }

        /* Here, start0==ip */
        if ((start2 - ip) < 3) {  /* First Match too small : removed */
            ml = ml2;
            ip = start2;
            ref =ref2;
            goto _Search2;
        }

_Search3:
        /* At this stage, we have :
        *  ml2 > ml1, and
        *  ip1+3 <= ip2 (usually < ip1+ml1) */
        if ((start2 - ip) < OPTIMAL_ML) {
            int correction;
            int new_ml = ml;
            if (new_ml > OPTIMAL_ML) new_ml = OPTIMAL_ML;
            if (ip+new_ml > start2 + ml2 - MINMATCH) new_ml = (int)(start2 - ip) + ml2 - MINMATCH;
            correction = new_ml - (int)(start2 - ip);
            if (correction > 0) {
                start2 += correction;
                ref2 += correction;
                ml2 -= correction;
            }
        }
        /* Now, we have start2 = ip+new_ml, with new_ml = min(ml, OPTIMAL_ML=18) */

        if (start2 + ml2 < mflimit)
            ml3 = LZ4HC_InsertAndGetWiderMatch(ctx, start2 + ml2 - 3, start2, matchlimit, ml2, &ref3, &start3, maxNbAttempts);
        else
            ml3 = ml2;

        if (ml3 == ml2) {  /* No better match : 2 sequences to encode */
            /* ip & ref are known; Now for ml */
            if (start2 < ip+ml)  ml = (int)(start2 - ip);
            /* Now, encode 2 sequences */
            optr = op;
            if (LZ4HC_encodeSequence(&ip, &op, &anchor, ml, ref, limit, oend)) goto _dest_overflow;
            ip = start2;
            optr = op;
            if (LZ4HC_encodeSequence(&ip, &op, &anchor, ml2, ref2, limit, oend)) goto _dest_overflow;
            continue;
        }

        if (start3 < ip+ml+3) {  /* Not enough space for match 2 : remove it */
            if (start3 >= (ip+ml)) {  /* can write Seq1 immediately ==> Seq2 is removed, so Seq3 becomes Seq1 */
                if (start2 < ip+ml) {
                    int correction = (int)(ip+ml - start2);
                    start2 += correction;
                    ref2 += correction;
                    ml2 -= correction;
                    if (ml2 < MINMATCH) {
                        start2 = start3;
                        ref2 = ref3;
                        ml2 = ml3;
                    }
                }

                optr = op;
                if (LZ4HC_encodeSequence(&ip, &op, &anchor, ml, ref, limit, oend)) goto _dest_overflow;
                ip  = start3;
                ref = ref3;
                ml  = ml3;

                start0 = start2;
                ref0 = ref2;
                ml0 = ml2;
                goto _Search2;
            }

            start2 = start3;
            ref2 = ref3;
            ml2 = ml3;
            goto _Search3;
        }

        /*
        * OK, now we have 3 ascending matches; let's write at least the first one
        * ip & ref are known; Now for ml
        */
        if (start2 < ip+ml) {
            if ((start2 - ip) < (int)ML_MASK) {
                int correction;
                if (ml > OPTIMAL_ML) ml = OPTIMAL_ML;
                if (ip + ml > start2 + ml2 - MINMATCH) ml = (int)(start2 - ip) + ml2 - MINMATCH;
                correction = ml - (int)(start2 - ip);
                if (correction > 0) {
                    start2 += correction;
                    ref2 += correction;
                    ml2 -= correction;
                }
            } else {
                ml = (int)(start2 - ip);
            }
        }
        optr = op;
        if (LZ4HC_encodeSequence(&ip, &op, &anchor, ml, ref, limit, oend)) goto _dest_overflow;

        ip = start2;
        ref = ref2;
        ml = ml2;

        start2 = start3;
        ref2 = ref3;
        ml2 = ml3;

        goto _Search3;
    }

_last_literals:
    /* Encode Last Literals */
    {   size_t lastRunSize = (size_t)(iend - anchor);  /* literals */
        size_t litLength = (lastRunSize + 255 - RUN_MASK) / 255;
        size_t const totalSize = 1 + litLength + lastRunSize;
        if (limit == limitedDestSize) oend += LASTLITERALS;  /* restore correct value */
        if (limit && (op + totalSize > oend)) {
            if (limit == limitedOutput) return 0;  /* Check output limit */
            /* adapt lastRunSize to fill 'dest' */
            lastRunSize  = (size_t)(oend - op) - 1;
            litLength = (lastRunSize + 255 - RUN_MASK) / 255;
            lastRunSize -= litLength;
        }
        ip = anchor + lastRunSize;

        if (lastRunSize >= RUN_MASK) {
            size_t accumulator = lastRunSize - RUN_MASK;
            *op++ = (RUN_MASK << ML_BITS);
            for(; accumulator >= 255 ; accumulator -= 255) *op++ = 255;
            *op++ = (U8) accumulator;
        } else {
            *op++ = (U8)(lastRunSize << ML_BITS);
        }
        memcpy(op, anchor, lastRunSize);
        op += lastRunSize;
    }

    /* End */
    *srcSizePtr = (int) (((const char*)ip) - source);
    return (int) (((char*)op)-dest);

_dest_overflow:
    if (limit == limitedDestSize) {
        op = optr;  /* restore correct out pointer */
        goto _last_literals;
    }
    return 0;
}

static int LZ4HC_getSearchNum(int compressionLevel)
{
    switch (compressionLevel) {
        default: return 0; /* unused */
        case 11: return 128;
        case 12: return 1<<10;
    }
}

static int LZ4HC_compress_generic (
    LZ4HC_CCtx_internal* const ctx,
    const char* const src,
    char* const dst,
    int* const srcSizePtr,
    int const dstCapacity,
    int cLevel,
    limitedOutput_directive limit
    )
{
    if (cLevel < 1) cLevel = LZ4HC_CLEVEL_DEFAULT;   /* note : convention is different from lz4frame, maybe to reconsider */
    if (cLevel > 9) {
        if (limit == limitedDestSize) cLevel = 10;
        switch (cLevel) {
            case 10:
                return LZ4HC_compress_hashChain(ctx, src, dst, srcSizePtr, dstCapacity, 1 << (15-1), limit);
            case 11:
                ctx->searchNum = LZ4HC_getSearchNum(cLevel);
                return LZ4HC_compress_optimal(ctx, src, dst, *srcSizePtr, dstCapacity, limit, 128, 0);
            default:
            case 12:
                ctx->searchNum = LZ4HC_getSearchNum(cLevel);
                return LZ4HC_compress_optimal(ctx, src, dst, *srcSizePtr, dstCapacity, limit, LZ4_OPT_NUM, 1);
        }
    }
    return LZ4HC_compress_hashChain(ctx, src, dst, srcSizePtr, dstCapacity, 1 << (cLevel-1), limit);  /* levels 1-9 */
}


int LZ4_sizeofStateHC(void) { return sizeof(LZ4_streamHC_t); }

int LZ4_compress_HC_extStateHC (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel)
{
    LZ4HC_CCtx_internal* const ctx = &((LZ4_streamHC_t*)state)->internal_donotuse;
    if (((size_t)(state)&(sizeof(void*)-1)) != 0) return 0;   /* Error : state is not aligned for pointers (32 or 64 bits) */
    LZ4HC_init (ctx, (const U8*)src);
    if (dstCapacity < LZ4_compressBound(srcSize))
        return LZ4HC_compress_generic (ctx, src, dst, &srcSize, dstCapacity, compressionLevel, limitedOutput);
    else
        return LZ4HC_compress_generic (ctx, src, dst, &srcSize, dstCapacity, compressionLevel, noLimit);
}

int LZ4_compress_HC(const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel)
{
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    LZ4_streamHC_t* const statePtr = (LZ4_streamHC_t*)malloc(sizeof(LZ4_streamHC_t));
#else
    LZ4_streamHC_t state;
    LZ4_streamHC_t* const statePtr = &state;
#endif
    int const cSize = LZ4_compress_HC_extStateHC(statePtr, src, dst, srcSize, dstCapacity, compressionLevel);
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    free(statePtr);
#endif
    return cSize;
}

/* LZ4_compress_HC_destSize() :
 * currently, only compatible with Hash Chain implementation,
 * hence limit compression level to LZ4HC_CLEVEL_OPT_MIN-1*/
int LZ4_compress_HC_destSize(void* LZ4HC_Data, const char* source, char* dest, int* sourceSizePtr, int targetDestSize, int cLevel)
{
    LZ4HC_CCtx_internal* const ctx = &((LZ4_streamHC_t*)LZ4HC_Data)->internal_donotuse;
    LZ4HC_init(ctx, (const U8*) source);
    return LZ4HC_compress_generic(ctx, source, dest, sourceSizePtr, targetDestSize, cLevel, limitedDestSize);
}



/**************************************
*  Streaming Functions
**************************************/
/* allocation */
LZ4_streamHC_t* LZ4_createStreamHC(void) { return (LZ4_streamHC_t*)malloc(sizeof(LZ4_streamHC_t)); }
int             LZ4_freeStreamHC (LZ4_streamHC_t* LZ4_streamHCPtr) { free(LZ4_streamHCPtr); return 0; }


/* initialization */
void LZ4_resetStreamHC (LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel)
{
    LZ4_STATIC_ASSERT(sizeof(LZ4HC_CCtx_internal) <= sizeof(size_t) * LZ4_STREAMHCSIZE_SIZET);   /* if compilation fails here, LZ4_STREAMHCSIZE must be increased */
    LZ4_streamHCPtr->internal_donotuse.base = NULL;
    if (compressionLevel > LZ4HC_CLEVEL_MAX) compressionLevel = LZ4HC_CLEVEL_MAX;  /* cap compression level */
    LZ4_streamHCPtr->internal_donotuse.compressionLevel = compressionLevel;
    LZ4_streamHCPtr->internal_donotuse.searchNum = LZ4HC_getSearchNum(compressionLevel);
}

int LZ4_loadDictHC (LZ4_streamHC_t* LZ4_streamHCPtr, const char* dictionary, int dictSize)
{
    LZ4HC_CCtx_internal* const ctxPtr = &LZ4_streamHCPtr->internal_donotuse;
    if (dictSize > 64 KB) {
        dictionary += dictSize - 64 KB;
        dictSize = 64 KB;
    }
    LZ4HC_init (ctxPtr, (const U8*)dictionary);
    ctxPtr->end = (const U8*)dictionary + dictSize;
    if (ctxPtr->compressionLevel >= LZ4HC_CLEVEL_OPT_MIN)
        LZ4HC_updateBinTree(ctxPtr, ctxPtr->end - MFLIMIT, ctxPtr->end - LASTLITERALS);
    else
        if (dictSize >= 4) LZ4HC_Insert (ctxPtr, ctxPtr->end-3);
    return dictSize;
}


/* compression */

static void LZ4HC_setExternalDict(LZ4HC_CCtx_internal* ctxPtr, const U8* newBlock)
{
    if (ctxPtr->compressionLevel >= LZ4HC_CLEVEL_OPT_MIN)
        LZ4HC_updateBinTree(ctxPtr, ctxPtr->end - MFLIMIT, ctxPtr->end - LASTLITERALS);
    else
        if (ctxPtr->end >= ctxPtr->base + 4) LZ4HC_Insert (ctxPtr, ctxPtr->end-3);   /* Referencing remaining dictionary content */

    /* Only one memory segment for extDict, so any previous extDict is lost at this stage */
    ctxPtr->lowLimit  = ctxPtr->dictLimit;
    ctxPtr->dictLimit = (U32)(ctxPtr->end - ctxPtr->base);
    ctxPtr->dictBase  = ctxPtr->base;
    ctxPtr->base = newBlock - ctxPtr->dictLimit;
    ctxPtr->end  = newBlock;
    ctxPtr->nextToUpdate = ctxPtr->dictLimit;   /* match referencing will resume from there */
}

static int LZ4_compressHC_continue_generic (LZ4_streamHC_t* LZ4_streamHCPtr,
                                            const char* src, char* dst,
                                            int* srcSizePtr, int dstCapacity,
                                            limitedOutput_directive limit)
{
    LZ4HC_CCtx_internal* const ctxPtr = &LZ4_streamHCPtr->internal_donotuse;
    /* auto-init if forgotten */
    if (ctxPtr->base == NULL) LZ4HC_init (ctxPtr, (const U8*) src);

    /* Check overflow */
    if ((size_t)(ctxPtr->end - ctxPtr->base) > 2 GB) {
        size_t dictSize = (size_t)(ctxPtr->end - ctxPtr->base) - ctxPtr->dictLimit;
        if (dictSize > 64 KB) dictSize = 64 KB;
        LZ4_loadDictHC(LZ4_streamHCPtr, (const char*)(ctxPtr->end) - dictSize, (int)dictSize);
    }

    /* Check if blocks follow each other */
    if ((const U8*)src != ctxPtr->end) LZ4HC_setExternalDict(ctxPtr, (const U8*)src);

    /* Check overlapping input/dictionary space */
    {   const U8* sourceEnd = (const U8*) src + *srcSizePtr;
        const U8* const dictBegin = ctxPtr->dictBase + ctxPtr->lowLimit;
        const U8* const dictEnd   = ctxPtr->dictBase + ctxPtr->dictLimit;
        if ((sourceEnd > dictBegin) && ((const U8*)src < dictEnd)) {
            if (sourceEnd > dictEnd) sourceEnd = dictEnd;
            ctxPtr->lowLimit = (U32)(sourceEnd - ctxPtr->dictBase);
            if (ctxPtr->dictLimit - ctxPtr->lowLimit < 4) ctxPtr->lowLimit = ctxPtr->dictLimit;
        }
    }

    return LZ4HC_compress_generic (ctxPtr, src, dst, srcSizePtr, dstCapacity, ctxPtr->compressionLevel, limit);
}

int LZ4_compress_HC_continue (LZ4_streamHC_t* LZ4_streamHCPtr, const char* src, char* dst, int srcSize, int dstCapacity)
{
    if (dstCapacity < LZ4_compressBound(srcSize))
        return LZ4_compressHC_continue_generic (LZ4_streamHCPtr, src, dst, &srcSize, dstCapacity, limitedOutput);
    else
        return LZ4_compressHC_continue_generic (LZ4_streamHCPtr, src, dst, &srcSize, dstCapacity, noLimit);
}

int LZ4_compress_HC_continue_destSize (LZ4_streamHC_t* LZ4_streamHCPtr, const char* src, char* dst, int* srcSizePtr, int targetDestSize)
{
    LZ4HC_CCtx_internal* const ctxPtr = &LZ4_streamHCPtr->internal_donotuse;
    if (ctxPtr->compressionLevel >= LZ4HC_CLEVEL_OPT_MIN) LZ4HC_init(ctxPtr, (const U8*)src);   /* not compatible with btopt implementation */
    return LZ4_compressHC_continue_generic(LZ4_streamHCPtr, src, dst, srcSizePtr, targetDestSize, limitedDestSize);
}



/* dictionary saving */

int LZ4_saveDictHC (LZ4_streamHC_t* LZ4_streamHCPtr, char* safeBuffer, int dictSize)
{
    LZ4HC_CCtx_internal* const streamPtr = &LZ4_streamHCPtr->internal_donotuse;
    int const prefixSize = (int)(streamPtr->end - (streamPtr->base + streamPtr->dictLimit));
    if (dictSize > 64 KB) dictSize = 64 KB;
    if (dictSize < 4) dictSize = 0;
    if (dictSize > prefixSize) dictSize = prefixSize;
    memmove(safeBuffer, streamPtr->end - dictSize, dictSize);
    {   U32 const endIndex = (U32)(streamPtr->end - streamPtr->base);
        streamPtr->end = (const U8*)safeBuffer + dictSize;
        streamPtr->base = streamPtr->end - endIndex;
        streamPtr->dictLimit = endIndex - dictSize;
        streamPtr->lowLimit = endIndex - dictSize;
        if (streamPtr->nextToUpdate < streamPtr->dictLimit) streamPtr->nextToUpdate = streamPtr->dictLimit;
    }
    return dictSize;
}


/***********************************
*  Deprecated Functions
***********************************/
/* These functions currently generate deprecation warnings */
/* Deprecated compression functions */
int LZ4_compressHC(const char* src, char* dst, int srcSize) { return LZ4_compress_HC (src, dst, srcSize, LZ4_compressBound(srcSize), 0); }
int LZ4_compressHC_limitedOutput(const char* src, char* dst, int srcSize, int maxDstSize) { return LZ4_compress_HC(src, dst, srcSize, maxDstSize, 0); }
int LZ4_compressHC2(const char* src, char* dst, int srcSize, int cLevel) { return LZ4_compress_HC (src, dst, srcSize, LZ4_compressBound(srcSize), cLevel); }
int LZ4_compressHC2_limitedOutput(const char* src, char* dst, int srcSize, int maxDstSize, int cLevel) { return LZ4_compress_HC(src, dst, srcSize, maxDstSize, cLevel); }
int LZ4_compressHC_withStateHC (void* state, const char* src, char* dst, int srcSize) { return LZ4_compress_HC_extStateHC (state, src, dst, srcSize, LZ4_compressBound(srcSize), 0); }
int LZ4_compressHC_limitedOutput_withStateHC (void* state, const char* src, char* dst, int srcSize, int maxDstSize) { return LZ4_compress_HC_extStateHC (state, src, dst, srcSize, maxDstSize, 0); }
int LZ4_compressHC2_withStateHC (void* state, const char* src, char* dst, int srcSize, int cLevel) { return LZ4_compress_HC_extStateHC(state, src, dst, srcSize, LZ4_compressBound(srcSize), cLevel); }
int LZ4_compressHC2_limitedOutput_withStateHC (void* state, const char* src, char* dst, int srcSize, int maxDstSize, int cLevel) { return LZ4_compress_HC_extStateHC(state, src, dst, srcSize, maxDstSize, cLevel); }
int LZ4_compressHC_continue (LZ4_streamHC_t* ctx, const char* src, char* dst, int srcSize) { return LZ4_compress_HC_continue (ctx, src, dst, srcSize, LZ4_compressBound(srcSize)); }
int LZ4_compressHC_limitedOutput_continue (LZ4_streamHC_t* ctx, const char* src, char* dst, int srcSize, int maxDstSize) { return LZ4_compress_HC_continue (ctx, src, dst, srcSize, maxDstSize); }


/* Deprecated streaming functions */
int LZ4_sizeofStreamStateHC(void) { return LZ4_STREAMHCSIZE; }

int LZ4_resetStreamStateHC(void* state, char* inputBuffer)
{
    LZ4HC_CCtx_internal *ctx = &((LZ4_streamHC_t*)state)->internal_donotuse;
    if ((((size_t)state) & (sizeof(void*)-1)) != 0) return 1;   /* Error : pointer is not aligned for pointer (32 or 64 bits) */
    LZ4HC_init(ctx, (const U8*)inputBuffer);
    ctx->inputBuffer = (U8*)inputBuffer;
    return 0;
}

void* LZ4_createHC (char* inputBuffer)
{
    LZ4_streamHC_t* hc4 = (LZ4_streamHC_t*)ALLOCATOR(1, sizeof(LZ4_streamHC_t));
    if (hc4 == NULL) return NULL;   /* not enough memory */
    LZ4HC_init (&hc4->internal_donotuse, (const U8*)inputBuffer);
    hc4->internal_donotuse.inputBuffer = (U8*)inputBuffer;
    return hc4;
}

int LZ4_freeHC (void* LZ4HC_Data) { FREEMEM(LZ4HC_Data); return 0; }

int LZ4_compressHC2_continue (void* LZ4HC_Data, const char* src, char* dst, int srcSize, int cLevel)
{
    return LZ4HC_compress_generic (&((LZ4_streamHC_t*)LZ4HC_Data)->internal_donotuse, src, dst, &srcSize, 0, cLevel, noLimit);
}

int LZ4_compressHC2_limitedOutput_continue (void* LZ4HC_Data, const char* src, char* dst, int srcSize, int dstCapacity, int cLevel)
{
    return LZ4HC_compress_generic (&((LZ4_streamHC_t*)LZ4HC_Data)->internal_donotuse, src, dst, &srcSize, dstCapacity, cLevel, limitedOutput);
}

char* LZ4_slideInputBufferHC(void* LZ4HC_Data)
{
    LZ4HC_CCtx_internal* const hc4 = &((LZ4_streamHC_t*)LZ4HC_Data)->internal_donotuse;
    int const dictSize = LZ4_saveDictHC((LZ4_streamHC_t*)LZ4HC_Data, (char*)(hc4->inputBuffer), 64 KB);
    return (char*)(hc4->inputBuffer + dictSize);
}
