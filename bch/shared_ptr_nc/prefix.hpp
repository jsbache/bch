/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_SHARED_PTR_PREFIX
#define BCH_SHARED_PTR_PREFIX

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <stdlib.h>
#include <memory>

#include <bch/common/header_prefix.hpp>

namespace bch {

template <typename T>
class shared_ptr_nc;

template <typename T, typename ... Args>
shared_ptr_nc<T> make_shared(Args&&...);

namespace detail {
/* Shared data that manages the lifetime of a shared instance.
The class holds a strong and a weak reference count.
Rules:
- The strong reference count is 1 at creation.
- The weak reference count is 0 at creation.
- When the strong reference count reaches 0, then it can never increase again.
- When the strong reference count reaches 0, then the destructor of the managed
    instance is executed. The memory of the managed instance is deleted if the
    smart pointer was created by wrapping an existing instance.
    If make_shared was used, then the memory of the control block and the instance
    is shared, and we cannot delete the memory until the control block can be deleted.
- When the strong and the weak reference count reaches 0, then the control block
    memory is released.
*/
class ControlBlock
{
public:
#if BCH_SMART_PTR_UNITTEST
    ~ControlBlock();
#endif

    // Increase the strong reference count
    void add_shared() noexcept;

    /* Decrease the strong reference count. If the reference count reaches 0, then
    we invoke the destructor of the provided ptr and optionally release its memory
    (depending on whether or not we are sharing memory between the control block
    and the instance [aka. inline]).
    The control block may be released during the execution of this class.
    This method cannot be noexcept as we don't know the exception property of
    T's destructor.
    template <typename T>
    void release_shared(T* ptr);
    */
    void release_shared();

    // Increase the weak reference count
    void add_weak() noexcept;

    /* Decrease the weak reference count
    The control block may be released during the execution of this class.
    */
    void release_weak() noexcept;

    // Return true if the managed instance and the control block share memory
    bool is_inline() const noexcept;
    
    // Return true if the strong reference count is > 0
    bool has_shared_references() const noexcept;

#if BCH_SMART_PTR_UNITTEST
    uint32_t strong_count() const;
    uint32_t weak_count() const;

    /// Number of live control block instances
    static uint32_t live_instance_count() noexcept;
#endif

protected:
    ControlBlock() noexcept;

    virtual void on_zero_shared() = 0;

private:
    ControlBlock(const ControlBlock&) = delete;
    ControlBlock(ControlBlock&&) = delete;
    ControlBlock& operator=(const ControlBlock&) = delete;
    ControlBlock& operator=(ControlBlock&&) = delete;

    void adjust() noexcept;

    /* Reference count.
    Performance for changing the reference count is on the order of 100,000,000 per second.
    The size of the control block is around 10 bytes depending on the ref count data type.
    The size of the shared pointer instance is two pointers = 16 bytes for a 64 bit implementation.
    
    Clang (with Xcode 6.2) uses "long" as data type for the reference count of std::shared_ptr.
    long is a 64 bit value.
    This seems excessively large.

    With a 32 bit value, we have the ability to have a ref count of more than 4E+9.
    If we had a use case where we used 4E+9, then we would have:
    7.5 GB of data used for the smart pointer instances.
    It would take 40 seconds to free an instance that has reached the maximum
    reference count.
    The same calculations for a 16 bit value is:
    We would use about 1 MB of shared pointer instance data.
    It would take 0.7 milliseconds to free an instance that has reached the maximum
    reference count.
    I have not seen a use case, where we need a reference count of 65535 or anything
    close to it, but I choose the 32 bit value to leave a comfortable gap between the
    reference count value range and valid use cases.
    */
    uint32_t    mStrong;
    uint32_t    mWeak;
    
#if BCH_SMART_PTR_UNITTEST
    static void register_cb_ctor() noexcept;
    static void register_cb_dtor() noexcept;
#endif
};

inline ControlBlock::
ControlBlock() noexcept :
    mStrong(1),
    mWeak(0)
{
#if BCH_SMART_PTR_UNITTEST
    register_cb_ctor();
#endif
}

#if BCH_SMART_PTR_UNITTEST
inline ControlBlock::
~ControlBlock()
{
    register_cb_dtor();
}
#endif

inline void ControlBlock::
add_shared() noexcept
{
#if BCH_SMART_PTR_DEBUG
    assert(mStrong > 0);
#endif

    ++mStrong;

#if BCH_SMART_PTR_DEBUG
    // If we reach 1M references to the same instance, then something is likely to be wrong.
    assert(mStrong < 1000000);
#endif
}

inline bool ControlBlock::
has_shared_references() const noexcept
{
    return (mStrong > 0);
}

inline void ControlBlock::
add_weak() noexcept
{
    ++mWeak;

#if BCH_SMART_PTR_DEBUG
    // If we reach 1M references to the same instance, then something is likely to be wrong.
    assert(mWeak < 1000000);
#endif
}

inline void ControlBlock::
release_shared()
{
    if (--mStrong == 0)
    {
        // temp weak ptr around releasing the shared ptr. This is to ensure that
        // the control block is kept alive during the dtor call.
        // If the dtor tries to lock a weak ptr to self, then we would otherwise
        // delete the control block inside the call to on_zero_shared
        
        ++mWeak;
        // TODO: Not exception safe - but std spec says that if the dtor throws
        // then functionality of standard library is undefined.
        on_zero_shared();
        --mWeak;
        
        adjust();
    }
}

inline void ControlBlock::
release_weak() noexcept
{
    if (--mWeak == 0)
        adjust();
}

template <typename T>
class ControlBlockDeleter: public ControlBlock
{
public:
    static ControlBlockDeleter* Create(T* ptr);

protected:
    virtual void on_zero_shared()
    {
        delete mPtr;
    }

private:
    explicit ControlBlockDeleter(T* ptr) :
        mPtr(ptr)
    { }

    ControlBlockDeleter(const ControlBlockDeleter&) = delete;
    ControlBlockDeleter(ControlBlockDeleter&&) = delete;
    ControlBlockDeleter& operator=(const ControlBlockDeleter&) = delete;
    ControlBlockDeleter& operator=(ControlBlockDeleter&&) = delete;

    T*      mPtr;
};

template <typename T>
ControlBlockDeleter<T>* ControlBlockDeleter<T>::Create(T* ptr)
{
    typedef ControlBlockDeleter<T> CBType;
    void* cbData = malloc(sizeof(CBType));
    return new (cbData) CBType(ptr);
}

template <typename T>
class ControlBlockDeleterInlineData: public ControlBlock
{
public:
    explicit ControlBlockDeleterInlineData(T* ptr) :
        mPtr(ptr)
    {
        // never NULL as this is invoked by make_shared when a new instance has
        // been created
        assert(mPtr != nullptr);
    }

protected:
    virtual void on_zero_shared()
    {
        mPtr->~T();
    }

private:
    ControlBlockDeleterInlineData(const ControlBlockDeleterInlineData&) = delete;
    ControlBlockDeleterInlineData(ControlBlockDeleterInlineData&&) = delete;
    ControlBlockDeleterInlineData& operator=(const ControlBlockDeleterInlineData&) = delete;
    ControlBlockDeleterInlineData& operator=(ControlBlockDeleterInlineData&&) = delete;

    T*      mPtr;
};

}   // namespace detail
}   //namespace bch

#include <bch/common/header_suffix.hpp>

#endif  // BCH_SHARED_PTR_PREFIX
