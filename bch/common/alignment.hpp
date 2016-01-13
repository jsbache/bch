/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_ALIGNMENT

#include <cstddef>
#include <cinttypes>

#include <bch/common/header_prefix.hpp>

namespace bch {

inline constexpr std::size_t CalculatePadding(uintptr_t nonAlignedSize, std::size_t alignment)
{
    /*
    padding = (alignment - (nonAlignedSize % alignment)) % alignment
    
    For alignment we have that modulus is equal to operator "&" with (alignment - 1).
    This is true because alignment must be a power of 2.
    Operator "&" is faster that modulus.
    Last I tested, Clang does not perform this optimization for us, so we get:
        (alignment - (nonAlignedSize & alignmentMask)) & alignmentMask
    where
        alignmentMask = alignment - 1
        Requirement: alignment > 0 and alignment = 2 ^ z

    constexpr in C++11 does not support variable definitions, so we have to
    put all on one line.
    */
    return static_cast<std::size_t>((alignment - (nonAlignedSize & (alignment-1))) & (alignment-1));
}

}   // namespace bch

#include <bch/common/header_suffix.hpp>

#endif  // BCH_ALIGNMENT
