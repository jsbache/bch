/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_SHARED_PTR_NC
#define BCH_SHARED_PTR_NC

#pragma once

#include "bch/shared_ptr_nc/prefix.hpp"

#include "bch/common/header_prefix.hpp"

namespace bch {

/* This method creates a new shared_ptr_nc which uses one less heap allocation
than the alternate version:
/code
    shared_ptr_nc<Foo> foo(new Foo);
/endcode
This has a potential small performance advantage to the alternate version.
The downside is that the memory for T is retained until the last weak pointer
has been released.
*/
template <typename T, typename ... Args>
shared_ptr_nc<T> make_shared(Args&&...);

template<typename T, typename U>
shared_ptr_nc<T> static_pointer_cast(const shared_ptr_nc<U>& ptr);

template<typename T, typename U>
shared_ptr_nc<T> dynamic_pointer_cast(const shared_ptr_nc<U>& ptr);

template<typename T, typename U>
shared_ptr_nc<T> const_pointer_cast(const shared_ptr_nc<const U>& ptr);

/* non concurrent implementation of a shared_ptr
*/
template <typename T>
class shared_ptr_nc
{
public:
    constexpr shared_ptr_nc() noexcept;
    template <typename U> explicit shared_ptr_nc(U* ptr);
    constexpr shared_ptr_nc(std::nullptr_t) noexcept;

    template <typename U> shared_ptr_nc(const shared_ptr_nc<U>& ptr) noexcept;
    shared_ptr_nc(const shared_ptr_nc& ptr) noexcept;

    template <typename U> shared_ptr_nc(shared_ptr_nc<U>&& ptr) noexcept;
    shared_ptr_nc(shared_ptr_nc&& ptr) noexcept;

    ~shared_ptr_nc();

    shared_ptr_nc& operator=(const shared_ptr_nc& ptr) noexcept;
    template <typename U>
    shared_ptr_nc& operator=(const shared_ptr_nc<U>& ptr) noexcept;

    shared_ptr_nc& operator=(shared_ptr_nc<T>&& ptr);
    template <typename U>
    shared_ptr_nc<T>& operator=(shared_ptr_nc<U>&& ptr);

    void swap(shared_ptr_nc& r) noexcept;
    void reset();
    template <typename U>
    void reset(U* ptr);

    T* get() const noexcept;
    T& operator*() const noexcept;
    T* operator->() const noexcept;

    long use_count() const noexcept {
        return static_cast<long>((mHandle != nullptr) ? mHandle->use_count() : 0);
    }

    bool unique() const noexcept {
        return (use_count() == 1);
    }

    explicit operator bool() const noexcept {
        return (mPtr != nullptr);
    }

#if BCH_SMART_PTR_UNITTEST
    std::uint32_t weak_count() const;
#endif

private:
    explicit shared_ptr_nc(detail::ControlBlock* handle, T* ptr, bool increaseRefCount) noexcept;

    template <typename U>
    friend class weak_ptr;

    template <typename U>
    friend class shared_ptr_nc;

    template <typename U, typename ... Args>
    friend shared_ptr_nc<U> make_shared(Args&&...);

    template <typename U, typename V>
    friend shared_ptr_nc<U> static_pointer_cast(const shared_ptr_nc<V>&);

    template <typename U, typename V>
    friend shared_ptr_nc<U> dynamic_pointer_cast(const shared_ptr_nc<V>&);

    template <typename U, typename V>
    friend shared_ptr_nc<U> const_pointer_cast(const shared_ptr_nc<const V>&);

    template <typename U>
    friend class enable_shared_from_this;

    T*                      mPtr{nullptr};
    detail::ControlBlock*   mHandle{nullptr};
};

/* Weak pointer mathing the Shared Pointer
*/
template <typename T>
class weak_ptr
{
public:
    constexpr weak_ptr() noexcept = default;
    ~weak_ptr();

    weak_ptr(const weak_ptr<T>& ptr) noexcept;
    template <typename U>
    weak_ptr(const weak_ptr<U>& ptr) noexcept;

    template <typename U>
    weak_ptr(weak_ptr<U>&& ptr) noexcept;

    template <typename U>
    weak_ptr(const shared_ptr_nc<U>& ptr) noexcept;

    weak_ptr& operator=(const weak_ptr& ptr) noexcept;
    template <typename U>
    weak_ptr& operator=(const weak_ptr<U>& ptr) noexcept;

    weak_ptr& operator=(weak_ptr<T>&& ptr);
    template <typename U>
    weak_ptr& operator=(weak_ptr<U>&& ptr);

    weak_ptr& operator=(const shared_ptr_nc<T>& ptr);

    /* Create a shared_ptr_nc from the weak pointer. This method will return a
    shared_pointer containing a null pointer if the referenced instance has been
    delete (or if the weak_pointer originally was created from a null shared pointer).
    */
    shared_ptr_nc<T> lock() const;

    bool expired() const;

    void reset() const;

#if BCH_SMART_PTR_UNITTEST
    std::uint32_t strong_count() const;
    std::uint32_t weak_count() const;
#endif

private:
    void Assign(T* ptr, detail::ControlBlock* handle) noexcept;

    mutable T*                      mPtr{nullptr};
    mutable detail::ControlBlock*   mHandle{nullptr};
};

template <typename T>
class enable_shared_from_this {
public:
    shared_ptr_nc<T> shared_from_this();
    shared_ptr_nc<const T> shared_from_this() const;

private:
    template <typename U>
    friend void detail::set_shared_from_this(U* ptr, detail::ControlBlock* cb);

    mutable detail::ControlBlock*   _shared_from_this_data{nullptr};
};

}   // namespace bch

#include "bch/common/header_suffix.hpp"

#include "bch/shared_ptr_nc/suffix.hpp"

#endif  // BCH_SHARED_PTR_NC
