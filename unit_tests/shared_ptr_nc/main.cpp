//
//  main.cpp
//  shared_ptr
//
//  Created by Jesper Storm Bache on 7/5/15.
//  Copyright (c) 2015 Jesper Storm Bache. All rights reserved.
//

#include <chrono>
#include <iostream>
#include <thread>

#include "bch/shared_ptr_nc.hpp"

#include "correctness.hpp"
#include "performance.hpp"

namespace bch {

class alignas(4) Test
{
public:
    Test()
    {
#if BCH_SMART_PTR_UNITTEST
        std::cout << "Test ctor(1)" << std::endl;
#endif
        Fill(1);
    }
    explicit Test(int v)
    {
#if BCH_SMART_PTR_UNITTEST
        std::cout << "Test ctor(2)" << std::endl;
#endif
        Fill(v);
    }
    ~Test()
    {
#if BCH_SMART_PTR_UNITTEST
        std::cout << "Test dtor" << std::endl;
#endif
    }
    
    void Baz()
    {
    }

private:
    void Fill(int v)
    {
        for (std::size_t i = 0; i < sizeof(mValue)/sizeof(*mValue); ++i)
            mValue[i] = static_cast<char>(v++);
    }
    char mValue[4];
};

class TestDer: public Test
{
public:
    TestDer()
    {
#if BCH_SMART_PTR_UNITTEST
        std::cout << "TestDer ctor" << std::endl;
#endif
    }
    ~TestDer()
    {
#if BCH_SMART_PTR_UNITTEST
        std::cout << "TestDer dtor" << std::endl;
#endif
    }
};

template <typename T>
void Foo(T value)
{
    value->Baz();
}

std::mutex mutex;
double stdPtrTime = 0;
double noThreadTime = 0;

class SizeTest
{
    char x;
};
}   // namespace bch

using namespace bch;

int main(int argc, const char * argv[])
{
#if BCH_SMART_PTR_UNITTEST
    bch::unittest::shared_ptr_nc::Perform();
#endif

#if !BCH_SMART_PTR_UNITTEST
    bch::unittest::shared_ptr_nc::TestPerformance();
#endif

    return 0;
}

namespace example {
typedef int MyObject;
typedef std::shared_ptr<MyObject> MyObjectPtr;

static void Foo(MyObject& object)
{
}

static void Bar(const std::shared_ptr<MyObject>& pointer)
{
}

static void Baz()
{
    MyObjectPtr instance;
    Foo(*instance);
    Bar(instance);
}

}   // namespace example
