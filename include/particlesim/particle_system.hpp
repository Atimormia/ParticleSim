#pragma once
#include "particle.hpp"
#include <vector>
namespace particlesim
{
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 10000);
        ~ParticleSystem() = default;

        size_t add_particle(const Particle& p);
        void update(float dt);

        const std::vector<Particle>& particles() const { return particles_; }

    private:
        std::vector<Particle> particles_;
    };

} // namespace particlesim
