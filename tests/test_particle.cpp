#include <gtest/gtest.h>
#include "particlesim/particle_system.hpp"
using namespace particlesim;

TEST(ParticleBasics, DefaultInit) {
    Particle p;
    EXPECT_FLOAT_EQ(p.position.x, 0.0f);
    EXPECT_FLOAT_EQ(p.position.y, 0.0f);
}

TEST(ParticleUpdate, VelocityAffectsPosition) {
    Particle p;
    p.velocity.x = 1.0f;
    p.update(1.0f);
    EXPECT_FLOAT_EQ(p.velocity.x, 1.0f);
}

TEST(ParticleUpdate, AccelerationAffectsVelocity) {
    Particle p;
    p.acceleration.x = 10.0f;
    p.update(0.5f);
    EXPECT_FLOAT_EQ(p.velocity.x, 5.0f);
}

TEST(ParticleUpdate, LifetimeExpires) {
    Particle p;
    p.lifetime = 1.0f;
    p.update(2.0f);
    EXPECT_FALSE(p.alive);
}

TEST(ParticleSystemUpdate, UpdatesAllParticles) {
    ParticleSystem ps {};
    Particle p1;
    Particle p2;
    p1.velocity.x = 1.0f;
    p2.velocity.x = 2.0f;

    ps.add_particle(p1);
    ps.add_particle(p2);

    ps.update(1.0f);

    const auto& arr = ps.particles();
    EXPECT_FLOAT_EQ(arr[0].position.x, 1.0f);
    EXPECT_FLOAT_EQ(arr[1].position.x, 2.0f);
}