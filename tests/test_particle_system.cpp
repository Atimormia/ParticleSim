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

    ps.addParticle(make_test_particle());
    EXPECT_EQ(ps.size(), 1u);

    ps.addParticle(make_test_particle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemAoSTest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataAoS> ps;
    Particle p = make_test_particle(1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.addParticle(p);

    ps.update(1.0f,true);

    const auto& v = ps.get();
    const auto& particle = v[0];
    EXPECT_FLOAT_EQ(particle.velocity.x, 1.5f);
    EXPECT_FLOAT_EQ(particle.position.x, 1.5f);
    EXPECT_FLOAT_EQ(particle.lifetime, 0.1f);
}

TEST(ParticleSystemSoATest, AddAndSize)
{
    ParticleSystem<ParticleSystemDataSoA> ps;
    EXPECT_EQ(ps.size(), 0u);

    ps.addParticle(make_test_particle());
    EXPECT_EQ(ps.size(), 1u);

    ps.addParticle(make_test_particle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemSoATest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataSoA> ps;
    Particle p = make_test_particle(1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.addParticle(p);

    ps.update(1.0f,true);

    const auto particles = ps.get();
    const auto& out = particles[0];

    EXPECT_FLOAT_EQ(out.position.x, 1.5f);
}

TEST(ParticleSystemTest, LifetimeExpiration)
{
    ParticleSystem<ParticleSystemDataAoS> ps_aos;
    Particle p1 = make_test_particle();
    p1.lifetime = 0.5f;
    ps_aos.addParticle(p1);

    ps_aos.update(1.0f, true);
    EXPECT_EQ(ps_aos.size(), 0u);

    ParticleSystem<ParticleSystemDataSoA> ps_soa;
    Particle p2 = make_test_particle();
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
        Particle p = make_test_particle(1.0f, 2.0f, 0.1f, 0.2f, 10.0f);
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
