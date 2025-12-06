#pragma once
#include "particle.hpp"
#include <vector>
#include <string>
#include "soa_container.hpp"

namespace particlesim
{
    template <typename T>
    concept ParticleLayout = requires(T layout, float dt, const Particle &p) {
        { layout.update(dt) } -> std::same_as<void>;
        { layout.size() } -> std::integral;
        { layout.add(p) } -> std::integral;
    };
    template <ParticleLayout Layout>
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 100000) : data(capacity) {}

        size_t add_particle(const Particle &p)
        {
            return data.add(p);
        }

        void update(float dt, bool compact = false)
        {
            data.update(dt, compact);
        }

        size_t size() const
        {
            return data.size();
        }

        std::vector<Particle> get() { return data.get(); }

    private:
        Layout data;
    };

    class ParticleSystemDataAoS
    {
    public:
        ParticleSystemDataAoS(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        std::vector<Particle> get() { return particles; }

        std::string tostring() const;

    private:
        std::vector<Particle> particles;
    };

    class ParticleSystemDataSoA
    {
    public:
        ParticleSystemDataSoA(size_t capacity = 100000);

        size_t add(const Particle &p);

        size_t size() const;

        void update(float dt, bool compact = false);

        std::vector<Particle> get();

    private:
        ParticleSoA particles;

        void compact_dead();
    };

} // namespace particlesim
