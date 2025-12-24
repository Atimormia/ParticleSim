#include "particlesim/particle_system.hpp"

namespace particlesim
{
    std::vector<Particle> ParticleSystemDataAllocated::get()
    {
        std::vector<Particle> out;
        out.reserve(activeIndices_.size());

        for (size_t index : activeIndices_)
        {
            out.push_back(pool_.get(index));
        }

        return out;
    }

    std::vector<Particle> ParticleSystemDataSoA::get()
    {
        std::vector<Particle> out;
        out.reserve(size());

        auto &[pos, vel, acc, life, alive] = fields();

        for (size_t i = 0; i < size(); i++)
        {
            Particle p;
            p.position = {pos.storage[0][i], pos.storage[1][i]};
            p.velocity = {vel.storage[0][i], vel.storage[1][i]};
            p.acceleration = {acc.storage[0][i], acc.storage[1][i]};
            p.lifetime = life[i];
            p.alive = (alive[i] != 0);

            out.push_back(p);
        }

        return out;
    }

    std::vector<Particle> ParticleSystemDataAoS::get()
    {
        return particles;
    }

    Particle make_test_particle(float vx = 1.0f, float vy = 0.0f, float ax = 0.0f, float ay = 0.0f, float lifetime = 1.0f)
    {
        Particle p;
        p.velocity = {vx, vy};
        p.acceleration = {ax, ay};
        p.lifetime = lifetime;
        p.alive = true;
        return p;
    }

    Particle makeAliveParticle()
    {
        Particle p;
        p.alive = true;
        return p;
    }

    Particle makeDeadParticle()
    {
        Particle p;
        p.alive = false;
        return p;
    }

} // namespace particlesim
