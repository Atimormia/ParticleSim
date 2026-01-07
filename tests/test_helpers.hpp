#pragma once
#include "particlesim/particle.hpp"
#include "particlesim/particle_system.hpp"
#include <gtest/gtest.h>

namespace particlesim
{
    Particle makeParticle(float px = 0.0f, float py = 0.0f, float vx = 1.0f, float vy = 0.0f, float ax = 0.0f, float ay = 0.0f, float lifetime = 1.0f);

    Particle makeAliveParticle();

    Particle makeDeadParticle();

    void expectVecNear(const Vector2D& a, const Vector2D& b, float eps = 1e-5f)
    {
        EXPECT_NEAR(a.x, b.x, eps);
        EXPECT_NEAR(a.y, b.y, eps);
    }


} // namespace particlesim
