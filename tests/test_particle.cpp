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