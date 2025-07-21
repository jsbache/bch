/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_SHARED_PTR_CORRECTNESS_TESTS
#define BCH_SHARED_PTR_CORRECTNESS_TESTS

#include "bch/common/header_prefix.hpp"

#if BCH_SMART_PTR_UNITTEST

namespace bch {
namespace unittest {
namespace shared_ptr_nc {

void Perform();

}   // namespace shared_ptr_nc
}   // namespace unittest
}   // namespace bch

#endif  // BCH_SMART_PTR_UNITTEST

#include "bch/common/header_suffix.hpp"

#endif // BCH_SHARED_PTR_CORRECTNESS_TESTS
