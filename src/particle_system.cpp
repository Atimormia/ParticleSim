#include "particlesim/particle_system.hpp"

using namespace particlesim;

ParticleSystem::ParticleSystem(size_t capacity) 
{
    particles_.reserve(capacity);
}

size_t ParticleSystem::add_particle(const Particle& p) 
{
    particles_.push_back(p);
    return particles_.size() - 1;
}

void ParticleSystem::update(float dt) 
{
    for (auto& p : particles_) 
    {
        if (p.alive) 
        {
            p.update(dt);
        }
    }
}
