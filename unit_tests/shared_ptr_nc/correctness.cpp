/**
Copyright: Jesper Storm Bache (bache.name)
*/

#include "./correctness.hpp"

#include <bch/shared_ptr_nc.hpp>

#if BCH_SMART_PTR_UNITTEST
#include <iostream>
#include <cassert>

namespace unittest {

void Require(bool expr)
{
    assert(expr);
}

}   // unittest

#define UNITTEST_REQUIRE(expr)      unittest::Require(expr)

namespace {

// -----------------------------------------------------------------------------
// Utilities

template <typename T>
class InstanceValidator
{
public:
    InstanceValidator() :
        mInitialCount(GetInstanceCount())
    { }

    void ValidateInitialState() const;
    void ValidateDelta(uint32_t delta) const;

private:
    static uint32_t GetInstanceCount()
    {
        return T::LiveInstanceCount();
    }

    const uint32_t      mInitialCount;
};

template <typename T>
inline void InstanceValidator<T>::ValidateInitialState() const
{
    ValidateDelta(0);
}

template <typename T>
void InstanceValidator<T>::ValidateDelta(uint32_t delta) const
{
    UNITTEST_REQUIRE(mInitialCount + delta == GetInstanceCount());
}

// -----------------------------------------------------------------------------

struct ControlBlockWrapper
{
    static uint32_t LiveInstanceCount()
    {
        return bch::detail::ControlBlock::live_instance_count();
    }
};

typedef InstanceValidator<ControlBlockWrapper>      ControlBlockInstanceValidator;

// -----------------------------------------------------------------------------

template <typename T>
void ValidateStrongCount(const bch::shared_ptr_nc<T>& ptr, uint32_t value)
{
    UNITTEST_REQUIRE(ptr.strong_count() == value);
}

template <typename T>
void ValidateWeakCount(const bch::shared_ptr_nc<T>& ptr, uint32_t value)
{
    UNITTEST_REQUIRE(ptr.weak_count() == value);
}

template <typename T>
void ValidateStrongCount(const bch::weak_ptr<T>& ptr, uint32_t value)
{
    UNITTEST_REQUIRE(ptr.strong_count() == value);
}

template <typename T>
void ValidateWeakCount(const bch::weak_ptr<T>& ptr, uint32_t value)
{
    UNITTEST_REQUIRE(ptr.weak_count() == value);
}

// -----------------------------------------------------------------------------

struct TestInstance
{
    TestInstance();
    ~TestInstance();
    static int32_t LiveInstanceCount();

    TestInstance(const TestInstance&) = delete;
    TestInstance(TestInstance&&) = delete;
    TestInstance& operator=(const TestInstance&) = delete;
    TestInstance& operator=(TestInstance&&) = delete;

private:
    static int32_t sInstanceCount;
};
int32_t TestInstance::sInstanceCount = 0;

TestInstance::TestInstance()
{
    ++sInstanceCount;
}
TestInstance::~TestInstance()
{
    --sInstanceCount;
}

int32_t TestInstance::LiveInstanceCount()
{
    return sInstanceCount;
}

typedef InstanceValidator<TestInstance>      TestInstanceValidator;


struct TestInstanceSubclass: public TestInstance
{
    TestInstanceSubclass();
    ~TestInstanceSubclass();
    static int32_t LiveInstanceCount();

    TestInstanceSubclass(const TestInstanceSubclass&) = delete;
    TestInstanceSubclass(TestInstanceSubclass&&) = delete;
    TestInstanceSubclass& operator=(const TestInstanceSubclass&) = delete;
    TestInstanceSubclass& operator=(TestInstanceSubclass&&) = delete;
private:
    static int32_t sInstanceCount;
};
int32_t TestInstanceSubclass::sInstanceCount = 0;

TestInstanceSubclass::TestInstanceSubclass()
{
    ++sInstanceCount;
}

TestInstanceSubclass::~TestInstanceSubclass()
{
    --sInstanceCount;
}

int32_t TestInstanceSubclass::LiveInstanceCount()
{
    return sInstanceCount;
}

typedef InstanceValidator<TestInstanceSubclass>      TestInstanceSubclassValidator;

// -----------------------------------------------------------------------------

struct TestInstanceVtable
{
    TestInstanceVtable();
    virtual ~TestInstanceVtable();
    static int32_t LiveInstanceCount();

    TestInstanceVtable(const TestInstanceVtable&) = delete;
    TestInstanceVtable(TestInstanceVtable&&) = delete;
    TestInstanceVtable& operator=(const TestInstanceVtable&) = delete;
    TestInstanceVtable& operator=(TestInstanceVtable&&) = delete;

private:
    static int32_t sInstanceCount;
};
int32_t TestInstanceVtable::sInstanceCount = 0;

TestInstanceVtable::TestInstanceVtable()
{
    ++sInstanceCount;
}
TestInstanceVtable::~TestInstanceVtable()
{
    --sInstanceCount;
}

int32_t TestInstanceVtable::LiveInstanceCount()
{
    return sInstanceCount;
}

typedef InstanceValidator<TestInstanceVtable>      TestInstanceVtableValidator;


struct TestInstanceSubclassVtable: public TestInstanceVtable
{
    TestInstanceSubclassVtable();
    ~TestInstanceSubclassVtable();
    static int32_t LiveInstanceCount();

    TestInstanceSubclassVtable(const TestInstanceSubclassVtable&) = delete;
    TestInstanceSubclassVtable(TestInstanceSubclassVtable&&) = delete;
    TestInstanceSubclassVtable& operator=(const TestInstanceSubclassVtable&) = delete;
    TestInstanceSubclassVtable& operator=(TestInstanceSubclassVtable&&) = delete;
private:
    static int32_t sInstanceCount;
};
int32_t TestInstanceSubclassVtable::sInstanceCount = 0;

TestInstanceSubclassVtable::TestInstanceSubclassVtable()
{
    ++sInstanceCount;
}

TestInstanceSubclassVtable::~TestInstanceSubclassVtable()
{
    --sInstanceCount;
}

int32_t TestInstanceSubclassVtable::LiveInstanceCount()
{
    return sInstanceCount;
}

typedef InstanceValidator<TestInstanceSubclassVtable>      TestInstanceSubclassVtableValidator;

// -----------------------------------------------------------------------------

void BasicTests()
{
    // --------------- shared_ptr ---------------

    // shared_ptr()
    {
        ControlBlockInstanceValidator cbValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
    }

    // shared_ptr()
    {
        ControlBlockInstanceValidator cbValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(nullptr);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
    }

    // shared_ptr(T* ptr) & ptr == nullptr
    {
        ControlBlockInstanceValidator cbValidator;
        {
            TestInstance* const nullInstance = nullptr;
            bch::shared_ptr_nc<TestInstance> foo(nullInstance);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
    }

    // shared_ptr(T* ptr) & ptr != nullptr
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr(T* ptr) & ptr != nullptr & ptr is a subclass of T
    // This tasl also validates that we capture the sub-class type in the ctor
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstanceSubclass);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        testInstanceSubclassValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // (shared_ptr(const shared_ptr& ptr)) & ptr == null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            cbValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            testInstanceValidator.ValidateDelta(0);

            bch::shared_ptr_nc<TestInstance> bar(foo);
            cbValidator.ValidateDelta(0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            testInstanceValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // (shared_ptr(const shared_ptr& ptr)) & ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            cbValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            testInstanceValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar(foo);
            cbValidator.ValidateDelta(1);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 0);
            testInstanceValidator.ValidateDelta(1);

            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // (shared_ptr<T>(const shared_ptr<U>& ptr)) & ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstanceSubclass);
            cbValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar(foo);
            cbValidator.ValidateDelta(1);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 0);
            testInstanceValidator.ValidateDelta(1);

            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T> operator=(const shared_ptr<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar;
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bar = foo;
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T> operator=(const shared_ptr<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar(new TestInstance);
            testInstanceValidator.ValidateDelta(2);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 0);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(2);
            
            bar = foo;
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T> operator=(const shared_ptr<U>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstanceSubclass> foo(new TestInstanceSubclass);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar;
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
            
            bar = foo;
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        testInstanceSubclassValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T>(shared_ptr<T>&&) & ptr == null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            foo = bch::shared_ptr_nc<TestInstance>();
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T>(shared_ptr<T>&&) & ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            foo = bch::shared_ptr_nc<TestInstance>();
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // shared_ptr<T>(shared_ptr<U>&&) & ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstanceSubclass);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            foo = bch::shared_ptr_nc<TestInstanceSubclass>();
            testInstanceValidator.ValidateDelta(0);
            testInstanceSubclassValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }
    
    // shared_ptr<T>(shared_ptr<U>&&) & ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::shared_ptr_nc<TestInstance> bar(new TestInstanceSubclass);
            testInstanceValidator.ValidateDelta(2);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(2);

            bar = std::move(foo);

            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }
    
    // reset of null with null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
            
            foo.reset();

            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // reset of null with non-null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
            
            foo.reset(new TestInstance);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // reset of null with non-null (sub-class)
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            testInstanceValidator.ValidateDelta(0);
            testInstanceSubclassValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
            
            foo.reset(new TestInstanceSubclass);

            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // reset of non-null with null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            foo.reset();

            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // reset of non-null with non-null and non-identity
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            foo.reset(new TestInstanceSubclass);
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        testInstanceValidator.ValidateInitialState();
        cbValidator.ValidateInitialState();
    }

    // --------------- weak_ptr ---------------
    // weak_ptr()
    {
        ControlBlockInstanceValidator cbValidator;
        {
            bch::weak_ptr<TestInstance> foo;
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
    }

    // weak_ptr()
    {
        ControlBlockInstanceValidator cbValidator;
        {
            bch::weak_ptr<TestInstance> foo;
            bch::weak_ptr<TestInstance> bar(foo);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr == null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo;
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
            
            bch::weak_ptr<TestInstance> bar(foo);
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateInitialState();
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr != null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> bar(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr != null + reset
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> bar(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            foo.reset();
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr != null + weak_ptr(&&)
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> bar(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            bar = bch::weak_ptr<TestInstance>();

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr != null + std::move (operator=(&&)
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> bar(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> baz;
            baz = std::move(bar);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            ValidateStrongCount(baz, 1);
            ValidateWeakCount(baz, 1);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr(shared_ptr) with ptr != null + operator=(&)
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> bar(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            bch::weak_ptr<TestInstance> baz;
            baz = bar;

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 2);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 2);
            ValidateStrongCount(baz, 1);
            ValidateWeakCount(baz, 2);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Lock with null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::weak_ptr<TestInstance> bar;

            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            cbValidator.ValidateDelta(0);
            
            bch::shared_ptr_nc<TestInstance> foo = bar.lock();

            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(0);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Lock with non null
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::weak_ptr<TestInstance> bar(foo);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            bch::shared_ptr_nc<TestInstance> baz = bar.lock();
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 2);
            ValidateWeakCount(bar, 1);
            ValidateStrongCount(baz, 2);
            ValidateWeakCount(baz, 1);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // weak_ptr with non null, reset , lock
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo(new TestInstance);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);

            bch::weak_ptr<TestInstance> bar(foo);
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 1);
            ValidateStrongCount(bar, 1);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            foo.reset();
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 1);
            cbValidator.ValidateDelta(1);
            
            // Note: here we test that lock resets a cb when the strong count
            // is 0
            bch::shared_ptr_nc<TestInstance> baz = bar.lock();
            testInstanceValidator.ValidateDelta(0);
            ValidateStrongCount(foo, 0);
            ValidateWeakCount(foo, 0);
            ValidateStrongCount(bar, 0);
            ValidateWeakCount(bar, 0);
            ValidateStrongCount(baz, 0);
            ValidateWeakCount(baz, 0);
            cbValidator.ValidateDelta(0);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Use make_shared<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo = bch::make_shared<TestInstance>();
            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Use make_shared<U>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        TestInstanceSubclassValidator testInstanceSubclassValidator;
        {
            bch::shared_ptr_nc<TestInstance> foo = bch::make_shared<TestInstanceSubclass>();
            testInstanceValidator.ValidateDelta(1);
            testInstanceSubclassValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 1);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
        testInstanceSubclassValidator.ValidateInitialState();
    }

    // Use static_pointer_cast<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstanceSubclass> foo = bch::make_shared<TestInstanceSubclass>();
            bch::shared_ptr_nc<TestInstance> bar(foo);
            bch::shared_ptr_nc<TestInstanceSubclass> baz = bch::static_pointer_cast<TestInstanceSubclass>(bar);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 3);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Use dynamic_pointer_cast<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceVtableValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<TestInstanceSubclassVtable> foo = bch::make_shared<TestInstanceSubclassVtable>();
            bch::shared_ptr_nc<TestInstanceVtable> bar(foo);
            bch::shared_ptr_nc<TestInstanceSubclassVtable> baz = bch::dynamic_pointer_cast<TestInstanceSubclassVtable>(bar);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 3);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }

    // Use const_pointer_cast<T>
    {
        ControlBlockInstanceValidator cbValidator;
        TestInstanceValidator testInstanceValidator;
        {
            bch::shared_ptr_nc<const TestInstanceSubclass> foo = bch::make_shared<TestInstanceSubclass>();
            bch::shared_ptr_nc<TestInstanceSubclass> baz = bch::const_pointer_cast<TestInstanceSubclass>(foo);

            testInstanceValidator.ValidateDelta(1);
            ValidateStrongCount(foo, 2);
            ValidateWeakCount(foo, 0);
            cbValidator.ValidateDelta(1);
        }
        cbValidator.ValidateInitialState();
        testInstanceValidator.ValidateInitialState();
    }
    
    // Ensure that a weak ptr cannot be resolved during a call to the destructor
    {
        class DtorTest;
        typedef bch::shared_ptr_nc<DtorTest>    DtorTestPtr;
        typedef bch::weak_ptr<DtorTest>         DtorTestWeakPtr;
        class DtorTest
        {
        public:
            static DtorTestPtr Create()
            {
                DtorTestPtr result(new DtorTest);
                result->mWeakSelf = result;
                return result;
            }

            ~DtorTest()
            {
                DtorTestPtr ptr = mWeakSelf.lock();
                UNITTEST_REQUIRE(ptr == nullptr);
            }

        private:
            DtorTest()
            {
            }
            DtorTestWeakPtr     mWeakSelf;
        };

        DtorTestPtr test = DtorTest::Create();
      
    }
}

// -----------------------------------------------------------------------------

struct alignas(8) Test03
{
    Test03()
    {
        Fill(1);
    }
    ~Test03()
    {
    }
    
    void Fill(int v)
    {
        for (int i = 0; i < sizeof(mValue)/sizeof(*mValue); ++i)
            mValue[i] = v++;
    }
    char mValue[4];
};

void AlignmentTest()
{
    bch::shared_ptr_nc<Test03> foo = bch::make_shared<Test03>();
    Test03* const ptr = foo.get();
    const uintptr_t valuePtr = reinterpret_cast<uintptr_t>(&(ptr->mValue[0]));
    const uintptr_t kAlignmentMask = alignof(Test03) - 1;
    UNITTEST_REQUIRE((valuePtr & kAlignmentMask) == 0);
}

// -----------------------------------------------------------------------------

}   // namespace

namespace bch {
namespace unittest {
namespace shared_ptr_nc {

void Perform()
try
{
    BasicTests();
    AlignmentTest();

    std::cout << "unit tests for shared_ptr_nc succeeded " << std::endl;
}
catch(...)
{
    std::cout << "unit tests for shared_ptr_nc failed " << std::endl;
}

}   // namespace shared_ptr_nc
}   // namespace unittest
}   // namespace bch

#endif // BCH_SMART_PTR_UNITTEST
