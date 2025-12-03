#include "particlesim/particle_system.hpp"
#include <sstream>

using namespace particlesim;

ParticleSystem::ParticleSystem(size_t capacity) 
{
    particles_.reserve(capacity);
}

particlesim::ParticleSystem::~ParticleSystem()
{
    particles_.clear();
}

size_t ParticleSystem::add_particle(const Particle& p) 
{
    particles_.push_back(p);
    return particles_.size() - 1;
}

void ParticleSystem::update(float dt) 
{
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
                       [](Particle& p){ return !p.alive; }),
        particles_.end());

    for (auto& p : particles_) 
    {
        if (p.alive) 
        {
            p.update(dt);
        }
    }
}

std::string ParticleSystem::tostring() const
{
    std::stringstream ss;
    for(const auto& p: particles_)
        ss << p.tostring() << "\n";
    return ss.str(); 
}
