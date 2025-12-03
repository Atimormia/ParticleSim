#include "particlesim/particle.hpp"
#include <sstream>

using namespace particlesim;

void Particle::reset()
{
    position = {0.0f, 0.0f};
    velocity = {0.0f, 0.0f};
    acceleration = {0.0f, 0.0f};
    lifetime = 0.0f;
    alive = true;
}

void Particle::kill()
{
    alive = false;
}

void Particle::update(float dt)
{
    if (!alive)
        return;

    velocity += acceleration * dt;
    position += velocity * dt;

    lifetime -= dt;
    if (lifetime <= 0.0f)
    {
        alive = false;
    }
}
std::string particlesim::Particle::tostring() const
{
    std::ostringstream ss;
    ss << "Pos: " << position.tostring()
       << ", Vel: " << velocity.tostring()
       << ", Acc: " << acceleration.tostring();
    return ss.str();
}