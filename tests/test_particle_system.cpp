#include "particlesim/particle.hpp"
#include "particlesim/particle_system.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace particlesim;

TEST(ParticleSystemAoSTest, AddAndSize)
{
    ParticleSystem<ParticleSystemDataAoS> ps;
    EXPECT_EQ(ps.size(), 0u);

    ps.addParticle(makeParticle());
    EXPECT_EQ(ps.size(), 1u);

    ps.addParticle(makeParticle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemAoSTest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataAoS> ps;
    Particle p = makeParticle(0, 0, 1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.addParticle(p);

    ps.update(1.0f, true);

    const auto &v = ps.get();
    const auto &particle = v[0];
    EXPECT_FLOAT_EQ(particle.velocity.x, 1.5f);
    EXPECT_FLOAT_EQ(particle.position.x, 1.5f);
    EXPECT_FLOAT_EQ(particle.lifetime, 0.1f);
}

TEST(ParticleSystemSoATest, AddAndSize)
{
    ParticleSystem<ParticleSystemDataSoA> ps;
    EXPECT_EQ(ps.size(), 0u);

    ps.addParticle(makeParticle());
    EXPECT_EQ(ps.size(), 1u);

    ps.addParticle(makeParticle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemSoATest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataSoA> ps;
    Particle p = makeParticle(0, 0, 1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.addParticle(p);

    ps.update(1.0f, true);

    const auto particles = ps.get();
    const auto &out = particles[0];

    EXPECT_FLOAT_EQ(out.position.x, 1.5f);
}

TEST(ParticleSystemTest, LifetimeExpiration)
{
    ParticleSystem<ParticleSystemDataAoS> ps_aos;
    Particle p1 = makeParticle();
    p1.lifetime = 0.5f;
    ps_aos.addParticle(p1);

    ps_aos.update(1.0f, true);
    EXPECT_EQ(ps_aos.size(), 0u);

    ParticleSystem<ParticleSystemDataSoA> ps_soa;
    Particle p2 = makeParticle();
    p2.lifetime = 0.5f;
    ps_soa.addParticle(p2);

    ps_soa.update(1.0f, true);
    EXPECT_EQ(ps_soa.size(), 0u);
}

TEST(ParticleSystemTest, MultiStepUpdate)
{
    ParticleSystem<ParticleSystemDataAoS> ps_aos;
    ParticleSystem<ParticleSystemDataSoA> ps_soa;

    for (int i = 0; i < 100; ++i)
    {
        Particle p = makeParticle(0, 0, 1.0f, 2.0f, 0.1f, 0.2f, 10.0f);
        ps_aos.addParticle(p);
        ps_soa.addParticle(p);
    }

    for (int step = 0; step < 5; ++step)
    {
        ps_aos.update(0.1f);
        ps_soa.update(0.1f);
    }

    EXPECT_EQ(ps_aos.size(), ps_soa.size());
}

TEST(ParticleSystemAllocatedTest, StartsEmpty)
{
    ParticleSystemDataAllocated system(10);
    EXPECT_EQ(system.size(), 0u);
}

TEST(ParticleSystemAllocatedTest, AddIncreasesSize)
{
    ParticleSystemDataAllocated system(10);

    system.add(makeAliveParticle());
    EXPECT_EQ(system.size(), 1u);

    system.add(makeAliveParticle());
    EXPECT_EQ(system.size(), 2u);
}

TEST(ParticleSystemAllocatedTest, CapacityLimit)
{
    ParticleSystemDataAllocated system(2);

    EXPECT_NE(system.add(makeAliveParticle()), INVALID_INDEX);
    EXPECT_NE(system.add(makeAliveParticle()), INVALID_INDEX);
    EXPECT_EQ(system.add(makeAliveParticle()), INVALID_INDEX);
}

TEST(ParticleSystemAllocatedTest, DeadParticlesAreRemovedOnUpdate)
{
    ParticleSystemDataAllocated system(4);

    system.add(makeAliveParticle());
    system.add(makeDeadParticle());
    system.add(makeAliveParticle());

    EXPECT_EQ(system.size(), 3u);

    system.update(0.016f);
    EXPECT_EQ(system.size(), 2u);
}

TEST(ParticleSystemAllocatedTest, SlotReuseAfterDeath)
{
    ParticleSystemDataAllocated system(2);

    size_t first = system.add(makeDeadParticle());
    system.update(0.016f);

    size_t second = system.add(makeAliveParticle());

    EXPECT_EQ(first, second);
}

TEST(ParticleSystemDataSoAParallelized, UpdatesParticlesCorrectly)
{
    ParticleSystemDataSoAParallelized data(4);
    ThreadPoolScheduler scheduler(4);

    data.add(makeParticle(0, 0, 1, 0, 0, 0, 10.0f));
    data.add(makeParticle(0, 0, 0, 1, 0, 0, 10.0f));

    const float dt = 1.0f;
    data.updateParallel(dt, scheduler, false);

    auto particles = data.get();
    ASSERT_EQ(particles.size(), 2u);

    expectVecNear(particles[0].position, {1.0f, 0.0f});
    expectVecNear(particles[1].position, {0.0f, 1.0f});

    EXPECT_FLOAT_EQ(particles[0].lifetime, 9.0f);
    EXPECT_FLOAT_EQ(particles[1].lifetime, 9.0f);
}

TEST(ParticleSystemDataSoAParallelized, IntegratesAcceleration)
{
    ParticleSystemDataSoAParallelized data(1);
    ThreadPoolScheduler scheduler(2);

    data.add(makeParticle(0, 0, 1, 1, 1, 2, 5.0f));

    const float dt = 1.0f;
    data.updateParallel(dt, scheduler, false);

    auto p = data.get().front();

    // v = v + a * dt
    expectVecNear(p.velocity, {2.0f, 3.0f});

    // pos = pos + v * dt
    expectVecNear(p.position, {2.0f, 3.0f});
}

TEST(ParticleSystemDataSoAParallelized, KillsParticlesWhenLifetimeExpires)
{
    ParticleSystemDataSoAParallelized data(2);
    ThreadPoolScheduler scheduler(2);

    data.add(makeParticle(0, 0, 0, 0, 0, 0, 0.5f));
    data.add(makeParticle(0, 0, 0, 0, 0, 0, 2.0f));

    data.updateParallel(1.0f, scheduler, false);

    auto particles = data.get();
    ASSERT_EQ(particles.size(), 2u);

    EXPECT_FALSE(particles[0].alive);
    EXPECT_TRUE(particles[1].alive);
}

TEST(ParticleSystemDataSoAParallelized, CompactsDeadParticles)
{
    ParticleSystemDataSoAParallelized data(3);
    ThreadPoolScheduler scheduler(2);

    data.add(makeParticle(0, 0, 0, 0, 0, 0, 0.1f));
    data.add(makeParticle(1, 1, 0, 0, 0, 0, 10.0f));
    data.add(makeParticle(2, 2, 0, 0, 0, 0, 0.1f));

    data.updateParallel(1.0f, scheduler, true);

    auto particles = data.get();
    ASSERT_EQ(particles.size(), 1u);

    expectVecNear(particles[0].position, {1.0f, 1.0f});
    EXPECT_TRUE(particles[0].alive);
}

TEST(ParticleSystemDataSoAParallelized, MatchesSingleThreadedUpdate)
{
    constexpr size_t N = 1000;
    constexpr float dt = 0.016f;

    ParticleSystemDataSoA serial(N);
    ParticleSystemDataSoAParallelized parallel(N);
    ThreadPoolScheduler scheduler(4);

    for (size_t i = 0; i < N; ++i)
    {
        Particle p = makeParticle(
            float(i), float(i),
            1.0f, -1.0f,
            0.1f, 0.2f,
            5.0f);

        serial.add(p);
        parallel.add(p);
    }

    serial.update(dt, false);
    parallel.updateParallel(dt, scheduler, false);

    auto a = serial.get();
    auto b = parallel.get();

    ASSERT_EQ(a.size(), b.size());

    for (size_t i = 0; i < a.size(); ++i)
    {
        expectVecNear(a[i].position, b[i].position);
        expectVecNear(a[i].velocity, b[i].velocity);
        EXPECT_FLOAT_EQ(a[i].lifetime, b[i].lifetime);
        EXPECT_EQ(a[i].alive, b[i].alive);
    }
}

TEST(ParticleSystemDataSoAParallelized, DeterministicAcrossRuns)
{
    constexpr size_t N = 512;
    constexpr float dt = 0.033f;

    std::vector<Particle> baseline;

    for (int run = 0; run < 3; ++run)
    {
        ParticleSystemDataSoAParallelized data(N);
        ThreadPoolScheduler scheduler(8);

        for (size_t i = 0; i < N; ++i)
            data.add(makeParticle(0, 0, 1, 1, 0, 0, 10.0f));

        data.updateParallel(dt, scheduler, false);

        if (run == 0)
            baseline = data.get();
        else
        {
            auto current = data.get();
            ASSERT_EQ(current.size(), baseline.size());

            for (size_t i = 0; i < current.size(); ++i)
                expectVecNear(current[i].position, baseline[i].position);
        }
    }
}
