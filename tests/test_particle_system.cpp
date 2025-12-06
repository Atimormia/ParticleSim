#include "particlesim/particle.hpp"
#include "particlesim/particle_system.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace particlesim;

Particle make_test_particle(float vx = 1.0f, float vy = 0.0f, float ax = 0.0f, float ay = 0.0f, float lifetime = 1.0f)
{
    Particle p;
    p.velocity = {vx, vy};
    p.acceleration = {ax, ay};
    p.lifetime = lifetime;
    p.alive = true;
    return p;
}

TEST(ParticleSystemAoSTest, AddAndSize)
{
    ParticleSystem<ParticleSystemDataAoS> ps;
    EXPECT_EQ(ps.size(), 0u);

    ps.add_particle(make_test_particle());
    EXPECT_EQ(ps.size(), 1u);

    ps.add_particle(make_test_particle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemAoSTest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataAoS> ps;
    Particle p = make_test_particle(1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.add_particle(p);

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

    ps.add_particle(make_test_particle());
    EXPECT_EQ(ps.size(), 1u);

    ps.add_particle(make_test_particle());
    EXPECT_EQ(ps.size(), 2u);
}

TEST(ParticleSystemSoATest, EulerIntegration)
{
    ParticleSystem<ParticleSystemDataSoA> ps;
    Particle p = make_test_particle(1.0f, 0.0f, 0.5f, 0.0f, 1.1f);
    ps.add_particle(p);

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
    ps_aos.add_particle(p1);

    ps_aos.update(1.0f, true);
    EXPECT_EQ(ps_aos.size(), 0u);

    ParticleSystem<ParticleSystemDataSoA> ps_soa;
    Particle p2 = make_test_particle();
    p2.lifetime = 0.5f;
    ps_soa.add_particle(p2);

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
        ps_aos.add_particle(p);
        ps_soa.add_particle(p);
    }

    for (int step = 0; step < 5; ++step)
    {
        ps_aos.update(0.1f);
        ps_soa.update(0.1f);
    }

    EXPECT_EQ(ps_aos.size(), ps_soa.size());
}
