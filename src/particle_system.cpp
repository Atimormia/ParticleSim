#include "particlesim/particle_system.hpp"
#include <sstream>

using namespace particlesim;
ParticleSystemDataAoS::ParticleSystemDataAoS(size_t capacity)
{
    particles.reserve(capacity);
}

void ParticleSystemDataAoS::update(float dt, bool compact)
{
    auto it = std::remove_if(particles.begin(), particles.end(), [&](Particle &p)
    {
        if (!p.alive) 
            return true;
        p.update(dt);
        return !p.alive; 
    });
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

std::span<const math::Vector2D> particlesim::ParticleSystemDataAoS::positions()
{
    return std::span<const math::Vector2D>();
}
std::string ParticleSystemDataAoS::tostring() const
{
    std::stringstream ss;
    for (const auto &p : particles)
        ss << p.tostring() << "\n";
    return ss.str();
}

ParticleSystemDataSoA::ParticleSystemDataSoA(size_t capacity)
{
    particles.reserve(capacity);
}

size_t ParticleSystemDataSoA::add(const Particle &p)
{
    auto &pos = particles.field<0>();
    auto &vel = particles.field<1>();
    auto &acc = particles.field<2>();
    auto &life = particles.field<3>();
    auto &alive = particles.field<4>();

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
    auto &pos = particles.field<0>();
    auto &vel = particles.field<1>();
    auto &acc = particles.field<2>();
    auto &life = particles.field<3>();
    auto &alive = particles.field<4>();

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
        compact_dead();
}

std::span<const math::Vector2D> particlesim::ParticleSystemDataSoA::positions()
{
    return std::span<const math::Vector2D>();
}

std::vector<Particle> ParticleSystemDataSoA::get()
{
    std::vector<Particle> out;
    out.reserve(size());

    auto &pos = particles.field<0>();
    auto &vel = particles.field<1>();
    auto &acc = particles.field<2>();
    auto &life = particles.field<3>();
    auto &alive = particles.field<4>();

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

void ParticleSystemDataSoA::compact_dead()
{
    auto &pos = particles.field<0>();
    auto &vel = particles.field<1>();
    auto &acc = particles.field<2>();
    auto &life = particles.field<3>();
    auto &alive = particles.field<4>();

    size_t n = particles.size();
    size_t i = 0;

    while (i < n)
    {
        if (!alive[i])
        {
            size_t last = n - 1;

            if (i != last)
            {
                for (int k = 0; k < 2; ++k)
                {
                    pos.storage[k][i] = pos.storage[k][last];
                    vel.storage[k][i] = vel.storage[k][last];
                    acc.storage[k][i] = acc.storage[k][last];
                }
                life.storage[0][i] = life.storage[0][last];
                alive.storage[0][i] = alive.storage[0][last];
            }

            for (int k = 0; k < 2; ++k)
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