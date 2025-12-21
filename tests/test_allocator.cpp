#include <gtest/gtest.h>
#include "particlesim/particle.hpp"

using namespace particlesim;

// ---------- Construction ----------

TEST(ParticlePoolAllocator, ConstructWithInvalidCapacityThrows)
{
    EXPECT_THROW(ParticlePoolAllocator(0), std::invalid_argument);
}

TEST(ParticlePoolAllocator, CapacityIsReportedCorrectly)
{
    ParticlePoolAllocator alloc(16);
    EXPECT_EQ(alloc.capacity(), 16u);
}

// ---------- Allocation ----------

TEST(ParticlePoolAllocator, AllocateAllParticles)
{
    constexpr size_t N = 8;
    ParticlePoolAllocator alloc(N);

    std::vector<Particle*> particles;
    particles.reserve(N);

    for (size_t i = 0; i < N; ++i)
    {
        particles.push_back(alloc.allocate());
        EXPECT_NE(particles.back(), nullptr);
    }
}

TEST(ParticlePoolAllocator, AllocateBeyondCapacityThrows)
{
    constexpr size_t N = 4;
    ParticlePoolAllocator alloc(N);

    for (size_t i = 0; i < N; ++i)
        alloc.allocate();

    EXPECT_THROW(alloc.allocate(), std::bad_alloc);
}

// ---------- Deallocation ----------

TEST(ParticlePoolAllocator, DeallocateNullptrThrows)
{
    ParticlePoolAllocator alloc(4);
    EXPECT_THROW(alloc.deallocate(nullptr), std::invalid_argument);
}

TEST(ParticlePoolAllocator, DeallocateForeignPointerThrows)
{
    ParticlePoolAllocator alloc(4);
    Particle foreign{};
    EXPECT_THROW(alloc.deallocate(&foreign), std::invalid_argument);
}

// ---------- Reuse semantics ----------

TEST(ParticlePoolAllocator, FreeAndReallocateReusesSamePointer)
{
    ParticlePoolAllocator alloc(2);

    Particle* p1 = alloc.allocate();
    Particle* p2 = alloc.allocate();

    alloc.deallocate(p2);
    Particle* reused = alloc.allocate();

    EXPECT_EQ(reused, p2);
}

TEST(ParticlePoolAllocator, LIFOReuseOrderIsDeterministic)
{
    ParticlePoolAllocator alloc(3);

    Particle* p1 = alloc.allocate();
    Particle* p2 = alloc.allocate();
    Particle* p3 = alloc.allocate();

    alloc.deallocate(p1);
    alloc.deallocate(p2);
    alloc.deallocate(p3);

    EXPECT_EQ(alloc.allocate(), p3);
    EXPECT_EQ(alloc.allocate(), p2);
    EXPECT_EQ(alloc.allocate(), p1);
}

// ---------- Partial free / reuse ----------

TEST(ParticlePoolAllocator, FreeHalfThenReallocate)
{
    constexpr size_t N = 6;
    ParticlePoolAllocator alloc(N);

    std::vector<Particle*> particles;
    for (size_t i = 0; i < N; ++i)
        particles.push_back(alloc.allocate());

    // Free even indices
    for (size_t i = 0; i < N; i += 2)
        alloc.deallocate(particles[i]);

    // Reallocate freed slots
    std::vector<Particle*> reused;
    for (size_t i = 0; i < N / 2; ++i)
        reused.push_back(alloc.allocate());

    // Ensure reused pointers come from freed ones
    for (Particle* p : reused)
    {
        EXPECT_NE(std::find(particles.begin(), particles.end(), p), particles.end());
    }
}

// ---------- Debug-only behavior ----------

#ifndef NDEBUG
TEST(ParticlePoolAllocator, DoubleFreeTriggersAssertion)
{
    ParticlePoolAllocator alloc(1);

    Particle* p = alloc.allocate();
    alloc.deallocate(p);

    // EXPECT_DEATH is correct for assert-based checks
    EXPECT_DEATH(alloc.deallocate(p), "Double free detected");
}
#endif
