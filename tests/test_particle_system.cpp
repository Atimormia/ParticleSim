#include "particlesim/particle.hpp"
#include "particlesim/particle_system.hpp"
#include <gtest/gtest.h>

using namespace particlesim;

TEST(ParticleSystemTest, AddAndSize) {
    ParticleSystem container;
    Particle p{};
    size_t actres = container.add_particle(p);
    size_t expres = 0;
    EXPECT_EQ(actres, expres);
    Particle p0{};
    actres = container.add_particle(p0);
    expres = 1;
    EXPECT_EQ(actres, expres);
}

TEST(ParticleSystemTest, EulerIntegration) {
    ParticleSystem ps;
    Particle p{};
    p.velocity = {1.0f, 0.0f};
    p.acceleration = {0.5f, 0.0f};
    p.lifetime = 1.0f;
    ps.add_particle(p);

    ps.update(1.0f); // 1 second step
    const auto& particles = ps.particles();
    ASSERT_EQ(particles.size(), 1);
    ASSERT_FLOAT_EQ(particles[0].position.x, 1.5f); // 1 + 0.5*1
    ASSERT_FLOAT_EQ(particles[0].velocity.x, 1.5f); // 1 + 0.5*1
    ASSERT_FLOAT_EQ(particles[0].lifetime, 0.0f);
}

TEST(ParticleSystemTest, LifetimeExpiration) {
    ParticleSystem ps;
    Particle p{};
    p.lifetime = 0.5f;
    ps.add_particle(p);

    ps.update(1.0f); // particle should die
    ASSERT_EQ(ps.particles().size(), 0);
}