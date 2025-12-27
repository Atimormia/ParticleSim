#include "particlesim/particle_system.hpp"
#include <sstream>
#include <algorithm>

namespace particlesim
{
    using namespace core;
    using namespace std;

    ParticleSystemDataAoS::ParticleSystemDataAoS(size_t capacity)
    {
        particles.reserve(capacity);
    }

    void ParticleSystemDataAoS::update(float dt, bool compact)
    {
        auto it = remove_if(particles.begin(), particles.end(), [&](Particle &p)
                            {
        if (!p.alive) 
            return true;
        p.update(dt);
        return !p.alive; });
        if (compact)
            particles.erase(it, particles.end());
    }

    size_t ParticleSystemDataAoS::add(const Particle &p)
    {
        particles.push_back(p);
        return particles.size() - 1;
    }

    size_t ParticleSystemDataAoS::size() const
    {
        return particles.size();
    }

    span<const Vector2D> particlesim::ParticleSystemDataAoS::positions()
    {
        const size_t count = particles.size();

        if (positionsCache_.size() < count)
            positionsCache_.resize(count);

        for (size_t i = 0; i < count; ++i)
            positionsCache_[i] = particles[i].position;

        return {positionsCache_.data(), count};
    }

    ParticleSystemDataSoA::ParticleSystemDataSoA(size_t capacity)
    {
        particles.reserve(capacity);
    }

    size_t ParticleSystemDataSoA::add(const Particle &p)
    {
        auto &[pos, vel, acc, life, alive] = fields();

        pos.push_back(p.position.x, p.position.y);
        vel.push_back(p.velocity.x, p.velocity.y);
        acc.push_back(p.acceleration.x, p.acceleration.y);
        life.push_back(p.lifetime);
        alive.push_back(p.alive ? 1 : 0);

        return particles.size() - 1;
    }

    size_t ParticleSystemDataSoA::size() const
    {
        return particles.size();
    }

    void ParticleSystemDataSoA::update(float dt, bool compact)
    {
        auto &[pos, vel, acc, life, alive] = fields();

        const size_t n = particles.size();
        if (n == 0)
            return;

        // raw pointers to contiguous storage - helps optimizer/vectorizer.
        float *pos_x = pos.x();
        float *pos_y = pos.y();
        float *vel_x = vel.x();
        float *vel_y = vel.y();
        float *acc_x = acc.x();
        float *acc_y = acc.y();
        float *life_p = life.data();
        uint8_t *alive_p = reinterpret_cast<uint8_t *>(alive.data());

        for (size_t i = 0; i < n; ++i)
        {
            if (alive_p[i] == 0)
                continue;

            float vx = vel_x[i] + acc_x[i] * dt;
            float vy = vel_y[i] + acc_y[i] * dt;
            vel_x[i] = vx;
            vel_y[i] = vy;

            pos_x[i] += vx * dt;
            pos_y[i] += vy * dt;

            float l = life_p[i] - dt;
            life_p[i] = l;
            if (l <= 0.0f)
                alive_p[i] = 0;
        }

        if (compact)
            compactDead();
    }

    span<const Vector2D> ParticleSystemDataSoA::positions()
    {
        auto &[pos, vel, acc, life, alive] = fields();
        const size_t count = pos.size();

        if (positionsCache_.size() < count)
            positionsCache_.resize(count);

        for (size_t i = 0; i < count; ++i)
            positionsCache_[i] = Vector2D(pos.x()[i], pos.y()[i]);

        return {positionsCache_.data(), count};
    }

    void ParticleSystemDataSoA::compactDead()
    {
        auto &[pos, vel, acc, life, alive] = fields();

        size_t n = particles.size();
        size_t i = 0;

        while (i < n)
        {
            if (!alive[i])
            {
                size_t last = n - 1;

                if (i != last)
                {
                    for (int8_t k = 0; k < 2; ++k)
                    {
                        pos.storage[k][i] = pos.storage[k][last];
                        vel.storage[k][i] = vel.storage[k][last];
                        acc.storage[k][i] = acc.storage[k][last];
                    }
                    life.storage[0][i] = life.storage[0][last];
                    alive.storage[0][i] = alive.storage[0][last];
                }

                for (int8_t k = 0; k < 2; ++k)
                {
                    pos.storage[k].pop_back();
                    vel.storage[k].pop_back();
                    acc.storage[k].pop_back();
                }
                life.storage[0].pop_back();
                alive.storage[0].pop_back();

                --n;
            }
            else
            {
                ++i;
            }
        }
    }
    size_t ParticleSystemDataAllocated::add(const Particle &p)
    {
        size_t index = pool_.allocate();
        if (index == INVALID_INDEX)
            return INVALID_INDEX;

        pool_.get(index) = p;
        activeIndices_.push_back(index);
        return index;
    }

    void ParticleSystemDataAllocated::update(float dt, bool /*compact*/)
    {
        for (size_t i = 0; i < activeIndices_.size();)
        {
            size_t index = activeIndices_[i];
            Particle &p = pool_.get(index);

            if (!p.alive)
            {
                pool_.deallocate(index);
                activeIndices_[i] = activeIndices_.back();
                activeIndices_.pop_back();
            }
            else
            {
                ++i;
                p.update(dt);
            }
        }
    }

    span<const Vector2D> ParticleSystemDataAllocated::positions()
    {
        const size_t count = activeIndices_.size();

        if (positionsCache_.size() < count)
            positionsCache_.resize(count);

        for (size_t i = 0; i < count; ++i)
            positionsCache_[i] = pool_.get(activeIndices_[i]).position;

        return {positionsCache_.data(), count};
    }

} // namespace particlesim
