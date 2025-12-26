#include <gtest/gtest.h>
#include "core/free_list.hpp"
#include "core/memory_arena.hpp"

using namespace core;

TEST(FreeListPoolTest, AllocateUpToCapacity)
{
    FreeListPool<int> pool(4);

    size_t a = pool.allocate();
    size_t b = pool.allocate();
    size_t c = pool.allocate();
    size_t d = pool.allocate();

    EXPECT_NE(a, INVALID_INDEX);
    EXPECT_NE(b, INVALID_INDEX);
    EXPECT_NE(c, INVALID_INDEX);
    EXPECT_NE(d, INVALID_INDEX);

    EXPECT_EQ(pool.allocate(), INVALID_INDEX);
}

TEST(FreeListPoolTest, DeallocateAndReuse)
{
    FreeListPool<int> pool(3);

    size_t a = pool.allocate();
    size_t b = pool.allocate();

    pool.deallocate(a);
    size_t c = pool.allocate();

    EXPECT_EQ(c, a);
}

TEST(FreeListPoolTest, StableStorage)
{
    FreeListPool<int> pool(2);

    size_t idx = pool.allocate();
    pool.get(idx) = 42;

    EXPECT_EQ(pool.get(idx), 42);
}

#ifndef NDEBUG
TEST(FreeListPoolTest, DoubleFreeTriggersAssert)
{
    FreeListPool<int> pool(1);
    size_t idx = pool.allocate();

    pool.deallocate(idx);
    EXPECT_DEATH(pool.deallocate(idx), "Double free");
}

TEST(FreeListPoolTest, AccessingFreeNodeTriggersAssert)
{
    FreeListPool<int> pool(1);
    size_t idx = pool.allocate();
    pool.deallocate(idx);

    EXPECT_DEATH(pool.get(idx), "Accessing free node");
}
#endif

TEST(FrameArenaTest, AllocateSingleObject)
{
    FrameArena arena(1024);

    int *p = arena.allocate<int>();
    ASSERT_NE(p, nullptr);
    *p = 42;
    EXPECT_EQ(*p, 42);
}

TEST(FrameArenaTest, AllocateArray)
{
    FrameArena arena(1024);

    int *arr = arena.allocateArray<int>(10);
    ASSERT_NE(arr, nullptr);

    for (int i = 0; i < 10; ++i)
        arr[i] = i * 2;

    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(arr[i], i * 2);
}

struct alignas(32) Aligned32
{
    uint64_t data[4];
};

TEST(FrameArenaTest, AllocationRespectsAlignment)
{
    FrameArena arena(1024);

    Aligned32 *p = arena.allocate<Aligned32>();
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);

    EXPECT_EQ(addr % alignof(Aligned32), 0u);
}

TEST(FrameArenaTest, SequentialAllocationsDoNotOverlap)
{
    FrameArena arena(1024);

    int *a = arena.allocate<int>();
    int *b = arena.allocate<int>();

    EXPECT_NE(a, b);
    EXPECT_LT(reinterpret_cast<uintptr_t>(a),
              reinterpret_cast<uintptr_t>(b));
}

TEST(FrameArenaTest, MixedTypeAllocations)
{
    FrameArena arena(1024);

    int *i = arena.allocate<int>();
    float *f = arena.allocate<float>();
    double *d = arena.allocate<double>();

    *i = 1;
    *f = 2.5f;
    *d = 3.0;

    EXPECT_EQ(*i, 1);
    EXPECT_FLOAT_EQ(*f, 2.5f);
    EXPECT_DOUBLE_EQ(*d, 3.0);
}

TEST(FrameArenaTest, ThrowsOnOutOfMemory)
{
    FrameArena arena(sizeof(int) * 4);

    arena.allocateArray<int>(4);

    EXPECT_THROW(
        arena.allocate<int>(),
        std::bad_alloc);
}

TEST(FrameArenaTest, LargeArrayAllocation)
{
    FrameArena arena(1024);

    constexpr size_t count = 128;
    int *arr = arena.allocateArray<int>(count);

    ASSERT_NE(arr, nullptr);
    for (size_t i = 0; i < count; ++i)
        arr[i] = static_cast<int>(i);

    for (size_t i = 0; i < count; ++i)
        EXPECT_EQ(arr[i], static_cast<int>(i));
}

#ifndef NDEBUG

struct NonTrivial
{
    ~NonTrivial() {}
};

TEST(FrameArenaTest, RejectsNonTriviallyDestructibleTypes)
{
    FrameArena arena(1024);

    EXPECT_DEATH(
        arena.allocate<NonTrivial>(),
        "trivially destructible");
}
#endif
