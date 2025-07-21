/**
Copyright: Jesper Storm Bache (bache.name)
*/

#include "performance.hpp"

#include "bch/shared_ptr_nc.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

namespace {
using namespace bch;

class Test
{
public:
    Test()
    {
    }

    ~Test()
    {
    }
    
    void Baz()
    {
    }
};

template <typename T>
void Foo(T value)
{
    value->Baz();
}

std::mutex mutex;
double stdPtrTime = 0;
double ncPtrTime = 0;

const unsigned int kTestCount = 100000000;
std::atomic_bool sStart;

static void std_pointer()
{
    while (!sStart) ;

    unsigned int count = kTestCount;
    typedef std::chrono::time_point<std::chrono::system_clock> TimerType;

    TimerType start = std::chrono::system_clock::now();

    std::shared_ptr<Test> stdValue(new Test);
    for (unsigned int i = 0; i < count; ++i)
    {
        Foo(stdValue);
    }
    TimerType end = std::chrono::system_clock::now();
 
    std::chrono::duration<double> elapsed_seconds = end-start;

    double elapsed = elapsed_seconds.count();;
    {
        mutex.lock();
        stdPtrTime += elapsed;
        mutex.unlock();
    }
}

static void nothread_pointer()
{
    while (!sStart) ;

    unsigned int count = kTestCount;
    typedef std::chrono::time_point<std::chrono::system_clock> TimerType;
    TimerType start = std::chrono::system_clock::now();

    shared_ptr_nc<Test> stdValue(new Test);
    for (unsigned int i = 0; i < count; ++i)
    {
        Foo(stdValue);
    }
    TimerType end = std::chrono::system_clock::now();
 
    std::chrono::duration<double> elapsed_seconds = end-start;
    double elapsed = elapsed_seconds.count();

    {
        mutex.lock();
        ncPtrTime += elapsed;
        mutex.unlock();
    }
}

template <typename T>
void TestRun(unsigned int count, T functor)
{
    sStart = false;
    typedef std::vector<std::thread> ThreadList;
    std::unique_ptr<ThreadList> threads(new ThreadList);
    
    for (unsigned int index = 0; index < count; ++index)
    {
        threads->push_back(std::thread(functor));
    }

    sStart = true;

    for (unsigned int index = 0; index < count; ++index)
    {
        threads->at(index).join();
    }
}

void Test(unsigned int threadCount)
{
    TestRun(threadCount, std_pointer);
    TestRun(threadCount, nothread_pointer);

    double delta = (stdPtrTime / ncPtrTime) * 100.0;

    std::cout << threadCount << '\t' << stdPtrTime << '\t' << ncPtrTime << '\t' << delta << std::endl << std::flush;

}

}   // namespace

namespace bch {
namespace unittest {
namespace shared_ptr_nc {

void TestPerformance()
{
    std::cout << "threads\tstd\tnc\tdelta" << std::endl << std::flush;

    for (unsigned int i = 1; i < 40; ++i)
    {
        Test(i);
    }
}

/*
Hardware Overview:

  Model Name:	MacBook Pro
  Model Identifier:	MacBookPro11,3
  Processor Name:	Intel Core i7
  Processor Speed:	2.3 GHz
  Number of Processors:	1
  Total Number of Cores:	4
  L2 Cache (per Core):	256 KB
  L3 Cache:	6 MB
  Memory:	16 GB

threads	std	nc	delta
1	1.69723	0.404006	420.101
2	4.85886	1.14477	424.439
3	9.68831	2.27942	425.034
4	16.2576	3.90373	416.462
5	26.1027	6.09592	428.199
6	39.8936	9.10469	438.166
7	58.0927	12.9209	449.603
8	81.5287	17.9108	455.193
9	110.725	24.1133	459.189
10	147.228	31.8417	462.373
11	191.121	41.4095	461.538
12	243.512	52.8975	460.348
13	305.031	66.5046	458.661
14	375.947	82.4599	455.915
15	456.559	100.717	453.308
16	549.424	121.927	450.619
17	653.768	145.235	450.145
18	770.248	170.944	450.584
19	900.099	199.372	451.467
20	1044.88	230.312	453.68
21	1204.67	264.344	455.723
22	1381	301.044	458.738
23	1573.55	341.067	461.36
24	1781.98	384.658	463.264
25	2010.65	431.786	465.659
26	2256.2	482.989	467.134
27	2522.12	538.187	468.632
28	2806.75	597.222	469.967
29	3112.31	660.709	471.056
30	3443.79	728.712	472.585
31	3790.38	801.66	472.816
32	4164.22	882.734	471.742
33	4561.33	964.97	472.692
34	4977.67	1052.12	473.108
35	5418.45	1144.77	473.322
36	5886.96	1243.09	473.576
37	6385.3	1346.21	474.318
38	6912.66	1455.27	475.008
39	7468.39	1570.2	475.633
*/

/*
The following test times are telated to seeing the impact of thread synchronized
memory access for reference counts.

In shared_ptr_nc/prefix.hpp, replace:
    uint32_t    mStrong;
    uint32_t    mWeak;

with:
    std::atomic_uint_fast32_t mStrong;
    std::atomic_uint_fast32_t mWeak;

threads	std	nc	delta
1	1.78442	1.75108	101.904
2	4.92816	4.90977	100.374
3	9.74958	9.722	100.284
4	16.2531	16.2045	100.3
5	26.097	25.971	100.485
6	39.8896	39.4406	101.138
7	57.7811	57.0738	101.239
8	80.7496	79.3904	101.712
9	109.658	107.81	101.715
10	145.458	143.12	101.634
11	189.047	186.097	101.585
12	240.754	236.514	101.792
13	301.055	295.464	101.892
14	370.842	364.552	101.726
15	452.018	443.895	101.83
16	543.604	533.617	101.871
17	647.651	635.505	101.911
18	762.683	749.847	101.712
19	891.016	877.449	101.546
20	1034.82	1019.07	101.545
21	1193.73	1175.42	101.558
22	1366.24	1347.23	101.411
23	1562	1534.84	101.77
24	1770.18	1739.37	101.771
25	1996.45	1960.88	101.814
26	2238.66	2200.65	101.727
27	2501.62	2459.25	101.723
28	2784.77	2738.09	101.705
29	3085.85	3036.4	101.628
30	3407.83	3355.15	101.57
31	3751.21	3696.17	101.489
32	4121.38	4059.29	101.53
33	4515.64	4444.78	101.594
34	4927.16	4853.76	101.512
35	5365.5	5286.47	101.495
36	5827.81	5745.45	101.433
37	6314.68	6230.67	101.348
38	6832.34	6741.6	101.346
39	7382.23	7282.1	101.375
*/
}   // namespace shared_ptr_nc
}   // namespace unittest
}   // namespace bch
