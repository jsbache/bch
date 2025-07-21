/**
Copyright: Jesper Storm Bache (bache.name)
*/

#include "bch/shared_ptr_nc.hpp"

#if BCH_SMART_PTR_UNITTEST
#include <atomic>
#endif


#if BCH_SMART_PTR_UNITTEST
namespace {
std::atomic_ulong sCBInstanceCount;
}   // namespace
#endif

namespace bch {
namespace detail {

#if BCH_SMART_PTR_UNITTEST
void ControlBlock::register_cb_ctor() noexcept
{
    ++sCBInstanceCount;
}

void ControlBlock::register_cb_dtor() noexcept
{
    --sCBInstanceCount;
}

uint32_t ControlBlock::live_instance_count() noexcept
{
    return static_cast<uint32_t>(sCBInstanceCount.load());
}
#endif
}   // namespace detail
}   // namespace bch
