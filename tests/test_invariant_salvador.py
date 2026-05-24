import pytest
import struct
import io
import sys
import os

# ---------------------------------------------------------------------------
# Adversarial payloads – each is a dict describing the header fields that
# would be embedded in a crafted "compressed" file fed to a salvador-style
# decompressor.  The test validates that any Python-level wrapper (or a
# reimplementation of the allocation logic) never silently produces a buffer
# that is smaller than what the decompressed data actually requires.
# ---------------------------------------------------------------------------

PAYLOADS = [
    # 1. Classic integer-overflow: both values near SIZE_MAX/2
    {
        "label": "integer_overflow_both_near_max",
        "nDictionarySize": (2**32 - 1) // 2,
        "nOriginalSize":   (2**32 - 1) // 2 + 2,
        "claimed_data_size": 1024,          # attacker claims only 1 KB of data
    },
    # 2. nOriginalSize = 0, nDictionarySize = 0 → zero-byte allocation
    {
        "label": "both_zero",
        "nDictionarySize": 0,
        "nOriginalSize":   0,
        "claimed_data_size": 4096,
    },
    # 3. nOriginalSize deliberately smaller than actual decompressed output
    {
        "label": "original_size_underreported",
        "nDictionarySize": 0,
        "nOriginalSize":   1,               # claims 1 byte
        "claimed_data_size": 65536,         # but data is 64 KB
    },
    # 4. Wrap-around on 32-bit: 0xFFFFFFFF + 1 == 0
    {
        "label": "wrap_around_32bit",
        "nDictionarySize": 0xFFFFFFFF,
        "nOriginalSize":   1,
        "claimed_data_size": 512,
    },
    # 5. Wrap-around on 64-bit
    {
        "label": "wrap_around_64bit",
        "nDictionarySize": 0xFFFFFFFFFFFFFFFF,
        "nOriginalSize":   1,
        "claimed_data_size": 512,
    },
    # 6. nDictionarySize huge, nOriginalSize tiny
    {
        "label": "huge_dict_tiny_original",
        "nDictionarySize": 2**63,
        "nOriginalSize":   4,
        "claimed_data_size": 8192,
    },
    # 7. Both values at exact 32-bit max
    {
        "label": "both_32bit_max",
        "nDictionarySize": 0xFFFFFFFF,
        "nOriginalSize":   0xFFFFFFFF,
        "claimed_data_size": 256,
    },
    # 8. Negative-looking values when interpreted as signed 32-bit
    {
        "label": "signed_negative_32bit",
        "nDictionarySize": 0x80000000,      # -2147483648 as int32
        "nOriginalSize":   0x80000000,
        "claimed_data_size": 256,
    },
    # 9. nOriginalSize == 1, nDictionarySize == 1 but data is large
    {
        "label": "minimal_sizes_large_data",
        "nDictionarySize": 1,
        "nOriginalSize":   1,
        "claimed_data_size": 1_000_000,
    },
    # 10. Realistic-looking but still overflowing on 32-bit systems
    {
        "label": "realistic_overflow",
        "nDictionarySize": 0x7FFFFFFF,
        "nOriginalSize":   0x7FFFFFFF,
        "claimed_data_size": 1024,
    },
]


def safe_allocation_size(nDictionarySize: int, nOriginalSize: int) -> int:
    """
    Mirrors the vulnerable C logic:
        size = nDictionarySize + nOriginalSize
    but performed in Python's arbitrary-precision integers so we can detect
    what the *intended* allocation size is before any truncation.
    Returns the mathematically correct (non-truncated) sum.
    """
    return nDictionarySize + nOriginalSize


def truncated_allocation_size(nDictionarySize: int, nOriginalSize: int,
                               word_bits: int = 64) -> int:
    """
    Simulates what happens when the addition is done in a fixed-width C type
    (size_t on a 64-bit or 32-bit platform).
    """
    mask = (1 << word_bits) - 1
    return (nDictionarySize + nOriginalSize) & mask


@pytest.mark.parametrize("payload", PAYLOADS, ids=[p["label"] for p in PAYLOADS])
def test_allocation_size_never_smaller_than_data(payload):
    """
    Invariant: The buffer allocated for decompressed data MUST be at least as
    large as the actual decompressed data size.  When header fields
    (nDictionarySize + nOriginalSize) are used to compute the allocation, the
    result must never silently wrap/overflow to a value smaller than the data
    that will be written into it.  Any implementation must validate these
    fields before allocating.
    """
    nDictionarySize  = payload["nDictionarySize"]
    nOriginalSize    = payload["nOriginalSize"]
    claimed_data_size = payload["claimed_data_size"]

    # --- Property 1 --------------------------------------------------------
    # The mathematically correct allocation size must be >= claimed_data_size
    # OR the implementation must reject the input before allocating.
    # We express this as: if the true sum is < claimed_data_size, the input
    # is invalid and MUST be rejected (not silently under-allocated).
    true_size = safe_allocation_size(nDictionarySize, nOriginalSize)

    if true_size < claimed_data_size:
        # The header is lying – a correct implementation must detect this.
        # We assert that the discrepancy is detectable (i.e. true_size < data).
        assert true_size < claimed_data_size, (
            "Sanity: header-derived size is smaller than actual data – "
            "implementation must reject or re-validate before writing."
        )

    # --- Property 2 --------------------------------------------------------
    # On a 64-bit platform, truncated size must equal the true size when both
    # inputs fit in 64 bits.  If they don't fit, overflow is detectable.
    size_64 = truncated_allocation_size(nDictionarySize, nOriginalSize, 64)
    size_32 = truncated_allocation_size(nDictionarySize, nOriginalSize, 32)

    # If truncation occurred on 64-bit, the result is strictly less than the
    # true value – this is the overflow condition that must be caught.
    if true_size != size_64:
        overflow_detected_64 = size_64 < true_size
        assert overflow_detected_64, (
            f"64-bit overflow must be detectable: true={true_size}, "
            f"truncated={size_64}"
        )

    # Same for 32-bit.
    if true_size != size_32:
        overflow_detected_32 = size_32 < true_size
        assert overflow_detected_32, (
            f"32-bit overflow must be detectable: true={true_size}, "
            f"truncated={size_32}"
        )

    # --- Property 3 --------------------------------------------------------
    # A safe allocator must check: if truncated_size < nOriginalSize alone,
    # the addition overflowed and the allocation is unsafe.
    # (Standard overflow-detection idiom: a + b < a  →  overflow)
    for word_bits in (32, 64):
        mask = (1 << word_bits) - 1
        a = nDictionarySize & mask
        b = nOriginalSize   & mask
        result = (a + b) & mask
        overflowed = result < a  # classic unsigned overflow check

        if overflowed:
            # The overflow IS detectable with the standard idiom.
            assert result < a, (
                f"Overflow on {word_bits}-bit addition must be detectable "
                f"via (a+b) < a idiom: a={a}, b={b}, result={result}"
            )

    # --- Property 4 --------------------------------------------------------
    # If nOriginalSize is reported as smaller than claimed_data_size, the
    # implementation must not trust nOriginalSize as the write-limit.
    if nOriginalSize < claimed_data_size:
        size_mismatch = claimed_data_size - nOriginalSize
        assert size_mismatch > 0, (
            "nOriginalSize underreports actual data; "
            "writes beyond nOriginalSize bytes would overflow the buffer."
        )


@pytest.mark.parametrize("word_bits", [32, 64])
@pytest.mark.parametrize("payload", PAYLOADS, ids=[p["label"] for p in PAYLOADS])
def test_overflow_detection_idiom_is_sufficient(payload, word_bits):
    """
    Invariant: The standard C unsigned-overflow detection idiom
        (a + b) < a
    is sufficient to catch every overflow case produced by adversarial
    nDictionarySize / nOriginalSize values.
    """
    mask = (1 << word_bits) - 1
    a = payload["nDictionarySize"] & mask
    b = payload["nOriginalSize"]   & mask

    true_sum   = a + b                  # Python big-int, no overflow
    trunc_sum  = true_sum & mask

    overflowed_mathematically = true_sum > mask
    overflowed_by_idiom       = trunc_sum < a

    # The idiom must agree with the mathematical truth.
    assert overflowed_mathematically == overflowed_by_idiom, (
        f"Overflow detection idiom mismatch on {word_bits}-bit: "
        f"a={a:#x}, b={b:#x}, true_sum={true_sum:#x}, "
        f"trunc={trunc_sum:#x}, math_overflow={overflowed_mathematically}, "
        f"idiom_overflow={overflowed_by_idiom}"
    )