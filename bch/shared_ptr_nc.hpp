/**
Copyright: Jesper Storm Bache (bache.name)
*/

#ifndef BCH_SHARED_PTR_NC

/* Preprocessor defines that are used by the smart-pointer implementation.
General pattern is that the project that use the functionality use the _ENABLE
variation, and we then calculate necessary preprocessor symbols based n this.
We rely on DEBUG being defined in debug builds.
BCH_SMART_PTR_DEBUG_ENABLE can be defined to 1 to add debug tests
BCH_SMART_PTR_UNITTEST_ENABLE can be defined to 1 to add code that is needed by
    unit tests
*/
#ifdef BCH_SMART_PTR_DEBUG
#error "BCH_SMART_PTR_DEBUG_ENABLE should be used rather than BCH_SMART_PTR_DEBUG"
#endif
#ifdef BCH_SMART_PTR_UNITTEST
#error "BCH_SMART_PTR_UNITTEST_ENABLE should be used rather than BCH_SMART_PTR_UNITTEST"
#endif

#ifndef BCH_SMART_PTR_DEBUG_ENABLE
#define BCH_SMART_PTR_DEBUG_ENABLE 0
#endif

#if BCH_SMART_PTR_DEBUG_ENABLE
    #define BCH_SMART_PTR_DEBUG 1

    #ifndef BCH_SMART_PTR_UNITTEST_ENABLE
    #define BCH_SMART_PTR_UNITTEST_ENABLE 0
    #endif

    #define BCH_SMART_PTR_UNITTEST BCH_SMART_PTR_UNITTEST_ENABLE

#else
    #define BCH_SMART_PTR_DEBUG 0
    #define BCH_SMART_PTR_UNITTEST 0
#endif


#include <bch/shared_ptr_nc/prefix.hpp>

#include <bch/common/header_prefix.hpp>

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
shared_ptr_nc<T> const_pointer_cast(const shared_ptr_nc<U>& ptr);

/* non concurrent implementation of a shared_ptr
*/
template <typename T>
class shared_ptr_nc
{
public:
    constexpr shared_ptr_nc() noexcept;
    constexpr shared_ptr_nc(std::nullptr_t) noexcept;
    ~shared_ptr_nc();

    template <typename U>
    explicit shared_ptr_nc(U* ptr);

    shared_ptr_nc(const shared_ptr_nc& ptr) noexcept;
    template <typename U>
    shared_ptr_nc(const shared_ptr_nc<U>& ptr) noexcept;

    shared_ptr_nc& operator=(const shared_ptr_nc& ptr) noexcept;
    template <typename U>
    shared_ptr_nc& operator=(const shared_ptr_nc<U>& ptr) noexcept;

    template <typename U>
    shared_ptr_nc(shared_ptr_nc<U>&& ptr) noexcept;

    shared_ptr_nc& operator=(shared_ptr_nc<T>&& ptr);
    template <typename U>
    shared_ptr_nc<T>& operator=(shared_ptr_nc<U>&& ptr);

    void reset();
    template <typename U>
    void reset(U* ptr);

    T* get() const noexcept;
    operator T*() const noexcept;
    
    T& operator*() const noexcept;
    T* operator->() const noexcept;

#if BCH_SMART_PTR_UNITTEST
    uint32_t strong_count() const;
    uint32_t weak_count() const;
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
    friend shared_ptr_nc<U> const_pointer_cast(const shared_ptr_nc<V>&);

    T*                      mPtr;
    detail::ControlBlock*   mHandle;
};

/* Weak pointer mathing the Shared Pointer
*/
template <typename T>
class weak_ptr
{
public:
    constexpr weak_ptr() noexcept;
    ~weak_ptr();

    weak_ptr(const weak_ptr<T>& ptr) noexcept;
    template <typename U>
    weak_ptr(const weak_ptr<U>& ptr) noexcept;

    template <typename U>
    weak_ptr(weak_ptr<U>&& ptr) noexcept;

    template <typename U>
    weak_ptr(shared_ptr_nc<U>& ptr) noexcept;

    weak_ptr& operator=(const weak_ptr& ptr) noexcept;
    template <typename U>
    weak_ptr& operator=(const weak_ptr<U>& ptr) noexcept;

    weak_ptr& operator=(weak_ptr<T>&& ptr);
    template <typename U>
    weak_ptr& operator=(weak_ptr<U>&& ptr);

    /* Create a shared_ptr_nc from the weak pointer. This method will return a
    shared_pointer containing a null pointer if the referenced instance has been
    delete (or if the weak_pointer originally was created from a null shared pointer).
    */
    shared_ptr_nc<T> lock();

#if BCH_SMART_PTR_UNITTEST
    uint32_t strong_count() const;
    uint32_t weak_count() const;
#endif

private:
    void Reset();
    void Assign(T* ptr, detail::ControlBlock* handle) noexcept;

    T*                      mPtr;
    detail::ControlBlock*   mHandle;
};

}   // namespace bch
#include <bch/common/header_suffix.hpp>

#include <bch/shared_ptr_nc/suffix.hpp>

#endif  // BCH_SHARED_PTR_NC
