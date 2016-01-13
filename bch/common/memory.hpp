/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_MEMORY

#include <cstddef>

#include <bch/common/header_prefix.hpp>

namespace bch {

/* Allocate a single block of memory to be used by two instances.
The location for the second instance must adhere to its alignment specification.
The method assumes that malloc creates memory that is suitably aligned for the first
object.
The caller is responsible for calling free on the returned memory (ptr1).
----
@param size1        Size of the first instance
@param size2        Size of the second instance
@param alignment    alignment requirement for the second instance
@param ptr1         [out] address for the first object. This is the address that must
                    be passed to free when the memory should be reclaimed by the system.
@param ptr2         [out] address for the second object
*/
void AllocateInstancePair(  std::size_t size1,
                            std::size_t size2,
                            std::size_t alignment,
                            void*& ptr1,
                            void*& ptr2);

}   // namespace bch

#include <bch/common/header_suffix.hpp>

#endif  // BCH_MEMORY
