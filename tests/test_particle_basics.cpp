#include <gtest/gtest.h>
#include "particlesim/particle.hpp"

TEST(ParticleBasics, DefaultInit) {
    particlesim::Particle p;
    EXPECT_FLOAT_EQ(p.x, 0.0f);
    EXPECT_FLOAT_EQ(p.y, 0.0f);
}
