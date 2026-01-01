#include <gtest/gtest.h>
#include "core/parallel_scheduler.hpp"

using namespace core;

TEST(ThreadPool, ParallelSum)
{
    ThreadPoolScheduler scheduler;
    std::vector<int> data(1'000'000, 1);
    std::atomic<int64_t> sum{0};

    scheduler.parallelFor(0, data.size(), 256, [&](size_t i) {
        sum.fetch_add(data[i], std::memory_order_relaxed);
    });

    EXPECT_EQ(sum.load(), data.size());
}

TEST(ThreadPool, AllTasksRun)
{
    ThreadPoolScheduler scheduler;
    std::atomic<int> counter{0};

    for (int i = 0; i < 1000; ++i)
        scheduler.submit([&] { counter.fetch_add(1); });

    scheduler.wait();
    EXPECT_EQ(counter.load(), 1000);
}

TEST(ThreadPool, Stress10kTasks)
{
    ThreadPoolScheduler scheduler;
    std::atomic<int> counter{0};

    for (int i = 0; i < 10'000; ++i)
        scheduler.submit([&] { counter.fetch_add(1); });

    scheduler.wait();
    EXPECT_EQ(counter.load(), 10'000);
}
