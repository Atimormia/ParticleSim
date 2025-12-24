#include <gtest/gtest.h>
#include "core/free_list.hpp"

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
#endif
TEST(FreeListPoolTest, AccessingFreeNodeTriggersAssert)
{
    FreeListPool<int> pool(1);
    size_t idx = pool.allocate();
    pool.deallocate(idx);

    EXPECT_DEATH(pool.get(idx), "Accessing free node");
}