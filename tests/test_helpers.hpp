#pragma once
#include "particlesim/particle.hpp"

namespace particlesim
{
    Particle make_test_particle(float vx = 1.0f, float vy = 0.0f, float ax = 0.0f, float ay = 0.0f, float lifetime = 1.0f);

    Particle makeAliveParticle();

    Particle makeDeadParticle();

} // namespace particlesim
