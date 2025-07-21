/**
Copyright: Jesper Storm Bache (bache.name)
*/

#include "bch/common/memory.hpp"

#include <stdlib.h>
#include <new>

namespace {

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

} // namespace

namespace bch {

void AllocateInstancePair(  std::size_t size1, std::size_t size2,
                            std::size_t alignment,
                            void*& ptr1, void*& ptr2)
{
    /* We need to allocate a block that can hold both instances (size1 and size2).
    We assume that malloc returns memory that is aligned properly for the first instance.
    For the second instance we must make sure to align to the desired alignment specification
    for the type.
    Padding is modulus the alignment, and we therefore have: padding < alignment.
    We therefore need to allocate alignment-1 for the potential padding bytes.
    The C++11 spec does not state that alignof(T) > 0 (although this seems reasonable),
    so we test explicitly for that case.
    */
    const size_t maxAlignmentPadding = (alignment > 0) ? alignment - 1 : 0;

    const size_t totalSize = size2 + maxAlignmentPadding + size1;

    ptr1 = malloc(totalSize);
    if (ptr1 == nullptr)
        throw std::bad_alloc();

    uintptr_t ptr2Base = reinterpret_cast<uintptr_t>(ptr1) + size1;

    const size_t padding = CalculatePadding(ptr2Base, alignment);
    ptr2 = reinterpret_cast<void*>(ptr2Base + padding);
}

}   // namespace bch
