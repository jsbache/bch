/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_SHARED_PTR_NC_SUFFIX

#include <utility>
#include <bch/common/memory.hpp>

namespace bch {

template <typename T>
inline shared_ptr_nc<T>::~shared_ptr_nc()
{
    if (mHandle != nullptr)
        mHandle->release_shared();
}

template <typename T>
inline constexpr
shared_ptr_nc<T>::shared_ptr_nc() noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
}

template <typename T>
inline constexpr
shared_ptr_nc<T>::shared_ptr_nc(std::nullptr_t) noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
}

template <typename T>
template <typename U>
inline
shared_ptr_nc<T>::shared_ptr_nc(U* ptr) :
    mHandle(nullptr),
    mPtr(static_cast<T*>(ptr))
{
    if (mPtr != nullptr)
        mHandle = detail::ControlBlockDeleter<U>::Create(ptr);
}

template <typename T>
inline
shared_ptr_nc<T>::
shared_ptr_nc(const shared_ptr_nc& ptr) noexcept :
    mPtr(ptr.mPtr)
{
    mHandle = ptr.mHandle;
    if (mHandle != nullptr)
        mHandle->add_shared();
}

template <typename T>
template <typename U>
inline
shared_ptr_nc<T>::
shared_ptr_nc(const shared_ptr_nc<U>& ptr) noexcept :
    mPtr(static_cast<T*>(ptr.mPtr))
{
    mHandle = ptr.mHandle;
    if (mHandle != nullptr)
        mHandle->add_shared();
}

template <typename T>
shared_ptr_nc<T>&
shared_ptr_nc<T>::operator=(const shared_ptr_nc& ptr) noexcept
{
    if (this != &ptr)
    {
        reset();
        if ((mHandle = ptr.mHandle) != nullptr)
        {
            mHandle->add_shared();
            mPtr = ptr.mPtr;
        }
    }
    return *this;
}

template <typename T>
template <typename U>
shared_ptr_nc<T>&
shared_ptr_nc<T>::operator=(const shared_ptr_nc<U>& ptr) noexcept
{
    reset();
    if ((mHandle = ptr.mHandle) != nullptr)
    {
        mHandle->add_shared();
        mPtr = static_cast<T*>(ptr.mPtr);
    }
    return *this;
}

template <typename T>
void shared_ptr_nc<T>::reset()
{
    if (mHandle != nullptr)
    {
        mHandle->release_shared();
        mHandle = nullptr;
        mPtr = nullptr;
    }
}

template <typename T>
template <typename U>
void shared_ptr_nc<T>::reset(U* ptr)
{
    reset();
    if (ptr != nullptr)
    {
        mHandle = detail::ControlBlockDeleter<U>::Create(ptr);
        mPtr = static_cast<T*>(ptr);
    }
}

template <typename T>
template <typename U>
shared_ptr_nc<T>::shared_ptr_nc(shared_ptr_nc<U>&& ptr) noexcept :
    mHandle(ptr.mHandle),
    mPtr(static_cast<T*>(ptr.mPtr))
{
    ptr.mHandle = nullptr;
    ptr.mPtr = nullptr;
}

template <typename T>
shared_ptr_nc<T>& shared_ptr_nc<T>::operator=(shared_ptr_nc<T>&& ptr)
{
    if (this != &ptr)
    {
        reset();
        std::swap(mHandle, ptr.mHandle);
        std::swap(mPtr, ptr.mPtr);
    }
    return *this;
}

template <typename T>
template <typename U>
shared_ptr_nc<T>& shared_ptr_nc<T>::operator=(shared_ptr_nc<U>&& ptr)
{
    reset();
    std::swap(mHandle, ptr.mHandle);
    
    mPtr = static_cast<T*>(ptr.mPtr);
    ptr.mPtr = nullptr;
    return *this;
}

template <typename T>
inline T* shared_ptr_nc<T>::get() const noexcept
{
    return mPtr;
}

template <typename T>
inline shared_ptr_nc<T>::operator T*() const noexcept
{
    return mPtr;
}

template <typename T>
inline T& shared_ptr_nc<T>::operator*() const noexcept
{
    return *mPtr;
}

template <typename T>
inline T* shared_ptr_nc<T>::operator->() const noexcept
{
    return mPtr;
}

template <typename T>
inline
shared_ptr_nc<T>::shared_ptr_nc(detail::ControlBlock* handle, T* ptr, bool increaseRefCount) noexcept :
    mHandle(handle),
    mPtr(ptr)
{
    if (increaseRefCount && mHandle != nullptr)
        mHandle->add_shared();
}

#if BCH_SMART_PTR_UNITTEST
template <typename T>
inline
uint32_t shared_ptr_nc<T>::strong_count() const
{
    return (mHandle == nullptr) ? 0 : mHandle->strong_count();
}

template <typename T>
inline
uint32_t shared_ptr_nc<T>::weak_count() const
{
    return (mHandle == nullptr) ? 0 : mHandle->weak_count();
}
#endif

template <typename T>
inline
weak_ptr<T>::~weak_ptr()
{
    if (mHandle != nullptr)
        mHandle->release_weak();
}

template <typename T>
inline constexpr
weak_ptr<T>::weak_ptr() noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
}

template <typename T>
weak_ptr<T>::weak_ptr(const weak_ptr<T>& ptr) noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
    Assign(ptr.mPtr, ptr.mHandle);
}

template <typename T>
template <typename U>
weak_ptr<T>::weak_ptr(const weak_ptr<U>& ptr) noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
    Assign(ptr.mPtr, ptr.mHandle);
}

template <typename T>
template <typename U>
weak_ptr<T>::weak_ptr(shared_ptr_nc<U>& ptr) noexcept :
    mHandle(nullptr),
    mPtr(nullptr)
{
    Assign(ptr.mPtr, ptr.mHandle);
}

template <typename T>
template <typename U>
weak_ptr<T>::weak_ptr(weak_ptr<U>&& ptr) noexcept :
    mHandle(ptr.mHandle),
    mPtr(static_cast<T*>(ptr.mPtr))
{
    ptr.mHandle = nullptr;
    ptr.mPtr = nullptr;
}

template <typename T>
weak_ptr<T>&
weak_ptr<T>::operator=(const weak_ptr& ptr) noexcept
{
    if (this != &ptr)
    {
        Reset();
        Assign(ptr.mPtr, ptr.mHandle);
    }
    return *this;
}

template <typename T>
template <typename U>
weak_ptr<T>&
weak_ptr<T>::operator=(const weak_ptr<U>& ptr) noexcept
{
    Reset();
    Assign(ptr.mPtr, ptr.mHandle);
    return *this;
}

template <typename T>
weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<T>&& ptr)
{
    if (this != &ptr)
    {
        Reset();
        std::swap(mHandle, ptr.mHandle);
        std::swap(mPtr, ptr.mPtr);
    }
    return *this;
}

template <typename T>
template <typename U>
weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<U>&& ptr)
{
    Reset();
    std::swap(mHandle, ptr.mHandle);
    std::swap(mPtr, ptr.mPtr);
    return *this;
}

template <typename T>
shared_ptr_nc<T> weak_ptr<T>::lock()
{
    if (mHandle == nullptr)
        return shared_ptr_nc<T>();

    if (!mHandle->has_shared_references())
    {
        Reset();
        return shared_ptr_nc<T>();
    }

    return shared_ptr_nc<T>(mHandle, mPtr, true);
}

template <typename T>
void weak_ptr<T>::Reset()
{
    if (mHandle != nullptr)
    {
        mHandle->release_weak();
        mHandle = nullptr;
        mPtr = nullptr;
    }
}

template <typename T>
void weak_ptr<T>::Assign(T* ptr, detail::ControlBlock* handle) noexcept
{
    if (handle != nullptr && handle->has_shared_references())
    {
        mHandle = handle;
        mPtr = ptr;
        mHandle->add_weak();
    }
}

#if BCH_SMART_PTR_UNITTEST
template <typename T>
inline
uint32_t weak_ptr<T>::strong_count() const
{
    return (mHandle == nullptr) ? 0 : mHandle->strong_count();
}

template <typename T>
inline
uint32_t weak_ptr<T>::weak_count() const
{
    return (mHandle == nullptr) ? 0 : mHandle->weak_count();
}
#endif

#if BCH_SMART_PTR_UNITTEST
inline uint32_t detail::ControlBlock::strong_count() const
{
    return mStrong;
}

inline uint32_t detail::ControlBlock::weak_count() const
{
    return mWeak;
}
#endif

inline void detail::ControlBlock::adjust() noexcept
{
    if ((mStrong == 0) && (mWeak == 0))
    {
#if BCH_SMART_PTR_UNITTEST
        this->~ControlBlock();
#endif

        free(this);
    }
}

/** Create a shared pointer by creating an instance of T with the provided arguments.
The shared pointer will use a single memory allocation for both the control block and
the instance.
*/
template <typename T, typename ...Args>
shared_ptr_nc<T> make_shared(Args&& ... args)
{
    typedef detail::ControlBlockDeleterInlineData<T> ControlBlockType;

    void* cbAddress = nullptr;
    void* instanceAddress = nullptr;
    AllocateInstancePair(   sizeof(ControlBlockType),
                            sizeof(T),
                            alignof(T),
                            cbAddress,
                            instanceAddress);

    auto deleteWithFree = [](void* p) {free(p);};
    std::unique_ptr<void, decltype(deleteWithFree)> guard(cbAddress, deleteWithFree);

    /* Invoke constructors for the control block and for T.
    Only the constructor for T can throw an exception.
    We therefore invoke the ctor of T first and then if that succeds we invoke the ctor of the control block.
    */

    T* const ptr = new (instanceAddress) T(std::forward<Args>(args)...);

    ControlBlockType* const cbPtr = new (cbAddress) ControlBlockType(ptr);

    guard.release();

    return shared_ptr_nc<T>(cbPtr, ptr, false);
}

template<typename T, typename U>
shared_ptr_nc<T> static_pointer_cast(const shared_ptr_nc<U>& ptr)
{
    return shared_ptr_nc<T>(ptr.mHandle, static_cast<T*>(ptr.mPtr), true);
}

template<typename T, typename U>
shared_ptr_nc<T> dynamic_pointer_cast(const shared_ptr_nc<U>& ptr)
{
    return shared_ptr_nc<T>(ptr.mHandle, dynamic_cast<T*>(ptr.mPtr), true);
}

template<typename T, typename U>
shared_ptr_nc<T> const_pointer_cast(const shared_ptr_nc<U>& ptr)
{
    typedef typename std::remove_extent<T>::type Tp;
    return shared_ptr_nc<T>(ptr.mHandle, const_cast<Tp*>(ptr.mPtr), true);
}

}   // namespace bch

#endif // BCH_SHARED_PTR_NC_SUFFIX
